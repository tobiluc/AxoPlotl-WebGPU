#pragma once
#include "AxoPlotl/rendering/renderers/Vector3Renderer.hpp"
#include "PropertyRendererSettingsBase.hpp"

namespace AxoPlotl::internal
{

struct PropertySettingsVec3 : public PropertySettingsBase
{
public:
    PropertySettingsVec3(Vector3Renderer& _renderer);

    void render_ui() override;

    void set_default_settings() override;
private:
    Vector3Renderer& renderer_;
};

}
