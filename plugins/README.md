# AxoPlotl Plugin System

The Plugin System is designed with the goal
to be as simple as possible, i.e. cause
the user as little headaches as possible.

To add a Plugin, you have to do the following:

1. Create a Plugin header (.hpp) with a Plugin class extending from PluginBase.
```cpp
#pragma once
#include "AxoPlotl/PluginBase.hpp"

namespace AxoPlotl
{
class MyNewPlugin : public PluginBase
{
public:
    MyNewPlugin() {}

    // We want to implement a custom
    // render function.
    void render_ui(Application& app) override;

    // This is optional, but it is
    // nice to have a custom name
    // showing up
    const char* name() const override {
        return "My New Plugin";
    }
};
}
```
2. Create a Plugin source and add this line within the AxoPlotl namespace:
```
REGISTER_AXOPLOTL_PLUGIN(MyNewPlugin)
```

```
#include "MyNewPlugin.hpp"

namespace AxoPlotl
{

void MyNewPlugin::render_ui(Application& app)
{
    ImGui::Text("Hello World");
}

// With this line, everything is handled
REGISTER_AXOPLOTL_PLUGIN(TestPlugin)

}
```
3. Add the MyNewPlugin.cpp to the Plugin.cmake within the plugins folder. You can also link
external libraries to it:
```
add_plugin(${PLUGINS_DIR}/MyNewPlugin.cpp
LINK_LIBS libGurti
)
```
