#pragma once
#include <glm/common.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace AxoPlotl
{
    using FT = float; // should always be float
    using Vec2 = glm::vec<2,FT>;
    using Vec3 = glm::vec<3,FT>;
    using Vec4f = glm::vec<4,FT>;
    using Vec4 = glm::vec<4,FT>;
    using Mat3x3 = glm::mat<3,3,FT>;
    using Mat4x4f = glm::mat<4,4,FT>;
    using Mat4x4 = glm::mat<4,4,FT>;
    using Quat = glm::quat;

    struct ViewSize {
        float width = 1;
        float height = 1;
        float aspect_ratio() const {return width/height;}
    };

    struct VP {
        Mat4x4 view;
        Mat4x4 proj;
        Mat4x4 viewproj() const {return proj*view;}
    };
    using ViewProjection = VP;

    struct MVP {
        Mat4x4 model;
        Mat4x4 view;
        Mat4x4 proj;
        Mat4x4 modelview() const {return view*model;}
        Mat4x4 viewproj() const {return proj*view;}
        Mat4x4 modelviewproj() const {return proj*view*model;}
    };
    using ModelViewProjection = MVP;
}
