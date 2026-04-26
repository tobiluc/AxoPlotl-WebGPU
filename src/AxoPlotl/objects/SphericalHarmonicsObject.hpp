#pragma once

#include "AxoPlotl/objects/BaseObject.hpp"
#include "AxoPlotl/rendering/SphericalHarmonicsRenderer.hpp"

namespace AxoPlotl
{

class SHObject : public ObjectBase
{
private:
    SHRenderer renderer_;
    Vec3f lengths = {1,1,1};
    Vec3f angles = {0,0,0};
    constexpr static size_t render_resolution_ = 64;

public:
    SHObject(
        Scene* _scene) :
        ObjectBase(_scene, "")
    {
        this->name_ = "SH " + std::to_string(id_);
    }

    ~SHObject() {
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

};

using SphericalHarmonicsObject = SHObject;

}
