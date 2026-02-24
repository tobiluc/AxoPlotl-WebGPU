#pragma once
#include <filesystem>
#include <string>

namespace AxoPlotl
{

// std::string parse_shader_with_includes(const std::string& _source);

std::string parse_shader_file_with_includes(std::filesystem::path _path);

}
