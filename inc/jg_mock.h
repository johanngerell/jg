#pragma once

#include <cassert>
#include <functional>
#include <string>
#include <mutex>
#include "jg_state_scope.h"
#include "jg_stacktrace.h"

//
// These mocking helper macros are defined and documented at the bottom of this file:
//
//   - MOCK
//   - MOCK_OVERLOAD
//
//   - MOCK_RESET
//   - MOCK_OVERLOAD_RESET
//
//   - MOCK_PROXY
//   - MOCK_PROXY_SUBJECT
//   - MOCK_PROXY_INIT
//

namespace jg
{
namespace detail 
{

template <typename T>
using base_t = std::remove_const_t<std::remove_reference_t<T>>; // C++20 has std::remove_cvref for this.

template <typename ...T>
using tuple_params_t = std::tuple<base_t<T>...>;

template<size_t N, typename... Params>
using nth_param_t = std::tuple_element_t<N, std::tuple<Params...>>;

template <size_t N, typename... Params>
using nth_param_base_t = base_t<nth_param_t<N, Params...>>;

template <size_t N, typename... Params>
nth_param_t<N, Params...> nth_param(Params&&... params)
{
    return std::get<N>(std::forward_as_tuple(params...));
}

// The "mock info" for a mocked function that takes N parameters has `params1(), ..., paramsN()` members
// holding the actual N parameters, for usage in tests.
template <size_t N, typename ...Params>
struct mock_info_parameters;

// The "mock info" for a mocked function that takes no parameters has no parameter-holding member.
template <>
struct mock_info_parameters<0>
{
};

template <typename ...Params>
struct mock_info_parameters_base
{
protected:
    template <typename, typename>
    friend struct mock_impl;

    template <typename ...Params2>
    void set_params(Params2&&... params) { m_params = std::make_tuple(params...); }

    tuple_params_t<Params...> m_params;
};

template <typename T1>
struct mock_info_parameters<1, T1> : mock_info_parameters_base<T1>
{
    T1 param1() const { return std::get<0>(m_params); }
};

template <typename T1, typename T2>
struct mock_info_parameters<2, T1, T2> : mock_info_parameters_base<T1, T2>
{
    T1 param1() const { return std::get<0>(m_params); }
    T2 param2() const { return std::get<1>(m_params); }
};

template <typename T1, typename T2, typename T3>
struct mock_info_parameters<3, T1, T2, T3> : mock_info_parameters_base<T1, T2, T3>
{
    T1 param1() const { return std::get<0>(m_params); }
    T2 param2() const { return std::get<1>(m_params); }
    T3 param3() const { return std::get<2>(m_params); }
};

template <typename T1, typename T2, typename T3, typename T4>
struct mock_info_parameters<4, T1, T2, T3, T4> : mock_info_parameters_base<T1, T2, T3, T4>
{
    T1 param1() const { return std::get<0>(m_params); }
    T2 param2() const { return std::get<1>(m_params); }
    T3 param3() const { return std::get<2>(m_params); }
    T4 param4() const { return std::get<3>(m_params); }
};

template <typename T1, typename T2, typename T3, typename T4, typename T5>
struct mock_info_parameters<5, T1, T2, T3, T4, T5> : mock_info_parameters_base<T1, T2, T3, T4, T5>
{
    T1 param1() const { return std::get<0>(m_params); }
    T2 param2() const { return std::get<1>(m_params); }
    T3 param3() const { return std::get<2>(m_params); }
    T4 param4() const { return std::get<3>(m_params); }
    T5 param5() const { return std::get<4>(m_params); }
};

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
struct mock_info_parameters<6, T1, T2, T3, T4, T5, T6> : mock_info_parameters_base<T1, T2, T3, T4, T5, T6>
{
    T1 param1() const { return std::get<0>(m_params); }
    T2 param2() const { return std::get<1>(m_params); }
    T3 param3() const { return std::get<2>(m_params); }
    T4 param4() const { return std::get<3>(m_params); }
    T5 param5() const { return std::get<4>(m_params); }
    T6 param6() const { return std::get<5>(m_params); }
};

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
struct mock_info_parameters<7, T1, T2, T3, T4, T5, T6, T7> : mock_info_parameters_base<T1, T2, T3, T4, T5, T6, T7>
{
    T1 param1() const { return std::get<0>(m_params); }
    T2 param2() const { return std::get<1>(m_params); }
    T3 param3() const { return std::get<2>(m_params); }
    T4 param4() const { return std::get<3>(m_params); }
    T5 param5() const { return std::get<4>(m_params); }
    T6 param6() const { return std::get<5>(m_params); }
    T7 param7() const { return std::get<6>(m_params); }
};

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct mock_info_parameters<8, T1, T2, T3, T4, T5, T6, T7, T8> : mock_info_parameters_base<T1, T2, T3, T4, T5, T6, T7, T8>
{
    T1 param1() const { return std::get<0>(m_params); }
    T2 param2() const { return std::get<1>(m_params); }
    T3 param3() const { return std::get<2>(m_params); }
    T4 param4() const { return std::get<3>(m_params); }
    T5 param5() const { return std::get<4>(m_params); }
    T6 param6() const { return std::get<5>(m_params); }
    T7 param7() const { return std::get<6>(m_params); }
    T8 param8() const { return std::get<7>(m_params); }
};

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
struct mock_info_parameters<9, T1, T2, T3, T4, T5, T6, T7, T8, T9> : mock_info_parameters_base<T1, T2, T3, T4, T5, T6, T7, T8, T9>
{
    T1 param1() const { return std::get<0>(m_params); }
    T2 param2() const { return std::get<1>(m_params); }
    T3 param3() const { return std::get<2>(m_params); }
    T4 param4() const { return std::get<3>(m_params); }
    T5 param5() const { return std::get<4>(m_params); }
    T6 param6() const { return std::get<5>(m_params); }
    T7 param7() const { return std::get<6>(m_params); }
    T8 param8() const { return std::get<7>(m_params); }
    T9 param9() const { return std::get<8>(m_params); }
};

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10>
struct mock_info_parameters<10, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10> : mock_info_parameters_base<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10>
{
    T1 param1() const { return std::get<0>(m_params); }
    T2 param2() const { return std::get<1>(m_params); }
    T3 param3() const { return std::get<2>(m_params); }
    T4 param4() const { return std::get<3>(m_params); }
    T5 param5() const { return std::get<4>(m_params); }
    T6 param6() const { return std::get<5>(m_params); }
    T7 param7() const { return std::get<6>(m_params); }
    T8 param8() const { return std::get<7>(m_params); }
    T9 param9() const { return std::get<8>(m_params); }
    T10 param10() const { return std::get<9>(m_params); }
};

#ifdef NDEBUG
inline void debug_check(bool) {}
template <typename T>
inline T* debug_check_ptr(T* ptr) { return ptr; }
#else
/// In release builds, this is a no-op. In debug builds, this asserts that `value` is `true`,
/// and a stack trace is output if it isn't.
inline void debug_check(bool value)
{
    if (!value)
        for (const auto& stack_frame : jg::stack_trace().include_frame_count(10).skip_frame_count(1).capture())
            std::cout << stack_frame << "\n";

    assert(value);
}
/// In release builds, this is a no-op. In debug builds, this asserts that `ptr` isn't `nullptr`,
/// and a stack trace is output if it is.
template <typename T>
inline T* debug_check_ptr(T* ptr)
{
    debug_check(ptr);
    return ptr;
}
#endif
    
/// Asserts that the wrapped value is set before it's being referenced.
/// @note The check is only done when the preprocessor macro NDEBUG is defined
template <typename T>
class checked_value final
{
public:
    checked_value& operator=(const T& other)
    {
        value = other;
        assigned = true;
        return *this;
    }

    checked_value& operator=(T&& other)
    {
        value = std::move(other);
        assigned = true;
        return *this;
    }

    operator T()
    {
        debug_check(assigned);
        return value;
    }

private:
    base_t<T> value = base_t<T>();
    bool assigned = false;
};

template <typename T, typename Enable = void>
struct mock_info_return;

// The "mock info" for a mocked function that returns non-`void` has a `result` member
// that can be set in tests, as a quicker way of just returning a desired value from a
// mocked dependency. The other way is to assign a lambda to the `func` member, but if
// just a return value needs to be modeled, then `result` is the easier way.
template <typename T>
struct mock_info_return<T, std::enable_if_t<!std::is_same<T, void>::value>>
{
    // Checked, to make sure that a test doesn't use the result without first setting it.
    checked_value<T> result;
};

// The "mock info" for a mocked function that returns `void` has no `result` member, and
// the mock implementation for it can only be controlled by assigning a lambda to the
// `func` member.
template <typename T>
struct mock_info_return<T, std::enable_if_t<std::is_same<T, void>::value>>
{
};

template <typename T, typename ...Params>
struct mock_info final : mock_info_return<T>, mock_info_parameters<sizeof...(Params), Params...>
{
    mock_info(std::string prototype)
        : m_count(0)
        , m_prototype(std::move(prototype))
    {}

    std::function<T(Params...)> func;
    size_t                      count() const { return m_count; }
    bool                        called() const { return m_count > 0; }
    std::string                 prototype() const { return m_prototype; }

private:
    template <typename, typename>
    friend struct mock_impl;
    
    size_t m_count;
    std::string m_prototype;
};

template <typename T, typename TImpl, typename Enable = void>
struct mock_impl_base;

// The "mock implementation" for a mocked function that returns `void` only calls `func` for the
// "mock info" if it's set in the test. Nothing is done if it's not set.
template <typename T, typename TImpl>
struct mock_impl_base<T, TImpl, std::enable_if_t<std::is_same<T, void>::value>>
{
    template <typename... Params>
    void impl(Params&&... params)
    {        
        auto& info = static_cast<TImpl*>(this)->info;

        if (info.func)
            info.func(std::forward<Params>(params)...);
    }
};

// The "mock implementation" for a mocked function that returns non-`void` calls the `func` member
// for the "mock info" if it's set in the test. If the `func` member isn't set, then the `result`
// member is used instead. If the `result` member isn't set, then an assertion failure is triggered
// and a stack trace is output in debug builds and the default value for the `result` member type is
// returned in release builds. 
template <typename T, typename TImpl>
struct mock_impl_base<T, TImpl, std::enable_if_t<!std::is_same<T, void>::value>>
{
    template <typename... Params>
    T impl(Params&&... params)
    {
        auto& info = static_cast<TImpl*>(this)->info;
        
        if (info.func)
            return info.func(std::forward<Params>(params)...);
        
        return info.result;
    }
};

// The generic part (same for both `void` and non-`void` functions) of the "mock implementation"
// for a mocked function just makes sure that the "mock info" state (call counter, etc.) is
// updated when the function returns.
template <typename T, typename TMockInfo>
struct mock_impl final : mock_impl_base<T, mock_impl<T, TMockInfo>>
{
    TMockInfo& info;

    mock_impl(const TMockInfo& info)
        : info(const_cast<TMockInfo&>(info)) // Minor hack to be able to use the same MOCK macro for
                                             // both member functions and free functions. `mutable`
                                             // would otherwise be needed for some member functions,
                                             // and that would require separate macro implementations.
                                             // It's a "minor" hack because it's an implementation
                                             // detail where we already know that the original instance
                                             // is non-const.
    {}

    ~mock_impl()
    {
        info.m_count++;
    }

    template <typename... Params>
    T impl(Params&&... params)
    {
        info.set_params(std::forward<Params>(params)...);
        return mock_impl_base::impl(std::forward<Params>(params)...);
    }

    T impl()
    {
        return mock_impl_base::impl();
    }
};

} // namespace detail
} // namespace jg

#define _MOCK_PREAMBLE(prefix, suffix, ret, function_name, function_name_suffix, overload_suffix, body_extra, ...) \
    jg::detail::mock_info<ret, __VA_ARGS__> function_name ## overload_suffix ## _  {#ret " " #function_name "(" #__VA_ARGS__ ") " #suffix}

// The variadic macro parameter count macros below are derived from these links:
//
// https://stackoverflow.com/questions/5530505/why-does-this-variadic-argument-count-macro-fail-with-vc
// https://stackoverflow.com/questions/26682812/argument-counting-macro-with-zero-arguments-for-visualstudio
// https://stackoverflow.com/questions/9183993/msvc-variadic-macro-expansion?rq=1

#define _JG_CONCAT2(x, y) x ## y
#define _JG_CONCAT(x, y) _JG_CONCAT2(x, y)
#define _JG_EXPAND(x) x
#define _JG_GLUE(x, y) x y
#define _JG_VA_COUNT_2(x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,N,...) N
//
// Clang and GCC handles empty __VA_ARGS__ differently from MSVC.
//
#ifdef _MSC_VER
#define _JG_VA_COUNT_1(...) _JG_EXPAND(_JG_VA_COUNT_2(__VA_ARGS__,10,9,8,7,6,5,4,3,2,1,0))
#define _JG_AUGMENTER(...) unused, __VA_ARGS__
#define _JG_VA_COUNT(...) _JG_VA_COUNT_1(_JG_AUGMENTER(__VA_ARGS__))
#else
#define _JG_VA_COUNT_1(...) _JG_EXPAND(_JG_VA_COUNT_2(0, ## __VA_ARGS__,10,9,8,7,6,5,4,3,2,1,0))
#define _JG_VA_COUNT(...) _JG_VA_COUNT_1(__VA_ARGS__)
#endif

#define _MOCK_FUNCTION_PARAMS_DECL_FIRST_0()
#define _MOCK_FUNCTION_PARAMS_DECL_FIRST_1(T1) T1
#define _MOCK_FUNCTION_PARAMS_DECL_FIRST_2(T1, T2) T1, T2
#define _MOCK_FUNCTION_PARAMS_DECL_FIRST_3(T1, T2, T3) T1, T2, T3
#define _MOCK_FUNCTION_PARAMS_DECL_FIRST_4(T1, T2, T3, T4) T1, T2, T3, T4
#define _MOCK_FUNCTION_PARAMS_DECL_FIRST_5(T1, T2, T3, T4, T5) T1, T2, T3, T4, T5
#define _MOCK_FUNCTION_PARAMS_DECL_FIRST_6(T1, T2, T3, T4, T5, T6) T1, T2, T3, T4, T5, T6
#define _MOCK_FUNCTION_PARAMS_DECL_FIRST_7(T1, T2, T3, T4, T5, T6, T7) T1, T2, T3, T4, T5, T6, T7
#define _MOCK_FUNCTION_PARAMS_DECL_FIRST_8(T1, T2, T3, T4, T5, T6, T7, T8) T1, T2, T3, T4, T5, T6, T7, T8
#define _MOCK_FUNCTION_PARAMS_DECL_FIRST_9(T1, T2, T3, T4, T5, T6, T7, T8, T9) T1, T2, T3, T4, T5, T6, T7, T8, T9
#define _MOCK_FUNCTION_PARAMS_DECL_FIRST_10(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10) T1, T2, T3, T4, T5, T6, T7, T8, T9, T10

#define _MOCK_FUNCTION_PARAMS_DECL_BOTH_0()
#define _MOCK_FUNCTION_PARAMS_DECL_BOTH_1(T1) T1 p1
#define _MOCK_FUNCTION_PARAMS_DECL_BOTH_2(T1, T2) T1 p1, T2 p2
#define _MOCK_FUNCTION_PARAMS_DECL_BOTH_3(T1, T2, T3) T1 p1, T2 p2, T3 p3
#define _MOCK_FUNCTION_PARAMS_DECL_BOTH_4(T1, T2, T3, T4) T1 p1, T2 p2, T3 p3, T4 p4
#define _MOCK_FUNCTION_PARAMS_DECL_BOTH_5(T1, T2, T3, T4, T5) T1 p1, T2 p2, T3 p3, T4 p4, T5 p5
#define _MOCK_FUNCTION_PARAMS_DECL_BOTH_6(T1, T2, T3, T4, T5, T6) T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6
#define _MOCK_FUNCTION_PARAMS_DECL_BOTH_7(T1, T2, T3, T4, T5, T6, T7) T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7
#define _MOCK_FUNCTION_PARAMS_DECL_BOTH_8(T1, T2, T3, T4, T5, T6, T7, T8) T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8
#define _MOCK_FUNCTION_PARAMS_DECL_BOTH_9(T1, T2, T3, T4, T5, T6, T7, T8, T9) T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8, T9 p9
#define _MOCK_FUNCTION_PARAMS_DECL_BOTH_10(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10) T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8, T9 p9, T10 p10

#define _MOCK_FUNCTION_PARAMS_CALL_0()
#define _MOCK_FUNCTION_PARAMS_CALL_1(T1) p1
#define _MOCK_FUNCTION_PARAMS_CALL_2(T1, T2) p1, p2
#define _MOCK_FUNCTION_PARAMS_CALL_3(T1, T2, T3) p1, p2, p3
#define _MOCK_FUNCTION_PARAMS_CALL_4(T1, T2, T3, T4) p1, p2, p3, p4
#define _MOCK_FUNCTION_PARAMS_CALL_5(T1, T2, T3, T4, T5) p1, p2, p3, p4, p5
#define _MOCK_FUNCTION_PARAMS_CALL_6(T1, T2, T3, T4, T5, T6) p1, p2, p3, p4, p5, p6
#define _MOCK_FUNCTION_PARAMS_CALL_7(T1, T2, T3, T4, T5, T6, T7) p1, p2, p3, p4, p5, p6, p7
#define _MOCK_FUNCTION_PARAMS_CALL_8(T1, T2, T3, T4, T5, T6, T7, T8) p1, p2, p3, p4, p5, p6, p7, p8
#define _MOCK_FUNCTION_PARAMS_CALL_9(T1, T2, T3, T4, T5, T6, T7, T8, T9) p1, p2, p3, p4, p5, p6, p7, p8, p9
#define _MOCK_FUNCTION_PARAMS_CALL_10(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10) p1, p2, p3, p4, p5, p6, p7, p8, p9, p10

#define _MOCK_FUNCTION_PARAMS_DECL_FIRST(...) \
    _JG_GLUE(_JG_CONCAT(_MOCK_FUNCTION_PARAMS_DECL_FIRST_, _JG_VA_COUNT(__VA_ARGS__)), (__VA_ARGS__))

#define _MOCK_FUNCTION_PARAMS_DECL_BOTH(...) \
    _JG_GLUE(_JG_CONCAT(_MOCK_FUNCTION_PARAMS_DECL_BOTH_, _JG_VA_COUNT(__VA_ARGS__)), (__VA_ARGS__))

#define _MOCK_FUNCTION_PARAMS_CALL(...) \
    _JG_GLUE(_JG_CONCAT(_MOCK_FUNCTION_PARAMS_CALL_, _JG_VA_COUNT(__VA_ARGS__)), (__VA_ARGS__))

#define _MOCK_BODY(prefix, suffix, ret, function_name, function_name_suffix, overload_suffix, body_extra, ...) \
    return jg::detail::mock_impl<ret, decltype(function_name ## overload_suffix ## _)>(function_name ## overload_suffix ## _).impl(_MOCK_FUNCTION_PARAMS_CALL(__VA_ARGS__))

#define _MOCK_BODY_PROXY(prefix, suffix, ret, function_name, function_name_suffix, overload_suffix, body_extra, ...) \
    auto proxy_func = jg::detail::debug_check_ptr(function_name ## overload_suffix ## body_extra ## _func); \
    return proxy_func ? proxy_func(_MOCK_FUNCTION_PARAMS_CALL(__VA_ARGS__)) : ret()

#define _MOCK_FUNCTION(body_macro, prefix, suffix, ret, function_name, function_name_suffix, overload_suffix, body_extra, ...) \
    prefix ret function_name ## function_name_suffix(_MOCK_FUNCTION_PARAMS_DECL_BOTH(__VA_ARGS__)) suffix \
    { body_macro(prefix, suffix, ret, function_name, function_name_suffix, overload_suffix, body_extra, __VA_ARGS__); }

/// @macro MOCK_RESET
///
/// Resets the "mock info" state for the named function, if that's needed in a test.
/// Counters are set to zero, and return value, parameters, and the "lambda implementation"
/// are are cleared.
#define MOCK_RESET(function_name) \
    function_name ## _ = decltype(function_name ## _)(function_name ## _.prototype())

/// @macro MOCK_OVERLOAD_RESET
///
/// Resets the "mock info" state for the named overloaded function, if that's needed in a test.
#define MOCK_OVERLOAD_RESET(function_name, overload_suffix) \
    function_name ## overload_suffix ## _  = decltype(function_name ## overload_suffix ## _)(function_name ## overload_suffix ## _.prototype())

/// @macro MOCK
///
/// Defines a free function, or a virtual function override, with auxiliary data ("mock info")
/// meant to be used in unit testing.
///
/// Example scenario: Assume that this virtual function is in an abstract base class `user_names`:
///
///     virtual const char* find_by_id(int id) = 0;
///
/// We can mock that function in a test by deriving a "mock" class `mock_user_names` from `user_names`
/// and use the `MOCK` macro in its body:
///
///     MOCK(,, const char*, find_by_id, int);
///
/// That is functionally equivalent to writing
///
///     MOCK(virtual, override, const char*, find_by_id, int);
///
/// but `virtual` and `override` aren't strictly mandatory to use when overriding virtual base class
/// functions, so they can be omitted from the MOCK declaration to make it easier to read. The two first
/// `MOCK` parameters `prefix` and `suffix` will simply be pasted before and after the function
/// declaration by the macro.
///
/// The mock class can then be instantiated in a test and passed to a type or a function under test
/// that depends on `user_names` for its functionality. By doing this, we can manipulate how that tested
/// entity behaves in regard to how it uses its `user_names` dependency. The `MOCK` macro sets up some
/// tools to help with that task in the test.
///
/// So, the two purposes of the `MOCK` macro are 1) to define an implementation for either a free
/// function or a base class virtual function, and 2) to set up an auxiliary data structure that can be used
/// to control the function implementation, and to examine it after the call to see what actually happened
/// during the call.
///
/// The function implementation can be manipulated in the test by either completely intercepting any call to
/// it using a lambda, or by just setting a return value that the macro-generated implementation will use
/// when called by the tested entity. The after-the-call examination includes checks for whether the
/// function was called at all, how many times it was called, and with what parameters it was called.
///
/// @example Typical usage
///     
///     TEST("tested entity can do its job")
///     {
///         // The mocked class instance.
///         mock_user_names names;
///     
///         // Whenever user_names::find_by_id is called by the tested entity, it always returns "Donald Duck".
///         names.find_by_id_.result = "Donald Duck";
///     
///         // Depends on user_names
///         some_tested_entity tested_entity(user_names);
///     
///         TEST_ASSERT(tested_entity.can_do_its_job());    // Allegedly uses user_names::find_by_id
///         TEST_ASSERT(names.find_by_id_.called());        // Did the tested entity even call it?
///         TEST_ASSERT(names.find_by_id_.param1() < 4711); // Did the tested entity pass it a valid id?
///     }
///
/// @example More complex usage. The `func` "mock info" member can be used instead of `result`
///
///     TEST("some_tested_entity can do its job")
///     {
///         // The mocked class instance.
///         mock_user_names names;
///     
///         // Make the mock fail twice.
///         names.find_by_id_.func = [&] (int /* id */)
///         {
///             return names.find_by_id_.count < 2 ? nullptr : "Donald Duck";
///         };
///     
///         // Depends on user_names
///         some_tested_entity tested_entity(user_names);
///     
///         TEST_ASSERT(tested_entity.can_do_its_job());      // Allegedly uses user_names::find_by_id
///         TEST_ASSERT(user_names.find_by_id_.count() == 3); // Failed twice, succeeded once
///     }
///
/// The "mock info" members available for a mocked function `foo` that returns void and takes 0 parameters are:
///
///     std::function<void()>            foo_.func;        // can be set in a test
///     ---------------------------------------------------------------------------
///     bool                             foo_.called();    // set by the mocking framework
///     size_t                           foo_.count();     // set by the mocking framework
///     std::string                      foo_.prototype(); // set by the mocking framework
///
/// The "mock info" members available for a mocked function `foo` that returns `T` and takes 0 parameters are:
///
///     std::function<T()>               foo_.func;        // can be set in a test
///     T                                foo_.result;      // can be set in a test
///     ---------------------------------------------------------------------------
///     bool                             foo_.called();    // set by the mocking framework
///     size_t                           foo_.count();     // set by the mocking framework
///     std::string                      foo_.prototype(); // set by the mocking framework
///
/// The "mock info" members available for a mocked function `foo` that returns void and takes N parameters of types T1..TN:
///
///     std::function<void(T1, ..., TN)> foo_.func;        // can be set in a test
///     ---------------------------------------------------------------------------
///     bool                             foo_.called();    // set by the mocking framework
///     size_t                           foo_.count();     // set by the mocking framework
///     std::string                      foo_.prototype(); // set by the mocking framework
///     T1                               foo_.param1();    // set by the mocking framework
///     .                                .
///     .                                .
///     TN                               foo_.paramN()     // set by the mocking framework
///
/// The "mock info" members available for a mocked function `foo` that returns T and takes N parameters of types T1..TN:
///
///     std::function<void(T1, ..., TN)> foo_.func;        // can be set in a test
///     T                                foo_.result;      // can be set in a test
///     ---------------------------------------------------------------------------
///     bool                             foo_.called();    // set by the mocking framework
///     size_t                           foo_.count();     // set by the mocking framework
///     std::string                      foo_.prototype(); // set by the mocking framework
///     T1                               foo_.param1();    // set by the mocking framework
///     .                                .
///     .                                .
///     TN                               foo_.paramN()     // set by the mocking framework
///
/// @param prefix `static`, `virtual`, etc. Can be left empty if the mocked function supports it.
/// @param suffix `override`, `const`, `noexcept`, etc. Can be left empty if the mocked function supports it.
/// @param return_value The type of the return value, or `void`.
/// @param function_name The name of the mocked function.
/// @param variadic The variadic parameter list holds the parameter types of the mocked function, if any.
#define MOCK(prefix, suffix, return_value, function_name, ...) \
    _MOCK_PREAMBLE(prefix, suffix, return_value, function_name,,,, __VA_ARGS__); \
    _MOCK_FUNCTION(_MOCK_BODY, prefix, suffix, return_value, function_name,,,, __VA_ARGS__)

/// @macro MOCK_OVERLOAD
///
/// Defines an overload of a free function, or an overload of a virtual function override, with auxiliary
/// data ("mock info") meant to be used in unit testing.
///
/// Example scenario: Assume that these two virtual functions are in an abstract base class `user_names`:
///
///     virtual const char* find_by_id(int id) = 0;
///     virtual const char* find_by_id(int id, int type) = 0;
///
/// We can mock these functions in a test by deriving a "mock" class `mock_user_names` from `user_names`
/// and use the `MOCK` and `MOCK_OVERLOAD` macros in its body:
///
///     MOCK(,, const char*, find_by_id, int);
///     MOCK_OVERLOAD(,, const char*, find_by_id, 1, int, int);
///
/// The difference between the two lines is that the first line with `MOCK` will add a "mock info" member
/// `find_by_id_`, while the second line with `MOCK_OVERLOAD` will add a "mock info" member `find_by_id_1`,
/// since we specified '1' for the mock parameter `overload_suffix`.
///
/// In a test, the first mocked function can be controlled with the `find_by_id_` "mock info" and the second
/// mocked function can be controlled with the `find_by_id_1` "mock info".
///
/// Apart from the above, the "mock info" generated by `MOCK_OVERLOAD` is identical in functionality to the
/// one from `MOCK_INFO`.
///
/// @param prefix `static`, `virtual`, etc. Can be left empty if the mocked function supports it.
/// @param suffix `override`, `const`, `noexcept`, etc. Can be left empty if the mocked function supports it.
/// @param return_value The type of the return value, or `void`.
/// @param function_name The name of the mocked function.
/// @param overload_suffix Suffix to add to the "mock info" variable name for an overloaded mocked function.
/// @param variadic The variadic parameter list holds the parameter types of the mocked function, if any.
#define MOCK_OVERLOAD(prefix, suffix, return_value, function_name, overload_suffix, ...) \
    _MOCK_PREAMBLE(prefix, suffix, return_value, function_name,, overload_suffix,, __VA_ARGS__); \
    _MOCK_FUNCTION(_MOCK_BODY, prefix, suffix, return_value, function_name,, overload_suffix,, __VA_ARGS__)

/// @macro MOCK_PROXY
///
/// Defines a free function that *needs* to be controlled or mocked in unit tests, and auxiliary data that
/// makes it possible to forward all calls to said function (the "proxy") to another mocked function (the
/// "subject") that *is* controlled in unit tests.
///
/// Example scenario: Assume that this free function from the global namespace (can also be declared in a
/// named namespace) is called from some tested entity:
///
///     const char* find_by_id(int id) noexcept;
///
/// The simplest way we can mock that function in a unit test is by using the `MOCK` macro. (If the function
/// is declared in a named namespace, then `MOCK` must be in the same namespace):
///
///     MOCK(, noexcept, const char*, find_by_id, int);
///
/// Since a function can only be defined once, if that function needs to be mocked in more than one unit
/// test translation unit, then the `MOCK` macro is insufficient. Each usage of it will add a definition
/// of the function, which by the standard is Undefined Behavior. This is when using the `MOCK_PROXY` macro
/// family is required.
///
/// Instead of using `MOCK` in multiple unit test translation unit and getting multiple defined functions,
/// the macro `MOCK_PROXY` is used in *one* translation unit (preferrably one without any unit tests) and
/// `MOCK_PROXY_SUBJECT` is used in *all* unit test translation units. By combining that with
/// `MOCK_PROXY_INIT` in tests where the free function is called, we get thread safe mocked access to the
/// same auxiliary unit test data as the `MOCK` macro gives.
///
/// The "proxying" works like this, a bit simplified:
///
///   1. `MOCK_PROXY` defines the function we need to mock (the "proxy"), plus a pointer to a function with
///      the same signature (the "subject"). If the function pointer points at a subject function when the
///      proxy function is called, then the call is forwarded to the subject function.
///   2. `MOCK_PROXY_SUBJECT` works exactly as the `MOCK` macro, in that it defines a mocked function and
///      auxiliary data that can be used in unit tests to query and control the function behavior. The
///      mocked function is just altered a bit to be callable as a subject function by the proxy function.
///   3. `MOCK_PROXY_INIT` will, in the scope it's used, set the subject function pointer from `MOCK_PROXY`
///      to point at the mocked function from `MOCK_PROXY_SUBJECT`, so that all calls to the original proxy
///      function are forwarded to the subject function, which we can control and query in unit tests via its
///      auxiliary mock data.
///
/// @see MOCK, MOCK_PROXY_SUBJECT, MOCK_PROXY_INIT
///
/// @example Typical usage
///
///     -------------
///     no_tests.cpp:
///     -------------
///
///     // The free function that gets called in the tested entities in test1.cpp and test2.cpp.
///     MOCK_PROXY(, noexcept, const char*, find_by_id, int);
///
///     -----------
///     tests1.cpp:
///     -----------
///
///     // The mocked subject function that gets called by `find_by_id` in no_tests.cpp.
///     MOCK_PROXY_SUBJECT(, noexcept, const char*, find_by_id, int);
///
///     TEST("tested entity can do its job")
///     {
///         // Connects the `find_by_id` proxy function in no_tests.cpp to the mocked subject function in
///         // tests1.cpp. Calling `find_by_id` concurrently in tests2.cpp on another thread and using its
///         // auxiliary mock data for controlling and querying it is safe in the same scope as
///         // `MOCK_PROXY_INIT` and after its usage there.
///         MOCK_PROXY_INIT(find_by_id);
///     
///         // Whenever `find_by_id` is called by the tested entity, it always returns "Donald Duck".
///         find_by_id_.result = "Donald Duck";
///     
///         // Depends on `find_by_id`.
///         some_tested_entity tested_entity;
///     
///         TEST_ASSERT(tested_entity.can_do_its_job()); // Did it work?
///         TEST_ASSERT(find_by_id_.called());           // Did the tested entity even call it?
///         TEST_ASSERT(find_by_id_.param1() < 4711);    // Did the tested entity pass it a valid id?
///     }
///
///     -----------
///     tests2.cpp:
///     -----------
///
///     // The mocked subject function that gets called by `find_by_id` in no_tests.cpp.
///     MOCK_PROXY_SUBJECT(, noexcept, const char*, find_by_id, int);
///
///     TEST("tested entity can't do its job")
///     {
///         // Connects the `find_by_id` proxy function in no_tests.cpp to the mocked subject function in
///         // tests2.cpp. Calling `find_by_id` concurrently in tests1.cpp on another thread and using its
///         // auxiliary mock data for controlling and querying it is safe in the same scope as
///         // `MOCK_PROXY_INIT` and after its usage there.
///         MOCK_PROXY_INIT(find_by_id);
///     
///         // Whenever `find_by_id` is called by the tested entity, it always returns `nullptr`.
///         find_by_id_.result = nullptr;
///     
///         // Depends on `find_by_id`.
///         some_tested_entity tested_entity;
///     
///         TEST_ASSERT(!tested_entity.can_do_its_job()); // Did it fail?
///         TEST_ASSERT(find_by_id_.called());            // Did the tested entity even call it?
///         TEST_ASSERT(find_by_id_.param1() < 4711);     // Did the tested entity pass it a valid id?
///     }
///
/// @param prefix `static`, etc. Can be left empty if the mocked function supports it.
/// @param suffix `noexcept`, etc. Can be left empty if the mocked function supports it.
/// @param return_value The type of the return value, or `void`.
/// @param function_name The name of the mocked function.
/// @param variadic The variadic parameter list holds the parameter types of the mocked function, if any.
#define MOCK_PROXY(prefix, suffix, return_value, function_name, ...) \
    typedef prefix return_value (*function_name ## _proxy_func_t)(__VA_ARGS__) suffix; \
    function_name ## _proxy_func_t function_name ## _proxy_func = nullptr; \
    std::mutex function_name ## _mutex; \
    _MOCK_FUNCTION(_MOCK_BODY_PROXY, prefix, suffix, return_value, function_name,,, _proxy, __VA_ARGS__)

/// @macro MOCK_PROXY_OVERLOAD
///
/// Defines an overload of a free or static member function, with auxiliary
/// data ("mock info") meant to be used in unit testing.
///
/// @see MOCK_PROXY, MOCK_OVERLOAD
#define MOCK_PROXY_OVERLOAD(prefix, suffix, return_value, function_name, overload_suffix, ...) \
    typedef prefix return_value (*function_name ## overload_suffix ## _proxy_func_t)(__VA_ARGS__) suffix; \
    function_name ## overload_suffix ## _proxy_func_t function_name ## overload_suffix ## _proxy_func = nullptr; \
    std::mutex function_name ## overload_suffix ## _mutex; \
    _MOCK_FUNCTION(_MOCK_BODY_PROXY, prefix, suffix, return_value, function_name,, overload_suffix, _proxy, __VA_ARGS__)

/// @macro MOCK_PROXY_SUBJECT
///
/// Works exactly like `MOCK`, with the difference that `MOCK_PROXY_SUBJECT` defines a mocked subject
/// function to be used with a proxy function from `MOCK_PROXY`.
///
/// @see MOCK, MOCK_PROXY, MOCK_PROXY_INIT
///
/// @param prefix Set to the same as for the matching MOCK_PROXY.
/// @param suffix Set to the same as for the matching MOCK_PROXY.
/// @param return_value Set to the same as for the matching MOCK_PROXY.
/// @param function_name Set to the same as for the matching MOCK_PROXY.
/// @param variadic Set to the same as for the matching MOCK_PROXY.
#define MOCK_PROXY_SUBJECT(prefix, suffix, return_value, function_name, ...) \
    typedef prefix return_value (*function_name ## _proxy_func_t)(__VA_ARGS__) suffix; \
    extern function_name ## _proxy_func_t function_name ## _proxy_func; \
    extern std::mutex function_name ## _mutex; \
    namespace { _MOCK_PREAMBLE(prefix, suffix, return_value, function_name,,,, __VA_ARGS__); \
    _MOCK_FUNCTION(_MOCK_BODY, prefix, suffix, return_value, function_name, _proxy,,, __VA_ARGS__); }

/// @macro MOCK_PROXY_OVERLOAD_SUBJECT
///
/// @see MOCK_PROXY, MOCK_PROXY_OVERLOAD, MOCK_OVERLOAD
#define MOCK_PROXY_OVERLOAD_SUBJECT(prefix, suffix, return_value, function_name, overload_suffix, ...) \
    typedef prefix return_value (*function_name ## overload_suffix ## _proxy_func_t)(__VA_ARGS__) suffix; \
    extern function_name ## overload_suffix ## _proxy_func_t function_name ## overload_suffix ## _proxy_func; \
    extern std::mutex function_name ## overload_suffix ## _mutex; \
    namespace { _MOCK_PREAMBLE(prefix, suffix, return_value, function_name,, overload_suffix,, __VA_ARGS__); \
    _MOCK_FUNCTION(_MOCK_BODY, prefix, suffix, return_value, function_name, overload_suffix ## _proxy, overload_suffix,, __VA_ARGS__) }

/// @macro MOCK_PROXY_INIT
///
/// Connects a mocked subject function with a proxy function with the same name and signature. The connection
/// between the two exists for the rest of the scope that `MOCK_PROXY_INIT` is used in. It is safe to call the
/// proxy function concurrently in different translation units and threads, and using the auxiliary data of its
/// mocked subjects for controlling and querying them. However, `MOCK_PROXY_INIT` usages cannot be nested.
///
/// This macro will effectively perform these actions:
///
///   1. Reset the state of the auxiliary data for the subject function.
///   2. Wait for and lock access to the proxy/subject connection for the rest of the scope.
///   3. Set up the connection so that the proxy function can be controlled and queried in unit tests,
///      via the auxiliary data of the subject function.
///   4. Remove the connection when leaving the scope.
///
/// @see MOCK, MOCK_PROXY, MOCK_PROXY_SUBJECT
#define MOCK_PROXY_INIT(function_name) \
    MOCK_RESET(function_name); \
    std::lock_guard<std::mutex> _JG_CONCAT(lock_, __LINE__)(function_name ## _mutex); \
    jg::state_scope_value _JG_CONCAT(scope_, __LINE__)(function_name ## _proxy_func, function_name ## _proxy, nullptr)

/// @macro MOCK_PROXY_OVERLOAD_INIT
///
/// @see MOCK_PROXY_INIT
#define MOCK_PROXY_OVERLOAD_INIT(function_name, overload_suffix) \
    MOCK_OVERLOAD_RESET(function_name, overload_suffix); \
    std::lock_guard<std::mutex> _JG_CONCAT(lock_, __LINE__)(function_name ## overload_suffix ## _mutex); \
    jg::state_scope_value _JG_CONCAT(scope_, __LINE__)(function_name ## overload_suffix ## _proxy_func, function_name ## overload_suffix ## _proxy, nullptr)
