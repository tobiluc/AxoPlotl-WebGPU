#pragma once

#include <AxoPlotl/IO/read_shader.hpp>
#include <webgpu/webgpu.h>
#include <webgpu/webgpu.hpp>

namespace AxoPlotl
{

inline wgpu::DepthStencilState create_default_depth_state()
{
    wgpu::DepthStencilState depthStencilState;
    depthStencilState = wgpu::Default;
    depthStencilState.depthCompare = wgpu::CompareFunction::Less;
    depthStencilState.depthWriteEnabled = wgpu::OptionalBool::True;
    wgpu::TextureFormat depthTextureFormat = wgpu::TextureFormat::Depth24Plus;
    depthStencilState.format = depthTextureFormat;

    return depthStencilState;
}

inline wgpu::FragmentState create_fragment_state(
    wgpu::Surface _surface,
    wgpu::Adapter _adapter,
    wgpu::ShaderModule _shader_module)
{
    wgpu::ColorTargetState color_targets[2] = {};
    wgpu::SurfaceCapabilities surf_caps;
    _surface.getCapabilities(_adapter, &surf_caps);

    // color
    color_targets[0].format = surf_caps.formats[0];
    color_targets[0].writeMask = wgpu::ColorWriteMask::All;

    // picking
    color_targets[1].format = surf_caps.formats[1];
    color_targets[1].blend = nullptr;
    color_targets[1].writeMask = wgpu::ColorWriteMask::All;

    wgpu::FragmentState fragmentState{};
    fragmentState.module = _shader_module;
    fragmentState.entryPoint = wgpu::StringView("fs_main");
    fragmentState.targetCount = 2;
    fragmentState.targets = color_targets;

    return fragmentState;
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
    wgpu::ShaderSourceWGSL wgslDesc = {};
    wgslDesc.chain.sType = wgpu::SType::ShaderSourceWGSL;
    std::string parsed_shader_code = IO::parse_shader_file_with_includes(_path);
    wgslDesc.code = wgpu::StringView(parsed_shader_code.c_str());

    wgpu::ShaderModuleDescriptor desc = {};
// #ifdef WEBGPU_BACKEND_WGPU
//     desc.hintCount = 0;
//     desc.hints = nullptr;
// #endif

    // Connect the chain
    desc.nextInChain = &wgslDesc.chain;
    desc.nextInChain = reinterpret_cast<const WGPUChainedStruct*>(&wgslDesc);
    desc.label = wgpu::StringView(_name);

    return _device.createShaderModule(desc);
}

inline constexpr void destroy_buffer(wgpu::Buffer& _buffer) {
    if (_buffer) {
        _buffer.destroy();
        _buffer.release();
        _buffer = nullptr;
    }
}

}
