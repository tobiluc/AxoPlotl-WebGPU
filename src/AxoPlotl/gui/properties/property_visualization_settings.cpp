#include "property_visualization_settings.hpp"
#include <imgui.h>

namespace AxoPlotl::internal
{

void VisualizationSettings::render_ui()
{
    ImGui::SeparatorText("TEST VIS");
    for (int rule_i = 0; rule_i < rules.size(); ++rule_i) {
        auto& rule = rules[rule_i];
        ImGui::PushID(rule_i);
        ImGui::Text("%s", rule.name.c_str());
        ImGui::SameLine();
        ImGui::Checkbox("Enabled", &rule.enabled);

        for (int interval_i = 0; interval_i < rule.condition.intervals.size(); ++interval_i) {

        }

        if (ImGui::BeginMenu("Add Condition")) {
            if (ImGui::MenuItem("Interval")) {
                rule.condition.intervals.emplace_back(0.0f, 1.0f);
            }
            ImGui::EndMenu();
        }

        ImGui::PopID();
    }
    if (ImGui::Button("+")) {
        rules.emplace_back();
    }
}

}
