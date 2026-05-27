#pragma once

#include <AxoPlotl/AxoPlotl_fwd.hpp>
#include <AxoPlotl/objects/BaseObject.hpp>
#include <AxoPlotl/rendering/MeshCellRenderer.hpp>

namespace AxoPlotl
{

class TetObject : public ObjectBase
{
private:
    ColoredCellRenderer renderer_;
    wgpu::Buffer vertex_position_buffer_;
    wgpu::Buffer cell_center_buffer_;

    std::array<std::array<float,4>,4> input_;

public:
    TetObject(
        Scene* _scene) :
        ObjectBase(_scene, "")
    {
        this->name_ = "Tet " + std::to_string(id_);
        input_[0] = {0,0,0,1};
        input_[1] = {0,0,1,1};
        input_[2] = {1,0,0,1};
        input_[3] = {0,1,0,1};
    }

    ~TetObject() {
        delete_buffers();
    }

    void render(
        wgpu::RenderPassEncoder _render_pass,
        const Mat4x4f& _view_projection) override;

    void render_ui_info() override;

    void render_ui_settings() override;

    void render_ui_properties() override;

    void render_ui_picking(PickResult _p, const PickConfig& _cfg) override;

    void init_buffers() override;

    void delete_buffers() override;

    void recompute_bounding_box() override;

    inline float* point(int i) {
        return input_[i].data();
    }
};

using TetrahedronObject = TetObject;

}
