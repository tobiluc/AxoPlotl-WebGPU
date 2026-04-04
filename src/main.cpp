#include "AxoPlotl/gui/themes.hpp"

#define WEBGPU_CPP_IMPLEMENTATION

#include "AxoPlotl/Application.hpp"
#include "AxoPlotl/tools/DataControlTool.hpp"
#include "AxoPlotl/tools/DebugTool.hpp"
#include "AxoPlotl/tools/PlottingTool.hpp"

int main()
{
  AxoPlotl::Application app;

  AxoPlotl::DebugTool tool_debug;
  AxoPlotl::DataControlTool tool_data_control;
  AxoPlotl::PlottingTool tool_plotting;

  app.inspector_callback() = [&](AxoPlotl::Application* _app)
  {
    tool_data_control.render_ui(*_app);
    tool_plotting.render_ui(*_app);
    tool_debug.render_ui(*_app);
  };

  if (!app.init()) {
      std::cerr << "Failed to initialice AxoPlotl" << std::endl;
      return 1;
  }
  AxoPlotl::GUI::apply_theme(AxoPlotl::GUI::Theme::ModernLight);

  app.run();

  return 0;
}
