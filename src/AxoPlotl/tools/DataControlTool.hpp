#pragma once

#include <AxoPlotl/core/AxoPlotl_fwd.hpp>
#include <AxoPlotl/gui/fonts.hpp>

namespace AxoPlotl
{

class DataControlTool
{
private:
    DataControlTool() {};
public:
    static void render_ui();
    static constexpr const char* icon() {return ICON_FA_SLIDERS;}
private:
    static int info_object_id_;
    static int settings_object_id_;
    static int properties_object_id_;
};

}
