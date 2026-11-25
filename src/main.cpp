#define WEBGPU_CPP_IMPLEMENTATION
#include "AxoPlotl/AxoPlotl.hpp"
#include "AxoPlotl/gui/themes.hpp"
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
            AxoPlotl::gui::render_tools_tab_items_ui();
            ImGui::EndTabBar();
        }
    }};

    AxoPlotl::gui::apply_theme(AxoPlotl::gui::Theme::ModernLight);

    AxoPlotl::app().run();

    return 0;
}
