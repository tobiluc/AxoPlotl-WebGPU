#include "DataControlTool.hpp"
#include "AxoPlotl/AxoPlotl.hpp"
#include <mach/task_info.h>
#include <mach/mach.h>
#include <AxoPlotl/gui/fonts.hpp>
#include <AxoPlotl/Scene.hpp>

namespace AxoPlotl
{

int DataControlTool::info_object_id_ = -1;
int DataControlTool::settings_object_id_ = -1;
int DataControlTool::properties_object_id_ = -1;

void DataControlTool::render_ui()
{
    if (!ImGui::CollapsingHeader("Data Control")) {return;}
    using ConstObj = const std::shared_ptr<ObjectBase>&;

    // Set Object ids to invalid if objects were deleted
    if (!AxoPlotl::scene().get_object(info_object_id_)) {info_object_id_ = -1;}
    if (!AxoPlotl::scene().get_object(settings_object_id_)) {settings_object_id_ = -1;}

    // For Convenience, we can apply things to all objects at once
    if (AxoPlotl::scene().get_objects().size() > 0 && ImGui::BeginMenu("Apply to all")) {
        if (ImGui::MenuItem("Sort alphabetically")) {
            AxoPlotl::scene().sort_objects([](ConstObj _obj1, ConstObj _obj2) {
                return _obj1->name() < _obj2->name();
            });
        }
        ImGui::SeparatorText("Visibility");
        if (ImGui::Button("Hide")) {
            for (const auto& obj : AxoPlotl::scene().get_objects()) {
                obj->visible() = false;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Show")) {
            for (const auto& obj : AxoPlotl::scene().get_objects()) {
                obj->visible() = true;
            }
        }
        ImGui::SeparatorText("Danger Zone");
        if (ImGui::Button("Delete")) {
            for (const auto& obj : AxoPlotl::scene().get_objects()) {
                obj->deleted() = true;
            }
            info_object_id_ = -1;
        }
        ImGui::EndMenu();
    }
    ImGui::Separator();

    // Data Control per Object
    for (const auto& obj : AxoPlotl::scene().get_objects()) {
        ImGui::PushID(obj->id());

        ImGui::Checkbox("##V", &obj->target());

        ImGui::SameLine();
        // Visible Checkbox
        if (ImGui::Button(obj->visible() ? ICON_FA_EYE : ICON_FA_EYE_SLASH)) {
            obj->visible() = !obj->visible();
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_MAGNIFYING_GLASS)) {
            AxoPlotl::scene().zoom_to_box(obj->bounding_box());
            obj->visible() = true;
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_GEAR)) {
            ImGui::OpenPopup("popup_object_settings");
            settings_object_id_ = obj->id();
        }
        ImGui::SameLine();
        // Toggle Selected
        if (ImGui::Selectable((obj->name()).c_str())) {
            if (info_object_id_ == obj->id()) {
                info_object_id_ = -1;
            } else {
                info_object_id_ = obj->id();
            }
        }

        // Settings Popup
        if (settings_object_id_ >= 0 && ImGui::BeginPopup("popup_object_settings")) {
            AxoPlotl::scene().get_object(settings_object_id_)->render_ui_settings();
            ImGui::EndPopup();
        }

        // Expand Menu
        if (info_object_id_ == obj->id()) {
            obj->render_ui_info();
            if (ImGui::Button(ICON_FA_TRASH)) {
                obj->deleted() = true;
            }
        }
        ImGui::PopID();
        ImGui::Separator();
    }

    // Property Visualization
    std::string title = "Properties";
    if (ImGui::CollapsingHeader(title.c_str())) {
        if (ImGui::BeginMenu("Object")) {
            for (const auto& obj : AxoPlotl::scene().get_objects()) {
                ImGui::PushID(obj->id());
                if (ImGui::MenuItem(obj->name().c_str())) {
                    if (properties_object_id_ == obj->id()) {
                        properties_object_id_ = -1;
                    } else {
                        properties_object_id_ = obj->id();
                    }
                }
                ImGui::PopID();
            }
            ImGui::EndMenu();
        }
        if (auto obj = AxoPlotl::scene().get_object(properties_object_id_)) {
            ImGui::SeparatorText(obj->name().c_str());
            obj->render_ui_properties();
        } else {properties_object_id_ = -1;}
    }
}

}
