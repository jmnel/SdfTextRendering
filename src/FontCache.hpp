#pragma once

#include <filesystem>
#include <memory>
#include <unordered_map>

#include <GLWrapper/TextureArray2D.hpp>
#include <Text/AtlasGlyph.hpp>
#include <Utility/StringId.hpp>

using std::shared_ptr;
using std::unique_ptr;
using std::unordered_map;
using std::weak_ptr;
namespace fs = std::filesystem;

namespace arc {

    class FontManager;

    class FontCache {
    private:
        FontManager const &m_fontManager;
        // FontLibrary

        gl::TextureArray2D m_atlasTexture;

        const size_t m_atlasWidth = 1024;
        const size_t m_atlasHeight = 1024;
        size_t m_atlasLineHeight = 0;
        static const size_t m_atlasLayers = 2;
        const size_t m_atlasMipLevelCount = 1;

        const double m_glyphScale = 24.0;
        const int m_glyphPadding = 12;

        Vec<size_t, 2> m_atlasPen = {0};
        size_t m_currentLayer = 0;

        using GlyphTable = unordered_map<uint16_t, shared_ptr<AtlasGlyph>>;
        unordered_map<StringId, unique_ptr<GlyphTable>> m_glyphTables;

    public:
        FontCache( FontManager const &fontManager );
        ~FontCache();

        bool loadCacheFromFile( fs::path const &filePath );
        bool saveCacheToFile( fs::path const &filePath );

        weak_ptr<AtlasGlyph> getGlyph( const uint16_t gid, const StringId fontFaceId );

        gl::TextureArray2D &getAtlasTexture();
    };

}  // namespace arc
