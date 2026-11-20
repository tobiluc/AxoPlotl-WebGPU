#include "MeshCellRenderer.hpp"
#include "AxoPlotl/rendering/MeshData.hpp"
#include "AxoPlotl/rendering/wgpu_commons.hpp"

namespace AxoPlotl::internal
{

PipelineState ColoredCellRenderer::triangle_pipeline_state_;


void ColoredCellRenderer::init(uint32_t _object_id,
                               const GpuContext& _gpu_context,
                               const RenderContext &_render_context,
                               const VolMeshData &_data)
{
    if (_data.mesh.n_cells() == 0lu) {return;}

    triangle_pipeline_state_.set_device(_gpu_context.device);
    // outline_pipeline_state_.set_device(_gpu_context.device);

    object_id_ = _object_id;

    property_color_map_.create(_gpu_context.device);
    property_color_map_.set_coolwarm();

    n_cells_ = _data.mesh.n_cells();
    n_positions_ = _data.vertices_positions.data.size();
    n_normals_ = _data.halffaces_normals.data.size();
    n_triangle_indices_ = _data.cells_indices.data.size();

    if (n_triangle_indices_ == 0) {
        initialized_ = false;
        return;
    }

    vertices_position_buffer_ = _data.vertices_positions.buffer;
    triangle_index_buffer_ = _data.cells_indices.buffer;
    cells_center_buffer_ = _data.cells_centers.buffer;
    halfface_normal_buffer_ = _data.halffaces_normals.buffer;

    create_buffers();
    create_bind_group_layout();
    create_bind_group(_gpu_context);
    create_triangle_pipeline(_gpu_context, _render_context);

    initialized_ = true;
}

void ColoredCellRenderer::clear()
{
    // We only delete the buffers that were created by this
    // renderer
    destroy_buffer(property_buffer_);
    destroy_buffer(uniform_buffer_);
    initialized_ = false;
}

void ColoredCellRenderer::create_buffers()
{
    wgpu::Device device = triangle_pipeline_state_.device_;
    wgpu::Queue queue = device.getQueue();

    // Property Buffer
    {
        wgpu::BufferDescriptor desc{};
        desc.usage = wgpu::BufferUsage::Storage |
                     wgpu::BufferUsage::CopyDst;
        desc.size = sizeof(PropContainer)*n_cells_;
        desc.mappedAtCreation = false;
        desc.label = wgpu::StringView("Mesh Cell Property Buffer");
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
        desc.label = wgpu::StringView("Cell Uniform Buffer");
        uniform_buffer_ = device.createBuffer(desc);
    }
}

void ColoredCellRenderer::create_bind_group_layout()
{
    if (triangle_pipeline_state_.bind_group_layout_) {return;}

    wgpu::BindGroupLayoutEntry entries[7]{};

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

    // 5 - Centers
    entries[5].binding = 5;
    entries[5].visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
    entries[5].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
    entries[5].buffer.minBindingSize = sizeof(Position);

    // 6 - Normals
    entries[6].binding = 6;
    entries[6].visibility = wgpu::ShaderStage::Vertex;
    entries[6].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
    entries[6].buffer.minBindingSize = sizeof(glm::vec4);

    wgpu::BindGroupLayoutDescriptor layoutDesc{};
    layoutDesc.entryCount = 7;
    layoutDesc.entries = entries;
    layoutDesc.label = wgpu::StringView("Cell Bind Group Layout");

    triangle_pipeline_state_.bind_group_layout_ = triangle_pipeline_state_.device_.createBindGroupLayout(layoutDesc);
}

void ColoredCellRenderer::create_bind_group(const GpuContext &_gpu_context)
{
    wgpu::BindGroupEntry groupEntries[7]{};

    // 0 - Uniform
    groupEntries[0].binding = 0;
    groupEntries[0].buffer = uniform_buffer_;
    groupEntries[0].offset = 0;
    groupEntries[0].size = sizeof(Uniforms);

    // 1 - Positions
    groupEntries[1].binding = 1;
    groupEntries[1].buffer = vertices_position_buffer_;
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
    groupEntries[4].size = sizeof(PropContainer) * std::max(n_cells_,1lu);

    // 5 - Cell Centers
    groupEntries[5].binding = 5;
    groupEntries[5].buffer = cells_center_buffer_;
    groupEntries[5].offset = 0;
    groupEntries[5].size = sizeof(Position) * std::max(n_cells_,1lu);

    // 6 - Normals
    groupEntries[6].binding = 6;
    groupEntries[6].buffer = halfface_normal_buffer_;
    groupEntries[6].offset = 0;
    groupEntries[6].size = sizeof(glm::vec4) * n_normals_;

    wgpu::BindGroupDescriptor bgDesc{};
    bgDesc.layout = triangle_pipeline_state_.bind_group_layout_;
    bgDesc.entryCount = 7;
    bgDesc.entries = groupEntries;

    bind_group_ = _gpu_context.device.createBindGroup(bgDesc);
}

void ColoredCellRenderer::create_triangle_pipeline(
    const GpuContext &_gpu_context,
    const RenderContext& _render_context)
{
    if (triangle_pipeline_state_.pipeline_ || n_cells_==0) {return;}

    wgpu::ShaderModule shaderModule = create_mesh_shader_module_from_file(
        _gpu_context.device,
        "mesh_cell_shader.wgsl",
        "Cell Shader");

    // 0 -- Vertex Index
    wgpu::VertexAttribute attrs[3]{};
    attrs[0].shaderLocation = 0;
    attrs[0].offset = offsetof(VolMeshData::CellIndex, vh);;
    attrs[0].format = wgpu::VertexFormat::Uint32;

    // 1 -- Cell Index
    attrs[1].shaderLocation = 1;
    attrs[1].offset = offsetof(VolMeshData::CellIndex, ch);
    attrs[1].format = wgpu::VertexFormat::Uint32;

    // 2 -- Halfface Index
    attrs[2].shaderLocation = 2;
    attrs[2].offset = offsetof(VolMeshData::CellIndex, hfh);
    attrs[2].format = wgpu::VertexFormat::Uint32;

    wgpu::VertexBufferLayout vertexBufferLayout{};
    vertexBufferLayout.arrayStride = sizeof(VolMeshData::CellIndex);
    vertexBufferLayout.attributeCount = 3;
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
    primitive.cullMode = wgpu::CullMode::None;

    // Multisample
    wgpu::MultisampleState multisample{};
    multisample.count = 1;
    multisample.mask = ~0u;
    multisample.alphaToCoverageEnabled = false;

    // Pipeline layout
    wgpu::PipelineLayoutDescriptor layoutDesc{};
    layoutDesc.bindGroupLayoutCount = 1;
    layoutDesc.bindGroupLayouts = reinterpret_cast<WGPUBindGroupLayout*>(&triangle_pipeline_state_.bind_group_layout_);
    wgpu::PipelineLayout pipelineLayout = _gpu_context.device.createPipelineLayout(layoutDesc);

    // Render pipeline
    wgpu::RenderPipelineDescriptor pipelineDesc{};
    wgpu::DepthStencilState depth = create_default_depth_state();
    pipelineDesc.depthStencil = &depth;
    pipelineDesc.layout = pipelineLayout;
    pipelineDesc.vertex = vertexState;
    pipelineDesc.fragment = &fragmentState;
    pipelineDesc.primitive = primitive;
    pipelineDesc.multisample = multisample;
    pipelineDesc.label = wgpu::StringView("Cell Triangle Pipeline");

    triangle_pipeline_state_.pipeline_ = _gpu_context.device.createRenderPipeline(pipelineDesc);
}

void ColoredCellRenderer::update_property_data(const std::vector<PropContainer>& _data)
{
    if (!initialized_) {return;}
    if (_data.size() != n_cells_) {
        std::cerr << "ERROR: #Props != #Cells" << std::endl;
        return;
    }

    triangle_pipeline_state_.device_.getQueue().writeBuffer(
        property_buffer_,
        0,
        _data.data(),
        sizeof(PropContainer) * _data.size()
        );
}

void ColoredCellRenderer::render(
    const ViewSize &_size,
    wgpu::RenderPassEncoder _render_pass,
    const Mat4x4& _mvp)
{
    if (!initialized_ || !enabled_ || n_cells_==0) {return;}

    const uint32_t offset = draw_index_ * uniform_stride_;

    // Update uniforms
    uniforms_.mvp_ = _mvp;
    uniforms_.viewport_size_ = {_size.width,_size.height};
    uniforms_.object_id_ = object_id_;

    triangle_pipeline_state_.device_.getQueue().writeBuffer(
        uniform_buffer_, offset, &uniforms_, sizeof(Uniforms));

    _render_pass.setPipeline(triangle_pipeline_state_.pipeline_);
    _render_pass.setBindGroup(0, bind_group_, 1, &offset);
    _render_pass.setVertexBuffer(0, triangle_index_buffer_, 0, n_triangle_indices_*sizeof(VolMeshData::CellIndex));
    _render_pass.draw(n_triangle_indices_, 1, 0, 0);

    set_next_draw_index();
}

}
