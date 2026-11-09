#include "Scene.hpp"
#include "AxoPlotl/IO/file_access.h"
#include "AxoPlotl/objects/SphericalHarmonicsObject.hpp"
#include "AxoPlotl/rendering/detail/create_static_render_data.hpp"
#include <AxoPlotl/Application.hpp>

namespace AxoPlotl
{

void Scene::init(Application *_app)
{
    app_ = _app;

    // Initialize the Coordinate Axes Cross
    using D = ColoredEdgeRenderer::Property::Data;
    axis_position_buffer_ = create_position_buffer(
        app_->device_, {Vec4f(0,0,0,1)});
    axis_renderer_.init(UINT32_MAX, app_, axis_position_buffer_);
    glm::mat3x4 m;
    m[0] = {1,0,0,1};
    m[1] = {0,1,0,1};
    m[2] = {0,0,1,1};
    axis_renderer_.update_matrix_data({m});
    axis_renderer_.draw_symmetric() = false;

    //add_object<SHObject>();
}

std::shared_ptr<OpenVolumeMeshObject> Scene::add_mesh(const OVMVolumeMesh &&_mesh)
{
    return add_object<OpenVolumeMeshObject>(std::move(_mesh));
}

std::shared_ptr<OpenVolumeMeshObject> Scene::add_mesh(const std::filesystem::path& _path)
{
    auto rr = IO::read_mesh(_path);
    if (rr.status_ != IO::ReadMeshStatus::OK) {
        return nullptr;
    }
    return add_object<OpenVolumeMeshObject>(std::move(rr.mesh_), _path);
}

void Scene::render(wgpu::RenderPassEncoder _render_pass)
{
    // Get Width and Height
    const auto& viewport = app_->viewport();
    camera_.update(app_->window(), viewport.hovered());
    const Mat4x4f& view = camera_.get_view_matrix();
    const Mat4x4f& proj = camera_.get_projection_matrix(viewport.size());

    const Mat4x4f view_proj = proj * view;

    // Render Coordinate Axis Cross at Origin
    axis_renderer_.render(viewport.size(),
        _render_pass,
        view_proj);

    // Render Scene Objects
    for (const auto& obj : objects_) {
        obj->render(_render_pass, view_proj);
    }

    // Remove deleted objects
    // This is deferred to avoid buffers being destroyed
    // before the command buffer is submitted
    app_->call_deferred([this]() {
        objects_.erase(
            std::remove_if(objects_.begin(), objects_.end(), [&](const std::shared_ptr<ObjectBase>& _obj) {
                return _obj->deleted();
            }), objects_.end());
        objects_.shrink_to_fit();
    });
}

}
