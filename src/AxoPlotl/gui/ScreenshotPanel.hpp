#pragma once

#include <AxoPlotl/core/AxoPlotl_fwd.hpp>
#include <AxoPlotl/typedefs/glm.hpp>
#include <filesystem>

namespace AxoPlotl::internal
{
class ScreenshotPanel
{
private:
    std::filesystem::path filepath = "AxoPlotl_Screenshot.png";
    ViewSize size = {512, 512};
    bool transparent_background = false;
    bool enabled = false;
    
public:
    static constexpr ViewSize MAX_SIZE = {8000,8000};

    void enable() {enabled = true;}

    void render_ui(Application* _app);

};
}