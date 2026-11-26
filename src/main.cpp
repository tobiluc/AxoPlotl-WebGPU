#define WEBGPU_CPP_IMPLEMENTATION
#include "AxoPlotl/AxoPlotl.hpp"
#include <AxoPlotl/tools/tools.hpp>

int main()
{
    if (!AxoPlotl::app().init()) {
      std::cerr << "Failed to initialize AxoPlotl" << std::endl;
      return 1;
    }

    AxoPlotl::app().inspector_callback() = {[&]()
    {
        if (ImGui::BeginTabBar("")) {
            AxoPlotl::gui::draw_tools_tab_items_ui();
            ImGui::EndTabBar();
        }
    }};

    AxoPlotl::app().run();

    return 0;
}
