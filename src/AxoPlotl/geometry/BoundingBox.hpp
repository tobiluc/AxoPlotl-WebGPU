#pragma once

#include <glm/geometric.hpp>
#include <AxoPlotl/typedefs/glm.hpp>
#include <algorithm>
#include <span>

namespace AxoPlotl
{

class BoundingBox
{
public:
    BoundingBox() {
        reset();
    }

    inline void reset() {
        for (int i = 0; i < 3; ++i) {
            min_[i] = std::numeric_limits<float>::infinity();
            max_[i] = -std::numeric_limits<float>::infinity();
        }
    }

    template<typename Vec3T>
    inline void expand_with_point(const Vec3T& _p) {
        for (int i = 0; i < 3; ++i) {
            if (std::isfinite(_p[i])) {
                min_[i] = std::min(min_[i], _p[i]);
                max_[i] = std::max(max_[i], _p[i]);
            }
        }
    }

    inline const Vec3f& min() const {return min_;}
    inline const Vec3f& max() const {return max_;}

    inline constexpr float diagonal() const {
        return glm::length(max_ - min_);
    }

private:
    Vec3f min_;
    Vec3f max_;
};

}
