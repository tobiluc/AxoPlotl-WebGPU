#pragma once

#include "AxoPlotl/gui/properties/property_settings.hpp"
#include <AxoPlotl/rendering/renderers/renderers.hpp>
#include "AxoPlotl/core/objects/ObjectBase.hpp"
#include <AxoPlotl/core/mesh/VolMeshBuffers.hpp>
#include "AxoPlotl/typedefs/ovm.hpp"
#include <filesystem>
#include <AxoPlotl/core/mesh/VolMeshPropertyVisualizer.hpp>

namespace AxoPlotl
{

class OpenVolumeMeshObject : public ObjectBase
{
protected:
    enum class PickMode {
        PROP_VIS,
        MINECRAFT_ADD,
        MINECRAFT_SUB
    } pick_mode_ = PickMode::PROP_VIS;
    struct MinecraftState
    {
        int k_ring = 1;
    } minecraft_state_;
    bool request_pick_popup_ = false;
    Picker::Pixel picked_pixel_;

    VolMeshBuffers mesh_data_;
    VolMeshPropertyVisualizer prop_vis_;
    VolMeshRendererCollection renderers_;
    std::optional<std::filesystem::path> filepath_ = std::nullopt;

public:

    OpenVolumeMeshObject(internal::Scene* _scene, VolMesh& _mesh,
                         const std::optional<std::filesystem::path>& _filepath = std::nullopt);

    ~OpenVolumeMeshObject() {
        cleanup();
    }

    bool on_mouse_just_pressed(const Picker::Pixel& _p, const Mouse::Button _button) override;

    bool on_mouse_pressed(const Picker::Pixel& _p, const Mouse::Button _button) override;

    void render(
        wgpu::RenderPassEncoder _render_pass,
        const ViewSize& _view_size,
        const Mat4x4& _view,
        const Mat4x4& _proj) override;

    void draw_viewport_ui() override;

    void draw_info_ui() override;

    void draw_settings_ui() override;

    void draw_selected_ui() override;

    void init() override;

    void cleanup() override;

    void recompute_bounding_box() override;

    bool visualize_property(
        const std::string& _property_name,
        std::optional<OVM::EntityType> _entity_type = std::nullopt,
        std::optional<std::string> _type_name = std::nullopt);

    void visualize_like(const OpenVolumeMeshObject& _obj);

    VolMesh& mesh() {return mesh_data_.mesh;}
    const VolMesh& mesh() const {return mesh_data_.mesh;}

    const std::optional<std::filesystem::path>& filepath() const {
        return filepath_;
    }
};

}
