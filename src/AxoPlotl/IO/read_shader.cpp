#include <AxoPlotl/IO/read_shader.hpp>
#include <fstream>
#include <sstream>

namespace AxoPlotl::IO
{

std::string parse_shader_file_with_includes(std::filesystem::path _path)
{
    _path = std::filesystem::path(AXOPLOTL_SHADERS_DIR)/_path;

    if (!std::filesystem::exists(_path)) {
        throw std::runtime_error("Shader file not found: " + _path.string());
    }

    // Read entire file
    std::ifstream file(_path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open shader file: " + _path.string());
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    std::string result = source;
    size_t pos = 0;

    while ((pos = result.find("#include", pos)) != std::string::npos)
    {
        size_t quote_start = result.find('"', pos);
        size_t quote_end   = result.find('"', quote_start + 1);

        if (quote_start == std::string::npos ||
            quote_end   == std::string::npos ||
            quote_start >= quote_end)
        {
            throw std::runtime_error("Malformed #include directive in shader:\n" + _path.string());
        }

        std::string include_name =
            result.substr(quote_start + 1, quote_end - quote_start - 1);

        // Resolve relative to current file directory
        std::filesystem::path include_path =
            _path.parent_path() / include_name;

        // Recursively parse included file
        std::string included_source =
            parse_shader_file_with_includes(include_path);

        // Replace entire #include line
        size_t line_end = result.find('\n', quote_end);
        if (line_end == std::string::npos) {
            line_end = result.size();
        }

        result.replace(pos, line_end - pos, included_source);

        // Continue searching after inserted content
        pos += included_source.size();
    }

    return result;
}

}
