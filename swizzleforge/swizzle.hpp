// main.cpp : Defines the entry point for the application.
//
#pragma once
#include <array>
#include <cassert>


using namespace std;

template<unsigned Mask, unsigned Len>
struct SwizzleMask {};



template<typename Char, size_t N>
struct my_fixed_string
{
    constexpr my_fixed_string(const Char(&str)[N])
    {
        std::copy_n(str, N, data);
    }

    constexpr size_t size() const
    {
        return N;
    }

    Char data[N] = { };
};

constexpr unsigned char_to_bits(const char c) {
    return c == 'x' ? 0U : c == 'y' ? 1U : c == 'z' ? 2U : 3U;
}

template<std::size_t N>
constexpr unsigned build_mask(const char(&str)[N]) {
    unsigned mask = 0;
    for (std::size_t i = 0; i < N - 1; ++i) { // N-1 because string literals have '\0'
        mask = (mask << 2) | char_to_bits(str[i]);
    }
    return mask;
}

// UDL: map each char to 2-bit index and accumulate
template <my_fixed_string Cs>
constexpr auto operator""_sw() {
    
    // Build bitmask: fold a 2-bit encoding
    constexpr unsigned mask = build_mask(Cs.data);
    return SwizzleMask<mask, Cs.size()-1>{};
}

