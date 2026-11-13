#include "DebugTool.hpp"
#include "AxoPlotl/AxoPlotl.hpp"
#include "AxoPlotl/debug/meshes.hpp"
#include <AxoPlotl/core/Scene.hpp>
#include <mach/task_info.h>
#include <mach/mach.h>
#include <AxoPlotl/rendering/debug/PickingTextureVis.hpp>
#include <AxoPlotl/core/Application.hpp>

namespace AxoPlotl
{

void DebugTool::render_ui()
{
    ImGui::SeparatorText("Performance");
    ImGui::Text("%f MB", AxoPlotl::memory_usage_mb());
    ImGui::Text("%f FPS", AxoPlotl::frames_per_second());
    ImGui::Text("Draw Frames %d", internal::n_draw_frames());

    ImGui::SeparatorText("Scene");
    ImGui::Text("#Objects: %zu", AxoPlotl::scene().num_objects());
    if (ImGui::BeginMenu("Add")) {
        if (ImGui::MenuItem("Add Mesh with Inf and NaN Properties")) {
            scene().add_object<OpenVolumeMeshObject>(mesh_with_nan_and_inf_props());
        }
        if (ImGui::MenuItem("Add empty Mesh")) {
            scene().add_object<OpenVolumeMeshObject>(OVMVolumeMesh());
        }
        ImGui::EndMenu();
    }

    ImGui::SeparatorText("Picking Texture");
    AxoPlotl::app().render_picking_texture_ui();
}
}
