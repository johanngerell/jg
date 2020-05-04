#include "catch2/catch.hpp"
#include <jg_optional.h>
#include <jg_mock.h>

JG_MOCK_REF(,,, void, test_assert, bool);

struct my_type final
{
    int i;
    bool b;
    std::string s;
};

template <typename Op>
void REQUIRE_ACCESSOR_ASSERT(Op op)
{
    test_assert_.reset(); \
    op();
    REQUIRE(test_assert_.called());
    REQUIRE(test_assert_.param<1>() == false);
}

void REQUIRE_ACCESSORS_ASSERT(const jg::optional<my_type>& optional)
{
    REQUIRE_ACCESSOR_ASSERT([&] { optional.value(); });
    REQUIRE_ACCESSOR_ASSERT([&] { optional->i; });
    REQUIRE_ACCESSOR_ASSERT([&] { (*optional).i; });
}

void REQUIRE_ACCESSORS(const jg::optional<my_type>& optional, int i, bool b, const std::string& s)
{
    REQUIRE(optional->i == i);
    REQUIRE(optional->b == b);
    REQUIRE(optional->s == s);  
    REQUIRE(optional.value().i == i);
    REQUIRE(optional.value().b == b);
    REQUIRE(optional.value().s == s);
    REQUIRE((*optional).i == i);
    REQUIRE((*optional).b == b);
    REQUIRE((*optional).s == s);
}

TEST_CASE("jg_optional")
{
    SECTION("Overhead is one bool + alignment padding")
    {
        REQUIRE(sizeof(jg::optional<my_type>) - sizeof(my_type) - sizeof(bool) < sizeof(void*));
    }

    SECTION("Construction")
    {
        SECTION("Default constructed has no value")
        {
            jg::optional<my_type> optional;

            REQUIRE(!optional);
            REQUIRE(!optional.has_value());
            REQUIRE_ACCESSORS_ASSERT(optional);
        }

        SECTION("Constructed with rvalue has value - alt 1")
        {
            jg::optional<my_type> optional{my_type{4711, true, "foobar"}};
            
            REQUIRE(optional);
            REQUIRE(optional.has_value());
            REQUIRE_ACCESSORS(optional, 4711, true, "foobar");
        }

        SECTION("Constructed with rvalue has value - alt 2")
        {
            my_type mt{4711, true, "foobar"};
            jg::optional<my_type> optional{std::move(mt)};

            REQUIRE(optional);
            REQUIRE(optional.has_value());
            REQUIRE_ACCESSORS(optional, 4711, true, "foobar");
        }

        SECTION("Constructed with lvalue has value")
        {
            const my_type mt{4711, true, "foobar"};
            jg::optional<my_type> optional{mt};

            REQUIRE(optional);
            REQUIRE(optional.has_value());
            REQUIRE_ACCESSORS(optional, 4711, true, "foobar");
        }

        SECTION("Constructed with rvalue optional has value - alt 1")
        {
            jg::optional<my_type> optional{jg::optional<my_type>{my_type{4711, true, "foobar"}}};
            
            REQUIRE(optional);
            REQUIRE(optional.has_value());
            REQUIRE_ACCESSORS(optional, 4711, true, "foobar");
        }

        SECTION("Constructed with rvalue optional has value - alt 2")
        {
            jg::optional<my_type> other{my_type{4711, true, "foobar"}};
            jg::optional<my_type> optional{std::move(other)};

            REQUIRE(optional);
            REQUIRE(optional.has_value());
            REQUIRE_ACCESSORS(optional, 4711, true, "foobar");
        }

        SECTION("Constructed with lvalue optional has value")
        {
            const jg::optional<my_type> other{my_type{4711, true, "foobar"}};
            jg::optional<my_type> optional{other};

            REQUIRE(optional);
            REQUIRE(optional.has_value());
            REQUIRE_ACCESSORS(optional, 4711, true, "foobar");
        }
    }

    SECTION("Assignment")
    {
        SECTION("Assigned with rvalue optional has value - alt 1")
        {
            jg::optional<my_type> optional{my_type{4712, false, "foo"}};
            optional = std::move(jg::optional<my_type>{my_type{4711, true, "foobar"}});
            
            REQUIRE(optional);
            REQUIRE(optional.has_value());
            REQUIRE_ACCESSORS(optional, 4711, true, "foobar");
        }

        SECTION("Assigned with rvalue optional has value - alt 2")
        {
            jg::optional<my_type> other{my_type{4711, true, "foobar"}};
            jg::optional<my_type> optional{my_type{4712, false, "bar"}};
            optional = std::move(other);

            REQUIRE(optional);
            REQUIRE(optional.has_value());
            REQUIRE_ACCESSORS(optional, 4711, true, "foobar");
        }

        SECTION("Assigned with lvalue optional has value")
        {
            const jg::optional<my_type> other{my_type{4711, true, "foobar"}};
            jg::optional<my_type> optional{my_type{4712, false, "bar"}};
            optional = other;

            REQUIRE(optional);
            REQUIRE(optional.has_value());
            REQUIRE_ACCESSORS(optional, 4711, true, "foobar");
        }

        SECTION("Assigned with rvalue has value - alt 1")
        {
            jg::optional<my_type> optional{my_type{4712, false, "bar"}};
            optional = std::move(my_type{4711, true, "foobar"});
            
            REQUIRE(optional);
            REQUIRE(optional.has_value());
            REQUIRE_ACCESSORS(optional, 4711, true, "foobar");
        }

        SECTION("Assigned with rvalue has value - alt 2")
        {
            my_type mt{4711, true, "foobar"};
            jg::optional<my_type> optional{my_type{4712, false, "bar"}};
            optional = std::move(mt);

            REQUIRE(optional);
            REQUIRE(optional.has_value());
            REQUIRE_ACCESSORS(optional, 4711, true, "foobar");
        }

        SECTION("Assigned with lvalue has value")
        {
            const my_type mt{4711, true, "foobar"};
            jg::optional<my_type> optional{my_type{4712, false, "bar"}};
            optional = mt;

            REQUIRE(optional);
            REQUIRE(optional.has_value());
            REQUIRE_ACCESSORS(optional, 4711, true, "foobar");
        }
    }
}
