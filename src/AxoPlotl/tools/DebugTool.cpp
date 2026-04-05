#include "DebugTool.hpp"
#include "AxoPlotl/AxoPlotl.hpp"
#include <AxoPlotl/Scene.hpp>
#include <mach/task_info.h>
#include <mach/mach.h>

namespace AxoPlotl
{

void DebugTool::render_ui()
{
    if (!ImGui::CollapsingHeader("Debug")) {return;}

    ImGui::SeparatorText("Performance");
    ImGui::Text("%f MB", AxoPlotl::memory_usage_mb());
    ImGui::Text("%f FPS", AxoPlotl::fps());
    ImGui::Text("Draw Frames %d", n_draw_frames());
    ImGui::SeparatorText("Scene");
    ImGui::Text("#Objects: %zu", AxoPlotl::scene().get_objects().size());
}
}
