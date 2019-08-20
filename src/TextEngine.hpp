#pragma once

#include <filesystem>
#include <string>
#include <vector>

//#define GLEW_STATIC
//#include <GL/glew.h>

#include <GLWrapper/TextureArray2D.hpp>
#include <GUI/Properties.hpp>
#include <RenderPipelines/PipelineSdf.hpp>
#include <Text/FontCache.hpp>
#include <Text/FontLibrary.hpp>
#include <Text/FontManager.hpp>
//#include <Text/TextRenderer.hpp>

using std::string;
using std::vector;
namespace fs = std::filesystem;

namespace arc {

class Shader;
class RenderView;
namespace gui {
class Style;
};

class TextEngine {
private:
  Shader &m_sdfShader;
  RenderView const &m_renderView;
  FontLibrary m_fontLibary;
  FontManager m_fontManager;
  PipelineSdf m_sdfPipeline;
  //        TextRenderer m_textRenderer;
  //

  FontCache m_fontCache;
  const fs::path m_fontCacheLocation = "***redacted***";
  //        gl::TextureArray2D *m_atlasTexture;

public:
  TextEngine(Shader &sdfShader, RenderView const &renderView);
  ~TextEngine();

  TextEngine(TextEngine const &) = delete;
  TextEngine &operator=(TextEngine const &) = delete;

  vector<SdfVertex> generateSdfVertices(
      string const &text, Vec2f const &position, Vec2f const &boxSize,
      //                                               gui::Properties const
      //                                               &properties );
      gui::Style const &style);

  void draw();
};

} // namespace arc
