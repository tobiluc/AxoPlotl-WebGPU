#pragma once

#include <AxoPlotl/rendering/ColorMap.hpp>
#include "imgui.h"
#include <AxoPlotl/typedefs/ovm.hpp>
#include <array>
#include <iostream>

namespace AxoPlotl::internal
{

// template<typename T>
// requires(std::is_scalar_v<T>)
// struct BucketT
// {
//     size_t n_elements = 0;
//     T min = 0;
//     bool min_inclusive = true;
//     T max = 0;
//     bool max_inclusive = true;
//     double percentage(size_t _total_n_elements) const {
//         return static_cast<double>(n_elements)/static_cast<double>(_total_n_elements);
//     }
//     bool contains(const T& _v) const {
//         bool bmin = min_inclusive? (_v >= min) : (_v > min);
//         bool bmax = max_inclusive? (_v <= max) : (_v < max);
//         return bmin && bmax;
//     }
// };

// template<typename T>
// class HistogramT
// {
// protected:
//     const std::vector<T>& data_;
//     std::vector<BucketT<T>> buckets_;
//     size_t n_inf_ = 0;
//     size_t n_nan_ = 0;
//     T global_min_inclusive;
//     T global_max_inclusive;
// public:
//     HistogramT(const std::vector<T>& _data, size_t _n_buckets = 10)
//         : data_(_data)
//     {
//         recompute(_n_buckets);
//     }

//     virtual void recompute(size_t _n_buckets) = 0;

//     std::optional<int> find_bucket_index(const T& _v) const {
//         for (int i = 0; i < buckets_.size(); ++i) {
//             if (buckets_[i].contains(_v)) {
//                 return i;
//             }
//         }
//         return std::nullopt;
//     }
// };

// class NewHistogramBool : public HistogramT<bool>
// {
//     void recompute(size_t _n_buckets) override
//     {
//         buckets_.clear();
//         n_nan_ = n_inf_ = 0;
//         global_min_inclusive = false;
//         global_max_inclusive = true;
//         buckets_.resize(2);
//         buckets_[0].n_elements = std::count(data_.begin(), data_.end(), false);
//         buckets_[0].min = false;
//         buckets_[0].max = false;
//         buckets_[0].min_inclusive = true;
//         buckets_[0].max_inclusive = true;
//         buckets_[1].n_elements = data_.size() - buckets_[0].n_elements;
//         buckets_[1].min = true;
//         buckets_[1].max = true;
//         buckets_[1].min_inclusive = true;
//         buckets_[1].max_inclusive = true;
//     }
// };

// template<typename FT>
// class NewHistogramFloat : public HistogramT<FT>
// {
//     void recompute(size_t _n_buckets) override
//     {
//         buckets_.clear();
//         n_nan_ = n_inf_ = 0;
//         global_min_inclusive = std::numeric_limits<FT>::max();
//         global_max_inclusive = std::numeric_limits<FT>::min();
//         for (uint32_t i = 0; i < data_.size(); ++i) {
//             const FT& v = data_[i];
//             if (std::isnan(v)) {++n_nan_;}
//             else if (std::isinf(v)) {++n_inf_;}
//             else {
//                 global_min_inclusive = std::min(global_min_inclusive, v);
//                 global_max_inclusive = std::max(global_max_inclusive, v);
//             }
//         }
//         if (n_nan_ + n_inf_ == data_.size()) {
//             // NO VALID VALUE EXISTS
//             return;
//         }
//         buckets_.resize(get_optimal_n_buckets(_n_buckets));
//     }
// };

struct HistogramBase
{
public:
    virtual void recompute(size_t _n_buckets) = 0;

    const size_t& n_buckets() const {return n_buckets_;}

    const size_t total_count() const {
        size_t count(0);
        for (int b = 0; b < n_buckets_; ++b) {
            count += bucket_count(b);
        }
        return count;
    }

    const size_t& bucket_count(int _b) const {
        return bucket_counts_[_b];
    }

    float bucket_rel_count(int _b) const {
        return static_cast<float>(bucket_count(_b)) / total_count();
    }

    virtual std::string bucket_tooltip(int _b) const {return "";};

protected:
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

    void recompute(size_t) override
    {
        n_buckets_ = 2;
        bucket_counts_.resize(2);
        bucket_counts_[0] = bucket_counts_[1] = 0;
        for (uint32_t i = 0; i < prop_->size(); ++i) {
            bucket_counts_[1] += prop_->at(i);
        }
        bucket_counts_[0] = prop_->size() - bucket_counts_[1];
    }

    bool bucket_max(int _b) const {
        return _b!=0;
    }

    bool bucket_min(int _b) const {
        return _b!=0;
    }

    int find_bucket(const bool& _val) const {
        return static_cast<int>(_val);
    }

protected:
    OVM::PropertyStorageT<bool>* prop_;

    std::string bucket_tooltip(int _b) const override {
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

    void recompute(size_t _n_buckets) override
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
            std::max(static_cast<size_t>(max_-min_),1lu));

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
            int b = find_bucket(v);
            if (b >= 0) {bucket_counts_[b] += 1;}
        }
    }

    IT bucket_min(int _b) const {
        return (_b==0)? min_ : bucket_max_[_b-1]+static_cast<IT>(1);
    }

    IT bucket_max(int _b) const {
        return bucket_max_[_b];
    }

    int find_bucket(const IT& _val) const {
        for (int b = 0; b < n_buckets_; ++b) {
            if (_val <= bucket_max(b)) {
                return b;
            }
        }
        return -1;
    }

    const IT& min() const {return min_;}
    const IT& max() const {return max_;}

    std::string bucket_tooltip(int _b) const override {
        char buf[64];
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

protected:
    std::vector<IT> bucket_max_;
    OVM::PropertyStorageT<IT>* prop_;
    IT min_;
    IT max_;
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

    void recompute(size_t _n_buckets) override
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
    FT bucket_min(int _b) const {
        return (_b==0)? this->min_ : bucket_max_[_b-1];
    }

    // inclusive
    FT bucket_max(int _b) const {
        return (_b==n_buckets_-1)? max_ :
            (std::nextafterf(bucket_max_[_b], -std::numeric_limits<FT>::infinity()));
    }

    bool has_valid_values() const {
        return n_nan_ + n_inf_ < prop_->size();
    }

    int find_bucket(const FT& _val) const {
        for (int b = 0; b < n_buckets_; ++b) {
            if (_val <= bucket_max(b)) {
                return b;
            }
        }
        return -1;
    }

    const FT& min() const {return min_;}
    const FT& max() const {return max_;}

    std::string bucket_tooltip(int _b) const override {
        char buf[128];
        snprintf(buf, sizeof(buf), "[%s,%s)\n%zu (%f %%)",
                 std::to_string(bucket_min(_b)).c_str(),
                 std::to_string(bucket_max(_b)).c_str(),
                 bucket_count(_b),
                 100.0f*bucket_rel_count(_b));
        return buf;
    }

protected:
    size_t n_nan_ = 0;
    size_t n_inf_ = 0;
    std::vector<FT> bucket_max_; // inclusive
    OVM::PropertyStorageT<FT>* prop_;
    FT min_;
    FT max_;
};

}
