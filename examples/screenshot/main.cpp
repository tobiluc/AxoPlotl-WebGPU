#define WEBGPU_CPP_IMPLEMENTATION

#include <AxoPlotl/AxoPlotl.hpp>
#include <AxoPlotl/core/Application.hpp>

int main()
{
    if (!AxoPlotl::init()) {
      std::cerr << "Failed to initialize AxoPlotl" << std::endl;
      return 1;
    }

    // Run for 100 ticks
    AxoPlotl::run(AxoPlotl::RunConfig{.num_ticks=100, .manual_cleanup=true});

    AxoPlotl::app().save_viewport_screenshot("/Users/tobiaskohler/Desktop/AxoPlotl_Screenshot.png", AxoPlotl::app().viewport().size());

    AxoPlotl::cleanup();

    return 0;
}
