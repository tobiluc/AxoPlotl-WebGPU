#pragma once

#include <webgpu/webgpu.hpp>

namespace AxoPlotl::gui
{

void Texture(const wgpu::TextureView& _view,
        float _width = 0, float _height = 0);

}
