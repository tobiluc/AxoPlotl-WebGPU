#pragma once
#include <AxoPlotl/PluginBase.hpp>

namespace AxoPlotl
{

class DebugPlugin : public PluginBase
{
public:
    DebugPlugin() {}

    void render_ui(Application& app) override;

    const char* name() const override {
        return "Debug Plugin";
    }
};

}
