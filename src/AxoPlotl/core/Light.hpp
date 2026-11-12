#pragma once

#include "AxoPlotl/typedefs/glm.hpp"
namespace AxoPlotl
{

struct DirectionalLight
{
    alignas(16) Vec3f color = {1,1,1};
    alignas(16) Vec3f direction = {-1,-1,-1};
    alignas(16) float strength = 0.1f;
};

}
