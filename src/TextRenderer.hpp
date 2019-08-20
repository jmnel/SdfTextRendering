#pragma once

namespace arc {

    class PipelineSdf;

    class TextRenderer {
    private:
        PipelineSdf& m_sdfPipeline;

    public:
        TextRenderer( PipelineSdf& sdfPipeline );
        ~TextRenderer();
    };

}  // namespace arc
