// vec_tests.cpp
#include <gtest/gtest.h>
#
#include "vec.hpp"    
#include "math/arithmetic.hpp"
#include "math/linearalgebra.hpp"
#include "swizzle.hpp"

// 1. Construction & equality --------------------------------------------------
TEST(Vec4Basic, ConstructorAndCompare)
{
    tc::vec4 a{ 1.0f, 2.0f, 3.0f, 4.0f };
    tc::vec4 b{ 1.0f, 2.0f, 3.0f, 4.0f };

    EXPECT_TRUE(tc::all(a == b));
    float ax = a.x;
    float aw = a.w;
    EXPECT_FLOAT_EQ(ax, 1.0f);
    EXPECT_FLOAT_EQ(aw, 4.0f);
    
    tc::vec4 c{ 1 };
    tc::vec4 ec{ 1,1,1,1 };
    EXPECT_TRUE(tc::all(c == ec));
}

// 2. Arithmetic operators -----------------------------------------------------
TEST(Vec4Basic, AddMulSub)
{
    tc::vec4 v{ 1, 2, 3, 4 };
    tc::vec4 u{ 4, 0, 1, 2 };

    auto sum = v + u;              // (5, 2, 4, 6)
    auto prod = v * 2.0f;           // (2, 4, 6, 8)
    auto diff = sum - prod;
    
    auto esum = tc::vec4(5, 2, 4, 6);
    auto eprod = tc::vec4(2, 4, 6, 8);
    auto ediff = tc::vec4(3, -2, -2, -2);

    EXPECT_TRUE(tc::all(esum == sum));
    EXPECT_TRUE(tc::all(eprod == prod));
    EXPECT_TRUE(tc::all(ediff == diff));
}

// 3. Swizzle round-trip --------------------------------------------------------
TEST(Vec4Swizzle, XYZWShuffle)
{
    tc::vec4 v{ 9,8,7,6 };

    // r-value swizzle
    tc::vec2 xy = v["yx"_sw];
    tc::vec2 expected{ 8.0f, 9.0f };
    
    EXPECT_TRUE(tc::all(xy == expected));

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
    tc::vec3 a{ 1,2,3 };
    tc::vec3 b{ 4,5,6 };

    float d = tc::dot(a, b);               // 1*4 + 2*5 + 3*6 = 32
    EXPECT_FLOAT_EQ(d, 32.0f);
}

TEST(VecMath, Normalize)
{
    tc::vec3 a{ 3,4,5 };
    tc::vec3 n = tc::normalize(a);

    float l = sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
    EXPECT_EQ(n.x, 3 / l) ;
    EXPECT_EQ(n.y, 4 / l);
    EXPECT_EQ(n.z, 5 / l);
}