#pragma once
#include <AxoPlotl/rendering/helpers/BufferData.hpp>
#include <AxoPlotl/typedefs/ovm.hpp>

namespace AxoPlotl::internal
{

class MeshBits
{
 private:
    std::shared_ptr<VolMesh> mesh_;
    BitBufferData vertex_bits_;
    BitBufferData edge_bits_;
    BitBufferData face_bits_;
    BitBufferData cell_bits_;
    wgpu::Device device_;

public:
    MeshBits(std::shared_ptr<VolMesh> _mesh) :
        mesh_(_mesh),
        vertex_bits_(_mesh->n_vertices()),
        edge_bits_(_mesh->n_edges()),
        face_bits_(_mesh->n_faces()),
        cell_bits_(_mesh->n_cells())
    {}

    void create_buffers(wgpu::Device _device);

    const BitBufferData& v_bits() const {return vertex_bits_;}
    const BitBufferData& e_bits() const {return edge_bits_;}
    const BitBufferData& f_bits() const {return face_bits_;}
    const BitBufferData& c_bits() const {return cell_bits_;}

    void enable_buffer_write();
    void disable_buffer_write();

    // Set individual bits
    void set_vertex_bit(OVM::VH _vh, bool _b);
    void set_edge_bit(OVM::EH _eh, bool _b);
    void set_face_bit(OVM::FH _fh, bool _b);
    void set_cell_bit(OVM::CH _ch, bool _b);

    // Apply Bits to incident entities e.g.
    // set_edge_cells(true, true) enables all cells incident to enabled edges while
    // set_cell_vertices(true, false) disables all vertices of enabled cells

    // void set_vertex_vertices(bool _v_bit, bool _v_bit2);
    // void set_vertex_edges(bool _v_bit, bool _e_bit);
    // void set_vertex_faces(bool _v_bit, bool _f_bit);
    // void set_vertex_cells(bool _v_bit, bool _c_bit);

    // void set_edge_vertices(bool _e_bit, bool _v_bit);
    // void set_edge_edges(bool _e_bit, bool _e_bit2);
    // void set_edge_faces(bool _e_bit, bool _f_bit);
    // void set_edge_cells(bool _e_bit, bool _c_bit);

    // void set_face_vertices(bool _f_bit, bool _v_bit);
    // void set_face_edges(bool _f_bit, bool _e_bit);
    // void set_face_faces(bool _f_bit, bool _f_bit2);
    // void set_face_cells(bool _f_bit, bool _c_bit);

    // void set_cell_vertices(bool _c_bit, bool _v_bit);
    // void set_cell_edges(bool _c_bit, bool _e_bit);
    // void set_cell_faces(bool _c_bit, bool _f_bit);
    // void set_cell_cells(bool _c_bit, bool _c_bit2);

};

}