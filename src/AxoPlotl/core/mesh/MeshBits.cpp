#include "MeshBits.hpp"

namespace AxoPlotl::internal
{

void MeshBits::create_buffers(wgpu::Device _device)
{
    this->device_ = _device;
    vertex_bits_.create_buffer(_device);
    edge_bits_.create_buffer(_device);
    face_bits_.create_buffer(_device);
    cell_bits_.create_buffer(_device);
}

void MeshBits::enable_buffer_write() {
    vertex_bits_.enable_buffer_write();
    edge_bits_.enable_buffer_write();
    face_bits_.enable_buffer_write();
    cell_bits_.enable_buffer_write();
}

void MeshBits::disable_buffer_write() {
    vertex_bits_.disable_buffer_write();
    edge_bits_.disable_buffer_write();
    face_bits_.disable_buffer_write();
    cell_bits_.disable_buffer_write();
}

void MeshBits::set_vertex_bit(OVM::VH _vh, bool _b) {
    assert(device_);
    vertex_bits_.write_bit(device_, _vh.idx(), _b);
}

void MeshBits::set_edge_bit(OVM::EH _eh, bool _b) {
    assert(device_);
    edge_bits_.write_bit(device_, _eh.idx(), _b);
}

void MeshBits::set_face_bit(OVM::FH _fh, bool _b) {
    assert(device_);
    face_bits_.write_bit(device_, _fh.idx(), _b);
}

void MeshBits::set_cell_bit(OVM::CH _ch, bool _b) {
    assert(device_);
    cell_bits_.write_bit(device_, _ch.idx(), _b);
}

}