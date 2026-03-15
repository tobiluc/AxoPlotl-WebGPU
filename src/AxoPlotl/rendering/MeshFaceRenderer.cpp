#include "MeshFaceRenderer.hpp"
#include "AxoPlotl/Application.hpp"
#include "AxoPlotl/rendering/detail/wgpu_commons.hpp"

namespace AxoPlotl
{


wgpu::RenderPipeline ColoredFacePropertyRenderer::pipeline_;
wgpu::BindGroupLayout ColoredFacePropertyRenderer::bind_group_layout_;

struct FaceIndex {
    uint32_t vh_;
    uint32_t fh_;
};

void ColoredFacePropertyRenderer::init(Application* _app,
    wgpu::Buffer _position_buffer,
    const std::vector<std::vector<uint32_t>>& _faces)
{
    property_color_map_.create(_app->device_);
    property_color_map_.set_coolwarm();
    app_ = _app;
    n_faces_ = _faces.size();
    n_positions_ = _position_buffer.getSize()/sizeof(Position);
    position_buffer_ = _position_buffer;
    create_buffers(_faces);
    create_bind_group_layout();
    create_bind_group();
    create_pipeline();
}

void ColoredFacePropertyRenderer::create_buffers(const std::vector<std::vector<uint32_t>> &_faces)
{
    wgpu::Device device = app_->device_;
    wgpu::Queue queue = device.getQueue();

    // Triangulate the Faces
    std::vector<FaceIndex> inds;
    inds.reserve(3*n_faces_);
    for (uint32_t fh = 0; fh < n_faces_; ++fh) {
        const auto& f = _faces[fh];
        for (int i = 1; i < f.size()-1; ++i) {
            inds.push_back({.vh_ = f[0], .fh_ = fh});
            inds.push_back({.vh_ = f[i], .fh_ = fh});
            inds.push_back({.vh_ = f[i+1], .fh_ = fh});
        }
    }
    n_indices_ = inds.size();

    // Face Index Buffer
    {
        wgpu::BufferDescriptor desc{};
        desc.usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
        desc.size = sizeof(FaceIndex) * inds.size();
        desc.mappedAtCreation = false;
        desc.label = "Mesh Face Index Buffer";

        face_index_buffer_ = device.createBuffer(desc);
        queue.writeBuffer(
            face_index_buffer_,
            0,
            inds.data(),
            sizeof(FaceIndex) * inds.size()
            );

        std::cout << desc.label << " Size: " << desc.size << std::endl;
    }

    // Property Buffer
    {
        wgpu::BufferDescriptor desc{};
        desc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
        desc.size = sizeof(Property::Data) * std::max(n_faces_,1lu);
        desc.mappedAtCreation = false;
        desc.label = "Mesh Face Property Buffer";

        property_buffer_ = device.createBuffer(desc);

        std::cout << desc.label << " Size: " << desc.size << std::endl;
    }

    // Uniform Buffer
    {
        wgpu::BufferDescriptor desc{};
        desc.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
        desc.size = sizeof(Uniforms);
        desc.mappedAtCreation = false;
        desc.label = "Mesh Face Uniform Buffer";

        uniform_buffer_ = device.createBuffer(desc);

        std::cout << desc.label << " Size: " << desc.size << std::endl;
    }
}

void ColoredFacePropertyRenderer::create_bind_group_layout()
{
    if (bind_group_layout_) {return;}

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

    bind_group_layout_ = app_->device_.createBindGroupLayout(layoutDesc);
}

void ColoredFacePropertyRenderer::create_bind_group()
{
    wgpu::BindGroupEntry groupEntries[5]{};

    // 0 - Uniform
    groupEntries[0].binding = 0;
    groupEntries[0].buffer = uniform_buffer_;
    groupEntries[0].offset = 0;
    groupEntries[0].size = sizeof(Uniforms);
    std::cout << "0: Mesh Face Uniforms #" << groupEntries[0].size << std::endl;

    // 1 - Positions
    groupEntries[1].binding = 1;
    groupEntries[1].buffer = position_buffer_;
    groupEntries[1].offset = 0;
    groupEntries[1].size = sizeof(Position) * n_positions_;
    std::cout << "1: Mesh Face Positions #" << groupEntries[1].size << std::endl;

    // 2 - Property Color Map
    groupEntries[2].binding = 2;
    groupEntries[2].textureView = property_color_map_.view_;
    std::cout << "2: Mesh Face Color Map #" << groupEntries[2].size << std::endl;

    // 3 - Color Map Sampler
    groupEntries[3].binding = 3;
    groupEntries[3].sampler = property_color_map_.sampler_;
    std::cout << "3: Mesh Face Color Sampler #" << groupEntries[3].size << std::endl;

    // 4 - Properties
    groupEntries[4].binding = 4;
    groupEntries[4].buffer = property_buffer_;
    groupEntries[4].offset = 0;
    groupEntries[4].size = sizeof(Property::Data) * std::max(n_faces_,1lu);
    std::cout << "4: Mesh Face Properties #" << groupEntries[4].size << std::endl;

    wgpu::BindGroupDescriptor bgDesc{};
    bgDesc.layout = bind_group_layout_;
    bgDesc.entryCount = 5;
    bgDesc.entries = groupEntries;

    bind_group_ = app_->device_.createBindGroup(bgDesc);
}

void ColoredFacePropertyRenderer::create_pipeline()
{
    if (pipeline_ || n_faces_==0) {return;}

    wgpu::ShaderModule shaderModule = create_mesh_shader_module_from_file(
        app_->device_,
        "mesh_face_shader.wgsl",
        "Face Shader");

    // 0 -- Vertex Index
    wgpu::VertexAttribute attrs[2]{};
    attrs[0].shaderLocation = 0;
    attrs[0].offset = offsetof(FaceIndex, vh_);;
    attrs[0].format = wgpu::VertexFormat::Uint32;

    // 1 -- Face Index
    attrs[1].shaderLocation = 1;
    attrs[1].offset = offsetof(FaceIndex, fh_);
    attrs[1].format = wgpu::VertexFormat::Uint32;

    wgpu::VertexBufferLayout vertexBufferLayout{};
    vertexBufferLayout.arrayStride = sizeof(FaceIndex);
    vertexBufferLayout.attributeCount = 2;
    vertexBufferLayout.attributes = attrs;
    vertexBufferLayout.stepMode = wgpu::VertexStepMode::Vertex;

    // // Vertex state
    wgpu::VertexState vertexState{};
    vertexState.module = shaderModule;
    vertexState.entryPoint = "vs_main";
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
    fragmentState.entryPoint = "fs_main";
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

    pipeline_ = app_->device_.createRenderPipeline(pipelineDesc);
}

void ColoredFacePropertyRenderer::update_property_data(const std::vector<Property::Data>& _data)
{
    app_->device_.getQueue().writeBuffer(
        property_buffer_,
        0,
        _data.data(),
        sizeof(Property::Data) * _data.size()
        );
    std::cout << "Update Face Property Data" << std::endl;
}

void ColoredFacePropertyRenderer::render(
    const Vec4f& _viewport,
    wgpu::RenderPassEncoder _render_pass,
    const Mat4x4f& _mvp)
{
    if (!enabled_ || n_faces_==0) {return;}

    // Update uniforms
    uniforms_.mvp_ = _mvp;
    uniforms_.viewport_size_ = {_viewport[2],_viewport[3]};
    app_->device_.getQueue().writeBuffer(
        uniform_buffer_, 0, &uniforms_, sizeof(Uniforms));

    _render_pass.setPipeline(pipeline_);
    _render_pass.setBindGroup(0, bind_group_, 0, nullptr);
    _render_pass.setVertexBuffer(0, face_index_buffer_, 0, n_indices_*sizeof(FaceIndex));
    _render_pass.draw(n_indices_, 1, 0, 0);
}

}
