#pragma once
#include <imgui.h>

namespace AxoPlotl
{

// forward declaration
class Application;

class PluginBase
{
public:
    virtual ~PluginBase() = default;

    virtual void render_ui(Application& app) = 0;

    virtual const char* name() const {
        return "Unnamed Plugin";
    }
};

}
