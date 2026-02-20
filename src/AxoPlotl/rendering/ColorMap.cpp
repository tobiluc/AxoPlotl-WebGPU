#include "ColorMap.hpp"

namespace AxoPlotl
{

void ColorMap::set_gradient(const std::vector<Vec3f>& _colors, int N)
{
    std::vector<float> data;
    data.reserve(N * 4);

    for (int i = 0; i < N; ++i)
    {
        float t = float(i) / float(N - 1);

        float scaled = t * (_colors.size() - 1);
        int idx0 = std::min(int(std::floor(scaled)), int(_colors.size() - 2));
        int idx1 = idx0 + 1;
        float local_t = scaled - idx0;

        const Vec3f &c0 = _colors[idx0];
        const Vec3f &c1 = _colors[idx1];

        float r = c0.r + local_t * (c1.r - c0.r);
        float g = c0.g + local_t * (c1.g - c0.g);
        float b = c0.b + local_t * (c1.b - c0.b);

        data.push_back(r);
        data.push_back(g);
        data.push_back(b);
        data.push_back(1.0f);
    }

    update(data);
}

void ColorMap::set_gradient(const Vec3f& _a, const Vec3f& _b, int _N)
{
    set_gradient({_a, _b}, _N);
}

void ColorMap::set_viridis(int N)
{
    static const std::vector<Vec3f> viridis = {
        {0.267004f, 0.004874f, 0.329415f},
        {0.282327f, 0.140926f, 0.457517f},
        {0.253935f, 0.265254f, 0.529983f},
        {0.206756f, 0.371758f, 0.553117f},
        {0.163625f, 0.471133f, 0.558148f},
        {0.127568f, 0.566949f, 0.550556f},
        {0.134692f, 0.658636f, 0.517649f},
        {0.266941f, 0.748751f, 0.440573f},
        {0.477504f, 0.821444f, 0.318195f},
        {0.741388f, 0.873449f, 0.149561f},
        {0.993248f, 0.906157f, 0.143936f}
    };
    set_gradient(viridis, N);
    name_ = "Viridis";
}

void ColorMap::set_plasma(int N)
{
    static const std::vector<Vec3f> plasma = {
        {0.050383f, 0.029803f, 0.527975f},
        {0.336627f, 0.063956f, 0.769953f},
        {0.648925f, 0.133936f, 0.781554f},
        {0.863001f, 0.386567f, 0.764393f},
        {0.975324f, 0.639465f, 0.519905f},
        {0.993248f, 0.906157f, 0.143936f}
    };
    set_gradient(plasma, N);
    name_ = "Plasma";
}

void ColorMap::set_inferno(int N)
{
    static const std::vector<Vec3f> inferno = {
        {0.001462f, 0.000466f, 0.013866f},
        {0.200403f, 0.018246f, 0.337641f},
        {0.468160f, 0.140614f, 0.493067f},
        {0.772053f, 0.313162f, 0.392499f},
        {0.940015f, 0.631844f, 0.152819f},
        {0.987053f, 0.991438f, 0.749504f}
    };
    set_gradient(inferno, N);
    name_ = "Inferno";
}

void ColorMap::set_magma(int N)
{
    static const std::vector<Vec3f> magma = {
        {0.001462f, 0.000466f, 0.013866f},
        {0.267004f, 0.004874f, 0.329415f},
        {0.477504f, 0.064254f, 0.557964f},
        {0.741388f, 0.214372f, 0.497832f},
        {0.965960f, 0.618919f, 0.319167f},
        {0.987053f, 0.991438f, 0.749504f}
    };
    set_gradient(magma, N);
    name_ = "Magma";
}

void ColorMap::set_rd_bu(int N)
{
    static const std::vector<Vec3f> rd_bu = {
        {0.403921f, 0.000000f, 0.121569f},
        {0.698039f, 0.094118f, 0.168627f},
        {0.870588f, 0.619608f, 0.592157f},
        {1.0f, 1.0f, 1.0f},
        {0.619608f, 0.792157f, 0.870588f},
        {0.098039f, 0.396078f, 0.686275f},
        {0.0f, 0.003922f, 0.258824f}
    };
    set_gradient(rd_bu, N);
    name_ = "RdBu";
}

void ColorMap::set_coolwarm(int N)
{
    static const std::vector<Vec3f> coolwarm = {
        {0.229805f, 0.298717f, 0.753683f}, // blue
        {0.541643f, 0.636480f, 0.864786f},
        {0.865003f, 0.865003f, 0.865003f}, // white / center
        {0.991248f, 0.697083f, 0.417556f},
        {0.956871f, 0.211055f, 0.131898f}  // red
    };
    set_gradient(coolwarm, N);
    name_ = "Coolwarm";
}

void ColorMap::update(const std::vector<float>& _data)
{
    int width = _data.size() / 4; // rgba
    static constexpr int height = 1;
    //...
}


}
