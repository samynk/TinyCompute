#pragma once
#include <array>
#include <cassert>
#include <algorithm>

template<unsigned Mask, unsigned Len>
struct SwizzleMask {
};

constexpr bool is_valid_char(const char c) {
	switch (c) {
	case 'x': case 'r': case 's':
	case 'y': case 'g': case 't':
	case 'z': case 'b': case 'p':
	case 'w': case 'a': case 'q':
		return true;
	default:
		return false;
	}
}

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

	constexpr bool isValid() const
	{
		for (std::size_t i = 0; i < N - 1; ++i) {
			if (!is_valid_char(data[i])) {
				return false;
			}
		}
		return true;
	}

	Char data[N] = { };
};




constexpr unsigned char_to_bits(const char c) {
	switch (c) {
	case 'x': case 'r': case 's': return 0u;
	case 'y': case 'g': case 't': return 1u;
	case 'z': case 'b': case 'p': return 2u;
	case 'w': case 'a': case 'q': return 3u;
	}
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
	static_assert(Cs.size() <= 4, "Invalid swizzle literal: maximum number of swizzles is 4");
	static_assert(Cs.isValid(), "Invalid swizzle literal: only characters in xyzw/rgba/stpq are allowed");
	// Build bitmask: fold a 2-bit encoding
	constexpr unsigned mask = build_mask(Cs.data);
	return SwizzleMask<mask, Cs.size() - 1>{};
}

