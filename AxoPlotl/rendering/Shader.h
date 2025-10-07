#pragma once
#include <filesystem>
#include <webgpu/webgpu.hpp>

namespace AxoPlotl
{

wgpu::ShaderModule loadShaderModule(wgpu::Device device, const std::filesystem::path& path);

}
