#include "file_access.h"
#include <fstream>

char* AxoPlotl::read_file(const char* path)
{
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        return nullptr;
    }

    // Get file size and allocate buffer (+1 for null terminator)
    std::streamsize size = file.tellg();
    char* buffer = new char[size + 1];

    // Read file contents
    file.seekg(0);
    file.read(buffer, size);
    buffer[size] = '\0';  // Null-terminate the string

    file.close();
    return buffer;
}
