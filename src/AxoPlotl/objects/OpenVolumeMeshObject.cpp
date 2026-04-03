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
                 pp != mesh_.persistent_props_end<EntityTag>(); ++pp)
            {
                ImGui::PushID((*pp)->name().c_str());
                if (ImGui::MenuItem(string_format("%s [%s]", (*pp)->name().c_str(), (*pp)->typeNameWrapper().c_str()).c_str())) {
                    visualize_property((*pp)->name(), EntityTag::type(), (*pp)->typeNameWrapper());
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

void OpenVolumeMeshObject::init_gpu_buffers()
{
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    const auto& data = create_static_render_data(mesh_);
    n_positions_ = mesh_.n_vertices();
    vertices_position_buffer_ = create_position_buffer(scene_->app()->device_, data.positions_);
    cells_center_buffer_ = create_position_buffer(scene_->app()->device_, get_cell_centers(mesh_));
    vertex_renderer_.init(id(), scene_->app(), vertices_position_buffer_, data.vertices_);
    edge_renderer_.init(id(), scene_->app(), vertices_position_buffer_, data.edges_);
    face_renderer_.init(id(), scene_->app(), vertices_position_buffer_, data.faces_);
    cell_renderer_.init(id(), scene_->app(), vertices_position_buffer_, data.cells_, cells_center_buffer_);
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

void OpenVolumeMeshObject::render_ui_picking(PickResult _p, const PickConfig &_cfg)
{
    if (_p.object_id_ != id()) [[unlikely]] {return;}

    auto squared_distance = [](const Vec3f _p0, const OVM::Vec3f& _p1) -> float {
        float dx = _p0[0] - _p1[0];
        float dy = _p0[1] - _p1[1];
        float dz = _p0[2] - _p1[2];
        return dx*dx + dy*dy + dz*dz;
    };

    auto show_prop_list = [&]<typename EntityTag>() {
        for (auto pp = mesh_.persistent_props_begin<EntityTag>();
             pp != mesh_.persistent_props_end<EntityTag>(); ++pp)
        {
            auto show_item = [&]<typename T>() {
                auto prop = mesh_.get_property<T,EntityTag>((*pp)->name()).value();
                const T& val = prop[OVM::handle_for_tag_t<EntityTag>(_p.index_)];
                ImGui::Text("%s: %s",
                    (*pp)->name().c_str(),
                    value_to_string(val).c_str()
                );
            };

            ImGui::PushID((*pp)->name().c_str());
            if ((*pp)->typeNameWrapper()=="double") {
                show_item.template operator()<double>();
            } else if ((*pp)->typeNameWrapper()=="int") {
                show_item.template operator()<int>();
            } else if ((*pp)->typeNameWrapper()=="uint") {
                show_item.template operator()<unsigned int>();
            } else if ((*pp)->typeNameWrapper()=="float") {
                show_item.template operator()<float>();
            } else if ((*pp)->typeNameWrapper()=="bool") {
                show_item.template operator()<bool>();
            } else if ((*pp)->typeNameWrapper()=="short") {
                show_item.template operator()<short>();
            } else if ((*pp)->typeNameWrapper()=="ushort") {
                show_item.template operator()<unsigned short>();
            } else if ((*pp)->typeNameWrapper()=="char") {
                show_item.template operator()<char>();
            } else if ((*pp)->typeNameWrapper()=="uchar") {
                show_item.template operator()<unsigned char>();
            } else if ((*pp)->typeNameWrapper()=="long") {
                show_item.template operator()<long>();
            } else if ((*pp)->typeNameWrapper()=="ulong") {
                show_item.template operator()<unsigned long>();
            } else if ((*pp)->typeNameWrapper()=="vec3d") {
                show_item.template operator()<OVM::Vec3d>();
            } else if ((*pp)->typeNameWrapper()=="vec3f") {
                show_item.template operator()<OVM::Vec3f>();
            } else if ((*pp)->typeNameWrapper()=="vec4d") {
                show_item.template operator()<OVM::Vec4d>();
            } else if ((*pp)->typeNameWrapper()=="vec4f") {
                show_item.template operator()<OVM::Vec4f>();
            }
            ImGui::PopID();
        }
    };

    // Better Picking. We might only want vertex picking in which
    // case we find the closest incident vertex to the actually
    // clicked entity
    if (_p.type_==3 && !_cfg.enable_cell_picking_) {
        float min_dist_sq = std::numeric_limits<float>::infinity();
        for (auto fh : mesh_.cell_faces(OVM::CH(_p.index_))) {
            float dist_sq = squared_distance(_p.position,mesh_.barycenter(fh));
            if (dist_sq < min_dist_sq) {
                _p.type_ = 2;
                _p.index_ = fh.idx();
                min_dist_sq = dist_sq;
            }
        }
    }
    if (_p.type_==2 && !_cfg.enable_face_picking_) {
        float min_dist_sq = std::numeric_limits<float>::infinity();
        for (auto eh : mesh_.face_edges(OVM::FH(_p.index_))) {
            float dist_sq = squared_distance(_p.position,mesh_.barycenter(eh));
            if (dist_sq < min_dist_sq) {
                _p.type_ = 1;
                _p.index_ = eh.idx();
                min_dist_sq = dist_sq;
            }
        }
    }
    if (_p.type_==1 && !_cfg.enable_edge_picking_) {
        OVM::HEH heh = OVM::EH(_p.index_).halfedge_handle(0);
        OVM::VH vh0 = mesh_.from_vertex_handle(heh);
        OVM::VH vh1 = mesh_.to_vertex_handle(heh);
        const auto& p0 = mesh_.vertex(vh0);
        const auto& p1 = mesh_.vertex(vh1);
        if (squared_distance(_p.position,p0) < squared_distance(_p.position,p1)) {
            _p.type_ = 0;
            _p.index_ = vh0.idx();
        } else {
            _p.type_ = 0;
            _p.index_ = vh1.idx();
        }
    }
    if (_p.type_==0 && !_cfg.enable_vertex_picking_) {
        return;
    }

    ImGui::SeparatorText(name().c_str());
    if (ImGui::BeginMenu("Settings")) {
        render_ui_settings();
        ImGui::EndMenu();
    }

    ImGui::Text("Position = (%f, %f, %f)",
        _p.position[0], _p.position[1], _p.position[2]);
    switch (_p.type_) {
    case 0:
        ImGui::Text("Vertex(%u)", _p.index_);
        show_prop_list.operator()<OVM::Entity::Vertex>();
        break;
    case 1:
        ImGui::Text("Edge(%u)", _p.index_);
        show_prop_list.operator()<OVM::Entity::Edge>();
        break;
    case 2:
        ImGui::Text("Face(%u)", _p.index_);
        show_prop_list.operator()<OVM::Entity::Face>();
        break;
    case 3:
        ImGui::Text("Cell(%u)", _p.index_);
        show_prop_list.operator()<OVM::Entity::Cell>();
        break;
    default: break;
    }
}

void OpenVolumeMeshObject::visualize_property(
    const std::string& _property_name,
    std::optional<OVM::EntityType> _entity_type,
    std::optional<std::string> _type_name)
{
    bool uploaded_property = false;

    auto select_property = [&]<typename EntityTag,typename T>(OVM::PropertyStorageBase* _pp) {

        prop<EntityTag>().prop_ = _pp;
        upload_buffer_property_data<T,EntityTag>(
            mesh_,
            _pp,
            prop<EntityTag>().filters_,
            renderer<EntityTag>()
            );
        prop<EntityTag>().filter_index_ = 0;
        selected_prop_entity_type_ = EntityTag::type();
        renderer<OVM::Entity::Vertex>().enabled() = false;
        renderer<OVM::Entity::Edge>().enabled() = false;
        renderer<OVM::Entity::Face>().enabled() = false;
        renderer<OVM::Entity::Cell>().enabled() = false;
        renderer<EntityTag>().enabled() = true;
        uploaded_property = true;
    };

    auto search_and_select_property = [&]<typename EntityTag>()
    {
        if (_entity_type.has_value() && _entity_type.value() != EntityTag::type()) {return;}
        for (auto pp = mesh_.persistent_props_begin<EntityTag>();
             pp != mesh_.persistent_props_end<EntityTag>(); ++pp)
        {
            if (uploaded_property || (_type_name.has_value()
                && (*pp)->typeNameWrapper() != _type_name.value())
                || _property_name != (*pp)->name())
            {continue;}

            if ((*pp)->typeNameWrapper()=="double") {
                select_property.template operator()<EntityTag,double>(*pp);
            } else if ((*pp)->typeNameWrapper()=="int") {
                select_property.template operator()<EntityTag,int>(*pp);
            } else if ((*pp)->typeNameWrapper()=="uint") {
                select_property.template operator()<EntityTag,unsigned int>(*pp);
            } else if ((*pp)->typeNameWrapper()=="float") {
                select_property.template operator()<EntityTag,float>(*pp);
            } else if ((*pp)->typeNameWrapper()=="bool") {
                select_property.template operator()<EntityTag,bool>(*pp);
            } else if ((*pp)->typeNameWrapper()=="short") {
                select_property.template operator()<EntityTag,short>(*pp);
            } else if ((*pp)->typeNameWrapper()=="ushort") {
                select_property.template operator()<EntityTag,unsigned short>(*pp);
            } else if ((*pp)->typeNameWrapper()=="char") {
                select_property.template operator()<EntityTag,char>(*pp);
            } else if ((*pp)->typeNameWrapper()=="uchar") {
                select_property.template operator()<EntityTag,unsigned char>(*pp);
            } else if ((*pp)->typeNameWrapper()=="long") {
                select_property.template operator()<EntityTag,long>(*pp);
            } else if ((*pp)->typeNameWrapper()=="ulong") {
                select_property.template operator()<EntityTag,unsigned long>(*pp);
            } else if ((*pp)->typeNameWrapper()=="vec3d") {
                select_property.template operator()<EntityTag,OVM::Vec3d>(*pp);
            } else if ((*pp)->typeNameWrapper()=="vec3f") {
                select_property.template operator()<EntityTag,OVM::Vec3f>(*pp);
            } else if ((*pp)->typeNameWrapper()=="vec4d") {
                select_property.template operator()<EntityTag,OVM::Vec4d>(*pp);
            } else if ((*pp)->typeNameWrapper()=="vec4f") {
                select_property.template operator()<EntityTag,OVM::Vec4f>(*pp);
            }
            if (uploaded_property) {
                break;
            }
        }
    };

    search_and_select_property.template operator()<OVM::Entity::Vertex>();
    search_and_select_property.template operator()<OVM::Entity::Edge>();
    search_and_select_property.template operator()<OVM::Entity::Face>();
    search_and_select_property.template operator()<OVM::Entity::Cell>();
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
        auto n = ToLoG::normalized(mesh_.normal(fh.halfface_handle(0)));
        D sphere_color{std::abs(n[0]),std::abs(n[1]),std::abs(n[2]),1};
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
