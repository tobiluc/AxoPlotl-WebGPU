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

    std::string name() const override {
        return "Vec3";
    }

    void set_default_settings() override;
private:
    Vector3Renderer& renderer_;
};

}
