#include "MeshEdgeRenderer.hpp"
#include "AxoPlotl/Application.hpp"
#include "AxoPlotl/rendering/detail/wgpu_commons.hpp"

namespace AxoPlotl
{
PipelineState ColoredEdgeRenderer::pipeline_state_;

void ColoredEdgeRenderer::init(uint32_t _object_id, Application* _app,
                                       wgpu::Buffer _position_buffer,
                                       const std::vector<std::pair<uint32_t,uint32_t>>& _edges)
{
    pipeline_state_.set_device(_app->device_);

    object_id_ = _object_id;
    property_color_map_.create(_app->device_);
    property_color_map_.set_coolwarm();
    app_ = _app;
    n_edges_ = _edges.size();
    n_positions_ = _position_buffer.getSize()/sizeof(Position);
    position_buffer_ = _position_buffer;
    create_buffers(_edges);
    create_bind_group_layout();
    create_bind_group();
    create_pipeline();
}

void ColoredEdgeRenderer::clear()
{
    destroy_buffer(property_buffer_);
    destroy_buffer(edge_index_buffer_);
    destroy_buffer(uniform_buffer_);
}

void ColoredEdgeRenderer::create_buffers(const std::vector<std::pair<uint32_t,uint32_t>>& _edges)
{
    wgpu::Device device = app_->device_;
    wgpu::Queue queue = device.getQueue();

    // Edge Index Buffer
    {
        std::vector<EdgeInstance> instances;
        instances.reserve(n_edges_);
        for (uint32_t eh = 0; eh < n_edges_; ++eh) {
            instances.push_back({
                .vh0_=_edges[eh].first,
                .vh1_=_edges[eh].second,
                .eh_=eh
            });
        }

        wgpu::BufferDescriptor desc{};
        desc.usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
        desc.size = sizeof(EdgeInstance) * std::max(n_edges_,1lu);
        desc.mappedAtCreation = false;
        desc.label = wgpu::StringView("Mesh Edge Index Buffer");

        edge_index_buffer_ = device.createBuffer(desc);
        queue.writeBuffer(
            edge_index_buffer_,
            0,
            instances.data(),
            sizeof(EdgeInstance) * instances.size()
            );

        std::cout << desc.label.data << " Size: " << desc.size << std::endl;
    }

    // Property Buffer
    {
        wgpu::BufferDescriptor desc{};
        desc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
        desc.size = sizeof(Property::Data) * std::max(n_edges_,1lu);
        desc.mappedAtCreation = false;
        desc.label = wgpu::StringView("Mesh Edge Property Buffer");

        property_buffer_ = device.createBuffer(desc);

        std::cout << desc.label.data << " Size: " << desc.size << std::endl;
    }

    // Uniform Buffer
    {
        wgpu::BufferDescriptor desc{};
        desc.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
        desc.size = sizeof(Uniforms);
        desc.mappedAtCreation = false;
        desc.label = wgpu::StringView("Mesh Edge Uniform Buffer");

        uniform_buffer_ = device.createBuffer(desc);

        std::cout << desc.label.data << " Size: " << desc.size << std::endl;
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

    wgpu::BindGroupLayoutDescriptor layoutDesc{};
    layoutDesc.entryCount = 5;
    layoutDesc.entries = entries;
    layoutDesc.label = wgpu::StringView("Edge Bind Group Layout");

    pipeline_state_.bind_group_layout_ = app_->device_.createBindGroupLayout(layoutDesc);
}

void ColoredEdgeRenderer::create_bind_group()
{
    wgpu::BindGroupEntry groupEntries[5]{};

    // 0 - Uniform
    groupEntries[0].binding = 0;
    groupEntries[0].buffer = uniform_buffer_;
    groupEntries[0].offset = 0;
    groupEntries[0].size = sizeof(Uniforms);
    std::cout << "0: Mesh Edge Uniforms #" << groupEntries[0].size << std::endl;

    // 1 - Positions
    groupEntries[1].binding = 1;
    groupEntries[1].buffer = position_buffer_;
    groupEntries[1].offset = 0;
    groupEntries[1].size = sizeof(Position) * n_positions_;
    std::cout << "1: Mesh Edge Positions #" << groupEntries[1].size << std::endl;

    // 2 - Property Color Map
    groupEntries[2].binding = 2;
    groupEntries[2].textureView = property_color_map_.view_;
    std::cout << "2: Mesh Edge Color Map #" << groupEntries[2].size << std::endl;

    // 3 - Color Map Sampler
    groupEntries[3].binding = 3;
    groupEntries[3].sampler = property_color_map_.sampler_;
    std::cout << "3: Mesh Edge Color Sampler #" << groupEntries[3].size << std::endl;

    // 4 - Properties
    groupEntries[4].binding = 4;
    groupEntries[4].buffer = property_buffer_;
    groupEntries[4].offset = 0;
    groupEntries[4].size = sizeof(Property::Data) * std::max(n_edges_,1lu);
    std::cout << "4: Mesh Edge Properties #" << groupEntries[4].size << std::endl;

    wgpu::BindGroupDescriptor bgDesc{};
    bgDesc.layout = pipeline_state_.bind_group_layout_;
    bgDesc.entryCount = 5;
    bgDesc.entries = groupEntries;

    bind_group_ = app_->device_.createBindGroup(bgDesc);
}

void ColoredEdgeRenderer::create_pipeline()
{
    if (pipeline_state_.pipeline_ || n_edges_==0) {return;}

    wgpu::ShaderModule shaderModule = create_mesh_shader_module_from_file(
        app_->device_,
        "mesh_edge_shader.wgsl",
        "Mesh Edge Shader");

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
    pipelineDesc.label = wgpu::StringView("Mesh Edge Pipeline");

    pipeline_state_.pipeline_ = app_->device_.createRenderPipeline(pipelineDesc);
}

void ColoredEdgeRenderer::update_property_data(const std::vector<Property::Data>& _data)
{
    app_->device_.getQueue().writeBuffer(
        property_buffer_,
        0,
        _data.data(),
        sizeof(Property::Data) * _data.size()
        );
    std::cout << "Update Edge Property Data" << std::endl;
}

void ColoredEdgeRenderer::render(
    const Vec4f& _viewport,
    wgpu::RenderPassEncoder _render_pass,
    const Mat4x4f& _mvp)
{
    if (!enabled_ || n_edges_==0) {return;}

    // Update uniforms
    uniforms_.mvp_ = _mvp;
    uniforms_.viewport_size_ = {_viewport[2],_viewport[3]};
    uniforms_.object_id_ = object_id_;
    app_->device_.getQueue().writeBuffer(
        uniform_buffer_, 0, &uniforms_, sizeof(Uniforms));

    _render_pass.setPipeline(pipeline_state_.pipeline_);
    _render_pass.setBindGroup(0, bind_group_, 0, nullptr);
    _render_pass.setVertexBuffer(0, edge_index_buffer_, 0, n_edges_*sizeof(EdgeInstance));

    // An Edge Instance is rendered as a quad
    _render_pass.draw(4, n_edges_, 0, 0);
}

}
