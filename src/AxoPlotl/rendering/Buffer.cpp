// #include "Buffer.h"

// namespace AxoPlotl
// {

// wgpu::Buffer createVertexBuffer(wgpu::Device device, const std::vector<float>& data, wgpu::BufferDescriptor& bufferDesc)
// {
//     bufferDesc.size = data.size() * sizeof(float);
//     bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex; // Vertex usage here!
//     bufferDesc.mappedAtCreation = false;
//     return device.createBuffer(bufferDesc);
// }

// wgpu::Buffer createIndexBuffer(wgpu::Device device, const std::vector<uint32_t> &data, wgpu::BufferDescriptor& bufferDesc)
// {
//     bufferDesc.size = data.size() * sizeof(uint32_t);
//     bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Index;
//     return device.createBuffer(bufferDesc);
// }

// }
