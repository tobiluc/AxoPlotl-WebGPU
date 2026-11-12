#include "MeshCellRenderer.hpp"
#include "AxoPlotl/rendering/wgpu_commons.hpp"

namespace AxoPlotl::internal
{

PipelineState ColoredCellRenderer::triangle_pipeline_state_;
PipelineState ColoredCellRenderer::outline_pipeline_state_;

struct CellTriangleIndex {
    uint32_t vh_;
    uint32_t ch_;
    uint32_t normal_index_;
};

struct CellLineIndex {
    uint32_t vh_;
    uint32_t ch_;
};

void ColoredCellRenderer::init(uint32_t _object_id,
                               const GpuContext& _gpu_context,
                               const RenderContext &_render_context,
                               wgpu::Buffer _vertices_position_buffer,
                               const std::vector<glm::vec4> &_vertices_position_data,
                               const std::vector<std::vector<std::vector<uint32_t>>>& _cells,
                               wgpu::Buffer _cells_center_buffer)
{
    triangle_pipeline_state_.set_device(_gpu_context.device);
    outline_pipeline_state_.set_device(_gpu_context.device);

    object_id_ = _object_id;
    property_color_map_.create(_gpu_context.device);
    property_color_map_.set_coolwarm();
    n_cells_ = _cells.size();
    n_positions_ = _vertices_position_buffer.getSize()/sizeof(Position);
    vertices_position_buffer_ = _vertices_position_buffer;
    cells_center_buffer_ = _cells_center_buffer;
    create_buffers(_cells, _vertices_position_data);
    create_bind_group_layout();
    create_bind_group(_gpu_context);
    create_triangle_pipeline(_gpu_context, _render_context);
    create_line_pipeline(_gpu_context, _render_context);

    initialized_ = true;
}

void ColoredCellRenderer::clear()
{
    // We only delete the buffers that were created by this
    // renderer
    destroy_buffer(property_buffer_);
    destroy_buffer(triangle_index_buffer_);
    destroy_buffer(line_index_buffer_);
    destroy_buffer(uniform_buffer_);
    destroy_buffer(normal_buffer_);
    initialized_ = false;
}

void ColoredCellRenderer::create_buffers(
    const std::vector<std::vector<std::vector<uint32_t>>>& _cells,
    const std::vector<glm::vec4>& _vertices_position_data)
{
    wgpu::Device device = triangle_pipeline_state_.device_;
    wgpu::Queue queue = device.getQueue();

    // Triangulate the Faces to get the vertices
    std::vector<CellTriangleIndex> triangle_indices;
    triangle_indices.reserve(n_cells_*4*3);
    uint32_t normal_idx = 0;
    for (uint32_t ch = 0; ch < n_cells_; ++ch) {
        for (const auto& f : _cells[ch]) {
            if (f.size()<3) {continue;}
            for (int i = 1; i < f.size()-1; ++i) {
                triangle_indices.push_back({.vh_ = f[0], .ch_ = ch, .normal_index_=normal_idx});
                triangle_indices.push_back({.vh_ = f[i], .ch_ = ch, .normal_index_=normal_idx});
                triangle_indices.push_back({.vh_ = f[i+1], .ch_ = ch, .normal_index_=normal_idx});
            }
            ++normal_idx;
        }
    }
    n_triangle_indices_ = triangle_indices.size();
    n_normals_ = normal_idx;

    // Normal Buffer
    {
        std::vector<glm::vec4> normal_data;
        normal_data.resize(n_normals_);
        normal_idx = 0;
        for (uint32_t ch = 0; ch < n_cells_; ++ch) {
            for (const auto& f : _cells[ch]) {
                if (f.size()<3) {continue;}
                const auto& p0 = _vertices_position_data[f[0]];
                const auto& p1 = _vertices_position_data[f[1]];
                const auto& p2 = _vertices_position_data[f[2]];
                const auto n = glm::normalize(glm::cross(glm::vec3(p1 - p0), glm::vec3(p2 - p1)));
                normal_data[normal_idx] = glm::vec4(n,1);
                ++normal_idx;
            }
        }

        wgpu::BufferDescriptor desc{};
        desc.usage = wgpu::BufferUsage::Vertex
                    | wgpu::BufferUsage::Storage
                    | wgpu::BufferUsage::CopyDst;
        desc.size = sizeof(glm::vec4) * normal_data.size();
        desc.mappedAtCreation = false;
        desc.label = wgpu::StringView("Mesh Cell Triangle Normal Buffer");
        normal_buffer_ = device.createBuffer(desc);
        queue.writeBuffer(
            normal_buffer_,
            0,
            normal_data.data(),
            desc.size
            );
    }

    // For the Edges, make sure we don't have duplicates
    std::vector<CellLineIndex> line_indices;
    line_indices.reserve(n_cells_*6*2);
    for (uint32_t ch = 0; ch < n_cells_; ++ch) {
        for (const auto& f : _cells[ch]) {
            for (int i = 0; i < f.size()-1; ++i) {
                if (f[i] < f[i+1]) {
                    line_indices.push_back({.vh_ = f[i], .ch_ = ch});
                    line_indices.push_back({.vh_ = f[i+1], .ch_ = ch});
                }
            }
        }
    }
    n_line_indices_ = line_indices.size();

    // Cell Triangle Index Buffer
    {
        wgpu::BufferDescriptor desc{};
        desc.usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
        desc.size = sizeof(CellTriangleIndex) * triangle_indices.size();
        desc.mappedAtCreation = false;
        desc.label = wgpu::StringView("Mesh Cell Triangle Index Buffer");

        triangle_index_buffer_ = device.createBuffer(desc);
        queue.writeBuffer(
            triangle_index_buffer_,
            0,
            triangle_indices.data(),
            desc.size
            );
    }

    // Cell Outline Index Buffer
    {
        wgpu::BufferDescriptor desc{};
        desc.usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
        desc.size = sizeof(CellLineIndex) * line_indices.size();
        desc.mappedAtCreation = false;
        desc.label = wgpu::StringView("Mesh Cell Outline Index Buffer");

        line_index_buffer_ = device.createBuffer(desc);
        queue.writeBuffer(
            line_index_buffer_,
            0,
            line_indices.data(),
            desc.size
            );
    }

    // Cell Property Buffer
    {
        wgpu::BufferDescriptor desc{};
        desc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
        desc.size = sizeof(Property::Data) * std::max(n_cells_,1lu);
        desc.mappedAtCreation = false;
        desc.label = wgpu::StringView("Mesh Cell Property Buffer");

        property_buffer_ = device.createBuffer(desc);
    }

    // Uniform Buffer
    {
        wgpu::BufferDescriptor desc{};
        desc.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
        desc.size = sizeof(Uniforms);
        desc.mappedAtCreation = false;
        desc.label = wgpu::StringView("Mesh Cell Uniform Buffer");

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
    entries[4].buffer.minBindingSize = sizeof(Property::Data);

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

    triangle_pipeline_state_.bind_group_layout_ = outline_pipeline_state_.device_.createBindGroupLayout(layoutDesc);
    outline_pipeline_state_.bind_group_layout_ = triangle_pipeline_state_.bind_group_layout_;
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
    groupEntries[4].size = sizeof(Property::Data) * std::max(n_cells_,1lu);

    // 5 - Cell Centers
    groupEntries[5].binding = 5;
    groupEntries[5].buffer = cells_center_buffer_;
    groupEntries[5].offset = 0;
    groupEntries[5].size = sizeof(Position) * std::max(n_cells_,1lu);

    // 6 - Normals
    groupEntries[6].binding = 6;
    groupEntries[6].buffer = normal_buffer_;
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
    attrs[0].offset = offsetof(CellTriangleIndex, vh_);;
    attrs[0].format = wgpu::VertexFormat::Uint32;

    // 1 -- Cell Index
    attrs[1].shaderLocation = 1;
    attrs[1].offset = offsetof(CellTriangleIndex, ch_);
    attrs[1].format = wgpu::VertexFormat::Uint32;

    // 2 -- Normal Index
    attrs[2].shaderLocation = 2;
    attrs[2].offset = offsetof(CellTriangleIndex, normal_index_);
    attrs[2].format = wgpu::VertexFormat::Uint32;

    wgpu::VertexBufferLayout vertexBufferLayout{};
    vertexBufferLayout.arrayStride = sizeof(CellTriangleIndex);
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

void ColoredCellRenderer::create_line_pipeline(const GpuContext& _gpu_context, const RenderContext &_render_context)
{
    if (outline_pipeline_state_.pipeline_ || n_cells_==0) {return;}

    wgpu::ShaderModule shaderModule = create_mesh_shader_module_from_file(
        _gpu_context.device,
        "mesh_cell_outline_shader.wgsl",
        "Cell Outline");

    // 0 -- Vertex Index
    wgpu::VertexAttribute attrs[2]{};
    attrs[0].shaderLocation = 0;
    attrs[0].offset = offsetof(CellLineIndex, vh_);;
    attrs[0].format = wgpu::VertexFormat::Uint32;

    // 1 -- Cell Index
    attrs[1].shaderLocation = 1;
    attrs[1].offset = offsetof(CellLineIndex, ch_);
    attrs[1].format = wgpu::VertexFormat::Uint32;

    wgpu::VertexBufferLayout vertexBufferLayout{};
    vertexBufferLayout.arrayStride = sizeof(CellLineIndex);
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

    // color
    color_targets[0].format = _render_context.surface_format;
    color_targets[0].writeMask = wgpu::ColorWriteMask::All;

    // picking
    color_targets[1].format = wgpu::TextureFormat::RGBA32Uint;
    color_targets[1].blend = nullptr;
    color_targets[1].writeMask = wgpu::ColorWriteMask::All;

    wgpu::FragmentState fragmentState{};
    fragmentState.module = shaderModule;
    fragmentState.entryPoint = wgpu::StringView("fs_main");
    fragmentState.targetCount = 2;
    fragmentState.targets = color_targets;

    // Primitive state
    wgpu::PrimitiveState primitive{};
    primitive.topology = wgpu::PrimitiveTopology::LineList;
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
    layoutDesc.bindGroupLayouts = reinterpret_cast<WGPUBindGroupLayout*>(&outline_pipeline_state_.bind_group_layout_);
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
    pipelineDesc.label = wgpu::StringView("Cell Outline Pipeline");

    outline_pipeline_state_.pipeline_ = _gpu_context.device.createRenderPipeline(pipelineDesc);
}

void ColoredCellRenderer::update_property_data(const std::vector<Property::Data>& _data)
{
    triangle_pipeline_state_.device_.getQueue().writeBuffer(
        property_buffer_,
        0,
        _data.data(),
        sizeof(Property::Data) * _data.size()
        );
}

void ColoredCellRenderer::render(
    const ViewSize &_size,
    wgpu::RenderPassEncoder _render_pass,
    const Mat4x4f& _mvp)
{
    if (!initialized_ || !enabled_ || n_cells_==0) {return;}

    // Update uniforms
    uniforms_.mvp_ = _mvp;
    uniforms_.viewport_size_ = {_size.width,_size.height};
    uniforms_.object_id_ = object_id_;
    triangle_pipeline_state_.device_.getQueue().writeBuffer(
        uniform_buffer_, 0, &uniforms_, sizeof(Uniforms));

    _render_pass.setPipeline(triangle_pipeline_state_.pipeline_);
    _render_pass.setBindGroup(0, bind_group_, 0, nullptr);
    _render_pass.setVertexBuffer(0, triangle_index_buffer_, 0, n_triangle_indices_*sizeof(CellTriangleIndex));
    _render_pass.draw(n_triangle_indices_, 1, 0, 0);

    if (outlines_enabled_) {
        _render_pass.setPipeline(outline_pipeline_state_.pipeline_);
        _render_pass.setBindGroup(0, bind_group_, 0, nullptr);
        _render_pass.setVertexBuffer(0, line_index_buffer_, 0, n_line_indices_*sizeof(CellLineIndex));
        _render_pass.draw(n_line_indices_, 1, 0, 0);
    }
}

}
