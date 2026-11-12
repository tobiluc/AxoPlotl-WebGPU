#pragma once
#include "AxoPlotl/properties/Histogram.hpp"
#include "AxoPlotl/rendering/renderers/PropertyRendererBase.hpp"
#include "PropertyRendererSettingsBase.hpp"

namespace AxoPlotl::internal
{

class PropertySettingsBool : public PropertySettingsBase
{
public:
    PropertySettingsBool(OVM::PropertyStorageT<bool>* _prop,
                         PropertyRendererBase* _renderer);

    void render_ui() override;

    std::string name() const override {
        return "Bool Filter";
    }

    void set_default_settings() override;

private:
    HistogramBool hist_;
    PropertyRendererBase* renderer_;

    Vec3f color_true_ = {0,1,0};
    Vec3f color_false_ = {1,0,0};
    bool show_true_ = true;
    bool show_false_ = true;
    bool changed_ = true;
};

}
