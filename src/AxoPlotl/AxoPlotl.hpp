#pragma once
#include <AxoPlotl/Scene.hpp>
#include <functional>

namespace AxoPlotl
{

bool init();

void run();

Scene& scene();

void set_inspector_callback(const std::function<void()>& _callback);

void show_inspector(bool _show);

float fps();

float memory_usage_mb();

}
