#pragma once
#include <GLFW/glfw3.h>
#include <glfw3webgpu.h>
#include <filesystem>

namespace AxoPlotl::gui
{
    GLFWcursor* load_cursor(const std::filesystem::path& _path, int xhot = 0, int yhot = 0);

    void set_test_cursor(GLFWwindow* _window);

}