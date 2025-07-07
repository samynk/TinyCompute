// vec_tests.cpp
#include <gtest/gtest.h>
#include "vec.hpp"          // your SIMD-friendly template
#include "swizzle.hpp"      // proxy helpers if split out

// 1. Construction & equality --------------------------------------------------
TEST(Vec4Basic, ConstructorAndCompare)
{
    sf::vec4 a{ 1.0f, 2.0f, 3.0f, 4.0f };
    sf::vec4 b{ 1.0f, 2.0f, 3.0f, 4.0f };

    EXPECT_EQ(a, b);
    float ax = a.x;
    float aw = a.w;
    EXPECT_FLOAT_EQ(ax, 1.0f);
    EXPECT_FLOAT_EQ(aw, 4.0f);
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

    EXPECT_EQ(sum, esum);
    EXPECT_EQ(prod, eprod);
    EXPECT_EQ(diff, ediff);
}

// 3. Swizzle round-trip --------------------------------------------------------
TEST(Vec4Swizzle, XYZWShuffle)
{
    sf::vec4 v{ 9,8,7,6 };

    // r-value swizzle
    auto xy = v["yx"_sw];  
    auto expected = sf::vec2( 8, 9 );
    EXPECT_EQ(xy, expected);

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

    //float dot = sf::dot(a, b);               // 1*4 + 2*5 + 3*6 = 32
    //EXPECT_FLOAT_EQ(dot, 32.0f);
}