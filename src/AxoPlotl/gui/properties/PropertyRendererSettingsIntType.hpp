#pragma once
#include "AxoPlotl/gui/widgets/input_slider.hpp"
#include "AxoPlotl/properties/Histogram.hpp"
#include "AxoPlotl/rendering/renderers/PropertyRendererBase.hpp"
#include "IconsFontAwesome7.h"
#include "PropertyRendererSettingsBase.hpp"
#include <AxoPlotl/gui/widgets/draw_range_sliders.hpp>
#include <AxoPlotl/gui/widgets/draw_histogram.hpp>

namespace AxoPlotl::internal
{


template<typename IT> requires(std::is_integral_v<IT>)
struct PropertySettingsInt : public PropertySettingsBase
{
public:
    enum class Mode {
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
        // Mode
        static const char* modes[]{"Value Range", "Exact Value"};
        int mode = static_cast<int>(mode_);
        if (ImGui::Combo(ICON_FA_FILTER, &mode, modes, 2)) {
            mode_ = static_cast<Mode>(mode);
        }
        ImGui::SetItemTooltip("Filter Mode");
        ImGui::SameLine();
        bool invert = renderer_->property_filter_options().invert;
        if (ImGui::Checkbox(ICON_FA_ARROW_RIGHT_ARROW_LEFT, &invert)) {
            renderer_->property_filter_options().invert = invert;
        }
        ImGui::SetItemTooltip("Invert");

        Vec2& rangef = renderer_->property_filter_options().scalar_range;
        ColorMap& cm = renderer_->color_map();

        switch (mode_) {
        case Mode::RANGE:
        {
            // Draw Histogram
            int b = gui::Histogram(hist_,
                    gui::HistogramOptions{
                        .colormap=&cm,
                        .pm_buttons=true
                    }
            );

            // When selecting bucket, set range
            // to bucket range
            if (b >= 0) {
                rangef.x = hist_.bucket_min(b);
                rangef.y = hist_.bucket_max(b);
            }

            // Colormap Menu
            cm.render_menu();

            // Range Setting
            gui::RangeSliders(
                rangef.x, rangef.y,
                gui::RangeSlidersOptions{
                    .min = static_cast<float>(hist_.min()),
                    .max = static_cast<float>(hist_.max()),
                    .integer = true,
                    .colormap = &cm
                }
                );
            if (gui::InputSlider2(ICON_FA_RULER_HORIZONTAL, &rangef.x, gui::InputSliderOptions{
                      .step = 1,
                      .min = static_cast<float>(hist_.min()),
                      .max = static_cast<float>(hist_.max()),
                      .display_name=false,
                      .integer = true
                  })) {
                rangef.x = std::clamp<float>(rangef.x, hist_.min(), hist_.max());
                rangef.y = std::clamp<float>(rangef.y, rangef.x, hist_.max());
            }
            break;
        }
        case Mode::EXACT:
        {
            int val = rangef.x;
            cm.render_menu();
            ImGui::InputInt("Value", &val);
            val = std::clamp(static_cast<IT>(val), hist_.min(), hist_.max());
            renderer_->property_filter_options().scalar_range = {val,val};
            break;
        }
        default:
            break;
        }
    }

    void set_default_settings() override {
        mode_ = Mode::RANGE;
        renderer_->property_filter_options().filter_mode = PropFilterMode::INSIDE_RANGE;
        renderer_->property_filter_options().invert = 0;
        renderer_->color_map().set_coolwarm();
        renderer_->property_filter_options().scalar_range = {
            hist_.min(), hist_.max()
    };
}

private:
    HistogramIT<IT> hist_;
    PropertyRendererBase* renderer_;
    Mode mode_ = Mode::RANGE;
};

}
