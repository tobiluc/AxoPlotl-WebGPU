#define WEBGPU_CPP_IMPLEMENTATION

#include <AxoPlotl/AxoPlotl.hpp>

int main(int argc, char* argv[])
{
    // Specify the input mesh file and screenshot png path using commandline parameters.
    if (argc != 3) {
        std::cerr << "Usage: <in_mesh_path> <out_image_path>" << std::endl;
        return 1;
    }
    const std::filesystem::path in_mesh_path = argv[1];
    const std::filesystem::path out_image_path = argv[2];

    // AxoPlotl needs to be initialized first.
    if (!AxoPlotl::app().init()) {
      std::cerr << "Failed to initialize AxoPlotl" << std::endl;
      return 1;
    }

    // Add a mesh to the Scene - The camera automatically zooms to the newly added object.
    auto obj = AxoPlotl::app().scene().add_mesh(in_mesh_path);
    if (!obj) {
        std::cerr << "Failed to read mesh" << std::endl;
    }

    // You could obj.entity_renderer<OpenVolumeMesh::Entity::...> here to configure
    // things like colors/line thickness/...
    // Similarly, you could configure AxoPlotl::app().scene().directional_light

    // We run the application for 100 frames. Normally textures and other resources are released
    // after run is called, so we set manual_cleanup=true.
    AxoPlotl::app().run(AxoPlotl::RunConfig{.num_ticks=100, .manual_cleanup=true, .headless=true});

    AxoPlotl::app().save_viewport_screenshot(
        out_image_path,
        AxoPlotl::app().viewport().size().scaled(4), // 4 x resolution of actual viewport
        true); // transparent background = true

    // Our manual cleanup.
    AxoPlotl::app().cleanup();

    return 0;
}
