#pragma once

#include "AxoPlotl/properties/property_filters.hpp"
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
    OpenVolumeMeshRenderer renderer_;
    VectorRenderer vertex_vector_renderer_;
    //VectorRenderer cell_vector_renderer_;

    std::optional<std::filesystem::path> filepath_;

    struct Property {
        std::optional<OVM::PropertyStorageBase*> prop_;
        std::vector<std::shared_ptr<PropertyFilterBase>> filters_;
        int filter_index_ = 0;
    };

    Property v_prop_;
    Property e_prop_;
    Property f_prop_;
    Property c_prop_;
    std::optional<OVM::EntityType> selected_prop_entity_type_ = std::nullopt;

    template<typename EntityTag> Property& prop();
    template<> Property& prop<OVM::Entity::Vertex>() {return v_prop_;}
    template<> Property& prop<OVM::Entity::Edge>() {return e_prop_;}
    template<> Property& prop<OVM::Entity::Face>() {return f_prop_;}
    template<> Property& prop<OVM::Entity::Cell>() {return c_prop_;}

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

    void render(
        wgpu::RenderPassEncoder _render_pass,
        const Mat4x4f& _view_projection) override;

    void render_ui_info() override;

    void render_ui_settings() override;

    void render_ui_properties() override;

    void init() override;

    void recompute_bounding_box() override;

    inline bool& visible() override {
        return renderer_.enabled();
    }

    inline OVMVolumeMesh& mesh() {
        return mesh_;
    }
};

}
