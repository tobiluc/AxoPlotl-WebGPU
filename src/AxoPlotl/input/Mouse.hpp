#ifndef MOUSEHANDLER_H
#define MOUSEHANDLER_H

#include <GLFW/glfw3.h>
#include <ostream>

namespace AxoPlotl::internal
{

class Mouse
{
public:
    enum class Button {LEFT, RIGHT, MIDDLE};

    static void update(GLFWwindow* window);

    static float POSITION[2];
    static float POSITION_DELTA[2];
    static float SCROLL_DELTA[2];
    static bool LEFT_PRESSED;
    static bool RIGHT_PRESSED;
    static bool MIDDLE_PRESSED;
    static bool LEFT_JUST_PRESSED;
    static bool RIGHT_JUST_PRESSED;
    static bool MIDDLE_JUST_PRESSED;
    static bool LEFT_JUST_RELEASED;
    static bool RIGHT_JUST_RELEASED;
    static bool MIDDLE_JUST_RELEASED;
};

void mouse_callback(GLFWwindow* window, double mouse_x, double mouse_y);

void scroll_callback(GLFWwindow* window, double dx, double dy);

inline std::ostream& operator<<(std::ostream& _os, Mouse::Button _button) {
    switch (_button) {
    case Mouse::Button::LEFT: return _os << "Mouse::Button::LEFT";
    case Mouse::Button::RIGHT: return _os << "Mouse::Button::RIGHT";
    case Mouse::Button::MIDDLE: return _os << "Mouse::Button::MIDDLE";
    default: return _os;
    }
}

}

#endif // MOUSEHANDLER_H
