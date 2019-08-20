#include <Console/Console.hpp>

#include <Text/FontLibrary.hpp>

namespace arc {

    // -- Constructor --
    FontLibrary::FontLibrary() {
    }

    // -- Move constructor --
    FontLibrary::FontLibrary( FontLibrary&& other )
        : m_ftLibrary( std::exchange( other.m_ftLibrary, nullptr ) ) {
    }

    // -- Destructor --
    FontLibrary::~FontLibrary() {
        if( m_ftLibrary ) {
            FT_Done_FreeType( m_ftLibrary );
        }
    }

    // -- Move assignment operator --
    FontLibrary& FontLibrary::operator=( FontLibrary&& other ) {
        m_ftLibrary = std::exchange( other.m_ftLibrary, nullptr );
        return *this;
    }

    // -- initialize function --
    Result<FontLibrary, string> FontLibrary::initalize() {
        console << "Initializing font library..." << endl;

        FT_Library ftLibrary = nullptr;
        auto ftInitRes = FT_Init_FreeType( &ftLibrary );

        if( ftInitRes == FT_Err_Ok ) {
            FontLibrary library;
            library.m_ftLibrary = ftLibrary;
            return Ok{std::move( library )};
        }
        return Error{string{"Failed to initialize Freetype2 library."}};
    }

    // -- initailizeFace function --
    Result<FontFace, string> FontLibrary::initailizeFace( const StringId name, fs::path const& file ) {
        FT_Face ftFace;

        //        cout
        //            << "Loading font `" << lookUpIdName( name ) << "` @" << file << "...";
        //        cout << endl;

        massertf( m_ftLibrary, "Invalid Freetype library handle." );

        const auto res =
            FT_New_Face( m_ftLibrary, file.c_str(), 0, &ftFace );

        if( res != FT_Err_Ok ) {
            return Error{
                string{"Error: Freetype could not load face from file `" + file.string() + "`."}};
        }

        FT_Set_Char_Size( ftFace, 64.0f, 64.0f, 0, 0 );

        const auto faceScale = 1.0f / ( (float)ftFace->units_per_EM * 0.001f );
        const auto lineHeight = 1.0f / ( (float)ftFace->height * 0.001f );

        // Create HarfBuzz font face.
        hb_font_t* hbFont = nullptr;
        hbFont = hb_ft_font_create( ftFace, nullptr );

        if( !hbFont ) {
            return Error{string{"Error: Failed to create HarfBuzz font face."}};
        }

        FontFace fontFace{name};
        fontFace.m_ftFace = ftFace;
        fontFace.m_faceScale = faceScale;
        fontFace.m_lineHeight = lineHeight;
        fontFace.m_hbFont = hbFont;

        return Ok{std::move( fontFace )};
    }

}  // namespace arc
