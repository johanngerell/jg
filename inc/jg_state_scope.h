#pragma once

namespace jg
{

/// @class jg::state_scope_value
///
/// Makes it safer and easier to write a test where a state must be changed during the test and
/// then reverted when the test ends. This is typically used for global/static state with lifetime
/// exceeding the scope of the test.
///
/// Using this class is **safer** because 1) it's impossible to forget to correctly reset the state
/// (a specific value) after a test, and 2) the correct state will be reset even in the presence of
/// exceptions.
///
/// Using this class is **easier** because it saves a boatload of typing required to cover all
/// possible test exit paths.
///
/// @note Depending on *any* global/static state in a test is inherently not thread safe. This
/// means that no tests that depend on global/static state should should run concurrently. That
/// is a deficiency of the tested code, not of the `jg::state_scope_value` helper class.
///
/// @example
///
///     extern bool g_some_global_flag;
///
///     TEST()
///     {
///         // Naming it '_' emphasizes its unused-ness.
///         jg::state_scope_value _(g_some_global_flag, false, true);
///     
///         // perform_action() depends on g_some_global_flag.
///         TEST_ASSERT(perform_action());
///     }
template <typename T, typename U = T, typename V = T>
class state_scope_value final
{
public:
    /// @param instance The `T` instance that is changed in the scope of a `jg::state_scope_value` instance.
    /// @param enter_value Value set to the `T` instance when `jg::state_scope_value` is constructed.
    /// @param exit_value Value set to the `T` instance when `jg::state_scope_value` is destroyed.
    state_scope_value(T& instance, U enter_value, V exit_value)
        : m_instance(&instance)
        , m_exit_value(std::move(exit_value))
    {
        *m_instance = std::move(enter_value);
    }

    ~state_scope_value()
    {
        *m_instance = std::move(m_exit_value);
    }

    state_scope_value(const state_scope_value&) = delete;
    state_scope_value& operator=(const state_scope_value&) = delete;

private:
    T* m_instance;
    V m_exit_value;
};

/// @class jg::state_scope_action
///
/// Makes it safer and easier to write a test where a state must be changed during the test and
/// then reverted when the test ends. This is typically used for global/static state with lifetime
/// exceeding the scope of the test.
///
/// Using this class is **safer** because 1) it's impossible to forget to correctly reset the state
/// (a specific value) after a test, and 2) the correct state will be reset even in the presence of
/// exceptions.
///
/// Using this class is **easier** because it saves a boatload of typing required to cover all
/// possible test exit paths.
///
/// @note Depending on *any* global/static state in a test is inherently not thread safe. This
/// means that no tests that depend on global/static state should should run concurrently. That
/// is a deficiency of the tested code, not of the `jg::state_scope_action` helper class.
///
/// @example
///
///     class stupid_singleton; // abstract or having protected state manipulatable by a derived class.
///
///     TEST()
///     {
///         auto* real_instance = stupid_singleton::get_instance();
///         test_singleton test_instance; // derives from stupid_singleton
///
///         // Naming it '_' emphasizes its unused-ness.
///         jg::state_scope_action _([&]{ stupid_singleton::set_instance(test_instance); },
///                                  [&]{ stupid_singleton::set_instance(real_instance); });
///     
///         // perform_action() depends on stupid_singleton.
///         TEST_ASSERT(perform_action());
///     }
template <typename U, typename V>
class state_scope_action final
{
public:
    /// @param enter_action Parameter-less callable that is invoked when `jg::state_scope_action` is instantiated.
    /// @param exit_action Parameter-less callable that is invoked when `jg::state_scope_action` is destroyed.
    state_scope_action(U enter_action, V exit_action)
        : m_exit_action(std::move(exit_action))
    {
        enter_action();
    }

    ~state_scope_action()
    {
        m_exit_action();
    }

    state_scope_action(const state_scope_action&) = delete;
    state_scope_action& operator=(const state_scope_action&) = delete;

private:
    V m_exit_action;
};

} // namespace jg
