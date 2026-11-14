#include "PropertyRendererSettingsVec3.hpp"
#include "AxoPlotl/gui/widgets/draw_basic_renderer_settings.hpp"

namespace AxoPlotl::internal
{

PropertySettingsVec3::PropertySettingsVec3(Vector3Renderer& _renderer) :
    renderer_(_renderer)
{
    set_default_settings();
}

void PropertySettingsVec3::render_ui()
{
    draw_basic_renderer_settings(renderer_);
}

void PropertySettingsVec3::set_default_settings() {
    renderer_.vector_scale() = 1.0f;
}

}
