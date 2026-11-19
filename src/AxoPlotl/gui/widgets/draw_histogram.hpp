#pragma once

#include <AxoPlotl/core/AxoPlotl_fwd.hpp>

namespace AxoPlotl::internal
{

struct DrawHistogramOptions
{
    const ColorMap* colormap = nullptr;
    int b_begin = -1000000;
    int b_end = 1000000;
    bool pm_buttons = false;
};

int draw_histogram(
    HistogramBase& _hist,
    const DrawHistogramOptions& _opt);

}
