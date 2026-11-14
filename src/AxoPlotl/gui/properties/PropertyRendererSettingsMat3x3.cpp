#include "PropertyRendererSettingsMat3x3.hpp"
#include "AxoPlotl/gui/widgets/draw_basic_renderer_settings.hpp"
#include "AxoPlotl/gui/widgets/input_slider.hpp"
#include <imgui.h>

namespace AxoPlotl::internal
{


PropertySettingsMat3x3::PropertySettingsMat3x3(Matrix3x3Renderer& _renderer) :
    renderer_(_renderer)
{
    set_default_settings();
}

void PropertySettingsMat3x3::render_ui()
{
    draw_basic_renderer_settings(renderer_);
}

void PropertySettingsMat3x3::set_default_settings() {
    renderer_.vector_scale() = 1.0f;
    renderer_.draw_symmetric() = true;
    renderer_.normalize() = false;
}

}
