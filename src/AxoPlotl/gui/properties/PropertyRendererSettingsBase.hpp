#pragma once

#include <string>

namespace AxoPlotl::internal
{

class PropertySettingsBase
{
public:
    virtual std::string name() const = 0;
    virtual void render_ui() = 0;
    virtual void set_default_settings() = 0;
};

}
