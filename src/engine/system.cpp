#include "system.h"

#include "gui/console.h"
#include "gui/gui.h"
#include "gui/textline.h"

namespace engine
{

ScreenInfo     screen_info;
SystemSettings system_settings;

gui::TextLine system_fps;

// =======================================================================
// General routines
// =======================================================================

void Sys_Init()
{
    system_fps.text.clear();

    system_fps.X = 10.0;
    system_fps.Y = 10.0;
    system_fps.Xanchor = gui::HorizontalAnchor::Right;
    system_fps.Yanchor = gui::VerticalAnchor::Bottom;

    system_fps.font_id = gui::FontType::Primary;
    system_fps.style_id = gui::FontStyle::MenuTitle;

    system_fps.show = true;

    gui::addLine(&system_fps);
}

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

void Sys_Destroy()
{
    system_fps.show = false;
    system_fps.text.clear();
}

} // namespace engine
