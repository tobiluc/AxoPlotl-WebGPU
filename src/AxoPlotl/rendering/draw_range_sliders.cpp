#include <AxoPlotl/rendering/draw_range_sliders.hpp>
#include <algorithm>
#include <imgui.h>
#include <string>

namespace AxoPlotl
{

bool draw_range_sliders(
    float& _l, float& _r,
    const float& _min, const float& _max)
{
    ImVec2 top_left = ImGui::GetCursorScreenPos();
    ImVec2 total_size = ImVec2(ImGui::GetContentRegionAvail().x, 20);
    ImVec2 bot_right = ImVec2(top_left.x + total_size.x, top_left.y + total_size.y);

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    // Helper to map property value
    // to screen x coordinates
    auto val_to_screen_x = [&](const float& _val) {
        float t = (_val - _min) / (_max - _min);
        return top_left.x + t * total_size.x;
    };

    // Float input to modify range
    {
        //ImGui::InputFloat("L", )
    }

    // Draw the Colormap Image
    const float visible_left = val_to_screen_x(_l);
    const float visible_right = val_to_screen_x(_r);
    // Background
    draw_list->AddRectFilled(top_left, bot_right, ImGui::GetColorU32(ImGuiCol_FrameBg));
    ImGui::SetCursorScreenPos(ImVec2(visible_left, top_left.y));
    // TODO: Investigate why the follwing line crashes
    //ImGui::Image((ImTextureID)view_, ImVec2(visible_right-visible_left, total_size.y));
    ImGui::SetCursorScreenPos(top_left);

    // Setup interaction
    bool changed = false;
    float handle_x[2] = {val_to_screen_x(_l),
                         val_to_screen_x(_r)};

    for (int i = 0; i < 2; ++i)
    {
        ImGui::PushID(i);
        ImVec2 pos = ImVec2(handle_x[i], top_left.y + total_size.y * 0.5f);

        // Transparent invisible button to capture drag
        const float button_width = 16.0f;
        ImGui::SetCursorScreenPos(ImVec2(pos.x - 0.5f*button_width, top_left.y));
        ImGui::InvisibleButton("##handle", ImVec2(button_width, total_size.y));

        if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
        {
            const float mouse_x = ImGui::GetIO().MousePos.x;
            const float t = std::clamp((mouse_x - top_left.x) / total_size.x, 0.0f, 1.0f);

            const float val = _min + t * (_max - _min);

            // Clamp
            if (i == 0) {
                _l = std::min(val, _r);
            } else {
                _r = std::max(val, _l);
            }

            changed = true;
        }

        // Draw the visual handle
        const ImU32 handle_fill_color = changed? ImGui::GetColorU32(ImGuiCol_SliderGrabActive) : ImGui::GetColorU32(ImGuiCol_SliderGrab);
        const ImU32 handle_outline_color = ImGui::GetColorU32(ImGuiCol_Border);
        draw_list->AddLine(ImVec2(handle_x[i], top_left.y), ImVec2(handle_x[i], bot_right.y), handle_fill_color, 4.0f);
        draw_list->AddCircleFilled(ImVec2(handle_x[i], bot_right.y), 4.0f, handle_fill_color);
        draw_list->AddCircle(ImVec2(handle_x[i], bot_right.y), 4.0f, handle_outline_color);

        // Handle Label
        char buf[32];
        snprintf(buf, sizeof(buf), "%s", (i == 0)?
                                             std::to_string(_l).c_str() :
                                             std::to_string(_r).c_str());
        ImVec2 text_size = ImGui::CalcTextSize(buf);
        const float text_x = handle_x[i] - (text_size.x * 0.5f);
        const float text_y = bot_right.y + 5.0f;
        draw_list->AddText(ImVec2(text_x, text_y), ImGui::GetColorU32(ImGuiCol_Text), buf);

        ImGui::PopID();
    }

    // Move the cursor down so the next ImGui element doesn't overlap the text
    const float reserved_space = 5.0f + ImGui::GetFontSize();
    ImGui::SetCursorScreenPos(ImVec2(top_left.x, bot_right.y + reserved_space + ImGui::GetStyle().ItemSpacing.y));

    return changed;
}

}
