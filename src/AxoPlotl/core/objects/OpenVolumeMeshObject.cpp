#include "OpenVolumeMeshObject.hpp"
#include "AxoPlotl/gui/widgets/draw_basic_renderer_settings.hpp"
#include "AxoPlotl/gui/widgets/draw_toggle.hpp"
#include "AxoPlotl/properties/property_calculations.hpp"
#include "AxoPlotl/properties/property_data.hpp"
#include "AxoPlotl/rendering/MeshData.hpp"
#include "IconsFontAwesome7.h"
#include "imgui.h"
#include <AxoPlotl/core/Application.hpp>
#include <OpenVolumeMesh/IO/ovmb_write.hh>

namespace AxoPlotl
{
OpenVolumeMeshObject::OpenVolumeMeshObject(
    internal::Scene* _scene,
    VolMesh &_mesh,
    const std::optional<std::filesystem::path>& _filepath)
    :
    ObjectBase(_scene, ""),
    mesh_data_(std::move(_mesh)),
    filepath_(_filepath)
{
    this->name_ = _filepath.has_value()?
                      _filepath.value().stem().string() :
                      "New Volume Mesh Object " + std::to_string(id_);

    if (_filepath.has_value()) {
        // std::cerr << _filepath.value() << std::endl;
        auto filename_prop = _mesh.request_mesh_property<std::string>("AxoPlotl::filepath");
        _mesh.set_persistent(filename_prop);
        filename_prop[OVM::MeshHandle(0)] = *_filepath;
    }
}

void OpenVolumeMeshObject::draw_settings_ui()
{
    draw_name_input_ui();

    ImGui::PushID(0);
    gui::Toggle(vertex_renderer_.enabled());
    ImGui::SameLine();
    if (ImGui::BeginMenu("Vertices")) {
        gui::draw_basic_renderer_settings(vertex_renderer_);
        ImGui::EndMenu();
    }
    ImGui::PopID();

    ImGui::PushID(1);
    gui::Toggle(edge_renderer_.enabled());
    ImGui::SameLine();
    if (ImGui::BeginMenu("Edges")) {
        gui::draw_basic_renderer_settings(edge_renderer_);
        ImGui::EndMenu();
    }
    ImGui::PopID();

    ImGui::PushID(2);
    gui::Toggle(face_renderer_.enabled());
    ImGui::SameLine();
    if (ImGui::BeginMenu("Faces")) {
        gui::draw_basic_renderer_settings(face_renderer_);
        ImGui::EndMenu();
    }
    ImGui::PopID();

    ImGui::PushID(3);
    gui::Toggle(cell_renderer_.enabled());
    ImGui::SameLine();
    if (ImGui::BeginMenu("Cells")) {
        gui::draw_basic_renderer_settings(cell_renderer_);
        ImGui::EndMenu();
    }
    ImGui::PopID();

    // Clip Box
    // Each entity technically has their own, but we
    // just modify all at once.
    const auto& bbox = bounding_box();
    ClipBox& cb = vertex_renderer_.clip_box();
    gui::draw_clipbox_settings(cb, bbox);
    edge_renderer_.clip_box() = cb;
    face_renderer_.clip_box() = cb;
    cell_renderer_.clip_box() = cb;
    vector3_renderer_.clip_box() = cb;
    mat3x3_renderer_.clip_box() = cb;
}

void OpenVolumeMeshObject::draw_info_ui()
{
    ImGui::Text("V/E/F/C = %zu/%zu/%zu/%zu",
        mesh().n_vertices(), mesh().n_edges(),
        mesh().n_faces(), mesh().n_cells());
    ImGui::Text("BBox Min (%f, %f, %f)",
        bbox_.min()[0], bbox_.min()[1], bbox_.min()[2]);
    ImGui::Text("BBox Max (%f, %f, %f)",
        bbox_.max()[0], bbox_.max()[1], bbox_.max()[2]);
}

void OpenVolumeMeshObject::draw_properties_ui()
{
    auto render_property_selection_menu = [&]<typename EntityTag>(const std::string& _prefix)
    {
        ImGui::PushID(_prefix.c_str());

        std::string title = _prefix + ": ";
        if (prop<EntityTag>().prop_.has_value()) {title += (*prop<EntityTag>().prop_)->name();}
        else {title += "none";}

        // Select Property
        if (mesh().n_props<EntityTag>()>0 &&
            ImGui::BeginMenu(title.c_str()))
        {
            for (auto pp = mesh().persistent_props_begin<EntityTag>();
                 pp != mesh().persistent_props_end<EntityTag>(); ++pp)
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
        if (!selected_prop_info_.has_value()
            || selected_prop_info_->entity_type != EntityTag::type()
            || !prop<EntityTag>().prop_.has_value())
        {return;}

        const std::string title = selected_prop_info_->entity_type_name
        + "(" + selected_prop_info_->type_name + "): " + selected_prop_info_->name;
        ImGui::SeparatorText(title.c_str());

        ImGui::SameLine();

        // Clear
        if (ImGui::Button(ICON_FA_XMARK)) {
            entity_renderer<EntityTag>().property_filter_options().set_default();
            selected_prop_info_ = std::nullopt;
            prop<EntityTag>().prop_ = std::nullopt;
            scene_->app()->call_deferred([&]() {
                vector3_renderer_.cleanup();
                vector3_renderer_.enabled() = false;
                mat3x3_renderer_.cleanup();
                mat3x3_renderer_.enabled() = false;
            });
        }
        ImGui::SetItemTooltip("Unselect Property");

        // Render Property Filter Settings
        prop<EntityTag>().settings_->render_ui();
    };

    if (ImGui::BeginTabBar("##xx"))
    {
        if (ImGui::BeginTabItem(ICON_FA_EYE_DROPPER))
        {
            if (ImGui::BeginMenu("Select Property")) {
                render_property_selection_menu.operator()<OVM::Entity::Vertex>(" V");
                render_property_selection_menu.operator()<OVM::Entity::Edge>(" E");
                render_property_selection_menu.operator()<OVM::Entity::HalfEdge>(" HE");
                render_property_selection_menu.operator()<OVM::Entity::Face>(" F");
                render_property_selection_menu.operator()<OVM::Entity::HalfFace>(" HF");
                render_property_selection_menu.operator()<OVM::Entity::Cell>(" C");
                ImGui::EndMenu();
            }
            render_property_visualization_settings_menu.operator()<OVM::Entity::Vertex>();
            render_property_visualization_settings_menu.operator()<OVM::Entity::Edge>();
            render_property_visualization_settings_menu.operator()<OVM::Entity::HalfEdge>();
            render_property_visualization_settings_menu.operator()<OVM::Entity::Face>();
            render_property_visualization_settings_menu.operator()<OVM::Entity::HalfFace>();
            render_property_visualization_settings_menu.operator()<OVM::Entity::Cell>();
            ImGui::EndTabItem();
        }
        if (mesh().n_persistent_props<OVM::Entity::Mesh>() && ImGui::BeginTabItem(ICON_FA_GLOBE))
        {
            static constexpr OVM::MeshHandle mh(0);
            for (auto p  = mesh().persistent_props_begin<OVM::Entity::Mesh>();
                 p != mesh().persistent_props_end<OVM::Entity::Mesh>(); ++p)
            {
                // std::cerr << (*p)->name() << std::endl;
                if ((*p)->typeNameWrapper() == "double") {
                    auto prop = mesh().get_property<double,OVM::Entity::Mesh>((*p)->name()).value();
                    ImGui::Text("%s = %f", (*p)->name().c_str(), static_cast<float>(prop[mh]));
                } else if ((*p)->typeNameWrapper() == "float") {
                    auto prop = mesh().get_property<float,OVM::Entity::Mesh>((*p)->name()).value();
                    ImGui::Text("%s = %f", (*p)->name().c_str(), prop[mh]);
                } else if ((*p)->typeNameWrapper() == "string") {
                    auto prop = mesh().get_property<std::string,OVM::Entity::Mesh>((*p)->name()).value();
                    ImGui::Text("%s = %s", (*p)->name().c_str(), prop[mh].c_str());
                }
            }
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem(ICON_FA_CALCULATOR))
        {
            if (mesh().n_vertices()>0 && ImGui::BeginMenu("Vertices")){
                if (ImGui::MenuItem("Vertex Index")) {
                    add_entity_index_prop<OpenVolumeMesh::Entity::Vertex>(mesh());
                }
                if (ImGui::MenuItem("Vertex Valence")) {
                    add_vertex_valence_prop(mesh());
                }
                ImGui::EndMenu();
            }
            if (mesh().n_edges()>0 && ImGui::BeginMenu("Edges")){
                if (ImGui::MenuItem("Edge Index")) {
                    add_entity_index_prop<OpenVolumeMesh::Entity::Edge>(mesh());
                }
                if (ImGui::MenuItem("Edge Valence")) {
                    add_edge_valence_prop(mesh());
                }
                ImGui::EndMenu();
            }
            if (mesh().n_halfedges()>0 && ImGui::BeginMenu("Halfedges")){
                if (ImGui::MenuItem("Halfedge Index")) {
                    add_entity_index_prop<OpenVolumeMesh::Entity::HalfEdge>(mesh());
                }
                ImGui::EndMenu();
            }
            if (mesh().n_faces()>0 && ImGui::BeginMenu("Faces")){
                if (ImGui::MenuItem("Face Index")) {
                    add_entity_index_prop<OpenVolumeMesh::Entity::Face>(mesh());
                }
                ImGui::EndMenu();
            }
            if (mesh().n_halffaces()>0 && ImGui::BeginMenu("Halffaces")){
                if (ImGui::MenuItem("Halfface Index")) {
                    add_entity_index_prop<OpenVolumeMesh::Entity::HalfFace>(mesh());
                }
                if (ImGui::MenuItem("Halfface Boundary")) {
                    auto p = mesh().request_halfface_property<bool>("AxoPlotl::Boundary");
                    mesh().set_persistent(p);
                    for (OVM::HFH hfh : mesh().halffaces()) {
                        p[hfh] = mesh().is_boundary(hfh);
                    }
                }
                ImGui::EndMenu();
            }
            if (mesh().n_cells()>0 && ImGui::BeginMenu("Cells")) {
                if (ImGui::MenuItem("Cell Index")) {
                    add_entity_index_prop<OpenVolumeMesh::Entity::Cell>(mesh());
                }
                ImGui::EndMenu();
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}

void OpenVolumeMeshObject::init()
{
    const auto& gpu_context = scene()->app()->gpu_context();
    const auto& render_context = scene()->app()->render_context();
    const wgpu::Device& device = gpu_context.device;

    mesh_data_.create_buffers(device);

    vertex_renderer_.init(id(), gpu_context, render_context,
        mesh_data_.vertices_positions);
    edge_renderer_.init(id(), gpu_context, render_context,
        mesh_data_.vertices_positions,
        mesh_data_.halfedges_instances);
    face_renderer_.init(id(), gpu_context, render_context,
        mesh_data_.vertices_positions,
        mesh_data_.halffaces_indices,
        mesh_data_.halffaces_normals);
    cell_renderer_.init(id(), gpu_context, render_context,
        mesh_data_.vertices_positions,
        mesh_data_.cells_indices,
        mesh_data_.cells_centers,
        mesh_data_.halffaces_normals);

    vertex_renderer_.enabled() = false;
    edge_renderer_.enabled() = false;
    face_renderer_.enabled() = false;
    cell_renderer_.enabled() = false;
    if (mesh().n_cells() > 0) {
        cell_renderer_.enabled() = true;
    } else if (mesh().n_faces() > 0) {
        face_renderer_.enabled() = true;
        edge_renderer_.enabled() = true;
    } else if (mesh().n_edges() > 0) {
        edge_renderer_.enabled() = true;
        vertex_renderer_.enabled() = true;
    } else if (mesh().n_vertices() > 0) {
        vertex_renderer_.enabled() = true;
    }
}

void OpenVolumeMeshObject::cleanup()
{
    vertex_renderer_.cleanup();
    edge_renderer_.cleanup();
    face_renderer_.cleanup();
    cell_renderer_.cleanup();
    vector3_renderer_.cleanup();
    mat3x3_renderer_.cleanup();
}

void OpenVolumeMeshObject::render(
    wgpu::RenderPassEncoder _render_pass, const ViewSize &_view_size,
    const Mat4x4& _view,
    const Mat4x4& _proj)
{
    if (deleted() || !visible()) {return;}

    const auto& modelview = _view * transform_;
    const auto& mvp = MVP{
        .model = transform_,
        .view = _view,
        .proj = _proj
    };
    const auto& mvp_mat = _proj * modelview;

    cell_renderer_.directional_light() = scene()->directional_light;
    face_renderer_.directional_light() = scene()->directional_light;
    edge_renderer_.directional_light() = scene()->directional_light;
    vertex_renderer_.directional_light() = scene()->directional_light;

    const auto& selected_entity = scene()->app()->picked_pixel().entity;
    cell_renderer_.render(_view_size, _render_pass, mvp_mat, selected_entity);
    face_renderer_.render(_view_size, _render_pass, mvp_mat, selected_entity);
    edge_renderer_.render(_view_size, _render_pass, mvp, selected_entity);
    vertex_renderer_.render(_view_size, _render_pass, mvp, selected_entity);

    vector3_renderer_.enabled() = vertex_renderer_.property_filter_options().value_mode == PropValueMode::VEC3
        || edge_renderer_.property_filter_options().value_mode == PropValueMode::VEC3
        || face_renderer_.property_filter_options().value_mode == PropValueMode::VEC3
        || cell_renderer_.property_filter_options().value_mode == PropValueMode::VEC3;

    vector3_renderer_.render(_view_size, _render_pass, mvp_mat);

    mat3x3_renderer_.render(_view_size, _render_pass, mvp_mat);

    // cell_translucent_renderer_.render(
    //     scene_->app()->scene_viewport(),
    //     _render_pass,
    //     mvp);
}

void OpenVolumeMeshObject::draw_picking_ui(const Picker &_p)
{
    if (_p.pixel().entity.object_id != id()) [[unlikely]] {
        return;
    } // error

    auto squared_distance = [](const Vec3 _p0, const OVM::Vec3f& _p1) -> float {
        float dx = _p0[0] - _p1[0];
        float dy = _p0[1] - _p1[1];
        float dz = _p0[2] - _p1[2];
        return dx*dx + dy*dy + dz*dz;
    };

    auto show_prop_list = [&]<typename EntityTag>(const OVM::handle_for_tag_t<EntityTag> _handle) {
        for (auto pp = mesh().persistent_props_begin<EntityTag>();
             pp != mesh().persistent_props_end<EntityTag>(); ++pp)
        {
            auto show_item = [&]<typename T>() {
                auto prop = mesh().get_property<T,EntityTag>((*pp)->name()).value();
                if (_handle.is_valid() && _handle.uidx() < mesh().n<EntityTag>()) {
                    const T& val = prop.at(_handle);
                    ImGui::Text("%s: %s",
                        (*pp)->name().c_str(),
                        value_to_string(val).c_str()
                    );
                }
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
            } else if ((*pp)->typeNameWrapper()=="3x3d") {
                show_item.template operator()<glm::mat<3,3,double>>();
            }
            ImGui::PopID();
        }
    };

    // Better Picking. We might only want vertex picking in which
    // case we find the closest incident vertex to the actually
    // clicked entity (TODO)

    ImGui::SeparatorText(name().c_str());

    ImGui::Text("Clicked at (%f, %f, %f)",
        _p.pixel().position[0],
        _p.pixel().position[1],
        _p.pixel().position[2]);

    switch (_p.pixel().entity.entity_type)
    {
    case Picker::EntityType::VERTEX:
    {
        const OVM::VH vh(_p.pixel().entity.entity_index);
        ImGui::Text("Vertex(%u)", vh.uidx());
        show_prop_list.operator()<OVM::Entity::Vertex>(vh);
        break;
    }
    case Picker::EntityType::HALFEDGE:
    {
        if (ImGui::BeginTabBar("##xx")) {
            const OVM::HEH heh(_p.pixel().entity.entity_index);
            if (ImGui::BeginTabItem("Edge")) {
                const OVM::EH eh = heh.edge_handle();
                ImGui::Text("Edge(%u)", eh.uidx());
                show_prop_list.operator()<OVM::Entity::Edge>(eh);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Halfedge")) {
                ImGui::Text("Halfedge(%u)", heh.uidx());
                show_prop_list.operator()<OVM::Entity::HalfEdge>(heh);
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        break;
    }
    case Picker::EntityType::HALFFACE:
    {
        if (ImGui::BeginTabBar("##xx")) {
            const OVM::HFH hfh(_p.pixel().entity.entity_index);
            if (ImGui::BeginTabItem("Face")) {
                const OVM::FH fh = hfh.face_handle();
                ImGui::Text("Face(%u)", fh.uidx());
                show_prop_list.operator()<OVM::Entity::Face>(fh);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Halfface")) {
                ImGui::Text("Halfface(%u)", hfh.uidx());
                show_prop_list.operator()<OVM::Entity::HalfFace>(hfh);
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        break;
    }
    case Picker::EntityType::CELL:
    {
        const OVM::CH ch(_p.pixel().entity.entity_index);
        ImGui::Text("Cell(%u)", ch.uidx());
        show_prop_list.operator()<OVM::Entity::Cell>(ch);
        break;
    }
    default: break;
    }

    // Zoom to the picked entity
    if (ImGui::Button(ICON_FA_MAGNIFYING_GLASS)) {
        const float s = std::max<float>(bbox_.diagonal()*0.01f, 0.01f);
        BoundingBox bbox;
        bbox.expand_with_point(_p.pixel().position-Vec3(s,s,s));
        bbox.expand_with_point(_p.pixel().position+Vec3(s,s,s));
        scene_->zoom_to_box(bbox);
    }
}

bool OpenVolumeMeshObject::visualize_property(
    const std::string& _property_name,
    std::optional<OVM::EntityType> _entity_type,
    std::optional<std::string> _type_name)
{
    bool uploaded_property = false;

    auto select_property = [&]<typename EntityTag,typename T>(OVM::PropertyStorageBase* _pp)
    {
        auto& col_rend = entity_renderer<EntityTag>();

        prop<EntityTag>().prop_ = _pp;

        // If we want to visualize a vec3/mat3 property,
        // intitialize the corresponding buffers here
        // and delete them again after unselecting.
        // This is to not waste a ton of time/memory
        // for the vector renderers if we never actually
        // visualize a vec3 property. Also, we use
        // a single vector renderer for all entities.
        if constexpr(is_vector<T>) {
            if constexpr(vector_dim<T> == 3) {
                vector3_renderer_.init(id(),
                   scene_->app()->gpu_context(),
                   scene()->app()->render_context(),
                   entity_center_buffer<EntityTag>());
            }
        }
        if constexpr(is_matrix<T>) {
            if constexpr(has_rows_and_cols<T,3,3>) {
                mat3x3_renderer_.init(id(),
                  scene_->app()->gpu_context(),
                  scene()->app()->render_context(),
                  entity_center_buffer<EntityTag>());
            }
        }

        // Setup Property Filters
        if constexpr(std::is_same_v<bool,T>) {
            prop<EntityTag>().settings_ = std::make_shared<internal::PropertySettingsBool>(
                _pp->cast_to_StorageT<bool>(), &col_rend);
        } else if constexpr(std::is_floating_point_v<T>) {
            prop<EntityTag>().settings_ = std::make_shared<internal::PropertySettingsFloatRange<T>>(
                _pp->cast_to_StorageT<T>(), &col_rend);
        } else if constexpr(std::is_integral_v<T>) {
            prop<EntityTag>().settings_ = std::make_shared<internal::PropertySettingsInt<T>>(
                _pp->cast_to_StorageT<T>(), &col_rend);
        } else if constexpr(is_vector<T>) {
            if constexpr(vector_dim<T> == 3) {
                prop<EntityTag>().settings_ = std::make_shared<internal::PropertySettingsVec3>(
                    vector3_renderer_);
            }
        } else if constexpr(is_matrix<T>) {
            if constexpr(has_rows_and_cols<T,3,3>) {
                prop<EntityTag>().settings_ = std::make_shared<internal::PropertySettingsMat3x3>(
                    mat3x3_renderer_);
            }
        }

        // Upload Data to Color Renderer
        // (except if matrix...)
        if constexpr(!is_matrix<T>) {
            const auto& data = get_buffer_property_data_vec4<T,EntityTag>(
                mesh(), _pp);
            if constexpr(std::is_same_v<EntityTag,OVM::Entity::Face>) {
                col_rend.update_face_property_data(data);
            } else if constexpr(std::is_same_v<EntityTag,OVM::Entity::HalfFace>) {
                col_rend.update_halfface_property_data(data);
            } else if constexpr(std::is_same_v<EntityTag,OVM::Entity::Edge>) {
                col_rend.update_edge_property_data(data);
            } else if constexpr(std::is_same_v<EntityTag,OVM::Entity::HalfEdge>) {
                col_rend.update_halfedge_property_data(data);
            } else {
                col_rend.update_property_data(data);
            }
            col_rend.property_filter_options().value_mode = get_buffer_property_type<T>();
        }

        // Upload Data to Vector Renderer
        if constexpr(is_vector<T>) {
            if constexpr(vector_dim<T> == 3) {
                const auto& data = get_buffer_property_data_vec4<T,EntityTag>(
                    mesh(), _pp);
                vector3_renderer_.update_vector_data(data);
            }
        }

        // Upload Data to Matrix Renderer
        if constexpr(is_matrix<T>) {
            if constexpr(has_rows_and_cols<T,3,3>) {
                const auto& data = get_buffer_property_data_mat3x4<T,EntityTag>(mesh(), _pp);
                mat3x3_renderer_.update_matrix_data(data);
            }
        }

        // Visibility Settings
        selected_prop_info_ = SelectedPropInfo{
            .entity_type = EntityTag::type(),
            .name = _pp->name(),
            .entity_type_name = OVM::entityTypeName<EntityTag>(),
            .type_name = _pp->typeNameWrapper()
        };

        entity_renderer<OVM::Entity::Vertex>().enabled() = false;
        entity_renderer<OVM::Entity::Edge>().enabled() = false;
        entity_renderer<OVM::Entity::Face>().enabled() = false;
        entity_renderer<OVM::Entity::Cell>().enabled() = false;
        col_rend.enabled() = true;
        uploaded_property = true;
    };

    auto search_and_select_property = [&]<typename EntityTag>()
    {
        if (_entity_type.has_value() && _entity_type.value() != EntityTag::type()) {return;}
        for (auto pp = mesh().persistent_props_begin<EntityTag>();
             pp != mesh().persistent_props_end<EntityTag>(); ++pp)
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
            } else if ((*pp)->typeNameWrapper()=="3x3d") {
                select_property.template operator()<EntityTag,glm::mat<3,3,double>>(*pp);
            }
            if (uploaded_property) {
                break;
            }
        }
    };

    search_and_select_property.template operator()<OVM::Entity::Vertex>();
    search_and_select_property.template operator()<OVM::Entity::Edge>();
    search_and_select_property.template operator()<OVM::Entity::HalfEdge>();
    search_and_select_property.template operator()<OVM::Entity::Face>();
    search_and_select_property.template operator()<OVM::Entity::HalfFace>();
    search_and_select_property.template operator()<OVM::Entity::Cell>();

    return uploaded_property;
}

void OpenVolumeMeshObject::visualize_like(const OpenVolumeMeshObject& _obj)
{
    for (const auto& prop : _obj.visualized_props()) {
        visualize_property(
            prop->name(),
            prop->entity_type(),
            prop->typeNameWrapper());
    }
    vertex_renderer_.configure_like(_obj.vertex_renderer_);
    edge_renderer_.configure_like(_obj.edge_renderer_);
    face_renderer_.configure_like(_obj.face_renderer_);
    cell_renderer_.configure_like(_obj.cell_renderer_);
    vector3_renderer_.configure_like(_obj.vector3_renderer_);
    mat3x3_renderer_.configure_like(_obj.mat3x3_renderer_);
}

std::vector<OVM::PropertyStorageBase*> OpenVolumeMeshObject::visualized_props() const
{
    std::vector<OVM::PropertyStorageBase*> props;
    if (v_prop_.prop_.has_value()) {props.push_back(*v_prop_.prop_);}
    if (e_prop_.prop_.has_value()) {props.push_back(*e_prop_.prop_);}
    if (f_prop_.prop_.has_value()) {props.push_back(*f_prop_.prop_);}
    if (c_prop_.prop_.has_value()) {props.push_back(*c_prop_.prop_);}
    return props;
}

// void OpenVolumeMeshObject::upload_default_vertex_property_data()
// {
//     using D = PropContainer;
//     std::vector<D> props;
//     props.reserve(mesh().n_vertices());
//     for (uint32_t i = 0; i < mesh().n_vertices(); ++i) {
//         props.push_back(D(1,1,1,1));
//     }
//     vertex_renderer_.update_property_data(props);
// }
// void OpenVolumeMeshObject::upload_default_edge_property_data()
// {
//     using D = PropContainer;
//     std::vector<D> props;
//     props.reserve(mesh().n_edges());
//     for (uint32_t i = 0; i < mesh().n_edges(); ++i) {
//         props.push_back(D(1,1,1,1));
//     }
//     edge_renderer_.update_property_data(props);
// }
// void OpenVolumeMeshObject::upload_default_face_property_data()
// {
//     using D = PropContainer;
//     std::vector<D> props;
//     props.reserve(mesh().n_faces());
//     for (OVM::FH fh : mesh().faces()) {
//         props.emplace_back(1,1,1,1);
//     }
//     face_renderer_.update_property_data(props);
// }
// void OpenVolumeMeshObject::upload_default_cell_property_data()
// {
//     using D = PropContainer;
//     std::vector<D> props;
//     props.reserve(mesh().n_cells());
//     for (OVM::CH ch : mesh().cells()) {
//         props.emplace_back(1,1,1,1);
//     }
//     cell_renderer_.update_property_data(props);
// }

void OpenVolumeMeshObject::recompute_bounding_box()
{
    bbox_.reset();
    for (const auto& p : mesh().vertex_positions()) {
        bbox_.expand_with_point(p);
    }
}

}
