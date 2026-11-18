#pragma once
#include <AxoPlotl/typedefs/glm.hpp>
#include <AxoPlotl/IO/json_glm.hpp>

namespace AxoPlotl
{

struct DirectionalLight
{
    alignas(16) Vec3 color = {1,1,1};
    alignas(16) Vec3 direction = {-0.1,-1,-1};
    alignas(16) float strength = 0.6f;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(DirectionalLight,
       color, direction, strength
       );
};

struct PointLight
{
    alignas(16) Vec3 color = {1,1,1};
    alignas(16) Vec3 position = {0,10,0};
    alignas(16) float strength = 0.6f;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(PointLight,
       color, position, strength
       );
};

}
