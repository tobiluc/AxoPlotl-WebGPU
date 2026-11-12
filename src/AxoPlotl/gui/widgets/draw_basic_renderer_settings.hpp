#pragma once
#include "AxoPlotl/geometry/BoundingBox.hpp"
#include <AxoPlotl/core/AxoPlotl_fwd.hpp>
#include <AxoPlotl/rendering/renderers/RendererBase.hpp>

namespace AxoPlotl::internal
{

bool RendererVisibilityToggle(RendererBase* _r);

void draw_clipbox_settings(RendererBase::ClipBox& _cb,
    const BoundingBox& _bb);

void draw_basic_renderer_settings(
    ColoredVertexRenderer& _r);

void draw_basic_renderer_settings(
    ColoredEdgeRenderer& _r);

void draw_basic_renderer_settings(
    ColoredFaceRenderer& _r);

void draw_basic_renderer_settings(
    ColoredCellRenderer& _r);

void draw_basic_renderer_settings(
    Vector3Renderer& _r);

void draw_basic_renderer_settings(
    Matrix3x3Renderer& _r);

}
