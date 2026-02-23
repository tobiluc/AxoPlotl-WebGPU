#include "VolumeMeshObject.hpp"
#include "AxoPlotl/properties/property_calculations.hpp"
#include "AxoPlotl/properties/property_data.hpp"
#include "AxoPlotl/rendering/detail/create_static_render_data.hpp"
#include "imgui.h"
#include <AxoPlotl/utils/commons.hpp>
#include <AxoPlotl/Application.hpp>

namespace AxoPlotl
{

void VolumeMeshObject::render_ui()
{
    if (ImGui::BeginMenu("Render Settings"))
    {
        ImGui::Checkbox("V", &renderer_.vertices().enabled());
        ImGui::SameLine();
        ImGui::Checkbox("E", &renderer_.edges().enabled());
        ImGui::SameLine();
        ImGui::Checkbox("F", &renderer_.faces().enabled());
        ImGui::SameLine();
        ImGui::Checkbox("C", &renderer_.cells().enabled());


        ImGui::SliderFloat("Point Size", &renderer_.vertices().point_size(), 0.0f, 32.0f);
        ImGui::SliderFloat("Line Width", &renderer_.edges().line_width(), 0.0f, 32.0f);
        ImGui::SliderFloat("Cell Scale", &renderer_.cells().cell_scale(), 0.0f, 1.0f);

        // Clip Box
        // Each entity technically has their own, but we
        // just modify all at once.
        const auto& bbox = bounding_box();
        RendererBase::ClipBox& cb = renderer_.vertices().clip_box();
        bool clip_box_enabled = cb.enabled_;
        if (ImGui::Checkbox("Enable Clip Box", &clip_box_enabled)) {
            cb.set(bbox.min(),bbox.max());
        }
        cb.enabled_ = clip_box_enabled;
        if (clip_box_enabled)
        {
            Vec2f x = {cb.min_[0],cb.max_[0]};
            Vec2f y = {cb.min_[1],cb.max_[1]};
            Vec2f z = {cb.min_[2],cb.max_[2]};
            ImGui::SliderFloat2("x", &x[0], bbox.min()[0], bbox.max()[0]);
            ImGui::SliderFloat2("y", &y[0], bbox.min()[1], bbox.max()[1]);
            ImGui::SliderFloat2("z", &z[0], bbox.min()[2], bbox.max()[2]);
            cb.min_ = {x[0],y[0],z[0]};
            cb.max_ = {x[1],y[1],z[1]};
        }
        renderer_.edges().clip_box() = cb;
        renderer_.faces().clip_box() = cb;
        renderer_.cells().clip_box() = cb;
        vertex_vector_renderer_.clip_box() = cb;

        ImGui::EndMenu(); //!Settings
    }
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
                        //upload_property_data<OVM::Vec3d,OVM::Entity::Vertex>(mesh_, *v_prop, prop_filters_,  renderer_);
                        const auto& vectors = vertex_buffer_property_data<OVM::Vec3d,OVM::Entity::Vertex>(mesh_,*v_prop);
                        renderer_.vertices().update_property_data(vectors);
                        vertex_vector_renderer_.update_vector_data(vectors);
                        renderer_.vertices().property_mode() = RendererBase::Property::Mode::VEC3;
                    }
                    renderer_.vertices().enabled() = true;
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

        if (ImGui::BeginMenu("Calculate"))
        {
            if (ImGui::BeginMenu("Cells")) {
                if (ImGui::MenuItem("Minimum Dihedral Angle")) {
                    calc_cell_min_dihedral_angle(mesh_);
                }
                if (ImGui::MenuItem("Boundary Distance")) {
                    calc_cell_boundary_distance(mesh_);
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenu(); //!Calculate
        }

        ImGui::EndMenu(); //!Properties
    }

    if (prop_)
    {
        ImGui::SeparatorText((*prop_)->name().c_str());

        if (vertex_vector_renderer_.enabled()) {
            ImGui::InputFloat("Vector Scale", &vertex_vector_renderer_.vector_scale());
        }

        if (!prop_filters_.empty()) {

            if (ImGui::BeginMenu("Change Filter")) {
                for (int i = 0; i < prop_filters_.size(); ++i) {
                    if (ImGui::MenuItem(prop_filters_[i]->name().c_str())) {
                        filter_index_ = i;
                        prop_filters_[i]->init(renderer_);
                    }
                }
                ImGui::EndMenu();
            }

            prop_filters_[filter_index_]->renderUI(renderer_);
        }

        if (ImGui::Button("Clear Property")) {
            upload_default_property_data();
            renderer_.vertices().property_mode() =
                renderer_.edges().property_mode() =
                renderer_.faces().property_mode() =
                renderer_.cells().property_mode() =
                RendererBase::Property::Mode::COLOR;
            prop_ = std::nullopt;
            prop_filters_.clear();
        }
    }
}

void VolumeMeshObject::init()
{
    const auto& data = create_static_render_data(mesh_);
    renderer_.init(scene_->app(), data);
    upload_default_property_data();
    vertex_vector_renderer_.init(scene_->app(), data.positions_);

    // std::vector<RendererBase::Position> cell_centers;
    // cell_centers.reserve(mesh_.n_cells());
    // for (OVM::CH ch : mesh_.cells()) {
    //     const auto& c = mesh_.barycenter(ch);
    //     cell_centers.emplace_back(c[0],c[1],c[2],1);
    // }
    // cell_vector_renderer_.init(scene_->app(), cell_centers);
}

void VolumeMeshObject::render(
    wgpu::RenderPassEncoder _render_pass,
    const Mat4x4f& _view_projection)
{
    const auto& mvp = _view_projection * transform_;

    renderer_.render(
        scene_->app()->scene_viewport(),
        _render_pass,
        mvp);

    if (renderer_.vertices().enabled() &&
        renderer_.vertices().property_mode() ==
        RendererBase::Property::Mode::VEC3) {
        vertex_vector_renderer_.render(
            scene_->app()->scene_viewport(),
            _render_pass,
            mvp);
    }
}

void VolumeMeshObject::upload_default_property_data()
{
    using D = RendererBase::Property::Data;
    std::vector<D> props;
    for (uint32_t i = 0; i < mesh_.n_vertices(); ++i) {
        props.push_back(D(0,0,0,1));
    }
    renderer_.vertices().update_property_data(props);

    props.clear();
    for (uint32_t i = 0; i < mesh_.n_edges(); ++i) {
        props.push_back(D(0,0,0,1));
    }
    renderer_.edges().update_property_data(props);

    props.clear();
    for (OVM::FH fh : mesh_.faces()) {
        auto p = ToLoG::normalized(mesh_.normal(fh.halfface_handle(0)));
        D sphere_color = Vec4f(
            0.5 * (p[0] + 1),
            0.5 * (p[1] + 1),
            0.5 * (p[2] + 1),
            1
            );
        props.push_back(sphere_color);
    }
    renderer_.faces().update_property_data(props);

    props.clear();
    for (OVM::CH ch : mesh_.cells()) {
        auto p = ToLoG::normalized(mesh_.barycenter(ch));
        D sphere_color = Vec4f(
            0.5 * (p[0] + 1),
            0.5 * (p[1] + 1),
            0.5 * (p[2] + 1),
            1
            );
        props.push_back(sphere_color);
    }
    renderer_.cells().update_property_data(props);
}

void VolumeMeshObject::recompute_bounding_box()
{
    bbox_.make_empty();
    for (const auto& p : mesh_.vertex_positions()) {
        bbox_.expand(Vec3f(p[0],p[1],p[2]));
    }
    bbox_ = bbox_.scaled(1.01f);
}

}
