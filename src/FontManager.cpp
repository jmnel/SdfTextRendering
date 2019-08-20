#include <Console/Console.hpp>
#include <Text/FontLibrary.hpp>
#include <Utility/Assert.hpp>

#include <Text/FontManager.hpp>

namespace arc {

    // -- Constructor --
    FontManager::FontManager( IFontLibrary& fontLibrary )
        : m_fontLibrary( fontLibrary ) {
    }

    // -- Destructor --
    FontManager::~FontManager() {
    }

    // -- setFontSearchPath function --
    void FontManager::setFontSearchPath( fs::path const& fontSearchPath ) {
        const auto pathExists = fs::exists( fontSearchPath );
        massertf( pathExists, "Font search path does not exist." );
        if( !pathExists ) {
            abort();
        }

        m_fontSearchPath = fontSearchPath;
    }

    // -- loadFontFace function --
    void FontManager::loadFontFace( const StringId name, string const& file ) {
        const auto fontFacePath = m_fontSearchPath / file;
        const auto fileExists = fs::exists( fontFacePath );
        massertf( fileExists, "Font face file: `" + fontFacePath + "` does not exist." );
        if( !fileExists ) {
            abort();
        }

        const auto faceSearch = m_fontFaces.find( name );

        const auto faceFound = faceSearch != m_fontFaces.end();
        massertf( !faceFound, "Font face with name `" + lookupIdName( name ) + "` already loaded." );
        if( faceFound ) {
            abort();
        }

        const auto absPath = m_fontSearchPath / file;

        const auto fileFound = fs::exists( absPath );
        assertf( fileFound );

        auto newFontResult = static_cast<FontLibrary&>( m_fontLibrary ).initailizeFace( name, absPath ).unwrap();

        m_fontFaces.insert_or_assign( name, std::move( newFontResult ) );

        //        match( newFontResult,
        //               [this, name]( Ok<FontFace> const& ok ) -> void {
        //                   m_fontFaces.insert( std::make_pair( name, ok.getValue() ) );
        //               },
        //               []( Error<string> const& err ) -> void {
        //                   cout << "Error: " << err.getError() << endl;
        //               } );

        //        if( newFontResult.isError() ) { }
        //        auto newFont = newFontResult.unwrap();

        //        m_fontLibary.initializeFaceFt(
    }

    // -- getFontSearchPath function --
    fs::path const& FontManager::getFontSearchPath() const {
        return m_fontSearchPath;
    }

    // -- getFontFace function --
    FontFace const& FontManager::getFontFace( const StringId name ) const {
        auto fontFaceSearch = m_fontFaces.find( name );
        const auto fontFaceFound = fontFaceSearch != m_fontFaces.end();
        //        console << lookUpIdName( name ) << endl;
        massertf( fontFaceFound, "Font face `" + lookUpIdName( name ) + "` not found." );
        if( !fontFaceFound ) {
            abort();
        }
        return fontFaceSearch->second;
    }

    // -- getFontFacesCount function --
    size_t FontManager::getFontFacesCount() const {
        return m_fontFaces.size();
    }

    // -- getFontFaceTable function --
    unordered_map<StringId, FontFace> const& FontManager::getFontFaceTable() const {
        return m_fontFaces;
    }

}  // namespace arc
