#include "Buffer.h"

namespace AxoPlotl
{

wgpu::Buffer createVertexBuffer(wgpu::Device device, const std::vector<float>& data)
{
    wgpu::BufferDescriptor desc;
    desc.size = data.size() * sizeof(float);
    desc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex; // Vertex usage here!
    desc.mappedAtCreation = false;
    return device.createBuffer(desc);
}

wgpu::Buffer createIndexBuffer(wgpu::Device device, const std::vector<uint32_t> &data)
{
    wgpu::BufferDescriptor desc;
    desc.size = data.size() * sizeof(uint32_t);
    desc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Index;
    return device.createBuffer(desc);
}

}
