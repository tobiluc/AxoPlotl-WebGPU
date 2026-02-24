#pragma once
#include "AxoPlotl/PluginBase.hpp"
#include <AxoPlotl/AxoPlotl_fwd.hpp>
#include <unordered_map>

namespace AxoPlotl
{

class DataControlTool
{
public:
    DataControlTool() {}

    void render_ui(Application& _app);

    const char* name() {
        return "Data Control";
    }
private:
    std::unordered_map<int,bool> obj_expanded_;
};

}
