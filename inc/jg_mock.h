#pragma once

#include <cassert>
#include <functional>
#include <string>
#ifdef JG_MOCK_ENABLE_PROXY_LOCK
#include <mutex>
#endif
#include "jg_state_scope.h"
#include "jg_stacktrace.h"

//
// These mocking macros are defined and documented at the bottom of this file:
//
//   - JG_MOCK
//   - JG_MOCK_RESET
//
//   - JG_MOCK_OVERLOAD
//   - JG_MOCK_OVERLOAD_RESET
//
//   - JG_MOCK_PROXY
//   - JG_MOCK_PROXY_SUBJECT
//   - JG_MOCK_PROXY_INIT
//
// Compilation flags
//
//   - JG_MOCK_ENABLE_SHORT_NAMES: Enables mocking macros named without the "JG_" prefix.
//   - JG_MOCK_ENABLE_PROXY_LOCK:  Enables locked access to a proxy and its auxiliary data in the same
//                                 scope as JG_MOCK_PROXY_INIT or JG_MOCK_PROXY_OVERLOAD_INIT.
//

//
// Note that some MSVC versions require /Zc:__cplusplus even if /std:c++14 or higher is specified
//
#if (__cplusplus < 201402L)
#error jg::mock needs C++14 or newer
#endif

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

// The "mock info" for a mocked function that takes N parameters has `param<1>(), ..., param<N>()` members
// holding the actual N parameters, for usage in tests.
template <size_t N, typename ...Params>
struct mock_info_parameters
{
    template <size_t Number>
    auto param() const { return std::get<Number - 1>(m_params); }

protected:
    template <typename, typename>
    friend struct mock_impl;

    template <typename ...Params2>
    void set_params(Params2&&... params) { m_params = std::make_tuple(params...); }

    tuple_params_t<Params...> m_params;
};

// The "mock info" for a mocked function that takes no parameters has no parameter-holding member.
template <>
struct mock_info_parameters<0>
{
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

template <typename T, typename Enable = void>
class checked_value;

/// Asserts that the wrapped value is set before it's being referenced.
/// @note The check is only done when the preprocessor macro NDEBUG is not defined
template <typename T>
class checked_value<T, std::enable_if_t<std::is_reference_v<T>>> final
{
public:
    checked_value& operator=(T other)
    {
        value = &other;
        assigned = true;
        return *this;
    }

    operator T()
    {
        debug_check(assigned);
        return *value;
    }

private:
    std::remove_reference_t<T>* value = nullptr;
    bool assigned = false;
};

/// Asserts that the wrapped value is set before it's being referenced.
/// @note The check is only done when the preprocessor macro NDEBUG is not defined
template <typename T>
class checked_value<T, std::enable_if_t<!std::is_reference_v<T>>> final
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
    T value{};
    bool assigned = false;
};

// The "mock info" for a mocked function that returns non-`void` has a `result` member
// that can be set in tests, as a quicker way of just returning a desired value from a
// mocked dependency. The other way is to assign a lambda to the `func` member, but if
// just a return value needs to be modeled, then `result` is the easier way.
template <typename T>
struct mock_info_return
{
    // Checked, to make sure that a test doesn't use the result without first setting it.
    checked_value<T> result;
};

// The "mock info" for a mocked function that returns `void` has no `result` member, and
// the mock implementation for it can only be controlled by assigning a lambda to the
// `func` member.
template <>
struct mock_info_return<void>
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
        : info(const_cast<TMockInfo&>(info)) // Minor hack to be able to use the same JG_MOCK macro for
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

#define _JG_CONCAT3(x, y, z) x ## y ## z
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

#define _JG_MOCK_PREAMBLE(prefix, suffix, ret, function_name, function_name_suffix, overload_suffix, body_extra, ...) \
    jg::detail::mock_info<ret, __VA_ARGS__> _JG_CONCAT3(function_name, overload_suffix, _) {#ret " " #function_name "(" #__VA_ARGS__ ") " #suffix}

// The variadic macro parameter count macros below are derived from these links:
//
// https://stackoverflow.com/questions/5530505/why-does-this-variadic-argument-count-macro-fail-with-vc
// https://stackoverflow.com/questions/26682812/argument-counting-macro-with-zero-arguments-for-visualstudio
// https://stackoverflow.com/questions/9183993/msvc-variadic-macro-expansion?rq=1

#define _JG_MOCK_FUNCTION_PARAMS_DECL_FIRST_0()
#define _JG_MOCK_FUNCTION_PARAMS_DECL_FIRST_1(T1) T1
#define _JG_MOCK_FUNCTION_PARAMS_DECL_FIRST_2(T1, T2) T1, T2
#define _JG_MOCK_FUNCTION_PARAMS_DECL_FIRST_3(T1, T2, T3) T1, T2, T3
#define _JG_MOCK_FUNCTION_PARAMS_DECL_FIRST_4(T1, T2, T3, T4) T1, T2, T3, T4
#define _JG_MOCK_FUNCTION_PARAMS_DECL_FIRST_5(T1, T2, T3, T4, T5) T1, T2, T3, T4, T5
#define _JG_MOCK_FUNCTION_PARAMS_DECL_FIRST_6(T1, T2, T3, T4, T5, T6) T1, T2, T3, T4, T5, T6
#define _JG_MOCK_FUNCTION_PARAMS_DECL_FIRST_7(T1, T2, T3, T4, T5, T6, T7) T1, T2, T3, T4, T5, T6, T7
#define _JG_MOCK_FUNCTION_PARAMS_DECL_FIRST_8(T1, T2, T3, T4, T5, T6, T7, T8) T1, T2, T3, T4, T5, T6, T7, T8
#define _JG_MOCK_FUNCTION_PARAMS_DECL_FIRST_9(T1, T2, T3, T4, T5, T6, T7, T8, T9) T1, T2, T3, T4, T5, T6, T7, T8, T9
#define _JG_MOCK_FUNCTION_PARAMS_DECL_FIRST_10(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10) T1, T2, T3, T4, T5, T6, T7, T8, T9, T10

#define _JG_MOCK_FUNCTION_PARAMS_DECL_BOTH_0()
#define _JG_MOCK_FUNCTION_PARAMS_DECL_BOTH_1(T1) T1 p1
#define _JG_MOCK_FUNCTION_PARAMS_DECL_BOTH_2(T1, T2) T1 p1, T2 p2
#define _JG_MOCK_FUNCTION_PARAMS_DECL_BOTH_3(T1, T2, T3) T1 p1, T2 p2, T3 p3
#define _JG_MOCK_FUNCTION_PARAMS_DECL_BOTH_4(T1, T2, T3, T4) T1 p1, T2 p2, T3 p3, T4 p4
#define _JG_MOCK_FUNCTION_PARAMS_DECL_BOTH_5(T1, T2, T3, T4, T5) T1 p1, T2 p2, T3 p3, T4 p4, T5 p5
#define _JG_MOCK_FUNCTION_PARAMS_DECL_BOTH_6(T1, T2, T3, T4, T5, T6) T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6
#define _JG_MOCK_FUNCTION_PARAMS_DECL_BOTH_7(T1, T2, T3, T4, T5, T6, T7) T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7
#define _JG_MOCK_FUNCTION_PARAMS_DECL_BOTH_8(T1, T2, T3, T4, T5, T6, T7, T8) T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8
#define _JG_MOCK_FUNCTION_PARAMS_DECL_BOTH_9(T1, T2, T3, T4, T5, T6, T7, T8, T9) T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8, T9 p9
#define _JG_MOCK_FUNCTION_PARAMS_DECL_BOTH_10(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10) T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8, T9 p9, T10 p10

#define _JG_MOCK_FUNCTION_PARAMS_CALL_0()
#define _JG_MOCK_FUNCTION_PARAMS_CALL_1(T1) p1
#define _JG_MOCK_FUNCTION_PARAMS_CALL_2(T1, T2) p1, p2
#define _JG_MOCK_FUNCTION_PARAMS_CALL_3(T1, T2, T3) p1, p2, p3
#define _JG_MOCK_FUNCTION_PARAMS_CALL_4(T1, T2, T3, T4) p1, p2, p3, p4
#define _JG_MOCK_FUNCTION_PARAMS_CALL_5(T1, T2, T3, T4, T5) p1, p2, p3, p4, p5
#define _JG_MOCK_FUNCTION_PARAMS_CALL_6(T1, T2, T3, T4, T5, T6) p1, p2, p3, p4, p5, p6
#define _JG_MOCK_FUNCTION_PARAMS_CALL_7(T1, T2, T3, T4, T5, T6, T7) p1, p2, p3, p4, p5, p6, p7
#define _JG_MOCK_FUNCTION_PARAMS_CALL_8(T1, T2, T3, T4, T5, T6, T7, T8) p1, p2, p3, p4, p5, p6, p7, p8
#define _JG_MOCK_FUNCTION_PARAMS_CALL_9(T1, T2, T3, T4, T5, T6, T7, T8, T9) p1, p2, p3, p4, p5, p6, p7, p8, p9
#define _JG_MOCK_FUNCTION_PARAMS_CALL_10(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10) p1, p2, p3, p4, p5, p6, p7, p8, p9, p10

#define _JG_MOCK_FUNCTION_PARAMS_DECL_FIRST(...) \
    _JG_GLUE(_JG_CONCAT(_JG_MOCK_FUNCTION_PARAMS_DECL_FIRST_, _JG_VA_COUNT(__VA_ARGS__)), (__VA_ARGS__))

#define _JG_MOCK_FUNCTION_PARAMS_DECL_BOTH(...) \
    _JG_GLUE(_JG_CONCAT(_JG_MOCK_FUNCTION_PARAMS_DECL_BOTH_, _JG_VA_COUNT(__VA_ARGS__)), (__VA_ARGS__))

#define _JG_MOCK_FUNCTION_PARAMS_CALL(...) \
    _JG_GLUE(_JG_CONCAT(_JG_MOCK_FUNCTION_PARAMS_CALL_, _JG_VA_COUNT(__VA_ARGS__)), (__VA_ARGS__))

#define _JG_MOCK_BODY(prefix, suffix, ret, function_name, function_name_suffix, overload_suffix, body_extra, ...) \
    return jg::detail::mock_impl<ret, decltype(_JG_CONCAT3(function_name, overload_suffix, _))>(_JG_CONCAT3(function_name, overload_suffix, _)).impl(_JG_MOCK_FUNCTION_PARAMS_CALL(__VA_ARGS__))

#define _JG_MOCK_BODY_PROXY(prefix, suffix, ret, function_name, function_name_suffix, overload_suffix, body_extra, ...) \
    auto proxy_func = jg::detail::debug_check_ptr(_JG_CONCAT3(function_name, overload_suffix, body_extra) ## _func); \
    return proxy_func ? proxy_func(_JG_MOCK_FUNCTION_PARAMS_CALL(__VA_ARGS__)) : ret()

#define _JG_MOCK_FUNCTION(body_macro, prefix, suffix, ret, function_name, function_name_suffix, overload_suffix, body_extra, ...) \
    prefix ret _JG_CONCAT3(function_name, function_name_suffix,)(_JG_MOCK_FUNCTION_PARAMS_DECL_BOTH(__VA_ARGS__)) suffix \
    { body_macro(prefix, suffix, ret, function_name, function_name_suffix, overload_suffix, body_extra, __VA_ARGS__); }

#ifdef JG_MOCK_ENABLE_PROXY_LOCK
#define _JG_MOCK_PROXY_LOCK_GUARD(function_name, overload_suffix) \
    std::lock_guard<std::mutex> _JG_CONCAT(lock_, __LINE__)(_JG_CONCAT3(function_name, overload_suffix, _mutex))
#define _JG_MOCK_PROXY_LOCK_DECLARE_MUTEX_VARIABLE(function_name, overload_suffix) \
    std::mutex _JG_CONCAT3(function_name, overload_suffix, _mutex)
#define _JG_MOCK_PROXY_LOCK_DECLARE_MUTEX_EXTERN_VARIABLE(function_name, overload_suffix) \
    extern std::mutex _JG_CONCAT3(function_name, overload_suffix, _mutex)
#else
#define _JG_MOCK_PROXY_LOCK_GUARD(function_name, overload_suffix)
#define _JG_MOCK_PROXY_LOCK_DECLARE_MUTEX_VARIABLE(function_name, overload_suffix)
#define _JG_MOCK_PROXY_LOCK_DECLARE_MUTEX_EXTERN_VARIABLE(function_name, overload_suffix)
#endif

// --------------------------- Implementation details go above this line ---------------------------

/// @macro JG_MOCK
///
/// Defines a free function, or a virtual function override, with auxiliary data ("mock info")
/// meant to be used in unit testing.
///
/// Example scenario: Assume that this virtual function is in an abstract base class `user_names`:
///
///     virtual const char* find_by_id(int id) = 0;
///
/// We can mock that function in a test by deriving a "mock" class `mock_user_names` from `user_names`
/// and use the `JG_MOCK` macro in its body:
///
///     JG_MOCK(,, const char*, find_by_id, int);
///
/// That is functionally equivalent to writing
///
///     JG_MOCK(virtual, override, const char*, find_by_id, int);
///
/// but `virtual` and `override` aren't strictly mandatory to use when overriding virtual base class
/// functions, so they can be omitted from the `JG_MOCK` declaration to make it easier to read. The two first
/// `JG_MOCK` parameters `prefix` and `suffix` will simply be pasted before and after the function
/// declaration by the macro.
///
/// The mock class can then be instantiated in a test and passed to a type or a function under test
/// that depends on `user_names` for its functionality. By doing this, we can manipulate how that tested
/// entity behaves in regard to how it uses its `user_names` dependency. The `JG_MOCK` macro sets up some
/// tools to help with that task in the test.
///
/// So, the two purposes of the `JG_MOCK` macro are 1) to define an implementation for either a free
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
///         TEST_ASSERT(tested_entity.can_do_its_job());      // Allegedly uses user_names::find_by_id
///         TEST_ASSERT(names.find_by_id_.called());          // Did the tested entity even call it?
///         TEST_ASSERT(names.find_by_id_.param<1>() < 4711); // Did the tested entity pass it a valid id?
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
///     T1                               foo_.param<1>();  // set by the mocking framework
///     .                                .
///     .                                .
///     TN                               foo_.param<N>()   // set by the mocking framework
///
/// The "mock info" members available for a mocked function `foo` that returns T and takes N parameters of types T1..TN:
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
/// @param prefix `static`, `virtual`, etc. Can be left empty if the mocked function supports it.
/// @param suffix `override`, `const`, `noexcept`, etc. Can be left empty if the mocked function supports it.
/// @param return_type The type of the return value, or `void`.
/// @param function_name The name of the mocked function.
/// @param variadic The variadic parameter list holds the parameter types of the mocked function, if any.
#define JG_MOCK(prefix, suffix, return_type, function_name, ...) \
    _JG_MOCK_PREAMBLE(prefix, suffix, return_type, function_name,,,, __VA_ARGS__); \
    _JG_MOCK_FUNCTION(_JG_MOCK_BODY, prefix, suffix, return_type, function_name,,,, __VA_ARGS__)

/// @macro JG_MOCK_RESET
///
/// Resets the "mock info" state for the named function, if that's needed in a test.
/// Counters are set to zero, and return value, parameters, and the "lambda implementation"
/// are are cleared.
#define JG_MOCK_RESET(function_name) \
    _JG_CONCAT2(function_name, _) = decltype(_JG_CONCAT2(function_name, _))(_JG_CONCAT2(function_name, _).prototype())

/// @macro JG_MOCK_OVERLOAD
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
/// and use the `JG_MOCK` and `JG_MOCK_OVERLOAD` macros in its body:
///
///     JG_MOCK(,, const char*, find_by_id, int);
///     JG_MOCK_OVERLOAD(,, const char*, find_by_id, 1, int, int);
///
/// The difference between the two lines is that the first line with `JG_MOCK` will add a "mock info" member
/// `find_by_id_`, while the second line with `JG_MOCK_OVERLOAD` will add a "mock info" member `find_by_id_1`,
/// since we specified '1' for the mock parameter `overload_suffix`.
///
/// In a test, the first mocked function can be controlled with the `find_by_id_` "mock info" and the second
/// mocked function can be controlled with the `find_by_id_1` "mock info".
///
/// Apart from the above, the "mock info" generated by `JG_MOCK_OVERLOAD` is identical in functionality to the
/// one from `MOCK_INFO`.
///
/// @param prefix `static`, `virtual`, etc. Can be left empty if the mocked function supports it.
/// @param suffix `override`, `const`, `noexcept`, etc. Can be left empty if the mocked function supports it.
/// @param return_type The type of the return value, or `void`.
/// @param function_name The name of the mocked function.
/// @param overload_suffix Suffix to add to the "mock info" variable name for an overloaded mocked function.
/// @param variadic The variadic parameter list holds the parameter types of the mocked function, if any.
#define JG_MOCK_OVERLOAD(prefix, suffix, return_type, function_name, overload_suffix, ...) \
    _JG_MOCK_PREAMBLE(prefix, suffix, return_type, function_name,, overload_suffix,, __VA_ARGS__); \
    _JG_MOCK_FUNCTION(_JG_MOCK_BODY, prefix, suffix, return_type, function_name,, overload_suffix,, __VA_ARGS__)

/// @macro JG_MOCK_OVERLOAD_RESET
///
/// Resets the "mock info" state for the named overloaded function, if that's needed in a test.
#define JG_MOCK_OVERLOAD_RESET(function_name, overload_suffix) \
    _JG_CONCAT3(function_name, overload_suffix, _) = decltype(_JG_CONCAT3(function_name, overload_suffix, _))(_JG_CONCAT3(function_name, overload_suffix, _).prototype())

/// @macro JG_MOCK_PROXY
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
/// The simplest way we can mock that function in a unit test is by using the `JG_MOCK` macro. (If the function
/// is declared in a named namespace, then `JG_MOCK` must be in the same namespace):
///
///     JG_MOCK(, noexcept, const char*, find_by_id, int);
///
/// Since a function can only be defined once, if that function needs to be mocked in more than one unit
/// test translation unit, then the `JG_MOCK` macro is insufficient. Each usage of it will add a definition
/// of the function, which by the standard is Undefined Behavior. This is when using the `JG_MOCK_PROXY` macro
/// family is required.
///
/// Instead of using `JG_MOCK` in multiple unit test translation unit and getting multiple defined functions,
/// the macro `JG_MOCK_PROXY` is used in *one* translation unit (preferrably one without any unit tests) and
/// `JG_MOCK_PROXY_SUBJECT` is used in *all* unit test translation units. By combining that with
/// `JG_MOCK_PROXY_INIT` in tests where the free function is called, we get thread safe mocked access to the
/// same auxiliary unit test data as the `JG_MOCK` macro gives.
///
/// The "proxying" works like this, a bit simplified:
///
///   1. `JG_MOCK_PROXY` defines the function we need to mock (the "proxy"), plus a pointer to a function with
///      the same signature (the "subject"). If the function pointer points at a subject function when the
///      proxy function is called, then the call is forwarded to the subject function.
///   2. `JG_MOCK_PROXY_SUBJECT` works exactly as the `JG_MOCK` macro, in that it defines a mocked function and
///      auxiliary data that can be used in unit tests to query and control the function behavior. The
///      mocked function is just altered a bit to be callable as a subject function by the proxy function.
///   3. `JG_MOCK_PROXY_INIT` will, in the scope it's used, set the subject function pointer from `JG_MOCK_PROXY`
///      to point at the mocked function from `JG_MOCK_PROXY_SUBJECT`, so that all calls to the original proxy
///      function are forwarded to the subject function, which we can control and query in unit tests via its
///      auxiliary mock data.
///
/// @see JG_MOCK, JG_MOCK_PROXY_SUBJECT, JG_MOCK_PROXY_INIT
///
/// @example Typical usage
///
///     -------------
///     no_tests.cpp:
///     -------------
///
///     // The free function that gets called in the tested entities in test1.cpp and test2.cpp.
///     JG_MOCK_PROXY(, noexcept, const char*, find_by_id, int);
///
///     -----------
///     tests1.cpp:
///     -----------
///
///     // The mocked subject function that gets called by `find_by_id` in no_tests.cpp.
///     JG_MOCK_PROXY_SUBJECT(, noexcept, const char*, find_by_id, int);
///
///     TEST("tested entity can do its job")
///     {
///         // Connects the `find_by_id` proxy function in no_tests.cpp to the mocked subject function in
///         // tests1.cpp. Calling `find_by_id` concurrently in tests2.cpp on another thread and using its
///         // auxiliary mock data for controlling and querying it is safe in the same scope as
///         // `JG_MOCK_PROXY_INIT` and after its usage there.
///         JG_MOCK_PROXY_INIT(find_by_id);
///     
///         // Whenever `find_by_id` is called by the tested entity, it always returns "Donald Duck".
///         find_by_id_.result = "Donald Duck";
///     
///         // Depends on `find_by_id`.
///         some_tested_entity tested_entity;
///     
///         TEST_ASSERT(tested_entity.can_do_its_job()); // Did it work?
///         TEST_ASSERT(find_by_id_.called());           // Did the tested entity even call it?
///         TEST_ASSERT(find_by_id_.param<1>() < 4711);  // Did the tested entity pass it a valid id?
///     }
///
///     -----------
///     tests2.cpp:
///     -----------
///
///     // The mocked subject function that gets called by `find_by_id` in no_tests.cpp.
///     JG_MOCK_PROXY_SUBJECT(, noexcept, const char*, find_by_id, int);
///
///     TEST("tested entity can't do its job")
///     {
///         // Connects the `find_by_id` proxy function in no_tests.cpp to the mocked subject function in
///         // tests2.cpp. Calling `find_by_id` concurrently in tests1.cpp on another thread and using its
///         // auxiliary mock data for controlling and querying it is safe in the same scope as
///         // `JG_MOCK_PROXY_INIT` and after its usage there.
///         JG_MOCK_PROXY_INIT(find_by_id);
///     
///         // Whenever `find_by_id` is called by the tested entity, it always returns `nullptr`.
///         find_by_id_.result = nullptr;
///     
///         // Depends on `find_by_id`.
///         some_tested_entity tested_entity;
///     
///         TEST_ASSERT(!tested_entity.can_do_its_job()); // Did it fail?
///         TEST_ASSERT(find_by_id_.called());            // Did the tested entity even call it?
///         TEST_ASSERT(find_by_id_.param<1>() < 4711);   // Did the tested entity pass it a valid id?
///     }
///
/// @param prefix `static`, etc. Can be left empty if the mocked function supports it.
/// @param suffix `noexcept`, etc. Can be left empty if the mocked function supports it.
/// @param return_type The type of the return value, or `void`.
/// @param function_name The name of the mocked function.
/// @param variadic The variadic parameter list holds the parameter types of the mocked function, if any.
#define JG_MOCK_PROXY(prefix, suffix, return_type, function_name, ...) \
    typedef prefix return_type (*_JG_CONCAT2(function_name, _proxy_func_t))(__VA_ARGS__) suffix; \
    _JG_CONCAT2(function_name, _proxy_func_t) _JG_CONCAT2(function_name, _proxy_func) = nullptr; \
    _JG_MOCK_PROXY_LOCK_DECLARE_MUTEX_VARIABLE(function_name,); \
    _JG_MOCK_FUNCTION(_JG_MOCK_BODY_PROXY, prefix, suffix, return_type, function_name,,, _proxy, __VA_ARGS__)

/// @macro JG_MOCK_PROXY_OVERLOAD
///
/// Defines an overload of a free or static member function, with auxiliary
/// data ("mock info") meant to be used in unit testing.
///
/// @see JG_MOCK_PROXY, JG_MOCK_OVERLOAD
#define JG_MOCK_PROXY_OVERLOAD(prefix, suffix, return_type, function_name, overload_suffix, ...) \
    typedef prefix return_type (*_JG_CONCAT3(function_name, overload_suffix, _proxy_func_t))(__VA_ARGS__) suffix; \
    _JG_CONCAT3(function_name, overload_suffix, _proxy_func_t) _JG_CONCAT3(function_name, overload_suffix, _proxy_func) = nullptr; \
    _JG_MOCK_PROXY_LOCK_DECLARE_MUTEX_VARIABLE(function_name, overload_suffix); \
    _JG_MOCK_FUNCTION(_JG_MOCK_BODY_PROXY, prefix, suffix, return_type, function_name,, overload_suffix, _proxy, __VA_ARGS__)

/// @macro JG_MOCK_PROXY_SUBJECT
///
/// Works exactly like `JG_MOCK`, with the difference that `JG_MOCK_PROXY_SUBJECT` defines a mocked subject
/// function to be used with a proxy function from `JG_MOCK_PROXY`.
///
/// @see JG_MOCK, JG_MOCK_PROXY, JG_MOCK_PROXY_INIT
///
/// @param prefix Set to the same as for the matching JG_MOCK_PROXY.
/// @param suffix Set to the same as for the matching JG_MOCK_PROXY.
/// @param return_type Set to the same as for the matching JG_MOCK_PROXY.
/// @param function_name Set to the same as for the matching JG_MOCK_PROXY.
/// @param variadic Set to the same as for the matching JG_MOCK_PROXY.
#define JG_MOCK_PROXY_SUBJECT(prefix, suffix, return_type, function_name, ...) \
    typedef prefix return_type (*_JG_CONCAT2(function_name, _proxy_func_t))(__VA_ARGS__) suffix; \
    extern _JG_CONCAT2(function_name, _proxy_func_t) _JG_CONCAT2(function_name, _proxy_func); \
    _JG_MOCK_PROXY_LOCK_DECLARE_MUTEX_EXTERN_VARIABLE(function_name,); \
    namespace \
    { \
        _JG_MOCK_PREAMBLE(prefix, suffix, return_type, function_name,,,, __VA_ARGS__); \
        _JG_MOCK_FUNCTION(_JG_MOCK_BODY, prefix, suffix, return_type, function_name, _proxy,,, __VA_ARGS__); \
    }

/// @macro JG_MOCK_PROXY_OVERLOAD_SUBJECT
///
/// @see JG_MOCK_PROXY, JG_MOCK_PROXY_OVERLOAD, JG_MOCK_OVERLOAD
#define JG_MOCK_PROXY_OVERLOAD_SUBJECT(prefix, suffix, return_type, function_name, overload_suffix, ...) \
    typedef prefix return_type (*_JG_CONCAT3(function_name, overload_suffix, _proxy_func_t))(__VA_ARGS__) suffix; \
    extern _JG_CONCAT3(function_name, overload_suffix, _proxy_func_t) _JG_CONCAT3(function_name, overload_suffix, _proxy_func); \
    _JG_MOCK_PROXY_LOCK_DECLARE_MUTEX_EXTERN_VARIABLE(function_name, overload_suffix); \
    namespace \
    { \
        _JG_MOCK_PREAMBLE(prefix, suffix, return_type, function_name,, overload_suffix,, __VA_ARGS__); \
        _JG_MOCK_FUNCTION(_JG_MOCK_BODY, prefix, suffix, return_type, function_name, overload_suffix ## _proxy, overload_suffix,, __VA_ARGS__) \
    }

/// @macro JG_MOCK_PROXY_INIT
///
/// Connects a mocked subject function with a proxy function with the same name and signature. The connection
/// between the two exists for the rest of the scope that `JG_MOCK_PROXY_INIT` is used in. It is safe to call the
/// proxy function concurrently in different translation units and threads, and using the auxiliary data of its
/// mocked subjects for controlling and querying them. However, `JG_MOCK_PROXY_INIT` usages cannot be nested.
///
/// This macro will effectively perform these actions:
///
///   1. Reset the state of the auxiliary data for the subject function.
///   2. Wait for and lock access to the proxy/subject connection for the rest of the scope.
///   3. Set up the connection so that the proxy function can be controlled and queried in unit tests,
///      via the auxiliary data of the subject function.
///   4. Remove the connection when leaving the scope.
///
/// @see JG_MOCK, JG_MOCK_PROXY, JG_MOCK_PROXY_SUBJECT
#define JG_MOCK_PROXY_INIT(function_name) \
    JG_MOCK_RESET(function_name); \
    _JG_MOCK_PROXY_LOCK_GUARD(function_name,); \
    jg::state_scope_value<_JG_CONCAT2(function_name, _proxy_func_t)> \
        _JG_CONCAT(scope_, __LINE__)(_JG_CONCAT2(function_name, _proxy_func), _JG_CONCAT2(function_name, _proxy), nullptr)

/// @macro JG_MOCK_PROXY_OVERLOAD_INIT
///
/// @see JG_MOCK_PROXY_INIT
#define JG_MOCK_PROXY_OVERLOAD_INIT(function_name, overload_suffix) \
    JG_MOCK_OVERLOAD_RESET(function_name, overload_suffix); \
    _JG_MOCK_PROXY_LOCK_GUARD(function_name, overload_suffix); \
    jg::state_scope_value<_JG_CONCAT2(function_name, overload_suffix, _proxy_func_t)> \
        _JG_CONCAT(scope_, __LINE__)(_JG_CONCAT3(function_name, overload_suffix, _proxy_func), _JG_CONCAT3(function_name, overload_suffix, _proxy), nullptr)

#ifdef JG_MOCK_ENABLE_SHORT_NAMES
    #define MOCK                        JG_MOCK
    #define MOCK_RESET                  JG_MOCK_RESET
    //
    #define MOCK_OVERLOAD               JG_MOCK_OVERLOAD
    #define MOCK_OVERLOAD_RESET         JG_MOCK_OVERLOAD_RESET
    //
    #define MOCK_PROXY                  JG_MOCK_PROXY
    #define MOCK_PROXY_SUBJECT          JG_MOCK_PROXY_SUBJECT
    #define MOCK_PROXY_INIT             JG_MOCK_PROXY_INIT
    //
    #define MOCK_PROXY_OVERLOAD         JG_MOCK_PROXY_OVERLOAD
    #define MOCK_PROXY_OVERLOAD_SUBJECT JG_MOCK_PROXY_OVERLOAD_SUBJECT
    #define MOCK_PROXY_OVERLOAD_INIT    JG_MOCK_PROXY_OVERLOAD_INIT
#endif
