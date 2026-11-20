#pragma once

#include "AxoPlotl/rendering/MyGpuContext.hpp"
#include <AxoPlotl/IO/read_shader.hpp>
#include <webgpu/webgpu.h>
#include <webgpu/webgpu.hpp>

namespace AxoPlotl::internal
{

template<typename T>
wgpu::Buffer create_and_write_bufferT(
    const wgpu::Device& _device,
    const std::vector<T>& _data,
    const WGPUBufferUsage _usage =
    wgpu::BufferUsage::Storage |
    wgpu::BufferUsage::CopyDst |
    wgpu::BufferUsage::Vertex,
    const std::string& _name = "Buffer T")
{
    wgpu::BufferDescriptor desc{};
    desc.usage =_usage;
    desc.size = sizeof(T)*std::max(_data.size(), 1lu);
    desc.mappedAtCreation = false;
    desc.label = wgpu::StringView(_name);
    wgpu::Buffer buffer = _device.createBuffer(desc);
    if (!_data.empty()) [[likely]] {
        _device.getQueue().writeBuffer(buffer, 0, _data.data(), buffer.getSize());
    }
    return buffer;
}

inline constexpr void destroy_buffer(wgpu::Buffer& _buffer) {
    if (_buffer) {
        _buffer.destroy();
        _buffer.release();
        _buffer = nullptr;
    }
}

template<typename T>
struct BufferData {
    wgpu::Buffer buffer;
    std::vector<T> data;
    ~BufferData() {destroy();}
    void create_and_write(const wgpu::Device& _device) {
        destroy_buffer(buffer);
        buffer = create_and_write_bufferT<T>(_device, data);
    }
    void destroy() {
        destroy_buffer(buffer);
        data.clear();
    }
};


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
    const RenderContext& _render_context,
    wgpu::ShaderModule _shader_module,
    wgpu::ColorTargetState* _color_targets,
    const char* _entry_point = "fs_main")
{
    // color
    _color_targets[0].format = _render_context.surface_format;
    _color_targets[0].writeMask = wgpu::ColorWriteMask::All;

    // picking
    _color_targets[1].format = RenderContext::picking_format;
    _color_targets[1].blend = nullptr;
    _color_targets[1].writeMask = wgpu::ColorWriteMask::All;

    wgpu::FragmentState fragmentState{};
    fragmentState.module = _shader_module;
    fragmentState.entryPoint = wgpu::StringView(_entry_point);
    fragmentState.targetCount = 2;
    fragmentState.targets = _color_targets;

    return fragmentState;
}

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

}
