#include "BaseObject.hpp"
#include <imgui.h>

namespace AxoPlotl
{

int ObjectBase::id_counter_ = 0;


bool ObjectBase::input_name() {
    return ImGui::InputText("Name", name_.data(), 1024);
}

}
