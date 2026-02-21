#include "create_static_render_data.hpp"


AxoPlotl::VolumeMeshRenderer::StaticData AxoPlotl::create_static_render_data(const SurfaceMesh& _mesh)
{
    AxoPlotl::VolumeMeshRenderer::StaticData data;
    data.n_faces_ = _mesh.n_faces();

    for (uint32_t i = 0; i < _mesh.n_vertices(); ++i) {
        data.positions_.emplace_back(
            _mesh.point(i)[0],
            _mesh.point(i)[1],
            _mesh.point(i)[2],
            1
            );
        data.vertex_instances_.push_back(i);
    }
    for (uint32_t i = 0; i < _mesh.n_edges(); ++i) {
        data.edge_instances_.push_back({
            .vh0_=static_cast<uint32_t>(_mesh.edge(i).vertex(0)),
            .vh1_=static_cast<uint32_t>(_mesh.edge(i).vertex(1)),
            .eh_=i
        });
    }
    for (uint32_t i = 0; i < _mesh.n_faces(); ++i) {
        const auto& f = _mesh.face(i);
        for (int j = 1; j < f.valence()-1; ++j) {
            data.face_draw_triangle_indices_.push_back({
                .vertex_index_ = static_cast<uint32_t>(f.vertices().at(0)),
                .face_index_ = i
            });
            data.face_draw_triangle_indices_.push_back({
                .vertex_index_ = static_cast<uint32_t>(f.vertices().at(j)),
                .face_index_ = i
            });
            data.face_draw_triangle_indices_.push_back({
                .vertex_index_ = static_cast<uint32_t>(f.vertices().at(j+1)),
                .face_index_ = i
            });
        }
    }
    return data;
}

AxoPlotl::VolumeMeshRenderer::StaticData AxoPlotl::create_static_render_data(const VolumeMesh& _mesh)
{
    AxoPlotl::VolumeMeshRenderer::StaticData data;
    data.n_faces_ = _mesh.n_faces();

    for (auto v_it = _mesh.v_iter(); v_it.is_valid(); ++v_it) {
        const auto& p = _mesh.vertex(*v_it);
        data.positions_.emplace_back(p[0],p[1],p[2],1);
        data.vertex_instances_.push_back(v_it->uidx());
    }
    for (auto e_it = _mesh.e_iter(); e_it.is_valid(); ++e_it) {
        const auto& e = _mesh.edge(*e_it);
        data.edge_instances_.push_back({
            .vh0_ = e.from_vertex().uidx(),
            .vh1_ = e.to_vertex().uidx(),
            .eh_ = e_it->uidx()
        });
    }
    for (auto f_it = _mesh.f_iter(); f_it.is_valid(); ++f_it) {
        const auto& vhs = _mesh.get_halfface_vertices(f_it->halfface_handle(0));
        for (auto j = 1u; j < vhs.size()-1; ++j) {
            data.face_draw_triangle_indices_.push_back({
                .vertex_index_ = vhs[0].uidx(),
                .face_index_ = f_it->uidx()
            });
            data.face_draw_triangle_indices_.push_back({
                .vertex_index_ = vhs[j].uidx(),
                .face_index_ = f_it->uidx()
            });
            data.face_draw_triangle_indices_.push_back({
                .vertex_index_ = vhs[j+1].uidx(),
                .face_index_ = f_it->uidx()
            });
        }
    }
    data.n_cells_ = _mesh.n_cells();
    for (auto ch : _mesh.cells()) {
        const auto& bary = _mesh.barycenter(ch);
        data.cell_incenters_.push_back(Vec4f(bary[0],bary[1],bary[2],1));

        // triangulate each face
        for (auto hfh : _mesh.cell(ch).halffaces()) {
            const auto& vhs = _mesh.get_halfface_vertices(hfh);
            for (int i = 1; i < vhs.size()-1; ++i) {
                data.cell_draw_triangle_indices_.push_back({
                    .vertex_index_ = vhs[0].uidx(),
                    .cell_index_ = ch.uidx()
                });
                data.cell_draw_triangle_indices_.push_back({
                    .vertex_index_ = vhs[i].uidx(),
                    .cell_index_ = ch.uidx()
                });
                data.cell_draw_triangle_indices_.push_back({
                    .vertex_index_ = vhs[i+1].uidx(),
                    .cell_index_ = ch.uidx()
                });
            }
        }
    }
    return data;
}
