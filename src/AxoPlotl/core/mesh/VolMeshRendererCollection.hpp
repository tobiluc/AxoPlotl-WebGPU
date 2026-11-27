#pragma once
#include <AxoPlotl/rendering/renderers/renderers.hpp>

namespace AxoPlotl::internal
{

class VolMeshRendererCollection
{
private:
    internal::MeshVertexRenderer vertex_renderer_;
    internal::MeshHalfEdgeRenderer edge_renderer_;
    internal::MeshHalfFaceRenderer face_renderer_;
    internal::MeshCellRenderer cell_renderer_;
    Vector3Renderer vec3_renderer_;
    Matrix3x3Renderer mat3x3_renderer_;

public:
    template<typename EntityTag>
    auto& entity_renderer();
    template<> auto& entity_renderer<OVM::Entity::Cell>() {return cell_renderer_;}
    template<> auto& entity_renderer<OVM::Entity::Face>() {return face_renderer_;}
    template<> auto& entity_renderer<OVM::Entity::HalfFace>() {return face_renderer_;}
    template<> auto& entity_renderer<OVM::Entity::Edge>() {return edge_renderer_;}
    template<> auto& entity_renderer<OVM::Entity::HalfEdge>() {return edge_renderer_;}
    template<> auto& entity_renderer<OVM::Entity::Vertex>() {return vertex_renderer_;}
    Vector3Renderer& vector3_renderer() {return vec3_renderer_;}
    Matrix3x3Renderer& matrix3x3_renderer() {return mat3x3_renderer_;}

    void init(uint32_t _object_id, const VolMeshBuffers& _data, const GpuContext& _gpu_context, const RenderContext& _render_context)
    {
        const wgpu::Device& device = _gpu_context.device;
        assert(_data.is_initialized());

        vertex_renderer_.init(_object_id, _gpu_context, _render_context,
            _data.vertices_positions);
        edge_renderer_.init(_object_id, _gpu_context, _render_context,
            _data.vertices_positions,
            _data.halfedges_instances);
        face_renderer_.init(_object_id, _gpu_context, _render_context,
            _data.vertices_positions,
            _data.halffaces_indices,
            _data.halffaces_normals);
        cell_renderer_.init(_object_id, _gpu_context, _render_context,
            _data.vertices_positions,
            _data.cells_indices,
            _data.cells_centers,
            _data.halffaces_normals);

        vertex_renderer_.enabled() = false;
        edge_renderer_.enabled() = false;
        face_renderer_.enabled() = false;
        cell_renderer_.enabled() = false;
        if (_data.mesh.n_cells() > 0) {
            cell_renderer_.enabled() = true;
        } else if (_data.mesh.n_faces() > 0) {
            face_renderer_.enabled() = true;
            edge_renderer_.enabled() = true;
        } else if (_data.mesh.n_edges() > 0) {
            edge_renderer_.enabled() = true;
            vertex_renderer_.enabled() = true;
        } else if (_data.mesh.n_vertices() > 0) {
            vertex_renderer_.enabled() = true;
        }
    }

    void cleanup() {
        vertex_renderer_.cleanup();
        edge_renderer_.cleanup();
        face_renderer_.cleanup();
        cell_renderer_.cleanup();
        vec3_renderer_.cleanup();
        mat3x3_renderer_.cleanup();
    }

    void set_directional_light(const DirectionalLight& _light) {
        cell_renderer_.directional_light() = _light;
        face_renderer_.directional_light() = _light;
        edge_renderer_.directional_light() = _light;
        vertex_renderer_.directional_light() = _light;
    }

    void set_clip_box(const ClipBox& _cb) {
        edge_renderer_.clip_box() = _cb;
        face_renderer_.clip_box() = _cb;
        cell_renderer_.clip_box() = _cb;
        vec3_renderer_.clip_box() = _cb;
        mat3x3_renderer_.clip_box() = _cb;
    }

    void configure_like(const VolMeshRendererCollection& _r) {
        vertex_renderer_.configure_like(_r.vertex_renderer_);
        edge_renderer_.configure_like(_r.edge_renderer_);
        face_renderer_.configure_like(_r.face_renderer_);
        cell_renderer_.configure_like(_r.cell_renderer_);
        vec3_renderer_.configure_like(_r.vec3_renderer_);
        mat3x3_renderer_.configure_like(_r.mat3x3_renderer_);
    }

    void render(const wgpu::RenderPassEncoder& _render_pass, const MVP& _mvp, const ViewSize& _size, const Picker::Entity& _selected_entity)
    {
        const auto& mvp_mat = _mvp.modelviewproj();

        cell_renderer_.render(_size, _render_pass, mvp_mat, _selected_entity);
        face_renderer_.render(_size, _render_pass, mvp_mat, _selected_entity);
        edge_renderer_.render(_size, _render_pass, _mvp, _selected_entity);
        vertex_renderer_.render(_size, _render_pass, _mvp, _selected_entity);

        vec3_renderer_.enabled() = vertex_renderer_.property_filter_options().value_mode == PropValueMode::VEC3
            || edge_renderer_.property_filter_options().value_mode == PropValueMode::VEC3
            || face_renderer_.property_filter_options().value_mode == PropValueMode::VEC3
            || cell_renderer_.property_filter_options().value_mode == PropValueMode::VEC3;

        vec3_renderer_.render(_size, _render_pass, mvp_mat);

        mat3x3_renderer_.render(_size, _render_pass, mvp_mat);
    }
};

}