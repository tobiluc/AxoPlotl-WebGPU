#pragma once
#include <optional>

namespace AxoPlotl
{

struct RunConfig
{
    std::optional<unsigned long long> num_ticks = std::nullopt;
    bool manual_cleanup = false;
};

}