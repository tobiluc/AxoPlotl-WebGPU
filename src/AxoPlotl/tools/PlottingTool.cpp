#include "PlottingTool.hpp"
#include <AxoPlotl/Scene.hpp>
#include "AxoPlotl/AxoPlotl.hpp"
#include "imgui.h"
#include <ibex/ibex.hpp>

namespace AxoPlotl
{

PlottingTool::Input PlottingTool::input_;
std::unordered_map<int,PlottingTool::Input> PlottingTool::objects_;
int PlottingTool::selected_id_ = -1;

void PlottingTool::render_ui()
{
    if (!ImGui::CollapsingHeader("Plotting")) {return;}

    if (ImGui::BeginMenu("Modify")) {
        for (auto& obj : AxoPlotl::scene().get_objects()) {
            if (objects_.contains(obj->id())) {
                ImGui::PushID(obj->id());
                if (ImGui::MenuItem(obj->name().c_str())) {
                    input_ = objects_[obj->id()];
                    selected_id_ = obj->id();
                }
                ImGui::PopID();
            }
        }
        ImGui::EndMenu();
    }

    if (selected_id_ >= 0 && objects_.contains(selected_id_)) {
        ImGui::SeparatorText(AxoPlotl::scene().get_object(selected_id_)->name().c_str());
    }

    ImGui::InputText("x(u,v)", input_.x_.data(), 1024);
    ImGui::InputText("y(u.v)", input_.y_.data(), 1024);
    ImGui::InputText("z(u,v)", input_.z_.data(), 1024);
    ImGui::InputFloat2("u", &input_.u_[0]);
    ImGui::InputFloat2("v", &input_.v_[0]);
    ImGui::InputInt("Resolution", &input_.resolution_);

    if (ImGui::Button("Submit")) {
        // Parse Input Text
        ibex::Functions funcs = ibex::common_functions();
        ibex::Variables vars = ibex::common_variables();
        auto rpn_x = ibex::generate_postfix(ibex::tokenize(input_.x_.c_str()));
        auto rpn_y = ibex::generate_postfix(ibex::tokenize(input_.y_.c_str()));
        auto rpn_z = ibex::generate_postfix(ibex::tokenize(input_.z_.c_str()));
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

            OVMVolumeMesh mesh;
            mesh.reserve_vertices((input_.resolution_+1)*(input_.resolution_+1));
            mesh.reserve_edges(2*(input_.resolution_+1)*(input_.resolution_+1));
            mesh.reserve_faces(input_.resolution_*input_.resolution_);
            auto u_prop = mesh.create_persistent_vertex_property<float>("u").value();
            auto v_prop = mesh.create_persistent_vertex_property<float>("v").value();
            using PointT = OVMVolumeMesh::PointT;
            for (int i = 0; i <= input_.resolution_; ++i) {
                float u = input_.u_.x + ((float)i/input_.resolution_)*(input_.u_.y-input_.u_.x);
                for (int j = 0; j <= input_.resolution_; ++j) {
                    float v = input_.v_.x + ((float)j/input_.resolution_)*(input_.v_.y-input_.v_.x);
                    auto p = func(u,v);
                    OVM::VH vh = mesh.add_vertex(PointT(p[0],p[1],p[2]));
                    u_prop[vh] = u;
                    v_prop[vh] = v;
                }
            }
            for (int i = 0; i < input_.resolution_; ++i) {
                for (int j = 0; j < input_.resolution_; ++j) {
                    OVM::VH vh0(i*(input_.resolution_+1)+j);
                    OVM::VH vh1(i*(input_.resolution_+1)+j+1);
                    OVM::VH vh2((i+1)*(input_.resolution_+1)+j+1);
                    OVM::VH vh3((i+1)*(input_.resolution_+1)+j);
                    mesh.add_face({vh0,vh1,vh2,vh3});
                }
            }
            auto obj = AxoPlotl::scene().add_mesh(std::move(mesh));
            objects_[obj->id()] = input_;
        }
    }
}

}
