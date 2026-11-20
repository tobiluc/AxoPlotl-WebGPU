#pragma once
#include <utility>
#include <type_traits>

namespace AxoPlotl::internal
{

// Callback to be run when variable goes out of scope
template <typename Fn>
class ScopeExit {
public:
    explicit ScopeExit(Fn fn)
        : fn_(std::move(fn))
    {}

    ScopeExit(const ScopeExit&) = delete;
    ScopeExit& operator=(const ScopeExit&) = delete;

    ScopeExit(ScopeExit&& other) noexcept
        : fn_(std::move(other.fn_)),
        active_(std::exchange(other.active_, false))
    {}

    ~ScopeExit() noexcept {
        if (active_) {
            fn_();
        }
    }

    void dismiss() noexcept {
        active_ = false;
    }

private:
    Fn fn_;
    bool active_ = true;
};

template <typename Fn>
ScopeExit(Fn) -> ScopeExit<Fn>;

}
