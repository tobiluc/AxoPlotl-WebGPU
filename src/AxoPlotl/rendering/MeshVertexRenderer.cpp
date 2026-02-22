#include "MeshVertexRenderer.hpp"
#include "AxoPlotl/Application.hpp"
#include "AxoPlotl/rendering/detail/wgpu_commons.hpp"

namespace AxoPlotl
{

static const char* shader_src = R"(
#include "VolumeMeshCommons.wgsl"

struct Uniforms {
    @align(16) mvp : mat4x4<f32>,
    @align(16) viewportSize: vec2<f32>,
    @align(16) pointSize: f32,
    @align(16) clipBox: ClipBox,
    @align(16) mode:Mode,
    @align(16) valueFilter: vec2<f32>
};

@group(0) @binding(0) var<uniform> ubo : Uniforms;
@group(0) @binding(1) var<storage, read> positions : array<vec3<f32>>;
@group(0) @binding(2) var colorMap : texture_2d<f32>;
@group(0) @binding(3) var colorSampler : sampler;
@group(0) @binding(4) var<storage, read> props : array<Property>;

struct VSOut {
    @builtin(position) position : vec4<f32>,
    @location(0) value : vec4<f32>,
    @location(1) corner : vec2<f32>
};

@vertex
fn vs_main(
    @builtin(vertex_index) vid : u32,
    @builtin(instance_index) iid : u32
) -> VSOut {
    var out : VSOut;

    let pos = positions[iid];
    out.position = ubo.mvp * vec4<f32>(pos, 1.0);

    // A Point Instance is rendered as a Quad
    out.corner = vec2<f32>(0.0);
    if (vid == 0u) {out.corner = vec2(-1.0, -1.0);}
    if (vid == 1u) {out.corner = vec2( 1.0, -1.0);}
    if (vid == 2u) {out.corner = vec2(-1.0,  1.0);}
    if (vid == 3u) {out.corner = vec2( 1.0,  1.0);}

    // Add Offset
    out.position += vec4<f32>(
        out.corner.x * (ubo.pointSize / ubo.viewportSize.x) * out.position.w,
        out.corner.y * (ubo.pointSize / ubo.viewportSize.y) * out.position.w,
        0.0,
        0.0
    );

    let value = props[iid].value;

    if (isOutsideClipBox(pos, ubo.clipBox)
|| (ubo.mode==1u && isOutsideRange(value.x, ubo.valueFilter))) {
        out.position = clippedPosition();
    }
    out.value = value;


    return out;
}

@fragment
fn fs_main(in : VSOut) -> @location(0) vec4<f32>
{
    if (length(in.corner) > 1.0) {discard;} //round
    return getFragmentColorFromPropertyValue(
in.value, ubo.mode, ubo.valueFilter, colorMap, colorSampler);
}
)";

wgpu::RenderPipeline MeshVertexRenderer::pipeline_;
wgpu::BindGroupLayout MeshVertexRenderer::bind_group_layout_;

void MeshVertexRenderer::init(Application* _app,
          wgpu::Buffer _position_buffer,
          const std::vector<uint32_t>& _indices)
{
    property_color_map_.create(_app->device_);
    property_color_map_.set_coolwarm();
    app_ = _app;
    n_vertices_ = _indices.size();
    n_positions_ = _position_buffer.getSize()/sizeof(Position);
    position_buffer_ = _position_buffer;
    create_buffers(_indices);
    create_bind_group_layout();
    create_bind_group();
    create_pipeline();
}

void MeshVertexRenderer::create_buffers(const std::vector<uint32_t> &_indices)
{
    wgpu::Device device = app_->device_;
    wgpu::Queue queue = device.getQueue();

    // Vertex Index Buffer
    if (n_vertices_ > 0)
    {
        wgpu::BufferDescriptor desc{};
        desc.usage = wgpu::BufferUsage::Index | wgpu::BufferUsage::CopyDst;
        desc.size = sizeof(uint32_t) * n_vertices_;
        desc.mappedAtCreation = false;
        desc.label = "Vertex Index";

        vertex_index_buffer_ = device.createBuffer(desc);
        queue.writeBuffer(
            vertex_index_buffer_,
            0,
            _indices.data(),
            sizeof(uint32_t) * _indices.size()
            );

        std::cout << "Vertex Index Buffer Size: " << desc.size << std::endl;
    }

    // Property Buffer
    if (n_vertices_ > 0)
    {
        wgpu::BufferDescriptor desc{};
        desc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
        desc.size = sizeof(Property::Data) * n_vertices_;
        desc.mappedAtCreation = false;
        desc.label = "Vertex Property";

        property_buffer_ = device.createBuffer(desc);

        std::cout << "Vertex Property Buffer Size: " << desc.size << std::endl;
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

void MeshVertexRenderer::create_bind_group_layout()
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

void MeshVertexRenderer::create_bind_group()
{
    wgpu::BindGroupEntry groupEntries[5]{};

    // 0 - Uniform
    groupEntries[0].binding = 0;
    groupEntries[0].buffer = uniform_buffer_;
    groupEntries[0].offset = 0;
    groupEntries[0].size = sizeof(Uniforms);
    std::cout << "0: Uniforms #" << groupEntries[0].size << std::endl;

    // 1 - Positions
    groupEntries[1].binding = 1;
    groupEntries[1].buffer = position_buffer_;
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

    // 4 - Properties
    groupEntries[4].binding = 4;
    groupEntries[4].buffer = property_buffer_;
    groupEntries[4].offset = 0;
    groupEntries[4].size = sizeof(Property::Data) * n_vertices_;
    std::cout << "4: Vertex Properties #" << groupEntries[4].size << std::endl;

    wgpu::BindGroupDescriptor bgDesc{};
    bgDesc.layout = bind_group_layout_;
    bgDesc.entryCount = 5;
    bgDesc.entries = groupEntries;

    bind_group_ = app_->device_.createBindGroup(bgDesc);
}

void MeshVertexRenderer::create_pipeline()
{
    if (pipeline_ || n_vertices_==0) {return;}

    wgpu::ShaderModule shaderModule = create_mesh_shader_module(
        app_->device_, shader_src, "Mesh Vertex Shader");

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

    pipeline_ = app_->device_.createRenderPipeline(pipelineDesc);
}

void MeshVertexRenderer::update_property_data(const std::vector<Property::Data>& _data)
{
    app_->device_.getQueue().writeBuffer(
        property_buffer_,
        0,
        _data.data(),
        sizeof(Property::Data) * _data.size()
        );
    std::cout << "Update Vertex Property Data" << std::endl;
}

void MeshVertexRenderer::render(
    const Vec4f& _viewport,
    wgpu::RenderPassEncoder _render_pass,
    const Mat4x4f& _mvp)
{
    if (!enabled_ || n_vertices_==0) {return;}

    // Update uniforms
    uniforms_.mvp_ = _mvp;
    uniforms_.viewport_size_ = {_viewport[2],_viewport[3]};
    app_->device_.getQueue().writeBuffer(
        uniform_buffer_, 0, &uniforms_, sizeof(Uniforms));

    // Instancing. For each instance, we draw 4 vertices
    // that form a quad
    _render_pass.setPipeline(pipeline_);
    _render_pass.setBindGroup(0, bind_group_, 0, nullptr);
    _render_pass.draw(4, n_vertices_, 0, 0);
}

}
