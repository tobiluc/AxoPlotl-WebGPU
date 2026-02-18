#pragma once
#include "AxoPlotl/PluginBase.hpp"

namespace AxoPlotl
{

class TestPlugin : public PluginBase
{
public:
    TestPlugin() {}

    void render_ui(Application& app) override;

    const char* name() const override {
        return "Evil Plugin";
    }
};

}
