#define WEBGPU_CPP_IMPLEMENTATION

#include "AxoPlotl/AxoPlotl.hpp"
#include "AxoPlotl/gui/themes.hpp"
#include "AxoPlotl/tools/DataControlTool.hpp"
#include "AxoPlotl/tools/DebugTool.hpp"
#include "AxoPlotl/tools/PlottingTool.hpp"

#include "AxoPlotl/properties/property_visualization_settings.hpp"
#include "AxoPlotl/rendering/draw_texture.hpp"
#include <AxoPlotl/Application.hpp>

int main()
{
    if (!AxoPlotl::init()) {
      std::cerr << "Failed to initialize AxoPlotl" << std::endl;
      return 1;
    }

    AxoPlotl::internal::VisualizationSettings settings;
    // AxoPlotl::PickingTextureRenderer r;
    // r.init(AxoPlotl::scene().app()->device_, 256, 256);

    AxoPlotl::set_inspector_callback([&]()
    {
        AxoPlotl::DataControlTool::render_ui();
        AxoPlotl::PlottingTool::render_ui();
        AxoPlotl::DebugTool::render_ui();

        // r.resize(AxoPlotl::scene().app()->viewport().width(),
        //          AxoPlotl::scene().app()->viewport().height());
        // wgpu::CommandEncoder encoder = AxoPlotl::scene().app()->device_.createCommandEncoder();
        // r.render(encoder, AxoPlotl::scene().app()->viewport().picking_view(), AxoPlotl::PickingTextureRenderer::Mode::EntityType);
        // wgpu::CommandBuffer cb = encoder.finish();
        // AxoPlotl::scene().app()->device_.getQueue().submit(1, &cb);
        // cb.release();
        // encoder.release();

        // AxoPlotl::draw_texture("PickingTexture", r.get_texture_view());

        settings.render_ui();
    });

    AxoPlotl::GUI::apply_theme(AxoPlotl::GUI::Theme::ModernLight);

    AxoPlotl::run();

    return 0;
}
