#pragma once
#include <AxoPlotl/core/AxoPlotl_fwd.hpp>

namespace AxoPlotl::gui
{

struct RangeSlidersOptions
{
    float min = 0;
    float max = 1000;
    bool integer = false;
    const internal::ColorMap* colormap = nullptr;
};

bool RangeSliders(
    float& _l, float& _r,
    const RangeSlidersOptions& _opt = RangeSlidersOptions());

}
