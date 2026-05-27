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
        OVM::HEH heh = eh.halfedge_handle(0);
        OVM::VH vh0 = _mesh.from_vertex_handle(heh);
        OVM::VH vh1 = _mesh.to_vertex_handle(heh);
        data.edges_.push_back({vh0.uidx(), vh1.uidx()});
        const auto& p = _mesh.barycenter(eh);
        data.edges_barycenters_.emplace_back(p[0],p[1],p[2],1);
    }
    for (auto fh : _mesh.faces()) {
        data.faces_.emplace_back();
        OVM::Vec3f bary(0,0,0);
        float bary_count(0);
        for (const auto& vh : _mesh.halfface_vertices(fh.halfface_handle(0))) {
            data.faces_.back().push_back(vh.uidx());
            bary += _mesh.vertex(vh);
            ++bary_count;
        }
        bary /= bary_count;
        data.faces_barycenters_.emplace_back(bary[0],bary[1],bary[2],1);
    }
    for (auto ch : _mesh.cells()) {
        data.cells_.emplace_back();
        OVM::Vec3f bary(0,0,0);
        float bary_count(0);
        for (auto hfh : _mesh.cell_halffaces(ch)) {
            data.cells_.back().emplace_back();
            for (const auto& vh : _mesh.halfface_vertices(hfh)) {
                data.cells_.back().back().push_back(vh.uidx());
                bary += _mesh.vertex(vh);
                ++bary_count;
            }
        }
        bary /= bary_count;
        data.cells_barycenters_.emplace_back(bary[0],bary[1],bary[2],1);
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
    desc.size = sizeof(RendererBase::Position) * std::max(_positions.size(), 1lu);
    desc.mappedAtCreation = false;
    desc.label = wgpu::StringView("Position");

    wgpu::Buffer buffer = _device.createBuffer(desc);
    if (!_positions.empty()) [[likely]] {
        _device.getQueue().writeBuffer(buffer, 0, _positions.data(), desc.size);
    }
    return buffer;
}
