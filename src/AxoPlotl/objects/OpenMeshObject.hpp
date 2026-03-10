#pragma once

#include "AxoPlotl/objects/BaseObject.hpp"
#include "AxoPlotl/rendering/OpenVolumeMeshRenderer.hpp"
#include "AxoPlotl/typedefs/om.hpp"
#include <filesystem>

namespace AxoPlotl
{

class OpenMeshObject : public ObjectBase
{
private:
    OMSurfaceMesh mesh_;
    OpenVolumeMeshRenderer renderer_;
    std::optional<std::filesystem::path> filepath_;

    void upload_default_property_data();

public:
    OpenMeshObject(Scene* _scene, OMSurfaceMesh& _mesh,
            const std::optional<std::filesystem::path>& _filepath = std::nullopt) :
        ObjectBase(_scene, ""),
        mesh_(std::move(_mesh)),
        filepath_(_filepath)
    {
        this->name_ = _filepath.has_value()?
                          _filepath.value().stem().string() :
                          "New Open Mesh Object " + std::to_string(id_);
        mesh_.request_face_normals();
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

    inline const OMSurfaceMesh& mesh() const {
        return mesh_;
    }
};

}
