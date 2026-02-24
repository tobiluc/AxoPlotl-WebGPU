// #pragma once
// #include "AxoPlotl/PluginBase.hpp"
// #include <functional>
// #include <typeindex>
// #include <string>

// namespace AxoPlotl
// {

// // forward declarations
// class Application;
// class PluginBase;

// class PluginRegistry
// {
// public:
//     using InstanceMap = std::unordered_map<std::type_index, std::unique_ptr<PluginBase>>;
//     using Factory = std::function<std::unique_ptr<PluginBase>()>;
//     using FactoryMap = std::unordered_map<std::type_index, Factory>;

// private:
//     static FactoryMap& get_factories() {
//         static FactoryMap factories_;
//         return factories_;
//     }

// public:
//     static InstanceMap& get_plugins() {
//         static InstanceMap plugins_;
//         return plugins_;
//     }

//     template<typename T>
//     static void register_plugin()
//     {
//         FactoryMap& factories = get_factories();
//         std::type_index id(typeid(T));
//         factories[id] = [] {
//             return std::make_unique<T>();
//         };
//     }

//     template<typename T>
//     static T& request()
//     {
//         InstanceMap& instances = get_plugins();
//         std::type_index id(typeid(T));

//         auto it = instances.find(id);
//         if (it == instances.end()) {
//             FactoryMap& factories = get_factories();
//             auto fit = factories.find(id);
//             if (fit == factories.end()) {
//                 throw std::runtime_error("Plugin not registered");
//             }

//             auto plugin = fit->second();
//             T& ref = *static_cast<T*>(plugin.get());
//             instances[id] = std::move(plugin);
//             return ref;
//         }
//         return *static_cast<T*>(it->second.get());
//     }

//     static void instantiate_all() {
//         for (auto& [id, factory] : get_factories()) {
//             if (get_plugins().count(id) == 0) {
//                 get_plugins()[id] = factory();
//                 get_plugins()[id]->init();
//             }
//         }
//     }
// };

// }

// #if defined(_MSC_VER)
// #define AXOPLOTL_USED __declspec(selectany)
// #else
// #define AXOPLOTL_USED __attribute__((used))
// #endif

// // Must be called within a Plugin.cpp
// // within the AxoPlotl namespace
// #define REGISTER_AXOPLOTL_PLUGIN(PluginType)\
// namespace {\
//     AXOPLOTL_USED inline const bool registered_##PluginType = []{\
//         PluginRegistry::register_plugin<PluginType>();\
//         return true;\
//     }();\
// }

