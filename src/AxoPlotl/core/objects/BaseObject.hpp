#pragma once
#include <AxoPlotl/core/AxoPlotl_fwd.hpp>
#include <AxoPlotl/core/picking.hpp>
#include <AxoPlotl/geometry/BoundingBox.hpp>
#include <filesystem>
#include "AxoPlotl/typedefs/glm.hpp"
#include "webgpu/webgpu.hpp"

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

    virtual void init_buffers() = 0;

    virtual void delete_buffers() = 0;

    virtual void recompute_bounding_box() = 0;

    virtual void render(
        wgpu::RenderPassEncoder _render_pass,
        const ViewSize& _view_size,
        const Mat4x4f& _view_projection) = 0;

    virtual void render_ui_info() = 0;

    virtual void render_ui_settings() = 0;

    virtual void render_ui_properties() = 0;

    virtual void render_ui_picking(const Picker& _p) = 0;

    virtual bool export_file(const std::filesystem::path& _path) const;

    const BoundingBox& bounding_box() const {return bbox_;}

    bool& target() {return target_;}

    bool& deleted() {return deleted_;}

    std::string& name() {return name_;}

    const int id() const {return id_;}

    bool& visible() {return visible_;}

    const Scene* scene() const {return scene_;}

    bool input_name();

protected:
    internal::Scene* scene_;
    int id_;
    std::string name_;
    //std::unique_ptr<RendererBase> renderer_;
    Mat4x4f transform_;
    static int id_counter_;
    BoundingBox bbox_;
    bool deleted_ = false;
    bool target_ = false;
    bool visible_ = true;
};

};
