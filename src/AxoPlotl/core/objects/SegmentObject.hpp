#pragma once

#include "AxoPlotl/rendering/renderers/MeshEdgeRenderer.hpp"
#include <AxoPlotl/core/AxoPlotl_fwd.hpp>
#include <AxoPlotl/core/objects/BaseObject.hpp>

namespace AxoPlotl
{

class SegmentObject : public ObjectBase
{
private:
    ColoredEdgeRenderer renderer_;
    wgpu::Buffer vertex_position_buffer_;

    std::array<std::array<float,4>,2> input_;

public:
    SegmentObject(
        internal::Scene* _scene) :
        ObjectBase(_scene, "")
    {
        this->name_ = "Segment " + std::to_string(id_);
        input_[0] = {0,0,0,1};
        input_[1] = {1,1,1,1};
    }

    ~SegmentObject() {
        destroy();
    }

    void render(
        wgpu::RenderPassEncoder _render_pass,
        const ViewSize& _view_size,
        const Mat4x4f& _view_projection) override;

    void render_ui_info() override;

    void render_ui_settings() override;

    void render_ui_properties() override;

    void render_ui_picking(const Picker& _p) override;

    void init() override;

    void destroy() override;

    void recompute_bounding_box() override;

    inline float* point(int i) {
        return input_[i].data();
    }
};

}
