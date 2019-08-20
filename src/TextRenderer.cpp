#include <iostream>

#include <Text/TextRenderer.hpp>

using std::cout;
using std::endl;

namespace arc {

    // -- Constructor --
    TextRenderer::TextRenderer( PipelineSdf& sdfPipeline )
        : m_sdfPipeline( sdfPipeline ) {

        cout << "Info: TextRenderer initialized." << endl;
    }

    // -- Destructor --
    TextRenderer::~TextRenderer() {
    }

};  // namespace arc
