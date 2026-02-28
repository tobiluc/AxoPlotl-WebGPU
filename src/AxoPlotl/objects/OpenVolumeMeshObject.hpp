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
    std::optional<OVM::PropertyStorageBase*> prop_;
    std::vector<std::shared_ptr<PropertyFilterBase>> prop_filters_;
    int filter_index_ = 0;

    void upload_default_property_data();

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

    void render_ui() override;

    void init() override;

    void recompute_bounding_box() override;

    inline bool& visible() override {
        return renderer_.enabled();
    }

    inline const OVMVolumeMesh& mesh() const {
        return mesh_;
    }
};

}
