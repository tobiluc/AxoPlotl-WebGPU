#include "DataControlPlugin.hpp"
#include "AxoPlotl/Application.hpp"
#include "AxoPlotl/PluginRegistry.hpp"
#include <mach/task_info.h>
#include <mach/mach.h>

namespace AxoPlotl
{

void DataControlPlugin::render_ui(Application& app)
{
    for (const auto& obj : app.scene().get_objects()) {
        ImGui::Checkbox("##V", obj->visible());
        ImGui::SameLine();
        if (ImGui::Selectable(obj->name().c_str())) {
            app.scene().zoom_to_box(obj->bounding_box());
        }
    }
}

REGISTER_AXOPLOTL_PLUGIN(DataControlPlugin)

}
