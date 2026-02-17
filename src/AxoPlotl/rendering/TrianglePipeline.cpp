// #include "TrianglePipeline.h"
// #include "Buffer.h"
// #include <glm/gtc/matrix_transform.hpp>
// #include <glm/gtc/type_ptr.hpp>
// #include "../geometry/mesh_adapter.h"

// namespace AxoPlotl
// {

// Pipeline::Pipeline()
// {}

// Pipeline::~Pipeline()
// {
//     //release();
// }

// void Pipeline::release()
// {
//     pipeline.release();
//     bindGroup.release();
//     layout.release();
//     bindGroupLayout.release();
//     uniformBuffer.release();
//     indexBuffer.release();
// }

// void Pipeline::init(wgpu::Device device, wgpu::Queue queue, wgpu::ShaderModule shaderModule, wgpu::TextureFormat colorFormat)
// {
//     // Pipeline
//     wgpu::RenderPipelineDescriptor desc;

//     //--------------
//     // Vertex
//     //--------------
//     wgpu::VertexBufferLayout vertexBufferLayout;
//     wgpu::VertexAttribute attribs[2];

//     // Position
//     attribs[0].shaderLocation = 0;
//     attribs[0].format = wgpu::VertexFormat::Float32x3;
//     attribs[0].offset = 0;

//     // Color
//     attribs[1].shaderLocation = 1;
//     attribs[1].format = wgpu::VertexFormat::Float32x3;
//     attribs[1].offset = 3 * sizeof(float);

//     // Two Attributes
//     vertexBufferLayout.attributeCount = 2;
//     vertexBufferLayout.attributes = attribs;

//     vertexBufferLayout.arrayStride = (3+3) * sizeof(float);
//     vertexBufferLayout.stepMode = wgpu::VertexStepMode::Vertex;

//     desc.vertex.bufferCount = 1;
//     desc.vertex.buffers = &vertexBufferLayout;

//     // Vertex Shader
//     desc.vertex.module = shaderModule;
//     desc.vertex.entryPoint = "vs_main";
//     desc.vertex.constantCount = 0;
//     desc.vertex.constants = nullptr;

//     //---------------
//     // Primitive
//     //---------------
//     desc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
//     desc.primitive.stripIndexFormat = wgpu::IndexFormat::Undefined;
//     desc.primitive.frontFace = wgpu::FrontFace::CCW;
//     desc.primitive.cullMode = wgpu::CullMode::None; // change for optimization

//     //-------------
//     // Fragment
//     //-------------
//     wgpu::FragmentState frag;
//     frag.module = shaderModule;
//     frag.entryPoint = "fs_main";
//     frag.constantCount = 0;
//     frag.constants = nullptr;

//     // Blend
//     wgpu::BlendState blendState;
//     blendState.color.srcFactor = wgpu::BlendFactor::SrcAlpha;
//     blendState.color.dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha;
//     blendState.color.operation = wgpu::BlendOperation::Add;
//     blendState.alpha.srcFactor = wgpu::BlendFactor::Zero;
//     blendState.alpha.dstFactor = wgpu::BlendFactor::One;
//     blendState.alpha.operation = wgpu::BlendOperation::Add;

//     wgpu::ColorTargetState colorTarget;
//     colorTarget.format = colorFormat;
//     colorTarget.blend = &blendState;
//     colorTarget.writeMask = wgpu::ColorWriteMask::All; // We could write to only some of the color channels.

//     // We have only one target because our render pass has only one output color attachment.
//     frag.targetCount = 1;
//     frag.targets = &colorTarget;
//     desc.fragment = &frag;

//     //-----------
//     // Depth
//     //-----------
//     wgpu::DepthStencilState depthStencilState = wgpu::Default;

//     depthStencilState.depthCompare = wgpu::CompareFunction::Less;
//     depthStencilState.depthWriteEnabled = true; // disable e.g. for ui elements
//     wgpu::TextureFormat depthTextureFormat = wgpu::TextureFormat::Depth24Plus; // 24 bits per pixel for depth, 8 for stencil
//     depthStencilState.format = depthTextureFormat;

//     // Deactivate the stencil alltogether
//     depthStencilState.stencilReadMask = 0;
//     depthStencilState.stencilWriteMask = 0;

//     desc.depthStencil = &depthStencilState;

//     //-----------
//     // Sampling
//     //-----------
//     desc.multisample.count = 1; // Samples per pixel
//     desc.multisample.mask = ~0u; // "all bits on"
//     desc.multisample.alphaToCoverageEnabled = false;

//     //------------
//     // Buffer
//     //------------
//     wgpu::BufferDescriptor bufferDesc;

//     //-----------------
//     // Load Mesh
//     PolyMesh mesh;
//     IO::loadMeshFromFile("/Users/tobiaskohler/Uni/HexHex/dataset/tet-ovmb/s17c_sphere_new_hex_igm.ovmb", mesh);

//     // Extract data

//     std::vector<uint32_t> indexData;
//     std::vector<float> vertexData;

//     for (auto f_it = mesh.f_iter(); f_it.is_valid(); ++f_it) {
//         std::vector<OVM::VH> vhs = mesh.get_halfface_vertices(f_it->halfface_handle(0));
//         for (uint i = 0; i < vhs.size(); ++i) {

//             // position
//             vertexData.push_back((float)mesh.vertex(vhs[i])[0]);
//             vertexData.push_back((float)mesh.vertex(vhs[i])[1]);
//             vertexData.push_back((float)mesh.vertex(vhs[i])[2]);

//             // color
//             vertexData.push_back((float)((i%3)==0));
//             vertexData.push_back((float)((i%3)==1));
//             vertexData.push_back((float)((i%3)==2));
//         }

//         // Triangulate face
//         uint o = indexData.size();
//         for (uint i = 0; i < vhs.size()-2; ++i) {
//             indexData.push_back(o + 0);
//             indexData.push_back(o + i + 1);
//             indexData.push_back(o + i + 2);
//         }
//     }

//     //-----------------

//     // Vertex Buffer
//     vertexCount = static_cast<uint32_t>(vertexData.size() / (3+3));
//     vertexBuffer = createVertexBuffer(device, vertexData, bufferDesc);
//     queue.writeBuffer(vertexBuffer, 0, vertexData.data(), bufferDesc.size);

//     // Index Buffer
//     indexCount = static_cast<uint32_t>(indexData.size());
//     indexBuffer = createIndexBuffer(device, indexData, bufferDesc);
//     queue.writeBuffer(indexBuffer, 0, indexData.data(), bufferDesc.size);

//     // Uniform Buffer
//     updateProjection(640.f/480.f);
//     uniformBuffer = createUniformBuffer<float>(device, 4*4, bufferDesc);
//     queue.writeBuffer(uniformBuffer, 0, &uniforms, bufferDesc.size);

//     //----------
//     // Layout
//     //----------

//     // Define binding layout (don't forget to = Default)
//     wgpu::BindGroupLayoutEntry bindingLayout = wgpu::Default;
//     bindingLayout.binding = 0; // @binding attribute in the shader
//     bindingLayout.visibility = wgpu::ShaderStage::Vertex; // Vertex Stage needs to access it
//     bindingLayout.buffer.type = wgpu::BufferBindingType::Uniform;
//     bindingLayout.buffer.minBindingSize = 16 * sizeof(float);
//     //bindingLayout.buffer.hasDynamicOffset = true;

//     // Create a bind group layout
//     wgpu::BindGroupLayoutDescriptor bindGroupLayoutDesc{};
//     bindGroupLayoutDesc.entryCount = 1;
//     bindGroupLayoutDesc.entries = &bindingLayout;
//     bindGroupLayout = device.createBindGroupLayout(bindGroupLayoutDesc);

//     // Create the pipeline layout
//     wgpu::PipelineLayoutDescriptor layoutDesc{};
//     layoutDesc.bindGroupLayoutCount = 1;
//     layoutDesc.bindGroupLayouts = (WGPUBindGroupLayout*)&bindGroupLayout;
//     layout = device.createPipelineLayout(layoutDesc);
//     desc.layout = layout;

//     pipeline = device.createRenderPipeline(desc);

//     //---------------
//     // Binding
//     //---------------

//     // Create a binding
//     wgpu::BindGroupEntry binding{};
//     binding.binding = 0;
//     binding.buffer = uniformBuffer;
//     binding.offset = 0;
//     binding.size = 16 * sizeof(float);

//     wgpu::BindGroupDescriptor bindGroupDesc{};
//     bindGroupDesc.layout = bindGroupLayout;
//     bindGroupDesc.entryCount = 1;
//     bindGroupDesc.entries = &binding;
//     bindGroup = device.createBindGroup(bindGroupDesc);
// }

// void Pipeline::render(wgpu::RenderPassEncoder& renderPass, wgpu::Queue &queue)
// {
//     // Update uniform
//     queue.writeBuffer(uniformBuffer, 0, &uniforms, sizeof(Uniforms));

//     renderPass.setPipeline(pipeline);

//     renderPass.setVertexBuffer(0, vertexBuffer, 0, vertexBuffer.getSize());
//     renderPass.setIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32, 0, indexBuffer.getSize());
//     renderPass.setBindGroup(0, bindGroup, 0, nullptr);

//     renderPass.drawIndexed(indexCount, 1, 0, 0, 0);
// }

// void Pipeline::updateProjection(float aspectRatio)
// {
//     glm::mat4 model = glm::mat4(1.0f);
//     glm::mat4 view = glm::lookAt(
//         glm::vec3(0, 0, 15),   // camera pos
//         glm::vec3(0, 0, 0),   // target
//         glm::vec3(0, 1, 0));  // up
//     glm::mat4 proj = glm::perspective(
//         glm::radians(45.0f),
//         aspectRatio,
//         0.01f, // near
//         1024.0f // far
//     );
//     proj[1][1] *= -1; // flip Y for WebGPU (Vulkan-style coords)

//     uniforms.mvp = proj * view * model;
// }

// }
