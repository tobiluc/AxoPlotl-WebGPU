#pragma once

#include <webgpu/webgpu.hpp>

namespace AxoPlotl
{

void draw_texture(const char* _name, wgpu::TextureView& _view, float _width = 0, float _height = 0);

}
