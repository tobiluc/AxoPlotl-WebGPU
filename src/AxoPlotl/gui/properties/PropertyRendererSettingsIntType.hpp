#pragma once
#include "AxoPlotl/properties/Histogram.hpp"
#include "AxoPlotl/rendering/renderers/PropertyRendererBase.hpp"
#include "PropertyRendererSettingsBase.hpp"
#include <AxoPlotl/gui/widgets/draw_range_sliders.hpp>

namespace AxoPlotl::internal
{


template<typename IT> requires(std::is_integral_v<IT>)
struct PropertySettingsInt : public PropertySettingsBase
{
public:
    enum class Mode : int {
        RANGE, EXACT
    };

    PropertySettingsInt(OVM::PropertyStorageT<IT>* _prop,
                             PropertyRendererBase* _renderer)
        : hist_(_prop), renderer_(_renderer)
    {
        set_default_settings();
    }

    void render_ui() override
    {
        static const char* modes[]{"Range","Exact"};
        int mode = static_cast<int>(mode_);
        if (ImGui::Combo("Mode", &mode, modes, 2)) {
            mode_ = static_cast<Mode>(mode);
        }

        Vec2f& rangef = renderer_->property_filter().range_;
        ColorMap& cm = renderer_->color_map();

        switch (mode_) {
        case Mode::RANGE:
        {
            int b = hist_.render_ui(cm);
            if (b >= 0) {
                rangef.x = hist_.bucket_min(b);
                rangef.y = hist_.bucket_max(b);
            }
            cm.render_menu();
            if (ImGui::InputFloat2("Range", &rangef.x)) {
                rangef.x = std::clamp<float>(rangef.x, hist_.min(), hist_.max());
                rangef.y = std::clamp<float>(rangef.y, rangef.x, hist_.max());
            }
            draw_range_sliders(rangef.x, rangef.y, hist_.min(), hist_.max(), cm);
            break;
        }
        case Mode::EXACT:
        {
            int val = rangef.x;
            cm.render_menu();
            ImGui::InputInt("Value", &val);
            val = std::clamp(static_cast<IT>(val), hist_.min(), hist_.max());
            renderer_->property_filter().range_ = {val,val};
            break;
        }
        default:
            break;
        }
    }

    std::string name() const override {
        return "Int";
    }

    void set_default_settings() override {
        mode_ = Mode::RANGE;
        renderer_->color_map().set_coolwarm();
        renderer_->property_filter().range_ = {
            hist_.min(), hist_.max()
    };
}

private:
    HistogramIT<IT> hist_;
    PropertyRendererBase* renderer_;
    Mode mode_;
};

// template<typename IT>
//     requires(std::is_integral_v<IT>)
// struct PropertySettingsIntValue : public PropertySettingsBase
// {
// public:
//     PropertySettingsIntValue(OVM::PropertyStorageT<IT>* _prop,
//                              PropertyRendererBase* _renderer)
//         : hist_(_prop), renderer_(_renderer)
//     {
//         set_default_settings();
//     }

//     void render_ui() override
//     {
//         ColorMap& cm = renderer_->color_map();
//         cm.render_menu();
//         ImGui::InputInt("Value", &vali_);
//         vali_ = std::clamp(static_cast<IT>(vali_), hist_.min(), hist_.max());
//         renderer_->property_filter().range_ = {vali_,vali_};
//     }

//     std::string name() const override {
//         return "Int Value";
//     }

//     void set_default_settings() override {
//         vali_ = hist_.min();
//         renderer_->property_filter().range_ = {
//             vali_, vali_
//         };
//         renderer_->color_map().set_coolwarm();
//     }

// private:
//     int vali_;
//     PropertyRendererBase* renderer_;
//     HistogramIT<IT> hist_;
// };

}
