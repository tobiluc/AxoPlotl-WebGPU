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

inline wgpu::ShaderModule create_mesh_shader_module(
    wgpu::Device _device,
    const char* _src,
    const char* _name = "Unlabeled Shader Module")
{
    // Specify the WGSL part of the shader module descriptor
    wgpu::ShaderModuleWGSLDescriptor wgslDesc = {};
    wgslDesc.chain.sType = wgpu::SType::ShaderModuleWGSLDescriptor;
    wgslDesc.code = _src;

    wgpu::ShaderModuleDescriptor desc = {};
#ifdef WEBGPU_BACKEND_WGPU
    desc.hintCount = 0;
    desc.hints = nullptr;
#endif

    // Connect the chain
    desc.nextInChain = &wgslDesc.chain;
    desc.nextInChain = reinterpret_cast<const WGPUChainedStruct*>(&wgslDesc);
    desc.label = _name;

    return _device.createShaderModule(desc);
}

}
