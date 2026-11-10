#include "Camera.hpp"
#include "AxoPlotl/gui/input_slider.hpp"
#include "AxoPlotl/input/Mouse.hpp"
#include "imgui.h"
#include <iostream>
#include <ostream>

namespace AxoPlotl
{

glm::mat4 Camera::get_view_matrix() const {
    return glm::lookAtRH(eye_position_, orbit_target_, orientation_ * UP);
}

glm::mat4 Camera::get_perspective_projection_matrix(const ViewSize &_viewport_size) const {
    float width_over_height = _viewport_size.aspect_ratio();
    return glm::perspectiveRH_ZO(fov_, width_over_height, near_plane_, far_plane_);
}

glm::mat4 Camera::get_orthographic_projection_matrix(const ViewSize& _viewport_size) const
{
    const float width_over_height = _viewport_size.aspect_ratio();
    const float distance = glm::length(eye_position_ - orbit_target_);
    const float half_height = distance * std::tan(fov_ * 0.5f);
    const float half_width = half_height * width_over_height;

    // We need to use orthoRH_ZO (zero to one)
    // as that is the webgpu clipping space (compared to OpenGL -1 to 1)
    return glm::orthoRH_ZO(
        -half_width,
        half_width,
        -half_height,
        half_height,
        near_plane_,
        far_plane_
    );
}

glm::mat4 Camera::get_projection_matrix(const ViewSize& _viewport_size) const {
    return use_ortho_? get_orthographic_projection_matrix(_viewport_size)
                      : get_perspective_projection_matrix(_viewport_size);
}

void Camera::update(GLFWwindow* window, bool _allow_input)
{
    float orbit_distance = glm::length(orbit_target_ - eye_position_);

    if (_allow_input)
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

void Camera::reset(GLFWwindow *window) {
    orbit_target_ = {0,0,0};
    eye_position_ = {0,0,1};
    orientation_ = {1,0,0,0};
    fov_ = 0.25*M_PI;
    use_ortho_ = false;
}

void Camera::zoom_to_box(const glm::vec3& min, const glm::vec3& max)
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

bool Camera::render_settings_ui(GLFWwindow* _window)
{
    bool changed = false;

    changed |= ImGui::Checkbox("Orthographic", &use_ortho_);

    changed |= InputSlider(
        "Sensitivity", &sensitivity_,
        InputSliderOptions{.step=0.001f, .min=0.001f, .max=0.02f});

    changed |= InputSlider(
        "Pan Speed", &pan_speed_,
        InputSliderOptions{.step=0.025f, .min=0.5f, .max=4.0f});

    changed |= InputSlider3(
        "Orbit Center", &orbit_target_[0]);

    changed |= InputSlider(
        "FOV", &fov_,
        InputSliderOptions{.step=M_PI/100.0f, .min=0.1*M_PI, .max=0.9*M_PI});

    if (ImGui::InputFloat2("Near/Far Plane", &near_plane_)) {
        near_plane_ = std::clamp<float>(near_plane_, 1.f/256.0f, 1.f/16.0f);
        far_plane_ = std::clamp<float>(far_plane_, (1<<6), (1<<12));
        changed = true;
    }

    if (ImGui::Button("Reset")) {
        reset(_window);
        changed = true;
    }

    return changed;
}

}
