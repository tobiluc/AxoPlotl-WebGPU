#pragma once
#include "AxoPlotl/AxoPlotl_fwd.hpp"
#include "AxoPlotl/typedefs/glm.hpp"
#include <array>
#include <cstring>
#include <string>
#include <unordered_map>

namespace AxoPlotl
{

class PlottingTool
{
private:
    PlottingTool() {}
public:

    static void render_ui();

private:
    struct Input
    {
        char x_[1024];
        char y_[1024];
        char z_[1024];
        Vec2f u_ = {-1,1};
        Vec2f v_ = {-1,1};
        int resolution_ = 16;
    };
    static Input input_;
    static std::unordered_map<int,Input> objects_;
    static int selected_id_;
};

}
