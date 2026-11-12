#include "TetrahedronObject.hpp"
#include "AxoPlotl/core/Application.hpp"
#include <AxoPlotl/core/Scene.hpp>
#include <AxoPlotl/rendering/create_static_render_data.hpp>

namespace AxoPlotl
{

void TetObject::render(
    wgpu::RenderPassEncoder _render_pass,
    const ViewSize& _view_size,
    const Mat4x4f& _view_projection)

{
    if (deleted() || !visible()) {return;}

    const auto& mvp = _view_projection * transform_;
    renderer_.directional_light() = scene()->directional_light;

    renderer_.render(_view_size, _render_pass, mvp);
}

void TetObject::render_ui_info()
{
}

void TetObject::render_ui_settings()
{
    input_name();

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
        scene()->app()->gpu_context().queue().writeBuffer(
            vertex_position_buffer_,
            0,
            input_.data(),
            sizeof(input_));
        recompute_bounding_box();
    }
}

void TetObject::render_ui_picking(const Picker& _p)
{
    if (_p.pixel().object_id_ != id()) [[unlikely]] {return;}

    ImGui::SeparatorText(name_.c_str());
    ImGui::Text("(x, y, z) = (%f, %f, %f)",
        _p.pixel().position_.x,
        _p.pixel().position_.y,
        _p.pixel().position_.z);
}

void TetObject::init_buffers()
{
    const wgpu::Device& device = scene()->app()->gpu_context().device;
    std::vector<ColoredCellRenderer::Position> positions = {
        {0,0,0,1}, {0,0,1,1}, {1,0,0,1}, {0,1,0,1}
    };
    vertex_position_buffer_ = internal::create_position_buffer(device, positions);
    std::vector<ColoredCellRenderer::Position> center(1);
    cell_center_buffer_ = internal::create_position_buffer(device, center);
    std::vector<std::vector<std::vector<uint32_t>>> cells = {
        {{0,1,2},{0,3,1},{0,2,3},{1,3,2}}
    };
    renderer_.init(id_, scene()->app()->gpu_context(),
                   scene()->app()->render_context(),
                   vertex_position_buffer_,
                   positions, cells, cell_center_buffer_);
    renderer_.update_property_data({{1,1,1,1}}); // white
    renderer_.cell_scale() = 1;
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
