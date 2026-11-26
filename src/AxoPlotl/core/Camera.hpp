#pragma once

#include <AxoPlotl/typedefs/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <AxoPlotl/IO/json_glm.hpp>
#include <AxoPlotl/utils/ViewSize.hpp>

namespace AxoPlotl::internal
{

class Camera
{
public:
    static constexpr Vec3 UP = {0,1,0};
    static constexpr Vec3 RIGHT = {1,0,0};
    static constexpr Vec3 FORWARD = {0,0,-1};

private:
    float sensitivity_ = 1.0f;
    float pan_speed_ = 160.0f;

    float near_plane_ = 1.0f/64.0f;
    float far_plane_ = (1<<11);

    Vec3 orbit_target_ = {0, 0, 0};
    Vec3 eye_position_ = {0, 0, 1};
    Quat orientation_ = {1, 0, 0, 0};

    float fov_ = 0.25*M_PI; // field of view in radians

    bool use_ortho_ = false;

public:
    Mat4x4 get_view_matrix() const;

    Mat4x4 get_perspective_projection_matrix(const ViewSize& _viewport_size) const;

    Mat4x4 get_orthographic_projection_matrix(const ViewSize& _viewport_size) const;

    Mat4x4 get_projection_matrix(const ViewSize& _viewport_size) const;

    void update(float _dt);

    void set_default() {
        *this = Camera();
    }

    void zoom_to_box(const Vec3& min, const Vec3& max, const float _padding = 0.1);

    bool draw_settings_ui();

    float orbit_distance() const {
        return glm::length(orbit_target_ - eye_position_);
    }

    float& near_plane() {return near_plane_;}

    float& far_plane() {return far_plane_;}

    const Quat& orientation() const {return orientation_;}

    void set_orientation(const Quat& _q) {
        orientation_ = glm::normalize(_q);
    }

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Camera,
        sensitivity_, pan_speed_, near_plane_, far_plane_,
        fov_,
        use_ortho_,
        orbit_target_, eye_position_,
        orientation_
    );
};

}
