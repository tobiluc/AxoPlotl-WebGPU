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

    const Mat4x4f view_projection = perspective_.projection() * perspective_.view();


    for (const auto& obj : objects_) {
        obj->render(_render_pass, view_projection);
    }
}

}
