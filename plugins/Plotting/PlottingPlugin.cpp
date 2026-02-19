#include "PlottingPlugin.hpp"
#include "AxoPlotl/PluginRegistry.hpp"
#include "AxoPlotl/Application.hpp"
#include "imgui.h"
#include <ibex/ibex.hpp>

namespace AxoPlotl
{

void PlottingPlugin::render_ui(Application& app)
{
    ImGui::InputText("##x", input_buffer_, sizeof(input_buffer_));

    if (ImGui::Button("Submit")) {
        // Parse Input Text
        ibex::Functions funcs = ibex::common_functions();
        ibex::Variables vars = ibex::common_variables();
        auto rpn = ibex::generate_postfix(ibex::tokenize(input_buffer_));

        auto func = [&](const float& v) {
            vars["x"] = v;
            return ibex::eval_postfix(rpn, vars, funcs);
        };

        for (int i = 0; i < sizeof(samples_); ++i) {
            float x = (float)i/(float)sizeof(samples_);
            samples_[i] = func(x);
        }
    }
    ImGui::PlotLines("##y", samples_, sizeof(samples_));
}


REGISTER_AXOPLOTL_PLUGIN(PlottingPlugin)

}
