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
}

void OpenVolumeMeshObject::render_ui_info()
{
    ImGui::Text("V/E/F/C = %zu/%zu/%zu/%zu",
        mesh_.n_vertices(), mesh_.n_edges(),
        mesh_.n_faces(), mesh_.n_cells());
}

void OpenVolumeMeshObject::render_ui_properties()
{
    if (ImGui::BeginMenu("Calculate Property"))
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
                if (ImGui::MenuItem(string_format("%s [%s]", (*pp)->name().c_str(), (*pp)->typeNameWrapper().c_str()).c_str())) {
                    prop<EntityTag>().prop_ = *pp;
                    selected_prop_entity_type_ = EntityTag::type();
                    if ((*pp)->typeNameWrapper()=="double") {
                        upload_property_data<double,EntityTag>(mesh_, *pp, prop<EntityTag>().filters_,  renderer_);
                    } else if ((*pp)->typeNameWrapper()=="int") {
                        upload_property_data<int,EntityTag>(mesh_, *pp, prop<EntityTag>().filters_,  renderer_);
                    } else if ((*pp)->typeNameWrapper()=="float") {
                        upload_property_data<float,EntityTag>(mesh_, *pp, prop<EntityTag>().filters_,  renderer_);
                    } else if ((*pp)->typeNameWrapper()=="bool") {
                        upload_property_data<bool,EntityTag>(mesh_, *pp, prop<EntityTag>().filters_,  renderer_);
                    } else if ((*pp)->typeNameWrapper()=="vec3d") {
                        //upload_property_data<OVM::Vec3d,OVM::Entity::Vertex>(mesh_, *v_prop, prop_filters_,  renderer_);
                        // const auto& vectors = vertex_buffer_property_data<OVM::Vec3d,OVM::Entity::Vertex>(mesh_,*v_prop);
                        // renderer_.vertices().update_property_data(vectors);
                        // vertex_vector_renderer_.update_vector_data(vectors);
                        // renderer_.vertices().property_mode() = RendererBase::Property::Mode::VEC3;
                    }
                    renderer_.vertices().enabled() = true;
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
                        prop<EntityTag>().filters_[i]->init(renderer_);
                    }
                }
                ImGui::EndMenu();
            }
            prop<EntityTag>().filters_[prop<EntityTag>().filter_index_]->renderUI(renderer_);
        }

        // Clear
        if (ImGui::Button("Clear Property")) {
            upload_default_property_data<EntityTag>();
            renderer_.entities<EntityTag>().property_mode()
                = RendererBase::Property::Mode::COLOR;
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
    renderer_.init(scene_->app(), data);
    upload_default_property_data<OVM::Entity::Vertex>();
    upload_default_property_data<OVM::Entity::Edge>();
    upload_default_property_data<OVM::Entity::Face>();
    upload_default_property_data<OVM::Entity::Cell>();
    vertex_vector_renderer_.init(scene_->app(), data.positions_);

    // std::vector<Vec4f> cell_centers;
    // std::vector<RendererBase::Property::Data> cell_colors;
    // cell_centers.reserve(data.cells_.size());
    // for (uint32_t ch = 0; ch < data.cells_.size(); ++ch) {
    //     cell_centers.push_back({0,0,0,0});
    //     cell_colors.push_back({dist(mt),dist(mt),dist(mt),1});
    //     uint32_t n(0);
    //     for (const auto& f : data.cells_[ch]) {
    //         for (const auto& vh : f) {
    //             cell_centers.back() += data.positions_[vh];
    //             ++n;
    //         }
    //     }
    //     cell_centers.back() /= n;
    // }

    // cell_translucent_renderer_.init(
    //     scene_->app(),
    //     renderer_.position_buffer(),
    //     data.cells_,
    //     cell_centers
    // );
    // cell_translucent_renderer_.update_property_data(cell_colors);
    // cell_translucent_renderer_.property_mode() = RendererBase::Property::Mode::COLOR;


    // std::vector<RendererBase::Position> cell_centers;
    // cell_centers.reserve(mesh_.n_cells());
    // for (OVM::CH ch : mesh_.cells()) {
    //     const auto& c = mesh_.barycenter(ch);
    //     cell_centers.emplace_back(c[0],c[1],c[2],1);
    // }
    // cell_vector_renderer_.init(scene_->app(), cell_centers);
}

void OpenVolumeMeshObject::render(
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

    // cell_translucent_renderer_.render(
    //     scene_->app()->scene_viewport(),
    //     _render_pass,
    //     mvp);
}

void OpenVolumeMeshObject::upload_default_vertex_property_data()
{
    using D = RendererBase::Property::Data;
    std::vector<D> props;
    props.reserve(mesh_.n_vertices());
    for (uint32_t i = 0; i < mesh_.n_vertices(); ++i) {
        props.push_back(D(0,0,0,1));
    }
    renderer_.vertices().update_property_data(props);
}
void OpenVolumeMeshObject::upload_default_edge_property_data()
{
    using D = RendererBase::Property::Data;
    std::vector<D> props;
    props.reserve(mesh_.n_edges());
    for (uint32_t i = 0; i < mesh_.n_edges(); ++i) {
        props.push_back(D(0,0,0,1));
    }
    renderer_.edges().update_property_data(props);
}
void OpenVolumeMeshObject::upload_default_face_property_data()
{
    using D = RendererBase::Property::Data;
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
    renderer_.faces().update_property_data(props);
}
void OpenVolumeMeshObject::upload_default_cell_property_data()
{
    using D = RendererBase::Property::Data;
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
    renderer_.cells().update_property_data(props);
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
