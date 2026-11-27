#include "fonts.hpp"
#include "imgui.h"
#include <filesystem>

void AxoPlotl::gui::load_fonts(const FontConfig& _font_config, const FontConfig& _icon_config)
{
    auto& io = ImGui::GetIO();
    {
        std::filesystem::path font_path(AXOPLOTL_FONTS_DIR / _font_config.path);
        ImFontConfig config;
        io.Fonts->AddFontFromFileTTF(
            font_path.c_str(),
            _font_config.font_size,
            &config
        );
    }
    {
        std::filesystem::path font_path(AXOPLOTL_FONTS_DIR / _icon_config.path);
        static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
        ImFontConfig config;
        config.MergeMode = true;
        config.PixelSnapH = true;
        io.Fonts->AddFontFromFileTTF(
            font_path.c_str(),
            _icon_config.font_size,
            &config,
            icons_ranges
        );
    }
}
