#pragma once


#include "AxoPlotl/Camera.hpp"
#include "AxoPlotl/IO/file_access.h"
#include "AxoPlotl/objects/BaseObject.hpp"
#include "AxoPlotl/objects/VolumeMeshObject.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include <filesystem>
namespace AxoPlotl
{

class Scene
{
public:
    // struct PerspectiveCamera {
    //     double t = 0;
    //     float fov_ = 0.25*M_PI;
    //     float near_ = 0.01;
    //     float far_ = 4096.0f;
    //     Vec3f position_ = {5,2,0};
    //     inline Mat4x4f projection(float _aspect_ratio) const {
    //         return glm::perspective(fov_, _aspect_ratio, near_, far_);
    //     }
    //     inline Mat4x4f view() {
    //         t += 0.005;
    //         position_ = {0.2*std::cos(t), 0.4, 0.2*std::sin(t)};
    //         Vec3f up = {0,1,0};
    //         Vec3f orbit_center = {0,0,0};
    //         return glm::lookAt(position_, orbit_center, up);
    //     }

    // } perspective_;
    PerspectiveCamera perspective_;

    Scene() {};

    void init(VolumeMeshRenderer::Context _render_context);

    void render(GLFWwindow* _window, wgpu::RenderPassEncoder _render_pass);

    inline void add_mesh(const std::filesystem::path& _path)
    {
        std::cout << "Loading from " << _path << "..." << std::endl;
        auto opt = IO::read_mesh(_path);
        if (!opt.has_value()) {return;}
        if (std::holds_alternative<VolumeMesh>(opt.value())) {
            add_object<VolumeMeshObject>(std::move(std::get<VolumeMesh>(opt.value())));
        } else if (std::holds_alternative<SurfaceMesh>(opt.value())) {
            add_object<VolumeMeshObject>(std::move(
            volume_mesh(std::get<SurfaceMesh>(opt.value()))));
        }
    }

    template<typename Object, typename ...Args>
    void add_object(Args... _args)
    {
        objects_.push_back(std::make_unique<Object>(_args...));
        objects_.back()->init_renderer(render_context_);
        objects_.back()->recompute_bounding_box();
    }

protected:
    VolumeMeshRenderer::Context render_context_;
    std::vector<std::unique_ptr<BaseObject>> objects_;
    VolumeMeshRenderer gizmo_renderer;
};

}
