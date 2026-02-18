#include "TestPlugin.hpp"
#include "AxoPlotl/PluginRegistry.hpp"
#include "imgui.h"

namespace AxoPlotl
{

void TestPlugin::render_ui(Application& app)
{
    ImGui::Text("I am an evil plugin. Click this button to delete all objects.");
    if (ImGui::Button("Delete all")) {

    }
}


REGISTER_AXOPLOTL_PLUGIN(TestPlugin)

}
