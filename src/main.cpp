#define WEBGPU_CPP_IMPLEMENTATION

#include "AxoPlotl/AxoPlotl.hpp"
#include "AxoPlotl/gui/themes.hpp"
#include "AxoPlotl/tools/DataControlTool.hpp"
#include "AxoPlotl/tools/DebugTool.hpp"
#include "AxoPlotl/tools/PlottingTool.hpp"
#include "AxoPlotl/tools/ScreenshotTool.hpp"

int main()
{
    if (!AxoPlotl::app().init()) {
      std::cerr << "Failed to initialize AxoPlotl" << std::endl;
      return 1;
    }

    AxoPlotl::app().inspector_callback() = {[&]()
    {
        if (ImGui::BeginTabBar("")) {
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
            ImGui::EndTabBar();
        }
    }};

    AxoPlotl::GUI::apply_theme(AxoPlotl::GUI::Theme::ModernLight);

    AxoPlotl::app().run();

    return 0;
}
