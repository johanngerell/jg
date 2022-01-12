#pragma once

#include <type_traits>
#include <tuple>
#include <functional>
#include <string>
#include "jg_verify.h"
#include "jg_string.h"

//
// These mocking macros are defined and documented at the bottom of this file:
//
//   - JG_MOCK_EX
//   - JG_MOCK_REF_EX
//
// These compilation flags affect how jg::mock is built
//
//   - JG_MOCK_ENABLE_SHORT_NAMES: Enables mocking macros named without the "JG_" prefix.
//
// These compilation flags affect how jg::mock functions
//
//   - JG_MOCK_THROW_NOT_IMPLEMENTED: If a mocked function is called without an assigned func or result,
//     a std::logic_error with the prototype of the called function is thrown. This makes it easier to
//     use jg_mock with test frameworks like Catch2 and similar that handles tests that fail due to
//     thrown exceptions.
//
// Note that MSVC versions before Visual Studio 2019 might require
// /Zc:__cplusplus to enable checking #if (__cplusplus < 201402L) etc.
//

#ifdef JG_MOCK_THROW_NOT_IMPLEMENTED
#include <stdexcept>
#endif

namespace jg::detail {

#if (__cplusplus >= 201402L)
// C++20 has std::remove_cvref<>.
template <typename T>
using base_t = std::remove_const_t<std::remove_reference_t<T>>;
#else
template <typename T>
using base_t = typename std::remove_const<typename std::remove_reference<T>::type>::type;
#endif

template <typename ...T>
using tuple_params_t = std::tuple<base_t<T>...>;

#if (__cplusplus >= 201402L)
template<size_t N, typename... Params>
using nth_param_t = std::tuple_element_t<N, std::tuple<Params...>>;

template <size_t N, typename... Params>
using nth_param_base_t = base_t<nth_param_t<N, Params...>>;

template <size_t N, typename... Params>
nth_param_t<N, Params...> nth_param(Params&&... params)
{
    return std::get<N>(std::forward_as_tuple(params...));
}
#endif

// The auxiliary data for a mock function that takes N parameters has `param<1>(), ..., param<N>()` members
// holding the actual N parameters the function was last called with, for usage in tests.
template <size_t N, typename ...Params>
class mock_aux_parameters
{
public:
#if (__cplusplus >= 201402L)
    template <size_t Number>
    auto param() const { return std::get<Number - 1>(m_params); }
#else
    // Return type auto deduction is C++14.
    template <size_t Number>
    typename std::tuple_element<Number - 1, std::tuple<Params...> >::type param() const { return std::get<Number - 1>(m_params); }
#endif

protected:
    template <typename, typename>
    friend class mock_impl;

    template <typename ...Params2>
    void set_params(Params2&&... params) { m_params = std::make_tuple(params...); }

    tuple_params_t<Params...> m_params;
};

// The auxiliary data for a mock function that takes no parameters has no parameter-holding member.
template <>
class mock_aux_parameters<0>
{
};

// Verifies that the wrapped value is set before it gets used.
template <typename T, typename Enable = void>
class verified;

#if (__cplusplus >= 201402L)
// C++17 has std::is_reference_v<>.
template <typename T>
class verified<T, std::enable_if_t<std::is_reference<T>::value>> final
#else
template <typename T>
class verified<T, typename std::enable_if<std::is_reference<T>::value>::type> final
#endif
{
public:
    verified& operator=(T other)
    {
        m_value = &other;
        m_assigned = true;
        return *this;
    }

    operator T()
    {
        verify(m_assigned);
        return *m_value;
    }

    bool assigned() const
    {
        return m_assigned;
    }

private:
#if (__cplusplus >= 201402L)
    std::remove_reference_t<T>* m_value = nullptr;
#else
    typename std::remove_reference<T>::type* m_value = nullptr;
#endif
    bool m_assigned = false;
};

#if (__cplusplus >= 201402L)
// C++17 has std::is_reference_v.
template <typename T>
class verified<T, std::enable_if_t<!std::is_reference<T>::value>> final
#else
template <typename T>
class verified<T, typename std::enable_if<!std::is_reference<T>::value>::type> final
#endif
{
public:
    verified& operator=(const T& other)
    {
        m_value = other;
        m_assigned = true;
        return *this;
    }

    verified& operator=(T&& other)
    {
        m_value = std::move(other);
        m_assigned = true;
        return *this;
    }

    operator T()
    {
        verify(m_assigned);
        return m_value;
    }

    bool assigned() const
    {
        return m_assigned;
    }

private:
    T m_value{};
    bool m_assigned = false;
};

template <typename TImpl>
class mock_aux_return_base
{
public:
	// If JG_MOCK_THROW_NOT_IMPLEMENTED is defined and func or result isn't set for a void function mock,
	// or func isn't set for a non-void function mock, then a std::logic_error exception is thrown if the
	// function is called. By calling stub() on the mock, or declaring the mock with any of the JG_STUB*
	// macros, the function can be called without setting func or result. This is handy when the function
	// has a long parameter list and writing an empty lambda for it is a bit tiresome. Making the mock a
	// stub is a persistent change, i.e., the mock remains a stub after being reset.
	void stub() { m_stub = true; static_cast<TImpl*>(this)->on_stub(); }
	bool is_stub() const { return m_stub; }

private:
	bool m_stub = false;
};

// The auxiliary data for a mock function that returns non-`void` has a `result` member
// that can be set in tests. This is the simplest way to just return a specific value from a
// mock function. The other way is to assign a callable (like a lambda) to the `func` member,
// but if just a return value needs to be modeled, then `result` is the easier way.
template <typename T>
class mock_aux_return : public mock_aux_return_base<mock_aux_return<T>>
{
public:
    // Verifying that a test doesn't use it without first setting it.
	// Setting the mock result is a persistent change, i.e., the result remains after the mock is reset.
    verified<T> result;

private:
	friend class mock_aux_return_base<mock_aux_return<T>>;
	void on_stub() { result = T{}; }
};

// The auxiliary data for a mock function that returns `void` has no `result` member, and
// the function implementation can only be controlled by assigning a callable (like a lambda)
// to the `func` member.
template <>
class mock_aux_return<void> : public mock_aux_return_base<mock_aux_return<void>>
{
private:
	friend class mock_aux_return_base<mock_aux_return<void>>;
	void on_stub() {}
};

template <typename T, typename ...Params>
class mock_aux final : public mock_aux_return<T>, public mock_aux_parameters<sizeof...(Params), Params...>
{
public:
	// The mock prototype is persistent, i.e., the it remains after the mock is reset.
    mock_aux(std::string prototype)
        : m_count(0)
        , m_prototype(trim(prototype, " "))
    {}

	// Setting the mock func is a persistent change, i.e., the func remains after the mock is reset.
    std::function<T(Params...)> func;
    size_t                      count() const { return m_count; }
    bool                        called() const { return m_count > 0; }
    std::string                 prototype() const { return m_prototype; }

    void                        reset()
    {
        m_count = 0;
        m_params = {};//tuple_params_t<Params...>
    }

private:
    template <typename, typename>
    friend class mock_impl;

    size_t m_count;
    std::string m_prototype;
};

template <typename T, typename TImpl, typename Enable = void>
class mock_impl_base;

// A mock function that returns `void` only calls `func` in its
// auxiliary data if it's set in the test. Nothing is done if it's not set.
#if (__cplusplus >= 201402L)
// C++17 has std::is_same_v<>.
template <typename T, typename TImpl>
class mock_impl_base<T, TImpl, std::enable_if_t<std::is_same<T, void>::value>>
#else
template <typename T, typename TImpl>
class mock_impl_base<T, TImpl, typename std::enable_if<std::is_same<T, void>::value>::type>
#endif
{
public:
    template <typename... Params>
    void impl(Params&&... params)
    {
        auto& aux = static_cast<TImpl*>(this)->m_aux;

		#ifdef JG_MOCK_THROW_NOT_IMPLEMENTED
		if (!aux.func && !aux.is_stub())
			throw std::logic_error(std::string("No func set for mocked function '").append(aux.prototype()).append("'."));
		#endif

        if (aux.func)
            aux.func(std::forward<Params>(params)...);
    }
};

// A mock function that returns non-`void` calls the `func` member
// in its auxiliary data if it's set in the test. If the `func` member isn't set, then the `result`
// member is used instead. If the `result` member isn't set, then by default an assertion failure is
// triggered and a stack trace is output, or the default value for the `result` member type is
// returned if the bahavior is non-default. The documentation for `jg::verify` has details on how to
// configure the assertion behavior at compile time.
// @see jg::verify
#if (__cplusplus >= 201402L)
// C++17 has std::is_same_v<>.
template <typename T, typename TImpl>
class mock_impl_base<T, TImpl, std::enable_if_t<!std::is_same<T, void>::value>>
#else
template <typename T, typename TImpl>
class mock_impl_base<T, TImpl, typename std::enable_if<!std::is_same<T, void>::value>::type>
#endif
{
public:
    template <typename... Params>
    T impl(Params&&... params)
    {
        auto& aux = static_cast<TImpl*>(this)->m_aux;

		#ifdef JG_MOCK_THROW_NOT_IMPLEMENTED
		if (!aux.func && !aux.result.assigned() && !aux.is_stub())
			throw std::logic_error(std::string("No func or result set for mocked function '").append(aux.prototype()).append("'."));
		#endif

        if (aux.func)
            return aux.func(std::forward<Params>(params)...);

        return aux.result;
    }
};

// A mock function does 2 things: it makes sure that its auxiliary data state (call counter, etc.)
// has been updated when the function returns, and it calls the client supplied callable or returns
// the client supplied result.
template <typename T, typename TMockAux>
class mock_impl final : public mock_impl_base<T, mock_impl<T, TMockAux>>
{
public:
    mock_impl(const TMockAux& aux)
        : m_aux(const_cast<TMockAux&>(aux)) // Minor hack to be able to use the same JG_MOCK_EX macro for
                                            // both member functions and free functions. `mutable`
                                            // would otherwise be needed for some members,
                                            // and that would require separate macro implementations.
                                            // It's a "minor" hack because it's an implementation
                                            // detail and we know that the original instance is non-const.
    {}

    ~mock_impl()
    {
        m_aux.m_count++;
    }

    template <typename... Params>
    T impl(Params&&... params)
    {
        m_aux.set_params(std::forward<Params>(params)...);
        return mock_impl_base<T, mock_impl<T, TMockAux>>::impl(std::forward<Params>(params)...);
    }

    T impl()
    {
        return mock_impl_base<T, mock_impl<T, TMockAux>>::impl();
    }

private:
	friend class mock_impl_base<T, mock_impl<T, TMockAux>>;
    TMockAux& m_aux;
};

} // namespace jg::detail

#define _JG_CONCAT5(x1, x2, x3, x4, x5) x1 ## x2 ## x3 ## x4 ## x5
#define _JG_CONCAT2(x1, x2) x1 ## x2
#define _JG_CONCAT(x1, x2) _JG_CONCAT2(x1, x2)

//
// Clang and GCC handles empty __VA_ARGS__ differently than MSVC.
//
#ifdef _MSC_VER
    // The core of _JG_OVERLOADED_MACRO for MSVC comes from here:
    //
    //   * https://stackoverflow.com/questions/5530505/why-does-this-variadic-argument-count-macro-fail-with-vc
    //   * https://stackoverflow.com/questions/26682812/argument-counting-macro-with-zero-arguments-for-visualstudio
    //   * https://stackoverflow.com/questions/9183993/msvc-variadic-macro-expansion?rq=1
    //
    #define _JG_EXPAND(x) x
    #define _JG_GLUE(x1, x2) x1 x2
    #define _JG_VA_COUNT_2(x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, ...) x12
    #define _JG_VA_COUNT_1(...) _JG_EXPAND(_JG_VA_COUNT_2(__VA_ARGS__,10,9,8,7,6,5,4,3,2,1,0))
    #define _JG_AUGMENTER(...) unused, __VA_ARGS__
    #define _JG_VA_COUNT(...) _JG_VA_COUNT_1(_JG_AUGMENTER(__VA_ARGS__))
    #define _JG_OVERLOADED_MACRO(name, ...) \
        _JG_GLUE(_JG_CONCAT(name, _JG_VA_COUNT(__VA_ARGS__)), (__VA_ARGS__))
#else
    // The core of _JG_OVERLOADED_MACRO for GCC and CLANG comes from here:
    //
    //   * https://stackoverflow.com/a/47425231/6345
    //
    #define _JG_ARG16(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, ...) _15
    #define _JG_HAS_COMMA(...) _JG_ARG16(__VA_ARGS__, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0)
    #define _JG_HAS_NO_COMMA(...) _JG_ARG16(__VA_ARGS__, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1)
    #define _JG_TRIGGER_PARENTHESIS_(...) ,
    #define _JG_IS_EMPTY_CASE_0001 ,
    #define _JG_HAS_ZERO_OR_ONE_ARGS2(_0, _1, _2, _3) _JG_HAS_NO_COMMA(_JG_CONCAT5(_JG_IS_EMPTY_CASE_, _0, _1, _2, _3))
    #define _JG_HAS_ZERO_OR_ONE_ARGS(...) \
        _JG_HAS_ZERO_OR_ONE_ARGS2( \
        /* test if there is just one argument, possibly empty */ \
        _JG_HAS_COMMA(__VA_ARGS__), \
        /* test if _TRIGGER_PARENTHESIS_ together with the argument adds a comma */ \
        _JG_HAS_COMMA(_JG_TRIGGER_PARENTHESIS_ __VA_ARGS__), \
        /* test if the argument together with a parenthesis adds a comma */ \
        _JG_HAS_COMMA(__VA_ARGS__ (~)), \
        /* test if placing it between _TRIGGER_PARENTHESIS_ and the parenthesis adds a comma */ \
        _JG_HAS_COMMA(_JG_TRIGGER_PARENTHESIS_ __VA_ARGS__ (~)) \
        )

    #define _JG_VA0(...) _JG_HAS_ZERO_OR_ONE_ARGS(__VA_ARGS__)
    #define _JG_VA1(...) _JG_HAS_ZERO_OR_ONE_ARGS(__VA_ARGS__)
    #define _JG_VA2(...) 2
    #define _JG_VA3(...) 3
    #define _JG_VA4(...) 4
    #define _JG_VA5(...) 5
    #define _JG_VA6(...) 6
    #define _JG_VA7(...) 7
    #define _JG_VA8(...) 8
    #define _JG_VA9(...) 9
    #define _JG_VA10(...) 10
    #define _JG_VA11(...) 11
    #define _JG_VA12(...) 12
    #define _JG_VA13(...) 13
    #define _JG_VA14(...) 14
    #define _JG_VA15(...) 15
    #define _JG_VA16(...) 16

    #define _JG_VA_NUM_ARGS_N(_1, _2, _3, _4, _5, _6, _7, _8, _9,_10, _11,_12,_13,_14,_15,_16,N,...) N
    #define _JG_VA_NUM_ARGS_IMPL(...) _JG_VA_NUM_ARGS_N(__VA_ARGS__)
    #define _JG_VA_NUM_ARGS(...) _JG_VA_NUM_ARGS_IMPL(__VA_ARGS__, _JG_PP_RSEQ_N(__VA_ARGS__) )

    #define _JG_PP_RSEQ_N(...) \
        _JG_VA16(__VA_ARGS__),_JG_VA15(__VA_ARGS__),_JG_VA14(__VA_ARGS__),_JG_VA13(__VA_ARGS__), \
        _JG_VA12(__VA_ARGS__),_JG_VA11(__VA_ARGS__),_JG_VA10(__VA_ARGS__),_JG_VA9(__VA_ARGS__), \
        _JG_VA8(__VA_ARGS__),_JG_VA7(__VA_ARGS__),_JG_VA6(__VA_ARGS__),_JG_VA5(__VA_ARGS__), \
        _JG_VA4(__VA_ARGS__),_JG_VA3(__VA_ARGS__),_JG_VA2(__VA_ARGS__),_JG_VA1(__VA_ARGS__), \
        _JG_VA0(__VA_ARGS__)

    #define _JG_OVERLOADED_MACRO(name, ...) \
        _JG_CONCAT(name, _JG_VA_NUM_ARGS(__VA_ARGS__)) (__VA_ARGS__)
#endif

#define _JG_MOCK_FUNC_PARAMS_DECL_0()
#define _JG_MOCK_FUNC_PARAMS_DECL_1(T1) T1 p1
#define _JG_MOCK_FUNC_PARAMS_DECL_2(T1, T2) T1 p1, T2 p2
#define _JG_MOCK_FUNC_PARAMS_DECL_3(T1, T2, T3) T1 p1, T2 p2, T3 p3
#define _JG_MOCK_FUNC_PARAMS_DECL_4(T1, T2, T3, T4) T1 p1, T2 p2, T3 p3, T4 p4
#define _JG_MOCK_FUNC_PARAMS_DECL_5(T1, T2, T3, T4, T5) T1 p1, T2 p2, T3 p3, T4 p4, T5 p5
#define _JG_MOCK_FUNC_PARAMS_DECL_6(T1, T2, T3, T4, T5, T6) T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6
#define _JG_MOCK_FUNC_PARAMS_DECL_7(T1, T2, T3, T4, T5, T6, T7) T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7
#define _JG_MOCK_FUNC_PARAMS_DECL_8(T1, T2, T3, T4, T5, T6, T7, T8) T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8
#define _JG_MOCK_FUNC_PARAMS_DECL_9(T1, T2, T3, T4, T5, T6, T7, T8, T9) T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8, T9 p9
#define _JG_MOCK_FUNC_PARAMS_DECL_10(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10) T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8, T9 p9, T10 p10

#define _JG_MOCK_FUNC_PARAMS_CALL_0()
#define _JG_MOCK_FUNC_PARAMS_CALL_1(T1) p1
#define _JG_MOCK_FUNC_PARAMS_CALL_2(T1, T2) p1, p2
#define _JG_MOCK_FUNC_PARAMS_CALL_3(T1, T2, T3) p1, p2, p3
#define _JG_MOCK_FUNC_PARAMS_CALL_4(T1, T2, T3, T4) p1, p2, p3, p4
#define _JG_MOCK_FUNC_PARAMS_CALL_5(T1, T2, T3, T4, T5) p1, p2, p3, p4, p5
#define _JG_MOCK_FUNC_PARAMS_CALL_6(T1, T2, T3, T4, T5, T6) p1, p2, p3, p4, p5, p6
#define _JG_MOCK_FUNC_PARAMS_CALL_7(T1, T2, T3, T4, T5, T6, T7) p1, p2, p3, p4, p5, p6, p7
#define _JG_MOCK_FUNC_PARAMS_CALL_8(T1, T2, T3, T4, T5, T6, T7, T8) p1, p2, p3, p4, p5, p6, p7, p8
#define _JG_MOCK_FUNC_PARAMS_CALL_9(T1, T2, T3, T4, T5, T6, T7, T8, T9) p1, p2, p3, p4, p5, p6, p7, p8, p9
#define _JG_MOCK_FUNC_PARAMS_CALL_10(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10) p1, p2, p3, p4, p5, p6, p7, p8, p9, p10

#define _JG_MOCK_FUNC_PARAMS_DECL(...) \
    _JG_OVERLOADED_MACRO(_JG_MOCK_FUNC_PARAMS_DECL_, __VA_ARGS__)

#define _JG_MOCK_FUNC_PARAMS_CALL(...) \
    _JG_OVERLOADED_MACRO(_JG_MOCK_FUNC_PARAMS_CALL_, __VA_ARGS__)

// --------------------------- Implementation details go above this line ---------------------------

/// @macro JG_MOCK_EX
///
/// Defines a free function, or a virtual member function override, with auxiliary data for use in testing.
///
/// @section Mocking a virtual member function
///
/// Assume that the virtual function `find_by_id` is a member of the base class `user_names`:
///
///     class user_names
///     {
///     public:
///         virtual const char* find_by_id(int id) = 0;
///         ...
///     };
///
/// We can mock that function in a test by deriving a mock class `mock_user_names` from `user_names`
/// and use the `JG_MOCK_EX` macro in its body:
///
///     class mock_user_names final : public user_names
///     {
///     public:
///         JG_MOCK_EX(,,, const char*, find_by_id, int);
///         ...
///     };
///
/// It's functionally equivalent to write this:
///
///     class mock_user_names final : public user_names
///     {
///     public:
///         JG_MOCK_EX(virtual, override,, const char*, find_by_id, int);
///         ...
///     };
///
/// Since `virtual` and `override` aren't strictly mandatory to use when overriding virtual base class
/// functions, they can be omitted from the `JG_MOCK_EX` declaration to make it easier to read. As the
/// `JG_MOCK_EX` declaration shows, its first two parameters are named `prefix` and `suffix` and they'll
/// simply be pasted before and after the function declaration by the macro. The third parameter is named
/// `overload_suffix` and it's empty in this example, which just means that we're not mocking an overloaded
/// function. If we were, we could set a suffix here that would be added to the auxiliary data
/// name for the particular overload.
///
/// The mock class can be instantiated in a test and passed to a type or a function under test
/// that depends on the base class `user_names` for its functionality. By doing this, we can manipulate how
/// that tested entity behaves regarding how it uses its `user_names` dependency. The `JG_MOCK_EX` macro sets
/// up some tools to help with that task in the test - the mock function auxiliary data.
///
/// The `JG_MOCK_EX` macro does two things. First, it sets up an auxiliary data structure that can be used
/// to 1) control what the mock function does when it's called, and 2) examine how it was called. Second,
/// it creates a function body that will use the auxiliary data in it's implementation.
///
/// The mock function implementation can be controlled in a test 1) by intercepting calls to it by setting
/// the auxiliary data `func` to a callable (like a lambda) that does the actual implementation, or 2) by
/// setting the auxiliary data `result` to a value that will be returned by the function.
///
/// Typical usage of a the mock class we defined above:
///
///     TEST("tested entity can do its job")
///     {
///         // The mock class instance.
///         mock_user_names names;
///
///         // Whenever user_names::find_by_id is called by the tested entity, it always returns "Donald Duck".
///         names.find_by_id_.result = "Donald Duck";
///
///         // Depends on user_names
///         some_tested_entity tested_entity(user_names);
///
///         TEST_ASSERT(tested_entity.can_do_its_job());      // Allegedly uses user_names::find_by_id()
///         TEST_ASSERT(names.find_by_id_.called());          // Did the tested entity even call it?
///         TEST_ASSERT(names.find_by_id_.param<1>() < 4711); // Did the tested entity pass it a valid id?
///     }
///
/// The `func` auxiliary data member can be used instead of `result`, or when the mock function is
/// a void function and the auxiliary data simply doesn't have a `result` member.
///
///     TEST("some_tested_entity retries twice when failing")
///     {
///         // The mock class instance.
///         mock_user_names names;
///
///         // Make the mock fail twice.
///         names.find_by_id_.func = [] (int id)
///         {
///             switch (id)
///             {
///                 case 0:  return "Huey";
///                 case 1:  return "Dewey";
///                 case 2:  return "Louie";
///                 default: return nullptr;
///             }
///         };
///
///         // Depends on user_names
///         nephew_reporter tested_entity(user_names);
///
///         const std::string expected_format = "Huey! Dewey? Louie!?";
///
///         TEST_ASSERT(tested_entity.format_nephews() == expected_format);
///         TEST_ASSERT(user_names.find_by_id_.count() == 3);
///     }
///
/// @section Mocking a free function
///
/// Assume that there is a free function `find_by_id` that we call in production code:
///
///     const char* find_by_id(int id);
///
/// We can mock it in a test, using the `JG_MOCK_EX` macro in the same way as with the virtual function:
///
///     JG_MOCK_EX(,,, const char*, find_by_id, int);
///
/// Typical usage of this mock:
///
///     TEST("tested entity can do its job")
///     {
///         find_by_id_.reset();
///         find_by_id_.result = "Donald Duck";
///
///         // Depends on find_by_id()
///         some_tested_entity tested_entity;
///
///         TEST_ASSERT(tested_entity.can_do_its_job()); // Allegedly uses find_by_id()
///         TEST_ASSERT(find_by_id_.called());           // Did the tested entity even call it?
///         TEST_ASSERT(find_by_id_.param<1>() < 4711);  // Did the tested entity pass it a valid id?
///     }
///
/// Note that, since this mock is global, its auxiliary data must be `reset()` in each test case.
/// That's not needed when virtual functions of a class is mocked, since their auxiliary data state
/// gets reset every time the mock class is instantiated. Regardless if a mock is global or not, its
/// auxiliary data _can_ be reset at any time in a test if needed.
///
/// @section Auxiliary data members
///
/// The auxiliary data members available for a mock function `foo` that returns void and takes 0 parameters are:
///
///     std::function<void()>            foo_.func;        // can be set in a test
///     ---------------------------------------------------------------------------
///     bool                             foo_.called();    // set by the mocking framework
///     size_t                           foo_.count();     // set by the mocking framework
///     std::string                      foo_.prototype(); // set by the mocking framework
///
/// The auxiliary data members available for a mock function `foo` that returns `T` and takes 0 parameters are:
///
///     std::function<T()>               foo_.func;        // can be set in a test
///     T                                foo_.result;      // can be set in a test
///     ---------------------------------------------------------------------------
///     bool                             foo_.called();    // set by the mocking framework
///     size_t                           foo_.count();     // set by the mocking framework
///     std::string                      foo_.prototype(); // set by the mocking framework
///
/// The auxiliary data members available for a mock function `foo` that returns void and takes N parameters of types T1..TN:
///
///     std::function<void(T1, ..., TN)> foo_.func;        // can be set in a test
///     ---------------------------------------------------------------------------
///     bool                             foo_.called();    // set by the mocking framework
///     size_t                           foo_.count();     // set by the mocking framework
///     std::string                      foo_.prototype(); // set by the mocking framework
///     T1                               foo_.param<1>();  // set by the mocking framework
///     .                                .
///     .                                .
///     TN                               foo_.param<N>()   // set by the mocking framework
///
/// The auxiliary data members available for a mock function `foo` that returns T and takes N parameters of types T1..TN:
///
///     std::function<void(T1, ..., TN)> foo_.func;        // can be set in a test
///     T                                foo_.result;      // can be set in a test
///     ---------------------------------------------------------------------------
///     bool                             foo_.called();    // set by the mocking framework
///     size_t                           foo_.count();     // set by the mocking framework
///     std::string                      foo_.prototype(); // set by the mocking framework
///     T1                               foo_.param<1>();  // set by the mocking framework
///     .                                .
///     .                                .
///     TN                               foo_.param<N>()   // set by the mocking framework
///
/// @param prefix "Things to the left in a function declaration". For instance `static`, `virtual`, etc. - often empty.
/// @param suffix "Things to the right in a function declaration". For instance `override`, `const`, `noexcept`, etc. - often empty.
/// @param overload_suffix An arbitrary suffix added to the auxiliary data name of overloaded functions to discriminate between them in tests - often empty.
/// @param return_type The type of the return value, or `void`.
/// @param function_name The name of the function to mock.
/// @param variadic Variadic parameter list of parameter types for the function, if any.
#define JG_MOCK_EX(prefix, suffix, overload_suffix, return_type, function_name, ...) \
    jg::detail::mock_aux<return_type, ##__VA_ARGS__> \
    function_name ## overload_suffix ## _ \
    {#return_type " " #function_name "(" #__VA_ARGS__ ") " #suffix}; \
    prefix return_type function_name(_JG_MOCK_FUNC_PARAMS_DECL(__VA_ARGS__)) suffix \
    { \
        return jg::detail::mock_impl<return_type, decltype(function_name ## overload_suffix ## _)> \
               (function_name ## overload_suffix ## _).impl(_JG_MOCK_FUNC_PARAMS_CALL(__VA_ARGS__)); \
    }

#define JG_MOCK(return_type, function_name, ...) \
    jg::detail::mock_aux<return_type, ##__VA_ARGS__> \
    function_name ## _ \
    {#return_type " " #function_name "(" #__VA_ARGS__ ")"}; \
    return_type function_name(_JG_MOCK_FUNC_PARAMS_DECL(__VA_ARGS__)) \
    { \
        return jg::detail::mock_impl<return_type, decltype(function_name ## _)> \
               (function_name ## _).impl(_JG_MOCK_FUNC_PARAMS_CALL(__VA_ARGS__)); \
    }

/// @macro JG_STUB_EX
///
/// Same as JG_MOCK_EX, but with the stub() function invoked automatically, to easily "stub out" functions.
#define JG_STUB_EX(prefix, suffix, overload_suffix, return_type, function_name, ...) \
    jg::detail::mock_aux<return_type, ##__VA_ARGS__> \
    function_name ## overload_suffix ## _ = [] { \
    	jg::detail::mock_aux<return_type, ##__VA_ARGS__> \
    	aux{#return_type " " #function_name "(" #__VA_ARGS__ ") " #suffix}; \
		aux.stub(); return aux;}();\
    prefix return_type function_name(_JG_MOCK_FUNC_PARAMS_DECL(__VA_ARGS__)) suffix \
    { \
        return jg::detail::mock_impl<return_type, decltype(function_name ## overload_suffix ## _)> \
               (function_name ## overload_suffix ## _).impl(_JG_MOCK_FUNC_PARAMS_CALL(__VA_ARGS__)); \
    }

#define JG_STUB(return_type, function_name, ...) \
    jg::detail::mock_aux<return_type, ##__VA_ARGS__> \
    function_name ## _ = [] { \
    	jg::detail::mock_aux<return_type, ##__VA_ARGS__> \
    	aux{#return_type " " #function_name "(" #__VA_ARGS__ ") "}; \
		aux.stub(); return aux;}();\
    return_type function_name(_JG_MOCK_FUNC_PARAMS_DECL(__VA_ARGS__)) \
    { \
        return jg::detail::mock_impl<return_type, decltype(function_name ## _)> \
               (function_name ## _).impl(_JG_MOCK_FUNC_PARAMS_CALL(__VA_ARGS__)); \
    }

/// @macro JG_MOCK_COM_EX
///
/// Same as JG_MOCK_EX, but with the calling convention STDMETHODCALLTYPE added to the function prototype
#ifdef STDMETHODCALLTYPE
#define JG_MOCK_COM_EX(prefix, suffix, overload_suffix, return_type, function_name, ...) \
    jg::detail::mock_aux<return_type, ##__VA_ARGS__> \
    function_name ## overload_suffix ## _ \
    {#return_type " " #function_name "(" #__VA_ARGS__ ") " #suffix}; \
    prefix return_type STDMETHODCALLTYPE function_name(_JG_MOCK_FUNC_PARAMS_DECL(__VA_ARGS__)) suffix \
    { \
        return jg::detail::mock_impl<return_type, decltype(function_name ## overload_suffix ## _)> \
               (function_name ## overload_suffix ## _).impl(_JG_MOCK_FUNC_PARAMS_CALL(__VA_ARGS__)); \
    }
#define JG_MOCK_COM(return_type, function_name, ...) \
    jg::detail::mock_aux<return_type, ##__VA_ARGS__> \
    function_name ## _ \
    {#return_type " " #function_name "(" #__VA_ARGS__ ") "}; \
    return_type STDMETHODCALLTYPE function_name(_JG_MOCK_FUNC_PARAMS_DECL(__VA_ARGS__)) \
    { \
        return jg::detail::mock_impl<return_type, decltype(function_name ## _)> \
               (function_name ## _).impl(_JG_MOCK_FUNC_PARAMS_CALL(__VA_ARGS__)); \
    }
/// @macro JG_STUB_COM_EX
///
/// Same as JG_MOCK_COM_EX, but with the stub() function invoked automatically.
#define JG_STUB_COM_EX(prefix, suffix, overload_suffix, return_type, function_name, ...) \
    jg::detail::mock_aux<return_type, ##__VA_ARGS__> \
    function_name ## overload_suffix ## _ = [] { \
    	jg::detail::mock_aux<return_type, ##__VA_ARGS__> \
    	aux{#return_type " " #function_name "(" #__VA_ARGS__ ") " #suffix}; \
		aux.stub(); return aux;}();\
    prefix return_type STDMETHODCALLTYPE function_name(_JG_MOCK_FUNC_PARAMS_DECL(__VA_ARGS__)) suffix \
    { \
        return jg::detail::mock_impl<return_type, decltype(function_name ## overload_suffix ## _)> \
               (function_name ## overload_suffix ## _).impl(_JG_MOCK_FUNC_PARAMS_CALL(__VA_ARGS__)); \
    }
#define JG_STUB_COM(return_type, function_name, ...) \
    jg::detail::mock_aux<return_type, ##__VA_ARGS__> \
    function_name ## _ = [] { \
    	jg::detail::mock_aux<return_type, ##__VA_ARGS__> \
    	aux{#return_type " " #function_name "(" #__VA_ARGS__ ") "}; \
		aux.stub(); return aux;}();\
    return_type STDMETHODCALLTYPE function_name(_JG_MOCK_FUNC_PARAMS_DECL(__VA_ARGS__)) \
    { \
        return jg::detail::mock_impl<return_type, decltype(function_name ## _)> \
               (function_name ## _).impl(_JG_MOCK_FUNC_PARAMS_CALL(__VA_ARGS__)); \
    }
#else
#define JG_MOCK_COM_EX JG_MOCK_EX
#define JG_MOCK_COM JG_MOCK
#define JG_STUB_COM_EX JG_MOCK_EX
#define JG_STUB_COM JG_STUB
#endif // STDMETHODCALLTYPE

/// @macro JG_MOCK_REF_EX
///
/// Makes an `extern` declaration of the auxiliary data defined by a corresponding usage of `JG_MOCK_EX` in
/// a .cpp file. This makes it possible to only have one definition of a mock function in an entire test
/// program, and using its auxiliary data in other translation units.
///
/// Mocking the free function `foo* foolib_create(const char* id)` in one translation unit and using it in
/// two other translation units can be done like this:
///
///   * foolib_mocks.cpp
///
///         #include "flubber_mocks.h"
///
///         JG_MOCK_EX(,,, foo*, foolib_create, const char*);
///
///   * flubber_mocks.h
///
///         #include <foolib.h>
///         #include <jg/jg_mock.h>
///
///         JG_MOCK_REF_EX(,,, foo*, foolib_create, const char*);
///
///   * flubber_tests.cpp
///
///         #include <flubber.h>
///         #include "foolib_mocks.h"
///
///         TEST("A flubber can do it")
///         {
///             foo dummy_result;
///             foolib_create_.reset();
///             foolib_create_.result = reinterpret_cast<foo*>(&dummy_result);
///
///             flubber f; // System under test - depends on foolib
///
///             TEST_ASSERT(f.do_it()); // If foolib_create succeeds, flubber can do it
///             TEST_ASSERT(foolib_create_.called());
///             TEST_ASSERT(foolib_create_.param<1>() != nullptr);
///         }
///
///   * fiddler_tests.cpp
///
///         #include <fiddler.h>
///         #include "foolib_mocks.h"
///
///         TEST("A fiddler can't play")
///         {
///             foolib_create_.reset();
///             foolib_create_.result = nullptr;
///
///             fiddler f; // System under test - depends on foolib
///
///             TEST_ASSERT(!f.play()); // If foolib_create fails, fiddler can't play
///             TEST_ASSERT(foolib_create_.called());
///             TEST_ASSERT(foolib_create_.param<1>() != nullptr);
///         }
///
/// Mismatched `JG_MOCK_EX` and `JG_MOCK_REF_EX` declarations leads to compilation and linker errors.
#define JG_MOCK_REF_EX(prefix, suffix, overload_suffix, return_type, function_name, ...) \
    extern jg::detail::mock_aux<return_type, ##__VA_ARGS__> function_name ## overload_suffix ## _

#define JG_MOCK_REF(return_type, function_name, ...) \
    extern jg::detail::mock_aux<return_type, ##__VA_ARGS__> function_name ## _

/// @macro JG_STUB_REF
///
/// Same as JG_MOCK_REF_EX but for "stubbed out" void functions.
#define JG_STUB_REF_EX(prefix, suffix, overload_suffix, return_type, function_name, ...) \
    extern jg::detail::mock_aux<return_type, ##__VA_ARGS__> function_name ## overload_suffix ## _

#define JG_STUB_REF(return_type, function_name, ...) \
    extern jg::detail::mock_aux<return_type, ##__VA_ARGS__> function_name ## _

#ifdef JG_MOCK_ENABLE_SHORT_NAMES
	//
    #define MOCK_EX     JG_MOCK_EX
    #define MOCK        JG_MOCK
    #define STUB_EX     JG_STUB_EX
    #define STUB        JG_STUB
	//
    #define MOCK_COM_EX JG_MOCK_COM_EX
    #define MOCK_COM    JG_MOCK_COM
    #define STUB_COM_EX JG_STUB_COM_EX
    #define STUB_COM    JG_STUB_COM
	//
    #define MOCK_REF_EX JG_MOCK_REF_EX
    #define MOCK_REF    JG_MOCK_REF
    #define STUB_REF_EX JG_STUB_REF_EX
    #define STUB_REF    JG_STUB_REF
#endif
