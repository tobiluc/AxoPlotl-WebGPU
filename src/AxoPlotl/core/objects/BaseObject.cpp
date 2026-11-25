#include "BaseObject.hpp"
#include <imgui.h>

namespace AxoPlotl
{

int ObjectBase::id_counter_ = 0;


bool ObjectBase::draw_name_input_ui() {
    return ImGui::InputText("Name", name_.data(), name_.size());
}

}
