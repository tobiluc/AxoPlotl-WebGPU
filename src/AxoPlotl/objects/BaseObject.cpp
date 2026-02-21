#include "BaseObject.hpp"

namespace AxoPlotl
{

int ObjectBase::id_counter_ = 0;

void ObjectBase::render(
    wgpu::RenderPassEncoder _render_pass,
    const Mat4x4f& _view_projection)
{
    renderer_.render(_render_pass, _view_projection * transform_);
}

}
