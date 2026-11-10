#include "save_texture_to_file.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <AxoPlotl/IO/stb_image_write.h>

namespace AxoPlotl
{

bool extract_texture_data(std::vector<uint8_t>& _data,
                            wgpu::Device _device, wgpu::Texture _texture,
                            uint32_t _width, uint32_t _height)
{
    const uint32_t bytes_per_pixel = 4;
    const uint32_t unpadded_bytes_per_row = _width * bytes_per_pixel;
    const uint32_t padded_bytes_per_row = (unpadded_bytes_per_row + 255) & ~255;
    const uint64_t buffer_size = static_cast<uint64_t>(padded_bytes_per_row) * _height;

    // Create Buffer
    wgpu::BufferDescriptor buffer_desc{};
    buffer_desc.size = buffer_size;
    buffer_desc.usage =
        wgpu::BufferUsage::CopyDst |
        wgpu::BufferUsage::MapRead;
    auto buffer = _device.createBuffer(buffer_desc);

    // Copy from the Texture to the Buffer
    wgpu::TexelCopyTextureInfo src{};
    src.texture = _texture;
    src.mipLevel = 0;
    src.origin = {0, 0, 0};
    src.aspect = wgpu::TextureAspect::All;

    wgpu::TexelCopyBufferInfo dst{};
    dst.buffer = buffer;
    dst.layout.offset = 0;
    dst.layout.bytesPerRow = padded_bytes_per_row;
    dst.layout.rowsPerImage = _height;

    wgpu::Extent3D extent{_width, _height, 1};

    // Submit
    wgpu::CommandEncoder encoder = _device.createCommandEncoder();
    encoder.copyTextureToBuffer(src, dst, extent);
    wgpu::CommandBuffer cb = encoder.finish();
    _device.getQueue().submit(1, &cb);
    cb.release();
    encoder.release();

    // Request mapping.
    struct Context {
        bool map_finished = false;
        bool map_succeeded = false;
    };


    auto on_buffer_mapped = [](
                                WGPUMapAsyncStatus status,
                                WGPUStringView message,
                                void* _user_data, void*)
    {
        Context* context = reinterpret_cast<Context*>(_user_data);
        context->map_succeeded = (status == wgpu::MapAsyncStatus::Success);
        context->map_finished = true;
    };

    Context context{};

    WGPUBufferMapCallbackInfo callback_info;
    callback_info.callback = on_buffer_mapped;
    callback_info.mode = WGPUCallbackMode_AllowSpontaneous;
    callback_info.userdata1 = (void*)&context;
    callback_info.userdata2 = nullptr;
    callback_info.nextInChain = nullptr;

    buffer.mapAsync(
        wgpu::MapMode::Read,
        0,
        buffer_size,
        callback_info
        );

    // Process GPU events until the callback has completed.
    // Exact Poll syntax depends on your wgpu-native version.
    while (!context.map_finished) {
        _device.poll(false, nullptr);
    }

    // Only access the buffer if mapping succeeded.
    if (!context.map_succeeded) {
        return false;
    }

    const void* mapped_data =
        buffer.getConstMappedRange(0, buffer_size);

    if (!mapped_data) {
        buffer.unmap();
        return false;
    }

    // Copy into CPU-owned memory before unmapping.
    _data.resize(buffer_size);
    std::memcpy(_data.data(), mapped_data, buffer_size);

    // Release the mapping.
    buffer.unmap();
    return true;
}

bool save_texture_to_file(const std::filesystem::path& _path, wgpu::Device _device, wgpu::Texture _texture, uint32_t _width, uint32_t _height)
{
    std::vector<uint8_t> data;
    if (!extract_texture_data(data, _device, _texture, _width, _height)) {
        return false;
    }
    return stbi_write_png(
        _path.string().c_str(),
        _width,
        _height,
        4,
        data.data(),
        _width * 4);
}

}
