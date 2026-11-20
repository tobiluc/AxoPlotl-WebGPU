#pragma once

#include <AxoPlotl/core/AxoPlotl_fwd.hpp>

namespace AxoPlotl::gui
{

struct HistogramOptions
{
    const internal::ColorMap* colormap = nullptr;
    int b_begin = -1000000;
    int b_end = 1000000;
    bool pm_buttons = false;
};

int Histogram(
    internal::HistogramBase& _hist,
    const HistogramOptions& _opt = HistogramOptions());

}
