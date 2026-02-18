#ifndef MOUSEHANDLER_H
#define MOUSEHANDLER_H

#include <GLFW/glfw3.h>

namespace AxoPlotl::Input
{

class Mouse
{
public:
    static void update(GLFWwindow* window);

    static float POSITION[2];
    static float POSITION_DELTA[2];
    static float SCROLL_DELTA[2];
    static bool LEFT_PRESSED;
    static bool RIGHT_PRESSED;
    static bool LEFT_JUST_PRESSED;
    static bool RIGHT_JUST_PRESSED;
    static bool LEFT_JUST_RELEASED;
    static bool RIGHT_JUST_RELEASED;
};

void mouse_callback(GLFWwindow* window, double mouse_x, double mouse_y);

void scroll_callback(GLFWwindow* window, double dx, double dy);

}

#endif // MOUSEHANDLER_H
