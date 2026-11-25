#include "tools.hpp"
#include <imgui.h>

namespace AxoPlotl::gui
{

void render_tools_tab_items_ui()
{
    if (ImGui::BeginTabItem(AxoPlotl::DataControlTool::icon())) {
        AxoPlotl::DataControlTool::draw_ui();
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem(AxoPlotl::PlottingTool::icon())) {
        AxoPlotl::PlottingTool::draw_ui();
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem(AxoPlotl::ScreenshotTool::icon())) {
        AxoPlotl::ScreenshotTool::draw_ui();
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem(AxoPlotl::DebugTool::icon())) {
        AxoPlotl::DebugTool::draw_ui();
        ImGui::EndTabItem();
    }
}

}