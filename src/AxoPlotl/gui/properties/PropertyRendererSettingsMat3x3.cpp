#include "PropertyRendererSettingsMat3x3.hpp"
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
    bool b = renderer_.normalize();
    ImGui::Checkbox("Normalize", &b);
    renderer_.normalize() = b;

    b = renderer_.draw_symmetric();
    ImGui::Checkbox("Symmetric", &b);
    renderer_.draw_symmetric() = b;

    InputSlider("Vector Scale", &renderer_.vector_scale());
}

void PropertySettingsMat3x3::set_default_settings() {
    renderer_.vector_scale() = 1.0f;
    renderer_.draw_symmetric() = true;
    renderer_.normalize() = false;
}

}
