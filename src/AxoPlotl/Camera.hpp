#pragma once

#include "AxoPlotl/typedefs/glm.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include <GLFW/glfw3.h>

namespace AxoPlotl
{

class PerspectiveCamera
{
private:
    static constexpr glm::vec3 world_up = glm::vec3(0,1,0);
    static constexpr float near = 0.01f;
    static constexpr float far = 4096.0f;
    float sensitivity_ = 0.001f;
    float pan_speed_ = 2.0f;

    Vec3f orbit_target_ = glm::vec3(0.0f);
    float orbit_distance_ = 1.0f;
    Vec3f position_;
    Vec3f up_;
    float yaw_ = 0; // Euler Angles in radians
    float pitch_ = 0;
    float fov_ = 0.25*M_PI; // field of view in radians

public:
    inline glm::mat4 getViewMatrix() const {
        return lookAt(position_, orbit_target_, up_);
    }

    inline glm::mat4 getProjectionMatrix(float width_over_height) const {
        return glm::perspective(fov_, width_over_height, near, far);
    }

    void update(GLFWwindow* window);

    void reset(GLFWwindow* window);

    void zoom_to_box(const glm::vec3& min, const glm::vec3& max);
};

}
