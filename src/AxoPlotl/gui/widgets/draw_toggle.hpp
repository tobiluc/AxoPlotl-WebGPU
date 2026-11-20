#pragma once
#include <imgui.h>
#include <AxoPlotl/gui/fonts.hpp>

namespace AxoPlotl::gui
{

struct ToggleOptions
{
    const char* label_on = ICON_FA_EYE;
    const char* label_off = ICON_FA_EYE_SLASH;
};

template<typename T>
bool Toggle(T& _value, const ToggleOptions& _opt = ToggleOptions())
{
    bool changed = false;
    bool b = static_cast<bool>(_value);
    if (ImGui::Button(b? _opt.label_on : _opt.label_off)) {
        b = !b;
        changed = true;
    }
    _value = static_cast<T>(b);
    return changed;
}

}
