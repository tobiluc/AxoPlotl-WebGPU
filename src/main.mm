#include "AxoPlotl/gui/themes.hpp"
#define WEBGPU_CPP_IMPLEMENTATION

#include "AxoPlotl/Application.hpp"
#include "AxoPlotl/tools/DataControlTool.hpp"
#include "AxoPlotl/tools/DebugTool.hpp"
#include "AxoPlotl/tools/PlottingTool.hpp"

AxoPlotl::DebugTool tool_debug;
AxoPlotl::DataControlTool tool_data_control;
AxoPlotl::PlottingTool tool_plotting;

void user_ui_callback(AxoPlotl::Application* _app)
{
  tool_data_control.render_ui(*_app);
  tool_plotting.render_ui(*_app);
  tool_debug.render_ui(*_app);
}

int main()
{
  // Initialize main Runner
  AxoPlotl::Application app;
  app.user_ui_callback() = user_ui_callback;
  if (!app.init()) {
      std::cerr << "Failed to initialice AxoPlotl" << std::endl;
      return 1;
  }
  AxoPlotl::GUI::apply_theme(AxoPlotl::GUI::Theme::ModernLight);

  // auto obj = app.scene().add_openvolumemesh("/Users/tobiaskohler/Uni/OpenFlipper-Free/libs/libigrec/build/Desktop_arm_darwin_generic_mach_o_64bit/Build/bin/RelWithDebInfo/tet_mesh.ovmb");
  // obj->visualize_property("IGRec::c_frame_dot");

  app.run();

  return 0;
}
