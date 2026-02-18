#pragma once
#include <functional>
#include <iostream>
#include <vector>

namespace AxoPlotl
{

// forward declarations
class Application;
class PluginBase;

class PluginRegistry
{
public:
    using PluginFactory = std::function<PluginBase*()>;

    // Called by plugins at static initialization
    static void register_plugin(PluginFactory fn) {
        get_factories().push_back(fn);
    }

    // Called by Application at runtime
    // to get actual plugin instances
    static std::vector<PluginBase*>& get_plugins() {
        static std::vector<PluginBase*> instances;
        if (instances.empty()) {
            for (auto& fn : get_factories()) {
                instances.push_back(fn());
            }
        }
        return instances;
    }

private:
    static std::vector<PluginFactory>& get_factories() {
        static std::vector<PluginFactory> factories;
        return factories;
    }
};

}

// Must be called within a Plugin.cpp
// within the AxoPlotl namespace
#define REGISTER_AXOPLOTL_PLUGIN(PluginType)                 \
namespace {                                        \
    struct PluginType##_Registrator {              \
        PluginType##_Registrator() {               \
            PluginRegistry::register_plugin([](){ return new PluginType(); }); \
    }                                          \
} PluginType##_registrator;                    \
}
