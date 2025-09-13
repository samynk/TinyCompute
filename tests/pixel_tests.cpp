// vec_tests.cpp
#include <gtest/gtest.h>
#
#include "vec.hpp"    
#include "kernel_intrinsics.hpp"
#include "math/arithmetic.hpp"
#include "images/ImageFormat.hpp"

TEST(PixelTest, ReadAndWriteChannels)
{
	using namespace tc;
	tc::cpu::RGB8 px{ 0.2,0.3,0.5 };

	float r = px.get<Channel::R>();
	float g = px.get<Channel::G>();
	float b = px.get<Channel::B>();
	float a = px.get<Channel::A>();

	EXPECT_EQ(r, 0.2);
	EXPECT_EQ(g, 0.3);
	EXPECT_EQ(b, 0.5);
	EXPECT_EQ(a, 1.0f);

	using r_t  = tc::cpu::RGB8::ChannelType<Channel::R>;
	static_assert(std::floating_point<r_t>);

	using g_t = tc::cpu::RGB8::ChannelType<Channel::G>;
	static_assert(std::floating_point<r_t>);

	using b_t = tc::cpu::RGB8::ChannelType<Channel::B>;
	static_assert(std::floating_point<r_t>);

	using a_t = tc::cpu::RGB8::ChannelType<Channel::A>;
	static_assert(std::floating_point<r_t>);
}

TEST(PixelTest, ChannelReadAndWrite)
{
	tc::cpu::Pixel<uint8_t, 1> gray{};
	uint8_t alpha = gray.get<tc::Channel::A>();

	EXPECT_EQ(alpha, 255u);

	tc::ImageBinding<tc::InternalFormat::RGBA8, tc::Dim::D2, tc::cpu::RGB8, 1> image;
	// vervanging door make_unique
	auto* pImageBuffer = new tc::BufferResource<tc::cpu::RGB8, tc::Dim::D2>{ tc::ivec2{32,32} };
	image.attach(pImageBuffer);

	tc::imageStore(image, tc::ivec2{ 2,2 }, tc::vec4{ 0.8f,0.71f, 0.24f, 0.5f });
	tc::vec4 color = tc::imageLoad(image, tc::ivec2{ 2,2 });

	EXPECT_EQ(color.x, 0.8f);
	EXPECT_EQ(color.y, 0.71f);
	EXPECT_EQ(color.z, 0.24f);
	EXPECT_EQ(color.w, 1.0f);
}

TEST(PixelTest, RWGrayImage)
{
	tc::ImageBinding<tc::InternalFormat::R8UI, tc::Dim::D2, tc::cpu::R8UI, 1> image;
	// vervanging door make_unique
	auto* pImageBuffer = new tc::BufferResource<tc::cpu::R8UI, tc::Dim::D2>{ tc::ivec2{32,32} };
	image.attach(pImageBuffer);

	tc::imageStore(image, tc::ivec2{ 2,2 }, tc::uint{ 127 });

	tc::uvec4 color = tc::imageLoad(image, tc::ivec2{ 2,2 });

	EXPECT_EQ(color.x, 127);
	EXPECT_EQ(color.y, 127);
	EXPECT_EQ(color.z, 127);
	EXPECT_EQ(color.w, 255);
}