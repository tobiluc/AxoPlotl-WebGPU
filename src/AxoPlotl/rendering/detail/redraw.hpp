#pragma once

#include <cstdint>

namespace AxoPlotl
{

void trigger_redraw(uint32_t _n_frames = 10);

bool on_draw();

uint32_t n_draw_frames();

}
