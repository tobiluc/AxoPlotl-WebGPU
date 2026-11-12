#pragma once
#include "AxoPlotl/rendering/renderers/Matrix3x3Renderer.hpp"
#include "PropertyRendererSettingsBase.hpp"

namespace AxoPlotl::internal
{


class PropertySettingsMat3x3 : public PropertySettingsBase
{
public:
    PropertySettingsMat3x3(Matrix3x3Renderer& _renderer);

    void render_ui() override;

    std::string name() const override {
        return "Mat3x3";
    }

    void set_default_settings() override;
private:
    Matrix3x3Renderer& renderer_;
};

}
