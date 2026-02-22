#include "Camera.hpp"
#include "AxoPlotl/input/Mouse.hpp"
#include "imgui.h"
#include <iostream>
#include <ostream>

namespace AxoPlotl
{

void PerspectiveCamera::update(GLFWwindow* window)
{
    // Compute Pitch/Yaw based on Input

    if (!ImGui::GetIO().WantCaptureMouse)
    {
        // Zoom
        float dy = Input::Mouse::SCROLL_DELTA[1];
        if (dy) {
            dy *= sensitivity_;
            orbit_distance_ = glm::clamp(orbit_distance_ * (1.0f-dy), near, far);
        }

        // Pan
        if (Input::Mouse::LEFT_PRESSED)
        {
            float dx = Input::Mouse::POSITION_DELTA[0];
            float dy = Input::Mouse::POSITION_DELTA[1];

            dx *= sensitivity_;
            dy *= sensitivity_;

            if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
                glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) {
                // Shift Move orbit target

                glm::vec3 forward = glm::normalize(orbit_target_ - position_);
                glm::vec3 right   = glm::normalize(glm::cross(forward, world_up));
                up_ = glm::normalize(glm::cross(right, forward));

                float pan_speed = pan_speed_ * orbit_distance_ * tan(fov_ * 0.5f);

                orbit_target_ -= right * dx * pan_speed;
                orbit_target_ += up_ * dy * pan_speed;
            }
            else
            {
                // Rotate
                yaw_ -= dx;
                pitch_ = glm::clamp(pitch_ - dy, -1.5f, 1.5f); // between -89 and 89 degrees
            }
        }
    }

    // Update Vectors
    glm::vec3 offset;
    offset.x = orbit_distance_ * cos(pitch_) * sin(yaw_);
    offset.y = orbit_distance_ * sin(pitch_);
    offset.z = orbit_distance_ * cos(pitch_) * cos(yaw_);

    position_ = orbit_target_ + offset;

    glm::vec3 forward = glm::normalize(-offset);
    glm::vec3 right = glm::normalize(glm::cross(forward, world_up));
    up_ = glm::normalize(glm::cross(right, forward));
}

void PerspectiveCamera::reset(GLFWwindow *window) {
    orbit_target_ = glm::vec3(0.0f);
    orbit_distance_ = 10.0f;
    fov_ = 0.25*M_PI;
    pitch_ = yaw_ = 0;
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
    orbit_target_ = 0.5f*(min+max);
    orbit_distance_ = glm::distance(max, min);
}

// void OrthographicCamera::zoomToBox(const glm::vec3& min, const glm::vec3& max)
// {
//     position = 0.5f*(min+max);
//     position[2] = 1;
//     height = 1.5f*(max[1] - min[1]);
// }

}
