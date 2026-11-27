#define STB_IMAGE_IMPLEMENTATION
#include "cursors.hpp"
#include <vector>
#include <AxoPlotl/io/stbi_image.h>
#include <iostream>

namespace AxoPlotl::gui
{

GLFWcursor* load_cursor(const std::filesystem::path& _path, int xhot, int yhot)
{
    int width, height, channels;

    unsigned char* pixels = stbi_load(
        (AXOPLOTL_IMAGES_DIR / _path).c_str(),
        &width,
        &height,
        &channels,
        4 // RGBA
    );

    if (!pixels) {
        std::cerr << "Failed to load cursor image: " << stbi_failure_reason() << std::endl;
        return nullptr;
    }

    GLFWimage image;
    image.width = width;
    image.height = height;
    image.pixels = pixels;

    GLFWcursor* cursor = glfwCreateCursor(&image, xhot, yhot);

    stbi_image_free(pixels);

    return cursor;
}

void set_test_cursor(GLFWwindow* _window)
{
    std::vector<unsigned char> pixels;
    pixels.reserve(32*32*4);
    for (int y = 0; y < 32; ++y) {
        for (int x = 0; x < 32; ++x) {
            pixels.push_back(255);
            pixels.push_back(0);
            pixels.push_back(0);
            pixels.push_back(255);
        }
    }

    GLFWimage image;
    image.width = 32;
    image.height = 32;
    image.pixels = pixels.data();

    GLFWcursor* cursor = glfwCreateCursor(&image, 2, 28);
    glfwSetCursor(_window, cursor);
}

}
