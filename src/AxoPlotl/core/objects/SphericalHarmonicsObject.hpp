#pragma once

#include "AxoPlotl/core/objects/BaseObject.hpp"
#include "AxoPlotl/rendering/renderers/SphericalHarmonicsRenderer.hpp"

namespace AxoPlotl
{

class SHObject : public ObjectBase
{
private:
    SHRenderer renderer_;
    Vec3f lengths = {1,1,1};
    Vec3f angles = {0,0,0};
    constexpr static size_t render_resolution_ = 64;

    enum class Type
    {
        OCTA,
        ODECO,
        CUSTOM // use vars x/y/z (cartesian) phi/theta (polar)
    } type_;
    std::string custom_input_ = "x^4 + y^4 + z^4";

public:
    SHObject(
        internal::Scene* _scene) :
        ObjectBase(_scene, "")
    {
        this->name_ = "SH " + std::to_string(id_);
    }

    ~SHObject() {
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

};

using SphericalHarmonicsObject = SHObject;

}
