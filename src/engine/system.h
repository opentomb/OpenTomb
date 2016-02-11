#pragma once

#include "util/helpers.h"
#include "gui/common.h"

#include <glm/glm.hpp>

#include <cstdint>

namespace engine
{
#define LOG_FILENAME     "debug.log"
#define LUA_LOG_FILENAME "lua.log"
#define GL_LOG_FILENAME  "gl.log"

struct SystemSettings
{
    bool logging = false;

    explicit SystemSettings(boost::property_tree::ptree& config)
    {
        logging = util::getSetting(config, "logging", false);
    }
};

struct ScreenInfo
{
    int16_t     x = 50;
    int16_t     y = 20;
    int16_t     w = 800;
    glm::float_t w_unit;   // Metering unit.
    int16_t     h = 600;
    glm::float_t h_unit;   // Metering unit.

    float       fps;
    float       fov = 75.0f;
    float       scale_factor = 1;
    bool        FS_flag = false;
    bool        show_debuginfo = false;
    bool        vsync = true;

    explicit ScreenInfo(boost::property_tree::ptree& config)
    {
        x = util::getSetting(config, "x", 50);
        y = util::getSetting(config, "y", 20);
        w = util::getSetting(config, "w", 800);
        h = util::getSetting(config, "h", 600);
        w_unit = w / gui::ScreenMeteringResolution;
        h_unit = h / gui::ScreenMeteringResolution;
        fov = util::getSetting(config, "fov", 75.0f);
        FS_flag = util::getSetting(config, "fullscreen", false);
        show_debuginfo = util::getSetting(config, "showDebugInfo", false);
        vsync = util::getSetting(config, "vsync", true);
    }
};

} // namespace engine
