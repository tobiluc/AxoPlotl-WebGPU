#include "PlottingTool.hpp"
#include "AxoPlotl/Application.hpp"
#include "imgui.h"
#include <ibex/ibex.hpp>

namespace AxoPlotl
{

void PlottingTool::render_ui(Application& app)
{
    if (!ImGui::CollapsingHeader("Plotting")) {return;}

    ImGui::InputText("##x", input_buffer_, sizeof(input_buffer_));

    if (ImGui::Button("Submit")) {
        // Parse Input Text
        ibex::Functions funcs = ibex::common_functions();
        ibex::Variables vars = ibex::common_variables();
        auto rpn = ibex::generate_postfix(ibex::tokenize(input_buffer_));
        if (!rpn.empty())
        {
            auto func = [&](const float& v) {
                vars["x"] = v;
                return ibex::eval_postfix(rpn, vars, funcs);
            };

            for (int i = 0; i < samples_.size(); ++i) {
                float x = (float)i/(float)samples_.size();
                float y = func(x);
                if (!std::isnan(y)) {
                    samples_[i] = y;
                }
            }
        }
    }
    ImGui::PlotLines("##y", samples_.data(), samples_.size());
}

}
