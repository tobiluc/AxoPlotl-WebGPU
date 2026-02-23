#include "VectorRenderer.hpp"
#include "AxoPlotl/Application.hpp"
#include "AxoPlotl/rendering/detail/create_static_render_data.hpp"
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
    @align(16) vecScale:f32
};

@group(0) @binding(0) var<uniform> ubo : Uniforms;
@group(0) @binding(1) var<storage, read> positions : array<vec3<f32>>;
@group(0) @binding(2) var<storage, read> vectors : array<vec3<f32>>;

struct VSOut {
    @builtin(position) position : vec4<f32>,
    @location(0) color : vec4<f32>
};

@vertex
fn vs_main(
    @builtin(vertex_index) vid : u32,
    @builtin(instance_index) iid : u32
) -> VSOut {
    var out : VSOut;

    var pos:vec3<f32> = positions[iid];

    // A Point Instance is rendered as a Line in some direction
    if (vid == 0u) {}
    if (vid == 1u) {
        pos += ubo.vecScale * vectors[iid];
    }

    out.position = ubo.mvp * vec4<f32>(pos, 1.0);
    out.color = vec4<f32>(normalize(pos),1.0);

    return out;
}

@fragment
fn fs_main(in : VSOut) -> @location(0) vec4<f32>
{
    return in.color;
}
)";

wgpu::RenderPipeline VectorRenderer::pipeline_;
wgpu::BindGroupLayout VectorRenderer::bind_group_layout_;

void VectorRenderer::init(Application* _app,
    const std::vector<Position>& _positions)
{
    app_ = _app;
    n_positions_ = _positions.size();
    create_buffers(_positions);
    create_bind_group_layout();
    create_bind_group();
    create_pipeline();
}

void VectorRenderer::create_buffers(const std::vector<Position>& _positions)
{
    wgpu::Device device = app_->device_;
    wgpu::Queue queue = device.getQueue();

    // Position Buffer
    position_buffer_ = create_position_buffer(device, _positions);

    // Vector Buffer
    {
        wgpu::BufferDescriptor desc{};
        desc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
        desc.size = sizeof(Vec4f) * std::max(n_positions_,1lu);
        desc.mappedAtCreation = false;
        desc.label = "Vector Buffer";

        vector_buffer_ = device.createBuffer(desc);

        std::cout << desc.label << " Size: " << desc.size << std::endl;
    }

    // Uniform Buffer
    {
        wgpu::BufferDescriptor desc{};
        desc.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
        desc.size = sizeof(Uniforms);
        desc.mappedAtCreation = false;
        desc.label = "Vector Uniform Buffer";

        uniform_buffer_ = device.createBuffer(desc);

        std::cout << desc.label << " Size: " << desc.size << std::endl;
    }
}

void VectorRenderer::create_bind_group_layout()
{
    if (bind_group_layout_) {return;}

    wgpu::BindGroupLayoutEntry entries[3]{};

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

    // 2 - Vectors
    entries[2].binding = 2;
    entries[2].visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
    entries[2].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
    entries[2].buffer.minBindingSize = sizeof(Property::Data);

    wgpu::BindGroupLayoutDescriptor layoutDesc{};
    layoutDesc.entryCount = 3;
    layoutDesc.entries = entries;

    bind_group_layout_ = app_->device_.createBindGroupLayout(layoutDesc);
}

void VectorRenderer::create_bind_group()
{
    wgpu::BindGroupEntry groupEntries[3]{};

    // 0 - Uniform
    groupEntries[0].binding = 0;
    groupEntries[0].buffer = uniform_buffer_;
    groupEntries[0].offset = 0;
    groupEntries[0].size = sizeof(Uniforms);
    std::cout << "0: Vector Uniforms #" << groupEntries[0].size << std::endl;

    // 1 - Positions
    groupEntries[1].binding = 1;
    groupEntries[1].buffer = position_buffer_;
    groupEntries[1].offset = 0;
    groupEntries[1].size = sizeof(Position) * std::max(n_positions_,1lu);
    std::cout << "1: Vector Positions #" << groupEntries[1].size << std::endl;

    // 2 - Vectors
    groupEntries[2].binding = 2;
    groupEntries[2].buffer = vector_buffer_;
    groupEntries[2].offset = 0;
    groupEntries[2].size = sizeof(Vec4f) * std::max(n_positions_,1lu);
    std::cout << "2: Vectors #" << groupEntries[2].size << std::endl;

    wgpu::BindGroupDescriptor bgDesc{};
    bgDesc.layout = bind_group_layout_;
    bgDesc.entryCount = 3;
    bgDesc.entries = groupEntries;

    bind_group_ = app_->device_.createBindGroup(bgDesc);
}

void VectorRenderer::create_pipeline()
{
    if (pipeline_ || n_positions_==0) {return;}

    wgpu::ShaderModule shaderModule = create_mesh_shader_module(app_->device_,
        shader_src, "Vector Shader");

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
    pipelineDesc.label = "Vector Pipeline";

    pipeline_ = app_->device_.createRenderPipeline(pipelineDesc);
}

void VectorRenderer::update_vector_data(const std::vector<Vec4f>& _data)
{
    app_->device_.getQueue().writeBuffer(
        vector_buffer_,
        0,
        _data.data(),
        sizeof(Vec4f) * _data.size()
        );
    std::cout << "Update Vector Data" << std::endl;
}

void VectorRenderer::render(
    const Vec4f& _viewport,
    wgpu::RenderPassEncoder _render_pass,
    const Mat4x4f& _mvp)
{
    if (!enabled_ || n_positions_==0) {return;}

    // Update uniforms
    uniforms_.mvp_ = _mvp;
    uniforms_.viewport_size_ = {_viewport[2],_viewport[3]};
    app_->device_.getQueue().writeBuffer(
        uniform_buffer_, 0, &uniforms_, sizeof(Uniforms));

    _render_pass.setPipeline(pipeline_);
    _render_pass.setBindGroup(0, bind_group_, 0, nullptr);
    _render_pass.draw(2, n_positions_, 0, 0);
}

}
