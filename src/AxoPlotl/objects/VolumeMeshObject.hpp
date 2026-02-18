#pragma once

#include "AxoPlotl/typedefs/ToLoG.hpp"
#include "AxoPlotl/objects/BaseObject.hpp"
#include "AxoPlotl/typedefs/ovm.hpp"

namespace AxoPlotl
{

class VolumeMeshObject : public BaseObject
{
private:
    VolumeMesh mesh_;


public:
    VolumeMeshObject(VolumeMesh& _mesh) :
        BaseObject("Volume Mesh Object"), mesh_(std::move(_mesh))
    {
    }

    void render_ui() override;

    void init_renderer(VolumeMeshRenderer::Context _render_context) override;

    void recompute_bounding_box() override;
};

}
