#pragma once
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
    int expanded_object_id_ = -1;
    int settings_object_id_ = -1;
};

}
