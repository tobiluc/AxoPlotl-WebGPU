#pragma once
#include "AxoPlotl/rendering/MeshCellRenderer.hpp"
#include "AxoPlotl/rendering/MeshEdgeRenderer.hpp"
#include "AxoPlotl/rendering/MeshFaceRenderer.hpp"
#include "AxoPlotl/rendering/MeshVertexRenderer.hpp"
#include <AxoPlotl/typedefs/ovm.hpp>

namespace AxoPlotl
{

template<typename EntityTag>
struct colored_renderer_for_entity;
template<> struct colored_renderer_for_entity<OVM::Entity::Vertex> {using type=ColoredVertexPropertyRenderer;};
template<> struct colored_renderer_for_entity<OVM::Entity::Edge> {using type=ColoredEdgePropertyRenderer;};
template<> struct colored_renderer_for_entity<OVM::Entity::Face> {using type=ColoredFacePropertyRenderer;};
template<> struct colored_renderer_for_entity<OVM::Entity::Cell> {using type=ColoredCellPropertyRenderer;};
template<typename EntityTag> using colored_renderer_for_entity_t = typename colored_renderer_for_entity<EntityTag>::type;

}
