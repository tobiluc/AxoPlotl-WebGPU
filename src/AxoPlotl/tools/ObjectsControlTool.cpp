#include "ObjectsControlTool.hpp"
#include "AxoPlotl/AxoPlotl.hpp"
#include "ImGuiFileDialog.h"
#include <mach/task_info.h>
#include <mach/mach.h>
#include <AxoPlotl/gui/fonts.hpp>
#include <AxoPlotl/core/Scene.hpp>
#include <AxoPlotl/gui/fonts.hpp>
#include "AxoPlotl/core/objects/SegmentObject.hpp"
#include "AxoPlotl/core/objects/SphericalHarmonicsObject.hpp"
#include "AxoPlotl/core/objects/TetrahedronObject.hpp"

namespace AxoPlotl::ObjectsControlTool
{

static int info_object_id_ = -1;
static int settings_object_id_ = -1;
static int properties_object_id_ = -1;

const char* icon() {return ICON_FA_SLIDERS;}

void draw_ui()
{
    // Add new Object
    if (ImGui::BeginMenu("Add Object")) {
        if (ImGui::MenuItem("Load Mesh")) {
                IGFD::FileDialogConfig config;
                config.path = "..";
                ImGuiFileDialog::Instance()->OpenDialog(
                    "LoadMeshDialogKey", "Choose File",
                    "Mesh files (*.obj *.ovm *.ovmb *ply){.obj,.ovm,.ovmb,.ply}",
                    config);
            }
        if (ImGui::MenuItem("Spherical Harmonics")) {
            app().scene().add_object<SHObject>();
        }
        if (ImGui::MenuItem("Tetrahedron")) {
            app().scene().add_object<TetObject>();
        }
        if (ImGui::MenuItem("Segment")) {
            app().scene().add_object<SegmentObject>();
        }
        ImGui::EndMenu(); //!Add Object
    }
    // Load Mesh File Dialog
    if (ImGuiFileDialog::Instance()->Display("LoadMeshDialogKey")) {
        if (ImGuiFileDialog::Instance()->IsOk()) { // action if OK
            std::filesystem::path filepath = ImGuiFileDialog::Instance()->GetFilePathName();
            app().scene().add_mesh(filepath);
        }
        ImGuiFileDialog::Instance()->Close();
    }

    using ConstObj = const std::shared_ptr<ObjectBase>&;

    // Set Object ids to invalid if objects were deleted
    if (!AxoPlotl::app().scene().get_object(info_object_id_)) {info_object_id_ = -1;}
    if (!AxoPlotl::app().scene().get_object(settings_object_id_)) {settings_object_id_ = -1;}

    // For Convenience, we can apply things to all objects at once
    if (AxoPlotl::app().scene().num_objects() > 0 && ImGui::BeginMenu("Selection"))
    {
        ImGui::SeparatorText("Select");
        if (ImGui::Button("Select All")) {
            for (const auto& obj : AxoPlotl::app().scene().get_objects()) {
                obj->target() = true;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Deselect All")) {
            for (const auto& obj : AxoPlotl::app().scene().get_objects()) {
                obj->target() = false;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Invert Selection")) {
            for (const auto& obj : AxoPlotl::app().scene().get_objects()) {
                obj->target() = !obj->target();
            }
        }

        ImGui::SeparatorText("Visibility");
        if (ImGui::Button(ICON_FA_EYE)) {
            for (const auto& obj : AxoPlotl::app().scene().get_target_objects()) {
                obj->visible() = true;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_EYE_SLASH)) {
            for (const auto& obj : AxoPlotl::app().scene().get_target_objects()) {
                if (obj->target()) {
                    obj->visible() = false;
                }
            }
        }

        ImGui::SeparatorText("Danger Zone");
        if (ImGui::Button(ICON_FA_TRASH)) {
            for (const auto& obj : AxoPlotl::app().scene().get_target_objects()) {
                obj->deleted() = true;
            }
            info_object_id_ = -1;
        }
        ImGui::EndMenu();
    }
    ImGui::Separator();

    // Data Control per Object
    for (const auto& obj : AxoPlotl::app().scene().get_objects())
    {
        ImGui::PushID(obj->id());

        // Target Toggle
        ImGui::Checkbox("##V", &obj->target());

        ImGui::SameLine();

        // Name
        ImGui::SeparatorText((obj->name()).c_str());

        // Visible Toggle
        if (ImGui::Button(obj->visible() ? ICON_FA_EYE : ICON_FA_EYE_SLASH)) {
            obj->visible() = !obj->visible();
        }

        ImGui::SameLine();

        // Zoom Button
        if (ImGui::Button(ICON_FA_MAGNIFYING_GLASS)) {
            AxoPlotl::app().scene().zoom_to_box(obj->bounding_box());
            obj->visible() = true;
        }

        ImGui::SameLine();

        // Settings Button
        if (ImGui::Button(ICON_FA_GEAR)) {
            ImGui::OpenPopup("popup_object_settings");
            settings_object_id_ = obj->id();
        }

        ImGui::SameLine();

        // Info Button
        if (ImGui::Button(ICON_FA_CIRCLE_INFO)) {
            ImGui::OpenPopup("popup_object_info");
            info_object_id_ = obj->id();
        }

        // ImGui::SameLine();
        // Export Button
        // if (ImGui::Button(ICON_FA_FILE_EXPORT)) {
        //     IGFD::FileDialogConfig cfg;
        //     cfg.path = "..";
        //     cfg.userDatas = (void*)(intptr_t)obj->id();
        //     ImGuiFileDialog::Instance()->OpenDialog(
        //         "SaveObjectDialogKey", "Choose File",
        //         "",
        //         cfg);
        // }

        // Info Popup
        if (info_object_id_ >= 0
            && ImGui::BeginPopup("popup_object_info"))
        {
            AxoPlotl::app().scene().get_object(info_object_id_)->draw_info_ui();
            ImGui::EndPopup();
        }

        // Settings Popup
        if (settings_object_id_ >= 0
            && ImGui::BeginPopup("popup_object_settings"))
        {
            AxoPlotl::app().scene().get_object(settings_object_id_)->draw_settings_ui();
            ImGui::SeparatorText("Danger Zone");
            if (ImGui::Button(ICON_FA_TRASH)) {
                obj->deleted() = true;
            }
            ImGui::EndPopup();
        }

        ImGui::PopID();
        ImGui::Separator();
    }
    if (AxoPlotl::app().scene().num_objects() == 0lu) {
        ImGui::TextWrapped("Your objects are displayed here, but you don't have any...");
    }

    // Export File Dialog
    // if (ImGuiFileDialog::Instance()->Display("SaveObjectDialogKey")) {
    //     if (ImGuiFileDialog::Instance()->IsOk()) { // action if OK
    //         std::filesystem::path filepath = ImGuiFileDialog::Instance()->GetFilePathName();
    //         intptr_t export_obj_id = (intptr_t)ImGuiFileDialog::Instance()->GetUserDatas();
    //         // TODO
    //         if (!scene().get_object(export_obj_id)->save_to_file(filepath)) {
    //             ImGui::OpenPopup("popup_export_error");
    //         }
    //     }
    //     ImGuiFileDialog::Instance()->Close();
    // }

    // Export Error Popup
    // if (ImGui::BeginPopup("popup_export_error"))
    // {
    //     ImGui::Text("Failed to export object. Maybe exporting is not supported?");
    //     ImGui::EndPopup();
    // }

    // Property Visualization
    const char* selected_name = "None";
    for (const auto& obj : AxoPlotl::app().scene().get_objects()) {
        if (obj->id() == properties_object_id_) {
            selected_name = obj->name().c_str();
            break;
        }
    }
    if (ImGui::BeginCombo("Object", selected_name)) {
        for (const auto& obj : AxoPlotl::app().scene().get_objects()) {
            ImGui::PushID(obj->id());

            const bool selected = (properties_object_id_ == obj->id());

            if (ImGui::Selectable(obj->name().c_str(), selected)) {
                properties_object_id_ = selected ? -1 : obj->id();
            }

            if (selected) {
                ImGui::SetItemDefaultFocus();
            }

            ImGui::PopID();
        }
        ImGui::EndCombo();
    }

    if (auto propObj = AxoPlotl::app().scene().get_object(properties_object_id_)) {
        ImGui::SeparatorText(("Properties of "+propObj->name()).c_str());
        propObj->draw_properties_ui();
    } else {
        properties_object_id_ = -1;
    }
}

}
