#pragma once

#include <array>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include "AxoPlotl/geometry/Vector.hpp"

namespace AxoPlotl
{

using sh_real = double;

Vec<sh_real,1> sh_band0(sh_real x, sh_real y, sh_real z);

Vec<sh_real,5> sh_band2(sh_real x, sh_real y, sh_real z);

Vec<sh_real,9> sh_band4(sh_real x, sh_real y, sh_real z);

Vec<sh_real,15> sh_band024(sh_real x, sh_real y, sh_real z);

inline static const Vec<sh_real,9> SH_BAND4_X = sh_band4(1,0,0);
inline static const Vec<sh_real,9> SH_BAND4_Y = sh_band4(0,1,0);
inline static const Vec<sh_real,9> SH_BAND4_Z = sh_band4(0,0,1);

inline static const Vec<sh_real,15> SH_BAND024_X = sh_band024(1,0,0);
inline static const Vec<sh_real,15> SH_BAND024_Y = sh_band024(0,1,0);
inline static const Vec<sh_real,15> SH_BAND024_Z = sh_band024(0,0,1);

Vec<sh_real,15> canonical_odeco_frame(sh_real _lx=1, sh_real _ly=1, sh_real _lz=1);

}
