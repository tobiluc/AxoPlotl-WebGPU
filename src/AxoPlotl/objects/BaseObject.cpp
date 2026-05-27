#include "BaseObject.hpp"
#include <imgui.h>

namespace AxoPlotl
{

int ObjectBase::id_counter_ = 0;


bool ObjectBase::input_name() {
    return ImGui::InputText("Name", name_.data(), 1024);
}

bool ObjectBase::save_file(const std::filesystem::path& _path) const
{
    return false;
}

}
