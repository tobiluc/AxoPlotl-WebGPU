#include "OpenVolumeMeshObject.hpp"
#include "AxoPlotl/properties/property_calculations.hpp"
#include "AxoPlotl/properties/property_data.hpp"
#include "AxoPlotl/rendering/detail/create_static_render_data.hpp"
#include "IconsFontAwesome7.h"
#include "imgui.h"
#include <AxoPlotl/Application.hpp>

namespace AxoPlotl
{

void OpenVolumeMeshObject::render_ui_settings()
{
    input_name();

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

    ImGui::ColorEdit3("Vertex Ambient", &vertex_renderer_.ambient()[0]);
    ImGui::ColorEdit3("Edge Ambient", &edge_renderer_.ambient()[0]);
    ImGui::ColorEdit3("Face Ambient", &face_renderer_.ambient()[0]);
    ImGui::ColorEdit3("Cell Ambient", &cell_renderer_.ambient()[0]);

    // Clip Box
    // Each entity technically has their own, but we
    // just modify all at once.
    const auto& bbox = bounding_box();
    RendererBase::ClipBox& cb = vertex_renderer_.clip_box();
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
    ImGui::Text("BBox Min (%f, %f, %f)",
        bbox_.min()[0], bbox_.min()[1], bbox_.min()[2]);
    ImGui::Text("BBox Max (%f, %f, %f)",
        bbox_.max()[0], bbox_.max()[1], bbox_.max()[2]);
}

void OpenVolumeMeshObject::render_ui_properties()
{
    if (ImGui::BeginMenu("Mesh Properties")) {
        static constexpr OVM::MeshHandle mh(0);
        for (auto p  = mesh_.persistent_props_begin<OVM::Entity::Mesh>();
             p != mesh_.persistent_props_end<OVM::Entity::Mesh>(); ++p)
        {
            if ((*p)->typeNameWrapper() == "double") {
                auto prop = mesh_.get_property<double,OVM::Entity::Mesh>((*p)->name()).value();
                ImGui::Text("%s = %f", (*p)->name().c_str(), static_cast<float>(prop[mh]));
            } else if ((*p)->typeNameWrapper() == "float") {
                auto prop = mesh_.get_property<float,OVM::Entity::Mesh>((*p)->name()).value();
                ImGui::Text("%s = %f", (*p)->name().c_str(), prop[mh]);
            }
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Calculate Property"))
    {
        if (mesh_.n_vertices()>0 && ImGui::BeginMenu("Vertices")){
            if (ImGui::MenuItem("Normalized Position")) {
                calc_vertex_normalized_position(mesh_);
            }
            ImGui::EndMenu();
        }
        if (mesh_.n_cells()>0 && ImGui::BeginMenu("Cells")) {
            // if (ImGui::MenuItem("Minimum Dihedral Angle")) {
            //     calc_cell_min_dihedral_angle(mesh_);
            // }
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
                std::string s = (*pp)->name() + " [" + (*pp)->typeNameWrapper() + "]";
                if (ImGui::MenuItem(s.c_str())) {
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
                        prop<EntityTag>().filters_[i]->set_default_settings();
                        prop<EntityTag>().filter_index_ = i;
                    }
                }
                ImGui::EndMenu();
            }
            prop<EntityTag>().filters_[prop<EntityTag>().filter_index_]->render_ui();
        }

        // Clear
        if (ImGui::Button("Unselect Property")) {
            upload_default_property_data<EntityTag>();
            colored_entity_renderer<EntityTag>().property_type()
                = RendererBase::Property::Type::COLOR;
            prop<EntityTag>().prop_ = std::nullopt;
            prop<EntityTag>().filters_.clear();
            scene_->app()->call_deferred([&]() {
                vector3_renderer_.clear();
                vector3_renderer_.enabled() = false;
            });
        }
    };

    if (ImGui::BeginMenu("Select Property")) {
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

void OpenVolumeMeshObject::init_buffers()
{
    const auto& data = create_static_render_data(mesh_);
    n_positions_ = data.positions_.size();

    vertices_position_buffer_ = create_position_buffer(scene_->app()->device_, data.positions_);
    edges_center_buffer_ = create_position_buffer(scene_->app()->device_, data.edges_barycenters_);
    faces_center_buffer_ = create_position_buffer(scene_->app()->device_, data.faces_barycenters_);
    cells_center_buffer_ = create_position_buffer(scene_->app()->device_, data.cells_barycenters_);

    vertex_renderer_.init(id(), scene_->app(), vertices_position_buffer_, data.vertices_);
    edge_renderer_.init(id(), scene_->app(), vertices_position_buffer_, data.edges_);
    face_renderer_.init(id(), scene_->app(), vertices_position_buffer_, data.faces_);
    cell_renderer_.init(id(), scene_->app(), vertices_position_buffer_, data.cells_, cells_center_buffer_);

    // vectors_on_vertices_renderer_.init(id(), scene_->app(), vertices_position_buffer_);
    // vectors_on_edges_renderer_.init(id(), scene_->app(), edges_center_buffer_);
    // vectors_on_faces_renderer_.init(id(), scene_->app(), faces_center_buffer_);
    // vectors_on_cells_renderer_.init(id(), scene_->app(), cells_center_buffer_);

    upload_default_property_data<OVM::Entity::Vertex>();
    upload_default_property_data<OVM::Entity::Edge>();
    upload_default_property_data<OVM::Entity::Face>();
    upload_default_property_data<OVM::Entity::Cell>();
}

void OpenVolumeMeshObject::delete_buffers()
{
    destroy_buffer(vertices_position_buffer_);
    destroy_buffer(edges_center_buffer_);
    destroy_buffer(faces_center_buffer_);
    destroy_buffer(cells_center_buffer_);
    vertex_renderer_.clear();
    edge_renderer_.clear();
    face_renderer_.clear();
    cell_renderer_.clear();
    vector3_renderer_.clear();
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

    vector3_renderer_.enabled() = vertex_renderer_.property_type() == RendererBase::Property::Type::VEC3
    || edge_renderer_.property_type() == RendererBase::Property::Type::VEC3
    || face_renderer_.property_type() == RendererBase::Property::Type::VEC3
    || cell_renderer_.property_type() == RendererBase::Property::Type::VEC3;

    vector3_renderer_.render(vp, _render_pass, mvp);

    // cell_translucent_renderer_.render(
    //     scene_->app()->scene_viewport(),
    //     _render_pass,
    //     mvp);
}

void OpenVolumeMeshObject::render_ui_picking(PickResult _p, const PickConfig &_cfg)
{
    if (_p.object_id_ != id()) [[unlikely]] {return;} // error

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
                const T& val = prop[OVM::handle_for_tag_t<EntityTag>(_p.entity_index_)];
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
    // clicked entity (TODO)

    ImGui::SeparatorText(name().c_str());
    if (ImGui::BeginMenu("Settings")) {
        render_ui_settings();
        ImGui::EndMenu();
    }

    ImGui::Text("Position = (%f, %f, %f)",
        _p.position_[0], _p.position_[1], _p.position_[2]);
    switch (_p.entity_type_)
    {
    case 0:
        ImGui::Text("Vertex(%u)", _p.entity_index_);
        show_prop_list.operator()<OVM::Entity::Vertex>();
        break;
    case 1:
        ImGui::Text("Edge(%u)", _p.entity_index_);
        show_prop_list.operator()<OVM::Entity::Edge>();
        break;
    case 2:
        ImGui::Text("Face(%u)", _p.entity_index_);
        show_prop_list.operator()<OVM::Entity::Face>();
        break;
    case 3:
        ImGui::Text("Cell(%u)", _p.entity_index_);
        show_prop_list.operator()<OVM::Entity::Cell>();
        break;
    default: break;
    }

    // Zoom to the picked entity
    if (ImGui::Button(ICON_FA_MAGNIFYING_GLASS)) {
        const float s = std::max<float>(bbox_.diagonal()*0.01f, 0.01f);
        BoundingBox bbox;
        bbox.expand_with_point(_p.position_-Vec3f(s,s,s));
        bbox.expand_with_point(_p.position_+Vec3f(s,s,s));
        scene_->zoom_to_box(bbox);
    }
}

void OpenVolumeMeshObject::visualize_property(
    const std::string& _property_name,
    std::optional<OVM::EntityType> _entity_type,
    std::optional<std::string> _type_name)
{
    bool uploaded_property = false;

    auto select_property = [&]<typename EntityTag,typename T>(OVM::PropertyStorageBase* _pp)
    {
        auto& col_rend = colored_entity_renderer<EntityTag>();

        prop<EntityTag>().prop_ = _pp;

        // If we want to visualize a vec3 property,
        // intitialize the corresponding buffers here
        // and delete them again after unselecting.
        // This is to not waste a ton of time/memory
        // for the vector renderers if we never actually
        // visualize a vec3 property. Also, we use
        // a single vector renderer for all entities.
        if constexpr(is_vector<T>) {
            if constexpr(vector_dim<T> == 3) {
                vector3_renderer_.init(id(), scene_->app(), entity_center_buffer<EntityTag>());
            }
        }

        // Setup Property Filters
        prop<EntityTag>().filters_.clear();
        if constexpr(std::is_same_v<bool,T>) {
            prop<EntityTag>().filters_.push_back(std::make_shared<PropertyFilterBool<EntityTag>>(
                _pp->cast_to_StorageT<bool>(), col_rend));
        } else if constexpr(std::is_floating_point_v<T>) {
            prop<EntityTag>().filters_.push_back(std::make_shared<PropertyFilterFloatRange<T,EntityTag>>(
                _pp->cast_to_StorageT<T>(), col_rend));
        } else if constexpr(std::is_integral_v<T>) {
            prop<EntityTag>().filters_.push_back(std::make_shared<PropertyFilterIntValue<T,EntityTag>>(
                _pp->cast_to_StorageT<T>(), col_rend));
            prop<EntityTag>().filters_.push_back(std::make_shared<PropertyFilterIntRange<T,EntityTag>>(
                _pp->cast_to_StorageT<T>(), col_rend));
        } else if constexpr(is_vector<T>) {
            if constexpr(vector_dim<T> == 3) {
                prop<EntityTag>().filters_.push_back(std::make_shared<PropertyFilterVec3<T,EntityTag>>(
                    vector3_renderer_));
            }
        }

        // Upload Data to Color Renderer
        const auto& data = get_buffer_property_data<T,EntityTag>(
            mesh_, _pp);
        col_rend.update_property_data(data);
        col_rend.property_type() = get_buffer_property_type<T>();

        // Upload Data to Vector Renderer
        if constexpr(is_vector<T>) {
            if constexpr(vector_dim<T> == 3) {
                const auto& data = get_buffer_property_data<T,EntityTag>(
                    mesh_, _pp);
                vector3_renderer_.update_vector_data(data);
            }
        }

        // Visibility Settings
        prop<EntityTag>().filter_index_ = 0;
        selected_prop_entity_type_ = EntityTag::type();
        colored_entity_renderer<OVM::Entity::Vertex>().enabled() = false;
        colored_entity_renderer<OVM::Entity::Edge>().enabled() = false;
        colored_entity_renderer<OVM::Entity::Face>().enabled() = false;
        colored_entity_renderer<OVM::Entity::Cell>().enabled() = false;
        col_rend.enabled() = true;
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
    using D = RendererBase::Property::Data;
    std::vector<D> props;
    props.reserve(mesh_.n_vertices());
    for (uint32_t i = 0; i < mesh_.n_vertices(); ++i) {
        props.push_back(D(1,1,1,1));
    }
    vertex_renderer_.update_property_data(props);
}
void OpenVolumeMeshObject::upload_default_edge_property_data()
{
    using D = RendererBase::Property::Data;
    std::vector<D> props;
    props.reserve(mesh_.n_edges());
    for (uint32_t i = 0; i < mesh_.n_edges(); ++i) {
        props.push_back(D(1,1,1,1));
    }
    edge_renderer_.update_property_data(props);
}
void OpenVolumeMeshObject::upload_default_face_property_data()
{
    using D = RendererBase::Property::Data;
    std::vector<D> props;
    props.reserve(mesh_.n_faces());
    for (OVM::FH fh : mesh_.faces()) {
        auto n = mesh_.normal(fh.halfface_handle(0));
        D sphere_color{std::abs(n[0]),std::abs(n[1]),std::abs(n[2]),1};
        props.push_back(sphere_color);
    }
    face_renderer_.update_property_data(props);
}
void OpenVolumeMeshObject::upload_default_cell_property_data()
{
    using D = RendererBase::Property::Data;
    std::vector<D> props;
    props.reserve(mesh_.n_cells());
    for (OVM::CH ch : mesh_.cells()) {
        auto p = mesh_.barycenter(ch).normalized();
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
    bbox_.reset();
    for (const auto& p : mesh_.vertex_positions()) {
        bbox_.expand_with_point(p);
    }
}

}
