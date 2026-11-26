#include "ScreenshotTool.hpp"
#include <filesystem>
#include <AxoPlotl/typedefs/glm.hpp>
#include <ImGuiFileDialog.h>
#include <AxoPlotl/gui/widgets/widgets.hpp>
#include <AxoPlotl/core/Application.hpp>
#include <AxoPlotl/AxoPlotl.hpp>

namespace AxoPlotl::ScreenshotTool
{

static constexpr uint32_t MAX_SIZE = 8000;
static std::filesystem::path filepath = "AxoPlotl_Screenshot.png";
static ViewSize size = {512, 512};
static bool transparent_background = false;

void draw_ui()
{
    const auto& viewport = AxoPlotl::app().viewport();
    Vec2 sizef = size.to_vec2();
    gui::InputSlider2("Size",
        &sizef[0],
        gui::InputSliderOptions{
        .step=1,.min=1,.max=MAX_SIZE,
        .display_name=true,
        .integer = true});
    size.from_vec2(sizef);
    if (ImGui::Button("Keep Aspect Ratio")) {
        if (size.height > size.width) {
            size.width = size.height * viewport.size().aspect_ratio();
        } else {
            size.height = size.width / viewport.size().aspect_ratio();
        }
    }
    size.clamp({1,1}, {MAX_SIZE,MAX_SIZE});

    ImGui::Checkbox("Transparent Background", &transparent_background);

    const float preview_width = ImGui::GetItemRectSize().x;
    const float preview_height = preview_width / size.aspect_ratio();

    gui::Texture(viewport.color_view(), preview_width, preview_height);

    if (ImGui::Button("Save")) {
        IGFD::FileDialogConfig config;
        config.path = filepath;
        ImGuiFileDialog::Instance()->OpenDialog(
            "SaveScreenshotDialogKey", "Choose File",
            "(*.png){.png}",
            config);
    }

    if (ImGuiFileDialog::Instance()->Display("SaveScreenshotDialogKey")) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            filepath = ImGuiFileDialog::Instance()->GetFilePathName();
            AxoPlotl::app().save_viewport_screenshot(
                    filepath,
                    size,
                    transparent_background);
        }
        ImGuiFileDialog::Instance()->Close();
    }
}

}
