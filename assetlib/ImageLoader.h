
#include <string>



#include "vec.hpp"
#include "images/ImageFormat.h"
#include <kernel_intrinsics.hpp>

namespace tc::assets {
    unsigned char* loadImage(const std::string& fileName,unsigned numChannels, tc::ivec2& dimension);
    void writeImage(const std::string& fileName, unsigned char* pImgData, unsigned w, unsigned h, unsigned channels, unsigned stride);
    
    void freeImage(unsigned char* imgData);

    template<tc::PixelType P>
    tc::BufferResource<P, tc::Dim::D2>* loadImage(const std::string& filename)
    {
        static_assert(P::NumChannels >= 1 && P::NumChannels <= 4,
            "Pixel NumChannels must be 1..4");

        ivec2 dimension;
        unsigned char* pixelData = loadImage(filename, P::NumChannels, dimension);

        using ResourceType = BufferResource<P, tc::Dim::D2>;
        tc::uint w = static_cast<tc::uint>(dimension.x);
        tc::uint h = static_cast<tc::uint>(dimension.y);
        auto* buffer = new ResourceType(ivec2{w,h});

        std::memcpy(buffer->data(), pixelData, w * h * sizeof(P));
        
        return buffer;
    }

    template<tc::PixelType P>
    void writeImage(const std::string& filename, 
        tc::BufferResource<P, tc::Dim::D2>* pImage)
    {
        static_assert(P::NumChannels >= 1 && P::NumChannels <= 4,
            "Pixel NumChannels must be 1..4");

        tc::uvec2 dim = pImage->getDimension();
        unsigned stride = dim.x * sizeof(P);
        writeImage(filename, (unsigned char*)pImage->data(), dim.x, dim.y, P::NumChannels, stride);
    }
}