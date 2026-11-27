#pragma once
#include <AxoPlotl/core/AxoPlotl_fwd.hpp>
#include <AxoPlotl/core/picking.hpp>
#include <AxoPlotl/geometry/BoundingBox.hpp>
#include <filesystem>
#include "AxoPlotl/typedefs/glm.hpp"
#include "webgpu/webgpu.hpp"
#include <nlohmann/json.hpp>
#include <AxoPlotl/utils/ViewSize.hpp>
#include <AxoPlotl/input/Mouse.hpp>

namespace AxoPlotl
{

using namespace internal;

class ObjectBase
{
friend class DebugPlugin;

public:
    ObjectBase(internal::Scene* _scene, std::string _name) :
        id_(++id_counter_),
        scene_(_scene),
        name_(_name),
        transform_(1)
    {}

    virtual ~ObjectBase() = default;

    virtual void init() = 0;

    virtual void cleanup() = 0;

    virtual void recompute_bounding_box() = 0;

    void update(float _dt);

    void fixed_update(float _dt);
    
    virtual bool on_mouse_just_pressed(const Picker::Pixel& _p, const Mouse::Button _button);

    virtual bool on_mouse_pressed(const Picker::Pixel& _p, const Mouse::Button _button);

    virtual void render(
        wgpu::RenderPassEncoder _render_pass,
        const ViewSize& _view_size,
        const Mat4x4& _view,
        const Mat4x4& _proj) = 0;

    virtual void draw_viewport_ui();

    virtual void draw_info_ui();

    virtual void draw_settings_ui() = 0;

    virtual void draw_selected_ui() = 0;

    const BoundingBox& bounding_box() const {return bbox_;}

    bool& target() {return target_;}

    bool& deleted() {return deleted_;}

    std::string& name() {return name_;}

    const int id() const {return id_;}

    bool& visible() {return visible_;}

    const Scene* scene() const {return scene_;}

    bool draw_name_input_ui();

    // Internal Objects can only be accessed by the scene itself and
    // are not displayed in the inspector.
    bool is_internal() const {
        return internal_;
    }

protected:
    internal::Scene* scene_;
    int id_;
    std::string name_;
    //std::unique_ptr<RendererBase> renderer_;
    Mat4x4 transform_;
    static int id_counter_;
    BoundingBox bbox_;
    bool deleted_ = false;
    bool target_ = false;
    bool visible_ = true;
    bool internal_ = false;
};

};
