#pragma once
#include "AxoPlotl/rendering/VolumeMeshRenderer.hpp"
#include "ToLoG/Core.hpp"

namespace AxoPlotl
{

class BaseObject
{
public:
    BaseObject(std::string _name) :
        id_(++id_counter_),
        name_(_name),
        renderer_(),
        transform_(1)
    {}

    virtual ~BaseObject() = default;

    virtual void init_renderer(VolumeMeshRenderer::Context _context) = 0;

    virtual void recompute_bounding_box() = 0;

    inline void render(wgpu::RenderPassEncoder _render_pass, const Mat4x4f& _view_projection) {
        renderer_.render(_render_pass, _view_projection * transform_);
    }

    virtual void render_ui() = 0;

    inline const ToLoG::AABB<ToLoG::Point<float,3>>& bounding_box() const
    {
        return bbox_;
    }

protected:
    int id_;
    std::string name_;
    VolumeMeshRenderer renderer_;
    Mat4x4f transform_;
    static int id_counter_;
    ToLoG::AABB<ToLoG::Point<float,3>> bbox_;
};

};
