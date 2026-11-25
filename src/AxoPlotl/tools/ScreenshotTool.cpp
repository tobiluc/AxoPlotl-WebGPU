#include "ScreenshotTool.hpp"
#include <filesystem>
#include <AxoPlotl/typedefs/glm.hpp>
#include <ImGuiFileDialog.h>
#include <AxoPlotl/gui/widgets/widgets.hpp>
#include <AxoPlotl/core/Application.hpp>
#include <AxoPlotl/AxoPlotl.hpp>

namespace AxoPlotl::ScreenshotTool
{

static constexpr ViewSize MAX_SIZE = {8000,8000};
static std::filesystem::path filepath = "AxoPlotl_Screenshot.png";
static ViewSize size = {512, 512};
static bool transparent_background = false;

const char* icon()
{
    return ICON_FA_IMAGE;
}

void draw_ui()
{
    const auto& viewport = AxoPlotl::app().viewport();
    gui::InputSlider2("Size",
                        &size.width,
                        gui::InputSliderOptions{
                        .step=1,.min=1,.max=8000,
                        .display_name=true});
    if (ImGui::Button("Keep Aspect Ratio")) {
        if (size.height > size.width) {
            size.width = size.height * viewport.size().aspect_ratio();
        } else {
            size.height = size.width / viewport.size().aspect_ratio();
        }
    }

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
            if (size.width <= MAX_SIZE.width && size.height <= MAX_SIZE.height) {
                AxoPlotl::app().save_viewport_screenshot(
                    filepath,
                    size,
                    transparent_background);
            }
        }
        ImGuiFileDialog::Instance()->Close();
    }
}

}
