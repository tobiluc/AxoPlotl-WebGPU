#include "tools.hpp"
#include <imgui.h>
#include <AxoPlotl/gui/fonts.hpp>

namespace AxoPlotl::gui
{

static bool run_once = false;

void draw_tools_tab_items_ui()
{
    if (ImGui::BeginTabItem(ICON_FA_LAYER_GROUP)) {
        AxoPlotl::ScenesControlTool::draw_ui();
        ImGui::EndTabItem();
    }
    ImGui::SetItemTooltip("Scenes");

    ImGuiTabItemFlags flags = ImGuiTabItemFlags_None;
    if (!run_once) {
        flags |= ImGuiTabItemFlags_SetSelected;
    }
    if (ImGui::BeginTabItem(ICON_FA_SLIDERS, nullptr, flags)) {
        AxoPlotl::ObjectsControlTool::draw_ui();
        ImGui::EndTabItem();
    }
    ImGui::SetItemTooltip("Objects");
    run_once = true;

    if (ImGui::BeginTabItem(ICON_FA_CHART_LINE)) {
        AxoPlotl::PlottingTool::draw_ui();
        ImGui::EndTabItem();
    }
    ImGui::SetItemTooltip("Plotting");

    if (ImGui::BeginTabItem(ICON_FA_IMAGE)) {
        AxoPlotl::ScreenshotTool::draw_ui();
        ImGui::EndTabItem();
    }
    ImGui::SetItemTooltip("Screenshot");

    if (ImGui::BeginTabItem(ICON_FA_BUG)) {
        AxoPlotl::DebugTool::draw_ui();
        ImGui::EndTabItem();
    }
    ImGui::SetItemTooltip("Debugging");
}

}