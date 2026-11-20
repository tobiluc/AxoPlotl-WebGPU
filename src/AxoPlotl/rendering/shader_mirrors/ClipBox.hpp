#pragma once
#include <AxoPlotl/typedefs/glm.hpp>

/**
 * Mirrors assets/shaders/includes/clipping.wgsl
 */

namespace AxoPlotl::internal
{

struct alignas(16) ClipBox {
    alignas(16) Vec3 min_;
    alignas(16) Vec3 max_;
    alignas(16) int32_t enabled_ = false;
};
static_assert(sizeof(ClipBox)==3*16);
static_assert(sizeof(ClipBox)%16==0);
static_assert(alignof(ClipBox) == 16);

}
