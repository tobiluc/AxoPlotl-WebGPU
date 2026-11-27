#include "MeshHalfFaceRenderer.hpp"
#include "AxoPlotl/rendering/wgpu_commons.hpp"

namespace AxoPlotl::internal
{
PipelineState MeshHalfFaceRenderer::pipeline_state_;

void MeshHalfFaceRenderer::init(
    uint32_t _object_id,
    const GpuContext& _gpu_context,
    const RenderContext& _render_context,
    const BufferData<Position>& _vertices_positions,
    const BufferData<VolMeshBuffers::HalfFaceIndex>& _halffaces_triangle_indices,
    const BufferData<VolMeshBuffers::Normal> &_halffaces_normals,
    const Options &_opt)
{
    pipeline_state_.set_device(_gpu_context.device);
    cleanup();

    object_id_ = _object_id;

    property_color_map_.create(_gpu_context.device);
    property_color_map_.set_coolwarm();

    n_halffaces_ = _halffaces_normals.data.size();
    n_positions_ = _vertices_positions.data.size();
    n_indices_ = _halffaces_triangle_indices.data.size();

    if (n_indices_ == 0) {
        initialized_ = false;
        return;
    }

    position_buffer_ = _vertices_positions.buffer;
    halfface_normal_buffer_ = _halffaces_normals.buffer;
    halfface_index_buffer_ = _halffaces_triangle_indices.buffer;

    create_buffers();
    create_bind_group_layout();
    create_bind_group();
    create_pipeline(_gpu_context, _render_context, _opt);

    initialized_ = true;
}

void MeshHalfFaceRenderer::cleanup()
{
    save_destroy_release(property_buffer_);
    save_destroy_release(uniform_buffer_);
    save_release(bind_group_);
    initialized_ = false;
}

void MeshHalfFaceRenderer::create_buffers()
{
    wgpu::Device device = pipeline_state_.device_;
    wgpu::Queue queue = device.getQueue();

    // Property Buffer
    {
        wgpu::BufferDescriptor desc{};
        desc.usage = wgpu::BufferUsage::Storage |
                     wgpu::BufferUsage::CopyDst;
        desc.size = sizeof(PropContainer)*n_halffaces_;
        desc.mappedAtCreation = false;
        desc.label = wgpu::StringView("Mesh Face Property Buffer");
        property_buffer_ = device.createBuffer(desc);
    }

    // Uniform Buffer
    {
        compute_uniform_stride<Uniforms>(device);
        wgpu::BufferDescriptor desc;
        desc.size = uniform_stride_ * std::max(max_draw_calls_, 1u);
        desc.usage = wgpu::BufferUsage::Uniform |
                     wgpu::BufferUsage::CopyDst;
        desc.mappedAtCreation = false;
        desc.label = wgpu::StringView("Face Uniform Buffer");
        uniform_buffer_ = device.createBuffer(desc);
    }
}

void MeshHalfFaceRenderer::create_bind_group_layout()
{
    if (pipeline_state_.bind_group_layout_) {return;}

    wgpu::BindGroupLayoutEntry entries[6]{};

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
    entries[4].buffer.minBindingSize = sizeof(PropContainer);

    // 5 - Normals
    entries[5].binding = 5;
    entries[5].visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
    entries[5].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
    entries[5].buffer.minBindingSize = sizeof(glm::vec4);

    wgpu::BindGroupLayoutDescriptor layoutDesc{};
    layoutDesc.entryCount = 6;
    layoutDesc.entries = entries;
    layoutDesc.label = wgpu::StringView("Face Bind Group Layout");

    pipeline_state_.bind_group_layout_ = pipeline_state_.device_.createBindGroupLayout(layoutDesc);
}

void MeshHalfFaceRenderer::create_bind_group()
{
    wgpu::BindGroupEntry groupEntries[6]{};

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
    groupEntries[4].size = sizeof(PropContainer) * std::max(n_halffaces_,1lu);

    // 5 - Normals
    groupEntries[5].binding = 5;
    groupEntries[5].buffer = halfface_normal_buffer_;
    groupEntries[5].offset = 0;
    groupEntries[5].size = sizeof(glm::vec4) * std::max(n_halffaces_,1lu);

    wgpu::BindGroupDescriptor bgDesc{};
    bgDesc.layout = pipeline_state_.bind_group_layout_;
    bgDesc.entryCount = 6;
    bgDesc.entries = groupEntries;

    bind_group_ = pipeline_state_.device_.createBindGroup(bgDesc);
}

void MeshHalfFaceRenderer::create_pipeline(
    const GpuContext& _gpu_context,
    const RenderContext &_render_context,
    const Options& _opt)
{
    if (pipeline_state_.pipeline_ || n_halffaces_==0) {return;}

    wgpu::ShaderModule shaderModule = create_mesh_shader_module_from_file(
        _gpu_context.device,
        "mesh_halfface_shader.wgsl",
        "Face Shader");

    // 0 -- Vertex Index
    wgpu::VertexAttribute attrs[2]{};
    attrs[0].shaderLocation = 0;
    attrs[0].offset = offsetof(VolMeshBuffers::HalfFaceIndex, vh);;
    attrs[0].format = wgpu::VertexFormat::Uint32;

    // 1 -- Face Index
    attrs[1].shaderLocation = 1;
    attrs[1].offset = offsetof(VolMeshBuffers::HalfFaceIndex, hfh);
    attrs[1].format = wgpu::VertexFormat::Uint32;

    wgpu::VertexBufferLayout vertexBufferLayout{};
    vertexBufferLayout.arrayStride = sizeof(VolMeshBuffers::HalfFaceIndex);
    vertexBufferLayout.attributeCount = 2;
    vertexBufferLayout.attributes = attrs;
    vertexBufferLayout.stepMode = wgpu::VertexStepMode::Vertex;

    // // Vertex state
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
    primitive.topology = wgpu::PrimitiveTopology::TriangleList;
    primitive.frontFace = wgpu::FrontFace::CCW;
    primitive.cullMode = _opt.cull_mode;

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
    pipelineDesc.depthStencil = &depth;
    pipelineDesc.layout = pipelineLayout;
    pipelineDesc.vertex = vertexState;
    pipelineDesc.fragment = &fragmentState;
    pipelineDesc.primitive = primitive;
    pipelineDesc.multisample = multisample;
    pipelineDesc.label = wgpu::StringView("Face Triangle Pipeline");

    pipeline_state_.pipeline_ = _gpu_context.device.createRenderPipeline(pipelineDesc);
}

void MeshHalfFaceRenderer::update_halfface_property_data(const std::vector<PropContainer>& _data)
{
    if (!initialized_) {return;}
    if (_data.size() != n_halffaces_) {
        std::cerr << "ERROR: #Props != #Halffaces" << std::endl;
        return;
    }

    pipeline_state_.device_.getQueue().writeBuffer(
        property_buffer_,
        0,
        _data.data(),
        sizeof(PropContainer) * _data.size()
        );
}

void MeshHalfFaceRenderer::update_face_property_data(const std::vector<PropContainer>& _data)
{
    if (!initialized_) {return;}
    if (_data.size()*2u != n_halffaces_) {
        std::cerr << "ERROR: #Props != #Faces" << std::endl;
        return;
    }
    std::vector<PropContainer> newData(n_halffaces_);
    for (size_t i = 0; i < _data.size(); ++i) {
        newData[(i<<1u)] = _data[i];
        newData[(i<<1u)+1u] = _data[i];
    }
    update_halfface_property_data(newData);
}

void MeshHalfFaceRenderer::render(
    const ViewSize& _size,
    wgpu::RenderPassEncoder _render_pass,
    const Mat4x4& _mvp,
    const Picker::Entity &_selected_entity)
{
    if (!initialized_ || !enabled_ || n_halffaces_==0) {return;}

    const uint32_t offset = draw_index_ * uniform_stride_;

    // Update uniforms
    uniforms_.mvp_ = _mvp;
    uniforms_.viewport_size_ = {_size.width,_size.height};;
    uniforms_.object_id_ = object_id_;
    uniforms_.selected_entity_ = _selected_entity;
    pipeline_state_.device_.getQueue().writeBuffer(
        uniform_buffer_, offset, &uniforms_, sizeof(Uniforms));

    _render_pass.setPipeline(pipeline_state_.pipeline_);
    _render_pass.setBindGroup(0, bind_group_, 1, &offset);
    _render_pass.setVertexBuffer(0, halfface_index_buffer_, 0, n_indices_*sizeof(VolMeshBuffers::HalfFaceIndex));
    _render_pass.draw(n_indices_, 1, 0, 0);

    set_next_draw_index();
}

}
