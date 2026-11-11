#pragma once
#include <AxoPlotl/core/AxoPlotl_fwd.hpp>

namespace AxoPlotl
{

bool draw_range_sliders(
    float& _l, float& _r,
    const float& _min, const float& _max,
    const internal::ColorMap& _cm);

}
