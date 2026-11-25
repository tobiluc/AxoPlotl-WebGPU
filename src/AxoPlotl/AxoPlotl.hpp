#pragma once
#include <AxoPlotl/core/Scene.hpp>
#include <functional>
#include <AxoPlotl/core/RunConfig.hpp>

namespace AxoPlotl
{

bool init();

void run(RunConfig _cfg = RunConfig());

void cleanup();

internal::Application& app();

internal::Scene& scene();

void set_inspector_callback(const std::function<void()>& _callback);

void show_inspector(bool _show);

float frames_per_second();

float memory_usage_mb();

}
