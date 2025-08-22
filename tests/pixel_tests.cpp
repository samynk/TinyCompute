// vec_tests.cpp
#include <gtest/gtest.h>
#
#include "vec.hpp"    
#include "kernel_intrinsics.hpp"
#include "math/arithmetic.hpp"
#include "images/ImageFormat.hpp"

TEST(PixelTest, ChannelReadAndWrite)
{
	tc::Pixel<uint8_t, 1> gray{};
	uint8_t alpha = gray.get<tc::Channel::A>();

	EXPECT_EQ(alpha, 255u);

	tc::ImageBinding<tc::GPUFormat::RGBA8, tc::Dim::D2, tc::RGB8, 1> image;
	// vervanging door make_unique
	auto* pImageBuffer = new tc::BufferResource<tc::RGB8, tc::Dim::D2>{ tc::ivec2{32,32} };
	image.attach(pImageBuffer);

	tc::imageStore(image, tc::ivec2{ 2,2 }, tc::vec4{ 127,250,0,127 });
	tc::vec4 color = tc::imageLoad(image, tc::ivec2{ 2,2 });

	EXPECT_EQ(color.x, 127u);
	EXPECT_EQ(color.y, 250u);
	EXPECT_EQ(color.z, 0u);
	EXPECT_EQ(color.w, 255u);
}

TEST(PixelTest, RWGrayImage)
{
	tc::ImageBinding<tc::GPUFormat::R8UI, tc::Dim::D2, tc::R8UI, 1> image;
	// vervanging door make_unique
	auto* pImageBuffer = new tc::BufferResource<tc::R8UI, tc::Dim::D2>{ tc::ivec2{32,32} };
	image.attach(pImageBuffer);

	tc::imageStore(image, tc::ivec2{ 2,2 }, tc::uint{ 127 });

	tc::uvec4 color = tc::imageLoad(image, tc::ivec2{ 2,2 });

	EXPECT_EQ(color.x, 127);
	EXPECT_EQ(color.y, 127);
	EXPECT_EQ(color.z, 127);
	EXPECT_EQ(color.w, 255);
}