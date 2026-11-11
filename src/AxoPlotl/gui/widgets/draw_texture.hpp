#pragma once

#include <webgpu/webgpu.hpp>

namespace AxoPlotl::internal
{

void draw_texture(const wgpu::TextureView& _view,
                  float _width = 0, float _height = 0);

}
