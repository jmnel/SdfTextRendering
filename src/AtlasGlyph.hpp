#pragma once

#include <array>

#include <Math/Vec.hpp>

using std::array;

namespace arc {

    struct AtlasGlyph {
        uint16_t m_codepoint = 0;
        size_t m_width = 0;
        size_t m_height = 0;
        Vec2f m_correction = Vec2f::zero();
        uint8_t* m_pixels = nullptr;
        size_t m_pixelSize = 0;
        array<Vec2f, 4> m_position;
        array<Vec3f, 4> m_uv;

        bool m_isRenderable = false;
    };

}  // namespace arc
