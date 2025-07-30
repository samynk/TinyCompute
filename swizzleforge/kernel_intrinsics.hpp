#pragma once

#include "vec.hpp"
#include <execution>
#include <ranges>
#include <algorithm>

// ──────────────────────────────────────────────────────────────
// 1.  Kernel entry‑point concept
// ──────────────────────────────────────────────────────────────
template<typename K>
concept KernelEntry = requires(K k)
{
    { k() } -> std::same_as<void>;
};

template<typename K>
constexpr bool HasLocalSize = requires(K k) {
    { k.local_size } -> std::convertible_to<sf::uvec3>;
};

namespace sf
{
    // Thread‑local slot that dispatcher writes before invoking kernel
    inline thread_local sf::uvec3 gl_GlobalInvocationID(0, 0, 0);
}
