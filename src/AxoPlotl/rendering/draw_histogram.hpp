#pragma once

#include <AxoPlotl/AxoPlotl_fwd.hpp>

namespace AxoPlotl
{

int draw_histogram(const HistogramBase* _hist,
    int b_begin, int b_end,
    const ColorMap& _cm);

int draw_histogram(const HistogramBase* _hist, const ColorMap& _cm);

}
