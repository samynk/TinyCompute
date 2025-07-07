#pragma once

namespace shader {
    struct binding {
        constexpr binding(unsigned int index) noexcept
            : index(index)
        {
        }

        unsigned int index;
    };

    struct set {
        constexpr set(unsigned int index) noexcept
            : index(index)
        {
        }

        unsigned int index;
    };
}