#include <gtest/gtest.h>
#include "vec.hpp"
#include "swizzle.hpp"

namespace {

// Helper that checks if all components in a boolean vector are true
template<std::size_t N>
bool AllTrue(const sf::vec_base<bool, N>& v) {
    for (std::size_t i = 0; i < N; ++i) {
        if (!v[i]) return false;
    }
    return true;
}

// Predicate used with EXPECT_PRED_FORMAT2 to print nice diagnostics
template<typename T, std::size_t N>
::testing::AssertionResult VecEqual(const char* lhs_expr,
                                    const char* rhs_expr,
                                    const sf::vec_base<T, N>& lhs,
                                    const sf::vec_base<T, N>& rhs) {
    auto cmp = lhs == rhs;
    for (std::size_t i = 0; i < N; ++i) {
        if (!cmp[i]) {
            return ::testing::AssertionFailure()
                   << lhs_expr << " and " << rhs_expr
                   << " differ at index " << i
                   << " (" << lhs[i] << " vs " << rhs[i] << ")";
        }
    }
    return ::testing::AssertionSuccess();
}

} // namespace

TEST(VecBasics, ConstructionAndAccess) {
    sf::vec4 a{1.f, 2.f, 3.f, 4.f};
    EXPECT_FLOAT_EQ(a.x, 1.f);
    EXPECT_FLOAT_EQ(a.w, 4.f);

    sf::vec4 b{1.f, 2.f, 3.f, 4.f};
    EXPECT_PRED_FORMAT2((VecEqual<float,4>), a, b);
}

TEST(VecBasics, ArithmeticOps) {
    sf::vec4 v{1, 2, 3, 4};
    sf::vec4 u{4, 0, 1, 2};

    sf::vec4 sum  = v + u;        // (5,2,4,6)
    sf::vec4 prod = v * 2.0f;     // (2,4,6,8)
    sf::vec4 diff = sum - prod;   // (3,-2,-2,-2)

    EXPECT_PRED_FORMAT2((VecEqual<float,4>), sum,  sf::vec4(5,2,4,6));
    EXPECT_PRED_FORMAT2((VecEqual<float,4>), prod, sf::vec4(2,4,6,8));
    EXPECT_PRED_FORMAT2((VecEqual<float,4>), diff, sf::vec4(3,-2,-2,-2));
}

TEST(VecSwizzle, ReadOnly) {
    sf::vec4 v{9,8,7,6};
    sf::vec2 swz = v["yx"_sw];
    EXPECT_PRED_FORMAT2((VecEqual<float,2>), swz, sf::vec2(8,9));
}

TEST(VecMath, DotProduct) {
    sf::vec3 a{1,2,3};
    sf::vec3 b{4,5,6};
    float dot = sf::dot(a,b);
    EXPECT_FLOAT_EQ(dot, 32.f);
}
