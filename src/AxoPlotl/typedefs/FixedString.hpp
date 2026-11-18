#pragma once
#include <cstddef>
#include <string_view>

namespace AxoPlotl::internal
{

template<size_t N>
struct FixedString
{
    char value[N];

    constexpr FixedString(const char (&str)[N]) {
        for (size_t i = 0; i < N; ++i) {
            value[i] = str[i];
        }
    }

    constexpr operator std::string_view() const {
        return {value, N-1};
    }
};

template<std::size_t N>
FixedString(const char (&)[N]) -> FixedString<N>;

}
