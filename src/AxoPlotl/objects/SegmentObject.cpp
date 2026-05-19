#include <AxoPlotl/objects/SegmentObject.hpp>
#include "AxoPlotl/Application.hpp"
#include <AxoPlotl/Scene.hpp>
#include <AxoPlotl/rendering/detail/create_static_render_data.hpp>

namespace AxoPlotl
{

void SegmentObject::render(
    wgpu::RenderPassEncoder _render_pass,
    const Mat4x4f& _view_projection)

{
    if (deleted() || !visible()) {return;}

    const auto& mvp = _view_projection * transform_;
    const auto& vp = scene_->app()->scene_viewport();

    renderer_.render(vp, _render_pass, mvp);
}

void SegmentObject::render_ui_info()
{
}

void SegmentObject::render_ui_settings()
{
    ImGui::ColorEdit3("Color", &renderer_.ambient()[0]);
}

void SegmentObject::render_ui_properties()
{
    bool changed = false;
    changed |= ImGui::InputFloat3("A", input_[0].data());
    changed |= ImGui::InputFloat3("B", input_[1].data());
    if (changed) {
        scene_->app()->device_.getQueue().writeBuffer(
            vertex_position_buffer_,
            0,
            input_.data(),
            sizeof(input_));
        recompute_bounding_box();
    }
}

void SegmentObject::render_ui_picking(PickResult _p, const PickConfig& _cfg)
{
    if (_p.object_id_ != id()) [[unlikely]] {return;}
    ImGui::SeparatorText(name_.c_str());
    ImGui::Text("(x, y, z) = (%f, %f, %f)",
                _p.position_.x, _p.position_.y, _p.position_.z);
}

void SegmentObject::init_buffers()
{
    std::vector<ColoredCellRenderer::Position> positions = {
        {0,0,0,1}, {1,1,1,1}
    };
    vertex_position_buffer_ = create_position_buffer(scene_->app()->device_, positions);
    renderer_.init(id_, scene_->app(), vertex_position_buffer_, {{0,1}});
    renderer_.update_property_data({{1,1,1,1}}); // white
}

void SegmentObject::delete_buffers()
{
    renderer_.clear();
}

void SegmentObject::recompute_bounding_box()
{
    bbox_ = BoundingBox();
    for (int i = 0; i < 2; ++i) {
        bbox_.expand_with_point(Vec3f(input_[i][0],input_[i][1],input_[i][2]));
    }
}

}
