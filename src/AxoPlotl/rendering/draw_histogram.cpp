#include <AxoPlotl/rendering/draw_histogram.hpp>
#include <imgui.h>
#include <AxoPlotl/properties/Histogram.hpp>

namespace AxoPlotl
{

int draw_histogram(const HistogramBase* _hist,
    int b_begin, int b_end,
    const ColorMap& _cm)
{
    // Clamp bucket indices in range
    if (b_begin < 0) {b_begin = 0;}
    if (b_end > _hist->n_buckets()) {b_end = _hist->n_buckets();}

    int selected_bucket(-1);

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    const float total_width = ImGui::GetContentRegionAvail().x;
    const float bar_width = (0.8f*total_width)/_hist->n_buckets();
    const float max_bar_height = 64.0f;
    const float spacing = (0.05f*total_width)/_hist->n_buckets();
    const float label_height = ImGui::GetTextLineHeight();

    // Find max count for scaling
    size_t total_count = _hist->total_count();
    size_t max_vis_count = 1;
    for (int b = b_begin; b < b_end; ++b) {
        max_vis_count = std::max(max_vis_count, _hist->bucket_count(b));
    }

    // Reserve total space for the widget so it doesn't overlap later UI
    ImVec2 start_pos = ImGui::GetCursorScreenPos();
    ImGui::Dummy(ImVec2((bar_width + spacing) * _hist->n_buckets(), max_bar_height + label_height + 10.0f));

    for (size_t i = b_begin; i < b_end; ++i)
    {
        size_t count = _hist->bucket_count(i);

        // Calculate the top-left of this specific bar's slot
        ImVec2 bar_slot_pos = ImVec2(start_pos.x + i * (bar_width + spacing), start_pos.y);

        // Color Calculation. Interpolate in color map
        const ImU32 bar_color = _cm.sample_color_packed(
            (b_end-b_begin==1)? 0.5f :
                (static_cast<float>(i)-b_begin)/(b_end-b_begin-1));

        // Draw the Bar Rectangle
        const float h = (static_cast<float>(count) / max_vis_count) * max_bar_height;
        draw_list->AddRectFilled(
            ImVec2(bar_slot_pos.x, bar_slot_pos.y + (max_bar_height - h)),
            ImVec2(bar_slot_pos.x + bar_width, bar_slot_pos.y + max_bar_height),
            bar_color,
            3.0f, ImDrawFlags_RoundCornersTop
            );

        // Invisible Button for Tooltips (placed exactly over the bar)
        ImGui::SetCursorScreenPos(bar_slot_pos);
        ImGui::PushID(i);
        ImGui::InvisibleButton("##bar_hitbox", ImVec2(bar_width, max_bar_height));

        // Draw Range and amount of bucket when hovering
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s", _hist->bucket_tooltip(i).c_str());
        }
        if (ImGui::IsItemClicked()) {
            selected_bucket = i;
        }
        ImGui::PopID();
    }
    return selected_bucket;
}

int draw_histogram(const HistogramBase* _hist, const ColorMap& _cm)
{
    return draw_histogram(_hist, 0, _hist->n_buckets(), _cm);
}

}
