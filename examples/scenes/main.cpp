#define WEBGPU_CPP_IMPLEMENTATION
#include <AxoPlotl/AxoPlotl.hpp>
#include <AxoPlotl/tools/tools.hpp>

int main()
{
    const std::vector<std::filesystem::path> mesh_files = {
        "../../../../assets/meshes/sphere.obj",
        "../../../../assets/meshes/cube.obj",
        "../../../../assets/meshes/torus.obj"
    };

    // Default Inspector
    AxoPlotl::app().inspector_callback() = {[&]()
    {
        if (ImGui::BeginTabBar("")) {
            AxoPlotl::gui::render_tools_tab_items_ui();
            ImGui::EndTabBar();
        }
    }};

    // AxoPlotl needs to be initialized first.
    if (!AxoPlotl::app().init()) {
        std::cerr << "Failed to initialize AxoPlotl" << std::endl;
        return 1;
    }

    // Let's add the first mesh to the default scene
    AxoPlotl::app().scene().add_mesh(mesh_files[0]);
    AxoPlotl::app().scene().name = "Sphere";

    // Add another scene with the name "Cube" with another mesh
    AxoPlotl::app().add_scene("Cube").add_mesh(mesh_files[1]);

    // And another one with slightly different syntax
    auto& scene = AxoPlotl::app().add_scene("Donut");
    scene.add_mesh(mesh_files[2]);

    // Set the active scene
    AxoPlotl::app().set_scene("Cube");

    // Finally, run the application
    AxoPlotl::app().run();

    return 0;
}
