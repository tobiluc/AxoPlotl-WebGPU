#pragma once

#include "AxoPlotl/typedefs/ToLoG.hpp"
#include "AxoPlotl/objects/BaseObject.hpp"
#include "AxoPlotl/typedefs/ovm.hpp"
#include <filesystem>

namespace AxoPlotl
{

class VolumeMeshObject : public ObjectBase
{
private:
    VolumeMesh mesh_;
    std::optional<std::filesystem::path> filepath_;

public:
    VolumeMeshObject(VolumeMesh& _mesh,
        const std::optional<std::filesystem::path>& _filepath = std::nullopt) :
        ObjectBase(""),
        mesh_(std::move(_mesh)),
        filepath_(_filepath)
    {
        this->name_ = _filepath.has_value()?
            _filepath.value().stem().string() :
            "New Volume Mesh Object " + std::to_string(id_);
    }

    void render_ui() override;

    void init_renderer(VolumeMeshRenderer::Context _render_context) override;

    void recompute_bounding_box() override;

    inline const VolumeMesh& mesh() const {
        return mesh_;
    }
};

}
