#pragma once
#include <glm/common.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace AxoPlotl
{
    using Vec2i = glm::vec<2,int>;
    using Vec2f = glm::vec<2,float>;
    using Vec2d = glm::vec<2,double>;
    using Vec3f = glm::vec<3,float>;
    using Vec4f = glm::vec<4,float>;
    using Mat3x3f = glm::mat<3,3,float>;
    using Mat4x4f = glm::mat<4,4,float>;
    using Vec4ui = glm::vec<4,uint32_t>;
    using Quat = glm::quat;

    struct ViewSize {
        float width = 1;
        float height = 1;
        float aspect_ratio() const {return width/height;}
    };

    struct VP {
        Mat4x4f view;
        Mat4x4f proj;
        Mat4x4f viewproj() const {return proj*view;}
    };
    using ViewProjection = VP;

    struct MVP {
        Mat4x4f model;
        Mat4x4f view;
        Mat4x4f proj;
        Mat4x4f modelview() const {return view*model;}
        Mat4x4f viewproj() const {return proj*view;}
        Mat4x4f modelviewproj() const {return proj*view*model;}
    };
    using ModelViewProjection = MVP;
}
