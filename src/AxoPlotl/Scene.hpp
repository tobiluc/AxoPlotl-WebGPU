#pragma once


#include "AxoPlotl/Camera.hpp"
#include "AxoPlotl/IO/file_access.h"
#include "AxoPlotl/objects/BaseObject.hpp"
#include "AxoPlotl/objects/VolumeMeshObject.hpp"
#include "AxoPlotl/rendering/detail/redraw.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include <filesystem>
namespace AxoPlotl
{

class Application;

class Scene
{
public:

    Scene() {};

    void init(Application* _app, VolumeMeshRenderer::Context _render_context);

    void render(wgpu::RenderPassEncoder _render_pass);

    inline void add_mesh(const std::filesystem::path& _path)
    {
        std::cout << "Loading from " << _path << "..." << std::endl;
        auto opt = IO::read_mesh(_path);
        if (!opt.has_value()) {return;}
        if (std::holds_alternative<VolumeMesh>(opt.value())) {
            add_object<VolumeMeshObject>(std::move(std::get<VolumeMesh>(opt.value())),_path);
        } else if (std::holds_alternative<SurfaceMesh>(opt.value())) {
            add_object<VolumeMeshObject>(std::move(
            volume_mesh(std::get<SurfaceMesh>(opt.value()))),_path);
        }
    }

    template<typename Object, typename ...Args>
    void add_object(Args... _args)
    {
        objects_.push_back(std::make_unique<Object>(_args...));
        objects_.back()->init_renderer(render_context_);
        objects_.back()->recompute_bounding_box();
        trigger_redraw();
    }

    const std::vector<std::unique_ptr<ObjectBase>>& get_objects() const {
        return objects_;
    }

protected:
    Application* app_ = nullptr;
    VolumeMeshRenderer::Context render_context_;
    std::vector<std::unique_ptr<ObjectBase>> objects_;
    VolumeMeshRenderer gizmo_renderer;

    PerspectiveCamera perspective_;
};

}
