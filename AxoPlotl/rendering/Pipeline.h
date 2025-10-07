#pragma once
#include <webgpu/webgpu.hpp>

namespace AxoPlotl
{
    wgpu::RenderPipeline createPipeline(wgpu::Device device, wgpu::ShaderModule shaderModule, wgpu::TextureFormat colorFormat);

}
