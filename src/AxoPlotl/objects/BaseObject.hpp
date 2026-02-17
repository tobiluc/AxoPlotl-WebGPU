#pragma once
#include "AxoPlotl/rendering/VolumeMeshRenderer.hpp"

namespace AxoPlotl
{

class BaseObject
{
public:
    BaseObject(VolumeMeshRenderer::Context _render_context, int _id, std::string _name) :
        id_(_id),
        name_(_name),
        renderer_(_render_context),
        transform_(1)
    {}

    virtual ~BaseObject() = default;

    inline void render(wgpu::RenderPassEncoder _render_pass, const Mat4x4f& _view_projection) {
        renderer_.render(_render_pass, _view_projection * transform_);
    }

protected:
    int id_;
    std::string name_;
    VolumeMeshRenderer renderer_;
    Mat4x4f transform_;
};

};
