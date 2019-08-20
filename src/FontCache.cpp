#include <array>
#include <fstream>
#include <ostream>

#include <Console/Console.hpp>
#include <Text/FontManager.hpp>
#include <Text/OutlineExtractor.hpp>

#include <Text/FontCache.hpp>

using std::array;
using std::ifstream;
using std::make_shared;
using std::make_unique;
using std::ofstream;

namespace arc {

    using gl::TextureArray2D;

    // -- Constructor --
    FontCache::FontCache( FontManager const &fontManager )
        : m_fontManager( fontManager ) {
        // Setup atlas texture.
        m_atlasTexture.bind();
        m_atlasTexture.setMinFilter( TextureArray2D::MinFilter::Linear );
        m_atlasTexture.setMagFilter( TextureArray2D::MagFilter::Linear );
        m_atlasTexture.setTextureWrapS( TextureArray2D::TextureWrap::ClampToEdge );
        m_atlasTexture.setTextureWrapT( TextureArray2D::TextureWrap::ClampToEdge );
        m_atlasTexture.setupStorage( 1,
                                     TextureArray2D::SizedInternalFormat::R8,
                                     m_atlasWidth,
                                     m_atlasHeight,
                                     m_atlasLayers );
        m_atlasTexture.clear( Red );
    }

    // -- Destructor --
    FontCache::~FontCache() {
    }

    // -- getAtlasTexture function --
    gl::TextureArray2D &FontCache::getAtlasTexture() {
        return m_atlasTexture;
    }

    // -- loadCacheFromFile function --a
    bool FontCache::loadCacheFromFile( fs::path const &filePath ) {
        /*
        console << "Reading font cache from file..." << endl;

        ifstream file;
        file.open( filePath.c_str(), std::ios::in | std::ios::binary );

        if( !file.good() ) {
            console << "Error: FontCache could not find font cache file." << endl;
            return false;
        }

        // Read faces count.
        size_t facesCount = 0;
        file.read( (char *)&facesCount, sizeof( size_t ) );

        for( size_t iFace = 0; iFace < facesCount; iFace++ ) {

            // Read face name id.
            StringId name = 0;
            file.read( (char *)&name, sizeof( StringId ) );
            auto tableIter = m_glyphTables.insert( {name, make_unique< GlyphTable>()} );
            auto table = &tableIter.first->second;
            size_t glyphCount = 0;

            file.read( (char *)&glyphCount, sizeof( size_t ) );

            for( size_t iGlyph = 0; iGlyph < glyphCount; iGlyph++ ) {
                // Read gid.
                uint16_t gid = 0;
                file.read( (char *)&gid, sizeof( uint16_t ) );
                // Read is renderable.
                bool isRenderable = false;
                file.read( (char *)&isRenderable, sizeof( bool ) );
                // Read width and height.
                size_t width = 0, height = 0;
                file.read( (char *)&width, sizeof( size_t ) );
                file.read( (char *)&height, sizeof( size_t ) );
                // Read correction.
                Vec2f correction{0.0f};
                file.read( (char *)&correction, sizeof( Vec2f ) );
                // Read vertex positions.
                array<Vec2f, 4> positions;
                file.read( (char *)&positions, 4 * sizeof( Vec2f ) );
                // Read pixel size and pixels.
                size_t pixelSize = 0;
                file.read( (char *)&pixelSize, sizeof( size_t ) );
                uint8_t *pixels = new uint8_t[pixelSize];
                file.read( (char *)pixels, pixelSize * sizeof( uint8_t ) );

                AtlasGlyph glyph;
                glyph.m_isRenderable = isRenderable;

                if( isRenderable ) {
                    glyph.m_width = width;
                    glyph.m_height = height;
                    glyph.m_correction = correction;
                    glyph.m_position[0] = positions[0];
                    glyph.m_position[1] = positions[1];
                    glyph.m_position[2] = positions[2];
                    glyph.m_position[3] = positions[3];
                    glyph.m_pixelSize = pixelSize;
                    glyph.m_pixels = pixels;

                    // Move pen to next line, if line is full.
                    if( m_atlasPen.x + width >= m_atlasWidth ) {
                        m_atlasPen.y += m_atlasLineHeight;
                        m_atlasPen.x = 0;
                    }

                    const auto uMin = (float)m_atlasPen.x / (float)m_atlasWidth;
                    const auto vMin = (float)m_atlasPen.y / (float)m_atlasHeight;
                    const auto uMax = float( m_atlasPen.x + width ) / (float)m_atlasWidth;
                    const auto vMax = float( m_atlasPen.y + height ) / (float)m_atlasHeight;

                    glyph.m_uv[0] = {uMin, vMin, 0.0f};
                    glyph.m_uv[1] = {uMax, vMin, 0.0f};
                    glyph.m_uv[2] = {uMax, vMax, 0.0f};
                    glyph.m_uv[3] = {uMin, vMax, 0.0f};

                    assertf( pixels );
                    m_atlasLineHeight = max( m_atlasLineHeight, height );
                    m_atlasTexture.bind();
                    m_atlasTexture.updateSubImage( 0,
                                                   m_atlasPen.x,
                                                   m_atlasPen.y,
                                                   0,
                                                   width,
                                                   height,
                                                   1,
                                                   TextureArray2D::PixelFormat::Red,
                                                   TextureArray2D::PixelType::UnsignedByte,
                                                   pixels );
                    m_atlasPen.x += width;
                }

                const auto glyphExists = table->find( gid ) != table->end();
                assertf( !glyphExists );

                table->insert( {gid, glyph} );
            }
        }
    */
        return true;
    }

    // -- saveCacheToFile function --
    bool FontCache::saveCacheToFile( fs::path const &filePath ) {

        /* 
        console << "Writing font cache to file..." << endl;
        ofstream file;
        file.open( filePath.c_str(), std::ios::out | std::ios::binary );

        const auto facesCount = m_fontManager.getFontFacesCount();

        console << "Writing " << facesCount << " faces to file" << endl;
        file.write( (char *)&facesCount, sizeof( size_t ) );

        auto &faceTable = m_fontManager.getFontFaceTable();
        for( auto it = faceTable.begin(); it != faceTable.end(); it++ ) {
            auto name = it->second.getName();
            auto glyphTableSearch = m_glyphTables.find( name );

            if( glyphTableSearch != m_glyphTables.end() ) {
                file.write( (char *)&name, sizeof( StringId ) );

                console << "name= " << lookUpIdName( name ) << endl;

                assert( glyphTableSearch != m_glyphTables.end() );
                //

                auto glyphTable = &glyphTableSearch->second;
                auto glyphCount = glyphTable->size();
                file.write( (char *)&glyphCount, sizeof( size_t ) );
                for( auto itGlyph = glyphTable->begin();
                     itGlyph != glyphTable->end();
                     itGlyph++ ) {
                    auto gid = itGlyph->first;
                    auto glyph = itGlyph->second;

                    // Write gid
                    file.write( (char *)&gid, sizeof( uint16_t ) );
                    // Write is renderable property
                    file.write( (char *)&glyph.m_isRenderable, sizeof( bool ) );
                    // Write width and height
                    file.write( (char *)&glyph.m_width, sizeof( size_t ) );
                    file.write( (char *)&glyph.m_height, sizeof( size_t ) );
                    // Write correction
                    file.write( (char *)&glyph.m_correction, sizeof( Vec2f ) );
                    // Write vertex positions
                    file.write( (char *)&glyph.m_position, 4 * sizeof( Vec2f ) );
                    // Write pixel size and pixels
                    file.write( (char *)&glyph.m_pixelSize, sizeof( size_t ) );
                    file.write( (char *)glyph.m_pixels,
                                sizeof( uint8_t ) * glyph.m_pixelSize );
                }
            }
        }

        file.close();
        */
        return true;
    }

    // -- getGlyph functionn --
    weak_ptr<AtlasGlyph> FontCache::getGlyph( const uint16_t gid, const StringId fontFaceId ) {

        //        console << "Requesting glyph gid=" << gid << endl;

        // Get font face from font manager.
        auto const &fontFace = m_fontManager.getFontFace( fontFaceId );

        // Find glyph table for font face, create if it doesn't exist.
        auto glyphTableSearch = m_glyphTables.find( fontFaceId );
        const auto glyphTableFound = glyphTableSearch != m_glyphTables.end();
        if( !glyphTableFound ) {
            m_glyphTables.insert( {fontFaceId, make_unique<GlyphTable>()} );
            glyphTableSearch = m_glyphTables.find( fontFaceId );
            assertf( glyphTableSearch != m_glyphTables.end() );
            //            glyphTableSearch = m_glyphTables.insert( {fontFaceId, GlyphTable()} ).first;
        }

        auto &glyphTable = *glyphTableSearch->second;

        // Search for glyph in font face glyph table.
        auto glyphSearch = glyphTable.find( gid );
        const auto glyphFound = glyphSearch != glyphTable.end();

        // Case 1: glyph found.
        if( glyphFound ) {
            return glyphSearch->second;
            //            console << "Case1: glyph gid=" << gid << " found in cache." << endl;
        }

        // Case 2: glyph not found
        else {
            //            console << "Case2: glyph gid=" << gid << " not found in cache." << endl;
            auto correction = 0.0f;
            auto pixels = OutlineExtractor::computeGlyphSDF( fontFace,
                                                             gid,
                                                             correction,
                                                             m_glyphScale,
                                                             m_glyphPadding );
            //            glyphSearch = glyphTable.insert( {gid, make_shared<AtlasGlyph>()} ).first;

            // Insert new glyph into table.
            glyphTable.insert( {gid, make_shared<AtlasGlyph>()} );
            glyphSearch = glyphTable.find( gid );
            assertf( glyphSearch != glyphTable.end() );

            auto &glyph = *glyphSearch->second;

            glyph.m_codepoint = gid;

            //            console << "added glyph gid=" << glyph.m_codepoint << " to glyph table." << endl;

            if( !pixels ) {
                glyph.m_isRenderable = false;
            } else {
                glyph.m_isRenderable = true;

                // Move pen to next line, if line is full.
                if( m_atlasPen.x + pixels->getWidth() >= m_atlasWidth ) {
                    m_atlasPen.y += m_atlasLineHeight;
                    m_atlasPen.x = 0;
                }

                // Scale glyph width up to multiple of 4 for pixel padding.
                const auto newWidth = ( pixels->getWidth() + 4 - 1 ) & ~( 4 - 1 );
                const auto newHeight = pixels->getHeight();
                auto pixels2 = new uint8_t[newWidth * newHeight]{0};
                for( size_t y = 0; y < pixels->getHeight(); y++ ) {
                    for( size_t x = 0; x < pixels->getWidth(); x++ ) {
                        const auto i = y * newWidth + x;
                        pixels2[i] = pixels->getAt( x, y );
                    }
                }

                /// @todo Make sure `pixels` is cleaned up.

                glyph.m_width = pixels->getWidth();
                glyph.m_height = pixels->getHeight();
                glyph.m_correction = {0.0f, correction};
                glyph.m_pixels = pixels2;
                glyph.m_pixelSize = newWidth * newHeight;

                const auto uMin = (float)m_atlasPen.x / (float)m_atlasWidth;
                const auto vMin = (float)m_atlasPen.y / (float)m_atlasHeight;
                const auto uMax =
                    float( m_atlasPen.x + glyph.m_width ) / (float)m_atlasWidth;
                const auto vMax =
                    float( m_atlasPen.y + glyph.m_height ) / (float)m_atlasHeight;
                const auto w = (float)glyph.m_width * 0.035f;
                const auto h = (float)glyph.m_height * 0.035f;
                glyph.m_uv[0] = {uMin, vMin, 0.0f};
                glyph.m_uv[1] = {uMax, vMin, 0.0f};
                glyph.m_uv[2] = {uMax, vMax, 0.0f};
                glyph.m_uv[3] = {uMin, vMax, 0.0f};
                glyph.m_position[0] = {0.0f, 0.0f};
                glyph.m_position[1] = {w, 0.0f};
                glyph.m_position[2] = {w, h};
                glyph.m_position[3] = {0.0f, h};

                m_atlasLineHeight = max( m_atlasLineHeight, pixels->getHeight() );
                m_atlasTexture.updateSubImage(
                    0,
                    m_atlasPen.x,
                    m_atlasPen.y,
                    0,
                    pixels->getWidth(),
                    pixels->getHeight(),
                    1,
                    TextureArray2D::PixelFormat::Red,
                    TextureArray2D::PixelType::UnsignedByte,
                    pixels2 );
                m_atlasPen.x += pixels->getWidth();
            }

            assertf( glyph.m_codepoint == gid );

            return glyphSearch->second;
        }
    }

}  // namespace arc
