#pragma once


#include "AxoPlotl/objects/BaseObject.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
namespace AxoPlotl
{

class Scene
{
public:
    struct PerspectiveCamera {
        double t = 0;
        float fov_ = 0.25*M_PI;
        float near_ = 0.01;
        float far_ = 4096.0f;
        Vec3f position_ = {5,2,0};
        inline Mat4x4f projection() const {
            return glm::perspective(fov_, 640.0f/480.0f, near_, far_);
        }
        inline Mat4x4f view() {
            t += 0.005;
            position_ = {0.2*std::cos(t), 0.4, 0.2*std::sin(t)};
            Vec3f up = {0,1,0};
            Vec3f orbit_center = {0,0,0};
            return glm::lookAt(position_, orbit_center, up);
        }

    } perspective_;

    Scene() {};

    void init(VolumeMeshRenderer::Context _render_context);

    void render(wgpu::RenderPassEncoder _render_pass);

    template<typename Object, typename ...Args>
    void add_object(Args... _args)
    {
        int id = objects_.size()+1;
        objects_.push_back(std::make_unique<Object>(id, _args...));
        objects_.back()->init_renderer(render_context_);
    }

protected:
    VolumeMeshRenderer::Context render_context_;
    std::vector<std::unique_ptr<BaseObject>> objects_;
    VolumeMeshRenderer gizmo_renderer;
};

}
