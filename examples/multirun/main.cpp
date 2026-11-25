#define WEBGPU_CPP_IMPLEMENTATION
#include <AxoPlotl/AxoPlotl.hpp>
#include <AxoPlotl/tools/tools.hpp>

int main()
{
    const std::vector<std::filesystem::path> mesh_files = {
        "../../../../assets/meshes/sphere.obj",
        "../../../../assets/meshes/cube.obj"
    };

    // Default Inspector
    AxoPlotl::app().inspector_callback() = {[&]()
    {
        if (ImGui::BeginTabBar("")) {
            if (ImGui::BeginTabItem(AxoPlotl::DataControlTool::icon())) {
                AxoPlotl::DataControlTool::render_ui();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem(AxoPlotl::PlottingTool::icon())) {
                AxoPlotl::PlottingTool::render_ui();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem(AxoPlotl::ScreenshotTool::icon())) {
                AxoPlotl::ScreenshotTool::render_ui();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem(AxoPlotl::DebugTool::icon())) {
                AxoPlotl::DebugTool::render_ui();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }};

    // AxoPlotl needs to be initialized first.
    if (!AxoPlotl::app().init()) {
        std::cerr << "Failed to initialize AxoPlotl" << std::endl;
        return 1;
    }

    // We want the manual cleanup at the end, not automatically after each run
    AxoPlotl::RunConfig cfg{.manual_cleanup = true};

    // Load each mesh
    for (const auto& mesh_file : mesh_files)
    {
        // Don't keep the mesh from the previous iteration
        AxoPlotl::app().scene().delete_objects();

        // Once initialized, we can add stuff to the scene
        auto obj = AxoPlotl::app().scene().add_mesh(mesh_file);
        if (!obj) {
            std::cerr << "Failed to load mesh " << mesh_file << std::endl;
            continue;
        }

        // Run the application. Once you close the window, the code continues
        // with the next mesh
        AxoPlotl::app().run(cfg);
    }

    AxoPlotl::app().cleanup();

    return 0;
}
