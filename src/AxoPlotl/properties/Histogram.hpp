#pragma once

#include <AxoPlotl/rendering/ColorMap.hpp>
#include "imgui.h"
#include <AxoPlotl/typedefs/ovm.hpp>
#include <array>
#include <iostream>

namespace AxoPlotl
{

template<typename T>
struct Histogram
{
    Histogram(OVM::PropertyStorageT<T>* _prop,
              size_t _n_buckets = 10) : prop_(_prop)
    {
        recompute(_n_buckets);
    }

    void recompute(size_t _n_buckets)
    {
        // Compute Range
        if (prop_->size() == 0) {
            min_ = T{0};
            max_ = T{0};
        } else if constexpr(std::is_same_v<T,bool>) {
            min_ = false;
            max_ = true;
        } else {
            min_ = prop_->at(0);
            max_ = prop_->at(0);
            for (const T& v : prop_->data_vector()) {
                min_ = std::min(min_, v);
                max_ = std::max(max_, v);
            }
        }

        // Determine number of buckets
        _n_buckets = std::clamp(_n_buckets, 1lu, 100lu);
        if constexpr(std::is_same_v<T, bool>) {
            n_buckets_ = 2;
        } else {
            for (n_buckets_ = _n_buckets; n_buckets_ > 1; --n_buckets_) {
                // Want bucket size > 0
                if ((max_ - min_)/n_buckets_ > 0) {
                    break;
                }
            }
        }

        // Compute Buckets
        bucket_counts_.resize(n_buckets_);
        bucket_max_.resize(n_buckets_);
        std::fill(bucket_counts_.begin(), bucket_counts_.end(), 0u);

        if constexpr(std::is_same_v<T,bool>) {
            bucket_max_[0] = false;
            bucket_max_[1] = true;
            for (bool v : prop_->data_vector()) {
                bucket_counts_[static_cast<int>(v)] += 1;
            }
        }
        else
        {
            const T s = static_cast<float>(max_ - min_) / n_buckets_;
            bucket_max_.back() = max_;
            for (int i = n_buckets_-2; i >= 0; --i) {
                bucket_max_[i] = bucket_max_[i+1] - s;
            }

            for (const T& v : prop_->data_vector()) {
                for (int b = 0; b < n_buckets_; ++b) {
                    if (v <= bucket_max_[b]) {
                        bucket_counts_[b] += 1;
                        break;
                    }
                }
            }
        }
    }

    size_t n_buckets_ = 1;
    T min_;
    T max_;
    std::vector<size_t> bucket_counts_;
    std::vector<T> bucket_max_; // inclusive
    OVM::PropertyStorageT<T>* prop_;

    inline T bucket_min(int _b) const {
        return (_b==0)? min_ : bucket_max_[_b-1];
    }

    inline friend std::ostream& operator<<(std::ostream& _os, const Histogram<T>& _h) {
        _os << "Range: " << _h.min_ << " - " << _h.max_ << std::endl;
        _os << "Buckets:" << std::endl;
        for (int b = 0; b < _h.n_buckets_; ++b) {
            _os << b << ": " << " <= "
                << _h.bucket_max_[b] << " (#=" << _h.bucket_counts_[b] << ")" << std::endl;
        }
        return _os;
    }

    inline int render_ui(
        const ColorMap& _cm)
    {
        {
            // Set Number of Buckets
            int n_buckets = n_buckets_;
            if (ImGui::InputInt("#Buckets", &n_buckets, 1)) {
                recompute(n_buckets);
            }
        }

        int selected_bucket(-1);

        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        const float total_width = ImGui::GetContentRegionAvail().x;
        const float bar_width = (0.8f*total_width)/n_buckets_;
        const float max_bar_height = 64.0f;
        const float spacing = (0.05f*total_width)/n_buckets_;
        const float label_height = ImGui::GetTextLineHeight();

        // Find max count for scaling
        size_t total_count = 0;
        size_t max_count = 1;
        for (const size_t& count : bucket_counts_) {
            total_count += count;
            max_count = std::max(max_count, count);
        }

        // Reserve total space for the widget so it doesn't overlap later UI
        ImVec2 start_pos = ImGui::GetCursorScreenPos();
        ImGui::Dummy(ImVec2((bar_width + spacing) * n_buckets_, max_bar_height + label_height + 10.0f));

        for (size_t i = 0; i < n_buckets_; ++i)
        {
            size_t count = bucket_counts_[i];

            // Calculate the top-left of this specific bar's slot
            ImVec2 bar_slot_pos = ImVec2(start_pos.x + i * (bar_width + spacing), start_pos.y);

            // Color Calculation
            const ImU32 bar_color = _cm.sample_color_packed(
                (n_buckets_==1)? 0.5f : static_cast<float>(i)/(n_buckets_-1));

            // Draw the Bar Rectangle
            const float h = (static_cast<float>(count) / max_count) * max_bar_height;
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
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Range %.2f - %.2f\nCount: %zu (%.2f %%)",
                    static_cast<float>(bucket_min(i)),
                    static_cast<float>(bucket_max_[i]),
                    count,
                    100.0f*static_cast<float>(count)/total_count);
            }
            if (ImGui::IsItemClicked()) {
                selected_bucket = i;
            }
            ImGui::PopID();

            // Label below the bar
            const std::string label = std::to_string(bucket_max_[i]);
            float text_width = ImGui::CalcTextSize(label.c_str()).x;
            float text_x_offset = (bar_width - text_width) * 0.5f;
            draw_list->AddText(
                ImVec2(bar_slot_pos.x + text_x_offset, bar_slot_pos.y + max_bar_height + 5.0f),
                IM_COL32_WHITE,
                label.c_str()
            );
        }
        return selected_bucket;
    }
};

}
