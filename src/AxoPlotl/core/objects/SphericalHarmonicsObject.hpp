#pragma once

#include "AxoPlotl/core/objects/ObjectBase.hpp"
#include "AxoPlotl/rendering/renderers/SphericalHarmonicsRenderer.hpp"

namespace AxoPlotl
{

class SHObject : public ObjectBase
{
private:
    SHRenderer renderer_;
    Vec3 lengths = {1,1,1};
    Vec3 angles = {0,0,0};
    constexpr static size_t render_resolution_ = 64;

    enum class Type
    {
        OCTA,
        ODECO,
        CUSTOM // use vars x/y/z (cartesian) phi/theta (polar)
    } type_;
    std::string custom_input_ = "x^4 + y^4 + z^4";
    bool request_pick_popup_ = false;

public:
    SHObject(
        internal::Scene* _scene) :
        ObjectBase(_scene, "")
    {
        this->name_ = "SH " + std::to_string(id_);
    }

    ~SHObject() {
        cleanup();
    }

    void render(
        wgpu::RenderPassEncoder _render_pass,
        const ViewSize& _view_size,
        const Mat4x4& _view,
        const Mat4x4& _proj) override;

    void draw_settings_ui() override;

    void draw_selected_ui() override;

    bool on_mouse_just_pressed(const Picker::Pixel& _p, const Mouse::Button _button) override;

    void draw_viewport_ui() override;

    void init() override;

    void cleanup() override;

    void recompute_bounding_box() override;

};

using SphericalHarmonicsObject = SHObject;

}
