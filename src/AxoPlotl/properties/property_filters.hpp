#pragma once

#include <AxoPlotl/rendering/VolumeMeshRenderer.hpp>
#include <AxoPlotl/typedefs/ovm.hpp>
#include <AxoPlotl/typedefs/glm.hpp>
#include <type_traits>
#include <imgui.h>

namespace AxoPlotl
{

struct PropertyFilterBase
{
    virtual void renderUI(VolumeMeshRenderer& _r) = 0;
    virtual std::string name() = 0;
};

template<typename Entity>
static VolumeMeshRenderer::Property& get_property(VolumeMeshRenderer& _r)
{
    if constexpr(std::is_same_v<Entity,OVM::Entity::Vertex>) {return _r.vertex_property_;}
    if constexpr(std::is_same_v<Entity,OVM::Entity::Edge>) {return _r.edge_property_;}
    if constexpr(std::is_same_v<Entity,OVM::Entity::Face>) {return _r.face_property_;}
    if constexpr(std::is_same_v<Entity,OVM::Entity::Cell>) {return _r.cell_property_;}
}

template<typename ST, typename Entity>
struct ScalarPropertyRangeFilter : public PropertyFilterBase
{
    using Scalar = ST;

    ScalarPropertyRangeFilter(ST _min=0, ST _max=1)
        : total_min_(_min), total_max_(_max) {}

    void renderUI(VolumeMeshRenderer& _r) override
    {
        VolumeMeshRenderer::Property& rp = get_property<Entity>(_r);

        std::string colormap_menu_title = "Colormap ("
            + _r.property_color_map_.name_ + ")";
        if (ImGui::BeginMenu(colormap_menu_title.c_str())) {
            if (ImGui::MenuItem("Viridis")) {
                _r.property_color_map_.set_viridis();
            }
            if (ImGui::MenuItem("Magma")) {
                _r.property_color_map_.set_magma();
            }
            if (ImGui::MenuItem("Plasma")) {
                _r.property_color_map_.set_plasma();
            }
            if (ImGui::MenuItem("Diverging Red Blue")) {
                _r.property_color_map_.set_rd_bu();
            }
            if (ImGui::MenuItem("Coolwarm")) {
                _r.property_color_map_.set_coolwarm();
            }
            if (ImGui::MenuItem("Rainbow")) {
                _r.property_color_map_.set_rainbow();
            }
            ImGui::EndMenu();
        }

        auto draw_colormap = [&]() {
            ImGui::Image(
                (ImTextureID)_r.property_color_map_.view_,
                ImVec2(ImGui::GetContentRegionAvail().x, 20),
                ImVec2(0,0),
                ImVec2(1,1)
            );
            ImGui::Spacing();
        };
        draw_colormap();

        // Slider
        Vec2f& vis_range = rp.filter_.scalar_range_;
        if constexpr(std::is_same_v<ST,int>) {
            Vec2i i = {vis_range[0],vis_range[1]};
            ImGui::SliderInt2("Show Range", &i.x, total_min_, total_max_);
            vis_range.x = i.x;
            vis_range.y = i.y;
        }
        else if constexpr(std::is_same_v<ST,float> || std::is_same_v<ST,double>) {
            Vec2f f = {vis_range.x,vis_range.y};
            ImGui::SliderFloat2("Show Range", &f.x, total_min_, total_max_);
            vis_range.x = std::clamp<float>(f.x, total_min_, vis_range.y);
            vis_range.y = std::clamp<float>(f.y, vis_range.x, total_max_);
        } else if constexpr(std::is_same_v<ST,bool>) {
            bool b_show_false = !vis_range.x;
            bool b_show_true = vis_range.y;
            ImGui::Checkbox("Show False", &b_show_false);
            ImGui::SameLine();
            ImGui::Checkbox("Show True", &b_show_true);
            vis_range.x = !b_show_false;
            vis_range.y = b_show_true;
        }
    }

    std::string name() override {
        return "Scalar Range";
    }

    ST total_min_; // global min/total_max_
    ST total_max_;
};

template<typename ST, typename Entity>
struct ScalarPropertyExactFilter : public PropertyFilterBase
{
    using Scalar = ST;

    ScalarPropertyExactFilter(ST _min=0, ST _max=1)
        : total_min_(_min), total_max_(_max)
    {
    }

    void renderUI(VolumeMeshRenderer& _r) override
    {
        VolumeMeshRenderer::Property& rprop = get_property<Entity>(_r);
        Vec2f& visible_range = rprop.filter_.scalar_range_;

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
        visible_range[0] = std::clamp<float>(visible_range[0], total_min_, total_max_);
        visible_range[1] = visible_range[0];

        if (ImGui::ColorEdit3("Color", &color[0])) {
            _r.property_color_map_.set_single_color({color[0],color[1],color[2]});
        }
    }

    std::string name() override {
        return "Exact Scalar";
    }

    Vec3f color = {1,0,0};
    ST total_min_; // global min/total_max_
    ST total_max_;
};

}
