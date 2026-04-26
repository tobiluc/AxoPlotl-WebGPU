#include "spherical_harmonics.hpp"
#include <glm/common.hpp>
#include <iostream>
#include <numbers>
#include <ostream>
#include <utility>

namespace AxoPlotl
{

static constexpr sh_real PI = std::numbers::pi_v<sh_real>;
static const sh_real SQRT_PI = std::sqrt(PI);
static const sh_real SQRT_7_12 = std::sqrt(7.0/12.0);
static const sh_real SQRT_5_12 = std::sqrt(5.0/12.0);

// l = 4
// for m=-l to m=l (9 components)
// Y_ml

// https://en.wikipedia.org/wiki/Table_of_spherical_harmonics
// Real spherical harmonics l=4

Vec<sh_real,1> sh_band0(sh_real x, sh_real y, sh_real z)
{
    return {0.282094791774}; // 0.5 * sqrt(1/pi)
}

Vec<sh_real,5> sh_band2(sh_real x, sh_real y, sh_real z)
{
    sh_real n = std::sqrt(x*x + y*y + z*z);
    x /= n; y /= n; z /= n;

    Vec<sh_real,5> Y;
    Y[0] = 1.09254843059 * (x*y);
    Y[1] = -1.09254843059 * (y*z);
    Y[2] = 0.315391565253 * (3*z*z - 1);
    Y[3] = -1.09254843059 * (x*z);
    Y[4] = 0.546274215296 * (x*x - y*y);
    return Y;
}

Vec<sh_real,9> sh_band4(sh_real x, sh_real y, sh_real z)
{
    sh_real n = std::sqrt(x*x + y*y + z*z);
    x /= n; y /= n; z /= n;

    sh_real x2 = x*x, y2 = y*y, z2 = z*z;
    Vec<sh_real,9> Y;
    Y[0] =  2.5033429417967046 * x*y*(x2 - y2);
    Y[1] = -1.7701307697799304 * y*z*(3*x2 - y2);
    Y[2] =  0.9461746957575601 * x*y*(7*z2 - 1);
    Y[3] = -0.6690465435572892 * y*z*(7*z2 - 3);
    Y[4] =  0.10578554691520431 * (35*z2*z2 - 30*z2 + 3);
    Y[5] = -0.6690465435572892 * x*z*(7*z2 - 3);
    Y[6] =  0.47308734787878004 * (x2 - y2)*(7*z2 - 1);
    Y[7] = -1.7701307697799304 * x*z*(x2 - 3*y2);
    Y[8] =  0.6258357354491761 * (x2*(x2 - 3*y2) - y2*(3*x2 - y2));
    return Y;
}

Vec<sh_real,15> sh_band024(sh_real x, sh_real y, sh_real z)
{
    const auto b0 = sh_band0(x, y, z);
    const auto b2 = sh_band2(x, y, z);
    const auto b4 = sh_band4(x, y, z);
    return {
        b0[0],
        b2[0],b2[1],b2[2],b2[3],b2[4],
        b4[0],b4[1],b4[2],b4[3],b4[4],b4[5],b4[6],b4[7],b4[8]
    };
}

Vec<sh_real,9> canonical_octa_frame(sh_real _lx, sh_real _ly, sh_real _lz)
{
    return ((SH_BAND4_X * _lx*_lx)
        + (SH_BAND4_Y * _ly*_ly)
        + (SH_BAND4_Z * _lz*_lz)).normalized();
}

Vec<sh_real,15> canonical_odeco_frame(sh_real _lx, sh_real _ly, sh_real _lz)
{
    return ((SH_BAND024_X * _lx*_lx)
            + (SH_BAND024_Y * _ly*_ly)
            + (SH_BAND024_Z * _lz*_lz)).normalized();
}

}
