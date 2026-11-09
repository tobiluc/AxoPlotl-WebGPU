// #pragma once

// #include "AxoPlotl/rendering/RendererBase.hpp"
// #include <webgpu/webgpu.hpp>

// namespace AxoPlotl
// {

// class PickingTextureRenderer
// {
// public:

//     enum class Mode : uint32_t
//     {
//         ObjectID = 0,
//         EntityType = 1,
//         EntityIndex = 2,
//         Depth = 3
//     };

//     bool initialize(
//         wgpu::Device& device,
//         uint32_t width,
//         uint32_t height
//         );

//     void render(
//         wgpu::CommandEncoder& encoder,
//         wgpu::TextureView& picking_texture,
//         Mode mode = Mode::ObjectID
//         );

//     wgpu::TextureView& get_texture_view() {
//         return output_view;
//     }

// private:

//     struct Uniforms {
//         alignas(16) uint32_t mode = 0;
//     };

//     wgpu::Texture output_texture;
//     wgpu::TextureView output_view;

//     wgpu::ShaderModule shader;
//     wgpu::BindGroupLayout bind_group_layout;

//     wgpu::Buffer uniform_buffer;

//     static PipelineState pipeline_state_;

//     uint32_t width = 0;
//     uint32_t height = 0;

//     bool initialized = false;
// };

// void draw_texture(const char* _name,
//                   wgpu::TextureView& _view,
//                   float _width = 0, float _height = 0);

// }
