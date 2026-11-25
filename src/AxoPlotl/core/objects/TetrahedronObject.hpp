#pragma once

#include <AxoPlotl/core/AxoPlotl_fwd.hpp>
#include <AxoPlotl/core/objects/BaseObject.hpp>
#include <AxoPlotl/rendering/renderers/MeshCellRenderer.hpp>

namespace AxoPlotl
{

class TetObject : public ObjectBase
{
private:
    MeshCellRenderer renderer_;
    VolMeshData mesh_data_;

    std::array<std::array<float,4>,4> input_;

public:
    TetObject(
        internal::Scene* _scene) :
        ObjectBase(_scene, "")
    {
        this->name_ = "Tet " + std::to_string(id_);
        OVM::VH vh0 = mesh_data_.mesh.add_vertex(OVMVec3(0,0,0));
        OVM::VH vh1 = mesh_data_.mesh.add_vertex(OVMVec3(0,0,1));
        OVM::VH vh2 = mesh_data_.mesh.add_vertex(OVMVec3(1,0,0));
        OVM::VH vh3 = mesh_data_.mesh.add_vertex(OVMVec3(0,1,0));
        OVM::HFH hfh0 = mesh_data_.mesh.add_face({vh0,vh1,vh2}).halfface_handle(0);
        OVM::HFH hfh1 = mesh_data_.mesh.add_face({vh0,vh2,vh3}).halfface_handle(0);
        OVM::HFH hfh2 = mesh_data_.mesh.add_face({vh0,vh1,vh3}).halfface_handle(0);
        OVM::HFH hfh3 = mesh_data_.mesh.add_face({vh1,vh2,vh3}).halfface_handle(0);
        mesh_data_.mesh.add_cell({hfh0,hfh1,hfh2,hfh3});
        input_[0] = {0,0,0,1};
        input_[1] = {0,0,1,1};
        input_[2] = {1,0,0,1};
        input_[3] = {0,1,0,1};
    }

    ~TetObject() {
        destroy();
    }

    void render(
        wgpu::RenderPassEncoder _render_pass,
        const ViewSize& _view_size,
        const Mat4x4& _view,
        const Mat4x4& _proj) override;

    void draw_info_ui() override;

    void draw_settings_ui() override;

    void draw_properties_ui() override;

    void draw_picking_ui(const Picker& _p) override;

    void init() override;

    void destroy() override;

    void recompute_bounding_box() override;

    inline float* point(int i) {
        return input_[i].data();
    }
};

using TetrahedronObject = TetObject;

}
