#pragma once
#include <AxoPlotl/core/AxoPlotl_fwd.hpp>

namespace AxoPlotl
{

struct DrawRangeSliderOptions
{
    float min = 0;
    float max = 1000;
    bool integer = false;
    const internal::ColorMap* colormap = nullptr;
};

bool draw_range_sliders(
    float& _l, float& _r,
    const DrawRangeSliderOptions& _opt = DrawRangeSliderOptions());

}
