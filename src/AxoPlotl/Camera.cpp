#include "Camera.hpp"
#include "AxoPlotl/input/Mouse.hpp"
#include "imgui.h"
#include <iostream>
#include <ostream>

namespace AxoPlotl
{

void PerspectiveCamera::update(GLFWwindow* window)
{
    float orbit_distance = glm::length(orbit_target_ - eye_position_);

    if (!ImGui::GetIO().WantCaptureMouse)
    {
        // Zoom
        float scroll = Input::Mouse::SCROLL_DELTA[1];
        if (scroll) {
            scroll *= sensitivity_;
            orbit_distance = glm::clamp(orbit_distance * (1.0f-scroll), near_plane_, far_plane_);
        }

        // Pan
        if (Input::Mouse::LEFT_PRESSED)
        {
            float dx = Input::Mouse::POSITION_DELTA[0] * sensitivity_;
            float dy = Input::Mouse::POSITION_DELTA[1] * sensitivity_;

            bool shift_pressed = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
                                glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;

            if (shift_pressed) {
                // Shift Move orbit target
                float pan_speed = pan_speed_ * orbit_distance * std::tan(fov_ * 0.5f);

                orbit_target_ -= (orientation_ * RIGHT) * dx * pan_speed;
                orbit_target_ += (orientation_ * UP) * dy * pan_speed;
            }
            else {
                // Orbit
                glm::quat rot_pitch = glm::angleAxis(dy, RIGHT);
                glm::quat rot_yaw = glm::angleAxis(-dx, UP);
                orientation_ = glm::normalize(orientation_ * rot_yaw * rot_pitch);
            }
        }
    }

    eye_position_ = orbit_target_ +
        orientation_ * (-FORWARD * orbit_distance);
}

void PerspectiveCamera::reset(GLFWwindow *window) {
    orbit_target_ = {0,0,0};
    eye_position_ = {0,0,1};
    orientation_ = {1,0,0,0};
    fov_ = 0.25*M_PI;
}

// void OrthographicCamera::update(GLFWwindow* window) {
//     if (GL::IMGUI_FOCUS) return;

//     // Move around
//     auto direction = glm::vec3(0,0,0);
//     if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) direction.x += 1;
//     if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) direction.x -= 1;
//     if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) direction.y += 1;
//     if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) direction.y -= 1;
//     if (!direction.x && !direction.y) return;
//     position += normalize(direction) * pan_speed_ * Time::DELTA_TIME;

//     // Zoom
//     float dy = MouseHandler::SCROLL_DELTA[1];
//     if (dy) {
//         dy *= sensitivity_;
//         height = std::clamp(height * (1.0f-dy), near, far);
//     }
// }

// void OrthographicCamera::reset(GLFWwindow *window) {
//     position = glm::vec3(0,0,1);
//     height = 10;
// }

void PerspectiveCamera::zoom_to_box(const glm::vec3& min, const glm::vec3& max)
{
    float required_distance = 0.0f;
    orbit_target_ = (min + max) * 0.5f;

    float aspect_ratio = 1; // todo - use actual ratio
    float half_fov_y = fov_ * 0.5f;
    float half_fov_x = std::atan(std::tan(half_fov_y) * aspect_ratio);

    for (int x = 0; x < 2; ++x) {
        for (int y = 0; y < 2; ++y) {
            for (int z = 0; z < 2; ++z)
            {
                glm::vec3 corner(
                    x? max.x : min.x,
                    y? max.y : min.y,
                    z? max.z : min.z
                );

                glm::vec3 local = glm::inverse(orientation_) *
                    (corner - orbit_target_);

                float required_x =
                    std::abs(local.x) / std::tan(half_fov_x) + local.z;

                float required_y =
                    std::abs(local.y) / std::tan(half_fov_y) + local.z;

                required_distance = std::max(
                    required_distance,
                    std::max(required_x, required_y)
                );
            }
        }
    }

    required_distance *= 1.2f; // padding

    eye_position_ = orbit_target_ +
        orientation_ * (-FORWARD * required_distance);
}

}
