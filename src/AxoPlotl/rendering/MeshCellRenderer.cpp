#include "MeshCellRenderer.hpp"
#include "AxoPlotl/Application.hpp"
#include "AxoPlotl/rendering/detail/wgpu_commons.hpp"

namespace AxoPlotl
{

PipelineState ColoredCellRenderer::triangle_pipeline_state_;
PipelineState ColoredCellRenderer::outline_pipeline_state_;

struct CellIndex {
    uint32_t vh_;
    uint32_t ch_;
};

void ColoredCellRenderer::init(uint32_t _object_id, Application* _app,
                                       wgpu::Buffer _vertices_position_buffer,
                                       const std::vector<std::vector<std::vector<uint32_t>>>& _cells,
                                       wgpu::Buffer _cells_center_buffer)
{
    triangle_pipeline_state_.set_device(_app->device_);
    outline_pipeline_state_.set_device(_app->device_);

    object_id_ = _object_id;
    property_color_map_.create(_app->device_);
    property_color_map_.set_coolwarm();
    app_ = _app;
    n_cells_ = _cells.size();
    n_positions_ = _vertices_position_buffer.getSize()/sizeof(Position);
    vertices_position_buffer_ = _vertices_position_buffer;
    cells_center_buffer_ = _cells_center_buffer;
    create_buffers(_cells);
    create_bind_group_layout();
    create_bind_group();
    create_triangle_pipeline();
    create_line_pipeline();
}

void ColoredCellRenderer::clear()
{
    // We only delete the buffers that were created by this
    // renderer
    destroy_buffer(property_buffer_);
    destroy_buffer(triangle_index_buffer_);
    destroy_buffer(line_index_buffer_);
    destroy_buffer(uniform_buffer_);
}

void ColoredCellRenderer::create_buffers(
    const std::vector<std::vector<std::vector<uint32_t>>>& _cells)
{
    wgpu::Device device = app_->device_;
    wgpu::Queue queue = device.getQueue();

    // Triangulate the Faces to get the vertices
    std::vector<CellIndex> triangle_indices;
    triangle_indices.reserve(n_cells_*4*3);
    for (uint32_t ch = 0; ch < n_cells_; ++ch) {
        for (const auto& f : _cells[ch]) {
            for (int i = 1; i < f.size()-1; ++i) {
                triangle_indices.push_back({.vh_ = f[0], .ch_ = ch});
                triangle_indices.push_back({.vh_ = f[i], .ch_ = ch});
                triangle_indices.push_back({.vh_ = f[i+1], .ch_ = ch});
            }
        }
    }
    n_triangle_indices_ = triangle_indices.size();

    // For the Edges, make sure we don't have duplicates
    std::vector<CellIndex> line_indices;
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

    // Cell Incenter Buffer
    // {
    //     wgpu::BufferDescriptor desc{};
    //     desc.usage =
    //         wgpu::BufferUsage::Storage |
    //         wgpu::BufferUsage::CopyDst |
    //         wgpu::BufferUsage::Vertex;
    //     desc.size = sizeof(Position) * std::max(n_cells_,1lu);
    //     desc.mappedAtCreation = false;
    //     desc.label = "Mesh Cell Incenter Buffer";

    //     center_buffer_ = device.createBuffer(desc);
    //     queue.writeBuffer(
    //         center_buffer_, 0,
    //         _centers.data(),
    //         sizeof(Position)*_centers.size());

    //     std::cout << "Mesh Cell Incenter Buffer Size: " << desc.size << std::endl;
    // }

    // Cell Triangle Index Buffer
    {
        wgpu::BufferDescriptor desc{};
        desc.usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
        desc.size = sizeof(CellIndex) * triangle_indices.size();
        desc.mappedAtCreation = false;
        desc.label = wgpu::StringView("Mesh Cell Triangle Index Buffer");

        triangle_index_buffer_ = device.createBuffer(desc);
        queue.writeBuffer(
            triangle_index_buffer_,
            0,
            triangle_indices.data(),
            desc.size
            );
        std::cout << desc.label.data << " Size: " << desc.size << std::endl;
    }

    // Cell Outline Index Buffer
    {
        wgpu::BufferDescriptor desc{};
        desc.usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
        desc.size = sizeof(CellIndex) * line_indices.size();
        desc.mappedAtCreation = false;
        desc.label = wgpu::StringView("Mesh Cell Outline Index Buffer");

        line_index_buffer_ = device.createBuffer(desc);
        queue.writeBuffer(
            line_index_buffer_,
            0,
            line_indices.data(),
            desc.size
            );
        std::cout << desc.label.data << " Size: " << desc.size << std::endl;
    }

    // Cell Property Buffer
    {
        wgpu::BufferDescriptor desc{};
        desc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
        desc.size = sizeof(Property::Data) * std::max(n_cells_,1lu);
        desc.mappedAtCreation = false;
        desc.label = wgpu::StringView("Mesh Cell Property Buffer");

        property_buffer_ = device.createBuffer(desc);

        std::cout << desc.label.data << " Size: " << desc.size << std::endl;
    }

    // Uniform Buffer
    {
        wgpu::BufferDescriptor desc{};
        desc.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
        desc.size = sizeof(Uniforms);
        desc.mappedAtCreation = false;
        desc.label = wgpu::StringView("Mesh Cell Uniform Buffer");

        uniform_buffer_ = device.createBuffer(desc);

        std::cout << desc.label.data << " Size: " << desc.size << std::endl;
    }
}

void ColoredCellRenderer::create_bind_group_layout()
{
    if (triangle_pipeline_state_.bind_group_layout_) {return;}

    wgpu::BindGroupLayoutEntry entries[6]{};

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

    wgpu::BindGroupLayoutDescriptor layoutDesc{};
    layoutDesc.entryCount = 6;
    layoutDesc.entries = entries;
    layoutDesc.label = wgpu::StringView("Cell Bind Group Layout");

    triangle_pipeline_state_.bind_group_layout_ = app_->device_.createBindGroupLayout(layoutDesc);
    outline_pipeline_state_.bind_group_layout_ = triangle_pipeline_state_.bind_group_layout_;
}

void ColoredCellRenderer::create_bind_group()
{
    wgpu::BindGroupEntry groupEntries[6]{};

    // 0 - Uniform
    groupEntries[0].binding = 0;
    groupEntries[0].buffer = uniform_buffer_;
    groupEntries[0].offset = 0;
    groupEntries[0].size = sizeof(Uniforms);
    std::cout << "0: Mesh Cell Uniforms #" << groupEntries[0].size << std::endl;

    // 1 - Positions
    groupEntries[1].binding = 1;
    groupEntries[1].buffer = vertices_position_buffer_;
    groupEntries[1].offset = 0;
    groupEntries[1].size = sizeof(Position) * n_positions_;
    std::cout << "1: Mesh Cell Positions #" << groupEntries[1].size << std::endl;

    // 2 - Property Color Map
    groupEntries[2].binding = 2;
    groupEntries[2].textureView = property_color_map_.view_;
    std::cout << "2: Mesh Cell Color Map #" << groupEntries[2].size << std::endl;

    // 3 - Color Map Sampler
    groupEntries[3].binding = 3;
    groupEntries[3].sampler = property_color_map_.sampler_;
    std::cout << "3: Mesh Cell Color Sampler #" << groupEntries[3].size << std::endl;

    // 4 - Properties
    groupEntries[4].binding = 4;
    groupEntries[4].buffer = property_buffer_;
    groupEntries[4].offset = 0;
    groupEntries[4].size = sizeof(Property::Data) * std::max(n_cells_,1lu);
    std::cout << "4: Mesh Cell Properties #" << groupEntries[4].size << std::endl;

    // 5 - Cell Centers
    groupEntries[5].binding = 5;
    groupEntries[5].buffer = cells_center_buffer_;
    groupEntries[5].offset = 0;
    groupEntries[5].size = sizeof(Position) * std::max(n_cells_,1lu);
    std::cout << "5: Mesh Cell Incenters #" << groupEntries[5].size << std::endl;

    wgpu::BindGroupDescriptor bgDesc{};
    bgDesc.layout = triangle_pipeline_state_.bind_group_layout_;
    bgDesc.entryCount = 6;
    bgDesc.entries = groupEntries;

    bind_group_ = app_->device_.createBindGroup(bgDesc);
}

void ColoredCellRenderer::create_triangle_pipeline()
{
    if (triangle_pipeline_state_.pipeline_ || n_cells_==0) {return;}

    wgpu::ShaderModule shaderModule = create_mesh_shader_module_from_file(
        app_->device_,
        "mesh_cell_shader.wgsl",
        "Cell Shader");

    // 0 -- Vertex Index
    wgpu::VertexAttribute attrs[2]{};
    attrs[0].shaderLocation = 0;
    attrs[0].offset = offsetof(CellIndex, vh_);;
    attrs[0].format = wgpu::VertexFormat::Uint32;

    // 1 -- Celll Index
    attrs[1].shaderLocation = 1;
    attrs[1].offset = offsetof(CellIndex, ch_);
    attrs[1].format = wgpu::VertexFormat::Uint32;

    wgpu::VertexBufferLayout vertexBufferLayout{};
    vertexBufferLayout.arrayStride = sizeof(CellIndex);
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
    wgpu::SurfaceCapabilities surf_caps;
    app_->surface_.getCapabilities(app_->adapter_, &surf_caps);

    // color
    color_targets[0].format = surf_caps.formats[0];
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
    wgpu::PipelineLayout pipelineLayout = app_->device_.createPipelineLayout(layoutDesc);

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

    triangle_pipeline_state_.pipeline_ = app_->device_.createRenderPipeline(pipelineDesc);
}

void ColoredCellRenderer::create_line_pipeline()
{
    if (outline_pipeline_state_.pipeline_ || n_cells_==0) {return;}

    wgpu::ShaderModule shaderModule = create_mesh_shader_module_from_file(
        app_->device_,
        "mesh_cell_outline_shader.wgsl",
        "Cell Outline");

    // 0 -- Vertex Index
    wgpu::VertexAttribute attrs[2]{};
    attrs[0].shaderLocation = 0;
    attrs[0].offset = offsetof(CellIndex, vh_);;
    attrs[0].format = wgpu::VertexFormat::Uint32;

    // 1 -- Cell Index
    attrs[1].shaderLocation = 1;
    attrs[1].offset = offsetof(CellIndex, ch_);
    attrs[1].format = wgpu::VertexFormat::Uint32;

    wgpu::VertexBufferLayout vertexBufferLayout{};
    vertexBufferLayout.arrayStride = sizeof(CellIndex);
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
    wgpu::SurfaceCapabilities surf_caps;
    app_->surface_.getCapabilities(app_->adapter_, &surf_caps);

    // color
    color_targets[0].format = surf_caps.formats[0];
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
    wgpu::PipelineLayout pipelineLayout = app_->device_.createPipelineLayout(layoutDesc);

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

    outline_pipeline_state_.pipeline_ = app_->device_.createRenderPipeline(pipelineDesc);
}

void ColoredCellRenderer::update_property_data(const std::vector<Property::Data>& _data)
{
    app_->device_.getQueue().writeBuffer(
        property_buffer_,
        0,
        _data.data(),
        sizeof(Property::Data) * _data.size()
        );
    std::cout << "Update Cell Property Data" << std::endl;
}

void ColoredCellRenderer::render(
    const Vec4f& _viewport,
    wgpu::RenderPassEncoder _render_pass,
    const Mat4x4f& _mvp)
{
    if (!enabled_ || n_cells_==0) {return;}

    // Update uniforms
    uniforms_.mvp_ = _mvp;
    uniforms_.viewport_size_ = {_viewport[2],_viewport[3]};
    uniforms_.object_id_ = object_id_;
    app_->device_.getQueue().writeBuffer(
        uniform_buffer_, 0, &uniforms_, sizeof(Uniforms));

    _render_pass.setPipeline(triangle_pipeline_state_.pipeline_);
    _render_pass.setBindGroup(0, bind_group_, 0, nullptr);
    _render_pass.setVertexBuffer(0, triangle_index_buffer_, 0, n_triangle_indices_*sizeof(CellIndex));
    _render_pass.draw(n_triangle_indices_, 1, 0, 0);

    _render_pass.setPipeline(outline_pipeline_state_.pipeline_);
    _render_pass.setBindGroup(0, bind_group_, 0, nullptr);
    _render_pass.setVertexBuffer(0, line_index_buffer_, 0, n_line_indices_*sizeof(CellIndex));
    _render_pass.draw(n_line_indices_, 1, 0, 0);
}

}
