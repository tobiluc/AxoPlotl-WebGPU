#pragma once
#include <AxoPlotl/typedefs/ovm.hpp>
#include <AxoPlotl/properties/property_data.hpp>
#include <AxoPlotl/properties/property_calculations.hpp>
#include <AxoPlotl/gui/properties/PropertyRendererSettingsBase.hpp>
#include <AxoPlotl/gui/properties/property_settings.hpp>
#include "VolMeshRendererCollection.hpp"

namespace AxoPlotl::internal
{

class VolMeshPropertyVisualizer
{
private:
    template<typename EntityTag>
    struct Property {
        std::optional<OVM::PropertyStorageBase*> prop_;
        std::shared_ptr<internal::PropertySettingsBase> settings_;
    };

    Property<OVM::Entity::Vertex> v_prop_;
    Property<OVM::Entity::Edge> e_prop_;
    Property<OVM::Entity::HalfEdge> he_prop_;
    Property<OVM::Entity::Face> f_prop_;
    Property<OVM::Entity::HalfFace> hf_prop_;
    Property<OVM::Entity::Cell> c_prop_;

    struct PropInfo {
        std::string name = "";
        OVM::EntityType entity_type;
        std::string type_name = "";
        std::string entity_type_name = "";
    };
    std::optional<PropInfo> selected_prop_info_ = std::nullopt;

public:
    template<typename EntityTag> Property<EntityTag>& prop();
    template<> Property<OVM::Entity::Vertex>& prop<OVM::Entity::Vertex>() {return v_prop_;}
    template<> Property<OVM::Entity::Edge>& prop<OVM::Entity::Edge>() {return e_prop_;}
    template<> Property<OVM::Entity::HalfEdge>& prop<OVM::Entity::HalfEdge>() {return he_prop_;}
    template<> Property<OVM::Entity::Face>& prop<OVM::Entity::Face>() {return f_prop_;}
    template<> Property<OVM::Entity::HalfFace>& prop<OVM::Entity::HalfFace>() {return hf_prop_;}
    template<> Property<OVM::Entity::Cell>& prop<OVM::Entity::Cell>() {return c_prop_;}

    std::vector<OVM::PropertyStorageBase*> visualized_props() const
    {
        std::vector<OVM::PropertyStorageBase*> props;
        if (v_prop_.prop_.has_value()) {props.push_back(*v_prop_.prop_);}
        if (e_prop_.prop_.has_value()) {props.push_back(*e_prop_.prop_);}
        if (f_prop_.prop_.has_value()) {props.push_back(*f_prop_.prop_);}
        if (c_prop_.prop_.has_value()) {props.push_back(*c_prop_.prop_);}
        return props;
    }

    bool visualize_property(
        uint32_t _object_id,
        const VolMeshBuffers& _mesh_data,
        const internal::GpuContext& _gpu_context,
        const internal::RenderContext& _render_context,
        VolMeshRendererCollection& _renderers,
        const std::string& _property_name,
        std::optional<OVM::EntityType> _entity_type = std::nullopt,
        std::optional<std::string> _type_name = std::nullopt)
    {
        bool uploaded_property = false;

        auto select_property = [&]<typename EntityTag,typename T>(OVM::PropertyStorageBase* _pp)
        {
            auto& col_rend = _renderers.entity_renderer<EntityTag>();

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
                    _renderers.vector3_renderer().init(_object_id,
                    _gpu_context,
                   _render_context,
                    _mesh_data.entity_center_buffer<EntityTag>());
                }
            } else if constexpr(is_matrix<T>) {
                if constexpr(has_rows_and_cols<T,3,3>) {
                    _renderers.matrix3x3_renderer().init(_object_id,
                    _gpu_context,
                    _render_context,
                    _mesh_data.entity_center_buffer<EntityTag>());
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
                        _renderers.vector3_renderer());
                }
            } else if constexpr(is_matrix<T>) {
                if constexpr(has_rows_and_cols<T,3,3>) {
                    prop<EntityTag>().settings_ = std::make_shared<internal::PropertySettingsMat3x3>(
                        _renderers.matrix3x3_renderer());
                }
            }

            // Upload Data to Color Renderer
            // (except if matrix...)
            if constexpr(!is_matrix<T> && !is_vector<T>) {
                const auto& data = get_buffer_property_data<T,EntityTag>(
                    _mesh_data.mesh, _pp);
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
                    const auto& data = get_buffer_property_data_vec4<T,EntityTag>(_mesh_data.mesh, _pp);
                    _renderers.vector3_renderer().update_vector_data(data);
                }
            }

            // Upload Data to Matrix Renderer
            if constexpr(is_matrix<T>) {
                if constexpr(has_rows_and_cols<T,3,3>) {
                    const auto& data = get_buffer_property_data_mat3x4<T,EntityTag>(_mesh_data.mesh, _pp);
                    _renderers.matrix3x3_renderer().update_matrix_data(data);
                }
            }

            // Visibility Settings
            selected_prop_info_ = PropInfo{
                .name = _pp->name(),
                .entity_type = EntityTag::type(),
                .type_name = _pp->typeNameWrapper(),
                .entity_type_name = OVM::entityTypeName<EntityTag>(),
            };

            _renderers.entity_renderer<OVM::Entity::Vertex>().enabled() = false;
            _renderers.entity_renderer<OVM::Entity::Edge>().enabled() = false;
            _renderers.entity_renderer<OVM::Entity::Face>().enabled() = false;
            _renderers.entity_renderer<OVM::Entity::Cell>().enabled() = false;
            col_rend.enabled() = true;
            uploaded_property = true;
        };

        auto search_and_select_property = [&]<typename EntityTag>()
        {
            if (_entity_type.has_value() && _entity_type.value() != EntityTag::type()) {return;}
            for (auto pp = _mesh_data.mesh.persistent_props_begin<EntityTag>();
                pp != _mesh_data.mesh.persistent_props_end<EntityTag>(); ++pp)
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

    [[nodiscard]] std::optional<PropInfo> draw_property_selection_menu_ui(const VolMesh& _mesh)
    {
        auto render_property_selection_menu = [&]<typename EntityTag>(const VolMesh& _mesh, const std::string& _prefix) -> std::optional<PropInfo>
        {
            std::optional<PropInfo> res = std::nullopt;

            ImGui::PushID(_prefix.c_str());

            std::string title = _prefix + ": ";
            if (prop<EntityTag>().prop_.has_value()) {title += (*prop<EntityTag>().prop_)->name();}
            else {title += "none";}

            // Select Property
            if (_mesh.n_persistent_props<EntityTag>()>0 &&
                ImGui::BeginMenu(title.c_str()))
            {
                for (auto pp = _mesh.persistent_props_begin<EntityTag>();
                    pp != _mesh.persistent_props_end<EntityTag>(); ++pp)
                {
                    ImGui::PushID((*pp)->name().c_str());
                    std::string s = (*pp)->name() + " [" + (*pp)->typeNameWrapper() + "]";
                    if (ImGui::MenuItem(s.c_str())) {
                        res = PropInfo{
                            .name=(*pp)->name(),
                            .entity_type=EntityTag::type(),
                            .type_name=(*pp)->typeNameWrapper(),
                            .entity_type_name=OVM::entityTypeName<EntityTag>()
                        };
                    }
                    ImGui::PopID();
                }
                ImGui::EndMenu(); // Props
            }
            ImGui::PopID();
            return res;
        };
        
        std::optional<PropInfo> p = std::nullopt;
        p = render_property_selection_menu.template operator()<OVM::Entity::Vertex>(_mesh, " V");
        if (p) {return p;}
        p = render_property_selection_menu.template operator()<OVM::Entity::Edge>(_mesh, " E");
        if (p) {return p;}
        p = render_property_selection_menu.template operator()<OVM::Entity::HalfEdge>(_mesh, " HE");
        if (p) {return p;}
        p = render_property_selection_menu.template operator()<OVM::Entity::Face>(_mesh, " F");
        if (p) {return p;}
        p = render_property_selection_menu.template operator()<OVM::Entity::HalfFace>(_mesh, " HF");
        if (p) {return p;}
        p = render_property_selection_menu.template operator()<OVM::Entity::Cell>(_mesh, " C");
        return p;
    }

    void draw_property_visualization_settings_menu_ui(VolMeshRendererCollection& _renderers)
    {
        auto render_property_visualization_settings_menu = [&]<typename EntityTag>(VolMeshRendererCollection& _renderers)
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
                _renderers.entity_renderer<EntityTag>().property_filter_options().set_default();
                selected_prop_info_ = std::nullopt;
                prop<EntityTag>().prop_ = std::nullopt;
                _renderers.vector3_renderer().enabled() = false;
                _renderers.matrix3x3_renderer().enabled() = false;
                _renderers.vector3_renderer().cleanup();
                _renderers.matrix3x3_renderer().cleanup();
            }
            ImGui::SetItemTooltip("Unselect Property");

            // Render Property Filter Settings
            prop<EntityTag>().settings_->render_ui();
        };
        render_property_visualization_settings_menu.template operator()<OVM::Entity::Vertex>(_renderers);
        render_property_visualization_settings_menu.template operator()<OVM::Entity::Edge>(_renderers);
        render_property_visualization_settings_menu.template operator()<OVM::Entity::HalfEdge>(_renderers);
        render_property_visualization_settings_menu.template operator()<OVM::Entity::Face>(_renderers);
        render_property_visualization_settings_menu.template operator()<OVM::Entity::HalfFace>(_renderers);
        render_property_visualization_settings_menu.template operator()<OVM::Entity::Cell>(_renderers);
    }

    void draw_mesh_properties_ui(const VolMesh& _mesh)
    {
        static constexpr OVM::MeshHandle mh(0);
        for (auto p  = _mesh.persistent_props_begin<OVM::Entity::Mesh>();
            p != _mesh.persistent_props_end<OVM::Entity::Mesh>(); ++p)
        {
            // std::cerr << (*p)->name() << std::endl;
            if ((*p)->typeNameWrapper() == "double") {
                auto prop = _mesh.get_property<double,OVM::Entity::Mesh>((*p)->name()).value();
                ImGui::Text("%s = %f", (*p)->name().c_str(), static_cast<float>(prop[mh]));
            } else if ((*p)->typeNameWrapper() == "float") {
                auto prop = _mesh.get_property<float,OVM::Entity::Mesh>((*p)->name()).value();
                ImGui::Text("%s = %f", (*p)->name().c_str(), prop[mh]);
            } else if ((*p)->typeNameWrapper() == "string") {
                auto prop = _mesh.get_property<std::string,OVM::Entity::Mesh>((*p)->name()).value();
                ImGui::Text("%s = %s", (*p)->name().c_str(), prop[mh].c_str());
            }
        }
    }

    void draw_picked_properties_ui(const VolMesh& _mesh, const Picker::Pixel& _p)
    {
        auto squared_distance = [](const Vec3 _p0, const OVM::Vec3f& _p1) -> float {
            float dx = _p0[0] - _p1[0];
            float dy = _p0[1] - _p1[1];
            float dz = _p0[2] - _p1[2];
            return dx*dx + dy*dy + dz*dz;
        };

        auto show_prop_list = [&]<typename EntityTag>(const OVM::handle_for_tag_t<EntityTag> _handle) {
            if (!_handle.is_valid()) {return;}
            for (auto pp = _mesh.persistent_props_begin<EntityTag>();
                pp != _mesh.persistent_props_end<EntityTag>(); ++pp)
            {
                auto show_item = [&]<typename T>() {
                    auto prop = _mesh.get_property<T,EntityTag>((*pp)->name()).value();
                    if (_handle.is_valid() && _handle.uidx() < _mesh.n<EntityTag>()) {
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

        // ImGui::SeparatorText(name().c_str());

        ImGui::Text("Clicked at (%f, %f, %f)",
            _p.position[0],
            _p.position[1],
            _p.position[2]);

        switch (_p.entity.entity_type)
        {
        case Picker::EntityType::VERTEX:
        {
            const OVM::VH vh(_p.entity.entity_index);
            ImGui::Text("Vertex(%u)", vh.uidx());
            show_prop_list.operator()<OVM::Entity::Vertex>(vh);
            break;
        }
        case Picker::EntityType::HALFEDGE:
        {
            if (ImGui::BeginTabBar("##xx")) {
                const OVM::HEH heh(_p.entity.entity_index);
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
                const OVM::HFH hfh(_p.entity.entity_index);
                const OVM::CH ch = _mesh.incident_cell(hfh.opposite_handle());
                if (ImGui::BeginTabItem("Face")) {
                    const OVM::FH fh = hfh.face_handle();
                    ImGui::Text("Face(%i)", fh.idx());
                    show_prop_list.operator()<OVM::Entity::Face>(fh);
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Halfface")) {
                    ImGui::Text("Halfface(%i)", hfh.idx());
                    show_prop_list.operator()<OVM::Entity::HalfFace>(hfh);
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Cell")) {
                    ImGui::Text("Cell(%i)", ch.idx());
                    show_prop_list.operator()<OVM::Entity::Cell>(ch);
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            break;
        }
        case Picker::EntityType::CELL:
        {
            const OVM::CH ch(_p.entity.entity_index);
            ImGui::Text("Cell(%u)", ch.uidx());
            show_prop_list.operator()<OVM::Entity::Cell>(ch);
            break;
        }
        default: break;
        }
    }

};

}