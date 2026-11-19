#include "MeshEdgeRenderer.hpp"
#include "AxoPlotl/rendering/wgpu_commons.hpp"

namespace AxoPlotl::internal
{
PipelineState ColoredEdgeRenderer::pipeline_state_;

void ColoredEdgeRenderer::init(uint32_t _object_id, const GpuContext &_gpu_context, const RenderContext &_render_context,
                               const VolMeshData& _data)
{
    pipeline_state_.set_device(_gpu_context.device);

    object_id_ = _object_id;
    property_color_map_.create(_gpu_context.device);
    property_color_map_.set_coolwarm();

    n_edges_ = _data.mesh.n_edges();
    n_positions_ = _data.vertices_positions.data.size();

    if (n_edges_ == 0) {
        initialized_ = false;
        return;
    }

    position_buffer_ = _data.vertices_positions.buffer;

    edge_index_buffer_ = _data.edges_instances.buffer;
    create_buffers();
    create_bind_group_layout();
    create_bind_group();
    create_pipeline(_gpu_context, _render_context);

    initialized_ = true;
}

void ColoredEdgeRenderer::clear()
{
    destroy_buffer(property_buffer_);
    destroy_buffer(uniform_buffer_);
    initialized_ = false;
}

void ColoredEdgeRenderer::create_buffers()
{
    wgpu::Device device = pipeline_state_.device_;
    wgpu::Queue queue = device.getQueue();

    // Property Buffer
    {
        property_buffer_ = create_bufferT<Prop::Data>(
            device, n_edges_,
            wgpu::BufferUsage::Storage |
                wgpu::BufferUsage::CopyDst,
            "Mesh Edge Property Buffer");
    }

    // Uniform Buffer
    {
        compute_uniform_stride<Uniforms>(device);
        wgpu::BufferDescriptor desc;
        desc.size = uniform_stride_ * std::max(max_draw_calls_, 1u);
        desc.usage = wgpu::BufferUsage::Uniform |
                     wgpu::BufferUsage::CopyDst;
        desc.mappedAtCreation = false;
        desc.label = wgpu::StringView("Edge Uniform Buffer");
        uniform_buffer_ = device.createBuffer(desc);
    }
}

void ColoredEdgeRenderer::create_bind_group_layout()
{
    if (pipeline_state_.bind_group_layout_) {return;}

    wgpu::BindGroupLayoutEntry entries[5]{};

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

    // 2 - Color Map
    entries[2].binding = 2;
    entries[2].visibility = wgpu::ShaderStage::Fragment;
    entries[2].texture.sampleType = wgpu::TextureSampleType::Float;
    entries[2].texture.viewDimension = wgpu::TextureViewDimension::_2D;

    // 3 - Color Sampler
    entries[3].binding = 3;
    entries[3].visibility = wgpu::ShaderStage::Fragment;
    entries[3].sampler.type = wgpu::SamplerBindingType::Filtering;

    // 4 - Properties
    entries[4].binding = 4;
    entries[4].visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
    entries[4].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
    entries[4].buffer.minBindingSize = sizeof(Prop::Data);

    wgpu::BindGroupLayoutDescriptor layoutDesc{};
    layoutDesc.entryCount = 5;
    layoutDesc.entries = entries;
    layoutDesc.label = wgpu::StringView("Edge Bind Group Layout");

    pipeline_state_.bind_group_layout_ = pipeline_state_.device_.createBindGroupLayout(layoutDesc);
}

void ColoredEdgeRenderer::create_bind_group()
{
    wgpu::BindGroupEntry groupEntries[5]{};

    // 0 - Uniform
    groupEntries[0].binding = 0;
    groupEntries[0].buffer = uniform_buffer_;
    groupEntries[0].offset = 0;
    groupEntries[0].size = sizeof(Uniforms);

    // 1 - Positions
    groupEntries[1].binding = 1;
    groupEntries[1].buffer = position_buffer_;
    groupEntries[1].offset = 0;
    groupEntries[1].size = sizeof(Position) * n_positions_;

    // 2 - Property Color Map
    groupEntries[2].binding = 2;
    groupEntries[2].textureView = property_color_map_.view_;

    // 3 - Color Map Sampler
    groupEntries[3].binding = 3;
    groupEntries[3].sampler = property_color_map_.sampler_;

    // 4 - Properties
    groupEntries[4].binding = 4;
    groupEntries[4].buffer = property_buffer_;
    groupEntries[4].offset = 0;
    groupEntries[4].size = sizeof(Prop::Data) * std::max(n_edges_,1lu);

    wgpu::BindGroupDescriptor bgDesc{};
    bgDesc.layout = pipeline_state_.bind_group_layout_;
    bgDesc.entryCount = 5;
    bgDesc.entries = groupEntries;

    bind_group_ = pipeline_state_.device_.createBindGroup(bgDesc);
}

void ColoredEdgeRenderer::create_pipeline(const GpuContext& _gpu_context, const RenderContext &_render_context)
{
    if (pipeline_state_.pipeline_ || n_edges_==0) {return;}

    wgpu::ShaderModule shaderModule = create_mesh_shader_module_from_file(
        _gpu_context.device,
        "mesh_edge_shader.wgsl",
        "Mesh Edge Shader");

    // 0 -- Vertex Index 0
    wgpu::VertexAttribute attrs[3]{};
    attrs[0].shaderLocation = 0;
    attrs[0].offset = offsetof(VolMeshData::EdgeInstance, vh0);
    attrs[0].format = wgpu::VertexFormat::Uint32;

    // 1 -- Vertex Index 1
    attrs[1].shaderLocation = 1;
    attrs[1].offset = offsetof(VolMeshData::EdgeInstance, vh1);
    attrs[1].format = wgpu::VertexFormat::Uint32;

    // 2 -- Edge Index
    attrs[2].shaderLocation = 2;
    attrs[2].offset = offsetof(VolMeshData::EdgeInstance, eh);
    attrs[2].format = wgpu::VertexFormat::Uint32;

    wgpu::VertexBufferLayout vertexBufferLayout{};
    vertexBufferLayout.arrayStride = sizeof(VolMeshData::EdgeInstance);
    vertexBufferLayout.attributeCount = 3;
    vertexBufferLayout.attributes = attrs;
    vertexBufferLayout.stepMode = wgpu::VertexStepMode::Instance;

    // Vertex state
    wgpu::VertexState vertexState{};
    vertexState.module = shaderModule;
    vertexState.entryPoint = wgpu::StringView("vs_main");
    vertexState.bufferCount = 1;
    vertexState.buffers = &vertexBufferLayout;

    // ---------------
    // Fragment state
    //-----------------
    wgpu::ColorTargetState color_targets[2] = {};
    wgpu::FragmentState fragmentState = create_fragment_state(_render_context, shaderModule, color_targets);

    // Primitive state
    wgpu::PrimitiveState primitive{};
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
    wgpu::PipelineLayout pipelineLayout = _gpu_context.device.createPipelineLayout(layoutDesc);

    // Render pipeline
    wgpu::RenderPipelineDescriptor pipelineDesc{};
    wgpu::DepthStencilState depth = create_default_depth_state();
    depth.depthBias = 0.0f;
    depth.depthBiasSlopeScale = 0.0f;
    depth.depthBiasClamp = 0.0f;
    depth.depthCompare = wgpu::CompareFunction::LessEqual; // allow edges exactly on faces
    pipelineDesc.depthStencil = &depth;
    pipelineDesc.layout = pipelineLayout;
    pipelineDesc.vertex = vertexState;
    pipelineDesc.fragment = &fragmentState;
    pipelineDesc.primitive = primitive;
    pipelineDesc.multisample = multisample;
    pipelineDesc.label = wgpu::StringView("Mesh Edge Pipeline");

    pipeline_state_.pipeline_ = _gpu_context.device.createRenderPipeline(pipelineDesc);
}

void ColoredEdgeRenderer::update_property_data(const std::vector<Prop::Data>& _data)
{
    if (!initialized_) {return;}

    pipeline_state_.device_.getQueue().writeBuffer(
        property_buffer_,
        0,
        _data.data(),
        sizeof(Prop::Data) * _data.size()
        );
}

void ColoredEdgeRenderer::render(
    const ViewSize& _size,
    wgpu::RenderPassEncoder _render_pass,
    const MVP& _mvp)
{
    if (!initialized_ || !enabled_ || n_edges_==0) {return;}

    const uint32_t offset = draw_index_ * uniform_stride_;

    // Update uniforms
    uniforms_.projection_ = _mvp.proj;
    uniforms_.model_view_ = _mvp.modelview();
    uniforms_.view_ = _mvp.view;
    uniforms_.mvp_ = _mvp.proj * uniforms_.model_view_;
    uniforms_.viewport_size_ = {_size.width,_size.height};;
    uniforms_.object_id_ = object_id_;
    pipeline_state_.device_.getQueue().writeBuffer(
        uniform_buffer_, offset, &uniforms_, sizeof(Uniforms));

    _render_pass.setPipeline(pipeline_state_.pipeline_);
    _render_pass.setBindGroup(0, bind_group_, 1, &offset);
    _render_pass.setVertexBuffer(0, edge_index_buffer_, 0, n_edges_*sizeof(VolMeshData::EdgeInstance));

    // An Edge Instance is rendered as a quad
    _render_pass.draw(4, n_edges_, 0, 0);

    set_next_draw_index();
}

}
