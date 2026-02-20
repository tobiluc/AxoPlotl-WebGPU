#pragma once

#include "webgpu/webgpu.hpp"

namespace AxoPlotl
{
inline wgpu::DepthStencilState create_default_depth_state()
{
    wgpu::DepthStencilState depthStencilState;
    depthStencilState = wgpu::Default;
    depthStencilState.depthCompare = wgpu::CompareFunction::Less;
    depthStencilState.depthWriteEnabled = true;
    wgpu::TextureFormat depthTextureFormat = wgpu::TextureFormat::Depth24Plus;
    depthStencilState.format = depthTextureFormat;
    return depthStencilState;
}
}
