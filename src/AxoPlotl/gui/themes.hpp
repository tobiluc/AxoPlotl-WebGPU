#pragma once

namespace AxoPlotl::GUI
{

enum class Theme {
    Dark,
    Light,
    Classic,
    ModernDark,
    ModernLight
};

static Theme global_theme = Theme::ModernDark;

void apply_theme(Theme _theme);

}
