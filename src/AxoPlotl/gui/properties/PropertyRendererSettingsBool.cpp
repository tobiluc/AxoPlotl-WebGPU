#include "PropertyRendererSettingsBool.hpp"

namespace AxoPlotl::internal
{


PropertySettingsBool::PropertySettingsBool(OVM::PropertyStorageT<bool>* _prop, PropertyRendererBase* _renderer)
    : hist_(_prop), renderer_(_renderer)
{
    set_default_settings();
}

void PropertySettingsBool::render_ui()
{
    auto& cm = renderer_->color_map();

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
        renderer_->property_filter().range_ = {
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

void PropertySettingsBool::set_default_settings() {
    color_true_ = {0,1,0};
    color_false_ = {1,0,0};
    show_true_ = true;
    show_false_ = true;
    changed_ = true;
}

}
