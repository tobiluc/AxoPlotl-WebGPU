#include "Scene.hpp"
#include "AxoPlotl/rendering/detail/create_static_render_data.hpp"
#include <AxoPlotl/Application.hpp>

namespace AxoPlotl
{

void Scene::init(Application *_app)
{
    app_ = _app;

    // Initialize the Coordinate Axes Cross
    using D = MeshEdgeRenderer::Property::Data;
    axis_position_buffer_ = create_position_buffer(
    app_->device_, {
        Vec4f(0,0,0,1),Vec4f(1,0,0,1),Vec4f(0,1,0,1),Vec4f(0,0,1,1)
    });
    axis_renderer_.init(app_, axis_position_buffer_,
        {{0,1},{0,2},{0,3}});
    axis_renderer_.update_property_data(
        std::vector<D>{
            D(1,0,0,1),
            D(0,1,0,1),
            D(0,0,1,1)
    });
}

void Scene::render(wgpu::RenderPassEncoder _render_pass)
{
    // Get Width and Height
    auto viewport = app_->scene_viewport();
    perspective_.update(app_->window());
    const Mat4x4f view_projection =
        perspective_.getProjectionMatrix(viewport[2]/viewport[3]) * perspective_.getViewMatrix();

    axis_renderer_.render(app_->scene_viewport(), _render_pass, view_projection);

    for (const auto& obj : objects_) {
        obj->render(_render_pass, view_projection);
    }

    // Remove deleted objects
    // This is deferred to avoid buffers being destroyed
    // before the command buffer is submitted
    app_->add_deferred_call([this]() {
        objects_.erase(
            std::remove_if(objects_.begin(), objects_.end(), [&](const std::shared_ptr<ObjectBase>& _obj) {
                return _obj->deleted();
            }), objects_.end());
        objects_.shrink_to_fit();
    });
}

}
