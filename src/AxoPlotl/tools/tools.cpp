#include "tools.hpp"
#include <imgui.h>

namespace AxoPlotl::gui
{

void render_tools_tab_items_ui()
{
    if (ImGui::BeginTabItem(AxoPlotl::DataControlTool::icon())) {
        AxoPlotl::DataControlTool::render_ui();
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem(AxoPlotl::PlottingTool::icon())) {
        AxoPlotl::PlottingTool::render_ui();
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem(AxoPlotl::ScreenshotTool::icon())) {
        AxoPlotl::ScreenshotTool::render_ui();
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem(AxoPlotl::DebugTool::icon())) {
        AxoPlotl::DebugTool::render_ui();
        ImGui::EndTabItem();
    }
}

}