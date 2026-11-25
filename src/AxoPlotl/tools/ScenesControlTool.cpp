#include "ScenesControlTool.hpp"
#include "AxoPlotl/AxoPlotl.hpp"
#include "ImGuiFileDialog.h"
#include <mach/task_info.h>
#include <mach/mach.h>
#include <AxoPlotl/gui/fonts.hpp>
#include <AxoPlotl/core/Scene.hpp>
#include <AxoPlotl/gui/fonts.hpp>
#include "misc/cpp/imgui_stdlib.h"

namespace AxoPlotl::ScenesControlTool
{

static int target_index = -1;

const char* icon() {return ICON_FA_LAYER_GROUP;}

void draw_ui()
{
    if (ImGui::BeginTabBar("Scenes")) {
        const uint32_t app_target_index = app().selected_scene_index();
        const bool switch_requested = target_index != app_target_index;
        for (uint32_t i = 0; i < app().num_scenes(); ++i) {
            auto scene = app().scene(i);
            if (!scene) {continue;}

            uintptr_t scene_id = reinterpret_cast<uintptr_t>(scene);
            ImGui::PushID(scene_id);

            ImGuiTabItemFlags flags = ImGuiTabItemFlags_None;
            if (switch_requested && i == app_target_index) {
                flags |= ImGuiTabItemFlags_SetSelected;
            }
            
            const std::string tab_label = scene->name + "###Scene_" + std::to_string(scene_id);
            if (ImGui::BeginTabItem(tab_label.c_str(), nullptr, flags))
            {
                if (!switch_requested || i == app_target_index) {
                    app().set_scene(i);
                }

                // Name Edit
                ImGui::InputText("Name", &(scene->name));

                // Settings
                if (ImGui::CollapsingHeader(ICON_FA_CAMERA)) {
                    scene->camera().render_settings_ui(app().window());
                }

                if (ImGui::CollapsingHeader(ICON_FA_LIGHTBULB)) {
                    ImGui::ColorEdit3("Color", &scene->directional_light.color[0]);
                    ImGui::InputFloat3("Direction", &scene->directional_light.direction[0]);
                    gui::InputSlider("Strength", &scene->directional_light.strength);
                    ImGui::Checkbox("From Camera", &scene->settings.light_from_camera);
                }

                if (ImGui::CollapsingHeader(ICON_FA_PALETTE)) {
                    ImGui::ColorEdit3("Background", scene->settings.clear_color);
                    ImGui::Checkbox("Axis Cross", &scene->settings.show_axis_cross);
                    ImGui::Checkbox("Orbit Cross", &scene->settings.show_orbit_target_cross);
                    ImGui::Checkbox("Zoom to New Object", &scene->settings.zoom_to_new_object);
                }

                // Delete Scene Button
                if (app().num_scenes() > 1lu) {
                    if (ImGui::Button(ICON_FA_TRASH)) {
                        app().delete_scene(i);
                        ImGui::EndTabItem();
                        ImGui::PopID();
                        break;
                    }
                    ImGui::SetItemTooltip("Delete Scene");
                }
                ImGui::EndTabItem();
            }

            ImGui::PopID();
        }
        target_index = app().selected_scene_index();

        // Add a Scene Button
        if (ImGui::TabItemButton(ICON_FA_PLUS, ImGuiTabItemFlags_Trailing)) {
            app().add_scene("New Scene");
        }

        ImGui::SetItemTooltip("Add Scene");

        ImGui::EndTabBar();
    }
}

}
