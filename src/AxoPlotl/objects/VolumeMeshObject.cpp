#include "VolumeMeshObject.hpp"
#include "AxoPlotl/properties/property_data.hpp"
#include "AxoPlotl/rendering/detail/create_static_render_data.hpp"
#include "imgui.h"
#include <AxoPlotl/utils/commons.hpp>
#include <AxoPlotl/Application.hpp>

namespace AxoPlotl
{

void VolumeMeshObject::render_ui()
{
    if (ImGui::BeginMenu("Select Property"))
    {
        if (mesh_.n_vertex_props()>0 && ImGui::BeginMenu("Vertices"))
        {
            for (auto v_prop = mesh_.vertex_props_begin(); v_prop != mesh_.vertex_props_end(); ++v_prop) {
                ImGui::PushID((*v_prop)->name().c_str());
                if (ImGui::MenuItem(string_format("%s [%s]", (*v_prop)->name().c_str(), (*v_prop)->typeNameWrapper().c_str()).c_str())) {
                    prop_ = *v_prop;
                    if ((*v_prop)->typeNameWrapper()=="double") {
                        upload_property_data<double,OVM::Entity::Vertex>(mesh_, *v_prop, prop_filters_,  renderer_);
                    } else if ((*v_prop)->typeNameWrapper()=="int") {
                        upload_property_data<int,OVM::Entity::Vertex>(mesh_, *v_prop, prop_filters_,  renderer_);
                    } else if ((*v_prop)->typeNameWrapper()=="float") {
                        upload_property_data<float,OVM::Entity::Vertex>(mesh_, *v_prop, prop_filters_,  renderer_);
                    } else if ((*v_prop)->typeNameWrapper()=="bool") {
                        upload_property_data<bool,OVM::Entity::Vertex>(mesh_, *v_prop, prop_filters_,  renderer_);
                    } else if ((*v_prop)->typeNameWrapper()=="vec3d") {
                        upload_property_data<OVM::Vec3d,OVM::Entity::Vertex>(mesh_, *v_prop, prop_filters_,  renderer_);
                    }
                }
                ImGui::PopID();
            }

            ImGui::EndMenu(); // Vertex Props
        }

        if (mesh_.n_edge_props()>0 && ImGui::BeginMenu("Edges"))
        {
            for (auto e_prop = mesh_.edge_props_begin(); e_prop != mesh_.edge_props_end(); ++e_prop) {
                ImGui::PushID((*e_prop)->name().c_str());
                if (ImGui::MenuItem(string_format("%s [%s]", (*e_prop)->name().c_str(), (*e_prop)->typeNameWrapper().c_str()).c_str())) {
                    prop_ = *e_prop;
                    if ((*e_prop)->typeNameWrapper()=="double") {
                        upload_property_data<double,OVM::Entity::Edge>(mesh_, *e_prop, prop_filters_,  renderer_);
                    } else if ((*e_prop)->typeNameWrapper()=="int") {
                        upload_property_data<int,OVM::Entity::Edge>(mesh_, *e_prop, prop_filters_,  renderer_);
                    } else if ((*e_prop)->typeNameWrapper()=="float") {
                        upload_property_data<float,OVM::Entity::Edge>(mesh_, *e_prop, prop_filters_,  renderer_);
                    } else if ((*e_prop)->typeNameWrapper()=="bool") {
                        upload_property_data<bool,OVM::Entity::Edge>(mesh_, *e_prop, prop_filters_,  renderer_);
                    } else if ((*e_prop)->typeNameWrapper()=="vec3d") {
                        upload_property_data<OVM::Vec3d,OVM::Entity::Edge>(mesh_, *e_prop, prop_filters_,  renderer_);
                    }
                }
                ImGui::PopID();
            }

            ImGui::EndMenu(); // Edge Props
        }

        if (mesh_.n_face_props()>0 && ImGui::BeginMenu("Faces"))
        {
            for (auto f_prop = mesh_.face_props_begin(); f_prop != mesh_.face_props_end(); ++f_prop) {
                ImGui::PushID((*f_prop)->name().c_str());
                if (ImGui::MenuItem(string_format("%s [%s]", (*f_prop)->name().c_str(), (*f_prop)->typeNameWrapper().c_str()).c_str())) {
                    prop_ = *f_prop;
                    if ((*f_prop)->typeNameWrapper()=="double") {
                        upload_property_data<double,OVM::Entity::Face>(mesh_, *f_prop, prop_filters_,  renderer_);
                    } else if ((*f_prop)->typeNameWrapper()=="int") {
                        upload_property_data<int,OVM::Entity::Face>(mesh_, *f_prop, prop_filters_,  renderer_);
                    } else if ((*f_prop)->typeNameWrapper()=="float") {
                        upload_property_data<float,OVM::Entity::Face>(mesh_, *f_prop, prop_filters_,  renderer_);
                    } else if ((*f_prop)->typeNameWrapper()=="bool") {
                        upload_property_data<bool,OVM::Entity::Face>(mesh_, *f_prop, prop_filters_,  renderer_);
                    } else if ((*f_prop)->typeNameWrapper()=="vec3d") {
                        upload_property_data<OVM::Vec3d,OVM::Entity::Face>(mesh_, *f_prop, prop_filters_,  renderer_);
                    }
                }
                ImGui::PopID();
            }

            ImGui::EndMenu(); // Face Props
        }

        if (mesh_.n_cell_props()>0 && ImGui::BeginMenu("Cells"))
        {
            for (auto c_prop = mesh_.cell_props_begin(); c_prop != mesh_.cell_props_end(); ++c_prop) {
                ImGui::PushID((*c_prop)->name().c_str());
                if (ImGui::MenuItem(string_format("%s [%s]", (*c_prop)->name().c_str(), (*c_prop)->typeNameWrapper().c_str()).c_str())) {
                    prop_ = *c_prop;
                    if ((*c_prop)->typeNameWrapper()=="double") {
                        upload_property_data<double,OVM::Entity::Cell>(mesh_, *c_prop, prop_filters_,  renderer_);
                    } else if ((*c_prop)->typeNameWrapper()=="int") {
                        upload_property_data<int,OVM::Entity::Cell>(mesh_, *c_prop, prop_filters_,  renderer_);
                    } else if ((*c_prop)->typeNameWrapper()=="float") {
                        upload_property_data<float,OVM::Entity::Cell>(mesh_, *c_prop, prop_filters_,  renderer_);
                    } else if ((*c_prop)->typeNameWrapper()=="bool") {
                        upload_property_data<bool,OVM::Entity::Cell>(mesh_, *c_prop, prop_filters_,  renderer_);
                    } else if ((*c_prop)->typeNameWrapper()=="vec3d") {
                        upload_property_data<OVM::Vec3d,OVM::Entity::Cell>(mesh_, *c_prop, prop_filters_,  renderer_);
                    }
                }
                ImGui::PopID();
            }

            ImGui::EndMenu(); // Cell Props
        }

        ImGui::EndMenu(); //!Properties
    }

    if (prop_)
    {
        ImGui::SeparatorText((*prop_)->name().c_str());

        if (!prop_filters_.empty()) {

            if (ImGui::BeginMenu("Change Filter")) {
                for (int i = 0; i < prop_filters_.size(); ++i) {
                    if (ImGui::MenuItem(prop_filters_[i]->name().c_str())) {
                        filter_index_ = i;
                    }
                }
                ImGui::EndMenu();
            }

            prop_filters_[filter_index_]->renderUI(renderer_);
        }

        if (ImGui::Button("Clear Property")) {
            upload_default_property_data();
            renderer_.vertex_property_.mode_ =
                renderer_.edge_property_.mode_ =
                renderer_.face_property_.mode_ =
                renderer_.cell_property_.mode_ =
                VolumeMeshRenderer::Property::Mode::COLOR;
            prop_ = std::nullopt;
            prop_filters_.clear();
        }
    }

    if (ImGui::BeginMenu("Settings"))
    {
        ImGui::Checkbox("V", &renderer_.render_vertices_);
        ImGui::SameLine();
        ImGui::Checkbox("E", &renderer_.render_edges_);
        ImGui::SameLine();
        ImGui::Checkbox("F", &renderer_.render_faces_);
        ImGui::SameLine();
        ImGui::Checkbox("C", &renderer_.render_cells_);


        ImGui::SliderFloat("Point Size", &renderer_.point_size_, 0.0f, 32.0f);
        ImGui::SliderFloat("Line Width", &renderer_.line_width_, 0.0f, 32.0f);
        ImGui::EndMenu(); //!Settings
    }
}

void VolumeMeshObject::init()
{
    renderer_.init(
        scene_->app(),
        create_static_render_data(mesh_)
    );
    upload_default_property_data();
}

void VolumeMeshObject::upload_default_property_data()
{
    std::vector<VolumeMeshRenderer::Property::Data> props;
    for (uint32_t i = 0; i < mesh_.n_vertices(); ++i) {
        props.push_back({.value_ = Vec4f(0,0,0,1)});
    }
    renderer_.update_vertex_property_data(props);

    props.clear();
    for (uint32_t i = 0; i < mesh_.n_edges(); ++i) {
        props.push_back({.value_ = Vec4f(0,0,0,1)});
    }
    renderer_.update_edge_property_data(props);

    props.clear();
    for (OVM::FH fh : mesh_.faces()) {
        auto p = ToLoG::normalized(mesh_.normal(fh.halfface_handle(0)));
        Vec4f sphere_color = Vec4f(
            0.5 * (p[0] + 1),
            0.5 * (p[1] + 1),
            0.5 * (p[2] + 1),
            1
            );
        props.push_back({.value_ = sphere_color});
    }
    renderer_.update_face_property_data(props);

    props.clear();
    for (OVM::CH ch : mesh_.cells()) {
        auto p = ToLoG::normalized(mesh_.barycenter(ch));
        Vec4f sphere_color = Vec4f(
            0.5 * (p[0] + 1),
            0.5 * (p[1] + 1),
            0.5 * (p[2] + 1),
            1
            );
        props.push_back({.value_ = sphere_color});
    }
    renderer_.update_cell_property_data(props);
}

void VolumeMeshObject::recompute_bounding_box()
{
    bbox_.make_empty();
    for (const auto& p : mesh_.vertex_positions()) {
        bbox_.expand(Vec3f(p[0],p[1],p[2]));
    }
}

}
