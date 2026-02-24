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
    std::string x_input_;
    std::string y_input_;
    std::string z_input_;
    // char u_input_buffer_[1024];
    // char v_input_buffer_[1024];
    Vec2f u_range_ = {0,1};
    Vec2f v_range_ = {0,1};
    int resolution_ = 10;
};

}
