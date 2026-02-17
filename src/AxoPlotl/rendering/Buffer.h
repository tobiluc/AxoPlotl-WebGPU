// #pragma once
// #include <webgpu/webgpu.hpp>

// namespace AxoPlotl
// {

// wgpu::Buffer createVertexBuffer(wgpu::Device device, const std::vector<float>& data, wgpu::BufferDescriptor& bufferDesc);

// wgpu::Buffer createIndexBuffer(wgpu::Device device, const std::vector<uint32_t> &data, wgpu::BufferDescriptor& bufferDesc);

// template<typename T>
// wgpu::Buffer createUniformBuffer(wgpu::Device device, size_t numElements, wgpu::BufferDescriptor& bufferDesc)
// {
//     bufferDesc.size = numElements * sizeof(T);
//     bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform;
//     bufferDesc.mappedAtCreation = false;
//     return device.createBuffer(bufferDesc);
// }

// }
