#pragma once


#include "AxoPlotl/objects/BaseObject.hpp"
namespace AxoPlotl
{

class Scene
{
public:
    Scene() {};

    void init(VolumeMeshRenderer::Context _render_context);

    void render(wgpu::RenderPassEncoder _render_pass);

    template<typename Object, typename ...Args>
    void add_object(Args... _args)
    {
        int id = objects_.size()+1;
        objects_.push_back(std::make_unique<Object>(render_context_, id, _args...));
    }

protected:
    VolumeMeshRenderer::Context render_context_;
    std::vector<std::unique_ptr<BaseObject>> objects_;
    Mat4x4f projection_matrix_;
    Mat4x4f view_matrix_;
};

}
