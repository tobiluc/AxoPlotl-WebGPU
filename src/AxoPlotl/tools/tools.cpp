#include "tools.hpp"
#include <imgui.h>

namespace AxoPlotl::gui
{

void render_tools_tab_items_ui()
{
    if (ImGui::BeginTabItem(AxoPlotl::ScenesControlTool::icon())) {
        AxoPlotl::ScenesControlTool::draw_ui();
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem(AxoPlotl::ObjectsControlTool::icon())) {
        AxoPlotl::ObjectsControlTool::draw_ui();
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