#pragma once

#include "AxoPlotl/geometry/geometry_traits.hpp"
#include "AxoPlotl/rendering/Vector3Renderer.hpp"
#include "AxoPlotl/rendering/renderer_types.hpp"
#include <AxoPlotl/typedefs/ovm.hpp>
#include <AxoPlotl/typedefs/glm.hpp>
#include <AxoPlotl/properties/Histogram.hpp>
#include <type_traits>
#include <imgui.h>
#include <AxoPlotl/gui/fonts.hpp>
#include <AxoPlotl/rendering/draw_range_sliders.hpp>

namespace AxoPlotl
{

struct PropertyFilterBase
{
    virtual std::string name() const = 0;
    virtual void render_ui() = 0;
    virtual void set_default_settings() = 0;
};

// template<class Renderer>
// struct PropertyFilterForRenderer : public PropertyFilterBase
// {
//     //virtual void init(Renderer& _r) = 0;
//     virtual void render_ui(Renderer& _r) = 0;
// };

template<typename EntityTag>
struct PropertyFilterBool : public PropertyFilterBase
{
public:
    PropertyFilterBool(OVM::PropertyStorageT<bool>* _prop, colored_renderer_for_entity_t<EntityTag>& _renderer)
        : hist_(_prop), renderer_(_renderer)
    {
        set_default_settings();
    }

    inline void render_ui() override
    {
        auto& cm = renderer_.color_map();

        int b = hist_.render_ui(cm);
        if (b==0) {
            show_false_ = true;
            show_true_ = false;
            changed_ = true;
        } else if (b==1) {
            show_false_ = false;
            show_true_ = true;
            changed_ = true;
        }

        changed_ |= ImGui::Checkbox("False", &show_false_);
        ImGui::SameLine();
        changed_ |= ImGui::ColorEdit3("##FalseCol", &color_false_[0]);

        changed_ |= ImGui::Checkbox("True", &show_true_);
        ImGui::SameLine();
        changed_ |= ImGui::ColorEdit3("##TrueCol", &color_true_[0]);

        if (changed_) {
            // Update Range
            renderer_.property_filter().range_ = {
                !show_false_, show_true_
            };

            // Update Color Map
            if (show_true_ && show_false_) {
                cm.set_gradient(color_false_, color_true_);
            } else if (show_false_) {
                cm.set_single_color(color_false_);
            } else if (show_true_) {
                cm.set_single_color(color_true_);
            }
        }
        changed_ = false;
    }

    inline std::string name() const override {
        return "Bool Filter";
    }

    inline void set_default_settings() override {
        color_true_ = {0,1,0};
        color_false_ = {1,0,0};
        show_true_ = true;
        show_false_ = true;
        changed_ = true;
    }

private:
    HistogramBool hist_;
    colored_renderer_for_entity_t<EntityTag>& renderer_;

    Vec3f color_true_ = {0,1,0};
    Vec3f color_false_ = {1,0,0};
    bool show_true_ = true;
    bool show_false_ = true;
    bool changed_ = true;
};

template<typename FT, typename EntityTag>
requires(std::is_floating_point_v<FT>)
struct PropertyFilterFloatRange : public PropertyFilterBase
{
public:
    PropertyFilterFloatRange(OVM::PropertyStorageT<FT>* _prop, colored_renderer_for_entity_t<EntityTag>& _renderer)
        : hist_(_prop), renderer_(_renderer)
    {
        set_default_settings();
    }

    inline void render_ui() override
    {
        if (!hist_.has_valid_values()) [[unlikely]] {
            ImGui::TextColored(ImVec4(1,0,0,1), "No valid value exists.\nEither there are no entities\nor every value is NaN or Infinity.");
            return;
        }
        Vec2f& rangef = renderer_.property_filter().range_;
        ColorMap& cm = renderer_.color_map();
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

    inline std::string name() const override {
        return "Float Range";
    }

    inline void set_default_settings() override {
        renderer_.color_map().set_coolwarm();
        renderer_.property_filter().range_ = {
            hist_.min(), hist_.max()
        };
    }

private:
    HistogramFT<FT> hist_;
    colored_renderer_for_entity_t<EntityTag>& renderer_;
    //bool show_only_visble_buckets_ = false;
};

template<typename IT, typename EntityTag>
requires(std::is_integral_v<IT>)
struct PropertyFilterIntRange : public PropertyFilterBase
{
public:
    PropertyFilterIntRange(OVM::PropertyStorageT<IT>* _prop, colored_renderer_for_entity_t<EntityTag>& _renderer)
        : hist_(_prop), renderer_(_renderer)
    {
        set_default_settings();
    }

    inline void render_ui() override
    {
        Vec2f& rangef = renderer_.property_filter().range_;
        ColorMap& cm = renderer_.color_map();
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
    }

    inline std::string name() const override {
        return "Int Range";
    }

    inline void set_default_settings() override {
        renderer_.color_map().set_coolwarm();
        renderer_.property_filter().range_ = {
            hist_.min(), hist_.max()
    };
}

private:
    HistogramIT<IT> hist_;
    colored_renderer_for_entity_t<EntityTag>& renderer_;
};

template<typename IT, typename EntityTag>
requires(std::is_integral_v<IT>)
struct PropertyFilterIntValue : public PropertyFilterBase
{
public:
    PropertyFilterIntValue(OVM::PropertyStorageT<IT>* _prop,
                           colored_renderer_for_entity_t<EntityTag>& _renderer)
        : hist_(_prop), renderer_(_renderer)
    {
        set_default_settings();
    }

    inline void render_ui() override
    {
        ColorMap& cm = renderer_.color_map();
        cm.render_menu();
        ImGui::InputInt("Value", &vali_);
        vali_ = std::clamp(static_cast<IT>(vali_), hist_.min(), hist_.max());
        renderer_.property_filter().range_ = {vali_,vali_};
    }

    inline std::string name() const override {
        return "Int Value";
    }

    inline void set_default_settings() override {
        vali_ = hist_.min();
        renderer_.property_filter().range_ = {
            vali_, vali_
        };
        renderer_.color_map().set_coolwarm();
    }

private:
    int vali_;
    colored_renderer_for_entity_t<EntityTag>& renderer_;
    HistogramIT<IT> hist_;
};

template<typename Vec3T, typename EntityTag>
requires(is_vector<Vec3T> && vector_dim<Vec3T> == 3)
struct PropertyFilterVec3 : public PropertyFilterBase
{
public:
    PropertyFilterVec3(Vector3Renderer& _renderer) :
        renderer_(_renderer)
    {
        set_default_settings();
    }

    inline void render_ui() override
    {
        ImGui::InputFloat("Vector Scale", &renderer_.vector_scale());
    }

    inline std::string name() const override {
        return "Vec3";
    }

    inline void set_default_settings() override {
        renderer_.vector_scale() = 1.0f;
    }
private:
    Vector3Renderer& renderer_;
};

}
