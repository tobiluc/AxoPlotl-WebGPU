#pragma once
#include <AxoPlotl/gui/fonts.hpp>

namespace AxoPlotl
{

class DebugTool
{
private:
    DebugTool() {}
public:
    static void render_ui();
    static constexpr const char* icon() {return ICON_FA_BUG;}
};

}
