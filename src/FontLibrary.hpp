#pragma once

#include <filesystem>
#include <string>

#include <Utility/Result.hpp>

#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

//#include <harfbuzz/hb-ft.h>

#include <Text/FontFace.hpp>

#include <Text/IFontLibrary.hpp>

using std::string;
namespace fs = std::filesystem;

namespace arc {

    class FontLibrary : public IFontLibrary {
    private:
        FT_Library m_ftLibrary = nullptr;

        const float ftCharacterSize = 64.0f;

    public:
        FontLibrary();
        FontLibrary( FontLibrary const & ) = delete;
        FontLibrary( FontLibrary &&other );
        ~FontLibrary() final;

        FontLibrary &operator=( FontLibrary const & ) = delete;
        FontLibrary &operator=( FontLibrary &&other );

        static Result<FontLibrary, string> initalize();

        Result<FontFace, string> initailizeFace( const StringId name, fs::path const &file );
    };

}  // namespace arc
