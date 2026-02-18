#include "Mouse.hpp"
#include "imgui.h"

namespace AxoPlotl::Input
{

float Mouse::POSITION[2] = {0,0};
float Mouse::POSITION_DELTA[2] = {0,0};
float Mouse::SCROLL_DELTA[2] = {0,0};
bool Mouse::LEFT_PRESSED = false;
bool Mouse::RIGHT_PRESSED = false;
bool Mouse::LEFT_JUST_PRESSED = false;
bool Mouse::RIGHT_JUST_PRESSED = false;
bool Mouse::LEFT_JUST_RELEASED = false;
bool Mouse::RIGHT_JUST_RELEASED = false;

void Mouse::update(GLFWwindow* window)
{
    Mouse::LEFT_JUST_PRESSED = !Mouse::LEFT_PRESSED && (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
    Mouse::LEFT_PRESSED = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
    Mouse::LEFT_JUST_RELEASED = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE);
    Mouse::RIGHT_JUST_PRESSED = !Mouse::RIGHT_PRESSED && (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);
    Mouse::RIGHT_PRESSED = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);
    Mouse::RIGHT_JUST_RELEASED = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE);

    Mouse::SCROLL_DELTA[0] = 0;
    Mouse::SCROLL_DELTA[1] = 0;
}

void mouse_callback(GLFWwindow* window, double mouse_x, double mouse_y)
{
    // Rendering::triggerRedraw();
    if (ImGui::GetIO().WantCaptureMouse) {return;}

    Mouse::POSITION_DELTA[0] = (float)(mouse_x - Mouse::POSITION[0]);
    Mouse::POSITION_DELTA[1] = (float)(Mouse::POSITION[1] - mouse_y); // reversed since y-coordinates range from bottom to top
    Mouse::POSITION[0] = (float)mouse_x;
    Mouse::POSITION[1] = (float)mouse_y;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) return; // Rotate by dragging
}

void scroll_callback(GLFWwindow* window, double dx, double dy)
{
    // Rendering::triggerRedraw();
    if (ImGui::GetIO().WantCaptureMouse) {return;}

    Mouse::SCROLL_DELTA[0] = dx;
    Mouse::SCROLL_DELTA[1] = dy;
}

}
