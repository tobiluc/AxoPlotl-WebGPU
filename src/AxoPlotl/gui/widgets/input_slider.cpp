#include "input_slider.hpp"
#include <unordered_map>
#include <algorithm>
#include <imgui.h>
#include <imgui_internal.h>
#include <iostream>
#include <ostream>

namespace AxoPlotl
{

struct InputSliderState
{
    bool editing = false;
    bool requestFocus = false;
    float originalValue = 0.0f;
    char buffer[128] = {};
};

static std::unordered_map<unsigned int, InputSliderState> input_slider_states_;


bool InputSlider(
    const char* _name,
    float* _value,
    InputSliderOptions _opt)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();

    if (window->SkipItems) {return false;}

    if (_opt.max <= _opt.min) {
        std::cerr << "max <= min for InputSlider" << std::endl;
        return false;
    }

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    const ImGuiID id = window->GetID(_name);

    auto& state = input_slider_states_[id];

    const float width = ImGui::CalcItemWidth() * _opt.scale_width;
    const float height = ImGui::GetFrameHeight();

    const ImVec2 pos = window->DC.CursorPos;
    const ImRect bb(pos, ImVec2(pos.x + width, pos.y + height));

    bool changed = false;

    // =========================================================
    // TEXT EDITING
    // =========================================================

    if (state.editing)
    {
        ImGui::SetCursorScreenPos(bb.Min);
        ImGui::SetNextItemWidth(width);

        ImGui::PushID(id);

        if (state.requestFocus)
        {
            ImGui::SetKeyboardFocusHere();
            state.requestFocus = false;
        }

        const bool escape = ImGui::IsKeyPressed(ImGuiKey_Escape);

        const bool submitted = ImGui::InputText(
            "##edit",
            state.buffer,
            sizeof(state.buffer),
            ImGuiInputTextFlags_EnterReturnsTrue |
                ImGuiInputTextFlags_AutoSelectAll
            );

        const bool deactivated = ImGui::IsItemDeactivated();

        ImGui::PopID();

        if (escape)
        {
            // Cancel editing.
            *_value = state.originalValue;
            state.editing = false;
        }
        else if (submitted || deactivated)
        {
            // Evaluate the expression.
            float result = *_value;
            bool valid = false;

            if (_opt.parser)
            {
                // Use provided parsing callback
                valid = _opt.parser(state.buffer, result);
            }
            else
            {
                // Basic numeric fallback.
                char* end = nullptr;
                result = std::strtof(state.buffer, &end);

                valid = end != state.buffer;

                if (valid) {
                    while (*end == ' ' || *end == '\t') {
                        ++end;
                    }
                    valid = (*end == '\0');
                }
            }

            if (valid) {
                result = std::clamp(result, _opt.min, _opt.max);

                if (result != *_value) {
                    *_value = result;
                    changed = true;
                }
            }

            state.editing = false;
        }

        return changed;
    }

    // =========================================================
    // NORMAL WIDGET
    // =========================================================

    ImGui::ItemSize(bb, style.FramePadding.y);

    if (!ImGui::ItemAdd(bb, id)) {
        return false;
    }

    bool hovered = false;
    bool held = false;

    const bool pressed = ImGui::ButtonBehavior(
        bb, id, &hovered, &held,
        ImGuiButtonFlags_PressedOnClick
        );

    // =========================================================
    // DOUBLE CLICK -> TEXT EDITING
    // =========================================================

    if (pressed &&
        ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
    {
        state.editing = true;
        state.requestFocus = true;
        state.originalValue = *_value;

        ImFormatString(
            state.buffer,
            sizeof(state.buffer),
            "%.6g",
            *_value
            );

        return false;
    }

    // =========================================================
    // DRAGGING
    // =========================================================

    if (held)
    {
        float sensitivity = _opt.step;

        if (sensitivity <= 0.0f) {
            sensitivity = (_opt.max - _opt.min) / 200.0f;
        }

        if (g.IO.KeyShift) {
            sensitivity *= 0.1f;
        }

        if (g.IO.KeyCtrl) {
            sensitivity *= 10.0f;
        }

        const float delta = g.IO.MouseDelta.x;

        const float newValue = std::clamp(
            *_value + delta * sensitivity / 5.0f,
            _opt.min,
            _opt.max
            );

        if (newValue != *_value) {
            *_value = newValue;
            changed = true;
        }
    }

    // =========================================================
    // MOUSE WHEEL
    // =========================================================

    if (hovered && !held && g.IO.MouseWheel != 0.0f)
    {
        const float newValue = std::clamp(
            *_value + g.IO.MouseWheel * _opt.step,
            _opt.min,
            _opt.max
            );

        if (newValue != *_value)
        {
            *_value = newValue;
            changed = true;
        }
    }

    // =========================================================
    // RENDERING
    // =========================================================

    if (hovered) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
    }

    const ImU32 bgColor = ImGui::GetColorU32(
        held ? ImGuiCol_FrameBgActive :
            hovered ? ImGuiCol_FrameBgHovered :
            ImGuiCol_FrameBg
        );

    window->DrawList->AddRectFilled(
        bb.Min,
        bb.Max,
        bgColor,
        style.FrameRounding
        );

    char buffer[64];

    ImFormatString(buffer, sizeof(buffer), _opt.fmt, *_value);

    const ImVec2 textSize = ImGui::CalcTextSize(buffer);

    window->DrawList->AddText(
        ImVec2(
            bb.Min.x + (bb.GetWidth() - textSize.x) * 0.5f,
            bb.Min.y + (bb.GetHeight() - textSize.y) * 0.5f
            ),
        ImGui::GetColorU32(ImGuiCol_Text),
        buffer
        );

    if (_opt.display_name) {
        window->DrawList->AddText(
            ImVec2(
                bb.Max.x,
                bb.Min.y + (bb.GetHeight() - textSize.y) * 0.5f
                ),
            ImGui::GetColorU32(ImGuiCol_Text),
            _name
        );
    }

    return changed;
}

bool InputSlider2(
    const char* _name,
    float* _value,
    InputSliderOptions _opt)
{
    bool display_name = _opt.display_name;
    _opt.display_name = false;
    _opt.scale_width *= 0.5f;
    bool changed = false;
    changed |= InputSlider((std::to_string(*_name)+"_0_##xx").c_str(), &_value[0], _opt);
    ImGui::SameLine();
    _opt.display_name = display_name;
    changed |= InputSlider(_name, &_value[1], _opt);
    return changed;
}

bool InputSlider3(
    const char* _name,
    float* _value,
    InputSliderOptions _opt)
{
    bool display_name = _opt.display_name;
    _opt.display_name = false;
    _opt.scale_width *= 1.f/3.f;
    bool changed = false;
    changed |= InputSlider((std::to_string(*_name)+"_0_##xx").c_str(), &_value[0], _opt);
    ImGui::SameLine();
    changed |= InputSlider((std::to_string(*_name)+"_1_##xx").c_str(), &_value[1], _opt);
    ImGui::SameLine();
    _opt.display_name = display_name;
    changed |= InputSlider(_name, &_value[2], _opt);
    return changed;
}

}
