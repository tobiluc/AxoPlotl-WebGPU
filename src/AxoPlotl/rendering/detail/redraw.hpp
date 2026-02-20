#pragma once

#include <cstdint>

namespace AxoPlotl
{

void trigger_redraw(uint32_t _n_frames = UINT32_MAX); // = 2

bool on_draw();

uint32_t n_draw_frames();

}
