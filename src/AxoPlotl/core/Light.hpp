#pragma once

#include "AxoPlotl/typedefs/glm.hpp"
namespace AxoPlotl
{

struct DirectionalLight
{
    alignas(16) Vec3 color = {1,1,1};
    alignas(16) Vec3 direction = {-0.1,-1,-1};
    alignas(16) float strength = 0.6f;
};

}
