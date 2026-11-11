#pragma once

#include <AxoPlotl/core/AxoPlotl_fwd.hpp>

namespace AxoPlotl::internal
{

int draw_histogram(const HistogramBase* _hist,
    int b_begin, int b_end,
    const ColorMap& _cm);

int draw_histogram(const HistogramBase* _hist, const ColorMap& _cm);

}
