#include "fonts.hpp"
#include "imgui.h"
#include <filesystem>

void AxoPlotl::GUI::load_fonts()
{
    auto& io = ImGui::GetIO();
    {
        std::filesystem::path font_path(AXOPLOTL_FONTS_DIR "inter.ttf");
        ImFontConfig config;
        io.Fonts->AddFontFromFileTTF(
            font_path.c_str(),
            16.0f,
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
            10.0f,
            &config,
            icons_ranges
        );
    }

}
