#include "themes.hpp"
#include "imgui.h"

namespace AxoPlotl::GUI
{

static void set_modern_dark_theme()
{
    ImGuiStyle& style = ImGui::GetStyle();
    ImGui::StyleColorsDark();

    style.WindowRounding = 0.0f;
    style.ChildRounding = 0.0f;
    style.FrameRounding = 3.0f;
    style.ScrollbarRounding = 6.0f;
    style.GrabRounding = 4.0f;
    style.FrameBorderSize = 1.0f;
    style.WindowBorderSize = 1.0f;

    style.WindowPadding = ImVec2(12, 12);
    style.FramePadding = ImVec2(8, 6);
    style.ItemSpacing = ImVec2(10, 8);

    ImVec4* colors = style.Colors;

    colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.105f, 0.11f, 1.0f);
    colors[ImGuiCol_Header] = ImVec4(0.20f, 0.205f, 0.21f, 1.0f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.30f, 0.305f, 0.31f, 1.0f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.15f, 0.1505f, 0.151f, 1.0f);

    colors[ImGuiCol_Button] = ImVec4(0.20f, 0.205f, 0.21f, 1.0f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.30f, 0.305f, 0.31f, 1.0f);
    colors[ImGuiCol_ButtonActive]       = ImVec4(0.15f, 0.1505f, 0.151f, 1.0f);

    colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.205f, 0.21f, 1.0f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.30f, 0.305f, 0.31f, 1.0f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.15f, 0.1505f, 0.151f, 1.0f);

    colors[ImGuiCol_Tab]                = ImVec4(0.15f, 0.1505f, 0.151f, 1.0f);
    colors[ImGuiCol_TabHovered]         = ImVec4(0.38f, 0.3805f, 0.381f, 1.0f);
    colors[ImGuiCol_TabActive]          = ImVec4(0.28f, 0.2805f, 0.281f, 1.0f);
    colors[ImGuiCol_TabUnfocused]       = ImVec4(0.15f, 0.1505f, 0.151f, 1.0f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.20f, 0.205f, 0.21f, 1.0f);

    colors[ImGuiCol_TitleBg]            = ImVec4(0.09f, 0.09f, 0.09f, 1.0f);
    colors[ImGuiCol_TitleBgActive]      = ImVec4(0.15f, 0.1505f, 0.151f, 1.0f);

    colors[ImGuiCol_CheckMark]          = ImVec4(0.80f, 0.80f, 0.83f, 1.0f);
    colors[ImGuiCol_SliderGrab]         = ImVec4(0.28f, 0.56f, 1.00f, 1.0f);
    colors[ImGuiCol_SliderGrabActive]   = ImVec4(0.37f, 0.61f, 1.00f, 1.0f);
}

static void set_modern_light_theme()
{
    ImGuiStyle& style = ImGui::GetStyle();
    ImGui::StyleColorsLight();

    style.WindowRounding    = 6.0f;
    style.FrameRounding     = 4.0f;
    style.ScrollbarRounding = 6.0f;
    style.GrabRounding      = 4.0f;
    style.FrameBorderSize   = 1.0f;
    style.WindowBorderSize  = 1.0f;

    style.WindowPadding = ImVec2(12, 12);
    style.FramePadding  = ImVec2(8, 6);
    style.ItemSpacing   = ImVec2(10, 8);

    ImVec4* colors = style.Colors;

    // Main backgrounds
    colors[ImGuiCol_WindowBg]           = ImVec4(0.95f, 0.96f, 0.98f, 1.0f);
    colors[ImGuiCol_ChildBg]            = ImVec4(0.93f, 0.94f, 0.96f, 1.0f);
    colors[ImGuiCol_PopupBg]            = ImVec4(1.00f, 1.00f, 1.00f, 1.0f);

    // Borders
    colors[ImGuiCol_Border]             = ImVec4(0.80f, 0.82f, 0.85f, 1.0f);

    // Headers (CollapsingHeader, TreeNode, Selectable)
    colors[ImGuiCol_Header]             = ImVec4(0.85f, 0.87f, 0.90f, 1.0f);
    colors[ImGuiCol_HeaderHovered]      = ImVec4(0.75f, 0.82f, 0.95f, 1.0f);
    colors[ImGuiCol_HeaderActive]       = ImVec4(0.65f, 0.75f, 0.92f, 1.0f);

    // Buttons
    colors[ImGuiCol_Button]             = ImVec4(0.88f, 0.90f, 0.93f, 1.0f);
    colors[ImGuiCol_ButtonHovered]      = ImVec4(0.75f, 0.82f, 0.95f, 1.0f);
    colors[ImGuiCol_ButtonActive]       = ImVec4(0.65f, 0.75f, 0.92f, 1.0f);

    // Frame background (inputs, sliders)
    colors[ImGuiCol_FrameBg]            = ImVec4(1.00f, 1.00f, 1.00f, 1.0f);
    colors[ImGuiCol_FrameBgHovered]     = ImVec4(0.90f, 0.92f, 0.95f, 1.0f);
    colors[ImGuiCol_FrameBgActive]      = ImVec4(0.85f, 0.87f, 0.90f, 1.0f);

    // Tabs
    colors[ImGuiCol_Tab]                = ImVec4(0.88f, 0.90f, 0.93f, 1.0f);
    colors[ImGuiCol_TabHovered]         = ImVec4(0.75f, 0.82f, 0.95f, 1.0f);
    colors[ImGuiCol_TabActive]          = ImVec4(0.80f, 0.85f, 0.95f, 1.0f);
    colors[ImGuiCol_TabUnfocused]       = ImVec4(0.90f, 0.92f, 0.95f, 1.0f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.85f, 0.87f, 0.90f, 1.0f);

    // Title bars
    colors[ImGuiCol_TitleBg]            = ImVec4(0.88f, 0.90f, 0.93f, 1.0f);
    colors[ImGuiCol_TitleBgActive]      = ImVec4(0.75f, 0.82f, 0.95f, 1.0f);

    // Accent elements
    colors[ImGuiCol_CheckMark]          = ImVec4(0.20f, 0.45f, 0.85f, 1.0f);
    colors[ImGuiCol_SliderGrab]         = ImVec4(0.20f, 0.45f, 0.85f, 1.0f);
    colors[ImGuiCol_SliderGrabActive]   = ImVec4(0.10f, 0.35f, 0.75f, 1.0f);

    // Text
    colors[ImGuiCol_Text]               = ImVec4(0.15f, 0.18f, 0.22f, 1.0f);
}

void apply_theme(Theme _theme)
{
    global_theme = _theme;
    switch (_theme)
    {
    case Theme::Dark:
        ImGui::StyleColorsDark();
        break;
    case Theme::Light:
        ImGui::StyleColorsLight();
        break;
    case Theme::Classic:
        ImGui::StyleColorsClassic();
        break;
    case Theme::ModernDark:
        set_modern_dark_theme();
        break;
    case Theme::ModernLight:
        set_modern_light_theme();
        break;
    default:
        break;
    }
}

}
