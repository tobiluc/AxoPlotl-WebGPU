#include "fonts.hpp"
#include "imgui.h"
#include <filesystem>

void AxoPlotl::GUI::load_fonts(float _font_size)
{
    auto& io = ImGui::GetIO();
    {
        std::filesystem::path font_path(AXOPLOTL_FONTS_DIR "inter.ttf");
        ImFontConfig config;
        io.Fonts->AddFontFromFileTTF(
            font_path.c_str(),
            _font_size,
            &config
        );
    }
    {
        std::filesystem::path font_path(AXOPLOTL_FONTS_DIR "fa-solid-900.otf");
        static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
        ImFontConfig config;
        config.MergeMode = true;
        config.PixelSnapH = true;
        io.Fonts->AddFontFromFileTTF(
            font_path.c_str(),
            _font_size,
            &config,
            icons_ranges
        );
    }
}
