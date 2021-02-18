#include <jg_optional.h>
#include <jg_mock.h>
#include <jg_test.h>

JG_MOCK_REF(,,, void, mock_assert, bool);

namespace {

struct my_type final
{
    int i;
    bool b;
    std::string s;
};

template <typename Op>
void jg_test_assert_accesso_fail(Op op)
{
    mock_assert_.reset();
    op();
    jg_test_assert(mock_assert_.called());
    jg_test_assert(mock_assert_.param<1>() == false);
}

void jg_test_assert_accessors_fail(const jg::optional<my_type>& optional)
{
    jg_test_assert_accesso_fail([&] { (void)optional.value(); });
    jg_test_assert_accesso_fail([&] { (void)optional->i; });
    jg_test_assert_accesso_fail([&] { (void)(*optional).i; });
}

void jg_test_assert_accessors(const jg::optional<my_type>& optional, int i, bool b, const std::string& s)
{
    jg_test_assert(optional->i == i);
    jg_test_assert(optional->b == b);
    jg_test_assert(optional->s == s);  
    jg_test_assert(optional.value().i == i);
    jg_test_assert(optional.value().b == b);
    jg_test_assert(optional.value().s == s);
    jg_test_assert((*optional).i == i);
    jg_test_assert((*optional).b == b);
    jg_test_assert((*optional).s == s);
}

jg::test_adder optional_tests { "optional", {
    jg::test_suite { "construction", {
        jg::test_case { "Overhead is one bool + alignment padding", [] {
            jg_test_assert(sizeof(jg::optional<my_type>) - sizeof(my_type) - sizeof(bool) < sizeof(void*));
        }},
        jg::test_case { "Default constructed has no value", [] {
            jg::optional<my_type> optional;

            jg_test_assert(!optional);
            jg_test_assert(!optional.has_value());
            jg_test_assert_accessors_fail(optional);
        }},
        jg::test_case { "Constructed with rvalue has value - alt 1", [] {
            jg::optional<my_type> optional{my_type{4711, true, "foobar"}};
            
            jg_test_assert(!!optional);
            jg_test_assert(optional.has_value());
            jg_test_assert_accessors(optional, 4711, true, "foobar");
        }},
        jg::test_case { "Constructed with rvalue has value - alt 2", [] {
            my_type mt{4711, true, "foobar"};
            jg::optional<my_type> optional{std::move(mt)};

            jg_test_assert(!!optional);
            jg_test_assert(optional.has_value());
            jg_test_assert_accessors(optional, 4711, true, "foobar");
        }},
        jg::test_case { "Constructed with lvalue has value", [] {
            const my_type mt{4711, true, "foobar"};
            jg::optional<my_type> optional{mt};

            jg_test_assert(!!optional);
            jg_test_assert(optional.has_value());
            jg_test_assert_accessors(optional, 4711, true, "foobar");
        }},
        jg::test_case { "Constructed with rvalue optional has value - alt 1", [] {
            jg::optional<my_type> optional{jg::optional<my_type>{my_type{4711, true, "foobar"}}};
            
            jg_test_assert(!!optional);
            jg_test_assert(optional.has_value());
            jg_test_assert_accessors(optional, 4711, true, "foobar");
        }},
        jg::test_case { "Constructed with rvalue optional has value - alt 2", [] {
            jg::optional<my_type> other{my_type{4711, true, "foobar"}};
            jg::optional<my_type> optional{std::move(other)};

            jg_test_assert(!!optional);
            jg_test_assert(optional.has_value());
            jg_test_assert_accessors(optional, 4711, true, "foobar");
        }},
        jg::test_case { "Constructed with lvalue optional has value", [] {
            const jg::optional<my_type> other{my_type{4711, true, "foobar"}};
            jg::optional<my_type> optional{other};

            jg_test_assert(!!optional);
            jg_test_assert(optional.has_value());
            jg_test_assert_accessors(optional, 4711, true, "foobar");
        }}
    }},
    jg::test_suite { "assignment", {
        jg::test_case { "Assigned with rvalue optional has value - alt 1", [] {
            jg::optional<my_type> optional{my_type{4712, false, "foo"}};
            optional = jg::optional<my_type>{my_type{4711, true, "foobar"}};
            
            jg_test_assert(!!optional);
            jg_test_assert(optional.has_value());
            jg_test_assert_accessors(optional, 4711, true, "foobar");
        }},
        jg::test_case { "Assigned with rvalue optional has value - alt 2", [] {
            jg::optional<my_type> other{my_type{4711, true, "foobar"}};
            jg::optional<my_type> optional{my_type{4712, false, "bar"}};
            optional = std::move(other);

            jg_test_assert(!!optional);
            jg_test_assert(optional.has_value());
            jg_test_assert_accessors(optional, 4711, true, "foobar");
        }},
        jg::test_case { "Assigned with lvalue optional has value", [] {
            const jg::optional<my_type> other{my_type{4711, true, "foobar"}};
            jg::optional<my_type> optional{my_type{4712, false, "bar"}};
            optional = other;

            jg_test_assert(!!optional);
            jg_test_assert(optional.has_value());
            jg_test_assert_accessors(optional, 4711, true, "foobar");
        }},
        jg::test_case { "Assigned with rvalue has value - alt 1", [] {
            jg::optional<my_type> optional{my_type{4712, false, "bar"}};
            optional = my_type{4711, true, "foobar"};
            
            jg_test_assert(!!optional);
            jg_test_assert(optional.has_value());
            jg_test_assert_accessors(optional, 4711, true, "foobar");
        }},
        jg::test_case { "Assigned with rvalue has value - alt 2", [] {
            my_type mt{4711, true, "foobar"};
            jg::optional<my_type> optional{my_type{4712, false, "bar"}};
            optional = std::move(mt);

            jg_test_assert(!!optional);
            jg_test_assert(optional.has_value());
            jg_test_assert_accessors(optional, 4711, true, "foobar");
        }},
        jg::test_case { "Assigned with lvalue has value", [] {
            const my_type mt{4711, true, "foobar"};
            jg::optional<my_type> optional{my_type{4712, false, "bar"}};
            optional = mt;

            jg_test_assert(!!optional);
            jg_test_assert(optional.has_value());
            jg_test_assert_accessors(optional, 4711, true, "foobar");
        }}
    }}
}};

}
