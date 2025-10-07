#pragma once
#include <webgpu/webgpu.hpp>

namespace AxoPlotl
{

wgpu::Buffer createVertexBuffer(wgpu::Device device, const std::vector<float>& data);

wgpu::Buffer createIndexBuffer(wgpu::Device device, const std::vector<uint32_t> &data);

}
