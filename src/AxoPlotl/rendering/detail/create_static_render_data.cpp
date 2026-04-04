#include <AxoPlotl/rendering/detail/create_static_render_data.hpp>

AxoPlotl::StaticRenderData AxoPlotl::create_static_render_data(const OVMVolumeMesh& _mesh)
{
    AxoPlotl::StaticRenderData data;
    data.positions_.reserve(_mesh.n_vertices());
    data.vertices_.reserve(_mesh.n_vertices());
    data.edges_.reserve(_mesh.n_edges());
    data.edges_barycenters_.reserve(_mesh.n_edges());
    data.faces_.reserve(_mesh.n_faces());
    data.faces_barycenters_.reserve(_mesh.n_faces());
    data.cells_.reserve(_mesh.n_cells());
    data.cells_barycenters_.reserve(_mesh.n_cells());

    for (auto v_it = _mesh.v_iter(); v_it.is_valid(); ++v_it) {
        const auto& p = _mesh.vertex(*v_it);
        data.positions_.emplace_back(p[0],p[1],p[2],1);
        data.vertices_.push_back(v_it->uidx());
    }
    for (auto eh : _mesh.edges()) {
        const auto& e = _mesh.edge(eh);
        data.edges_.push_back({
            e.from_vertex().uidx(),
            e.to_vertex().uidx()
        });
        const auto& p = _mesh.barycenter(eh);
        data.edges_barycenters_.emplace_back(p[0],p[1],p[2],1);
    }
    for (auto fh : _mesh.faces()) {
        const auto& vhs = _mesh.get_halfface_vertices(fh.halfface_handle(0));
        data.faces_.emplace_back();
        for (const auto& vh : vhs) {
            data.faces_.back().push_back(vh.uidx());
        }
        const auto& p = _mesh.barycenter(fh);
        data.faces_barycenters_.emplace_back(p[0],p[1],p[2],1);
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
        const auto& p = _mesh.barycenter(ch);
        data.cells_barycenters_.emplace_back(p[0],p[1],p[2],1);
    }
    return data;
}

wgpu::Buffer AxoPlotl::create_position_buffer(
    wgpu::Device _device,
    const std::vector<PropertyRendererBase::Position>& _positions)
{
    wgpu::BufferDescriptor desc{};
    desc.usage =
        wgpu::BufferUsage::Storage |
        wgpu::BufferUsage::CopyDst |
        wgpu::BufferUsage::Vertex;
    desc.size = sizeof(PropertyRendererBase::Position) * std::max(_positions.size(), 1lu);
    desc.mappedAtCreation = false;
    desc.label = "Position";

    wgpu::Buffer buffer = _device.createBuffer(desc);
    if (!_positions.empty()) [[likely]] {
        _device.getQueue().writeBuffer(buffer, 0, _positions.data(), desc.size);
    }
    return buffer;
}
