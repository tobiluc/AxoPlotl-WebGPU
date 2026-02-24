#include "DataControlTool.hpp"
#include "AxoPlotl/Application.hpp"
#include <mach/task_info.h>
#include <mach/mach.h>

namespace AxoPlotl
{

void DataControlTool::render_ui(Application& _app)
{
    if (!ImGui::CollapsingHeader("Data Control")) {return;}

    // For Convenience, we can apply things to all objects at once
    if (ImGui::BeginMenu("Apply to all")) {
        ImGui::SeparatorText("Details");
        if (ImGui::Button("Collapse")) {
            obj_expanded_.clear();
        }
        ImGui::SameLine();
        if (ImGui::Button("Expand")) {
            for (const auto& obj : _app.scene().get_objects()) {
                obj_expanded_[obj->id()] = true;
            }
        }
        // if (ImGui::MenuItem("Sort alphabetically")) {
        //     _app.scene()
        // }
        ImGui::SeparatorText("Visibility");
        if (ImGui::Button("Hide")) {
            for (const auto& obj : _app.scene().get_objects()) {
                obj->visible() = false;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Show")) {
            for (const auto& obj : _app.scene().get_objects()) {
                obj->visible() = true;
            }
        }
        ImGui::SeparatorText("Danger Zone");
        if (ImGui::Button("Delete")) {
            for (const auto& obj : _app.scene().get_objects()) {
                obj->deleted() = true;
            }
            _app.add_deferred_call([this] {
                obj_expanded_.clear();
            });
        }
        ImGui::EndMenu();
    }

    // Data Control per Object
    for (const auto& obj : _app.scene().get_objects()) {
        ImGui::PushID(obj->id());

        if (!obj_expanded_.contains(obj->id())) {
            obj_expanded_[obj->id()] = false;
        }

        // Visible Checkbox
        ImGui::Checkbox("##V", &obj->visible());
        ImGui::SameLine();
        if (ImGui::Button("Zoom")) {
            _app.scene().zoom_to_box(obj->bounding_box());
            obj->visible() = true;
        }
        ImGui::SameLine();
        // Toggle Selected
        if (ImGui::Selectable(obj->name().c_str())) {
            obj_expanded_[obj->id()] = !obj_expanded_[obj->id()];
        }

        // Expand Menu
        if (obj_expanded_[obj->id()]) {
            obj->render_ui();
            if (ImGui::Button("Delete Object")) {
                obj->deleted() = true;
            }
        }
        ImGui::PopID();
    }
}

}
