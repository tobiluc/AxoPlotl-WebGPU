#pragma once

#include "AxoPlotl/gui/properties/property_settings.hpp"
#include <AxoPlotl/rendering/renderers/renderers.hpp>
#include "AxoPlotl/core/objects/BaseObject.hpp"
#include "AxoPlotl/typedefs/ovm.hpp"
#include <filesystem>

namespace AxoPlotl
{

class OpenVolumeMeshObject : public ObjectBase
{
protected:
    OVMVolumeMesh mesh_;
    std::optional<std::filesystem::path> filepath_;

    //template<typename EntityTag> using filter_t = PropertyFilterForRenderer<colored_entity_renderer_t<EntityTag>>;

    template<typename EntityTag>
    struct Property {
        std::optional<OVM::PropertyStorageBase*> prop_;
        std::vector<std::shared_ptr<internal::PropertySettingsBase>> settings_;
        int filter_index_ = 0;
    };

    Property<OVM::Entity::Vertex> v_prop_;
    Property<OVM::Entity::Edge> e_prop_;
    Property<OVM::Entity::Face> f_prop_;
    Property<OVM::Entity::Cell> c_prop_;
    std::optional<OVM::EntityType> selected_prop_entity_type_ = std::nullopt;

    template<typename EntityTag> Property<EntityTag>& prop();
    template<> Property<OVM::Entity::Vertex>& prop<OVM::Entity::Vertex>() {return v_prop_;}
    template<> Property<OVM::Entity::Edge>& prop<OVM::Entity::Edge>() {return e_prop_;}
    template<> Property<OVM::Entity::Face>& prop<OVM::Entity::Face>() {return f_prop_;}
    template<> Property<OVM::Entity::Cell>& prop<OVM::Entity::Cell>() {return c_prop_;}

    std::vector<OVM::PropertyStorageBase*> visualized_props() const;

    void upload_default_vertex_property_data();
    void upload_default_edge_property_data();
    void upload_default_face_property_data();
    void upload_default_cell_property_data();

    template<typename EntityTag> void upload_default_property_data();
    template<> void upload_default_property_data<OVM::Entity::Vertex>() {
        upload_default_vertex_property_data();}
    template<> void upload_default_property_data<OVM::Entity::Edge>() {
        upload_default_edge_property_data();}
    template<> void upload_default_property_data<OVM::Entity::Face>() {
        upload_default_face_property_data();}
    template<> void upload_default_property_data<OVM::Entity::Cell>() {
        upload_default_cell_property_data();}

public:
    OpenVolumeMeshObject(internal::Scene* _scene, OVMVolumeMesh& _mesh,
                         const std::optional<std::filesystem::path>& _filepath = std::nullopt);

    ~OpenVolumeMeshObject() {
        delete_buffers();
    }

    void render(
        wgpu::RenderPassEncoder _render_pass,
        const ViewSize& _view_size,
        const Mat4x4f& _view_projection) override;

    void render_ui_info() override;

    void render_ui_settings() override;

    void render_ui_properties() override;

    void render_ui_picking(const Picker& _p) override;

    void init_buffers() override;

    void delete_buffers() override;

    void recompute_bounding_box() override;

    bool export_file(const std::filesystem::path& _path) const override;

    bool visualize_property(
        const std::string& _property_name,
        std::optional<OVM::EntityType> _entity_type = std::nullopt,
        std::optional<std::string> _type_name = std::nullopt);

    void visualize_like(const OpenVolumeMeshObject& _obj);

    OVMVolumeMesh& mesh() {
        return mesh_;
    }

    std::optional<std::filesystem::path> filepath() const {
        return filepath_;
    }

protected:
    template<typename EntityTag>
    auto& colored_entity_renderer();
    template<> auto& colored_entity_renderer<OVM::Entity::Cell>() {return cell_renderer_;}
    template<> auto& colored_entity_renderer<OVM::Entity::Face>() {return face_renderer_;}
    template<> auto& colored_entity_renderer<OVM::Entity::Edge>() {return edge_renderer_;}
    template<> auto& colored_entity_renderer<OVM::Entity::Vertex>() {return vertex_renderer_;}

    template<typename EntityTag>
    wgpu::Buffer& entity_center_buffer();
    template<> wgpu::Buffer& entity_center_buffer<OVM::Entity::Cell>() {return cells_center_buffer_;}
    template<> wgpu::Buffer& entity_center_buffer<OVM::Entity::Face>() {return faces_center_buffer_;}
    template<> wgpu::Buffer& entity_center_buffer<OVM::Entity::Edge>() {return edges_center_buffer_;}
    template<> wgpu::Buffer& entity_center_buffer<OVM::Entity::Vertex>() {return vertices_position_buffer_;}

protected:
    size_t n_positions_;
    wgpu::Buffer vertices_position_buffer_;
    wgpu::Buffer edges_center_buffer_;
    wgpu::Buffer faces_center_buffer_;
    wgpu::Buffer cells_center_buffer_;
    ColoredVertexRenderer vertex_renderer_;
    ColoredEdgeRenderer edge_renderer_;
    ColoredFaceRenderer face_renderer_;
    ColoredCellRenderer cell_renderer_;

    Vector3Renderer vector3_renderer_;
    Matrix3x3Renderer mat3x3_renderer_;
};

}
