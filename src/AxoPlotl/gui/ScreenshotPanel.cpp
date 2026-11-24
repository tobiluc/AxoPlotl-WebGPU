#include "ScreenshotPanel.hpp"
#include <imgui.h>
#include <AxoPlotl/gui/widgets/widgets.hpp>
#include <AxoPlotl/core/Application.hpp>
#include <ImGuiFileDialog.h>

namespace AxoPlotl::internal
{
    void ScreenshotPanel::render_ui(Application* _app)
    {
        if (!enabled || !_app) {return;}

        ImGui::SetNextWindowSize(
            ImVec2(640, 480),
            ImGuiCond_FirstUseEver
            );
        if (ImGui::Begin(
                "Screenshot",
                &enabled))
        {
            gui::InputSlider2("Size",
                         &size.width,
                         gui::InputSliderOptions{
                            .step=1,.min=1,.max=8000,
                            .display_name=true});
            if (ImGui::Button("Keep Aspect Ratio")) {
                if (size.height > size.width) {
                    size.width = size.height * _app->viewport().size().aspect_ratio();
                } else {
                    size.height = size.width / _app->viewport().size().aspect_ratio();
                }
            }
            ImGui::SameLine();
            ImGui::Checkbox("Transparent Background", &transparent_background);

            const float preview_width = ImGui::GetItemRectSize().x;
            const float preview_height = preview_width / size.aspect_ratio();

            gui::Texture(_app->viewport().color_view(), preview_width, preview_height);

            if (ImGui::Button("Save")) {
                IGFD::FileDialogConfig config;
                    config.path = filepath;
                    ImGuiFileDialog::Instance()->OpenDialog(
                        "SaveScreenshotDialogKey", "Choose File",
                        "(*.png){.png}",
                        config);
            }
        }

        if (ImGuiFileDialog::Instance()->Display("SaveScreenshotDialogKey")) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                this->filepath = ImGuiFileDialog::Instance()->GetFilePathName();
                if (size.width <= MAX_SIZE.width && size.height <= MAX_SIZE.height) {
                    _app->save_viewport_screenshot(
                                filepath,
                                size,
                                transparent_background);
                }
            }
            ImGuiFileDialog::Instance()->Close();
        }

        ImGui::End(); //!Screenshot
    }
}