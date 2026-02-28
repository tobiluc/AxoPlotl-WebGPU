#pragma once
#include "AxoPlotl/AxoPlotl_fwd.hpp"
#include "AxoPlotl/typedefs/glm.hpp"
#include <array>
#include <cstring>
#include <string>

namespace AxoPlotl
{

class PlottingTool
{
public:
    PlottingTool() {
        // strncpy(u_input_buffer_, "", sizeof(u_input_buffer_)-1);
        // input_buffer_[sizeof(input_buffer_)-1] = '\0';
        // for (int i = 0; i < sizeof(samples_); ++i) {samples_[i] = 0;}

    }

    void render_ui(Application& app);

private:
    struct Input
    {
        std::string x_;
        std::string y_;
        std::string z_;
        Vec2f u_ = {-1,1};
        Vec2f v_ = {-1,1};
        int resolution_ = 16;
    } input_;
    std::unordered_map<int,Input> objects_;
    int selected_id_ = -1;
};

}
