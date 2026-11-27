#pragma once
#include <webgpu/webgpu.hpp>

namespace AxoPlotl::internal
{

template<typename... Res>
constexpr void save_destroy_release(Res&... _res) {
    ([&] {
        if (_res) {
            _res.destroy();
            _res.release();
            _res = nullptr;
        }
    }(), ...);
}

template<typename... Res>
constexpr void save_release(Res&... _res) {
    ([&] {
        if (_res) {
            _res.release();
            _res = nullptr;
        }
    }(), ...);
}

template<typename... Res>
constexpr void save_unconfigure_release(Res&... _res) {
    ([&] {
        if (_res) {
            _res.unconfigure();
            _res.release();
            _res = nullptr;
        }
    }(), ...);
}

}
