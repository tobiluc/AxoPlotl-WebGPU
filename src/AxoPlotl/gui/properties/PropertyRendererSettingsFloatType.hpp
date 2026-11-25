#pragma once
#include "AxoPlotl/gui/widgets/draw_histogram.hpp"
#include "AxoPlotl/gui/widgets/draw_range_sliders.hpp"
#include "AxoPlotl/gui/widgets/input_slider.hpp"
#include "AxoPlotl/properties/Histogram.hpp"
#include "AxoPlotl/rendering/renderers/PropertyRendererBase.hpp"
#include "IconsFontAwesome7.h"
#include "PropertyRendererSettingsBase.hpp"
#include <imgui.h>

namespace AxoPlotl::internal
{

template<typename FT>
requires(std::is_floating_point_v<FT>)
struct PropertySettingsFloatRange : public PropertySettingsBase
{
public:

    enum class Mode {
        RANGE, SHOW_NAN, SHOW_INF
    };

    PropertySettingsFloatRange(OVM::PropertyStorageT<FT>* _prop, PropertyRendererBase* _renderer)
        : hist_(_prop), renderer_(_renderer)
    {
        set_default_settings();
    }

    void render_ui() override
    {
        // Modes
        static const char* modes[]{"Value Range", "NaN", "Inf"};
        int mode = static_cast<int>(mode_);
        if (ImGui::Combo(ICON_FA_FILTER, &mode, modes, 3)) {
            mode_ = static_cast<Mode>(mode);
            auto& fm = renderer_->property_filter_options().filter_mode;
            switch(mode_) {
            case Mode::RANGE:
                fm = PropFilterMode::INSIDE_RANGE;
                break;
            case Mode::SHOW_NAN:
                fm = PropFilterMode::IS_NAN;
                break;
            case Mode::SHOW_INF:
                fm = PropFilterMode::IS_INF;
                break;
            default:
                break;
            }
        }
        ImGui::SetItemTooltip("Filter Mode");
        ImGui::SameLine();
        bool invert = renderer_->property_filter_options().invert!=0u;
        if (ImGui::Checkbox(ICON_FA_ARROW_RIGHT_ARROW_LEFT, &invert)) {
            renderer_->property_filter_options().invert = invert? 1u : 0u;
        }
        ImGui::SetItemTooltip("Invert");

        // Warning if only INF/NaN
        if (!hist_.has_valid_values()) [[unlikely]] {
            ImGui::TextColored(ImVec4(1,0,0,1), "No valid value exists.\nEither there are no entities\nor every value is NaN or Infinity.");
            return;
        }

        Vec2& rangef = renderer_->property_filter_options().scalar_range;
        ColorMap& cm = renderer_->color_map();

        switch(mode_)
        {
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
            cm.draw_menu_ui();

            // Range setting
            gui::RangeSliders(
                rangef.x, rangef.y,
                gui::RangeSlidersOptions{
                    .min = static_cast<float>(hist_.min()),
                    .max = static_cast<float>(hist_.max()),
                    .integer = false,
                    .colormap = &cm
                }
                );
            if (gui::InputSlider2(ICON_FA_RULER_HORIZONTAL, &rangef.x, gui::InputSliderOptions{
                .step = static_cast<float>(hist_.max()-hist_.min())/1000.0f,
                .min = static_cast<float>(hist_.min()),
                .max = static_cast<float>(hist_.max()),
                .display_name=false,
                .integer = false
            })) {
                rangef.x = std::clamp<float>(rangef.x, hist_.min(), hist_.max());
                rangef.y = std::clamp<float>(rangef.y, rangef.x, hist_.max());
            }
        } break;
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
    HistogramFT<FT> hist_;
    PropertyRendererBase* renderer_;
    Mode mode_ = Mode::RANGE;
};


}
