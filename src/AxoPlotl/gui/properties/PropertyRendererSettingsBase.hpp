#pragma once

namespace AxoPlotl::internal
{

class PropertySettingsBase
{
public:
    virtual void render_ui() = 0;
    virtual void set_default_settings() = 0;
};

}
