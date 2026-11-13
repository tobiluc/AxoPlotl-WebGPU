#include "Matrix3x3Renderer.hpp"
#include "AxoPlotl/rendering/wgpu_commons.hpp"

namespace AxoPlotl::internal
{

PipelineState Matrix3x3Renderer::pipeline_state_;

void Matrix3x3Renderer::init(uint32_t _object_id,
                             const GpuContext& _gpu_context,
                             const RenderContext& _render_context,
                           wgpu::Buffer _position_buffer)
{
    pipeline_state_.set_device(_gpu_context.device);
    clear();

    object_id_ = _object_id;
    position_buffer_ = _position_buffer;
    n_positions_ = _position_buffer.getSize()/sizeof(Position);
    create_buffers();
    create_bind_group_layout();
    create_bind_group();
    create_pipeline(_gpu_context, _render_context);

    initialized_ = true;
}

void Matrix3x3Renderer::clear()
{
    destroy_buffer(matrix_buffer_);
    destroy_buffer(uniform_buffer_);
    initialized_ = false;
}

void Matrix3x3Renderer::create_buffers()
{
    wgpu::Device device = pipeline_state_.device_;
    // wgpu::Queue queue = device.getQueue();

    // Matrix Buffer
    {
        wgpu::BufferDescriptor desc{};
        desc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
        desc.size = sizeof(glm::mat3x4) * std::max(n_positions_,1lu);
        desc.mappedAtCreation = false;
        desc.label = wgpu::StringView("Matrix Buffer");

        matrix_buffer_ = device.createBuffer(desc);
    }

    // Uniform Buffer
    {
        compute_uniform_stride<Uniforms>(device);

        wgpu::BufferDescriptor desc{};
        desc.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
        desc.size = uniform_stride_ * max_draw_calls_;
        desc.mappedAtCreation = false;
        desc.label = wgpu::StringView("Matrix3x3 Uniform Buffer");

        uniform_buffer_ = device.createBuffer(desc);
    }
}

void Matrix3x3Renderer::create_bind_group_layout()
{
    if (pipeline_state_.bind_group_layout_) {return;}

    wgpu::BindGroupLayoutEntry entries[3]{};

    // 0 - Uniform
    entries[0].binding = 0;
    entries[0].visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
    entries[0].buffer.type = wgpu::BufferBindingType::Uniform;
    entries[0].buffer.minBindingSize = sizeof(Uniforms);
    entries[0].buffer.hasDynamicOffset = true;

    // 1 - Positions
    entries[1].binding = 1;
    entries[1].visibility = wgpu::ShaderStage::Vertex;
    entries[1].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
    entries[1].buffer.minBindingSize = sizeof(Position);

    // 2 - Matrices
    entries[2].binding = 2;
    entries[2].visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
    entries[2].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
    entries[2].buffer.minBindingSize = sizeof(glm::mat3x4);

    wgpu::BindGroupLayoutDescriptor layoutDesc{};
    layoutDesc.entryCount = 3;
    layoutDesc.entries = entries;
    layoutDesc.label = wgpu::StringView("Matrix3x3 Bind Group Layout");

    pipeline_state_.bind_group_layout_ = pipeline_state_.device_.createBindGroupLayout(layoutDesc);
}

void Matrix3x3Renderer::create_bind_group()
{
    wgpu::BindGroupEntry groupEntries[3]{};

    // 0 - Uniform
    groupEntries[0].binding = 0;
    groupEntries[0].buffer = uniform_buffer_;
    groupEntries[0].offset = 0;
    groupEntries[0].size = sizeof(Uniforms);

    // 1 - Positions
    groupEntries[1].binding = 1;
    groupEntries[1].buffer = position_buffer_;
    groupEntries[1].offset = 0;
    groupEntries[1].size = sizeof(Position) * std::max(n_positions_,1lu);

    // 2 - Matrices
    groupEntries[2].binding = 2;
    groupEntries[2].buffer = matrix_buffer_;
    groupEntries[2].offset = 0;
    groupEntries[2].size = sizeof(glm::mat3x4) * std::max(n_positions_,1lu);

    wgpu::BindGroupDescriptor bgDesc{};
    bgDesc.layout = pipeline_state_.bind_group_layout_;
    bgDesc.entryCount = 3;
    bgDesc.entries = groupEntries;

    bind_group_ = pipeline_state_.device_.createBindGroup(bgDesc);
}

void Matrix3x3Renderer::create_pipeline(const GpuContext& _gpu_context, const RenderContext &_render_context)
{
    if (pipeline_state_.pipeline_ || n_positions_==0) {return;}

    wgpu::ShaderModule shaderModule = create_mesh_shader_module_from_file(
        _gpu_context.device,
        "mat3x3_shader.wgsl",
        "Matrix3x3 Shader");

    // Vertex state (no vertex buffer)
    wgpu::VertexState vertexState{};
    vertexState.module = shaderModule;
    vertexState.entryPoint = wgpu::StringView("vs_main");
    vertexState.bufferCount = 0;
    vertexState.buffers = nullptr;

    // ---------------
    // Fragment state
    //-----------------
    wgpu::ColorTargetState color_targets[2] = {};
    wgpu::FragmentState fragmentState = create_fragment_state(_render_context, shaderModule, color_targets);

    // Primitive state
    wgpu::PrimitiveState primitive{};
    // primitive.topology = wgpu::PrimitiveTopology::LineList;
    primitive.topology = wgpu::PrimitiveTopology::TriangleStrip;
    primitive.frontFace = wgpu::FrontFace::CCW;
    primitive.cullMode = wgpu::CullMode::None;

    // Multisample
    wgpu::MultisampleState multisample{};
    multisample.count = 1;
    multisample.mask = ~0u;
    multisample.alphaToCoverageEnabled = false;

    // Pipeline layout
    wgpu::PipelineLayoutDescriptor layoutDesc{};
    layoutDesc.bindGroupLayoutCount = 1;
    layoutDesc.bindGroupLayouts = reinterpret_cast<WGPUBindGroupLayout*>(&pipeline_state_.bind_group_layout_);

    // Pipeline
    wgpu::RenderPipelineDescriptor pipelineDesc{};
    wgpu::DepthStencilState depth = create_default_depth_state();
    pipelineDesc.depthStencil = &depth;
    pipelineDesc.layout = _gpu_context.device.createPipelineLayout(layoutDesc);
    pipelineDesc.vertex = vertexState;
    pipelineDesc.fragment = &fragmentState;
    pipelineDesc.primitive = primitive;
    pipelineDesc.multisample = multisample;
    pipelineDesc.label = wgpu::StringView("Matrix3x3 Pipeline");

    pipeline_state_.pipeline_ = _gpu_context.device.createRenderPipeline(pipelineDesc);
}

void Matrix3x3Renderer::update_matrix_data(const std::vector<glm::mat3x4> &_data)
{
    pipeline_state_.device_.getQueue().writeBuffer(
        matrix_buffer_,
        0,
        _data.data(),
        sizeof(glm::mat3x4) * _data.size()
        );
}

void Matrix3x3Renderer::render(
    const ViewSize& _size,
    wgpu::RenderPassEncoder _render_pass,
    const Mat4x4f& _mvp)
{
    if (!initialized_ || !enabled_ || n_positions_==0) {return;}

    const uint32_t offset = draw_index_ * uniform_stride_;

    // Update uniforms
    uniforms_.mvp_ = _mvp;
    uniforms_.viewport_size_ = {_size.width,_size.height};
    uniforms_.object_id_ = object_id_;
    pipeline_state_.device_.getQueue().writeBuffer(
        uniform_buffer_, offset, &uniforms_, sizeof(Uniforms));

    _render_pass.setPipeline(pipeline_state_.pipeline_);
    _render_pass.setBindGroup(0, bind_group_, 1, &offset);
    // _render_pass.draw(6, n_positions_, 0, 0);
    _render_pass.draw(4, n_positions_*3u, 0, 0);

    set_next_draw_index();
}

}
