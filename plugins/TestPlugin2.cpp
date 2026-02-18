#include "TestPlugin2.hpp"
#include "AxoPlotl/PluginRegistry.hpp"

namespace AxoPlotl
{

void TestPlugin2::render_ui(Application& app)
{
    ImGui::Text("Hello World. I am a Plugin. These are all the plugins:");
    for (const auto& plugin : PluginRegistry::get_plugins()) {
        ImGui::Text("%s", plugin->name());
    }
}

REGISTER_AXOPLOTL_PLUGIN(TestPlugin2)

}
