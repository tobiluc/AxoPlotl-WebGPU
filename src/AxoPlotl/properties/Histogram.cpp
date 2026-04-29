#include <AxoPlotl/properties/Histogram.hpp>
#include <AxoPlotl/rendering/draw_histogram.hpp>

namespace AxoPlotl
{

int HistogramBase::render_ui(
    int b_begin, int b_end,
    const ColorMap& _cm)
{
    if (!render_ui_begin()) {return -1;}

    return draw_histogram(this, b_begin, b_end, _cm);
}

}
