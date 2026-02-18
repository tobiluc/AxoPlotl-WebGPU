#include "Scene.hpp"
#include "AxoPlotl/input/Mouse.hpp"
#include "AxoPlotl/objects/VolumeMeshObject.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"

namespace AxoPlotl
{

void Scene::init(VolumeMeshRenderer::Context _render_context)
{
    render_context_ = _render_context;

    // Initialize the Coordinate Axes Cross
    VolumeMeshRenderer::StaticData data;
    data.positions_ = {
        Vec4f(0,0,0,1),Vec4f(1,0,0,1),Vec4f(0,2,0,1),Vec4f(0,0,3,1)
    };
    data.vertex_draw_indices_ = {0,1,2,3};
    data.edge_draw_indices_.push_back({.vertex_index_=0,.edge_index_=0});
    data.edge_draw_indices_.push_back({.vertex_index_=1,.edge_index_=0});
    data.edge_draw_indices_.push_back({.vertex_index_=0,.edge_index_=1});
    data.edge_draw_indices_.push_back({.vertex_index_=2,.edge_index_=1});
    data.edge_draw_indices_.push_back({.vertex_index_=0,.edge_index_=2});
    data.edge_draw_indices_.push_back({.vertex_index_=3,.edge_index_=2});
    gizmo_renderer.init(_render_context, data);

    std::vector<VolumeMeshRenderer::PropertyData> e_props;
    e_props.push_back({.color_ = {1,0,0,1}});
    e_props.push_back({.color_ = {0,1,0,1}});
    e_props.push_back({.color_ = {0,0,1,1}});
    gizmo_renderer.update_edge_property_data(e_props);
}

void Scene::render(GLFWwindow *_window, wgpu::RenderPassEncoder _render_pass)
{
    // Get Width and Height
    int w, h;
    glfwGetWindowSize(_window, &w, &h);

    perspective_.update(_window);
    const Mat4x4f view_projection = perspective_.getProjectionMatrix((float)w/(float)h) * perspective_.getViewMatrix();

    gizmo_renderer.render(_render_pass, view_projection);

    for (const auto& obj : objects_) {
        obj->render(_render_pass, view_projection);
    }
}

}
