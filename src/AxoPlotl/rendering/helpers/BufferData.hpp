#pragma once
#include <webgpu/webgpu.hpp>
#include "save_cleanup.hpp"

namespace AxoPlotl::internal
{

template<typename T>
wgpu::Buffer create_and_write_bufferT(
    const wgpu::Device& _device,
    const std::vector<T>& _data,
    const WGPUBufferUsage _usage =
    wgpu::BufferUsage::Storage |
    wgpu::BufferUsage::CopyDst |
    wgpu::BufferUsage::Vertex,
    const std::string& _name = "Buffer T")
{
    wgpu::BufferDescriptor desc{};
    desc.usage =_usage;
    desc.size = sizeof(T)*std::max(_data.size(), 1lu);
    desc.mappedAtCreation = false;
    desc.label = wgpu::StringView(_name);
    wgpu::Buffer buffer = _device.createBuffer(desc);
    if (!_data.empty()) [[likely]] {
        _device.getQueue().writeBuffer(buffer, 0, _data.data(), buffer.getSize());
    }
    return buffer;
}

template<typename T>
struct BufferData {
    wgpu::Buffer buffer;
    std::vector<T> data;
    ~BufferData() {cleanup();}
    void create_and_write(const wgpu::Device& _device) {
        save_destroy_release(buffer);
        buffer = create_and_write_bufferT<T>(_device, data);
    }
    void cleanup() {
        save_destroy_release(buffer);
        data.clear();
    }
};

struct BitBufferData
{
public:
    using Word = uint32_t;
    using Word4 = std::array<Word,4>;
    static constexpr size_t bits_per_word = 32;
    static constexpr size_t words_per_element = 4;
    static constexpr size_t bits_per_element = words_per_element * bits_per_word;
    static constexpr Word word0 = 0u;
    static constexpr Word word1 = 0xFFFFFFFFu;

private:
    wgpu::Buffer buffer_;
    std::vector<Word4> data_;
    size_t num_bits_ = 0;
    bool enable_buffer_write_ = true;
    size_t popcount_ = 0;

    void update_popcount()
    {
         popcount_ = 0;

        for (size_t i = 0; i < data_.size(); ++i) {
            for (size_t j = 0; j < words_per_element; ++j) {
                const size_t word_idx = i * words_per_element + j;
                const size_t first_bit = word_idx * bits_per_word;

                if (first_bit >= num_bits_) {break;}

                const size_t valid_bits = std::min(
                    bits_per_word,
                    num_bits_ - first_bit
                );

                Word word = data_[i][j];
                if (valid_bits < bits_per_word) {
                    word &= (1u << valid_bits) - 1u;
                }
                popcount_ += std::popcount(word);
            }
        }
    }

public:
    BitBufferData(size_t _num_bits = 0) {
        resize(_num_bits);
    }
    ~BitBufferData() {cleanup();}
    void resize(size_t _num_bits) {
        if (_num_bits == num_bits_) {return;}
        cleanup();
        num_bits_ = _num_bits;
        data_.resize(
            _num_bits / bits_per_element +
            (_num_bits % bits_per_element != 0)
        );
        for (size_t i = 0; i < data_.size(); ++i) {
            for (size_t j = 0; j < words_per_element; ++j) {
                data_[i][j] = word1;
            }
        }
        popcount_ = num_bits_;
    }
    void cleanup() {
        save_destroy_release(buffer_);
        data_.clear();
        num_bits_ = 0;
        popcount_ = 0;
    }
    void enable_buffer_write() {enable_buffer_write_ = true;}
    void disable_buffer_write() {enable_buffer_write_ = false;}
    size_t num_bits() const {return num_bits_;}
    size_t popcount() const {return popcount_;}
    const wgpu::Buffer& buffer() const {return buffer_;}
    const std::vector<Word4>& data() const {return data_;}
    void create_buffer(
        const wgpu::Device& _device,
        const WGPUBufferUsage _usage =
        wgpu::BufferUsage::Storage |
        wgpu::BufferUsage::CopyDst |
        wgpu::BufferUsage::Vertex,
        const std::string& _name = "BitBuffer")
    {
        save_destroy_release(buffer_);
        wgpu::BufferDescriptor desc{};
        desc.usage =_usage;
        desc.size = sizeof(Word)*words_per_element*std::max(data_.size(), 1lu);
        desc.mappedAtCreation = false;
        desc.label = wgpu::StringView(_name);
        buffer_ = _device.createBuffer(desc);
        write_data(_device);
    }
    bool read_bit(size_t _idx) const
    {
        if (_idx >= num_bits_) {return false;}

        const size_t word_idx = _idx / bits_per_word;
        const size_t lane_idx = word_idx % words_per_element;
        const size_t element_idx = word_idx / words_per_element;
        const size_t bit_idx = _idx % bits_per_word;

        const uint32_t& word = data_[element_idx][lane_idx];

        return (word & (1u << bit_idx)) != 0u;
    }
    void write_bits(const wgpu::Device& _device, size_t _start_idx, size_t _num_bits, bool _bit = true)
    {
        if (!_device || _num_bits == 0 || _start_idx >= num_bits_) {
            return;
        }

        // Clamp to available capacity.
        _num_bits = std::min(_num_bits, num_bits_ - _start_idx);

        const auto& queue = _device.getQueue();

        size_t current_bit = _start_idx;
        size_t remaining = _num_bits;

        while (remaining > 0)
        {
            const size_t word_idx = current_bit / bits_per_word;
            const size_t element_idx = word_idx / words_per_element;
            const size_t lane_idx = word_idx % words_per_element;
            const size_t bit_idx = current_bit % bits_per_word;

            // Number of bits we can process in this word.
            const size_t bits_this_word = std::min(
                remaining,
                bits_per_word - bit_idx
            );

            uint32_t& word = data_[element_idx][lane_idx];
            const Word old_word = word;

            const uint32_t mask = (bits_this_word == bits_per_word)? word1
                    : ((1u << bits_this_word) - 1u) << bit_idx;

            if (_bit) {word |= mask;}
            else {word &= ~mask;}

            const int delta =
                static_cast<int>(std::popcount(word)) -
                static_cast<int>(std::popcount(old_word));
            if (delta >= 0) {popcount_ += static_cast<size_t>(delta);}
            else {popcount_ -= static_cast<size_t>(-delta);}

            if (enable_buffer_write_) {
                const uint64_t offset =
                    element_idx * sizeof(data_[0]) +
                    lane_idx * sizeof(uint32_t);
                queue.writeBuffer(
                    buffer_,
                    offset,
                    &word,
                    sizeof(uint32_t)
                );
            }

            current_bit += bits_this_word;
            remaining -= bits_this_word;
        }
    }
    void write_bit(const wgpu::Device& _device, size_t _idx, bool _bit = true) {
        if (_bit == read_bit(_idx)) {return;}
        write_bits(_device, _idx, 1, _bit);
    }
    void write_all_bits(const wgpu::Device& _device, bool _bit = true) {
        const Word word = _bit ? word1 : word0;
        std::fill(
            data_.begin(),
            data_.end(),
            std::array<Word, 4>{word, word, word, word}
        );
        if (data_.empty()) {return;}

        // Mask off unused bits in the final element.
        if (_bit && (num_bits_ % bits_per_element != 0))
        {
            const size_t valid_bits = num_bits_ % bits_per_element;
            const size_t valid_words = (valid_bits + bits_per_word - 1) / bits_per_word;
            const size_t remaining_bits = valid_bits % bits_per_word;

            if (remaining_bits != 0) {
                data_.back()[valid_words - 1] = (1u << remaining_bits) - 1u;
            }

            for (size_t i = valid_words; i < words_per_element; ++i) {
                data_.back()[i] = 0;
            }
        }
        popcount_ = _bit? num_bits_ : 0u;
        write_data(_device);
    }
    void invert_all_bits(const wgpu::Device& _device) {
        for (Word4& word4 : data_) {
            for (Word& word : word4) {
                word = ~word;
            }
        }
        popcount_ = num_bits_ - popcount_;
        write_data(_device);
    }
    void write_data(const wgpu::Device& _device) const {
        if (data_.empty() || !enable_buffer_write_) {return;}
        _device.getQueue().writeBuffer(buffer_, 0, data_.data(), data_.size()*sizeof(data_[0]));
    }
};

}
