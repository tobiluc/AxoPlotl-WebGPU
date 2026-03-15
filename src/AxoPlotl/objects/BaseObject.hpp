#pragma once
#include "AxoPlotl/typedefs/glm.hpp"
#include "ToLoG/Core.hpp"
#include "webgpu/webgpu.hpp"

namespace AxoPlotl
{

class Scene;

class ObjectBase
{
friend class DebugPlugin;

public:
    ObjectBase(Scene* _scene, std::string _name) :
        id_(++id_counter_),
        scene_(_scene),
        name_(_name),
        transform_(1)
    {}

    virtual ~ObjectBase() = default;

    virtual void init() = 0;

    virtual void recompute_bounding_box() = 0;

    virtual void render(
        wgpu::RenderPassEncoder _render_pass,
        const Mat4x4f& _view_projection) = 0;

    virtual void render_ui_info() = 0;

    virtual void render_ui_settings() = 0;

    virtual void render_ui_properties() = 0;

    inline const ToLoG::AABB<Vec3f>& bounding_box() const {
        return bbox_;
    }

    inline bool& target() {
        return target_;
    }

    inline bool& deleted() {
        return deleted_;
    }

    inline const std::string& name() const {
        return name_;
    }

    inline const int id() const {
        return id_;
    }

    inline bool& visible() {
        return visible_;
    }

protected:
    Scene* scene_;
    int id_;
    std::string name_;
    //std::unique_ptr<RendererBase> renderer_;
    Mat4x4f transform_;
    static int id_counter_;
    ToLoG::AABB<Vec3f> bbox_;
    bool deleted_ = false;
    bool target_ = false;
    bool visible_ = true;
};

};
