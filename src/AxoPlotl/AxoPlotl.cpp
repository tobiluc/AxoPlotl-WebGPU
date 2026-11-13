#include <AxoPlotl/AxoPlotl.hpp>
#include <AxoPlotl/core/Application.hpp>
#include <mach/task_info.h>
#include <mach/mach.h>

namespace AxoPlotl
{

internal::Application& app() {
    static internal::Application app_instance;
    return app_instance;
}

bool init()
{
    return app().init();
}

void run()
{
    app().run();
}

internal::Scene& scene()
{
    return app().scene();
}

void set_inspector_callback(const std::function<void()>& _callback)
{
    app().inspector_callback() = _callback;
}

void show_inspector(bool _show)
{
    app().inspector_enabled() = _show;
}

float frames_per_second()
{
    return app().fps();
}

float memory_usage_mb()
{
    mach_task_basic_info info;
    mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;
    task_info(mach_task_self(), MACH_TASK_BASIC_INFO, (task_info_t)&info, &count);
    return info.resident_size / (1024.0 * 1024.0);
}

}
