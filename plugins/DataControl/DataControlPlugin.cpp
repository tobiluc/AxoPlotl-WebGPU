#include "DataControlPlugin.hpp"
#include "AxoPlotl/Application.hpp"
#include "AxoPlotl/PluginRegistry.hpp"
#include <mach/task_info.h>
#include <mach/mach.h>

namespace AxoPlotl
{

void DataControlPlugin::render_ui(Application& _app)
{
    for (const auto& obj : _app.scene().get_objects()) {
        ImGui::PushID(obj->id());

        // Visible Checkbox
        ImGui::Checkbox("##V", &obj->visible());
        ImGui::SameLine();
        if (ImGui::Button("Zoom")) {
            _app.scene().zoom_to_box(obj->bounding_box());
        }
        ImGui::SameLine();
        // Toggle Selected
        if (ImGui::Selectable(obj->name().c_str())) {
            obj->selected() = !obj->selected();
        }



        // Expand Menu
        if (obj->selected()) {
            obj->render_ui();
            if (ImGui::Button("Delete Object")) {
                obj->deleted() = true;
            }
        }
        ImGui::PopID();
    }
}

REGISTER_AXOPLOTL_PLUGIN(DataControlPlugin)

}
