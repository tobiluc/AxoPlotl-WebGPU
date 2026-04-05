#define WEBGPU_CPP_IMPLEMENTATION

#include "AxoPlotl/AxoPlotl.hpp"
#include "AxoPlotl/gui/themes.hpp"
#include "AxoPlotl/tools/DataControlTool.hpp"
#include "AxoPlotl/tools/DebugTool.hpp"
#include "AxoPlotl/tools/PlottingTool.hpp"

int main()
{
    if (!AxoPlotl::init()) {
      std::cerr << "Failed to initialice AxoPlotl" << std::endl;
      return 1;
    }

    AxoPlotl::set_inspector_callback([&]()
    {
        AxoPlotl::DataControlTool::render_ui();
        AxoPlotl::PlottingTool::render_ui();
        AxoPlotl::DebugTool::render_ui();
    });

    AxoPlotl::GUI::apply_theme(AxoPlotl::GUI::Theme::ModernLight);

    AxoPlotl::run();

    return 0;
}
