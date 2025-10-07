#include "Pipeline.h"

namespace AxoPlotl
{

wgpu::RenderPipeline createPipeline(wgpu::Device device, wgpu::ShaderModule shaderModule, wgpu::TextureFormat colorFormat)
{
    // Pipeline
    wgpu::RenderPipelineDescriptor desc;

    //--------------
    // Vertex
    //--------------
    wgpu::VertexBufferLayout vertexBufferLayout;
    wgpu::VertexAttribute attribs[2];

    // Position
    attribs[0].shaderLocation = 0;
    attribs[0].format = wgpu::VertexFormat::Float32x2;
    attribs[0].offset = 0;

    // Color
    attribs[1].shaderLocation = 1;
    attribs[1].format = wgpu::VertexFormat::Float32x3;
    attribs[1].offset = 2 * sizeof(float);

    // Two Attributes
    vertexBufferLayout.attributeCount = 2;
    vertexBufferLayout.attributes = attribs;

    vertexBufferLayout.arrayStride = (2+3) * sizeof(float);
    vertexBufferLayout.stepMode = wgpu::VertexStepMode::Vertex;

    desc.vertex.bufferCount = 1;
    desc.vertex.buffers = &vertexBufferLayout;

    // Vertex Shader
    desc.vertex.module = shaderModule;
    desc.vertex.entryPoint = "vs_main";
    desc.vertex.constantCount = 0;
    desc.vertex.constants = nullptr;

    //---------------
    // Primitive
    //---------------
    desc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
    desc.primitive.stripIndexFormat = wgpu::IndexFormat::Undefined;
    desc.primitive.frontFace = wgpu::FrontFace::CCW;
    desc.primitive.cullMode = wgpu::CullMode::None; // change for optimization

    //-------------
    // Fragment
    //-------------
    wgpu::FragmentState frag;
    frag.module = shaderModule;
    frag.entryPoint = "fs_main";
    frag.constantCount = 0;
    frag.constants = nullptr;

    // Blend
    wgpu::BlendState blendState;
    blendState.color.srcFactor = wgpu::BlendFactor::SrcAlpha;
    blendState.color.dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha;
    blendState.color.operation = wgpu::BlendOperation::Add;
    blendState.alpha.srcFactor = wgpu::BlendFactor::Zero;
    blendState.alpha.dstFactor = wgpu::BlendFactor::One;
    blendState.alpha.operation = wgpu::BlendOperation::Add;

    wgpu::ColorTargetState colorTarget;
    colorTarget.format = colorFormat;
    colorTarget.blend = &blendState;
    colorTarget.writeMask = wgpu::ColorWriteMask::All; // We could write to only some of the color channels.

    // We have only one target because our render pass has only one output color attachment.
    frag.targetCount = 1;
    frag.targets = &colorTarget;
    desc.fragment = &frag;

    //-----------
    // Depth
    //-----------
    desc.depthStencil = nullptr; // We do not use stencil/depth testing for now

    //-----------
    // Sampling
    //-----------
    desc.multisample.count = 1; // Samples per pixel
    desc.multisample.mask = ~0u; // "all bits on"
    desc.multisample.alphaToCoverageEnabled = false;
    desc.layout = nullptr;

    return device.createRenderPipeline(desc);

}

}
