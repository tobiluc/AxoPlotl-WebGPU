#pragma once
#include "AxoPlotl/properties/Histogram.hpp"
#include "AxoPlotl/rendering/renderers/PropertyRendererBase.hpp"
#include "PropertyRendererSettingsBase.hpp"
#include <imgui.h>

namespace AxoPlotl::internal
{

template<typename FT>
    requires(std::is_floating_point_v<FT>)
struct PropertySettingsFloatRange : public PropertySettingsBase
{
public:
    PropertySettingsFloatRange(OVM::PropertyStorageT<FT>* _prop, PropertyRendererBase* _renderer)
        : hist_(_prop), renderer_(_renderer)
    {
        set_default_settings();
    }

    void render_ui() override
    {
        if (!hist_.has_valid_values()) [[unlikely]] {
            ImGui::TextColored(ImVec4(1,0,0,1), "No valid value exists.\nEither there are no entities\nor every value is NaN or Infinity.");
            return;
        }
        Vec2f& rangef = renderer_->property_filter().range_;
        ColorMap& cm = renderer_->color_map();
        int b = hist_.render_ui(cm);
        if (b >= 0) {
            rangef.x = hist_.bucket_min(b);
            rangef.y = hist_.bucket_max(b);
        }
        cm.render_menu();
        // std::cerr << rangef.x << "/"<< rangef.y<< "/"<< hist_.min()<< "/"<< hist_.max() << std::endl;
        if (ImGui::InputFloat2("Range", &rangef.x)) {
            rangef.x = std::clamp<float>(rangef.x, hist_.min(), hist_.max());
            rangef.y = std::clamp<float>(rangef.y, rangef.x, hist_.max());
        }
        draw_range_sliders(rangef.x, rangef.y, hist_.min(), hist_.max(), cm);
    }

    std::string name() const override {
        return "Float Range";
    }

    void set_default_settings() override {
        renderer_->color_map().set_coolwarm();
        renderer_->property_filter().range_ = {
            hist_.min(), hist_.max()
    };
}

private:
          HistogramFT<FT> hist_;
PropertyRendererBase* renderer_;
//bool show_only_visble_buckets_ = false;
};


}
