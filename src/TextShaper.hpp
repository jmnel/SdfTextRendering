#pragma once

#include <memory>
#include <string>
#include <vector>

//#include <GUI/Properties.hpp>
#include <GUI/Style.hpp>
#include <Math/Vec.hpp>
#include <RenderPipelines/VertexTypes/SdfVertex.hpp>

using std::shared_ptr;
using std::string;
using std::vector;
using std::weak_ptr;

namespace arc {

struct AtlasGlyph;
class FontCache;
class FontManager;

class TextShaper {
private:
  FontCache &m_fontCache;
  FontManager &m_fontManager;

  struct LayoutGlyph {
    weak_ptr<AtlasGlyph> m_glyphWeak;
    Vec2f m_position;
    Vec2f m_advance;
    Vec2f m_correction;
    LayoutGlyph(weak_ptr<AtlasGlyph> glyphWeak, Vec2f const &position)
        : m_glyphWeak(glyphWeak), m_position(position){};
  };

  struct LayoutLine {
    string m_text;
    //            gui::Properties m_properties;
    gui::Style m_style;
    vector<LayoutGlyph> m_glyphs;

    float m_width = 0.0f;
    float m_height = 0.0f;

    LayoutLine(string const &text, gui::Style const &style)
        : m_text(text), m_style(style){};

    //            LayoutLine( string const& text, gui::Properties const&
    //            properties )
    //                : m_text( text ), m_properties( properties ){};
  };

  Vec2f m_origin;
  Vec2f m_box;

  vector<LayoutLine> m_lines;
  const float advConst = 0.0005f * 0.1f;
  vector<SdfVertex> m_vertices;

public:
  TextShaper(FontCache &fontCache, FontManager &fontManager);

  vector<SdfVertex> &getVertices();
  //        void addText( string const& text, gui::Properties const& properties
  //        );
  void addText(string const &text, gui::Style const &style);
  void setBox(Vec2f const &boxSize);
  void setOrigin(Vec2f const &origin);
  void shape();

private:
  void splitNewlines();
};

} // namespace arc
