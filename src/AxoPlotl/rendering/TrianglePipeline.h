// #pragma once
// #include <webgpu/webgpu.hpp>
// #include <glm/glm.hpp>

// namespace AxoPlotl
// {

// class Pipeline
// {
// public:
//     Pipeline();

//     ~Pipeline();

//     void release();

//     void init(wgpu::Device device, wgpu::Queue queue, wgpu::ShaderModule shaderModule, wgpu::TextureFormat colorFormat);

//     void render(wgpu::RenderPassEncoder& renderPass, wgpu::Queue& queue);

//     void updateProjection(float aspectRatio);

// private:
//     wgpu::RenderPipeline pipeline;
//     wgpu::PipelineLayout layout;
//     wgpu::BindGroupLayout bindGroupLayout;
//     wgpu::BindGroup bindGroup;

//     wgpu::Buffer uniformBuffer;

//     wgpu::Buffer vertexBuffer;
//     uint32_t vertexCount = 0;
//     wgpu::Buffer indexBuffer;
//     uint32_t indexCount = 0;

//     typedef struct {
//         glm::mat4x4 mvp;
//     } Uniforms;
//     Uniforms uniforms;
// };


// }
