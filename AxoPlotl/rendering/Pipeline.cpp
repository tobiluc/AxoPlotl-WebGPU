#include "Pipeline.h"
#include "Buffer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace AxoPlotl
{

Pipeline::Pipeline()
{}

Pipeline::~Pipeline()
{
    //release();
}

void Pipeline::release()
{
    pipeline.release();
    bindGroup.release();
    layout.release();
    bindGroupLayout.release();
    uniformBuffer.release();
    indexBuffer.release();
}

void Pipeline::init(wgpu::Device device, wgpu::Queue queue, wgpu::ShaderModule shaderModule, wgpu::TextureFormat colorFormat)
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
    attribs[0].format = wgpu::VertexFormat::Float32x3;
    attribs[0].offset = 0;

    // Color
    attribs[1].shaderLocation = 1;
    attribs[1].format = wgpu::VertexFormat::Float32x3;
    attribs[1].offset = 3 * sizeof(float);

    // Two Attributes
    vertexBufferLayout.attributeCount = 2;
    vertexBufferLayout.attributes = attribs;

    vertexBufferLayout.arrayStride = (3+3) * sizeof(float);
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

    //------------
    // Buffer
    //------------
    wgpu::BufferDescriptor bufferDesc;

    // Vertex Buffer
    std::vector<float> vertexData = {
        // x0, y0
        -0.5, -0.5, 0.0,  1.0,0.0,0.0,
        // x1, y1
        +0.5, -0.5, 0.0, 0.0,1.0,0.0,
        // x2, y2
        +0.0, +0.5,0.0,  0.0,0.0,1.0,

        // Add a second triangle:
        -0.55f, -0.5, 0.0,1.0,0.0,0.0,
        -0.05f, +0.5, 0.0,0.0,1.0,0.0,
        -0.55f, +0.5, 0.0, 0.0,0.0,1.0
    };
    vertexCount = static_cast<uint32_t>(vertexData.size() / (3+3));
    vertexBuffer = createVertexBuffer(device, vertexData, bufferDesc);
    queue.writeBuffer(vertexBuffer, 0, vertexData.data(), bufferDesc.size);

    // Index Buffer
    std::vector<uint32_t> indexData = {
        0, 1, 2
    };
    indexCount = static_cast<uint32_t>(indexData.size());
    indexBuffer = createIndexBuffer(device, indexData, bufferDesc);
    queue.writeBuffer(indexBuffer, 0, indexData.data(), bufferDesc.size);

    // Uniform Buffer
    updateProjection(640.f/480.f);
    uniformBuffer = createUniformBuffer<float>(device, 4*4, bufferDesc);
    queue.writeBuffer(uniformBuffer, 0, &uniforms, bufferDesc.size);

    //----------
    // Layout
    //----------

    // Define binding layout (don't forget to = Default)
    wgpu::BindGroupLayoutEntry bindingLayout = wgpu::Default;
    bindingLayout.binding = 0; // @binding attribute in the shader
    bindingLayout.visibility = wgpu::ShaderStage::Vertex; // Vertex Stage needs to access it
    bindingLayout.buffer.type = wgpu::BufferBindingType::Uniform;
    bindingLayout.buffer.minBindingSize = 16 * sizeof(float);
    //bindingLayout.buffer.hasDynamicOffset = true;

    // Create a bind group layout
    wgpu::BindGroupLayoutDescriptor bindGroupLayoutDesc{};
    bindGroupLayoutDesc.entryCount = 1;
    bindGroupLayoutDesc.entries = &bindingLayout;
    bindGroupLayout = device.createBindGroupLayout(bindGroupLayoutDesc);

    // Create the pipeline layout
    wgpu::PipelineLayoutDescriptor layoutDesc{};
    layoutDesc.bindGroupLayoutCount = 1;
    layoutDesc.bindGroupLayouts = (WGPUBindGroupLayout*)&bindGroupLayout;
    layout = device.createPipelineLayout(layoutDesc);
    desc.layout = layout;

    pipeline = device.createRenderPipeline(desc);

    //---------------
    // Binding
    //---------------

    // Create a binding
    wgpu::BindGroupEntry binding{};
    binding.binding = 0;
    binding.buffer = uniformBuffer;
    binding.offset = 0;
    binding.size = 16 * sizeof(float);

    wgpu::BindGroupDescriptor bindGroupDesc{};
    bindGroupDesc.layout = bindGroupLayout;
    bindGroupDesc.entryCount = 1;
    bindGroupDesc.entries = &binding;
    bindGroup = device.createBindGroup(bindGroupDesc);
}

void Pipeline::render(wgpu::RenderPassEncoder& renderPass, wgpu::Queue &queue)
{
    // Update uniform
    queue.writeBuffer(uniformBuffer, 0, &uniforms, sizeof(Uniforms));

    renderPass.setPipeline(pipeline);

    renderPass.setVertexBuffer(0, vertexBuffer, 0, vertexBuffer.getSize());
    renderPass.setIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32, 0, indexBuffer.getSize());
    renderPass.setBindGroup(0, bindGroup, 0, nullptr);

    renderPass.drawIndexed(indexCount, 1, 0, 0, 0);
}

void Pipeline::updateProjection(float aspectRatio)
{
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::lookAt(
        glm::vec3(0, 0, 2),   // camera pos
        glm::vec3(0, 0, 0),   // target
        glm::vec3(0, 1, 0));  // up
    glm::mat4 proj = glm::perspective(
        glm::radians(45.0f),
        aspectRatio,
        0.1f,
        10.0f);
    proj[1][1] *= -1; // flip Y for WebGPU (Vulkan-style coords)

    uniforms.mvp = proj * view * model;
}

}
