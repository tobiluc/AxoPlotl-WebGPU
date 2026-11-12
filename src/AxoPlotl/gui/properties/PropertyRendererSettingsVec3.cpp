#include "PropertyRendererSettingsVec3.hpp"
#include "AxoPlotl/gui/widgets/input_slider.hpp"

namespace AxoPlotl::internal
{

PropertySettingsVec3::PropertySettingsVec3(Vector3Renderer& _renderer) :
    renderer_(_renderer)
{
    set_default_settings();
}

void PropertySettingsVec3::render_ui()
{
    InputSlider("Vector Scale", &renderer_.vector_scale());
}

void PropertySettingsVec3::set_default_settings() {
    renderer_.vector_scale() = 1.0f;
}

}
