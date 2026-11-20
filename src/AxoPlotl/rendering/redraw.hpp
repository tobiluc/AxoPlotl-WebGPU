#pragma once

#include <cstdint>

namespace AxoPlotl::internal
{

void trigger_redraw(uint32_t _n_frames = 10);

bool should_redraw();

uint32_t n_draw_frames();

}
