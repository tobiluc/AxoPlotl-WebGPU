#pragma once

#include <cstdlib>
#include <limits>

namespace AxoPlotl
{

using InputSliderParser = bool(*)(const char*, float&);

struct InputSliderOptions
{
    float step = 0.01f;
    float min = -std::numeric_limits<float>::infinity();
    float max = std::numeric_limits<float>::infinity();
    InputSliderParser parser = nullptr;
    bool display_name = true;
    float scale_width = 1.0f;
    const char* fmt = "%.3f";
};

bool InputSlider(
    const char* _name,
    float* _value,
    InputSliderOptions _opt = InputSliderOptions());

bool InputSlider2(
    const char* _name,
    float* _value,
    InputSliderOptions _opt = InputSliderOptions());

bool InputSlider3(
    const char* _name,
    float* _value,
    InputSliderOptions _opt = InputSliderOptions());

}
