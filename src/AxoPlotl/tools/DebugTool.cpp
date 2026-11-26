#include "DebugTool.hpp"
#include "AxoPlotl/AxoPlotl.hpp"
#include "AxoPlotl/debug/meshes.hpp"
#include <AxoPlotl/core/Scene.hpp>
#include <mach/task_info.h>
#include <mach/mach.h>
#include <AxoPlotl/rendering/PickingTextureVis.hpp>
#include <AxoPlotl/core/Application.hpp>
#include <AxoPlotl/gui/fonts.hpp>
#include <AxoPlotl/input/Mouse.hpp>

namespace AxoPlotl::DebugTool
{

void draw_ui()
{
    if (ImGui::CollapsingHeader("Performance"))
    {
        ImGui::Text("%f MB", AxoPlotl::memory_usage_mb());
        ImGui::Text("%f/%f FPS", app().ticks_per_second(), app().fixed_ticks_per_second());
        ImGui::Text("Draw Frames %d", internal::n_draw_frames());
    }

    if (ImGui::CollapsingHeader("Scene"))
    {
        ImGui::Text("#Objects: %zu", AxoPlotl::app().scene().num_objects());
        if (ImGui::BeginMenu("Add")) {
            if (ImGui::MenuItem("Add Mesh with Inf and NaN Properties")) {
                AxoPlotl::app().scene().add_object<OpenVolumeMeshObject>(mesh_with_nan_and_inf_props());
            }
            if (ImGui::MenuItem("Add empty Mesh")) {
                AxoPlotl::app().scene().add_object<OpenVolumeMeshObject>(VolMesh());
            }
            ImGui::EndMenu();
        }
    }

    if (ImGui::CollapsingHeader("Mouse"))
    {
        ImGui::Text("Pos: %f, %f", Mouse::POSITION[0], Mouse::POSITION[1]);
        ImGui::Text("dPos: %f, %f", Mouse::POSITION_DELTA[0], Mouse::POSITION_DELTA[1]);
        ImGui::Text("dScroll: %f, %f", Mouse::SCROLL_DELTA[0], Mouse::SCROLL_DELTA[1]);
    }

    if (ImGui::CollapsingHeader("Picking Texture"))
    {
        AxoPlotl::app().render_picking_texture_ui();
        const auto& p = AxoPlotl::app().picked_pixel();
        ImGui::Text("Entity: (%u, %u, %u)", p.entity.object_id, p.entity.entity_type, p.entity.entity_index);
        ImGui::Text("Pos: (%f, %f, %f)", p.position.x, p.position.y, p.position.z);
        ImGui::Text("NDC: (%f, %f, %f)", p.ndc.x, p.ndc.y, p.ndc.z);
        ImGui::Text("f: %f", p.function_value);
    }
}
}
