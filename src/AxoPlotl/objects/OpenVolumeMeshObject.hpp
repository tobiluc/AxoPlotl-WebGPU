#pragma once

#include "AxoPlotl/properties/property_filters.hpp"
#include "AxoPlotl/rendering/MeshCellRenderer.hpp"
#include "AxoPlotl/rendering/MeshEdgeRenderer.hpp"
#include "AxoPlotl/rendering/MeshFaceRenderer.hpp"
#include "AxoPlotl/rendering/MeshVertexRenderer.hpp"
#include "AxoPlotl/rendering/VectorRenderer.hpp"
#include "AxoPlotl/typedefs/ToLoG.hpp"
#include "AxoPlotl/objects/BaseObject.hpp"
#include "AxoPlotl/typedefs/ovm.hpp"
#include <filesystem>

namespace AxoPlotl
{

class OpenVolumeMeshObject : public ObjectBase
{
private:
    OVMVolumeMesh mesh_;
    std::optional<std::filesystem::path> filepath_;

private:
    template<typename EntityTag>
    struct renderer_for_property;
    template<> struct renderer_for_property<OVM::Entity::Vertex> {using type=ColoredVertexPropertyRenderer;};
    template<> struct renderer_for_property<OVM::Entity::Edge> {using type=ColoredEdgePropertyRenderer;};
    template<> struct renderer_for_property<OVM::Entity::Face> {using type=ColoredFacePropertyRenderer;};
    template<> struct renderer_for_property<OVM::Entity::Cell> {using type=ColoredCellPropertyRenderer;};
    template<typename EntityTag> using renderer_t = typename renderer_for_property<EntityTag>::type;
    template<typename EntityTag> using filter_t = PropertyFilterForRenderer<renderer_t<EntityTag>>;

    template<typename EntityTag>
    struct Property {
        std::optional<OVM::PropertyStorageBase*> prop_;
        std::vector<std::shared_ptr<filter_t<EntityTag>>> filters_;
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
    OpenVolumeMeshObject(Scene* _scene, OVMVolumeMesh& _mesh,
        const std::optional<std::filesystem::path>& _filepath = std::nullopt) :
        ObjectBase(_scene, ""),
        mesh_(std::move(_mesh)),
        filepath_(_filepath)
    {
        this->name_ = _filepath.has_value()?
            _filepath.value().stem().string() :
            "New Volume Mesh Object " + std::to_string(id_);
    }

    ~OpenVolumeMeshObject()
    {
        vertices_position_buffer_.destroy();
        vertices_position_buffer_.release();
        cells_center_buffer_.destroy();
        cells_center_buffer_.release();
    }

    void render(
        wgpu::RenderPassEncoder _render_pass,
        const Mat4x4f& _view_projection) override;

    void render_ui_info() override;

    void render_ui_settings() override;

    void render_ui_properties() override;

    void render_ui_picking(PickResult _p, const PickConfig& _cfg) override;

    void init_gpu_buffers() override;

    void recompute_bounding_box() override;

    void visualize_property(
        const std::string& _property_name,
        std::optional<OVM::EntityType> _entity_type = std::nullopt,
        std::optional<std::string> _type_name = std::nullopt);

    inline OVMVolumeMesh& mesh() {
        return mesh_;
    }

    template<typename EntityTag>
    inline auto& renderer();
    template<> inline auto& renderer<OVM::Entity::Cell>() {return cell_renderer_;}
    template<> inline auto& renderer<OVM::Entity::Face>() {return face_renderer_;}
    template<> inline auto& renderer<OVM::Entity::Edge>() {return edge_renderer_;}
    template<> inline auto& renderer<OVM::Entity::Vertex>() {return vertex_renderer_;}

private:
    size_t n_positions_;
    wgpu::Buffer vertices_position_buffer_;
    wgpu::Buffer cells_center_buffer_;
    ColoredVertexPropertyRenderer vertex_renderer_;
    ColoredEdgePropertyRenderer edge_renderer_;
    ColoredFacePropertyRenderer face_renderer_;
    ColoredCellPropertyRenderer cell_renderer_;
    //VectorRenderer vectors_on_vertices_renderer_;
};

}
