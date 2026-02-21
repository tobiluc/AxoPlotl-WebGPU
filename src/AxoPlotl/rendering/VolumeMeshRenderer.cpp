#include "VolumeMeshRenderer.hpp"
#include "AxoPlotl/rendering/detail/wgpu_commons.hpp"
#include "AxoPlotl/rendering/shaders/cell_shader_wgsl.hpp"
#include "AxoPlotl/rendering/shaders/edge_shader_wgsl.hpp"
#include "AxoPlotl/rendering/shaders/face_shader_wgsl.hpp"
#include "AxoPlotl/rendering/shaders/vertex_shader_wgsl.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <cstddef>
#include <AxoPlotl/Application.hpp>

namespace AxoPlotl
{

// Static Members
wgpu::RenderPipeline VolumeMeshRenderer::vertices_pipeline_;
wgpu::RenderPipeline VolumeMeshRenderer::edges_pipeline_;
wgpu::RenderPipeline VolumeMeshRenderer::face_triangles_pipeline_;
wgpu::RenderPipeline VolumeMeshRenderer::cell_triangles_pipeline_;
//wgpu::DepthStencilState VolumeMeshRenderer::depth_stencil_state_;
wgpu::BindGroupLayout VolumeMeshRenderer::bind_group_layout_;

void VolumeMeshRenderer::init(Application *_app, const StaticData& _data)
{
    app_ = _app;

    property_color_map_.create(_app->device_);
    property_color_map_.set_coolwarm();

    // For simplicity, to avoid webgpu errors related
    // to the property buffers having size zero, we always have
    // at least 1 entity. TODO: Find better solution
    n_positions_ = _data.positions_.size();
    n_face_triangle_indices_ = _data.face_draw_triangle_indices_.size();
    n_cell_triangle_indices_ = _data.cell_draw_triangle_indices_.size();

    n_vertices_ = std::max(_data.vertex_instances_.size(), 1lu);
    n_edges_ = std::max(_data.edge_instances_.size(), 1lu);
    n_faces_ = std::max(_data.n_faces_, 1LU);
    n_cells_ = std::max(_data.n_cells_, 1LU);

    create_buffers(_data);
    create_bind_group_layout();
    create_bind_group();

    create_vertices_pipeline();
    create_edges_pipeline();
    create_face_triangle_pipeline();
    create_cell_triangle_pipeline();
}

void VolumeMeshRenderer::create_buffers(const StaticData &_data)
{
    wgpu::Device device = app_->device_;
    wgpu::Queue queue = device.getQueue();

    // Position Buffer
    {
        wgpu::BufferDescriptor desc{};
        desc.usage =
            wgpu::BufferUsage::Storage |
            wgpu::BufferUsage::CopyDst |
            wgpu::BufferUsage::Vertex;
        desc.size = sizeof(Position) * n_positions_;
        desc.mappedAtCreation = false;
        desc.label = "Position";

        positionBuffer_ = device.createBuffer(desc);
        queue.writeBuffer(positionBuffer_, 0, _data.positions_.data(), desc.size);

        std::cout << "Position Buffer Size: " << desc.size << std::endl;
    }

    // Cell Incenter Buffer
    if (n_cells_ > 0)
    {
        wgpu::BufferDescriptor desc{};
        desc.usage =
            wgpu::BufferUsage::Storage |
            wgpu::BufferUsage::CopyDst |
            wgpu::BufferUsage::Vertex;
        desc.size = sizeof(Position) * n_cells_;
        desc.mappedAtCreation = false;
        desc.label = "Cell Incenter";

        cell_incenter_buffer_ = device.createBuffer(desc);
        queue.writeBuffer(
            cell_incenter_buffer_, 0,
            _data.cell_incenters_.data(),
            sizeof(Position)*_data.cell_incenters_.size());

        std::cout << "Cell Incenter Buffer Size: " << desc.size << std::endl;
    }

    // Vertex Index Buffer
    if (n_vertices_ > 0)
    {
        wgpu::BufferDescriptor desc{};
        desc.usage = wgpu::BufferUsage::Index | wgpu::BufferUsage::CopyDst;
        desc.size = sizeof(uint32_t) * n_vertices_;
        desc.mappedAtCreation = false;
        desc.label = "Vertex Index";

        vertexIndexBuffer_ = device.createBuffer(desc);
        queue.writeBuffer(
            vertexIndexBuffer_,
            0,
            _data.vertex_instances_.data(),
            sizeof(uint32_t) * _data.vertex_instances_.size()
        );

        std::cout << "Vertex Index Buffer Size: " << desc.size << std::endl;
    }

    // Edge Index Buffer
    if (n_edges_ > 0)
    {
        wgpu::BufferDescriptor desc{};
        desc.usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
        desc.size = sizeof(EdgeInstance) * n_edges_;
        desc.mappedAtCreation = false;
        desc.label = "Edge Index";

        edgeIndexBuffer_ = device.createBuffer(desc);
        queue.writeBuffer(
            edgeIndexBuffer_,
            0,
            _data.edge_instances_.data(),
            sizeof(EdgeInstance) * _data.edge_instances_.size()
        );

        std::cout << "Edge Index Buffer Size: " << desc.size << std::endl;
    }

    // Face Triangle Index Buffer
    {
        wgpu::BufferDescriptor desc{};
        desc.usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
        desc.size = sizeof(FaceHandle) * _data.face_draw_triangle_indices_.size();
        desc.mappedAtCreation = false;
        desc.label = "Face Triangle Index";

        faceTriangleIndexBuffer_ = device.createBuffer(desc);
        queue.writeBuffer(
            faceTriangleIndexBuffer_,
            0,
            _data.face_draw_triangle_indices_.data(),
            desc.size
            );
        std::cout << "Face Triangle Index Buffer Size: " << desc.size << std::endl;
    }

    // Cell Triangle Index Buffer
    {
        wgpu::BufferDescriptor desc{};
        desc.usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
        desc.size = sizeof(CellHandle) * _data.cell_draw_triangle_indices_.size();
        desc.mappedAtCreation = false;
        desc.label = "Cell Triangle Index";

        cellTriangleIndexBuffer_ = device.createBuffer(desc);
        queue.writeBuffer(
            cellTriangleIndexBuffer_,
            0,
            _data.cell_draw_triangle_indices_.data(),
            desc.size
        );
        std::cout << "Cell Triangle Index Buffer Size: " << desc.size << std::endl;
    }

    // Vertex Property Buffer
    if (n_vertices_ > 0)
    {
        wgpu::BufferDescriptor desc{};
        desc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
        desc.size = sizeof(Property::Data) * n_vertices_;
        desc.mappedAtCreation = false;
        desc.label = "Vertex Property";

        vertex_property_buffer_ = device.createBuffer(desc);

        std::cout << "Vertex Property Buffer Size: " << desc.size << std::endl;
    }

    // Edge Property Buffer
    if (n_edges_ > 0)
    {
        wgpu::BufferDescriptor desc{};
        desc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
        desc.size = sizeof(Property::Data) * n_edges_;
        desc.mappedAtCreation = false;
        desc.label = "Edge Property";

        edge_property_buffer_ = device.createBuffer(desc);

        std::cout << "Edge Property Buffer Size: " << desc.size << std::endl;
    }

    // Face Property Buffer
    {
        wgpu::BufferDescriptor desc{};
        desc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
        desc.size = sizeof(Property::Data) * n_faces_;
        desc.mappedAtCreation = false;
        desc.label = "Face Property";

        face_property_buffer_ = device.createBuffer(desc);

        std::cout << "Face Property Buffer Size: " << desc.size << std::endl;
    }

    // Cell Property Buffer
    {
        wgpu::BufferDescriptor desc{};
        desc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
        desc.size = sizeof(Property::Data) * n_cells_;
        desc.mappedAtCreation = false;
        desc.label = "Cell Property";

        cell_property_buffer_ = device.createBuffer(desc);

        std::cout << "Cell Property Buffer Size: " << desc.size << std::endl;
    }

    // Uniform Buffer
    {
        wgpu::BufferDescriptor desc{};
        desc.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
        desc.size = sizeof(Uniforms);
        desc.mappedAtCreation = false;
        desc.label = "Uniform";

        uniform_buffer_ = device.createBuffer(desc);

        std::cout << "Uniform Buffer Size: " << desc.size << std::endl;
    }
}

void VolumeMeshRenderer::create_bind_group_layout()
{
    if (bind_group_layout_) {return;}

    wgpu::BindGroupLayoutEntry entries[9]{};

    // 0 - Uniform (MVP)
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

    // 4 - Vertex Properties
    entries[4].binding = 4;
    entries[4].visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
    entries[4].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
    entries[4].buffer.minBindingSize = sizeof(Property::Data);

    // 5 - Edge Properties
    entries[5].binding = 5;
    entries[5].visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
    entries[5].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
    entries[5].buffer.minBindingSize = sizeof(Property::Data);

    // 6 - Face Properties
    entries[6].binding = 6;
    entries[6].visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
    entries[6].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
    entries[6].buffer.minBindingSize = sizeof(Property::Data);

    // 7 - Cell Properties
    entries[7].binding = 7;
    entries[7].visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
    entries[7].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
    entries[7].buffer.minBindingSize = sizeof(Property::Data);

    // 8 - Cell Incenter
    entries[8].binding = 8;
    entries[8].visibility = wgpu::ShaderStage::Vertex;
    entries[8].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
    entries[8].buffer.minBindingSize = sizeof(Position);

    wgpu::BindGroupLayoutDescriptor layoutDesc{};
    layoutDesc.entryCount = 9;
    layoutDesc.entries = entries;

    bind_group_layout_ = app_->device_.createBindGroupLayout(layoutDesc);
}

void VolumeMeshRenderer::create_bind_group()
{
    wgpu::BindGroupEntry groupEntries[9]{};

    // 0 - Uniform
    groupEntries[0].binding = 0;
    groupEntries[0].buffer = uniform_buffer_;
    groupEntries[0].offset = 0;
    groupEntries[0].size = sizeof(Uniforms);
    std::cout << "0: Uniforms #" << groupEntries[0].size << std::endl;

    // 1 - Positions
    groupEntries[1].binding = 1;
    groupEntries[1].buffer = positionBuffer_;
    groupEntries[1].offset = 0;
    groupEntries[1].size = sizeof(Position) * n_positions_;
    std::cout << "1: Positions #" << groupEntries[1].size << std::endl;

    // 2 - Property Color Map
    groupEntries[2].binding = 2;
    groupEntries[2].textureView = property_color_map_.view_;
    std::cout << "2: Color Map #" << groupEntries[2].size << std::endl;

    // 3 - Color Map Sampler
    groupEntries[3].binding = 3;
    groupEntries[3].sampler = property_color_map_.sampler_;
    std::cout << "3: Color Sampler #" << groupEntries[3].size << std::endl;

    // 4 - Vertex Properties
    groupEntries[4].binding = 4;
    groupEntries[4].buffer = vertex_property_buffer_;
    groupEntries[4].offset = 0;
    groupEntries[4].size = sizeof(Property::Data) * n_vertices_;
    std::cout << "4: Vertex Properties #" << groupEntries[4].size << std::endl;

    // 5 - Edge Properties
    groupEntries[5].binding = 5;
    groupEntries[5].buffer = edge_property_buffer_;
    groupEntries[5].offset = 0;
    groupEntries[5].size = sizeof(Property::Data) * n_edges_;
    std::cout << "5: Edge Properties #" << groupEntries[5].size << std::endl;

    // 6 - Face Properties
    groupEntries[6].binding = 6;
    groupEntries[6].buffer = face_property_buffer_;
    groupEntries[6].offset = 0;
    groupEntries[6].size = sizeof(Property::Data) * n_faces_;
    std::cout << "6: Face Properties #" << groupEntries[6].size << std::endl;

    // 7 - Cell Properties
    groupEntries[7].binding = 7;
    groupEntries[7].buffer = cell_property_buffer_;
    groupEntries[7].offset = 0;
    groupEntries[7].size = sizeof(Property::Data) * n_cells_;
    std::cout << "7: Cell Properties #" << groupEntries[7].size << std::endl;

    // 8 - Cell Incenter
    groupEntries[8].binding = 8;
    groupEntries[8].buffer = cell_incenter_buffer_;
    groupEntries[8].offset = 0;
    groupEntries[8].size = sizeof(Position) * n_cells_;
    std::cout << "8: Cell Incenters #" << groupEntries[8].size << std::endl;

    // 9 Halfedge Props

    // 10 Halfface Props

    wgpu::BindGroupDescriptor bgDesc{};
    bgDesc.layout = bind_group_layout_;
    bgDesc.entryCount = 9;
    bgDesc.entries = groupEntries;

    bind_group_ = app_->device_.createBindGroup(bgDesc);
}

void VolumeMeshRenderer::create_vertices_pipeline()
{
    if (vertices_pipeline_ || n_vertices_==0) {return;}

    wgpu::ShaderModule shaderModule = create_mesh_shader_module(app_->device_, vertex_shader_wgsl, "Vertex Point");

    // Vertex state (no vertex buffer)
    wgpu::VertexState vertexState{};
    vertexState.module = shaderModule;
    vertexState.entryPoint = "vs_main";
    vertexState.bufferCount = 0;
    vertexState.buffers = nullptr;

    // Fragment state
    wgpu::ColorTargetState colorTarget{};
    wgpu::SurfaceCapabilities surf_caps;
    app_->surface_.getCapabilities(app_->adapter_, &surf_caps);
    colorTarget.format = surf_caps.formats[0];
    colorTarget.writeMask = wgpu::ColorWriteMask::All;

    wgpu::FragmentState fragmentState{};
    fragmentState.module = shaderModule;
    fragmentState.entryPoint = "fs_main";
    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTarget;

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
    layoutDesc.bindGroupLayouts = reinterpret_cast<WGPUBindGroupLayout*>(&bind_group_layout_);

    // Pipeline
    wgpu::RenderPipelineDescriptor pipelineDesc{};
    wgpu::DepthStencilState depth = create_default_depth_state();
    pipelineDesc.depthStencil = &depth;
    pipelineDesc.layout = app_->device_.createPipelineLayout(layoutDesc);
    pipelineDesc.vertex = vertexState;
    pipelineDesc.fragment = &fragmentState;
    pipelineDesc.primitive = primitive;
    pipelineDesc.multisample = multisample;
    pipelineDesc.label = "Vertex Point Pipeline";

    vertices_pipeline_ = app_->device_.createRenderPipeline(pipelineDesc);
}

void VolumeMeshRenderer::create_edges_pipeline()
{
    if (edges_pipeline_ || n_edges_==0) {return;}

    wgpu::ShaderModule shaderModule = create_mesh_shader_module(app_->device_, edge_shader_wgsl, "Edge Line");

    // 0 -- Vertex Index 0
    wgpu::VertexAttribute attrs[3]{};
    attrs[0].shaderLocation = 0;
    attrs[0].offset = offsetof(EdgeInstance, vh0_);
    attrs[0].format = wgpu::VertexFormat::Uint32;

    // 1 -- Vertex Index 1
    attrs[1].shaderLocation = 1;
    attrs[1].offset = offsetof(EdgeInstance, vh1_);
    attrs[1].format = wgpu::VertexFormat::Uint32;

    // 2 -- Edge Index
    attrs[2].shaderLocation = 2;
    attrs[2].offset = offsetof(EdgeInstance, eh_);
    attrs[2].format = wgpu::VertexFormat::Uint32;

    wgpu::VertexBufferLayout vertexBufferLayout{};
    vertexBufferLayout.arrayStride = sizeof(EdgeInstance);
    vertexBufferLayout.attributeCount = 3;
    vertexBufferLayout.attributes = attrs;
    vertexBufferLayout.stepMode = wgpu::VertexStepMode::Instance;

    // Vertex state
    wgpu::VertexState vertexState{};
    vertexState.module = shaderModule;
    vertexState.entryPoint = "vs_main";
    vertexState.bufferCount = 1;
    vertexState.buffers = &vertexBufferLayout;

    // Fragment state
    wgpu::ColorTargetState colorTarget{};
    wgpu::SurfaceCapabilities surf_caps;
    app_->surface_.getCapabilities(app_->adapter_, &surf_caps);
    colorTarget.format = surf_caps.formats[0];
    colorTarget.writeMask = wgpu::ColorWriteMask::All;

    wgpu::FragmentState fragmentState{};
    fragmentState.module = shaderModule;
    fragmentState.entryPoint = "fs_main";
    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTarget;

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
    layoutDesc.bindGroupLayouts = reinterpret_cast<WGPUBindGroupLayout*>(&bind_group_layout_);
    wgpu::PipelineLayout pipelineLayout = app_->device_.createPipelineLayout(layoutDesc);

    // Render pipeline
    wgpu::RenderPipelineDescriptor pipelineDesc{};
    wgpu::DepthStencilState depth = create_default_depth_state();
    depth.depthBias = 4; // These help with z-fighting between edges & faces
    depth.depthBiasSlopeScale = -1.5f;
    depth.depthBiasClamp = 0.0f;
    pipelineDesc.depthStencil = &depth;
    pipelineDesc.layout = pipelineLayout;
    pipelineDesc.vertex = vertexState;
    pipelineDesc.fragment = &fragmentState;
    pipelineDesc.primitive = primitive;
    pipelineDesc.multisample = multisample;
    pipelineDesc.label = "Edge Line Pipeline";

    edges_pipeline_ = app_->device_.createRenderPipeline(pipelineDesc);
}

void VolumeMeshRenderer::create_face_triangle_pipeline()
{
    if (face_triangles_pipeline_ || n_faces_==0) {return;}

    wgpu::ShaderModule shaderModule = create_mesh_shader_module(app_->device_, face_shader_wgsl, "Face Triangle");

    // 0 -- Vertex Index
    wgpu::VertexAttribute attrs[2]{};
    attrs[0].shaderLocation = 0;
    attrs[0].offset = offsetof(FaceHandle, vertex_index_);;
    attrs[0].format = wgpu::VertexFormat::Uint32;

    // 1 -- Face Index
    attrs[1].shaderLocation = 1;
    attrs[1].offset = offsetof(FaceHandle, face_index_);
    attrs[1].format = wgpu::VertexFormat::Uint32;

    wgpu::VertexBufferLayout vertexBufferLayout{};
    vertexBufferLayout.arrayStride = sizeof(FaceHandle);
    vertexBufferLayout.attributeCount = 2;
    vertexBufferLayout.attributes = attrs;
    vertexBufferLayout.stepMode = wgpu::VertexStepMode::Vertex;

    // // Vertex state
    wgpu::VertexState vertexState{};
    vertexState.module = shaderModule;
    vertexState.entryPoint = "vs_main";
    vertexState.bufferCount = 1;
    vertexState.buffers = &vertexBufferLayout;

    // Fragment state
    wgpu::ColorTargetState colorTarget{};
    wgpu::SurfaceCapabilities surf_caps;
    app_->surface_.getCapabilities(app_->adapter_, &surf_caps);
    colorTarget.format = surf_caps.formats[0];
    colorTarget.writeMask = wgpu::ColorWriteMask::All;

    wgpu::FragmentState fragmentState{};
    fragmentState.module = shaderModule;
    fragmentState.entryPoint = "fs_main";
    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTarget;

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
    layoutDesc.bindGroupLayouts = reinterpret_cast<WGPUBindGroupLayout*>(&bind_group_layout_);
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
    pipelineDesc.label = "Face Triangle Pipeline";

    face_triangles_pipeline_ = app_->device_.createRenderPipeline(pipelineDesc);
}

void VolumeMeshRenderer::create_cell_triangle_pipeline()
{
    if (cell_triangles_pipeline_ || n_cells_==0) {return;}

    wgpu::ShaderModule shaderModule = create_mesh_shader_module(app_->device_, cell_shader_wgsl, "Cell Triangle");

    // 0 -- Vertex Index
    wgpu::VertexAttribute attrs[2]{};
    attrs[0].shaderLocation = 0;
    attrs[0].offset = offsetof(FaceHandle, vertex_index_);;
    attrs[0].format = wgpu::VertexFormat::Uint32;

    // 1 -- Cell Index
    attrs[1].shaderLocation = 1;
    attrs[1].offset = offsetof(CellHandle, cell_index_);
    attrs[1].format = wgpu::VertexFormat::Uint32;

    wgpu::VertexBufferLayout vertexBufferLayout{};
    vertexBufferLayout.arrayStride = sizeof(FaceHandle);
    vertexBufferLayout.attributeCount = 2;
    vertexBufferLayout.attributes = attrs;
    vertexBufferLayout.stepMode = wgpu::VertexStepMode::Vertex;

    // // Vertex state
    wgpu::VertexState vertexState{};
    vertexState.module = shaderModule;
    vertexState.entryPoint = "vs_main";
    vertexState.bufferCount = 1;
    vertexState.buffers = &vertexBufferLayout;

    // Fragment state
    wgpu::ColorTargetState colorTarget{};
    wgpu::SurfaceCapabilities surf_caps;
    app_->surface_.getCapabilities(app_->adapter_, &surf_caps);
    colorTarget.format = surf_caps.formats[0];
    colorTarget.writeMask = wgpu::ColorWriteMask::All;

    wgpu::FragmentState fragmentState{};
    fragmentState.module = shaderModule;
    fragmentState.entryPoint = "fs_main";
    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTarget;

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
    layoutDesc.bindGroupLayouts = reinterpret_cast<WGPUBindGroupLayout*>(&bind_group_layout_);
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
    pipelineDesc.label = "Cell Triangle Pipeline";

    cell_triangles_pipeline_ = app_->device_.createRenderPipeline(pipelineDesc);
}

void VolumeMeshRenderer::update_vertex_property_data(const std::vector<Property::Data>& _data)
{
    app_->device_.getQueue().writeBuffer(
        vertex_property_buffer_,
        0,
        _data.data(),
        sizeof(Property::Data) * _data.size()
        );
    std::cout << "Update Vertex Property Data" << std::endl;
}

void VolumeMeshRenderer::update_edge_property_data(const std::vector<Property::Data>& _data)
{
    app_->device_.getQueue().writeBuffer(
        edge_property_buffer_,
        0,
        _data.data(),
        sizeof(Property::Data) * _data.size()
        );
    std::cout << "Update Edge Property Data" << std::endl;
}

void VolumeMeshRenderer::update_face_property_data(const std::vector<Property::Data>& _data)
{
    app_->device_.getQueue().writeBuffer(
        face_property_buffer_,
        0,
        _data.data(),
        sizeof(Property::Data) * _data.size()
        );
    std::cout << "Update Face Property Data" << std::endl;
}

void VolumeMeshRenderer::update_cell_property_data(const std::vector<Property::Data>& _data)
{
    app_->device_.getQueue().writeBuffer(
        cell_property_buffer_,
        0,
        _data.data(),
        sizeof(Property::Data) * _data.size()
        );
    std::cout << "Update Cell Property Data" << std::endl;
}

void VolumeMeshRenderer::render(wgpu::RenderPassEncoder _render_pass, const Mat4x4f& _mvp)
{
    if (!render_anything_) {return;}
    wgpu::Queue queue = app_->device_.getQueue();

    // Update common uniforms
    queue.writeBuffer(uniform_buffer_, offsetof(Uniforms, mvp_), &_mvp, sizeof(Mat4x4f));
    int fbw, fbh;
    glfwGetFramebufferSize(app_->window(), &fbw, &fbh);
    Vec2f viewport_size = {fbw, fbh};
    queue.writeBuffer(uniform_buffer_, offsetof(Uniforms, viewport_size_), &viewport_size, sizeof(Vec2f));
    queue.writeBuffer(uniform_buffer_, offsetof(Uniforms, point_size_), &point_size_, sizeof(float));
    queue.writeBuffer(uniform_buffer_, offsetof(Uniforms, line_width_), &line_width_, sizeof(float));
    queue.writeBuffer(uniform_buffer_, offsetof(Uniforms, clip_box_), &clip_box_, sizeof(ClipBox));

    // Draw cells
    if (render_cells_ && n_cells_ > 0
        && cell_triangles_pipeline_)
    {
        // Set cell specific uniforms
        queue.writeBuffer(uniform_buffer_, offsetof(Uniforms, mode_), &cell_property_.mode_, sizeof(Property::Mode));
        queue.writeBuffer(uniform_buffer_, offsetof(Uniforms, cell_scale_), &cell_scale_, sizeof(float));

        _render_pass.setPipeline(cell_triangles_pipeline_);
        _render_pass.setBindGroup(0, bind_group_, 0, nullptr);
        _render_pass.setVertexBuffer(0, cellTriangleIndexBuffer_, 0, n_cell_triangle_indices_*sizeof(CellHandle));
        _render_pass.draw(n_cell_triangle_indices_, 1, 0, 0);
    }

    // Draw faces
    if (render_faces_ && n_faces_ > 0
        && face_triangles_pipeline_)
    {
        // Set face specific uniforms
        queue.writeBuffer(uniform_buffer_, offsetof(Uniforms, mode_), &face_property_.mode_, sizeof(Property::Mode));

        _render_pass.setPipeline(face_triangles_pipeline_);
        _render_pass.setBindGroup(0, bind_group_, 0, nullptr);
        _render_pass.setVertexBuffer(0, faceTriangleIndexBuffer_, 0, n_face_triangle_indices_*sizeof(FaceHandle));
        _render_pass.draw(n_face_triangle_indices_, 1, 0, 0);
    }

    // Draw edges
    if (render_edges_ && n_edges_ > 0
        && edges_pipeline_)
    {
        // Set edge specific uniforms
        queue.writeBuffer(uniform_buffer_, offsetof(Uniforms, mode_), &edge_property_.mode_, sizeof(Property::Mode));

        _render_pass.setPipeline(edges_pipeline_);
        _render_pass.setBindGroup(0, bind_group_, 0, nullptr);
        _render_pass.setVertexBuffer(0, edgeIndexBuffer_, 0, n_edges_*sizeof(EdgeInstance));

        // An Edge Instance is rendered as a quad
        _render_pass.draw(4, n_edges_, 0, 0);
    }

    // Draw points
    if (render_vertices_ && n_vertices_ > 0
        && vertices_pipeline_)
    {
        // Set vertex specific uniforms
        queue.writeBuffer(uniform_buffer_, offsetof(Uniforms, mode_), &vertex_property_.mode_, sizeof(Property::Mode));
        queue.writeBuffer(uniform_buffer_, offsetof(Uniforms, filter_), &vertex_property_.filter_, sizeof(Property::Filter));

        _render_pass.setPipeline(vertices_pipeline_);
        _render_pass.setBindGroup(0, bind_group_, 0, nullptr);
        //_render_pass.draw(n_vertex_point_indices_, 1, 0, 0);

        // Instancing. For each instance, we draw 4 vertices
        // that form a quad
        _render_pass.draw(4, n_vertices_, 0, 0);
    }
}

void VolumeMeshRenderer::release()
{
    positionBuffer_.release();
    vertexIndexBuffer_.release();
    edgeIndexBuffer_.release();
    faceTriangleIndexBuffer_.release();
    cellTriangleIndexBuffer_.release();
    cell_incenter_buffer_.release();

    vertex_property_buffer_.release();
    edge_property_buffer_.release();
    face_property_buffer_.release();
    cell_property_buffer_.release();
    uniform_buffer_.release();

    bind_group_.release();
}

}
