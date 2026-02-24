#include "PlottingTool.hpp"
#include "AxoPlotl/Application.hpp"
#include "imgui.h"
#include <ibex/ibex.hpp>

namespace AxoPlotl
{

void PlottingTool::render_ui(Application& app)
{
    if (!ImGui::CollapsingHeader("Plotting")) {return;}

    ImGui::InputText("x(u,v)", x_input_.data(), 1024);
    ImGui::InputText("y(u.v)", y_input_.data(), 1024);
    ImGui::InputText("z(u,v)", z_input_.data(), 1024);
    ImGui::InputFloat2("u", &u_range_[0]);
    ImGui::InputFloat2("v", &v_range_[0]);
    ImGui::InputInt("Resolution", &resolution_);

    if (ImGui::Button("Submit")) {
        // Parse Input Text
        ibex::Functions funcs = ibex::common_functions();
        ibex::Variables vars = ibex::common_variables();
        auto rpn_x = ibex::generate_postfix(ibex::tokenize(x_input_.c_str()));
        auto rpn_y = ibex::generate_postfix(ibex::tokenize(y_input_.c_str()));
        auto rpn_z = ibex::generate_postfix(ibex::tokenize(z_input_.c_str()));
        if (!rpn_x.empty() && !rpn_y.empty() && !rpn_z.empty())
        {
            auto func = [&](const float& u, const float& v) -> Vec3f {
                vars["u"] = u;
                vars["v"] = v;
                return Vec3f{
                    ibex::eval_postfix(rpn_x, vars, funcs),
                    ibex::eval_postfix(rpn_y, vars, funcs),
                    ibex::eval_postfix(rpn_z, vars, funcs)
                };
            };

            VolumeMesh mesh;
            for (int i = 0; i <= resolution_; ++i) {
                float u = u_range_.x + ((float)i/resolution_)*(u_range_.y-u_range_.x);
                for (int j = 0; j <= resolution_; ++j) {
                    float v = v_range_.x + ((float)j/resolution_)*(v_range_.y-v_range_.x);
                    auto p = func(u,v);
                    mesh.add_vertex(OVM::Vec3f(p[0],p[1],p[2]));
                }
            }
            for (int i = 0; i < resolution_; ++i) {
                for (int j = 0; j < resolution_; ++j) {
                    OVM::VH vh0(i*(resolution_+1)+j);
                    OVM::VH vh1(i*(resolution_+1)+j+1);
                    OVM::VH vh2((i+1)*(resolution_+1)+j+1);
                    OVM::VH vh3((i+1)*(resolution_+1)+j);
                    mesh.add_face({vh0,vh1,vh2,vh3});
                }
            }
            app.scene().add_object<VolumeMeshObject>(mesh);
        }
    }
    // ImGui::PlotLines("##y", samples_.data(), samples_.size());
}

}
