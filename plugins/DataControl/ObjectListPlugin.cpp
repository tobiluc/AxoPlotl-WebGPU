#include "ObjectListPlugin.hpp"
#include "AxoPlotl/Application.hpp"
#include "AxoPlotl/PluginRegistry.hpp"
#include <mach/task_info.h>
#include <mach/mach.h>

namespace AxoPlotl
{

void ObjectListPlugin::render_ui(Application& app)
{
    for (const auto& obj : app.scene().get_objects()) {
        ImGui::SeparatorText(obj->name().c_str());
    }
}

REGISTER_AXOPLOTL_PLUGIN(ObjectListPlugin)

}
