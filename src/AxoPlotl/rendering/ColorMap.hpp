#pragma once

#include <AxoPlotl/typedefs/glm.hpp>
#include <string>
#include <vector>

namespace AxoPlotl
{

class ColorMap {
public:
    ColorMap() {}

    void set_gradient(const std::vector<Vec3f>& _colors, int _N = 256);

    void set_gradient(const Vec3f& _a, const Vec3f& _b, int _N);

    void set_viridis(int N);

    void set_magma(int N);

    void set_inferno(int N);

    void set_plasma(int N);

    void set_rd_bu(int N);

    void set_coolwarm(int N);

    void update(const std::vector<float>& _data);

    std::string name_ = "";
};

}
