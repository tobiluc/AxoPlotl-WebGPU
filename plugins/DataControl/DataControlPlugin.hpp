#pragma once
#include "AxoPlotl/PluginBase.hpp"

namespace AxoPlotl
{

class DataControlPlugin : public PluginBase
{
public:
    DataControlPlugin() {}

    void render_ui(Application& app) override;

    const char* name() const override {
        return "Data Control";
    }
};

}
