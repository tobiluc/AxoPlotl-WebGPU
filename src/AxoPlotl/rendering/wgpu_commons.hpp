#pragma once

#include "AxoPlotl/rendering/MyGpuContext.hpp"
#include <AxoPlotl/IO/read_shader.hpp>
#include <webgpu/webgpu.h>
#include <webgpu/webgpu.hpp>

namespace AxoPlotl::internal
{

// class ChunkedBuffer
// {
// private:
//     std::vector<wgpu::Buffer> buffers_;
//     size_t total_size_;
//     size_t chunk_size_;
// public:
//     struct Options {
//         uint64_t total_size;
//         uint64_t chunk_size;
//         wgpu::BufferUsage usage;
//         const char* label;
//     };
//     ChunkedBuffer(const wgpu::Device& _device,
//                   const Options& _opt)
//     {
//         if (_opt.chunk_size == 0) {
//             throw std::invalid_argument("chunk size cannot be zero");
//         }
//         total_size_ = _opt.total_size;
//         chunk_size_ = _opt.chunk_size;
//         const uint64_t count = (_opt.total_size + _opt.chunk_size - 1) / _opt.chunk_size;

//         buffers_.reserve(count);

//         for (uint64_t i = 0; i < count; ++i) {
//             uint64_t size = std::min(
//                 _opt.chunk_size,
//                 _opt.total_size-i*_opt.chunk_size);
//             wgpu::BufferDescriptor desc{};
//             desc.size = size;
//             desc.usage = _opt.usage;
//             desc.label = wgpu::StringView(_opt.label);

//             buffers_.push_back(_device.createBuffer(desc));
//         }
//     }
//     ~ChunkedBuffer() {
//         destroy();
//     }
//     void destroy() {
//         for (const auto& buffer : buffers_) {
//             if (buffer) {
//                 buffer.destroy();
//                 buffer.release();
//             }
//         }
//     }
//     size_t size() const {
//         return total_size_;
//     }

//     size_t chunk_size() const {
//         return chunk_size_;
//     }

//     size_t chunk_count() const {
//         return buffers_.size();
//     }

//     const wgpu::Buffer& chunk(size_t _idx) const {
//         return buffers_.at(_idx);
//     }

//     void write(
//         const wgpu::Queue& _queue,
//         uint64_t _offset,
//         const void* _data,
//         uint64_t _size)
//     {
//         if (_offset + _size > total_size_) {
//             throw std::out_of_range("SplitBuffer write out of bounds");
//         }

//         const auto* src = static_cast<const uint8_t*>(_data);

//         while (_size > 0)
//         {
//             uint64_t chunkIndex = _offset / chunk_size_;
//             uint64_t localOffset = _offset % chunk_size_;

//             uint64_t available_size =
//                 chunk(chunkIndex).getSize() - localOffset;

//             uint64_t copySize = std::min(_size, available_size);

//             _queue.writeBuffer(
//                 buffers_[chunkIndex],
//                 localOffset,
//                 src,
//                 copySize);

//             _offset += copySize;
//             src += copySize;
//             _size -= copySize;
//         }
//     }
// };

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

template<typename... Res>
constexpr void save_destroy_release(Res&... _res) {
    ([&] {
        if (_res) {
            _res.destroy();
            _res.release();
            _res = nullptr;
        }
    }(), ...);
}

template<typename... Res>
constexpr void save_release(Res&... _res) {
    ([&] {
        if (_res) {
            _res.release();
            _res = nullptr;
        }
    }(), ...);
}

template<typename... Res>
constexpr void save_unconfigure_release(Res&... _res) {
    ([&] {
        if (_res) {
            _res.unconfigure();
            _res.release();
            _res = nullptr;
        }
    }(), ...);
}

template<typename T>
struct BufferData {
    wgpu::Buffer buffer;
    std::vector<T> data;
    ~BufferData() {cleanup();}
    void create_and_write(const wgpu::Device& _device) {
        save_destroy_release(buffer);
        buffer = create_and_write_bufferT<T>(_device, data);
    }
    void cleanup() {
        save_destroy_release(buffer);
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
