#include <AxoPlotl/rendering/SphericalHarmonicsRenderer.hpp>
#include <AxoPlotl/Application.hpp>
#include <AxoPlotl/rendering/detail/create_static_render_data.hpp>

namespace AxoPlotl
{

PipelineState SHRenderer::pipeline_state_;

void SHRenderer::init(
    uint32_t _object_id,
    Application* _app,
    size_t _resolution)
{
    resolution_ = _resolution;
    pipeline_state_.set_device(_app->device_);

    color_map_.create(_app->device_);
    color_map_.set_coolwarm();

    object_id_ = _object_id;
    app_ = _app;
    create_buffers();
    create_bind_group_layout();
    create_bind_group();
    create_pipeline();

    update([](const Vec3f& _pos) -> float32_t {
        return std::pow(_pos.x, 4) + std::pow(_pos.y, 4) + std::pow(_pos.z, 4);
    });
}

void SHRenderer::clear()
{
    destroy_buffer(vertex_buffer_);
    destroy_buffer(index_buffer_);
    destroy_buffer(uniform_buffer_);
}

void SHRenderer::create_buffers()
{
    wgpu::Device device = app_->device_;
    wgpu::Queue queue = device.getQueue();

    // Create the Mesh Data based on resolution_
    std::vector<uint32_t> indices;
    vertices_.clear();
    uint32_t stacks = resolution_;
    uint32_t slices = resolution_;
    for (uint32_t i = 0; i <= stacks; ++i) {
        float v = float(i) / stacks;
        float theta = v * M_PI;
        for (uint32_t j = 0; j <= slices; ++j) {
            float u = float(j) / slices;
            float phi = u * 2.0f * M_PI;

            float x = std::sin(theta) * std::cos(phi);
            float y = std::cos(theta);
            float z = std::sin(theta) * std::sin(phi);

            vertices_.emplace_back();
            vertices_.back().position_ = {x, y, z};
            vertices_.back().value_ = std::pow(x,4)+std::pow(y,4)+std::pow(z,4);
        }
    }
    for (uint32_t i = 0; i < stacks; ++i) {
        for (uint32_t j = 0; j < slices; ++j) {
            uint32_t row1 = i * (slices + 1);
            uint32_t row2 = (i + 1) * (slices + 1);

            uint32_t a = row1 + j;
            uint32_t b = row1 + j + 1;
            uint32_t c = row2 + j;
            uint32_t d = row2 + j + 1;

            indices.push_back(a);
            indices.push_back(c);
            indices.push_back(b);

            indices.push_back(b);
            indices.push_back(c);
            indices.push_back(d);
        }
    }

    n_indices_ = indices.size();

    // Vertex Buffer
    {
        wgpu::BufferDescriptor desc{};
        desc.usage =
            wgpu::BufferUsage::CopyDst |
            wgpu::BufferUsage::Vertex;
        desc.size = sizeof(Vertex) * vertices_.size();
        desc.mappedAtCreation = false;
        desc.label = wgpu::StringView("SH Vertex");

        vertex_buffer_ = device.createBuffer(desc);
        queue.writeBuffer(vertex_buffer_, 0, vertices_.data(), desc.size);
    }

    // Index Buffer
    {
        wgpu::BufferDescriptor desc{};
        desc.usage = wgpu::BufferUsage::Index | wgpu::BufferUsage::CopyDst;
        desc.size = sizeof(uint32_t) * indices.size();
        desc.mappedAtCreation = false;
        desc.label = wgpu::StringView("SH Index");

        index_buffer_ = device.createBuffer(desc);
        queue.writeBuffer(
            index_buffer_,
            0,
            indices.data(),
            sizeof(uint32_t) * indices.size()
            );

        //std::cout << desc.label.data << " Size: " << desc.size << std::endl;
    }

    // Uniform Buffer
    {
        wgpu::BufferDescriptor desc{};
        desc.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
        desc.size = sizeof(Uniforms);
        desc.mappedAtCreation = false;
        desc.label = wgpu::StringView("SH Uniform Buffer");

        uniform_buffer_ = device.createBuffer(desc);

        //std::cout << desc.label.data << " Size: " << desc.size << std::endl;
    }
}

void SHRenderer::create_bind_group_layout()
{
    if (pipeline_state_.bind_group_layout_) {return;}

    wgpu::BindGroupLayoutEntry entries[3]{};

    // 0 - Uniform
    entries[0].binding = 0;
    entries[0].visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
    entries[0].buffer.type = wgpu::BufferBindingType::Uniform;
    entries[0].buffer.minBindingSize = sizeof(Uniforms);

    // 2 - Color Map
    entries[1].binding = 1;
    entries[1].visibility = wgpu::ShaderStage::Fragment;
    entries[1].texture.sampleType = wgpu::TextureSampleType::Float;
    entries[1].texture.viewDimension = wgpu::TextureViewDimension::_2D;

    // 3 - Color Sampler
    entries[2].binding = 2;
    entries[2].visibility = wgpu::ShaderStage::Fragment;
    entries[2].sampler.type = wgpu::SamplerBindingType::Filtering;

    wgpu::BindGroupLayoutDescriptor layoutDesc{};
    layoutDesc.entryCount = 3;
    layoutDesc.entries = entries;
    layoutDesc.label = wgpu::StringView("SH Bind Group Layout");

    pipeline_state_.bind_group_layout_ = app_->device_.createBindGroupLayout(layoutDesc);
}

void SHRenderer::create_bind_group()
{
    wgpu::BindGroupEntry groupEntries[3]{};

    // 0 - Uniform
    groupEntries[0].binding = 0;
    groupEntries[0].buffer = uniform_buffer_;
    groupEntries[0].offset = 0;
    groupEntries[0].size = sizeof(Uniforms);
    std::cout << "0: SH Uniforms #" << groupEntries[0].size << std::endl;

    // 2 - Property Color Map
    groupEntries[1].binding = 1;
    groupEntries[1].textureView = color_map_.view_;
    //std::cout << "2: Mesh Vertex Color Map #" << groupEntries[2].size << std::endl;

    // 3 - Color Map Sampler
    groupEntries[2].binding = 2;
    groupEntries[2].sampler = color_map_.sampler_;
    //std::cout << "3: Mesh Vertex Color Sampler #" << groupEntries[3].size << std::endl;

    wgpu::BindGroupDescriptor bgDesc{};
    bgDesc.layout = pipeline_state_.bind_group_layout_;
    bgDesc.entryCount = 3;
    bgDesc.entries = groupEntries;

    bind_group_ = app_->device_.createBindGroup(bgDesc);
}

void SHRenderer::create_pipeline()
{
    if (pipeline_state_.pipeline_) {return;}

    wgpu::ShaderModule shaderModule = create_mesh_shader_module_from_file(
        app_->device_,
        "spherical_harmonics_shader.wgsl",
        "SH Shader");

    // 0 -- Vertex Position
    wgpu::VertexAttribute attrs[2]{};
    attrs[0].shaderLocation = 0;
    attrs[0].offset = offsetof(Vertex, position_);
    attrs[0].format = wgpu::VertexFormat::Float32x3;

    // 1 -- Vertex Value
    attrs[1].shaderLocation = 1;
    attrs[1].offset = offsetof(Vertex, value_);
    attrs[1].format = wgpu::VertexFormat::Float32;

    wgpu::VertexBufferLayout vertexBufferLayout{};
    vertexBufferLayout.arrayStride = sizeof(Vertex);
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
    layoutDesc.bindGroupLayouts = reinterpret_cast<WGPUBindGroupLayout*>(&pipeline_state_.bind_group_layout_);
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
    pipelineDesc.label = wgpu::StringView("SH Triangle Pipeline");

    pipeline_state_.pipeline_ = app_->device_.createRenderPipeline(pipelineDesc);
}

void SHRenderer::update(const std::function<float32_t (const Vec3f &)> &_f)
{
    uniforms_.value_range_.x = std::numeric_limits<float>::infinity();
    uniforms_.value_range_.y = -std::numeric_limits<float>::infinity();
    for (int i = 0; i < vertices_.size(); ++i)
    {
        vertices_[i].value_ = _f(vertices_[i].position_);
        uniforms_.value_range_.x = std::min(uniforms_.value_range_.x, vertices_[i].value_);
        uniforms_.value_range_.y = std::max(uniforms_.value_range_.y, vertices_[i].value_);
    }

    app_->device_.getQueue().writeBuffer(
        vertex_buffer_,
        0,
        vertices_.data(),
        sizeof(Vertex) * vertices_.size()
        );
    //std::cout << "Update SH Vertex Data" << std::endl;
}

void SHRenderer::render(
    const Vec4f& _viewport,
    wgpu::RenderPassEncoder _render_pass,
    const Mat4x4f& _mvp)
{
    if (!enabled_) {return;}

    // Update uniforms
    uniforms_.mvp_ = _mvp;
    uniforms_.object_id_ = object_id_;
    app_->device_.getQueue().writeBuffer(
        uniform_buffer_, 0, &uniforms_, sizeof(Uniforms));

    _render_pass.setPipeline(pipeline_state_.pipeline_);
    _render_pass.setBindGroup(0, bind_group_, 0, nullptr);
    _render_pass.setVertexBuffer(0, vertex_buffer_, 0, vertices_.size()*sizeof(Vertex));
    _render_pass.setIndexBuffer(index_buffer_, wgpu::IndexFormat::Uint32, 0, n_indices_*sizeof(uint32_t));
    _render_pass.drawIndexed(n_indices_, 1, 0, 0, 0);
    //_render_pass.draw(n_indices_, 1, 0, 0);
}

}
