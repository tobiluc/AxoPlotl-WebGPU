#include "OpenMeshObject.hpp"
#include <vector>
#include "imgui.h"
#include <AxoPlotl/utils/commons.hpp>
#include <AxoPlotl/Application.hpp>
#include <AxoPlotl/rendering/detail/create_static_render_data.hpp>

namespace AxoPlotl
{

void OpenMeshObject::render_ui_settings()
{
    ImGui::Checkbox("V", &renderer_.vertices().enabled());
    ImGui::SameLine();
    ImGui::Checkbox("E", &renderer_.edges().enabled());
    ImGui::SameLine();
    ImGui::Checkbox("F", &renderer_.faces().enabled());
    ImGui::SameLine();
    ImGui::Checkbox("C", &renderer_.cells().enabled());


    ImGui::SliderFloat("Point Size", &renderer_.vertices().point_size(), 0.0f, 32.0f);
    ImGui::SliderFloat("Line Width", &renderer_.edges().line_width(), 0.0f, 32.0f);
    ImGui::SliderFloat("Cell Scale", &renderer_.cells().cell_scale(), 0.0f, 1.0f);

    // Clip Box
    // Each entity technically has their own, but we
    // just modify all at once.
    const auto& bbox = bounding_box();
    RendererBase::ClipBox& cb = renderer_.vertices().clip_box();
    bool clip_box_enabled = cb.enabled_;
    if (ImGui::Checkbox("Enable Clip Box", &clip_box_enabled)) {
        cb.set(bbox.min(),bbox.max());
    }
    cb.enabled_ = clip_box_enabled;
    if (clip_box_enabled)
    {
        Vec2f x = {cb.min_[0],cb.max_[0]};
        Vec2f y = {cb.min_[1],cb.max_[1]};
        Vec2f z = {cb.min_[2],cb.max_[2]};
        ImGui::SliderFloat2("x", &x[0], bbox.min()[0], bbox.max()[0]);
        ImGui::SliderFloat2("y", &y[0], bbox.min()[1], bbox.max()[1]);
        ImGui::SliderFloat2("z", &z[0], bbox.min()[2], bbox.max()[2]);
        cb.min_ = {x[0],y[0],z[0]};
        cb.max_ = {x[1],y[1],z[1]};
    }
    renderer_.edges().clip_box() = cb;
    renderer_.faces().clip_box() = cb;
    renderer_.cells().clip_box() = cb;
}

void OpenMeshObject::render_ui_properties()
{
}

void OpenMeshObject::render_ui_info()
{
}

void OpenMeshObject::init()
{
    const auto& data = create_static_render_data(mesh_);
    renderer_.init(scene_->app(), data);
    upload_default_property_data();
    //vertex_vector_renderer_.init(scene_->app(), data.positions_);

    // std::vector<RendererBase::Position> cell_centers;
    // cell_centers.reserve(mesh_.n_cells());
    // for (OVM::CH ch : mesh_.cells()) {
    //     const auto& c = mesh_.barycenter(ch);
    //     cell_centers.emplace_back(c[0],c[1],c[2],1);
    // }
    // cell_vector_renderer_.init(scene_->app(), cell_centers);
}

void OpenMeshObject::render(
    wgpu::RenderPassEncoder _render_pass,
    const Mat4x4f& _view_projection)
{
    if (deleted()) [[unlikely]] {return;}

    const auto& mvp = _view_projection * transform_;

    renderer_.render(
        scene_->app()->scene_viewport(),
        _render_pass,
        mvp);
}

void OpenMeshObject::upload_default_property_data()
{
    using D = RendererBase::Property::Data;
    std::vector<D> props;
    for (uint32_t i = 0; i < mesh_.n_vertices(); ++i) {
        props.push_back(D(0,0,0,1));
    }
    renderer_.vertices().update_property_data(props);

    props.clear();
    for (uint32_t i = 0; i < mesh_.n_edges(); ++i) {
        props.push_back(D(0,0,0,1));
    }
    renderer_.edges().update_property_data(props);

    props.clear();
    for (const auto& f : mesh_.faces()) {
        const auto& n = mesh_.calc_normal(f);
        D sphere_color = Vec4f(
            0.5 * (n[0] + 1),
            0.5 * (n[1] + 1),
            0.5 * (n[2] + 1),
            1
        );
        props.push_back(sphere_color);
    }
    renderer_.faces().update_property_data(props);
}

void OpenMeshObject::recompute_bounding_box()
{
    bbox_.make_empty();
    for (const auto& v : mesh_.vertices()) {
        const auto& p = mesh_.point(v);
        bbox_.expand(Vec3f(p[0],p[1],p[2]));
    }
    bbox_ = bbox_.scaled(1.01f);
}

}
