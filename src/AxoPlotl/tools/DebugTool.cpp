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

const char* icon() {return ICON_FA_BUG;}

void render_ui()
{
    if (ImGui::CollapsingHeader("Performance"))
    {
        ImGui::Text("%f MB", AxoPlotl::memory_usage_mb());
        ImGui::Text("%f FPS", AxoPlotl::frames_per_second());
        ImGui::Text("Draw Frames %d", internal::n_draw_frames());
    }

    if (ImGui::CollapsingHeader("Scene"))
    {
        ImGui::Text("#Objects: %zu", AxoPlotl::scene().num_objects());
        if (ImGui::BeginMenu("Add")) {
            if (ImGui::MenuItem("Add Mesh with Inf and NaN Properties")) {
                scene().add_object<OpenVolumeMeshObject>(mesh_with_nan_and_inf_props());
            }
            if (ImGui::MenuItem("Add empty Mesh")) {
                scene().add_object<OpenVolumeMeshObject>(VolMesh());
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
    }
}
}
