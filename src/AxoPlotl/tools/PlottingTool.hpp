#pragma once
#include "AxoPlotl/AxoPlotl_fwd.hpp"
#include <cstring>

namespace AxoPlotl
{

class PlottingTool
{
public:
    PlottingTool() {
        strncpy(input_buffer_, "", sizeof(input_buffer_)-1);
        input_buffer_[sizeof(input_buffer_)-1] = '\0';
        for (int i = 0; i < sizeof(samples_); ++i) {samples_[i] = 0;}
    }

    void render_ui(Application& app);

private:
    char input_buffer_[1024];
    float samples_[100];
};

}
