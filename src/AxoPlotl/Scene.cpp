#include "Scene.hpp"
#include "AxoPlotl/objects/VolumeMeshObject.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"

namespace AxoPlotl
{

void Scene::init(VolumeMeshRenderer::Context _render_context)
{
    render_context_ = _render_context;

    add_object<VolumeMeshObject>();
}

void Scene::render(wgpu::RenderPassEncoder _render_pass)
{
    float fov = 0.25*M_PI;
    float near = 0.01;
    float far = 4096.0f;
    projection_matrix_ = glm::perspective(fov, 640.0f/480.0f, near, far);
    Vec3f up = {0,1,0};
    Vec3f position = {3,2,10};
    Vec3f orbit_center = {0,0,0};
    view_matrix_ = glm::lookAt(position, orbit_center, up);

    const Mat4x4f view_projection = projection_matrix_ * view_matrix_;

    for (const auto& obj : objects_) {
        obj->render(_render_pass, view_projection);
    }
}

}
