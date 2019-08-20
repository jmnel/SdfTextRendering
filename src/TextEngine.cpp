#include <iostream>
#include <vector>

#include <Console/Console.hpp>
//#include <GUI/Properties.hpp>
#include <GUI/Style.hpp>
#include <Math/FunctionsMatrix.hpp>
#include <RenderPipelines/RenderView.hpp>
#include <Shaders/Shader.hpp>
#include <Text/TextShaper.hpp>
#include <Utility/Assert.hpp>

#include <Text/TextEngine.hpp>

// using std::cout;
using std::endl;
using std::vector;

namespace arc {

// -- Constructor -
TextEngine::TextEngine(Shader &sdfShader, RenderView const &renderView)
    : m_sdfShader(sdfShader), m_renderView(renderView),
      m_fontLibary(FontLibrary::initalize().unwrap()),
      m_fontManager(m_fontLibary), m_sdfPipeline(m_sdfShader),
      m_fontCache(m_fontManager) {
  //          m_textRenderer( m_sdfPipeline ) {
  console << "Text engine initialized." << endl;

  m_fontManager.setFontSearchPath("***redacted***");
  m_fontManager.loadFontFace(internString("FiraCodeRetina"),
                             "FiraCode-Retina.ttf");
  m_fontManager.loadFontFace(internString("RobotoThin"), "Roboto-Thin.ttf");

  //        m_fontCache.loadCacheFromFile( m_fontCacheLocation );

  //        m_atlasTexture = new gl::TextureArray2D(
  //        TextureArray2dGL::Type::TextureArray2D );
}

// -- Destructor --
TextEngine::~TextEngine() {
  //        m_fontCache.saveCacheToFile( m_fontCacheLocation );
}

// -- generateSdfVertices function --
vector<SdfVertex> TextEngine::generateSdfVertices(
    string const &text, Vec2f const &position, Vec2f const &boxSize,
    //                                                       gui::Properties
    //                                                       const &properties )
    //                                                       {
    gui::Style const &style) {

  TextShaper textShaper(m_fontCache, m_fontManager);
  textShaper.setOrigin(position);
  textShaper.setBox(boxSize);
  textShaper.addText(text, style);
  textShaper.shape();

  auto &vertices = textShaper.getVertices();
  assertf(!vertices.empty());
  return vertices;
}

// -- draw function --
void TextEngine::draw() {

  m_fontCache.getAtlasTexture().bind();
  gui::Style style;
  //        gui::Properties props;
  //        props.m_fontFace = SID( "FiraCodeRetina" );
  //        props.m_wordWrap = gui::WordWrap::Off;
  //        auto v = generateSdfVertices( "abcd",
  //                                      Vec2f{0.0f},
  //                                      Vec2f{1.0f, 0.5f},
  //                                      props );

  m_sdfShader.use();
  //        const float ratio = m_renderV
  const auto ratio = m_renderView.getAspectRatio();
  const auto mvp = createOrtho(-1.0f * ratio, 1.0f * ratio, -1.0f, 1.0f);
  m_sdfShader.setUniformMat4x4f("mvp", mvp);
  m_sdfShader.setUniform1i("sampler", 0);
  //        m_sdfPipeline.submit( v );

  //        const auto s = 0.1f;
  //        vector<SdfVertex> vertices{
  //            {Vec3f{0.0f}, Vec3f{0.0f}, White, 0.0f, 0.0f},
  //            {Vec3f{s, 0.0f, 0.0f}, Vec3f{0.0f}, White, 0.0f, 0.0f},
  //            {Vec3f{s, s, 0.0f}, Vec3f{0.0f}, White, 0.0f, 0.0f}};

  //        m_sdfPipeline.submit( vertices );
}

} // namespace arc
