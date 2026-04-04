#pragma once
#include <OpenVolumeMesh/Core/Handles.hh>
#include <AxoPlotl/AxoPlotl_fwd.hpp>

namespace AxoPlotl
{

template<typename EntityTag>
struct colored_renderer_for_entity;
template<> struct colored_renderer_for_entity<OpenVolumeMesh::Entity::Vertex> {using type=ColoredVertexRenderer;};
template<> struct colored_renderer_for_entity<OpenVolumeMesh::Entity::Edge> {using type=ColoredEdgeRenderer;};
template<> struct colored_renderer_for_entity<OpenVolumeMesh::Entity::Face> {using type=ColoredFaceRenderer;};
template<> struct colored_renderer_for_entity<OpenVolumeMesh::Entity::Cell> {using type=ColoredCellRenderer;};
template<typename EntityTag> using colored_renderer_for_entity_t = typename colored_renderer_for_entity<EntityTag>::type;

}
