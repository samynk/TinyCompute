// vec_tests.cpp
#include <gtest/gtest.h>
#
#include "vec.hpp"    
#include "math/arithmetic.hpp"
#include "swizzle.hpp"

// 1. Construction & equality --------------------------------------------------
TEST(Vec4Basic, ConstructorAndCompare)
{
    sf::vec4 a{ 1.0f, 2.0f, 3.0f, 4.0f };
    sf::vec4 b{ 1.0f, 2.0f, 3.0f, 4.0f };

    EXPECT_TRUE(sf::all(a == b));
    float ax = a.x;
    float aw = a.w;
    EXPECT_FLOAT_EQ(ax, 1.0f);
    EXPECT_FLOAT_EQ(aw, 4.0f);
    
    sf::vec4 c{ 1 };
    sf::vec4 ec{ 1,1,1,1 };
    EXPECT_TRUE(sf::all(c == ec));
}

// 2. Arithmetic operators -----------------------------------------------------
TEST(Vec4Basic, AddMulSub)
{
    sf::vec4 v{ 1, 2, 3, 4 };
    sf::vec4 u{ 4, 0, 1, 2 };

    auto sum = v + u;              // (5, 2, 4, 6)
    auto prod = v * 2.0f;           // (2, 4, 6, 8)
    auto diff = sum - prod;
    
    auto esum = sf::vec4(5, 2, 4, 6);
    auto eprod = sf::vec4(2, 4, 6, 8);
    auto ediff = sf::vec4(3, -2, -2, -2);

    EXPECT_TRUE(sf::all(esum == sum));
    EXPECT_TRUE(sf::all(eprod == prod));
    EXPECT_TRUE(sf::all(ediff == diff));
}

// 3. Swizzle round-trip --------------------------------------------------------
TEST(Vec4Swizzle, XYZWShuffle)
{
    sf::vec4 v{ 9,8,7,6 };

    // r-value swizzle
    sf::vec2 xy = v["yx"_sw];
    sf::vec2 expected{ 8.0f, 9.0f };
    
    EXPECT_TRUE(sf::all(xy == expected));

    // l-value swizzle assignment (swap x and w)
    // todo
    // v.wx = v["xw"_sw];                            // now v = (6,8,7,9)
    // EXPECT_EQ(v, (sf::vec<4, float>(6, 8, 7, 9)));

    // chained swizzle on temporary
    // todo
    // auto zzx = v.zzx;                       // (7,7,6)
    // EXPECT_EQ(zzx, (sf::vec<3, float>(7, 7, 6)));
}

// 4. Dot product utility -------------------------------------------------------
TEST(VecMath, DotProduct)
{
    sf::vec3 a{ 1,2,3 };
    sf::vec3 b{ 4,5,6 };

    float d = sf::dot(a, b);               // 1*4 + 2*5 + 3*6 = 32
    EXPECT_FLOAT_EQ(d, 32.0f);
}