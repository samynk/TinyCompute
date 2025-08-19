#include "ImageLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

unsigned char* tc::assets::loadImage(const std::string& fileName, unsigned numChannels, tc::ivec2& dimension)
{
    int channels;
    stbi_uc* data = stbi_load(fileName.c_str(), &dimension.x, &dimension.y, &channels, numChannels);

    if (!data)
        throw std::runtime_error("Failed to load image: " + fileName);

    if (channels != numChannels)
        throw std::runtime_error("Unexpected channel count: " + std::to_string(channels));
    return data;
}

void tc::assets::writeImage(const std::string& fileName, 
    unsigned char* pImgData, 
    unsigned w, unsigned h,
    unsigned channels, unsigned stride)
{
    stbi_write_png(fileName.c_str(), w, h, channels, pImgData, stride);
}

void tc::assets::freeImage(unsigned char* imgData)
{
    stbi_image_free(imgData);
}
