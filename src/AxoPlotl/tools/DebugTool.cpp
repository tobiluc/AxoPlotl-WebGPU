#include "DebugTool.hpp"
#include "AxoPlotl/Application.hpp"
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

void DebugTool::render_ui(Application& app)
{
    if (!ImGui::CollapsingHeader("Debug")) {return;}

    ImGui::SeparatorText("Performance");
    ImGui::Text("%f MB", memory_usage_mb());
    ImGui::Text("%f FPS", app.fps());
    ImGui::Text("Draw Frames %d", n_draw_frames());
    ImGui::SeparatorText("Scene");
    ImGui::Text("#Objects: %zu", app.scene().get_objects().size());
}
}
