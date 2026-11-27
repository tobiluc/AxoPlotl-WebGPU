#pragma once

// #define FONT_ICON_FILE_NAME_FAR AXOPLOTL_FONTS_DIR "fa-regular-400.otf"
// #define FONT_ICON_FILE_NAME_FAS AXOPLOTL_FONTS_DIR "fa-solid-900.otf"
#include "IconsFontAwesome7.h"
#include <filesystem>

namespace AxoPlotl::gui
{

struct FontConfig
{
    float font_size;
    std::filesystem::path path;
};

void load_fonts(const FontConfig& _font_config = FontConfig{.font_size=16.0f,.path="Inter_24pt-Regular.ttf"},
                const FontConfig& _icon_config = FontConfig{.font_size=16.0f,.path="fa-solid-900.otf"});

}
