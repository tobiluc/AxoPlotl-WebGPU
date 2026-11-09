// #include "draw_texture.hpp"
// #include "AxoPlotl/rendering/detail/wgpu_commons.hpp"
// #include <imgui.h>
// #include <array>

// namespace AxoPlotl
// {

// PipelineState PickingTextureRenderer::pipeline_state_;

// void draw_texture(const char* _name, wgpu::TextureView& _view, float _width, float _height)
// {
//     if (!_view) {return;}

//     ImGui::Begin(_name);

//     ImVec2 size = {_width, _height};
//     if (_width <= 0.0f || _height <= 0.0f) {
//         size = ImGui::GetContentRegionAvail();
//     }

//     WGPUTextureView raw_view = _view;

//     ImGui::Image(
//         (ImTextureID)(intptr_t)raw_view,
//         size
//     );

//     ImGui::End();
// }

// bool PickingTextureRenderer::initialize(
//     wgpu::Device& device,
//     uint32_t width,
//     uint32_t height)
// {
//     this->pipeline_state_.device_ = device;
//     this->width = width;
//     this->height = height;

//     // Output texture.

//     wgpu::TextureDescriptor texture_desc{};
//     texture_desc.label = wgpu::StringView("Picking visualization texture");
//     texture_desc.size = {
//         width,
//         height,
//         1
//     };
//     texture_desc.mipLevelCount = 1;
//     texture_desc.sampleCount = 1;
//     texture_desc.format = wgpu::TextureFormat::RGBA8Unorm;
//     texture_desc.usage =
//         wgpu::TextureUsage::RenderAttachment |
//         wgpu::TextureUsage::TextureBinding;
//     output_texture = device.createTexture(texture_desc);

//     output_view = output_texture.createView();

//     // Shader module.
//     wgpu::ShaderModule shaderModule = create_mesh_shader_module_from_file(
//         device,
//         "picking_vis_shader.wgsl",
//         "Picking Visualization Shader");

//     // Bind group layout.
//     std::array<wgpu::BindGroupLayoutEntry, 2> entries{};

//     entries[0].binding = 0;
//     entries[0].visibility = wgpu::ShaderStage::Fragment;
//     entries[0].texture.sampleType =
//         wgpu::TextureSampleType::Uint;
//     entries[0].texture.viewDimension =
//         wgpu::TextureViewDimension::_2D;

//     entries[1].binding = 1;
//     entries[1].visibility = wgpu::ShaderStage::Fragment;
//     entries[1].buffer.type =
//         wgpu::BufferBindingType::Uniform;
//     entries[1].buffer.minBindingSize = sizeof(Uniforms);

//     wgpu::BindGroupLayoutDescriptor bgl_desc{};
//     bgl_desc.entryCount = entries.size();
//     bgl_desc.entries = entries.data();

//     bind_group_layout =
//         device.createBindGroupLayout(bgl_desc);

//     // Pipeline layout.

//     wgpu::PipelineLayoutDescriptor pl_desc{};
//     pl_desc.bindGroupLayoutCount = 1;
//     pl_desc.bindGroupLayouts = reinterpret_cast<WGPUBindGroupLayout*>(&pipeline_state_.bind_group_layout_);

//     wgpu::PipelineLayout pipelineLayout =
//         device.createPipelineLayout(pl_desc);

//     // Render pipeline.

//     wgpu::ColorTargetState color_target{};
//     color_target.format = wgpu::TextureFormat::RGBA8Unorm;
//     color_target.writeMask = wgpu::ColorWriteMask::All;

//     wgpu::FragmentState fragment{};
//     fragment.module = shader;
//     fragment.entryPoint = wgpu::StringView("fs_main");
//     fragment.targetCount = 1;
//     fragment.targets = &color_target;

//     wgpu::RenderPipelineDescriptor pipeline_desc{};
//     pipeline_desc.layout = pipelineLayout;

//     pipeline_desc.vertex.module = shader;
//     pipeline_desc.vertex.entryPoint = wgpu::StringView("vs_main");

//     pipeline_desc.primitive.topology =
//         wgpu::PrimitiveTopology::TriangleList;

//     pipeline_desc.fragment = &fragment;

//     pipeline_state_.pipeline_ = device.createRenderPipeline(pipeline_desc);

//     // Uniform buffer.

//     wgpu::BufferDescriptor buffer_desc{};
//     buffer_desc.label = wgpu::StringView("Picking visualization uniforms");
//     buffer_desc.size = sizeof(Uniforms);
//     buffer_desc.usage =
//         wgpu::BufferUsage::Uniform |
//         wgpu::BufferUsage::CopyDst;

//     uniform_buffer = device.createBuffer(buffer_desc);

//     initialized = pipeline_state_.pipeline_ && output_view && uniform_buffer;

//     return initialized;
// }

// void PickingTextureRenderer::render(
//     wgpu::CommandEncoder& encoder,
//     wgpu::TextureView& picking_texture,
//     Mode mode)
// {
//     if (!initialized || !picking_texture)
//         return;

//     // Update visualization mode.

//     Uniforms uniforms{};
//     uniforms.mode = static_cast<uint32_t>(mode);

//     pipeline_state_.device_.getQueue().writeBuffer(
//         uniform_buffer,
//         0,
//         &uniforms,
//         sizeof(uniforms)
//         );

//     // Bind group.

//     std::array<wgpu::BindGroupEntry, 2> entries{};

//     entries[0].binding = 0;
//     entries[0].textureView = picking_texture;

//     entries[1].binding = 1;
//     entries[1].buffer = uniform_buffer;
//     entries[1].offset = 0;
//     entries[1].size = sizeof(Uniforms);

//     wgpu::BindGroupDescriptor bg_desc{};
//     bg_desc.layout = bind_group_layout;
//     bg_desc.entryCount = entries.size();
//     bg_desc.entries = entries.data();

//     wgpu::BindGroup bind_group =
//         pipeline_state_.device_.createBindGroup(bg_desc);

//     // Render pass.

//     wgpu::RenderPassColorAttachment color_attachment{};
//     color_attachment.view = output_view;
//     color_attachment.loadOp = wgpu::LoadOp::Clear;
//     color_attachment.storeOp = wgpu::StoreOp::Store;
//     color_attachment.clearValue = {
//         0.0, 0.0, 0.0, 1.0
//     };

//     wgpu::RenderPassDescriptor pass_desc{};
//     pass_desc.colorAttachmentCount = 1;
//     pass_desc.colorAttachments = &color_attachment;

//     wgpu::RenderPassEncoder pass =
//         encoder.beginRenderPass(pass_desc);

//     pass.setPipeline(pipeline_state_.pipeline_);
//     pass.setBindGroup(0, bind_group, 0, nullptr);
//     pass.draw(3, 1, 0, 0);
//     pass.end();
// }


// }
