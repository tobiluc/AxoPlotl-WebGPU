#include "OpenVolumeMeshRenderer.hpp"
#include "AxoPlotl/rendering/detail/create_static_render_data.hpp"
#include <cstddef>
#include <AxoPlotl/Application.hpp>

namespace AxoPlotl
{

void OpenVolumeMeshRenderer::init(Application *_app, const StaticData& _data)
{
    app_ = _app;

    position_buffer_ = create_position_buffer(app_->device_, _data.positions_);
    n_positions_ = _data.positions_.size();

    vertex_renderer_.init(app_, position_buffer_, _data.vertices_);
    edge_renderer_.init(app_, position_buffer_, _data.edges_);
    face_renderer_.init(app_, position_buffer_, _data.faces_);

    std::vector<Vec4f> cell_centers;
    cell_centers.reserve(_data.cells_.size());
    for (uint32_t ch = 0; ch < _data.cells_.size(); ++ch) {
        cell_centers.push_back({0,0,0,0});
        uint32_t n(0);
        for (const auto& f : _data.cells_[ch]) {
            for (const auto& vh : f) {
                cell_centers.back() += _data.positions_[vh];
                ++n;
            }
        }
        cell_centers.back() /= n;
    }
    cell_renderer_.init(app_, position_buffer_, _data.cells_, cell_centers);
}

void OpenVolumeMeshRenderer::render(const Vec4f &_viewport, wgpu::RenderPassEncoder _render_pass, const Mat4x4f& _mvp)
{
    if (!enabled_) {return;}

    cell_renderer_.render(_viewport, _render_pass, _mvp);
    face_renderer_.render(_viewport, _render_pass, _mvp);
    edge_renderer_.render(_viewport, _render_pass, _mvp);
    vertex_renderer_.render(_viewport, _render_pass, _mvp);
}

void OpenVolumeMeshRenderer::release()
{
    position_buffer_.destroy();
    position_buffer_.release();
}

}
