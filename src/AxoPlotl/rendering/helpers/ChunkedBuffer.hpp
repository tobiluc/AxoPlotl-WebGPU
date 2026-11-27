#pragma once

namespace AxoPlotl::internal
{

// class ChunkedBuffer
// {
// private:
//     std::vector<wgpu::Buffer> buffers_;
//     size_t total_size_;
//     size_t chunk_size_;
// public:
//     struct Options {
//         uint64_t total_size;
//         uint64_t chunk_size;
//         wgpu::BufferUsage usage;
//         const char* label;
//     };
//     ChunkedBuffer(const wgpu::Device& _device,
//                   const Options& _opt)
//     {
//         if (_opt.chunk_size == 0) {
//             throw std::invalid_argument("chunk size cannot be zero");
//         }
//         total_size_ = _opt.total_size;
//         chunk_size_ = _opt.chunk_size;
//         const uint64_t count = (_opt.total_size + _opt.chunk_size - 1) / _opt.chunk_size;

//         buffers_.reserve(count);

//         for (uint64_t i = 0; i < count; ++i) {
//             uint64_t size = std::min(
//                 _opt.chunk_size,
//                 _opt.total_size-i*_opt.chunk_size);
//             wgpu::BufferDescriptor desc{};
//             desc.size = size;
//             desc.usage = _opt.usage;
//             desc.label = wgpu::StringView(_opt.label);

//             buffers_.push_back(_device.createBuffer(desc));
//         }
//     }
//     ~ChunkedBuffer() {
//         destroy();
//     }
//     void destroy() {
//         for (const auto& buffer : buffers_) {
//             if (buffer) {
//                 buffer.destroy();
//                 buffer.release();
//             }
//         }
//     }
//     size_t size() const {
//         return total_size_;
//     }

//     size_t chunk_size() const {
//         return chunk_size_;
//     }

//     size_t chunk_count() const {
//         return buffers_.size();
//     }

//     const wgpu::Buffer& chunk(size_t _idx) const {
//         return buffers_.at(_idx);
//     }

//     void write(
//         const wgpu::Queue& _queue,
//         uint64_t _offset,
//         const void* _data,
//         uint64_t _size)
//     {
//         if (_offset + _size > total_size_) {
//             throw std::out_of_range("SplitBuffer write out of bounds");
//         }

//         const auto* src = static_cast<const uint8_t*>(_data);

//         while (_size > 0)
//         {
//             uint64_t chunkIndex = _offset / chunk_size_;
//             uint64_t localOffset = _offset % chunk_size_;

//             uint64_t available_size =
//                 chunk(chunkIndex).getSize() - localOffset;

//             uint64_t copySize = std::min(_size, available_size);

//             _queue.writeBuffer(
//                 buffers_[chunkIndex],
//                 localOffset,
//                 src,
//                 copySize);

//             _offset += copySize;
//             src += copySize;
//             _size -= copySize;
//         }
//     }
// };

}
