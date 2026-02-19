#pragma once
#include "AxoPlotl/PluginBase.hpp"

namespace AxoPlotl
{

class ObjectListPlugin : public PluginBase
{
public:
    ObjectListPlugin() {}

    void render_ui(Application& app) override;

    const char* name() const override {
        return "Object List";
    }
};

}
