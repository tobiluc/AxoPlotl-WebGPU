#include "Scene.hpp"
#include <AxoPlotl/Application.hpp>

namespace AxoPlotl
{

void Scene::init(Application *_app)
{
    app_ = _app;

    // Initialize the Coordinate Axes Cross
    VolumeMeshRenderer::StaticData data;
    data.positions_ = {
        Vec4f(0,0,0,1),Vec4f(1,0,0,1),Vec4f(0,1,0,1),Vec4f(0,0,1,1)
    };
    data.vertex_instances_ = {0,1,2,3};
    data.edge_instances_.push_back({.vh0_=0,.vh1_=1,.eh_=0});
    data.edge_instances_.push_back({.vh0_=0,.vh1_=2,.eh_=1});
    data.edge_instances_.push_back({.vh0_=0,.vh1_=3,.eh_=2});
    gizmo_renderer.init(_app, data);

    std::vector<VolumeMeshRenderer::Property::Data> e_props;
    e_props.push_back({.value_ = {1,0,0,1}});
    e_props.push_back({.value_ = {0,1,0,1}});
    e_props.push_back({.value_ = {0,0,1,1}});
    gizmo_renderer.update_edge_property_data(e_props);
}

void Scene::render(wgpu::RenderPassEncoder _render_pass)
{
    // Get Width and Height
    auto viewport = app_->scene_viewport();
    perspective_.update(app_->window());
    const Mat4x4f view_projection =
        perspective_.getProjectionMatrix(viewport[2]/viewport[3]) * perspective_.getViewMatrix();

    gizmo_renderer.render(app_->scene_viewport(), _render_pass, view_projection);

    for (const auto& obj : objects_) {
        obj->render(_render_pass, view_projection);
    }

    // Remove deleted objects
    // This is deferred to avoid buffers being destroyed
    // before the command buffer is submitted
    app_->deferred_calls_.push_back([this]() {
        objects_.erase(
            std::remove_if(objects_.begin(), objects_.end(), [&](const std::unique_ptr<ObjectBase>& _obj) {
                return _obj->deleted();
            }), objects_.end());
        objects_.shrink_to_fit();
    });
}

}
