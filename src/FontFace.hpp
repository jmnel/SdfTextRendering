#pragma once

#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

#include <harfbuzz/hb-ft.h>

#include <Utility/StringId.hpp>

namespace arc {

    class FontLibrary;
    class OutlineExtractor;

    class FontFace {
    public:
        const StringId m_name;
        float m_faceScale = 0.0f;
        float m_lineHeight = 0.0f;

        FT_Face m_ftFace;
        hb_font_t *m_hbFont = nullptr;

        bool m_isValidFlag = false;

    public:
        FontFace( const StringId name );
        FontFace( FontFace const & ) = delete;
        FontFace( FontFace &&other );

        ~FontFace();

        FontFace &operator=( FontFace const & ) = delete;
        FontFace &operator=( FontFace &&other );

        StringId getName() const;

        friend class FontLibrary;
        friend class OutlineExtractor;
    };

}  // namespace arc
