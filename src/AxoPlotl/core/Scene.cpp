#include "Scene.hpp"
#include "AxoPlotl/IO/file_access.hpp"
#include "AxoPlotl/core/objects/SphericalHarmonicsObject.hpp"
#include "AxoPlotl/rendering/create_static_render_data.hpp"
#include <AxoPlotl/core/Application.hpp>

namespace AxoPlotl::internal
{

void Scene::init(Application *_app)
{
    app_ = _app;

    // Initialize the Coordinate Axes Cross
    using D = ColoredEdgeRenderer::Property::Data;
    axis_position_buffer_ = create_position_buffer(
        app_->gpu_context().device, {Vec4f(0,0,0,1)});
    axis_renderer_.init(UINT32_MAX,
                        app()->gpu_context(),
                        app()->render_context(),
                        axis_position_buffer_);
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

void Scene::render(wgpu::RenderPassEncoder _render_pass, const ViewSize _view_size, bool _allow_input)
{
    // Get Width and Height
    camera_.update(app_->window(), _allow_input);
    const Mat4x4f& view = camera_.get_view_matrix();
    const Mat4x4f& proj = camera_.get_projection_matrix(_view_size);

    const Mat4x4f view_proj = proj * view;

    // Render Coordinate Axis Cross at Origin
    axis_renderer_.enabled() = settings.show_axis_cross;
    axis_renderer_.render(_view_size,
        _render_pass,
        view_proj);

    // Render Scene Objects
    for (const auto& obj : objects_) {
        obj->render(_render_pass, _view_size, view_proj);
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

void Scene::render_settings_ui()
{
    if (ImGui::BeginMenu("Camera")) {
        camera().render_settings_ui(app_->window());
        ImGui::EndMenu();
    }

    ImGui::ColorEdit3("Background", settings.clear_color);
    ImGui::Checkbox("Axis Cross", &settings.show_axis_cross);
    ImGui::Checkbox("Zoom to New Object", &settings.zoom_to_new_object);
}

}
