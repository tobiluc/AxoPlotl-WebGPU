#pragma once

#include <AxoPlotl/rendering/OpenVolumeMeshRenderer.hpp>
#include <AxoPlotl/typedefs/ovm.hpp>
#include <AxoPlotl/typedefs/glm.hpp>
#include <AxoPlotl/properties/Histogram.hpp>
#include <type_traits>
#include <imgui.h>
#include <AxoPlotl/gui/fonts.hpp>

namespace AxoPlotl
{

struct PropertyFilterBase
{
    virtual void init(OpenVolumeMeshRenderer& _r) = 0;
    virtual void renderUI(OpenVolumeMeshRenderer& _r) = 0;
    virtual std::string name() = 0;
};

template<typename Entity>
static Vec2f& get_property_value_filter(OpenVolumeMeshRenderer& _r)
{
    if constexpr(std::is_same_v<Entity,OVM::Entity::Vertex>) {return _r.vertices().property_filter().range_;}
    if constexpr(std::is_same_v<Entity,OVM::Entity::Edge>) {return _r.edges().property_filter().range_;}
    if constexpr(std::is_same_v<Entity,OVM::Entity::Face>) {return _r.faces().property_filter().range_;}
    if constexpr(std::is_same_v<Entity,OVM::Entity::Cell>) {return _r.cells().property_filter().range_;}
}

template<typename Entity>
static ColorMap& get_property_color_map(OpenVolumeMeshRenderer& _r)
{
    if constexpr(std::is_same_v<Entity,OVM::Entity::Vertex>) {return _r.vertices().color_map();}
    if constexpr(std::is_same_v<Entity,OVM::Entity::Edge>) {return _r.edges().color_map();}
    if constexpr(std::is_same_v<Entity,OVM::Entity::Face>) {return _r.faces().color_map();}
    if constexpr(std::is_same_v<Entity,OVM::Entity::Cell>) {return _r.cells().color_map();}
}

template<typename ST, typename Entity>
struct ScalarPropertyRangeFilter : public PropertyFilterBase
{
    using Scalar = ST;

    ScalarPropertyRangeFilter(Histogram<Scalar> _hist)
        : hist_(_hist) {}

    void init(OpenVolumeMeshRenderer& _r) override
    {
        get_property_color_map<Entity>(_r).set_coolwarm();
    }

    void renderUI(OpenVolumeMeshRenderer& _r) override
    {
        Vec2f& vis_range_f = get_property_value_filter<Entity>(_r);
        ColorMap& cm = get_property_color_map<Entity>(_r);

        if (!hist_.any_valid_) [[unlikely]] {
            ImGui::TextColored(ImVec4(1,0,0,1), "No valid value exists.\nEither there are no entities\nor every value is NaN or Infinity.");
            return;
        }

        if constexpr(!std::is_same_v<Scalar,bool>) {
            ImGui::Checkbox(ICON_FA_EYE_LOW_VISION, &show_only_visble_buckets_);
            ImGui::SameLine();
        }

        // Render Histogram.
        // If we click a bar, set it to the visible range
        int selected_bucket(-1);
        if (show_only_visble_buckets_) {
            selected_bucket = hist_.render_ui(
                hist_.bucket(hist_.interpolate(hist_.interpolation_t(vis_range_f.x))),
                hist_.bucket(hist_.interpolate(hist_.interpolation_t(vis_range_f.y)))+1,
                cm);
        } else {
            selected_bucket = hist_.render_ui(cm);
        }

        if (selected_bucket >= 0) {
            vis_range_f.x = static_cast<float>(hist_.bucket_min(selected_bucket));
            vis_range_f.y = static_cast<float>(hist_.bucket_max_[selected_bucket]);
        }

        std::string colormap_menu_title = "Colormap ("
            + cm.name_ + ")";
        if (ImGui::BeginMenu(colormap_menu_title.c_str())) {
            if (ImGui::MenuItem("Viridis")) {
                cm.set_viridis();
            }
            if (ImGui::MenuItem("Magma")) {
                cm.set_magma();
            }
            if (ImGui::MenuItem("Plasma")) {
                cm.set_plasma();
            }
            if (ImGui::MenuItem("Diverging Red Blue")) {
                cm.set_rd_bu();
            }
            if (ImGui::MenuItem("Coolwarm")) {
                cm.set_coolwarm();
            }
            if (ImGui::MenuItem("Rainbow")) {
                cm.set_rainbow();
            }
            ImGui::EndMenu();
        }

        auto draw_colormap = [&]() {
            ImGui::Image(
                (ImTextureID)cm.view_,
                ImVec2(ImGui::GetContentRegionAvail().x, 20),
                ImVec2(0,0),
                ImVec2(1,1)
            );
            ImGui::Spacing();
        };
        auto draw_colormap_sliders = [&]() -> bool
        {
            const float hist_minf = static_cast<float>(hist_.min_);
            const float hist_maxf = static_cast<float>(hist_.max_);

            ImVec2 top_left = ImGui::GetCursorScreenPos();
            ImVec2 total_size = ImVec2(ImGui::GetContentRegionAvail().x, 20);
            ImVec2 bot_right = ImVec2(top_left.x + total_size.x, top_left.y + total_size.y);

            ImDrawList* draw_list = ImGui::GetWindowDrawList();

            // Helper to map property value float
            // to screen x coordinates
            auto val_to_screen_x = [&](const float& val) {
                float t = (val - hist_minf) / (hist_maxf - hist_minf);
                return top_left.x + t * total_size.x;
            };

            // Draw the Colormap Image
            const float visible_left = val_to_screen_x(vis_range_f.x);
            const float visible_right = val_to_screen_x(vis_range_f.y);
            draw_list->AddRectFilled(top_left, bot_right, ImGui::GetColorU32(ImGuiCol_FrameBg));
            ImGui::SetCursorScreenPos(ImVec2(visible_left, top_left.y));
            ImGui::Image((ImTextureID)cm.view_, ImVec2(visible_right-visible_left, total_size.y));
            ImGui::SetCursorScreenPos(top_left);

            // Setup interaction
            bool changed = false;
            float handle_x[2] = {val_to_screen_x(vis_range_f.x),
                                 val_to_screen_x(vis_range_f.y)};

            for (int i = 0; i < 2; ++i)
            {
                ImGui::PushID(i);
                ImVec2 pos = ImVec2(handle_x[i], top_left.y + total_size.y * 0.5f);

                // Transparent invisible button to capture drag
                const float button_width = 16.0f;
                ImGui::SetCursorScreenPos(ImVec2(pos.x - 0.5f*button_width, top_left.y));
                ImGui::InvisibleButton("##handle", ImVec2(button_width, total_size.y));

                if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
                {
                    const float mouse_x = ImGui::GetIO().MousePos.x;
                    const float t = std::clamp((mouse_x - top_left.x) / total_size.x, 0.0f, 1.0f);

                    const float valf = hist_minf + t * (hist_maxf - hist_minf);

                    // Clamp
                    if (i == 0) {vis_range_f.x = std::min(valf, vis_range_f.y);}
                    else {vis_range_f.y = std::max(valf, vis_range_f.x);}

                    changed = true;
                }

                // Get visible range in scalar type
                glm::vec<2,Scalar> vis_range_s = {
                    static_cast<Scalar>(vis_range_f.x),
                    static_cast<Scalar>(vis_range_f.y)
                };
                vis_range_f.x = static_cast<float>(vis_range_s.x);
                vis_range_f.y = static_cast<float>(vis_range_s.y);

                // Draw the visual handle
                const ImU32 handle_fill_color = changed? ImGui::GetColorU32(ImGuiCol_SliderGrabActive) : ImGui::GetColorU32(ImGuiCol_SliderGrab);
                const ImU32 handle_outline_color = ImGui::GetColorU32(ImGuiCol_Border);
                draw_list->AddLine(ImVec2(handle_x[i], top_left.y), ImVec2(handle_x[i], bot_right.y), handle_fill_color, 4.0f);
                draw_list->AddCircleFilled(ImVec2(handle_x[i], bot_right.y), 4.0f, handle_fill_color);
                draw_list->AddCircle(ImVec2(handle_x[i], bot_right.y), 4.0f, handle_outline_color);

                // Handle Label
                char buf[32];
                snprintf(buf, sizeof(buf), "%s", (i == 0)?
                    std::to_string(vis_range_s.x).c_str() :
                    std::to_string(vis_range_s.y).c_str());
                ImVec2 text_size = ImGui::CalcTextSize(buf);
                const float text_x = handle_x[i] - (text_size.x * 0.5f);
                const float text_y = bot_right.y + 5.0f;
                draw_list->AddText(ImVec2(text_x, text_y), ImGui::GetColorU32(ImGuiCol_Text), buf);

                ImGui::PopID();
            }

            // Move the cursor down so the next ImGui element doesn't overlap the text
            const float reserved_space = 5.0f + ImGui::GetFontSize();
            ImGui::SetCursorScreenPos(ImVec2(top_left.x, bot_right.y + reserved_space + ImGui::GetStyle().ItemSpacing.y));

            return changed;
        };

        if constexpr(std::is_same_v<ST,bool>) {
            bool b_show_false = !vis_range_f.x;
            bool b_show_true = vis_range_f.y;
            ImGui::Checkbox("Show False", &b_show_false);
            ImGui::SameLine();
            ImGui::Checkbox("Show True", &b_show_true);
            vis_range_f.x = !b_show_false;
            vis_range_f.y = b_show_true;
        } else {
            draw_colormap_sliders();
        }
    }

    std::string name() override {
        return "Scalar Range";
    }

    Histogram<Scalar> hist_;
    bool show_only_visble_buckets_ = false;
};

template<typename ST, typename Entity>
struct ScalarPropertyExactFilter : public PropertyFilterBase
{
    using Scalar = ST;

    ScalarPropertyExactFilter(Histogram<Scalar> _hist)
        : hist_(_hist)
    {
    }

    void init(OpenVolumeMeshRenderer& _r) override
    {
        get_property_color_map<Entity>(_r).set_single_color(
            {1.0f, 0.0f, 0.0f}
        );
        color = {1.0f, 0.0f, 0.0f};
    }

    void renderUI(OpenVolumeMeshRenderer& _r) override
    {
        if (!hist_.any_valid_) [[unlikely]] {
            ImGui::TextColored(ImVec4(1,0,0,1), "No valid value exists.\nEither there are no entities\nor every value is NaN or Infinity.");
            return;
        }

        Vec2f& visible_range = get_property_value_filter<Entity>(_r);

        //_r.v_prop_range
        if constexpr(std::is_same_v<ST,int>) {
            int i = visible_range[0];
            ImGui::InputInt("Value", &i);
            visible_range[0] = visible_range[1] = i;
        }
        else if constexpr(std::is_same_v<ST,float> || std::is_same_v<ST,double>) {
            float f = visible_range[0];
            ImGui::InputFloat("Value", &f);
            visible_range[0] = visible_range[1] = f;
        }
        else if constexpr(std::is_same_v<ST,bool>) {
            bool b = visible_range[0];
            ImGui::Checkbox("True", &b);
            visible_range[0] = visible_range[1] = b;
        }

        // Clamp to total range
        visible_range[0] = std::clamp<float>(visible_range[0], hist_.min_, hist_.max_);
        visible_range[1] = visible_range[0];

        if (ImGui::ColorEdit3("Color", &color[0])) {
            auto& cm = get_property_color_map<Entity>(_r);
            cm.set_single_color({color[0],color[1],color[2]});
        }
    }

    std::string name() override {
        return "Exact Scalar";
    }

    Vec3f color = {1,0,0};
    Histogram<Scalar> hist_;
};

}
