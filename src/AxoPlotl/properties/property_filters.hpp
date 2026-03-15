#pragma once

#include <AxoPlotl/typedefs/ovm.hpp>
#include <AxoPlotl/typedefs/glm.hpp>
#include <AxoPlotl/properties/Histogram.hpp>
#include <type_traits>
#include <imgui.h>
#include <AxoPlotl/gui/fonts.hpp>

namespace AxoPlotl
{
template<class Renderer>
struct PropertyFilterForRenderer
{
    virtual void init(Renderer& _r) = 0;
    virtual void render_ui(Renderer& _r) = 0;
    virtual std::string name() = 0;
};

template<typename EntityTag, class Renderer>
struct PropertyFilterBool : public PropertyFilterForRenderer<Renderer>
{
public:
    PropertyFilterBool(OVM::PropertyStorageT<bool>* _prop)
        : hist_(_prop) {}

    inline void init(Renderer& _r) override {

    }

    inline void render_ui(Renderer& _r) override
    {
        auto& cm = _r.color_map();

        int b = hist_.render_ui(cm);
        if (b==0) {show_false_ = true; show_true_ = false;}
        else if (b==1) {show_false_ = false; show_true_ = true;}

        changed_ |= ImGui::Checkbox("False", &show_false_);
        ImGui::SameLine();
        changed_ |= ImGui::ColorEdit3("##FalseCol", &color_false_[0]);

        changed_ |= ImGui::Checkbox("True", &show_true_);
        ImGui::SameLine();
        changed_ |= ImGui::ColorEdit3("##TrueCol", &color_true_[0]);

        if (changed_) {
            // Update Range
            _r.property_filter().range_ = {
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

    inline std::string name() override {
        return "Bool Filter";
    }

private:
    HistogramBool hist_;
    Vec3f color_true_ = {0,1,0};
    Vec3f color_false_ = {1,0,0};
    bool show_true_ = true;
    bool show_false_ = true;
    bool changed_ = true;
};

template<typename FT, typename EntityTag, class Renderer>
requires(std::is_floating_point_v<FT>)
struct PropertyFilterFloatRange : public PropertyFilterForRenderer<Renderer>
{
public:
    PropertyFilterFloatRange(OVM::PropertyStorageT<FT>* _prop)
        : hist_(_prop) {}

    inline void init(Renderer& _r) override {
        _r.color_map().set_coolwarm();
        _r.property_filter().range_ = {
            hist_.min(), hist_.max()
        };
    }

    inline void render_ui(Renderer& _r) override
    {
        if (!hist_.has_valid_values()) [[unlikely]] {
            ImGui::TextColored(ImVec4(1,0,0,1), "No valid value exists.\nEither there are no entities\nor every value is NaN or Infinity.");
            return;
        }
        Vec2f& rangef = _r.property_filter().range_;
        ColorMap& cm = _r.color_map();
        int b = hist_.render_ui(cm);
        if (b >= 0) {
            rangef.x = hist_.bucket_min(b);
            rangef.y = hist_.bucket_max(b);
        }
        cm.render_menu();
        // std::cerr << rangef.x << "/"<< rangef.y<< "/"<< hist_.min()<< "/"<< hist_.max() << std::endl;
        cm.render_with_sliders<float>(rangef.x, rangef.y, hist_.min(), hist_.max());
    }

    inline std::string name() override {
        return "Float Range";
    }

private:
    HistogramFT<FT> hist_;
    //bool show_only_visble_buckets_ = false;
};

template<typename IT, typename EntityTag, class Renderer>
requires(std::is_integral_v<IT>)
struct PropertyFilterIntRange : public PropertyFilterForRenderer<Renderer>
{
public:
    PropertyFilterIntRange(OVM::PropertyStorageT<IT>* _prop)
        : hist_(_prop) {}

    inline void init(Renderer& _r) override {
        _r.color_map().set_coolwarm();
        _r.property_filter().range_ = {
            hist_.min(), hist_.max()
        };
    }

    inline void render_ui(Renderer& _r) override
    {
        Vec2f& rangef = _r.property_filter().range_;
        ColorMap& cm = _r.color_map();
        int b = hist_.render_ui(cm);
        if (b >= 0) {
            rangef.x = hist_.bucket_min(b);
            rangef.y = hist_.bucket_max(b);
        }
        cm.render_menu();
        cm.render_with_sliders<float>(rangef.x, rangef.y, hist_.min(), hist_.max());
    }

    inline std::string name() override {
        return "Int Range";
    }

private:
    HistogramIT<IT> hist_;
};

template<typename IT, typename EntityTag, class Renderer>
requires(std::is_integral_v<IT>)
struct PropertyFilterIntValue : public PropertyFilterForRenderer<Renderer>
{
public:
    PropertyFilterIntValue(OVM::PropertyStorageT<IT>* _prop)
        : hist_(_prop) {}

    inline void init(Renderer& _r) override {
        vali_ = hist_.min();
        _r.color_map().set_coolwarm();
        _r.property_filter().range_ = {
            vali_, vali_
        };
    }

    inline void render_ui(Renderer& _r) override
    {
        ColorMap& cm = _r.color_map();
        cm.render_menu();
        ImGui::InputInt("Value", &vali_);
        vali_ = std::clamp(static_cast<IT>(vali_), hist_.min(), hist_.max());
        _r.property_filter().range_ = {vali_,vali_};
    }

    inline std::string name() override {
        return "Int Value";
    }

private:
    int vali_;
    HistogramIT<IT> hist_;
};

}
