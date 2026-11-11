#pragma once

#include <glm/ext/vector_float4.hpp>
#include <string>
#include <vector>

namespace AxoPlotl::internal
{

struct IntervalSet {
    std::vector<std::pair<float, float>> intervals;

    constexpr bool matches(float value) const {
        for (const auto& [min, max] : intervals) {
            if (value >= min && value <= max) {
                return true;
            }
        }
        return false;
    }
};

struct VisualizationStyle {
    glm::vec4 color[2] = {{1.f,0.f,0.f,1.f}, {1.f,0.f,0.f,1.f}};
    float point_size[2] = {1.0f, 1.0f};
    float line_width[2] = {5.0f, 5.0f};
    float cell_scale[2] = {0.9f, 0.9f};
};



struct VisualizationRule {
    std::string name = "Rule";
    IntervalSet condition;
    VisualizationStyle style;
    bool enabled = true;
};

struct VisualizationSettings
{
    std::vector<VisualizationRule> rules;
    VisualizationStyle default_style;

    void render_ui();
};

}
