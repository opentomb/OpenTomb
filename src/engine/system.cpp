#include "system.h"

namespace engine
{

ScreenInfo     screen_info;
SystemSettings system_settings;

// =======================================================================
// General routines
// =======================================================================

void Sys_InitGlobals()
{
    screen_info.x = 50;
    screen_info.y = 20;
    screen_info.w = 800;
    screen_info.h = 600;
    screen_info.FS_flag = 0;
    screen_info.show_debuginfo = 0;
    screen_info.fov = 75.0;

    system_settings.logging = false;
}

} // namespace engine
