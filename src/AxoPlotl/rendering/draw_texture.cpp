#include "draw_texture.hpp"
#include <imgui.h>

namespace AxoPlotl
{

void draw_texture(const char* _name, wgpu::TextureView& _view, float _width, float _height)
{
    if (!_view) {return;}

    ImGui::Begin(_name);

    ImVec2 size = {_width, _height};
    if (_width <= 0.0f || _height <= 0.0f) {
        size = ImGui::GetContentRegionAvail();
    }

    WGPUTextureView raw_view = _view;

    ImGui::Image(
        (ImTextureID)(intptr_t)raw_view,
        size
    );

    ImGui::End();
}

}
