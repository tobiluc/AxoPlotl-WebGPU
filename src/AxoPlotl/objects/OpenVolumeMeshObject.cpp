#include "OpenVolumeMeshObject.hpp"
#include "AxoPlotl/properties/property_calculations.hpp"
#include "AxoPlotl/properties/property_data.hpp"
#include "AxoPlotl/rendering/detail/create_static_render_data.hpp"
#include "IconsFontAwesome7.h"
#include "imgui.h"
#include <AxoPlotl/utils/commons.hpp>
#include <AxoPlotl/Application.hpp>
#include <random>

namespace AxoPlotl
{

void OpenVolumeMeshObject::render_ui_settings()
{
    ImGui::Checkbox("V", &vertex_renderer_.enabled());
    ImGui::SameLine();
    ImGui::Checkbox("E", &edge_renderer_.enabled());
    ImGui::SameLine();
    ImGui::Checkbox("F", &face_renderer_.enabled());
    ImGui::SameLine();
    ImGui::Checkbox("C", &cell_renderer_.enabled());

    ImGui::SliderFloat("Point Size", &vertex_renderer_.point_size(), 0.0f, 32.0f);
    ImGui::SliderFloat("Line Width", &edge_renderer_.line_width(), 0.0f, 32.0f);
    ImGui::SliderFloat("Cell Scale", &cell_renderer_.cell_scale(), 0.0f, 1.0f);

    // Clip Box
    // Each entity technically has their own, but we
    // just modify all at once.
    const auto& bbox = bounding_box();
    PropertyRendererBase::ClipBox& cb = vertex_renderer_.clip_box();
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
    edge_renderer_.clip_box() = cb;
    face_renderer_.clip_box() = cb;
    cell_renderer_.clip_box() = cb;
    // vertex_vector_renderer_.clip_box() = cb;
}

void OpenVolumeMeshObject::render_ui_info()
{
    ImGui::Text("V/E/F/C = %zu/%zu/%zu/%zu",
        mesh_.n_vertices(), mesh_.n_edges(),
        mesh_.n_faces(), mesh_.n_cells());
}

void OpenVolumeMeshObject::render_ui_properties()
{
    if (ImGui::BeginMenu("Calculate"))
    {
        if (mesh_.n_cells()>0 && ImGui::BeginMenu("Cells")) {
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

    auto render_property_selection_menu = [&]<typename EntityTag>(const std::string& _prefix)
    {
        ImGui::PushID(_prefix.c_str());

        std::string title = _prefix + ": ";
        if (prop<EntityTag>().prop_.has_value()) {title += (*prop<EntityTag>().prop_)->name();}
        else {title += "none";}

        // Select Property
        if (mesh_.n_props<EntityTag>()>0 &&
            ImGui::BeginMenu(title.c_str()))
        {
            // Select an already selected property
            if (ImGui::IsItemClicked()) {
                selected_prop_entity_type_ = EntityTag::type();
            }

            for (auto pp = mesh_.persistent_props_begin<EntityTag>();
                 pp != mesh_.persistent_props_end<EntityTag>(); ++pp) {
                ImGui::PushID((*pp)->name().c_str());

                auto upload_data = [&]<typename T>() {
                    upload_buffer_property_data<T,EntityTag>(
                        mesh_,
                        *pp,
                        prop<EntityTag>().filters_,
                        renderer<EntityTag>()
                    );
                    prop<EntityTag>().filter_index_ = 0;
                };

                if (ImGui::MenuItem(string_format("%s [%s]", (*pp)->name().c_str(), (*pp)->typeNameWrapper().c_str()).c_str())) {
                    prop<EntityTag>().prop_ = *pp;
                    selected_prop_entity_type_ = EntityTag::type();
                    if ((*pp)->typeNameWrapper()=="double") {
                        upload_data.template operator()<double>();
                    } else if ((*pp)->typeNameWrapper()=="int") {
                        upload_data.template operator()<int>();
                    } else if ((*pp)->typeNameWrapper()=="uint") {
                        upload_data.template operator()<unsigned int>();
                    } else if ((*pp)->typeNameWrapper()=="float") {
                        upload_data.template operator()<float>();
                    } else if ((*pp)->typeNameWrapper()=="bool") {
                        upload_data.template operator()<bool>();
                    } else if ((*pp)->typeNameWrapper()=="short") {
                        upload_data.template operator()<short>();
                    } else if ((*pp)->typeNameWrapper()=="ushort") {
                        upload_data.template operator()<unsigned short>();
                    } else if ((*pp)->typeNameWrapper()=="char") {
                        upload_data.template operator()<char>();
                    } else if ((*pp)->typeNameWrapper()=="uchar") {
                        upload_data.template operator()<unsigned char>();
                    } else if ((*pp)->typeNameWrapper()=="long") {
                        upload_data.template operator()<long>();
                    } else if ((*pp)->typeNameWrapper()=="ulong") {
                        upload_data.template operator()<unsigned long>();
                    } else if ((*pp)->typeNameWrapper()=="vec3d") {
                        upload_data.template operator()<OVM::Vec3d>();
                    } else if ((*pp)->typeNameWrapper()=="vec3f") {
                        upload_data.template operator()<OVM::Vec3f>();
                    } else if ((*pp)->typeNameWrapper()=="vec4d") {
                        upload_data.template operator()<OVM::Vec4d>();
                    } else if ((*pp)->typeNameWrapper()=="vec4f") {
                        upload_data.template operator()<OVM::Vec4f>();
                    }
                    renderer<EntityTag>().enabled() = true;
                }
                ImGui::PopID();
            }
            ImGui::EndMenu(); // Props
        }
        ImGui::PopID();
    };

    auto render_property_visualization_settings_menu = [&]<typename EntityTag>()
    {
        if (!selected_prop_entity_type_.has_value()
            || *selected_prop_entity_type_ != EntityTag::type()
            || !prop<EntityTag>().prop_.has_value())
        {return;}

        // Render Property Filter Settings
        if (!prop<EntityTag>().filters_.empty()) {

            if (ImGui::BeginMenu("Change Filter")) {
                for (int i = 0; i < prop<EntityTag>().filters_.size(); ++i) {
                    if (ImGui::MenuItem(prop<EntityTag>().filters_[i]->name().c_str())) {
                        prop<EntityTag>().filter_index_ = i;
                        prop<EntityTag>().filters_[i]->init(renderer<EntityTag>());
                    }
                }
                ImGui::EndMenu();
            }
            prop<EntityTag>().filters_[prop<EntityTag>().filter_index_]->render_ui(renderer<EntityTag>());
        }

        // Clear
        if (ImGui::Button("Clear Property")) {
            upload_default_property_data<EntityTag>();
            renderer<EntityTag>().property_type()
                = PropertyRendererBase::Property::Type::COLOR;
            prop<EntityTag>().prop_ = std::nullopt;
            prop<EntityTag>().filters_.clear();
        }
    };

    if (ImGui::BeginMenu("Select")) {
        render_property_selection_menu.operator()<OVM::Entity::Vertex>(" V");
        render_property_selection_menu.operator()<OVM::Entity::Edge>(" E");
        render_property_selection_menu.operator()<OVM::Entity::Face>(" F");
        render_property_selection_menu.operator()<OVM::Entity::Cell>(" C");
        ImGui::EndMenu();
    }
    render_property_visualization_settings_menu.operator()<OVM::Entity::Vertex>();
    render_property_visualization_settings_menu.operator()<OVM::Entity::Edge>();
    render_property_visualization_settings_menu.operator()<OVM::Entity::Face>();
    render_property_visualization_settings_menu.operator()<OVM::Entity::Cell>();
}

void OpenVolumeMeshObject::init()
{
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    const auto& data = create_static_render_data(mesh_);
    n_positions_ = mesh_.n_vertices();
    vertices_position_buffer_ = create_position_buffer(scene_->app()->device_, data.positions_);
    cells_center_buffer_ = create_position_buffer(scene_->app()->device_, get_cell_centers(mesh_));
    vertex_renderer_.init(scene_->app(), vertices_position_buffer_, data.vertices_);
    edge_renderer_.init(scene_->app(), vertices_position_buffer_, data.edges_);
    face_renderer_.init(scene_->app(), vertices_position_buffer_, data.faces_);
    cell_renderer_.init(scene_->app(), vertices_position_buffer_, data.cells_, cells_center_buffer_);
    //vectors_on_vertices_renderer_.init(scene_->app(), vertices_position_buffer_);

    upload_default_property_data<OVM::Entity::Vertex>();
    upload_default_property_data<OVM::Entity::Edge>();
    upload_default_property_data<OVM::Entity::Face>();
    upload_default_property_data<OVM::Entity::Cell>();
}

void OpenVolumeMeshObject::render(
    wgpu::RenderPassEncoder _render_pass,
    const Mat4x4f& _view_projection)
{
    if (deleted() || !visible()) {return;}

    const auto& mvp = _view_projection * transform_;
    const auto& vp = scene_->app()->scene_viewport();

    cell_renderer_.render(vp, _render_pass, mvp);
    face_renderer_.render(vp, _render_pass, mvp);
    edge_renderer_.render(vp, _render_pass, mvp);
    vertex_renderer_.render(vp, _render_pass, mvp);

    // renderer_.render(
    //     scene_->app()->scene_viewport(),
    //     _render_pass,
    //     mvp);

    // if (renderer_.vertices().enabled() &&
    //     renderer_.vertices().property_type() ==
    //     PropertyRendererBase::Property::Type::VEC3) {
    //     vertex_vector_renderer_.render(
    //         scene_->app()->scene_viewport(),
    //         _render_pass,
    //         mvp);
    // }

    // cell_translucent_renderer_.render(
    //     scene_->app()->scene_viewport(),
    //     _render_pass,
    //     mvp);
}

void OpenVolumeMeshObject::upload_default_vertex_property_data()
{
    using D = PropertyRendererBase::Property::Data;
    std::vector<D> props;
    props.reserve(mesh_.n_vertices());
    for (uint32_t i = 0; i < mesh_.n_vertices(); ++i) {
        props.push_back(D(0,0,0,1));
    }
    vertex_renderer_.update_property_data(props);
}
void OpenVolumeMeshObject::upload_default_edge_property_data()
{
    using D = PropertyRendererBase::Property::Data;
    std::vector<D> props;
    props.reserve(mesh_.n_edges());
    for (uint32_t i = 0; i < mesh_.n_edges(); ++i) {
        props.push_back(D(0,0,0,1));
    }
    edge_renderer_.update_property_data(props);
}
void OpenVolumeMeshObject::upload_default_face_property_data()
{
    using D = PropertyRendererBase::Property::Data;
    std::vector<D> props;
    props.reserve(mesh_.n_faces());
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
    face_renderer_.update_property_data(props);
}
void OpenVolumeMeshObject::upload_default_cell_property_data()
{
    using D = PropertyRendererBase::Property::Data;
    std::vector<D> props;
    props.reserve(mesh_.n_cells());
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
    cell_renderer_.update_property_data(props);
}

void OpenVolumeMeshObject::recompute_bounding_box()
{
    bbox_.make_empty();
    for (const auto& p : mesh_.vertex_positions()) {
        bbox_.expand(Vec3f(p[0],p[1],p[2]));
    }
    bbox_ = bbox_.scaled(1.01f);
}

}
