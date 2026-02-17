#include "VolumeMeshRenderer.hpp"
#include "AxoPlotl/rendering/shaders/edge_shader_wgsl.hpp"
#include "AxoPlotl/rendering/shaders/vertex_shader_wgsl.hpp"
#include "glm/gtc/type_ptr.hpp"

static wgpu::ShaderModule create_shader_module(
    wgpu::Device _device, const char* _src, const char* _name = "Unlabeled Shader Module")
{
    // Specify the WGSL part of the shader module descriptor
    wgpu::ShaderModuleWGSLDescriptor wgslDesc = {};
    wgslDesc.chain.sType = wgpu::SType::ShaderModuleWGSLDescriptor;
    wgslDesc.code = _src;

    wgpu::ShaderModuleDescriptor desc = {};
#ifdef WEBGPU_BACKEND_WGPU
    desc.hintCount = 0;
    desc.hints = nullptr;
#endif

    // Connect the chain
    desc.nextInChain = &wgslDesc.chain;
    desc.nextInChain = reinterpret_cast<const WGPUChainedStruct*>(&wgslDesc);
    desc.label = _name;

    return _device.createShaderModule(desc);
}

namespace AxoPlotl
{

VolumeMeshRenderer::VolumeMeshRenderer(Context _context) :
    context_(_context)
{}

void VolumeMeshRenderer::init(const StaticData& _data)
{
    n_vertex_point_indices_ = _data.vertex_draw_indices_.size();
    n_edge_line_indices_ = _data.edge_draw_indices_.size();
    n_vertices_ = n_vertex_point_indices_;
    n_edges_ = n_edge_line_indices_/2;
    n_positions_ = _data.positions_.size();

    create_buffers(_data);
    create_bind_group_layout();
    create_bind_group();

    create_vertex_point_pipeline();
    create_edge_line_pipeline();
}

void VolumeMeshRenderer::create_buffers(const StaticData &_data)
{
    wgpu::Device device = context_.device_;
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

    // Vertex Index Buffer
    {
        wgpu::BufferDescriptor desc{};
        desc.usage = wgpu::BufferUsage::Index | wgpu::BufferUsage::CopyDst;
        desc.size = sizeof(uint32_t) * _data.vertex_draw_indices_.size();
        desc.mappedAtCreation = false;
        desc.label = "Vertex Index";

        vertexIndexBuffer_ = device.createBuffer(desc);
        queue.writeBuffer(
            vertexIndexBuffer_,
            0,
            _data.vertex_draw_indices_.data(),
            desc.size
            );

        std::cout << "Vertex Index Buffer Size: " << desc.size << std::endl;
    }

    // Edge Index Buffer
    {
        wgpu::BufferDescriptor desc{};
        desc.usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
        desc.size = sizeof(EdgeHandle) * _data.edge_draw_indices_.size();
        desc.mappedAtCreation = false;
        desc.label = "Edge Index";

        edgeIndexBuffer_ = device.createBuffer(desc);
        queue.writeBuffer(
            edgeIndexBuffer_,
            0,
            _data.edge_draw_indices_.data(),
            desc.size
            );

        std::cout << "Edge Index Buffer Size: " << desc.size << std::endl;
    }

    // Vertex Property Buffer
    {
        wgpu::BufferDescriptor desc{};
        desc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
        desc.size = sizeof(VertexPropertyData) * n_vertices_;
        desc.mappedAtCreation = false;
        desc.label = "Vertex Property";

        vertexPropertyBuffer_ = device.createBuffer(desc);

        std::cout << "Vertex Property Buffer Size: " << desc.size << std::endl;
    }

    // Edge Property Buffer
    {
        wgpu::BufferDescriptor desc{};
        desc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
        desc.size = sizeof(EdgePropertyData) * n_edges_;
        desc.mappedAtCreation = false;
        desc.label = "Edge Property";

        edgePropertyBuffer_ = device.createBuffer(desc);

        std::cout << "Edge Property Buffer Size: " << desc.size << std::endl;
    }

    // Uniform Buffer
    {
        wgpu::BufferDescriptor desc{};
        desc.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
        desc.size = sizeof(Uniforms);
        desc.mappedAtCreation = false;
        desc.label = "Uniform";

        uniformBuffer_ = device.createBuffer(desc);

        std::cout << "Uniform Buffer Size: " << desc.size << std::endl;
    }
}

void VolumeMeshRenderer::create_bind_group_layout()
{
    wgpu::BindGroupLayoutEntry entries[4]{};

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

    // 2 - Vertex Properties
    entries[2].binding = 2;
    entries[2].visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
    entries[2].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
    entries[2].buffer.minBindingSize = sizeof(VertexPropertyData);

    // 3 - Edge Properties
    entries[3].binding = 3;
    entries[3].visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
    entries[3].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
    entries[3].buffer.minBindingSize = sizeof(EdgePropertyData);

    wgpu::BindGroupLayoutDescriptor layoutDesc{};
    layoutDesc.entryCount = 4;
    layoutDesc.entries = entries;

    bind_group_layput_ = context_.device_.createBindGroupLayout(layoutDesc);
}

void VolumeMeshRenderer::create_bind_group()
{
    wgpu::BindGroupEntry groupEntries[4]{};

    // 0 - Uniform
    groupEntries[0].binding = 0;
    groupEntries[0].buffer = uniformBuffer_;
    groupEntries[0].offset = 0;
    groupEntries[0].size = sizeof(Uniforms);
    std::cout << "0: Uniforms #" << groupEntries[0].size << std::endl;

    // 1 - Positions
    groupEntries[1].binding = 1;
    groupEntries[1].buffer = positionBuffer_;
    groupEntries[1].offset = 0;
    groupEntries[1].size = sizeof(Position) * n_positions_;
    std::cout << "1: Positions #" << groupEntries[1].size << std::endl;

    // 2 - Vertex Properties
    groupEntries[2].binding = 2;
    groupEntries[2].buffer = vertexPropertyBuffer_;
    groupEntries[2].offset = 0;
    groupEntries[2].size = sizeof(VertexPropertyData) * n_vertices_;
    std::cout << "2: Vertex Properties #" << groupEntries[2].size << std::endl;

    // 3 - Edge Properties
    groupEntries[3].binding = 3;
    groupEntries[3].buffer = edgePropertyBuffer_;
    groupEntries[3].offset = 0;
    groupEntries[3].size = sizeof(EdgePropertyData) * n_edges_;
    std::cout << "3: Edge Properties #" << groupEntries[3].size << std::endl;

    wgpu::BindGroupDescriptor bgDesc{};
    bgDesc.layout = bind_group_layput_;
    bgDesc.entryCount = 4;
    bgDesc.entries = groupEntries;

    bind_group_ = context_.device_.createBindGroup(bgDesc);
}

void VolumeMeshRenderer::create_vertex_point_pipeline()
{
    wgpu::ShaderModule shaderModule = create_shader_module(context_.device_, vertex_shader_wgsl, "Vertex Point");

    // Vertex state (no vertex buffer)
    wgpu::VertexState vertexState{};
    vertexState.module = shaderModule;
    vertexState.entryPoint = "vs_main";
    vertexState.bufferCount = 0;
    vertexState.buffers = nullptr;

    // Fragment state
    wgpu::ColorTargetState colorTarget{};
    wgpu::SurfaceCapabilities surf_caps;
    context_.surface_.getCapabilities(context_.adapter_, &surf_caps);
    colorTarget.format = surf_caps.formats[0];
    colorTarget.writeMask = wgpu::ColorWriteMask::All;

    wgpu::FragmentState fragmentState{};
    fragmentState.module = shaderModule;
    fragmentState.entryPoint = "fs_main";
    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTarget;

    // Primitive state
    wgpu::PrimitiveState primitive{};
    primitive.topology = wgpu::PrimitiveTopology::PointList;
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
    layoutDesc.bindGroupLayouts = reinterpret_cast<WGPUBindGroupLayout*>(&bind_group_layput_);

    // Pipeline
    wgpu::RenderPipelineDescriptor pipelineDesc{};
    pipelineDesc.layout = context_.device_.createPipelineLayout(layoutDesc);
    pipelineDesc.vertex = vertexState;
    pipelineDesc.fragment = &fragmentState;
    pipelineDesc.primitive = primitive;
    pipelineDesc.multisample = multisample;
    pipelineDesc.label = "Vertex Point Pipeline";

    vertex_points_pipeline_ = context_.device_.createRenderPipeline(pipelineDesc);
}

void VolumeMeshRenderer::create_edge_line_pipeline()
{
    wgpu::ShaderModule shaderModule = create_shader_module(context_.device_, edge_shader_wgsl, "Edge Line");

    // 0 -- Vertex Index
    wgpu::VertexAttribute attrs[2]{};
    attrs[0].shaderLocation = 0;
    attrs[0].offset = 0;
    attrs[0].format = wgpu::VertexFormat::Uint32;

    // 1 -- Edge Index
    attrs[1].shaderLocation = 1;
    attrs[1].offset = sizeof(uint32_t);//(void*)offsetof(EdgeHandle, edge_index);;
    attrs[1].format = wgpu::VertexFormat::Uint32;

    wgpu::VertexBufferLayout vertexBufferLayout{};
    vertexBufferLayout.arrayStride = sizeof(EdgeHandle);
    vertexBufferLayout.attributeCount = 2;
    vertexBufferLayout.attributes = attrs;
    vertexBufferLayout.stepMode = wgpu::VertexStepMode::Vertex;

    // Vertex state
    wgpu::VertexState vertexState{};
    vertexState.module = shaderModule;
    vertexState.entryPoint = "vs_main";
    vertexState.bufferCount = 1;
    vertexState.buffers = &vertexBufferLayout;

    // Fragment state
    wgpu::ColorTargetState colorTarget{};
    wgpu::SurfaceCapabilities surf_caps;
    context_.surface_.getCapabilities(context_.adapter_, &surf_caps);
    colorTarget.format = surf_caps.formats[0];
    colorTarget.writeMask = wgpu::ColorWriteMask::All;

    wgpu::FragmentState fragmentState{};
    fragmentState.module = shaderModule;
    fragmentState.entryPoint = "fs_main";
    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTarget;

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
    layoutDesc.bindGroupLayouts = reinterpret_cast<WGPUBindGroupLayout*>(&bind_group_layput_);
    wgpu::PipelineLayout pipelineLayout = context_.device_.createPipelineLayout(layoutDesc);

    // Render pipeline
    wgpu::RenderPipelineDescriptor pipelineDesc{};
    pipelineDesc.layout = pipelineLayout;
    pipelineDesc.vertex = vertexState;
    pipelineDesc.fragment = &fragmentState;
    pipelineDesc.primitive = primitive;
    pipelineDesc.multisample = multisample;

    edge_lines_pipeline_ = context_.device_.createRenderPipeline(pipelineDesc);
}

void VolumeMeshRenderer::update_vertex_property_data(const std::vector<VertexPropertyData>& _data)
{
    context_.device_.getQueue().writeBuffer(
        vertexPropertyBuffer_,
        0,
        _data.data(),
        sizeof(VertexPropertyData) * _data.size()
        );
    std::cout << "Update Vertex Property Data" << std::endl;
}

void VolumeMeshRenderer::update_edge_property_data(const std::vector<EdgePropertyData>& _data)
{
    context_.device_.getQueue().writeBuffer(
        edgePropertyBuffer_,
        0,
        _data.data(),
        sizeof(EdgePropertyData) * _data.size()
        );
    std::cout << "Update Edge Property Data" << std::endl;
}

void VolumeMeshRenderer::render(wgpu::RenderPassEncoder _render_pass, const Mat4x4f& _mvp)
{
    // Update uniforms
    unforms_.mvp_ = _mvp;
    context_.device_.getQueue().writeBuffer(uniformBuffer_, 0, &unforms_, sizeof(Uniforms));

    // Draw points
    _render_pass.setPipeline(vertex_points_pipeline_);
    _render_pass.setBindGroup(0, bind_group_, 0, nullptr);
    _render_pass.draw(n_vertex_point_indices_, 1, 0, 0);

    // Draw edges
    _render_pass.setPipeline(edge_lines_pipeline_);
    _render_pass.setBindGroup(0, bind_group_, 0, nullptr);
    _render_pass.setVertexBuffer(0, edgeIndexBuffer_, 0, n_edge_line_indices_*sizeof(EdgeHandle));
    _render_pass.draw(n_edge_line_indices_, 1, 0, 0);
}

}
