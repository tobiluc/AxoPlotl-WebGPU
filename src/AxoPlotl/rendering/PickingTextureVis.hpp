#pragma once

#include <AxoPlotl/rendering/renderers/RendererBase.hpp>
#include <webgpu/webgpu.hpp>

namespace AxoPlotl::internal
{

/*
Responsible for rendering the picking texture to a visually interpretable texture.
This is used for debugging purposes or also to set the mask for transparent background in screenshots.s
*/
class PickingTextureVis
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

    void draw_ui();

    const wgpu::Texture& texture() const {return output_texture;}
    const wgpu::TextureView& texture_view() const {return output_view;}

    uint32_t width() const {return width_;}
    uint32_t height() const {return height_;}

    Mode& mode() {return uniforms_.mode;}

private:

    struct Uniforms {
        alignas(16) Mode mode = Mode::ObjectID;
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
