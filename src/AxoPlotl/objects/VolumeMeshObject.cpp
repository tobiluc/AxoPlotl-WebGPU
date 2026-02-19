#include "VolumeMeshObject.hpp"
#include "AxoPlotl/rendering/detail/create_static_render_data.hpp"

namespace AxoPlotl
{

void VolumeMeshObject::render_ui()
{

}

void VolumeMeshObject::init_renderer(VolumeMeshRenderer::Context _render_context)
{
    renderer_.init(_render_context, create_static_render_data(mesh_));

    std::vector<VolumeMeshRenderer::PropertyData> props;
    for (uint32_t i = 0; i < mesh_.n_edges(); ++i) {
        props.push_back({.color_ = Vec4f(0,0,0,1)});
    }
    renderer_.update_edge_property_data(props);

    props.clear();
    for (OVM::FH fh : mesh_.faces()) {
        auto p = ToLoG::normalized(mesh_.normal(fh.halfface_handle(0)));
        Vec4f sphere_color = Vec4f(
            0.5 * (p[0] + 1),
            0.5 * (p[1] + 1),
            0.5 * (p[2] + 1),
            1
        );
        props.push_back({.color_ = sphere_color});
    }
    renderer_.update_face_property_data(props);
}

void VolumeMeshObject::recompute_bounding_box()
{
    bbox_.make_empty();
    for (const auto& p : mesh_.vertex_positions()) {
        bbox_.expand(Vec3f(p[0],p[1],p[2]));
    }
}

}
