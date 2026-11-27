#include "ObjectBase.hpp"
#include <imgui.h>

namespace AxoPlotl
{

int ObjectBase::id_counter_ = 0;

void ObjectBase::update(float _dt) {}

void ObjectBase::fixed_update(float _dt) {}

void ObjectBase::draw_info_ui() {}

void ObjectBase::draw_viewport_ui() {}

bool ObjectBase::on_mouse_just_pressed(const Picker::Pixel& _p, const Mouse::Button _button) {
    return false;
}   

bool ObjectBase::on_mouse_pressed(const Picker::Pixel& _p, const Mouse::Button _button) {
    return false;
} 

bool ObjectBase::draw_name_input_ui() {
    return ImGui::InputText("Name", name_.data(), name_.size());
}

}
