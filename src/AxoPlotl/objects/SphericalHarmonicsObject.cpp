#include "SphericalHarmonicsObject.hpp"
#include "AxoPlotl/Application.hpp"
#include "AxoPlotl/geometry/spherical_harmonics.hpp"
#include "ibex/ibex.hpp"
#include <AxoPlotl/Scene.hpp>

namespace AxoPlotl
{

void SHObject::render(
    wgpu::RenderPassEncoder _render_pass,
    const Mat4x4f& _view_projection)

{
    if (deleted() || !visible()) {return;}

    const auto& mvp = _view_projection * transform_;
    const auto& vp = scene_->app()->scene_viewport();

    renderer_.render(vp, _render_pass, mvp);
}

void SHObject::render_ui_info()
{
}

void SHObject::render_ui_settings()
{
    std::string str = "Type (";
    switch (type_) {
    case Type::OCTA: str += "Octahedral)"; break;
    case Type::ODECO: str += "Odeco)"; break;
    case Type::CUSTOM: str += "Custom)"; break;
    default: break;
    }

    if (ImGui::BeginMenu(str.c_str()))
    {
        if (ImGui::MenuItem("Octa")) {type_ = Type::OCTA;}
        if (ImGui::MenuItem("Odeco")) {type_ = Type::ODECO;}
        if (ImGui::MenuItem("Custom")) {type_ = Type::CUSTOM;}
        ImGui::EndMenu();
    }
    renderer_.color_map().render_menu();
}

void SHObject::render_ui_properties()
{
    if (type_ == Type::CUSTOM)
    {
        ImGui::InputText("f(x,y,z,phi,theta)", custom_input_.data(), 1024);

        if (ImGui::Button("Submit")) {
            ibex::Functions funcs = ibex::common_functions();
            ibex::Variables vars = ibex::common_variables();
            auto rpn = ibex::generate_postfix(ibex::tokenize(custom_input_.c_str()));
            if (!rpn.empty())
            {
                auto func = [&](
                    const double& x,
                    const double& y,
                    const double& z) -> double
                {
                    vars["x"] = x;
                    vars["y"] = y;
                    vars["z"] = z;
                    // vars["phi"] = z;
                    // vars["theta"] = z;
                    return ibex::eval_postfix(rpn, vars, funcs);
                };
                renderer_.update([&](const Vec3f& _pos) -> float
                {
                    return func(_pos.x, _pos.y, _pos.z);
                });
            }
        }
    }
    else if (type_ == Type::ODECO)
    {
        bool updated_(false);

        updated_ |= ImGui::SliderFloat3("stretch", &lengths[0], 0, 10);
        updated_ |= ImGui::SliderFloat3("angles", &angles[0], 0, 0.5*M_PI);

        if (updated_)
        {
            renderer_.update([&](const Vec3f& _pos) -> float
            {
                return (1.0 + (
                             canonical_odeco_frame(lengths[0],lengths[1],lengths[2])
                                 .dot(sh_band024(_pos.x,_pos.y,_pos.z))
                             ))*0.5;
                 //return canonical_frame(lx,ly,lz).dot(sh_band4(_pos.x,_pos.y,_pos.z));
            });
        }
    }
    else if (type_ == Type::OCTA)
    {
        bool updated_(false);

        updated_ |= ImGui::SliderFloat3("angles", &angles[0], 0, 0.5*M_PI);

        if (updated_)
        {
            renderer_.update([&](const Vec3f& _pos) -> float
            {
                return (1.0 + (
                 canonical_octa_frame()
                     .dot(sh_band4(_pos.x,_pos.y,_pos.z))
                 ))*0.5;
            });
        }
    }
}

void SHObject::render_ui_picking(PickResult _p, const PickConfig& _cfg)
{
    if (_p.object_id_ != id()) [[unlikely]] {return;}

    auto polar_angle = [](const Vec3f& _pos) -> float {
        return std::acos(_pos.z/(glm::length(_pos)));
    };
    auto azimuth_angle = [](const Vec3f& _pos) -> float {
        float a = std::acos(_pos.x/(std::sqrt(_pos.x*_pos.x+_pos.y*_pos.y)));
        return (_pos.y<0)? -a : a;
    };

    // The input position is the direction from the origin
    // (ignore deformed geometry)
    _p.position_ = glm::normalize(_p.position_);

    ImGui::SeparatorText(name_.c_str());
    ImGui::Text("(x, y, z) = (%f, %f, %f)",
        _p.position_.x, _p.position_.y, _p.position_.z);
    ImGui::Text("(phi, theta) = (%f, %f)·π",
                polar_angle(_p.position_)/M_PI,
                azimuth_angle(_p.position_)/M_PI);
    ImGui::Text("Value = %f", _p.function_value_);
}

void SHObject::init_buffers()
{
    renderer_.init(id_, scene_->app(), render_resolution_);

    renderer_.update([&](const Vec3f& _pos) -> float
    {
        return canonical_odeco_frame(1,1,1).dot(sh_band024(_pos.x,_pos.y,_pos.z));
    });
}

void SHObject::delete_buffers()
{
    renderer_.clear();
}

void SHObject::recompute_bounding_box()
{
    bbox_ = BoundingBox();
    bbox_.expand_with_point(Vec3f(-1,-1,-1));
    bbox_.expand_with_point(Vec3f(1,1,1));
}

}
