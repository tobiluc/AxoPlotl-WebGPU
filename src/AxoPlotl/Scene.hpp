#pragma once


#include "AxoPlotl/Camera.hpp"
#include "AxoPlotl/IO/file_access.h"
#include "AxoPlotl/objects/BaseObject.hpp"
#include "AxoPlotl/objects/VolumeMeshObject.hpp"
#include "AxoPlotl/rendering/MeshVertexRenderer.hpp"
#include "AxoPlotl/rendering/VectorRenderer.hpp"
#include "AxoPlotl/rendering/detail/redraw.hpp"
#include <filesystem>
#include <AxoPlotl/AxoPlotl_fwd.hpp>

namespace AxoPlotl
{

class Scene
{
public:

    Scene() {};

    ~Scene() {
        axis_position_buffer_.destroy();
        axis_position_buffer_.release();
    }

    void init(Application* _app);

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
        objects_.push_back(std::make_unique<Object>(this, _args...));
        objects_.back()->init();
        objects_.back()->recompute_bounding_box();
        zoom_to_box(objects_.back()->bounding_box());
    }

    inline const std::vector<std::unique_ptr<ObjectBase>>& get_objects() const {
        return objects_;
    }

    inline void sort_objects(
        std::function<bool(
            const std::unique_ptr<ObjectBase>&,
            const std::unique_ptr<ObjectBase>&)> _comp)
    {
        std::sort(objects_.begin(), objects_.end(), _comp);
    }

    inline void zoom_to_box(const ToLoG::AABB<Vec3f>& _bbox) {
        perspective_.zoom_to_box(_bbox.min(),_bbox.max());
        trigger_redraw();
    }

    Application* app() {
        return app_;
    }

protected:
    Application* app_ = nullptr;
    std::vector<std::unique_ptr<ObjectBase>> objects_;
    wgpu::Buffer axis_position_buffer_;
    MeshEdgeRenderer axis_renderer_;

    PerspectiveCamera perspective_;
};

}
