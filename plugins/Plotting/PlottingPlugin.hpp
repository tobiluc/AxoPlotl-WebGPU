#pragma once
#include "AxoPlotl/PluginBase.hpp"

namespace AxoPlotl
{

class PlottingPlugin : public PluginBase
{
public:
    PlottingPlugin() {
        strncpy(input_buffer_, "", sizeof(input_buffer_)-1);
        input_buffer_[sizeof(input_buffer_)-1] = '\0';
        for (int i = 0; i < sizeof(samples_); ++i) {samples_[i] = 0;}
    }

    void render_ui(Application& app) override;

    const char* name() const override {
        return "Plotting Plugin";
    }

private:
    char input_buffer_[1024];
    float samples_[100];
};

}
