#include "create_static_render_data.hpp"


AxoPlotl::VolumeMeshRenderer::StaticData AxoPlotl::create_static_render_data(const SurfaceMesh& _mesh)
{
    AxoPlotl::VolumeMeshRenderer::StaticData data;

    for (uint32_t i = 0; i < _mesh.n_vertices(); ++i) {
        data.positions_.emplace_back(
            _mesh.point(i)[0],
            _mesh.point(i)[1],
            _mesh.point(i)[2],
            1
            );
        data.vertices_.push_back(i);
    }
    for (uint32_t i = 0; i < _mesh.n_edges(); ++i) {
        data.edges_.push_back({
            static_cast<uint32_t>(_mesh.edge(i).vertex(0)),
            static_cast<uint32_t>(_mesh.edge(i).vertex(1))
        });
    }
    for (uint32_t i = 0; i < _mesh.n_faces(); ++i) {
        data.faces_.emplace_back();
        for (const auto& vh : _mesh.face(i).vertices()) {
            data.faces_.back().push_back(vh);
        }
    }
    return data;
}

AxoPlotl::VolumeMeshRenderer::StaticData AxoPlotl::create_static_render_data(const VolumeMesh& _mesh)
{
    AxoPlotl::VolumeMeshRenderer::StaticData data;

    for (auto v_it = _mesh.v_iter(); v_it.is_valid(); ++v_it) {
        const auto& p = _mesh.vertex(*v_it);
        data.positions_.emplace_back(p[0],p[1],p[2],1);
        data.vertices_.push_back(v_it->uidx());
    }
    for (auto e_it = _mesh.e_iter(); e_it.is_valid(); ++e_it) {
        const auto& e = _mesh.edge(*e_it);
        data.edges_.push_back({
            e.from_vertex().uidx(),
            e.to_vertex().uidx()
        });
    }
    for (auto f_it = _mesh.f_iter(); f_it.is_valid(); ++f_it) {
        const auto& vhs = _mesh.get_halfface_vertices(f_it->halfface_handle(0));
        data.faces_.emplace_back();
        for (const auto& vh : vhs) {
            data.faces_.back().push_back(vh.uidx());
        }
    }
    for (auto ch : _mesh.cells()) {
        data.cells_.emplace_back();
        for (auto hfh : _mesh.cell(ch).halffaces()) {
            data.cells_.back().emplace_back();
            const auto& vhs = _mesh.get_halfface_vertices(hfh);
            for (const auto& vh : vhs) {
                data.cells_.back().back().push_back(vh.uidx());
            }
        }
    }
    return data;
}

wgpu::Buffer AxoPlotl::create_position_buffer(
    wgpu::Device _device,
    const std::vector<RendererBase::Position>& _positions)
{
    wgpu::BufferDescriptor desc{};
    desc.usage =
        wgpu::BufferUsage::Storage |
        wgpu::BufferUsage::CopyDst |
        wgpu::BufferUsage::Vertex;
    desc.size = sizeof(RendererBase::Position) * _positions.size();
    desc.mappedAtCreation = false;
    desc.label = "Position";

    wgpu::Buffer buffer = _device.createBuffer(desc);
    _device.getQueue().writeBuffer(buffer, 0, _positions.data(), desc.size);
    return buffer;
}
