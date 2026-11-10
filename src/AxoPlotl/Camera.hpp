#pragma once

#include <AxoPlotl/typedefs/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <GLFW/glfw3.h>

namespace AxoPlotl::internal
{

class Camera
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

    bool use_ortho_ = false;

public:
    glm::mat4 get_view_matrix() const;

    glm::mat4 get_perspective_projection_matrix(const ViewSize& _viewport_size) const;

    glm::mat4 get_orthographic_projection_matrix(const ViewSize& _viewport_size) const;

    glm::mat4 get_projection_matrix(const ViewSize& _viewport_size) const;

    void update(GLFWwindow* window, bool _allow_input = true);

    void reset(GLFWwindow* window);

    void zoom_to_box(const glm::vec3& min, const glm::vec3& max);

    bool render_settings_ui(GLFWwindow* _window);
};

}
