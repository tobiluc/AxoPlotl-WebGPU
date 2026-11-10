#include "draw_texture.hpp"
#include <imgui.h>

namespace AxoPlotl::internal
{

void draw_texture(wgpu::TextureView& _view, float _width, float _height)
{    
    if (!_view) {return;}
    ImVec2 size = {_width, _height};
    if (_width <= 0.0f || _height <= 0.0f) {
        size = ImGui::GetContentRegionAvail();
    }
    WGPUTextureView raw_view = _view;
    ImGui::Image(
        (ImTextureID)(intptr_t)raw_view,
        size
    );
}

}
