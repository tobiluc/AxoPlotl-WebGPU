#pragma once

#include "AxoPlotl/rendering/shaders/includes/shader_includes.hpp"
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

// inline wgpu::ShaderModule create_mesh_shader_module(
//     wgpu::Device _device,
//     const std::string& _source,
//     const char* _name = "Unlabeled Shader Module")
// {
//     // Specify the WGSL part of the shader module descriptor
//     wgpu::ShaderModuleWGSLDescriptor wgslDesc = {};
//     wgslDesc.chain.sType = wgpu::SType::ShaderModuleWGSLDescriptor;
//     std::string parsed_shader_code = parse_shader_with_includes(_source);
//     wgslDesc.code = parsed_shader_code.c_str();

//     wgpu::ShaderModuleDescriptor desc = {};
// #ifdef WEBGPU_BACKEND_WGPU
//     desc.hintCount = 0;
//     desc.hints = nullptr;
// #endif

//     // Connect the chain
//     desc.nextInChain = &wgslDesc.chain;
//     desc.nextInChain = reinterpret_cast<const WGPUChainedStruct*>(&wgslDesc);
//     desc.label = _name;

//     return _device.createShaderModule(desc);
// }

inline wgpu::ShaderModule create_mesh_shader_module_from_file(
    wgpu::Device _device,
    const std::filesystem::path& _path,
    const char* _name = "Unlabeled Shader Module")
{
    // Specify the WGSL part of the shader module descriptor
    wgpu::ShaderModuleWGSLDescriptor wgslDesc = {};
    wgslDesc.chain.sType = wgpu::SType::ShaderModuleWGSLDescriptor;
    std::string parsed_shader_code = parse_shader_file_with_includes(_path);
    wgslDesc.code = parsed_shader_code.c_str();

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
