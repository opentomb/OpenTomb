
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include "script.h"
#include "../core/system.h"
#include "../core/console.h"
#include "../controls.h"
#include "../render/camera.h"
#include "../render/render.h"
#include "../audio/audio.h"

/*
 * Game structures parse
 */
int Script_ParseControls(lua_State *lua, struct control_settings_s *cs)
{
    if(lua)
    {
        int top = lua_gettop(lua);

        lua_getglobal(lua, "controls");

        lua_getfield(lua, -1, "mouse_sensitivity_x");
        cs->mouse_sensitivity_x = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "mouse_sensitivity_y");
        cs->mouse_sensitivity_y = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "use_joy");
        cs->use_joy = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "joy_number");
        cs->joy_number = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "joy_rumble");
        cs->joy_rumble = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "joy_look_axis_x");
        cs->joy_axis_map[AXIS_LOOK_X] = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "joy_look_axis_y");
        cs->joy_axis_map[AXIS_LOOK_Y] = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "joy_look_invert_x");
        cs->joy_look_invert_x = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "joy_look_invert_y");
        cs->joy_look_invert_y = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "joy_look_sensitivity");
        cs->joy_look_sensitivity = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "joy_look_deadzone");
        cs->joy_look_deadzone = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "joy_move_axis_x");
        cs->joy_axis_map[AXIS_MOVE_X] = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "joy_move_axis_y");
        cs->joy_axis_map[AXIS_MOVE_Y] = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "joy_move_invert_x");
        cs->joy_move_invert_x = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "joy_move_invert_y");
        cs->joy_move_invert_y = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "joy_move_sensitivity");
        cs->joy_move_sensitivity = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "joy_move_deadzone");
        cs->joy_move_deadzone = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_settop(lua, top);
        return 1;
    }

    return -1;
}

int Script_ParseScreen(lua_State *lua, struct screen_info_s *sc)
{
    if(lua)
    {
        int top = lua_gettop(lua);

        lua_getglobal(lua, "screen");
        lua_getfield(lua, -1, "x");
        sc->x = (int16_t)lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "y");
        sc->y = (int16_t)lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "width");
        sc->w = (int16_t)lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "height");
        sc->h = (int16_t)lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "fullscreen");
        sc->fullscreen = (int8_t)lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "debug_view_state");
        sc->debug_view_state = (int8_t)lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "crosshair");
        sc->crosshair = (int8_t)lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "fov");
        sc->fov = (float)lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_settop(lua, top);
        return 1;
    }

    return -1;
}

int Script_ParseRender(lua_State *lua, struct render_settings_s *rs)
{
    if(lua)
    {
        int top = lua_gettop(lua);

        lua_getglobal(lua, "render");
        lua_getfield(lua, -1, "mipmap_mode");
        rs->mipmap_mode = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "mipmaps");
        rs->mipmaps = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "lod_bias");
        rs->lod_bias = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "anisotropy");
        rs->anisotropy = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "antialias");
        rs->antialias = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "antialias_samples");
        rs->antialias_samples = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "texture_border");
        rs->texture_border = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "z_depth");
        rs->z_depth = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "fog_enabled");
        rs->fog_enabled = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "fog_start_depth");
        rs->fog_start_depth = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "fog_end_depth");
        rs->fog_end_depth = lua_tonumber(lua, -1);
        lua_pop(lua, 1);


        lua_getfield(lua, -1, "fog_color");
        if(lua_istable(lua, -1))
        {
            lua_getfield(lua, -1, "r");
            rs->fog_color[0] = (GLfloat)lua_tonumber(lua, -1) / 255.0;
            lua_pop(lua, 1);

            lua_getfield(lua, -1, "g");
            rs->fog_color[1] = (GLfloat)lua_tonumber(lua, -1) / 255.0;
            lua_pop(lua, 1);

            lua_getfield(lua, -1, "b");
            rs->fog_color[2] = (GLfloat)lua_tonumber(lua, -1) / 255.0;
            lua_pop(lua, 1);

            rs->fog_color[3] = 1.0; // Not sure if we need this at all...
        }

        if(rs->z_depth != 8 && rs->z_depth != 16 && rs->z_depth != 24)
        {
            rs->z_depth = 24;
        }

        lua_settop(lua, top);
        return 1;
    }

    return -1;
}

int Script_ParseAudio(lua_State *lua, struct audio_settings_s *as)
{
    if(lua)
    {
        int top = lua_gettop(lua);

        lua_getglobal(lua, "audio");
        lua_getfield(lua, -1, "music_volume");
        as->music_volume = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "sound_volume");
        as->sound_volume = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "use_effects");
        as->use_effects  = lua_tointeger(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "listener_is_player");
        as->listener_is_player = lua_tointeger(lua, -1);
        lua_pop(lua, 1);

        lua_settop(lua, top);
        return 1;
    }

    return -1;
}

int Script_ParseConsole(lua_State *lua, struct console_params_s *cp)
{
    if(lua)
    {
        int top = lua_gettop(lua);

        lua_getglobal(lua, "console");
        lua_getfield(lua, -1, "background_color");
        if(lua_istable(lua, -1))
        {
            lua_getfield(lua, -1, "r");
            cp->background_color[0] = lua_tointeger(lua, -1) & 0xFF;
            lua_pop(lua, 1);

            lua_getfield(lua, -1, "g");
            cp->background_color[1] = lua_tointeger(lua, -1) & 0xFF;
            lua_pop(lua, 1);

            lua_getfield(lua, -1, "b");
            cp->background_color[2] = lua_tointeger(lua, -1) & 0xFF;
            lua_pop(lua, 1);

            lua_getfield(lua, -1, "a");
            cp->background_color[3] = lua_tointeger(lua, -1) & 0xFF;
            lua_pop(lua, 1);
        }
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "spacing");
        cp->spacing = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "height");
        cp->height = lua_tointeger(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "lines_count");
        cp->lines_count = lua_tointeger(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "commands_count");
        cp->commands_count = lua_tointeger(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "show");
        cp->show = (lua_tointeger(lua, -1)) ? (0x01) : (0x00);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "show_cursor_period");
        cp->show_cursor_period = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_settop(lua, top);
        return 1;
    }

    return -1;
}


void Script_LuaRegisterConfigFuncs(lua_State *lua)
{
    char buff[128];
    for(int i = 0; i < ACTIONS::ACT_LASTINDEX; ++i)
    {
        Controls_ActionToStr(buff, (enum ACTIONS)i);
        lua_pushinteger(lua, i);
        lua_setglobal(lua, buff);
    }
}


void Script_ExportConfig(const char *path)
{
    FILE *f = fopen(path, "wb");
    char buff[128];

    if(f)
    {
        fprintf(f, "-- LUA config file\n");
        fprintf(f, "screen =\n{\n");
        fprintf(f, "    x = %d;\n    y = %d;\n", (int)screen_info.x, (int)screen_info.y);
        fprintf(f, "    width = %d;\n    height = %d;\n", (int)screen_info.w, (int)screen_info.h);
        fprintf(f, "    fov = %.1f;\n", screen_info.fov);
        fprintf(f, "    debug_view_state = %d;\n", (int)screen_info.debug_view_state);
        fprintf(f, "    fullscreen = %d;\n", (int)screen_info.fullscreen);
        fprintf(f, "    crosshair = %d;\n", (int)screen_info.crosshair);
        fprintf(f, "}\n\n");

        fprintf(f, "audio =\n{\n");
        fprintf(f, "    sound_volume = %.2f;\n", audio_settings.sound_volume);
        fprintf(f, "    music_volume = %.2f;\n", audio_settings.music_volume);
        fprintf(f, "    use_effects = %d;\n", (int)audio_settings.use_effects);
        fprintf(f, "    listener_is_player = %d;\n", (int)audio_settings.listener_is_player);
        fprintf(f, "}\n\n");

        fprintf(f, "render =\n{\n");
        fprintf(f, "    mipmap_mode = %d;\n", renderer.settings.mipmap_mode);
        fprintf(f, "    mipmaps = %d;\n", renderer.settings.mipmaps);
        fprintf(f, "    lod_bias = %.3f;\n", renderer.settings.lod_bias);
        fprintf(f, "    anisotropy = %d;\n", renderer.settings.anisotropy);
        fprintf(f, "    antialias = %d;\n", renderer.settings.antialias);
        fprintf(f, "    antialias_samples = %d;\n", renderer.settings.antialias_samples);
        fprintf(f, "    z_depth = %d;\n", renderer.settings.z_depth);
        fprintf(f, "    texture_border = %d;\n", renderer.settings.texture_border);
        {
            int r = renderer.settings.fog_color[0] * 255.5f;
            int g = renderer.settings.fog_color[1] * 255.5f;
            int b = renderer.settings.fog_color[2] * 255.5f;
            fprintf(f, "    fog_color = {r = %d, g = %d, b = %d};\n", r, g, b);
        }
        fprintf(f, "}\n\n");

        fprintf(f, "controls =\n{\n");
        fprintf(f, "    mouse_sensitivity_x = %.2f;\n", control_settings.mouse_sensitivity_x);
        fprintf(f, "    mouse_sensitivity_y = %.2f;\n\n", control_settings.mouse_sensitivity_y);
        fprintf(f, "    use_joy = %d;\n", (int)control_settings.use_joy);
        fprintf(f, "    joy_number = %d;\n", (int)control_settings.joy_number);
        fprintf(f, "    joy_rumble = %d;\n\n", (int)control_settings.joy_rumble);
        fprintf(f, "    joy_move_axis_x = %d;\n", (int)control_settings.joy_axis_map[AXIS_MOVE_X]);
        fprintf(f, "    joy_move_axis_y = %d;\n", (int)control_settings.joy_axis_map[AXIS_MOVE_Y]);
        fprintf(f, "    joy_move_invert_x = %d;\n", (int)control_settings.joy_move_invert_x);
        fprintf(f, "    joy_move_invert_y = %d;\n", (int)control_settings.joy_move_invert_y);
        fprintf(f, "    joy_move_sensitivity = %.2f;\n", control_settings.joy_move_sensitivity);
        fprintf(f, "    joy_move_deadzone = %d;\n\n", (int)control_settings.joy_move_deadzone);
        fprintf(f, "    joy_look_axis_x = %d;\n", (int)control_settings.joy_axis_map[AXIS_LOOK_X]);
        fprintf(f, "    joy_look_axis_y = %d;\n", (int)control_settings.joy_axis_map[AXIS_LOOK_Y]);
        fprintf(f, "    joy_look_invert_x = %d;\n", (int)control_settings.joy_look_invert_x);
        fprintf(f, "    joy_look_invert_y = %d;\n", (int)control_settings.joy_look_invert_y);
        fprintf(f, "    joy_look_sensitivity = %.2f;\n", control_settings.joy_look_sensitivity);
        fprintf(f, "    joy_look_deadzone = %d;\n", (int)control_settings.joy_look_deadzone);
        fprintf(f, "}\n\n");

        {
            console_params_t cp = { 0 };
            Con_GetParams(&cp);
            fprintf(f, "console =\n{\n");
            fprintf(f, "    background_color = {r = %d, g = %d, b = %d, a = %d};\n",
                cp.background_color[0],
                cp.background_color[1],
                cp.background_color[2],
                cp.background_color[3]);
            fprintf(f, "    commands_count = %d;\n", cp.commands_count);
            fprintf(f, "    lines_count = %d;\n", cp.lines_count);
            fprintf(f, "    height = %d;\n", cp.height);
            fprintf(f, "    spacing = %.2f;\n", cp.spacing);
            fprintf(f, "    show_cursor_period = %.2f;\n", cp.show_cursor_period);
            fprintf(f, "    show = 0;\n");
            fprintf(f, "}\n\n");
        }

        fprintf(f, "-- Keys binding\n"
                   "-- Please note that on XInput game controllers (XBOX360 and such), triggers are NOT\n"
                   "-- coded as joystick buttons. Instead, they have unique names: JOY_TRIGGERLEFT and\n"
                   "-- JOY_TRIGGERRIGHT.\n\n"
                   "dofile(base_path .. \"scripts/config/control_constants.lua\");\n\n");

        for(int i = 0; i < ACT_LASTINDEX; ++i)
        {
            control_action_p act = control_states.actions + i;
            if(act->primary)
            {
                Controls_ActionToStr(buff, (enum ACTIONS)i);
                fprintf(f, "bind(");
                fputs(buff, f);
                fprintf(f, ", ");
                Controls_KeyToStr(buff, act->primary);
                fputs(buff, f);
                if(act->secondary)
                {
                    fprintf(f, ", ");
                    Controls_KeyToStr(buff, act->secondary);
                    fputs(buff, f);
                }
                fprintf(f, ");\n");
            }
        }
        fclose(f);
    }
}
