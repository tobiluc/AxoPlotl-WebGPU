#include <AxoPlotl/objects/TetrahedronObject.hpp>
#include "AxoPlotl/Application.hpp"
#include <AxoPlotl/Scene.hpp>
#include <AxoPlotl/rendering/detail/create_static_render_data.hpp>

namespace AxoPlotl
{

void TetObject::render(
    wgpu::RenderPassEncoder _render_pass,
    const Mat4x4f& _view_projection)

{
    if (deleted() || !visible()) {return;}

    const auto& mvp = _view_projection * transform_;
    const auto& vp = scene_->app()->scene_viewport();

    renderer_.render(vp, _render_pass, mvp);
}

void TetObject::render_ui_info()
{
}

void TetObject::render_ui_settings()
{
    ImGui::ColorEdit3("Color", &renderer_.ambient()[0]);
}

void TetObject::render_ui_properties()
{
    bool changed = false;
    changed |= ImGui::InputFloat3("A", input_[0].data());
    changed |= ImGui::InputFloat3("B", input_[1].data());
    changed |= ImGui::InputFloat3("C", input_[2].data());
    changed |= ImGui::InputFloat3("D", input_[3].data());
    if (changed) {
        scene_->app()->device_.getQueue().writeBuffer(
            vertex_position_buffer_,
            0,
            input_.data(),
            sizeof(input_));
        recompute_bounding_box();
    }
}

void TetObject::render_ui_picking(PickResult _p, const PickConfig& _cfg)
{
    if (_p.object_id_ != id()) [[unlikely]] {return;}
    ImGui::SeparatorText(name_.c_str());
    ImGui::Text("(x, y, z) = (%f, %f, %f)",
                _p.position_.x, _p.position_.y, _p.position_.z);
}

void TetObject::init_buffers()
{
    std::vector<ColoredCellRenderer::Position> positions = {
        {0,0,0,1}, {0,0,1,1}, {1,0,0,1}, {0,1,0,1}
    };
    vertex_position_buffer_ = create_position_buffer(scene_->app()->device_, positions);
    std::vector<ColoredCellRenderer::Position> center(1);
    cell_center_buffer_ = create_position_buffer(scene_->app()->device_, center);
    std::vector<std::vector<std::vector<uint32_t>>> cells = {
        {{0,1,2},{0,3,1},{0,2,3},{1,3,2}}
    };
    renderer_.init(id_, scene_->app(), vertex_position_buffer_, cells, cell_center_buffer_);
    renderer_.update_property_data({{1,1,1,1}}); // white
}

void TetObject::delete_buffers()
{
    renderer_.clear();
}

void TetObject::recompute_bounding_box()
{
    bbox_ = BoundingBox();
    for (int i = 0; i < 4; ++i) {
        bbox_.expand_with_point(Vec3f(input_[i][0],input_[i][1],input_[i][2]));
    }
}

}
