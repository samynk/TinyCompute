#pragma once

#include "vec.hpp"
#include <execution>
#include <ranges>
#include <algorithm>

// ──────────────────────────────────────────────────────────────
// 1.  Kernel entry‑point concept
// ──────────────────────────────────────────────────────────────
template<class K>
concept KernelEntry = requires(K k)
{
    // must be callable without arguments
    { k() } -> std::same_as<void>;
};

namespace sf
{
    // Thread‑local slot that dispatcher writes before invoking kernel
    inline thread_local sf::uvec3 gl_GlobalInvocationID(0, 0, 0);

    // ──────────────────────────────────────────────────────────────
    // 3.  Generic CPU dispatcher that accepts any KernelEntry
    // ──────────────────────────────────────────────────────────────
    template<KernelEntry K>
    void dispatch_cpu(K&& kernel, std::size_t totalWork)
    {
        auto range = std::views::iota(std::size_t{ 0 }, totalWork);
        std::for_each(std::execution::par_unseq,
            range.begin(),range.end(),
            [&](std::size_t xi) {
                sf::gl_GlobalInvocationID.x = xi;
                kernel(); 
            });
    }
}
