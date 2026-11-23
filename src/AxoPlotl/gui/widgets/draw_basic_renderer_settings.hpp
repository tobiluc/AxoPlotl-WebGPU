#pragma once
#include "AxoPlotl/geometry/BoundingBox.hpp"
#include "AxoPlotl/rendering/shader_mirrors/ClipBox.hpp"
#include <AxoPlotl/core/AxoPlotl_fwd.hpp>
#include <AxoPlotl/rendering/renderers/RendererBase.hpp>

namespace AxoPlotl::gui
{

// bool RendererVisibilityToggle(internal::RendererBase* _r);

void draw_clipbox_settings(internal::ClipBox& _cb,
    const BoundingBox& _bb);

void draw_basic_renderer_settings(
    internal::MeshVertexRenderer& _r);

void draw_basic_renderer_settings(
    internal::MeshHalfEdgeRenderer& _r);

void draw_basic_renderer_settings(
    internal::MeshHalfFaceRenderer& _r);

void draw_basic_renderer_settings(
    internal::MeshCellRenderer& _r);

void draw_basic_renderer_settings(
    internal::Vector3Renderer& _r);

void draw_basic_renderer_settings(
    internal::Matrix3x3Renderer& _r);

}
