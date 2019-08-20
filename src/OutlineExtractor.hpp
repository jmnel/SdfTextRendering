#pragma once

#include <iostream>
#include <vector>

//#include <png++/png.hpp>
//#include <ft2build.h>
//#include FT_FREETYPE_H
//#include FT_GLYPH_H
//#include FT_OUTLINE_H

#include <Graphics/PixelBuffer.hpp>
#include <Graphics/PixelGrid.hpp>
#include <Math/RootSolver.hpp>
#include <Math/Vec.hpp>
#include <Text/FontFace.hpp>

using std::cout;
using std::endl;
using std::vector;

namespace arc {

    class OutlineExtractor {
    private:
        //static constexpr double scale = 12.0;
        // static constexpr double scale = 36.0;
        //static constexpr int glyphPadding = 128;
    public:
        static PixelGrid<uint8_t>* computeGlyphSDF( FontFace const& fontFace,
                                                    uint16_t gid,
                                                    float& correction,
                                                    double scale,
                                                    int glyphPadding );

        //        static PixelGrid<uint8_t>* computeGlyphSDF( FT_Face ftFace,
        //                                                    uint16_t gid,
        //                                                    float* correction,
        //                                                    double scale,
        //                                                    int glyphPadding );
    };
}  // namespace arc
