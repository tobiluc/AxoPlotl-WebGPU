#pragma once

#include <AxoPlotl/typedefs/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <GLFW/glfw3.h>

namespace AxoPlotl
{

class PerspectiveCamera
{

private:
    static constexpr glm::vec3 UP = {0,1,0};
    static constexpr glm::vec3 RIGHT = {1,0,0};
    static constexpr glm::vec3 FORWARD = {0,0,-1};

    float sensitivity_ = 0.005f;
    float pan_speed_ = 2.0f;

    float near_plane_ = 1.0f/64.0f;
    float far_plane_ = (1<<11);

    Vec3f orbit_target_ = {0, 0, 0};
    Vec3f eye_position_ = {0, 0, 1};
    Quat orientation_ = {1, 0, 0, 0};

    float fov_ = 0.25*M_PI; // field of view in radians

public:
    glm::mat4 getViewMatrix() const {
        return lookAt(eye_position_, orbit_target_, orientation_ * UP);
    }

    glm::mat4 getProjectionMatrix(float width_over_height) const {
        return glm::perspective(fov_, width_over_height, near_plane_, far_plane_);
    }

    void update(GLFWwindow* window);

    void reset(GLFWwindow* window);

    void zoom_to_box(const glm::vec3& min, const glm::vec3& max);

    float& sensitivity() {return sensitivity_;}

    float& pan_speed() {return pan_speed_;}

    float& near_plane() {return near_plane_;}

    float& far_plane() {return far_plane_;}

    float& fov() {return fov_;}

    Vec3f& orbit_target() {return orbit_target_;}
};

}
