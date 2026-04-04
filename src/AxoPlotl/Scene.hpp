#pragma once


#include "AxoPlotl/Camera.hpp"
#include "AxoPlotl/IO/file_access.h"
#include "AxoPlotl/objects/BaseObject.hpp"
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
        destroy();
    }

    void init(Application* _app);

    void render(wgpu::RenderPassEncoder _render_pass);

    inline void destroy() {
        destroy_buffer(axis_position_buffer_);
        objects_.clear();
    }

    inline const PerspectiveCamera& perspective() const {
        return perspective_;
    }

    std::shared_ptr<OpenVolumeMeshObject> add_mesh(const std::filesystem::path& _path);

    std::shared_ptr<OpenVolumeMeshObject> add_mesh(const OVMVolumeMesh&& _mesh);

    template<typename Object, typename ...Args>
    std::shared_ptr<Object> add_object(Args... _args)
    {
        objects_.push_back(std::make_shared<Object>(this, _args...));
        objects_.back()->init_buffers();
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
