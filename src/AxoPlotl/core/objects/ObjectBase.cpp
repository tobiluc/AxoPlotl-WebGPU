#include "ObjectBase.hpp"
#include <imgui.h>

namespace AxoPlotl
{

int ObjectBase::id_counter_ = 0;

void ObjectBase::update(float _dt) {}

bool ObjectBase::draw_name_input_ui() {
    return ImGui::InputText("Name", name_.data(), name_.size());
}

}
