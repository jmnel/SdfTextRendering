#include <Console/Console.hpp>
#include <Text/AtlasGlyph.hpp>
#include <Text/FontCache.hpp>
#include <Text/FontManager.hpp>

#include <Text/TextShaper.hpp>

namespace arc {

    // -- Constructor --
    TextShaper::TextShaper( FontCache &fontCache, FontManager &fontManager )
        : m_fontCache( fontCache ),
          m_fontManager( fontManager ) {
    }

    // setOrigin function --
    void TextShaper::setOrigin( Vec2f const &origin ) {
        m_origin = origin;
    }

    // -- setBox function --
    void TextShaper::setBox( Vec2f const &boxSize ) {
        m_box = boxSize;
    }

    // -- addText function --
    //    void TextShaper::addText( string const &text, gui::Properties const &properties ) {
    void TextShaper::addText( string const &text, gui::Style const &style ) {
        m_lines.emplace_back( text, style );
    }

    // -- shape function --
    void TextShaper::shape() {
        massertf( !m_lines.empty(), "Tried to shape with no text lines." );
        splitNewlines();

        if( !m_vertices.empty() ) {
            m_vertices.clear();
        }

        // Get wrap property from first line.
        const auto wordWrap = m_lines[0].m_style.m_wordWrap.value().unwrap();

        // -- Step 1: Collect all glyphs. --
        // Loop through each line.
        assertf( m_lines.size() == 1 );
        for( size_t i = 0; i < m_lines.size(); i++ ) {
            auto &line = m_lines[i];

            // Get font face from font manager.
            const auto fontFaceId = line.m_style.m_fontFamily.value().unwrap();
            const auto &fontFace = m_fontManager.getFontFace( fontFaceId );

            // Setup Harfbuzz shaping buffer.
            auto hbBuffer = hb_buffer_create();
            hb_buffer_add_utf8( hbBuffer, line.m_text.c_str(), -1, 0, -1 );

            // Use HarfBuzz to shape line.
            hb_buffer_guess_segment_properties( hbBuffer );
            hb_shape( fontFace.m_hbFont, hbBuffer, nullptr, 0 );

            // Get some property values.
            const auto fontSize = line.m_style.m_fontSize.value().unwrap();
            const auto faceScale = fontFace.m_faceScale;
            auto s = fontSize * 0.001f;
            const auto lineHeight = fontFace.m_lineHeight * s * 4.0;
            line.m_height = lineHeight;

            // Get glyph information from HarfBuzz.
            const size_t shapedCount = hb_buffer_get_length( hbBuffer );
            const auto hbGlyphInfo = hb_buffer_get_glyph_infos( hbBuffer, nullptr );
            const auto hbGlyphPos = hb_buffer_get_glyph_positions( hbBuffer, nullptr );

            for( size_t p = 0; p < shapedCount; p++ ) {
                for( size_t q = 0; q < shapedCount; q++ ) {
                    if( p != q ) {
                        auto cp0 = hbGlyphInfo[p].codepoint;
                        auto cp1 = hbGlyphInfo[q].codepoint;
                        assertf( cp0 != cp1 );
                    }
                }
            }

            // Loop through line glyhps.
            for( size_t j = 0; j < shapedCount; j++ ) {
                // Get glyph from font cache.
                const auto codepoint = hbGlyphInfo[j].codepoint;
                auto glyphWeak = m_fontCache.getGlyph( codepoint, fontFaceId );
                auto glyph = glyphWeak.lock();

                //                cout << "Got glyph from font cache. cp=" << glyph->m_codepoint << endl;

                //                cout << "j: " << codepoint << endl;

                // Calculate advance for this glyph.
                Vec2f advance( (float)hbGlyphPos[j].x_advance, 0.0f );
                advance = advance * advConst * fontSize;

                // Calulate correction.
                const Vec2f correction = -glyph->m_correction * 0.22f * s * faceScale;

                line.m_glyphs.emplace_back( glyph, Vec2f( 0.0f ) );
                line.m_glyphs.back().m_advance = advance;
                line.m_glyphs.back().m_correction = correction;

                //                cout << "1: ";
                //                cout << line.m_glyphs.back().m_glyph.m_uv[0] << endl;
            }

            // Check codepoints not same for any glyphs.
            //            for( size_t p = 0; p < line.m_glyphs.size(); p++ ) {
            //                for( size_t q = 0; q < line.m_glyphs.size(); q++ ) {
            //                    if( p != q ) {
            //                        const auto &pG = line.m_glyphs[p];
            //                        const auto &qG = line.m_glyphs[q];
            //                        if( pG.m_glyph->m_codepoint == qG.m_glyph->m_codepoint ) {
            //                            console << "Wrong glyph indices @" << p << " - " << q << endl;
            //                        }
            //                        assertf( pG.m_glyph.m_codepoint != qG.m_glyph.m_codepoint );
            //                    }
            //                }
            //            }
        }

        // -- Step 2 (optional): Apply word wrap to lines. --
        if( wordWrap == gui::properties::WordWrapValue::Hard ) {
            /// @todo Implement hard word wrap.
            assertf( false );
            abort();
        } else if( wordWrap == gui::properties::WordWrapValue::Soft ) {
            /// @todo Implement soft word wrap.
            assertf( false );
            abort();
        }

        // -- Step 3 (optional): measure horizontal width of each line --
        Vec2f pen( 0.0f );

        pen = m_origin - Vec2f( 0.0f, m_lines[0].m_height ) * Vec2f( 0.0f, m_box.y );

        float linesCombinedHeight = 0.0f;

        for( auto &line : m_lines ) {
            auto lineWidth = 0.0f;
            for( auto &glyph : line.m_glyphs ) {
                glyph.m_position = pen + glyph.m_correction;
                lineWidth += glyph.m_advance.x;
                pen = pen + glyph.m_advance;
            }
            line.m_width = lineWidth;
            linesCombinedHeight += line.m_height;
            pen = pen + Vec2f( 0.0f, -line.m_height );
            pen.x = m_origin.x;
        }

        // -- Step 4: generate vertices. --
        Vec2f blockOffset( 0.0f );

        const auto vertAlign = m_lines[0].m_style.m_alignVert;

        if( vertAlign == gui::properties::AlignVertValue::Middle ) {
            blockOffset.y = -m_box.y * 0.5f + linesCombinedHeight * 0.5f;
        }

        for( auto &line : m_lines ) {

            const auto fontFaceId = line.m_style.m_fontFamily.value().unwrap();
            const auto fontSize = line.m_style.m_fontSize.value().unwrap();
            const auto &face = m_fontManager.getFontFace( fontFaceId );
            const auto faceScale = face.m_faceScale;
            const auto s = fontSize * 0.001;

            Vec2f offset( 0.0f );

            const auto alignHorz = line.m_style.m_alignHorz.value().unwrap();

            if( alignHorz == gui::properties::AlignHorzValue::Center ) {
                offset.x = m_box.x * 0.5f - line.m_width * 0.5f;
            }

            for( auto glyph : line.m_glyphs ) {
                auto atlasGlyph = glyph.m_glyphWeak.lock();

                //                cout << atlasGlyph->m_uv[0] << endl;
                if( atlasGlyph->m_isRenderable ) {
                    vector<Vec2f> p = {atlasGlyph->m_position[0],
                                       atlasGlyph->m_position[1],
                                       atlasGlyph->m_position[2],
                                       atlasGlyph->m_position[3]};

                    vector<Vec3f> uv = {atlasGlyph->m_uv[0],
                                        atlasGlyph->m_uv[1],
                                        atlasGlyph->m_uv[2],
                                        atlasGlyph->m_uv[3]};

                    for( size_t j = 0; j < 4; j++ ) {
                        p[j] = p[j] * s * faceScale + glyph.m_position + offset + blockOffset;
                    }

                    const auto step = 0.05f;
                    const auto threshold = 0.53f;

                    const auto color = line.m_style.m_color.value().unwrap();

                    //                    const auto color = std::get<gui::properties::ColorValue>(
                    //                        line.m_style.m_color );

                    m_vertices.emplace_back(
                        Vec3f( p[0], 0.0f ), uv[0], color, step, threshold );
                    m_vertices.emplace_back(
                        Vec3f( p[1], 0.0f ), uv[1], color, step, threshold );
                    m_vertices.emplace_back(
                        Vec3f( p[2], 0.0f ), uv[2], color, step, threshold );

                    m_vertices.emplace_back(
                        Vec3f( p[0], 0.0f ), uv[0], color, step, threshold );
                    m_vertices.emplace_back(
                        Vec3f( p[2], 0.0f ), uv[2], color, step, threshold );
                    m_vertices.emplace_back(
                        Vec3f( p[3], 0.0f ), uv[3], color, step, threshold );
                }
            }
        }
    }

    // -- splitNewlines function --
    void TextShaper::splitNewlines() {
        vector<LayoutLine> newLines;

        for( auto const &line : m_lines ) {
            auto text = line.m_text;
            auto style = line.m_style;

            size_t pos = 0;
            size_t pre = 0;

            while( ( pos = text.find( '\n', pre ) ) != std::string::npos ) {
                newLines.emplace_back( text.substr( pre, pos - pre ), style );
                pre = pos + 1;
            }

            newLines.emplace_back( text.substr( pre, text.size() ), style );
        }

        std::swap( m_lines, newLines );
    }

    // -- getVertices function --
    vector<SdfVertex> &TextShaper::getVertices() {
        return m_vertices;
    }

}  // namespace arc
