#include "DebugPlugin.hpp"
#include "AxoPlotl/Application.hpp"
#include "AxoPlotl/PluginRegistry.hpp"
#include "AxoPlotl/utils/fps.hpp"
#include <mach/task_info.h>
#include <mach/mach.h>

static double inline memory_usage_mb() {
    mach_task_basic_info info;
    mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;
    task_info(mach_task_self(), MACH_TASK_BASIC_INFO, (task_info_t)&info, &count);
    return info.resident_size / (1024.0 * 1024.0);
}

namespace AxoPlotl
{

void DebugPlugin::render_ui(Application& app)
{
    ImGui::SeparatorText("Loaded Plugins");
    for (const auto& plugin : PluginRegistry::get_plugins()) {
        ImGui::Text("%s", plugin.second->name());
    }
    ImGui::SeparatorText("Performance");
    ImGui::Text("%f MB", memory_usage_mb());
    ImGui::Text("%f FPS", Time::FRAMES_PER_SECOND);
    ImGui::SeparatorText("Scene");
    ImGui::Text("#Objects: %zu", app.scene().get_objects().size());
}

REGISTER_AXOPLOTL_PLUGIN(DebugPlugin)

}
