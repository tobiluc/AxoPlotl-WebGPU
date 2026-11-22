#include "TetrahedronObject.hpp"
#include "AxoPlotl/core/Application.hpp"
#include <AxoPlotl/core/Scene.hpp>
#include <AxoPlotl/rendering/MeshData.hpp>

namespace AxoPlotl
{

void TetObject::render(
    wgpu::RenderPassEncoder _render_pass,
    const ViewSize& _view_size,
    const Mat4x4& _view,
    const Mat4x4& _proj)

{
    if (deleted() || !visible()) {return;}

    const auto& mvp = (_proj * _view) * transform_;
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
            mesh_data_.vertices_positions.buffer,
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

void TetObject::init()
{
    const wgpu::Device& device = scene()->app()->gpu_context().device;

    mesh_data_.create_buffers(device);

    renderer_.init(
        id_, scene()->app()->gpu_context(),
        scene()->app()->render_context(),
        mesh_data_.vertices_positions,
        mesh_data_.cells_indices,
        mesh_data_.cells_centers,
        mesh_data_.halffaces_normals);
    renderer_.update_property_data({{1,1,1,1}}); // white
    renderer_.cell_scale() = 1;
}

void TetObject::destroy()
{
    renderer_.clear();
}

void TetObject::recompute_bounding_box()
{
    bbox_ = BoundingBox();
    for (int i = 0; i < 4; ++i) {
        bbox_.expand_with_point(Vec3(input_[i][0],input_[i][1],input_[i][2]));
    }
}

}
