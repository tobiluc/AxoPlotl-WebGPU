#pragma once

#include <AxoPlotl/rendering/wgpu_commons.hpp>
#include <AxoPlotl/rendering/redraw.hpp>
#include <AxoPlotl/core/objects/BaseObject.hpp>
#include <AxoPlotl/core/Camera.hpp>
#include <AxoPlotl/rendering/renderers/Matrix3x3Renderer.hpp>
#include <webgpu/webgpu.hpp>
#include <AxoPlotl/typedefs/ovm.hpp>
#include <AxoPlotl/core/objects/OpenVolumeMeshObject.hpp>
#include <filesystem>
#include <AxoPlotl/core/Light.hpp>

namespace AxoPlotl::internal
{

class Scene
{
public:

    Scene() {};

    ~Scene() {
        destroy();
    }

    void init(Application* _app);

    void render(wgpu::RenderPassEncoder _render_pass,
                const ViewSize _view_size,
                bool _allow_input = true);

    void render_settings_ui();

    inline void destroy() {
        destroy_buffer(axis_position_buffer_);
        objects_.clear();
    }

    std::shared_ptr<OpenVolumeMeshObject> add_mesh(const std::filesystem::path& _path);

    std::shared_ptr<OpenVolumeMeshObject> add_mesh(const OVMVolumeMesh&& _mesh);

    template<typename Object, typename ...Args>
    std::shared_ptr<Object> add_object(Args... _args)
    {
        objects_.push_back(std::make_shared<Object>(this, _args...));
        objects_.back()->init_buffers();
        objects_.back()->recompute_bounding_box();
        if (settings.zoom_to_new_object) {
            zoom_to_box(objects_.back()->bounding_box());
        }
        return std::static_pointer_cast<Object>(objects_.back());
    }

    inline size_t num_objects() const {
        size_t n(0);
        for (const auto& obj : objects_) {
            n += !obj->deleted();
        }
        return n;
    }

    inline auto get_objects() const {
        return objects_ | std::views::filter([&](const std::shared_ptr<ObjectBase> _obj) {return !_obj->deleted();});;
    }

    inline auto get_target_objects() const {
        return get_objects() | std::views::filter([&](const std::shared_ptr<ObjectBase> _obj) {return _obj->target();});
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

    inline void zoom_to_box(const BoundingBox& _bbox) {
        camera_.zoom_to_box(_bbox.min(),_bbox.max());
        trigger_redraw();
    }

    Application* app() {return app_;}
    const Application* app() const {return app_;}

    Camera& camera() {
        return camera_;
    }

    struct Settings
    {
        bool zoom_to_new_object = true;
        float clear_color[3] = {1,1,1};
        bool show_axis_cross = true;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(Settings,
            zoom_to_new_object,
            clear_color, show_axis_cross);
    } settings;

protected:
    Application* app_ = nullptr;
    std::vector<std::shared_ptr<ObjectBase>> objects_;
    wgpu::Buffer axis_position_buffer_;
    // ColoredEdgeRenderer axis_renderer_;
    Matrix3x3Renderer axis_renderer_;
    Camera camera_;

public:

    DirectionalLight directional_light;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Scene,
        camera_, settings);
};

}
