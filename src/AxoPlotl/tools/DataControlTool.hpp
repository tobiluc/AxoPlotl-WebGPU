#pragma once

#include <AxoPlotl/AxoPlotl_fwd.hpp>

namespace AxoPlotl
{

class DataControlTool
{
private:
    DataControlTool() {};
public:
    static void render_ui();

    static const char* name() {
        return "Data Control";
    }
private:
    static int info_object_id_;
    static int settings_object_id_;
    static int properties_object_id_;
};

}
