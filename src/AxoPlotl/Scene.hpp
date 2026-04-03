#pragma once


#include "AxoPlotl/Camera.hpp"
#include "AxoPlotl/IO/file_access.h"
#include "AxoPlotl/objects/BaseObject.hpp"
#include "AxoPlotl/objects/OpenMeshObject.hpp"
#include "AxoPlotl/objects/OpenVolumeMeshObject.hpp"
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

    inline std::shared_ptr<ObjectBase> add_mesh(const std::filesystem::path& _path)
    {
        auto opt = IO::read_mesh(_path);
        if (!opt.has_value()) {return nullptr;}
        if (std::holds_alternative<OVMVolumeMesh>(opt.value())) {
            return add_object<OpenVolumeMeshObject>(std::move(std::get<OVMVolumeMesh>(opt.value())),_path);
        } else if (std::holds_alternative<OMSurfaceMesh>(opt.value())) {
            //add_object<OpenMeshObject>(std::move(std::get<OMSurfaceMesh>(opt.value())),_path);
        } else if (std::holds_alternative<SurfaceMesh>(opt.value())) {
            return add_object<OpenVolumeMeshObject>(std::move(
                volume_mesh(std::get<SurfaceMesh>(opt.value()))),_path);
        }
        return nullptr;
    }

    inline std::shared_ptr<OpenVolumeMeshObject> add_openvolumemesh(const std::filesystem::path& _path)
    {
        auto opt = IO::read_mesh(_path);
        if (!opt.has_value()) {return nullptr;}
        if (std::holds_alternative<OVMVolumeMesh>(opt.value())) {
            return add_object<OpenVolumeMeshObject>(std::move(std::get<OVMVolumeMesh>(opt.value())),_path);
        }
        return nullptr;
    }

    template<typename Object, typename ...Args>
    std::shared_ptr<Object> add_object(Args... _args)
    {
        objects_.push_back(std::make_shared<Object>(this, _args...));
        objects_.back()->init_gpu_buffers();
        objects_.back()->recompute_bounding_box();
        zoom_to_box(objects_.back()->bounding_box());
        return std::static_pointer_cast<Object>(objects_.back());
    }

    inline const std::vector<std::shared_ptr<ObjectBase>>& get_objects() const {
        return objects_;
    }

    inline std::shared_ptr<ObjectBase> get_object(int _id) const {
        for (auto obj : objects_) {
            if (!obj->deleted() && obj->id() == _id) {
                return obj;
            }
        }
        return nullptr;
    }

    template <typename Object>
    inline std::shared_ptr<Object> get_object(int _id) const {
        for (auto const& obj : objects_) {
            if (!obj->deleted() && obj->id() == _id) {
                return std::dynamic_pointer_cast<Object>(obj);
            }
        }
        return nullptr;
    }

    inline void sort_objects(
        std::function<bool(
            const std::shared_ptr<ObjectBase>&,
            const std::shared_ptr<ObjectBase>&)> _comp)
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

    inline bool& axis_cross_enabled() {
        return axis_renderer_.enabled();
    }

protected:
    Application* app_ = nullptr;
    std::vector<std::shared_ptr<ObjectBase>> objects_;
    wgpu::Buffer axis_position_buffer_;
    ColoredEdgePropertyRenderer axis_renderer_;

    PerspectiveCamera perspective_;
};

}
