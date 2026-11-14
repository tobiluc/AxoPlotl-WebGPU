#include "draw_basic_renderer_settings.hpp"
#include "AxoPlotl/gui/widgets/input_slider.hpp"
#include <IconsFontAwesome7.h>
#include <AxoPlotl/rendering/renderers/renderers.hpp>
#include <imgui.h>

namespace AxoPlotl::internal
{

bool RendererVisibilityToggle(RendererBase* _r)
{
    if (ImGui::Button(_r->enabled() ? ICON_FA_EYE : ICON_FA_EYE_SLASH)) {
        _r->enabled() = !_r->enabled();
        return true;
    }
    return false;
}

void draw_clipbox_settings(RendererBase::ClipBox& _cb, const BoundingBox &_bb)
{
    bool clip_box_enabled = _cb.enabled_;
    if (ImGui::Checkbox("Enable Clip Box", &clip_box_enabled)) {
        _cb.min_ = _bb.min();
        _cb.max_ = _bb.max();
        _cb.enabled_ = true;
    }
    _cb.enabled_ = clip_box_enabled;
    if (clip_box_enabled)
    {
        Vec2f x = {_cb.min_[0],_cb.max_[0]};
        Vec2f y = {_cb.min_[1],_cb.max_[1]};
        Vec2f z = {_cb.min_[2],_cb.max_[2]};
        ImGui::SliderFloat2("x", &x[0], _bb.min()[0], _bb.max()[0]);
        ImGui::SliderFloat2("y", &y[0], _bb.min()[1], _bb.max()[1]);
        ImGui::SliderFloat2("z", &z[0], _bb.min()[2], _bb.max()[2]);
        _cb.min_ = {x[0],y[0],z[0]};
        _cb.max_ = {x[1],y[1],z[1]};
    }
}

void draw_basic_renderer_settings(
    ColoredVertexRenderer& _r)
{
    ImGui::ColorEdit3("Vertex Ambient", &_r.ambient()[0]);
    InputSlider("Point Size",
                &_r.point_size(),
                InputSliderOptions{
                                   .step=0.1f, .min= 0.0f, .max=128.0f});
    InputSlider("Depth Offset",
                &_r.depth_offset(),
                InputSliderOptions{
                                   .step=0.00001,
                                   .min = -0.5f,
                                   .max = 0.5f,
                                   .fmt = "%.5f"});
}

void draw_basic_renderer_settings(
    ColoredEdgeRenderer& _r)
{
    ImGui::ColorEdit3("Edge Ambient", &_r.ambient()[0]);
    InputSlider("Line Width",
                &_r.line_width(),
                InputSliderOptions{
                    .step=0.1f, .min= 0.0f, .max=32.0f});
    InputSlider("Depth Offset",
                &_r.depth_offset(),
                InputSliderOptions{
                    .step=0.00001,
                    .min = -0.5f,
                    .max = 0.5f,
                    .fmt = "%.5f"
                });
}

void draw_basic_renderer_settings(
    ColoredFaceRenderer& _r)
{
    ImGui::ColorEdit3("Face Ambient", &_r.ambient()[0]);
    InputSlider("Depth Offset",
                &_r.depth_offset(),
                InputSliderOptions{
                                   .step=0.00001,
                                   .min = -0.5f,
                                   .max = 0.5f,
                                   .fmt = "%.5f"});
}

void draw_basic_renderer_settings(
    ColoredCellRenderer& _r)
{
    ImGui::ColorEdit3("Cell Ambient", &_r.ambient()[0]);
    InputSlider("Cell Scale", &_r.cell_scale(), InputSliderOptions{.min= 0.0f, .max=2.0f});
}

void draw_basic_renderer_settings(
    Vector3Renderer& _r)
{
    InputSlider("Vector Scale", &_r.vector_scale());
}

void draw_basic_renderer_settings(
    Matrix3x3Renderer& _r)
{
    bool b = _r.normalize();
    ImGui::Checkbox("Normalize", &b);
    _r.normalize() = b;

    b = _r.draw_symmetric();
    ImGui::Checkbox("Symmetric", &b);
    _r.draw_symmetric() = b;

    InputSlider("Vector Scale", &_r.vector_scale());
    InputSlider("Line Width", &_r.line_width());

    ImGui::ColorEdit3("Color 1", &_r.axis_colors()[0][0]);
    ImGui::ColorEdit3("Color 2", &_r.axis_colors()[1][0]);
    ImGui::ColorEdit3("Color 3", &_r.axis_colors()[2][0]);
}

}
