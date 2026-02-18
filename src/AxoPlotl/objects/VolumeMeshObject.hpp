#pragma once

#include "AxoPlotl/typedefs/glm.hpp"
#include "AxoPlotl/objects/BaseObject.hpp"
#include "ToLoG/io/obj_reader.hpp"
#include "ToLoG/mesh/polygon_mesh.hpp"

namespace AxoPlotl
{

class VolumeMeshObject : public BaseObject
{
public:
    VolumeMeshObject(int _id) :
        BaseObject(_id, "Volume Mesh Object")
    {
    }

    void init_renderer(VolumeMeshRenderer::Context _render_context) override
    {
        using P = ToLoG::Point<float,3>;
        ToLoG::PolygonMesh<P> mesh;
        ToLoG::IO::read_polygon_mesh_obj("/Users/tobiaskohler/Uni/IGRec/IGRec-Dataset/Input/bunny.obj", mesh);

        VolumeMeshRenderer::StaticData data;
        for (uint32_t i = 0; i < mesh.n_vertices(); ++i) {
            data.positions_.emplace_back(
                mesh.point(i)[0],
                mesh.point(i)[1],
                mesh.point(i)[2],
                1
                );
            data.vertex_draw_indices_.push_back(i);
        }
        std::vector<VolumeMeshRenderer::EdgePropertyData> e_props;
        for (uint32_t i = 0; i < mesh.n_edges(); ++i) {
            data.edge_draw_indices_.push_back({
                .vertex_index_ = static_cast<uint32_t>(mesh.edge(i).vertex(0)),
                .edge_index_ = i
            });
            data.edge_draw_indices_.push_back({
                .vertex_index_ = static_cast<uint32_t>(mesh.edge(i).vertex(1)),
                .edge_index_ = i
            });
            auto p = ToLoG::normalized(mesh.point(mesh.edge(i).vertex(0)));
            Vec4f sphere_color = Vec4f(
                0.5 * (p[0] + 1),
                0.5 * (p[1] + 1),
                0.5 * (p[2] + 1),
                1
                );
            e_props.push_back({.color_ = sphere_color});
        }

        renderer_.init(_render_context, data);
        renderer_.update_edge_property_data(e_props);
    }
};

}
