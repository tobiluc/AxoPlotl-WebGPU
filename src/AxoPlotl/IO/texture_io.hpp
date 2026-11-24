#pragma once

#include <filesystem>
#include <webgpu/webgpu.hpp>

namespace AxoPlotl
{

bool extract_texture_data(std::vector<uint8_t>& _image_data,
                          wgpu::Device _device, wgpu::Texture _texture,
                          uint32_t _width, uint32_t _height);

bool save_texture_data_to_file(const std::filesystem::path& _path,
                                const std::vector<uint8_t>& _data,
                            const uint32_t _width, const uint32_t _height);

bool save_texture_to_file(const std::filesystem::path& _path,
    wgpu::Device _device, wgpu::Texture _texture,
    uint32_t _width, uint32_t _height,
    std::optional<wgpu::Texture> _mask = std::nullopt);

}
