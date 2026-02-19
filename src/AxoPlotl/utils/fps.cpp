#include "fps.hpp"
#include "GLFW/glfw3.h"

namespace AxoPlotl
{

float Time::DELTA_TIME = 0;
float Time::FRAMES_PER_SECOND = 0;
// float Time::SECONDS_SINCE_LAST_FRAME = 0;
// float Time::TIME_OF_LAST_FRAME = 0;
static float SECONDS_SINCE_LAST_FRAME = 0;
static float TIME_OF_LAST_FRAME = 0;

void Time::update()
{
    // Compute frames per second
    float TIME_OF_CURRENT_FRAME = (float)glfwGetTime();
    DELTA_TIME = TIME_OF_CURRENT_FRAME - TIME_OF_LAST_FRAME;
    TIME_OF_LAST_FRAME = TIME_OF_CURRENT_FRAME;
    SECONDS_SINCE_LAST_FRAME += DELTA_TIME;
    if (SECONDS_SINCE_LAST_FRAME >= 1.0f) {
        SECONDS_SINCE_LAST_FRAME -= 1.0f;
        FRAMES_PER_SECOND = 1.0f / DELTA_TIME;
    }
}

}
