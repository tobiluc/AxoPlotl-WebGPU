#include "BaseObject.hpp"
#include "AxoPlotl/Application.hpp"
#include "AxoPlotl/Scene.hpp"

namespace AxoPlotl
{

int ObjectBase::id_counter_ = 0;

void ObjectBase::render(
    wgpu::RenderPassEncoder _render_pass,
    const Mat4x4f& _view_projection)
{
    renderer_.render(scene_->app()->scene_viewport(),
                    _render_pass,
                    _view_projection * transform_);
}

}
