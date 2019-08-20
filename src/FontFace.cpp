#include <Utility/Assert.hpp>

#include <Text/FontFace.hpp>

namespace arc {

    // -- Constructor --
    FontFace::FontFace( const StringId name )
        : m_name( name ) {
    }

    // -- Move constructor --
    FontFace::FontFace( FontFace&& other )
        : m_name( other.m_name ),
          m_faceScale( other.m_faceScale ),
          m_lineHeight( other.m_lineHeight ),
          m_ftFace( other.m_ftFace ),
          m_hbFont( std::exchange( other.m_hbFont, nullptr ) ) {

        m_isValidFlag = other.m_isValidFlag;
        other.m_isValidFlag = false;
    }

    // -- Destructor --
    FontFace::~FontFace() {
        if( m_hbFont ) {
            hb_font_destroy( m_hbFont );
            m_hbFont = nullptr;
        }

        if( m_isValidFlag ) {
            FT_Done_Face( m_ftFace );
        }

        m_isValidFlag = false;
    }

    // -- Move assignment operator --
    FontFace& FontFace::operator=( FontFace&& other ) {
        if( &other == this ) {
            return *this;
        }

        assertf( m_name == other.m_name );

        m_ftFace = other.m_ftFace;
        m_hbFont = other.m_hbFont;
        m_faceScale = other.m_faceScale;
        m_lineHeight = other.m_lineHeight;
        m_isValidFlag = std::exchange( other.m_isValidFlag, false );

        return *this;
    }

    // -- getName function --
    StringId FontFace::getName() const {
        return m_name;
    }

}  // namespace arc
