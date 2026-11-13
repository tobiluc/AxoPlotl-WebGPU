#pragma once

#include <AxoPlotl/rendering/renderers/RendererBase.hpp>
#include <webgpu/webgpu.hpp>

namespace AxoPlotl::internal
{

class DebugVis_PickingTextureRenderer
{
public:

    enum class Mode : uint32_t
    {
        ObjectID = 0,
        EntityType = 1,
        EntityIndex = 2,
        Depth = 3
    };

    bool init(
        const wgpu::Device& device,
        uint32_t _width,
        uint32_t _height
        );

    void render(
        wgpu::CommandEncoder& encoder,
        const wgpu::TextureView& picking_texture);

    void resize(uint32_t width, uint32_t height);

    void render_ui();

private:

    struct Uniforms {
        alignas(16) uint32_t mode = static_cast<uint32_t>(Mode::ObjectID);
    } uniforms_;

    wgpu::Texture output_texture;
    wgpu::TextureView output_view;

    wgpu::ShaderModule shader;

    wgpu::Buffer uniform_buffer;

    static PipelineState pipeline_state_;

    uint32_t width_ = 0;
    uint32_t height_ = 0;

    bool initialized = false;
};

}
