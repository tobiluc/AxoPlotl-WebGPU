#pragma once
#include "ToLoG/mesh/polygon_mesh_concepts.hpp"
#include <ToLoG/utils/OVM_Traits.hpp>

namespace AxoPlotl
{

namespace OVM = OpenVolumeMesh;

using VolumeMesh = OpenVolumeMesh::GeometryKernel<OpenVolumeMesh::Vec3f, OpenVolumeMesh::TopologyKernel>;;

template<typename Entity>
struct EntityType {};

template<>
struct EntityType<OVM::Entity::Vertex> {
    static constexpr OVM::EntityType type() {
        return OVM::EntityType::Vertex;
    }
};
template<>
struct EntityType<OVM::Entity::Edge> {
    static constexpr OVM::EntityType type() {
        return OVM::EntityType::Edge;
    }
};
template<>
struct EntityType<OVM::Entity::Face> {
    static constexpr OVM::EntityType type() {
        return OVM::EntityType::Face;
    }
};
template<>
struct EntityType<OVM::Entity::Cell> {
    static constexpr OVM::EntityType type() {
        return OVM::EntityType::Cell;
    }
};
template<>
struct EntityType<OVM::Entity::HalfFace> {
    static constexpr OVM::EntityType type() {
        return OVM::EntityType::HalfFace;
    }
};
template<>
struct EntityType<OVM::Entity::HalfEdge> {
    static constexpr OVM::EntityType type() {
        return OVM::EntityType::HalfEdge;
    }
};

template<OVM::EntityType ET>
size_t mesh_n_entities(const VolumeMesh& _mesh)
{
    if constexpr(ET==OVM::EntityType::Vertex) {return _mesh.n_vertices();}
    if constexpr(ET==OVM::EntityType::Edge) {return _mesh.n_edges();}
    if constexpr(ET==OVM::EntityType::Face) {return _mesh.n_faces();}
    if constexpr(ET==OVM::EntityType::Cell) {return _mesh.n_cells();}
    if constexpr(ET==OVM::EntityType::HalfFace) {return _mesh.n_halffaces();}
    if constexpr(ET==OVM::EntityType::HalfEdge) {return _mesh.n_halfedges();}
}

template<OVM::EntityType ET>
auto mesh_entity_handle(int _idx)
{
    if constexpr(ET==OVM::EntityType::Vertex) {return OVM::VH(_idx);}
    if constexpr(ET==OVM::EntityType::Edge) {return OVM::EH(_idx);}
    if constexpr(ET==OVM::EntityType::Face) {return OVM::FH(_idx);}
    if constexpr(ET==OVM::EntityType::Cell) {return OVM::CH(_idx);}
    if constexpr(ET==OVM::EntityType::HalfFace) {return OVM::HFH(_idx);}
    if constexpr(ET==OVM::EntityType::HalfEdge) {return OVM::HEH(_idx);}
}


template<OVM::EntityType ET>
auto mesh_entities(const VolumeMesh& _mesh)
{
    if constexpr(ET==OVM::EntityType::Vertex) {return _mesh.vertices();}
    if constexpr(ET==OVM::EntityType::Edge) {return _mesh.edges();}
    if constexpr(ET==OVM::EntityType::Face) {return _mesh.faces();}
    if constexpr(ET==OVM::EntityType::Cell) {return _mesh.cells();}
    if constexpr(ET==OVM::EntityType::HalfFace) {return _mesh.halffaces();}
    if constexpr(ET==OVM::EntityType::HalfEdge) {return _mesh.halfedges();}
}

template<ToLoG::polygon_mesh_3 Mesh>
VolumeMesh volume_mesh(const Mesh& _mesh)
{
    using VI = typename ToLoG::Traits<Mesh>::vertex_index;
    VolumeMesh res;

    res.reserve_vertices(_mesh.n_vertices());
    for (int i = 0; i < _mesh.n_vertices(); ++i) {
        const auto& p = _mesh.point(VI(i));
        res.add_vertex(OVM::Vec3f(p[0],p[1],p[2]));
    }
    res.reserve_edges(_mesh.n_edges());
    for (const auto& e : _mesh.edges()) {
        res.add_edge(OVM::VH(ToLoG::index(e.vertex(0))),
                     OVM::VH(ToLoG::index(e.vertex(1))));
    }
    res.reserve_faces(_mesh.n_faces());
    for (const auto& f : _mesh.faces()) {
        std::vector<OVM::VH> vhs;
        for (const auto& v : f.vertices()) {
            vhs.push_back(OVM::VH(ToLoG::index(v)));
        }
        res.add_face(vhs);
    }

    return res;
}

}
