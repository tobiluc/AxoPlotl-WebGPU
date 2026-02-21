#pragma once
#include "AxoPlotl/rendering/VolumeMeshRenderer.hpp"
#include "ToLoG/Core.hpp"

namespace AxoPlotl
{

class Scene;

class ObjectBase
{
public:
    ObjectBase(Scene* _scene, std::string _name) :
        id_(++id_counter_),
        scene_(_scene),
        name_(_name),
        renderer_(),
        transform_(1)
    {}

    virtual ~ObjectBase() = default;

    virtual void init() = 0;

    virtual void recompute_bounding_box() = 0;

    void render(
        wgpu::RenderPassEncoder _render_pass,
        const Mat4x4f& _view_projection);

    virtual void render_ui() = 0;

    inline const ToLoG::AABB<Vec3f>& bounding_box() const {
        return bbox_;
    }

    inline bool& visible() {
        return renderer_.render_anything_;
    }

    inline bool& selected() {
        return selected_;
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

protected:
    Scene* scene_;
    int id_;
    std::string name_;
    VolumeMeshRenderer renderer_;
    Mat4x4f transform_;
    static int id_counter_;
    ToLoG::AABB<Vec3f> bbox_;
    bool deleted_ = false;
    bool selected_ = false;
};

};
