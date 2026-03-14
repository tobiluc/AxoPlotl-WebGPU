#pragma once

#include <AxoPlotl/rendering/ColorMap.hpp>
#include "imgui.h"
#include <AxoPlotl/typedefs/ovm.hpp>
#include <array>
#include <iostream>

namespace AxoPlotl
{

struct HistogramBase
{
public:
    virtual void recompute(size_t _n_buckets) = 0;

    inline int render_ui(
        int b_begin, int b_end,
        const ColorMap& _cm)
    {
        if (!render_ui_begin()) {return -1;}

        // Clamp bucket indices in range
        if (b_begin < 0) {b_begin = 0;}
        if (b_end > n_buckets_) {b_end = n_buckets_;}

        int selected_bucket(-1);

        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        const float total_width = ImGui::GetContentRegionAvail().x;
        const float bar_width = (0.8f*total_width)/n_buckets_;
        const float max_bar_height = 64.0f;
        const float spacing = (0.05f*total_width)/n_buckets_;
        const float label_height = ImGui::GetTextLineHeight();

        // Find max count for scaling
        size_t total_count = 0;
        size_t max_vis_count = 1;
        for (const size_t& count : bucket_counts_) {total_count += count;}
        for (int b = b_begin; b < b_end; ++b) {
            max_vis_count = std::max(max_vis_count, bucket_counts_.at(b));
        }

        // Reserve total space for the widget so it doesn't overlap later UI
        ImVec2 start_pos = ImGui::GetCursorScreenPos();
        ImGui::Dummy(ImVec2((bar_width + spacing) * n_buckets_, max_bar_height + label_height + 10.0f));

        for (size_t i = b_begin; i < b_end; ++i)
        {
            size_t count = bucket_counts_[i];

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
                ImGui::SetTooltip("%s", bucket_tooltip(i).c_str());
            }
            if (ImGui::IsItemClicked()) {
                selected_bucket = i;
            }
            ImGui::PopID();
        }
        return selected_bucket;
    }

    inline int render_ui(const ColorMap& _cm) {
        return render_ui(0, n_buckets_, _cm);
    }

    inline const size_t& n_buckets() const {return n_buckets_;}

    inline const size_t total_count() const {
        size_t count(0);
        for (int b = 0; b < n_buckets_; ++b) {
            count += bucket_count(b);
        }
        return count;
    }

    inline const size_t& bucket_count(int _b) const {
        return bucket_counts_[_b];
    }

    inline float bucket_rel_count(int _b) const {
        return static_cast<float>(bucket_count(_b)) / total_count();
    }

protected:
    virtual inline bool render_ui_begin() {return true;};
    virtual inline std::string bucket_tooltip(int _b) const {return "";};

    size_t n_buckets_ = 1;
    std::vector<size_t> bucket_counts_;
};

struct HistogramBool : public HistogramBase
{
public:
    HistogramBool(OVM::PropertyStorageT<bool>* _prop,
                size_t _n_buckets = 10) : prop_(_prop)
    {
        recompute(2);
    }

    inline void recompute(size_t) override
    {
        n_buckets_ = 2;
        bucket_counts_.resize(2);
        bucket_counts_[0] = bucket_counts_[1] = 0;
        for (uint32_t i = 0; i < prop_->size(); ++i) {
            bucket_counts_[1] += prop_->at(i);
        }
        bucket_counts_[0] = prop_->size() - bucket_counts_[1];
    }

    inline bool bucket_max(int _b) const {
        return _b!=0;
    }

    inline bool bucket_min(int _b) const {
        return _b!=0;
    }

    inline int find_bucket(const bool& _val) const {
        return static_cast<int>(_val);
    }

protected:
    OVM::PropertyStorageT<bool>* prop_;

    inline std::string bucket_tooltip(int _b) const override {
        char buf[32];
        snprintf(buf, sizeof(buf), "%s\n%zu (%f %%)", (_b>0)? "true" : "false",
            bucket_count(_b), 100.0f*bucket_rel_count(_b));
        return buf;
    }
};

template <typename IT> requires std::integral<IT>
struct HistogramIT : public HistogramBase
{
public:
    HistogramIT(OVM::PropertyStorageT<IT>* _prop,
                size_t _n_buckets = 10) : prop_(_prop)
    {
        recompute(_n_buckets);
    }

    inline void recompute(size_t _n_buckets) override
    {
        n_buckets_ = _n_buckets;

        if (prop_->size()==0) [[unlikely]] {return;}

        min_ = prop_->at(0);
        max_ = prop_->at(0);
        for (int i = 1; i < prop_->size(); ++i) {
            const IT& val = prop_->at(i);
            min_ = std::min(min_, val);
            max_ = std::max(max_, val);
        }

        n_buckets_ = std::clamp(n_buckets_, 1lu,
            static_cast<size_t>(max_-min_));

        bucket_counts_.clear();
        bucket_max_.clear();
        bucket_counts_.resize(n_buckets_, 0);
        bucket_max_.resize(n_buckets_, 0);
        int s = (max_-min_)/n_buckets_;
        int m = (max_-min_)%n_buckets_;
        for (int b = 0; b < n_buckets_; ++b) {
            bucket_max_[b] = min_ + (b+1)*s;
            if (b<m) {bucket_max_[b] += 1;}
        }
        for (const IT& v : prop_->data_vector()) {
            bucket_counts_[find_bucket(v)] += 1;
        }
    }

    inline IT bucket_min(int _b) const {
        return (_b==0)? min_ : bucket_max_[_b-1]+static_cast<IT>(1);
    }

    inline IT bucket_max(int _b) const {
        return bucket_max_[_b];
    }

    inline int find_bucket(const IT& _val) const {
        for (int b = 0; b < n_buckets_; ++b) {
            if (_val <= bucket_max(b)) {
                return b;
            }
        }
        return -1;
    }

    inline const IT& min() const {return min_;}
    inline const IT& max() const {return max_;}

protected:
    std::vector<IT> bucket_max_; // <=
    OVM::PropertyStorageT<IT>* prop_;
    IT min_;
    IT max_;

    inline std::string bucket_tooltip(int _b) const override {
        char buf[32];
        if (bucket_min(_b)==bucket_max(_b)) {
            snprintf(buf, sizeof(buf), "%s\n%zu (%f %%)",
                std::to_string(bucket_max(_b)).c_str(),
                bucket_count(_b), 100.0f*bucket_rel_count(_b));
        } else {
            snprintf(buf, sizeof(buf), "[%s,%s]\n%zu (%f %%)",
                std::to_string(bucket_min(_b)).c_str(),
                 std::to_string(bucket_max(_b)).c_str(),
                 bucket_count(_b), 100.0f*bucket_rel_count(_b));
        }
        return buf;
    }

    inline bool render_ui_begin() override {
        // Set Number of Buckets
        int n_buckets = n_buckets_;
        if (ImGui::InputInt("##", &n_buckets, 1)) {
            recompute(n_buckets);
        }
        return true;
    }
};

template <typename FT> requires std::floating_point<FT>
struct HistogramFT : public HistogramBase
{
public:
    HistogramFT(OVM::PropertyStorageT<FT>* _prop,
              size_t _n_buckets = 10) : prop_(_prop)
    {
        recompute(_n_buckets);
    }

    inline void recompute(size_t _n_buckets) override
    {
        n_buckets_ = _n_buckets;

        n_nan_ = n_inf_ = 0;
        min_ = std::numeric_limits<FT>::infinity();
        max_ = -std::numeric_limits<FT>::infinity();
        for (int i = 0; i < prop_->size(); ++i) {
            const FT& val = prop_->at(i);
            if (std::isnan(val)) [[unlikely]] {++n_nan_;}
            else if (std::isinf(val)) [[unlikely]] {++n_inf_;}
            else {
                min_ = std::min(min_, val);
                max_ = std::max(max_, val);
            }
        }

        n_buckets_ = std::clamp(n_buckets_, 1lu, 100lu);
        bucket_counts_.clear();
        bucket_counts_.resize(n_buckets_, 0);
        bucket_max_.resize(n_buckets_);
        const FT bucket_size = (max_ - min_) / n_buckets_;
        bucket_max_.back() = max_;
        for (int i = n_buckets_-2; i >= 0; --i) {
            bucket_max_[i] = bucket_max_[i+1] - bucket_size;
        }
        for (const FT& v : prop_->data_vector()) {
            for (int b = 0; b < n_buckets_; ++b) {
                if (v <= bucket_max_[b]) {
                    bucket_counts_[b] += 1;
                    break;
                }
            }
        }
    }

    // inclusive
    inline FT bucket_min(int _b) const {
        return (_b==0)? this->min_ : bucket_max_[_b-1];
    }

    // inclusive
    inline FT bucket_max(int _b) const {
        return (_b==n_buckets_-1)? max_ :
            (std::nextafterf(bucket_max_[_b], -std::numeric_limits<FT>::infinity()));
    }

    inline bool has_valid_values() const {
        return n_nan_ + n_inf_ < prop_->size();
    }

    inline int find_bucket(const FT& _val) const {
        for (int b = 0; b < n_buckets_; ++b) {
            if (_val <= bucket_max(b)) {
                return b;
            }
        }
        return -1;
    }

    inline const FT& min() const {return min_;}
    inline const FT& max() const {return max_;}

protected:
    size_t n_nan_ = 0;
    size_t n_inf_ = 0;
    std::vector<FT> bucket_max_; // inclusive
    OVM::PropertyStorageT<FT>* prop_;
    FT min_;
    FT max_;

    inline bool render_ui_begin() override {
        if (n_nan_ > 0) {ImGui::Text("#NAN = %zu", n_nan_);}
        if (n_nan_ && n_inf_) {ImGui::SameLine();}
        if (n_nan_ > 0) {ImGui::Text("#INF = %zu", n_inf_);}

        // Set Number of Buckets
        int n_buckets = n_buckets_;
        if (ImGui::InputInt("##", &n_buckets, 1)) {
            recompute(n_buckets);
        }

        return has_valid_values();
    }

    inline std::string bucket_tooltip(int _b) const override {
        char buf[32];
        snprintf(buf, sizeof(buf), "[%s,%s)\n%zu (%f %%)",
            std::to_string(bucket_min(_b)).c_str(),
            std::to_string(bucket_max(_b)).c_str(),
            bucket_count(_b),
            100.0f*bucket_rel_count(_b));
        return buf;
    }
};

// template<typename T>
// struct Histogram
// {
//     Histogram(OVM::PropertyStorageT<T>* _prop,
//               size_t _n_buckets = 10) : prop_(_prop)
//     {
//         recompute(_n_buckets);
//     }

//     void recompute(size_t _n_buckets)
//     {
//         n_nan = n_inf = 0;

//         // Compute Range
//         if (prop_->size() == 0) {
//             any_valid_ = false;
//             min_ = T{0};
//             max_ = T{0};
//         } else if constexpr(std::is_same_v<T,bool>) {
//             any_valid_ = true;
//             min_ = false;
//             max_ = true;
//         } else if constexpr(std::is_floating_point_v<T>) {
//             // Compute Range while counting NaNs and Infs
//             any_valid_ = false;
//             min_ = std::numeric_limits<T>::infinity();
//             max_ = -std::numeric_limits<T>::infinity();
//             for (int i = 0; i < prop_->size(); ++i) {
//                 const T& val = prop_->at(i);
//                 if (std::isnan(val)) {++n_nan;}
//                 else if (std::isinf(val)) {++n_inf;}
//                 else {
//                     min_ = std::min(min_, val);
//                     max_ = std::max(max_, val);
//                     any_valid_ = true;
//                     continue;
//                 }
//             }
//         } else {
//             // Compute Range for integer type
//             any_valid_ = true;
//             min_ = prop_->at(0);
//             max_ = prop_->at(0);
//             for (int i = 1; i < prop_->size(); ++i) {
//                 const T& val = prop_->at(i);
//                 min_ = std::min(min_, val);
//                 max_ = std::max(max_, val);
//             }
//         }

//         // Determine number of buckets
//         _n_buckets = std::clamp(_n_buckets, 1lu, 100lu);
//         if constexpr(std::is_same_v<T, bool>) {
//             n_buckets_ = 2;
//         } else {
//             for (n_buckets_ = _n_buckets; n_buckets_ > 1; --n_buckets_) {
//                 // Want bucket size > 0
//                 if ((max_ - min_)/n_buckets_ > 0) {
//                     break;
//                 }
//             }
//         }

//         // Compute Buckets
//         bucket_counts_.resize(n_buckets_);
//         bucket_max_.resize(n_buckets_);
//         std::fill(bucket_counts_.begin(), bucket_counts_.end(), 0u);

//         if constexpr(std::is_same_v<T,bool>) {
//             bucket_max_[0] = false;
//             bucket_max_[1] = true;
//             for (bool v : prop_->data_vector()) {
//                 bucket_counts_[static_cast<int>(v)] += 1;
//             }
//         }
//         else
//         {
//             const T s = static_cast<float>(max_ - min_) / n_buckets_;
//             bucket_max_.back() = max_;
//             for (int i = n_buckets_-2; i >= 0; --i) {
//                 bucket_max_[i] = bucket_max_[i+1] - s;
//             }

//             for (const T& v : prop_->data_vector()) {
//                 for (int b = 0; b < n_buckets_; ++b) {
//                     if (v <= bucket_max_[b]) {
//                         bucket_counts_[b] += 1;
//                         break;
//                     }
//                 }
//             }
//         }
//     }

//     size_t n_buckets_ = 1;
//     T min_;
//     T max_;
//     std::vector<size_t> bucket_counts_;
//     std::vector<T> bucket_max_; // inclusive
//     size_t n_nan = 0;
//     size_t n_inf = 0;
//     bool any_valid_ = true;
//     OVM::PropertyStorageT<T>* prop_;

//     inline T bucket_min(int _b) const {
//         if constexpr(std::is_same_v<T,bool>) {return (_b>0)? true : false;}
//         else {return (_b==0)? min_ : bucket_max_[_b-1];}
//     }

//     // range 0-1 to type value
//     inline T interpolate(float _t) const {
//         _t = std::clamp(_t, 0.0f, 1.0f);
//         if constexpr(std::is_same_v<T,bool>) {return (_t<0.5f)? false : true;}
//         else {return static_cast<T>(static_cast<float>(min_)+_t*static_cast<float>(max_-min_));}
//     }

//     // float value to range 0-1
//     inline float interpolation_t(float _f) const {
//         const float minf = static_cast<float>(min_);
//         const float maxf = static_cast<float>(max_);
//         return std::clamp((_f - minf) / (maxf-minf), 0.0f, 1.0f);
//     }

//     inline friend std::ostream& operator<<(std::ostream& _os, const Histogram<T>& _h) {
//         _os << "Range: " << _h.min_ << " - " << _h.max_ << std::endl;
//         _os << "Buckets:" << std::endl;
//         for (int b = 0; b < _h.n_buckets_; ++b) {
//             _os << b << ": " << " <= "
//                 << _h.bucket_max_[b] << " (#=" << _h.bucket_counts_[b] << ")" << std::endl;
//         }
//         return _os;
//     }

//     inline int bucket(const T& _val) const {
//         if constexpr(std::is_same_v<T,bool>) {return _val? 1 : 0;}
//         if (_val < min_) {return -1;}
//         int b(0);
//         for (b = 0; b < n_buckets_; ++b) {
//             if (_val <= bucket_max_[b]) {
//                 break;
//             }
//         }
//         return b;
//     }

//     inline int render_ui(
//         int b_begin, int b_end,
//         const ColorMap& _cm)
//     {
//         if (!any_valid_) {return -1;}

//         {
//             // Set Number of Buckets
//             int n_buckets = n_buckets_;
//             if (ImGui::InputInt("##", &n_buckets, 1)) {
//                 recompute(n_buckets);
//             }
//         }

//         // Show NaNs and Infs inf floating type
//         if constexpr(std::is_floating_point_v<T>) {
//             if (n_nan > 0) {ImGui::Text("#NAN = %zu", n_nan);}
//             if (n_nan && n_inf) {ImGui::SameLine();}
//             if (n_inf > 0) {ImGui::Text("#INF = %zu", n_inf);}
//         }

//         // Clamp bucket indices in range
//         if (b_begin < 0) {b_begin = 0;}
//         if (b_end > n_buckets_) {b_end = n_buckets_;}

//         int selected_bucket(-1);

//         ImDrawList* draw_list = ImGui::GetWindowDrawList();

//         const float total_width = ImGui::GetContentRegionAvail().x;
//         const float bar_width = (0.8f*total_width)/n_buckets_;
//         const float max_bar_height = 64.0f;
//         const float spacing = (0.05f*total_width)/n_buckets_;
//         const float label_height = ImGui::GetTextLineHeight();

//         // Find max count for scaling
//         size_t total_count = 0;
//         size_t max_vis_count = 1;
//         for (const size_t& count : bucket_counts_) {total_count += count;}
//         for (int b = b_begin; b < b_end; ++b) {
//             max_vis_count = std::max(max_vis_count, bucket_counts_[b]);
//         }

//         // Reserve total space for the widget so it doesn't overlap later UI
//         ImVec2 start_pos = ImGui::GetCursorScreenPos();
//         ImGui::Dummy(ImVec2((bar_width + spacing) * n_buckets_, max_bar_height + label_height + 10.0f));

//         for (size_t i = b_begin; i < b_end; ++i)
//         {
//             size_t count = bucket_counts_[i];

//             // Calculate the top-left of this specific bar's slot
//             ImVec2 bar_slot_pos = ImVec2(start_pos.x + i * (bar_width + spacing), start_pos.y);

//             // Color Calculation. Interpolate in color map
//             const ImU32 bar_color = _cm.sample_color_packed(
//                 (b_end-b_begin==1)? 0.5f :
//                 (static_cast<float>(i)-b_begin)/(b_end-b_begin-1));

//             // Draw the Bar Rectangle
//             const float h = (static_cast<float>(count) / max_vis_count) * max_bar_height;
//             draw_list->AddRectFilled(
//                 ImVec2(bar_slot_pos.x, bar_slot_pos.y + (max_bar_height - h)),
//                 ImVec2(bar_slot_pos.x + bar_width, bar_slot_pos.y + max_bar_height),
//                 bar_color,
//                 3.0f, ImDrawFlags_RoundCornersTop
//                 );

//             // Invisible Button for Tooltips (placed exactly over the bar)
//             ImGui::SetCursorScreenPos(bar_slot_pos);
//             ImGui::PushID(i);
//             ImGui::InvisibleButton("##bar_hitbox", ImVec2(bar_width, max_bar_height));

//             // Draw Range and amount of bucket when hovering
//             if (ImGui::IsItemHovered()) {
//                 const float percentage = 100.0f*static_cast<float>(count)/total_count;
//                 if constexpr(std::is_same_v<bool,T>) {
//                     ImGui::SetTooltip(std::string("%s\n%zu (%.2f %%)").c_str(),
//                       (i==0)? "false" : "true",
//                       count,
//                       percentage);
//                 } else if (bucket_max_[i]==bucket_min(i)) {
//                     ImGui::SetTooltip(std::string("%s\n%zu (%.2f %%)").c_str(),
//                       std::to_string(bucket_min(i)).c_str(),
//                       count,
//                       percentage);
//                 } else {
//                     ImGui::SetTooltip((std::string("[%s, %s")
//                        + ((i==n_buckets_-1)? "]" : ")")
//                        + "\n%zu (%.2f %%)").c_str(),
//                       std::to_string(bucket_min(i)).c_str(),
//                       std::to_string(bucket_max_[i]).c_str(),
//                       count,
//                       percentage);
//                 }

//             }
//             if (ImGui::IsItemClicked()) {
//                 selected_bucket = i;
//             }
//             ImGui::PopID();
//         }
//         return selected_bucket;
//     }

//     inline int render_ui(const ColorMap& _cm) {
//         return render_ui(0, n_buckets_, _cm);
//     }
// };

}
