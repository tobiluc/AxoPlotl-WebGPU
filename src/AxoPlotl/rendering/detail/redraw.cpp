#include "redraw.hpp"

static uint32_t n_frames_ = 10;

void AxoPlotl::trigger_redraw(uint32_t _n_frames)
{
    n_frames_ = _n_frames;
}

bool AxoPlotl::on_draw()
{
    n_frames_ -= (n_frames_>0);
    return (n_frames_>0);
}

uint32_t AxoPlotl::n_draw_frames()
{
    return n_frames_;
}
