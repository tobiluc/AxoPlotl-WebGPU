#include "SegmentObject.hpp"
#include "AxoPlotl/core/Application.hpp"
#include "AxoPlotl/gui/widgets/draw_basic_renderer_settings.hpp"
#include <AxoPlotl/core/Scene.hpp>
#include <AxoPlotl/rendering/MeshData.hpp>

namespace AxoPlotl
{

void SegmentObject::render(
    wgpu::RenderPassEncoder _render_pass,
    const ViewSize& _view_size,
    const Mat4x4& _view,
    const Mat4x4& _proj)

{
    if (deleted() || !visible()) {return;}

    renderer_.directional_light() = scene()->directional_light;

    const auto& mvp = MVP{
        .model=transform_,
        .view = _view,
        .proj = _proj
    };

    renderer_.render(_view_size, _render_pass, mvp);
}

void SegmentObject::draw_info_ui()
{
}

void SegmentObject::draw_settings_ui()
{
    draw_name_input_ui();
    gui::draw_basic_renderer_settings(renderer_);
}

void SegmentObject::draw_properties_ui()
{
    bool changed = false;

    changed |= ImGui::InputFloat3("A", input_[0].data());
    changed |= ImGui::InputFloat3("B", input_[1].data());
    if (changed) {
        scene()->app()->gpu_context().queue().writeBuffer(
            mesh_data_.vertices_positions.buffer,
            0,
            input_.data(),
            sizeof(input_));
        recompute_bounding_box();
    }
}

void SegmentObject::draw_picking_ui(const Picker& _p)
{
    if (_p.pixel().entity.object_id != id()) [[unlikely]] {return;}
    ImGui::SeparatorText(name_.c_str());
    ImGui::Text("(x, y, z) = (%f, %f, %f)",
        _p.pixel().position.x,
        _p.pixel().position.y,
        _p.pixel().position.z);
}

void SegmentObject::init()
{
    // std::vector<ColoredCellRenderer::Position> positions = {
    //     {0,0,0,1}, {1,1,1,1}
    // };
    // vertex_position_buffer_ = internal::create_position_buffer(scene()->app()->gpu_context().device, positions);
    mesh_data_.create_buffers(scene()->app()->gpu_context().device,
                VolMeshData::Flag::INIT_EDGE_BUFFERS);
    renderer_.init(id_, scene()->app()->gpu_context(), scene()->app()->render_context(), mesh_data_.vertices_positions, mesh_data_.halfedges_instances);
    renderer_.update_edge_property_data({{1,1,1,1}}); // white
}

void SegmentObject::cleanup()
{
    renderer_.cleanup();
}

void SegmentObject::recompute_bounding_box()
{
    bbox_ = BoundingBox();
    for (int i = 0; i < 2; ++i) {
        bbox_.expand_with_point(Vec3(input_[i][0],input_[i][1],input_[i][2]));
    }
}

}
