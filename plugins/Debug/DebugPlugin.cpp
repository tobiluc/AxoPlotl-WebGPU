#include "DebugPlugin.hpp"
#include "AxoPlotl/Application.hpp"
#include "AxoPlotl/PluginRegistry.hpp"
#include "AxoPlotl/utils/fps.hpp"
#include <mach/task_info.h>
#include <mach/mach.h>

static double inline memory_usage_mb() {
    mach_task_basic_info info;
    mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;
    task_info(mach_task_self(), MACH_TASK_BASIC_INFO, (task_info_t)&info, &count);
    return info.resident_size / (1024.0 * 1024.0);
}

namespace AxoPlotl
{

void DebugPlugin::render_ui(Application& app)
{
    ImGui::SeparatorText("Loaded Plugins");
    for (auto& obj : app.scene().get_objects()) {
        ImGui::SeparatorText(obj->name().c_str());
        ImGui::Text("Vertex: Mode = %d, Range = (%f,%f)",
                    obj->renderer_.vertex_property_.mode_,
                    obj->renderer_.vertex_property_.filter_.scalar_range_.x,
                    obj->renderer_.vertex_property_.filter_.scalar_range_.y);
        ImGui::Text("Edge: Mode = %d, Range = (%f,%f)",
                    obj->renderer_.edge_property_.mode_,
                    obj->renderer_.edge_property_.filter_.scalar_range_.x,
                    obj->renderer_.edge_property_.filter_.scalar_range_.y);
        ImGui::Text("Face: Mode = %d, Range = (%f,%f)",
                    obj->renderer_.face_property_.mode_,
                    obj->renderer_.face_property_.filter_.scalar_range_.x,
                    obj->renderer_.face_property_.filter_.scalar_range_.y);
        ImGui::Text("Cell: Mode = %d, Range = (%f,%f)",
                    obj->renderer_.cell_property_.mode_,
                    obj->renderer_.cell_property_.filter_.scalar_range_.x,
                    obj->renderer_.cell_property_.filter_.scalar_range_.y);
    }
    // for (const auto& plugin : PluginRegistry::get_plugins()) {
    //     ImGui::Text("%s", plugin.second->name());
    // }
    ImGui::SeparatorText("Performance");
    ImGui::Text("%f MB", memory_usage_mb());
    ImGui::Text("%f FPS", Time::FRAMES_PER_SECOND);
    ImGui::Text("Draw Frames %d", n_draw_frames());
    ImGui::SeparatorText("Scene");
    ImGui::Text("#Objects: %zu", app.scene().get_objects().size());
}

REGISTER_AXOPLOTL_PLUGIN(DebugPlugin)

}
