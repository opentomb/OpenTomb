
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "script.h"
#include "system.h"
#include "console.h"
#include "entity.h"
#include "world.h"
#include "engine.h"
#include "controls.h"
#include "game.h"
#include "anim_state_control.h"
#include "character_controller.h"
#include "vmath.h"
#include "render.h"
#include "audio.h"

extern "C" {
#include "lua/lua.h"
#include "lua/lualib.h"
#include "lua/lauxlib.h"
}

/*
 * CVARS section
 */
int CVAR_Register(const char *name, const char *val)
{
    int top;

    if(!name || !engine_lua || !val)
    {
        return -1;
    }

    top = lua_gettop(engine_lua);
    lua_getglobal(engine_lua, CVAR_LUA_TABLE_NAME);
    if (!lua_istable(engine_lua, -1))
    {
        lua_settop(engine_lua, top);
        return 0;
    }

    lua_pushstring(engine_lua, val);
    lua_setfield(engine_lua, -2, name);
    lua_settop(engine_lua, top);
    return 1;
}


/*
 * delete cvar.field
 */
int CVAR_Delete(const char *name)
{
    int top;

    if(!name || !engine_lua)
    {
        return -1;
    }

    top = lua_gettop(engine_lua);
    lua_getglobal(engine_lua, CVAR_LUA_TABLE_NAME);
    if (!lua_istable(engine_lua, -1))
    {
        lua_settop(engine_lua, top);
        return 0;
    }

    lua_pushnil(engine_lua);
    lua_setfield(engine_lua, -2, name);
    lua_settop(engine_lua, top);
    return 1;
}


btScalar CVAR_get_val_d(const char *key)
{
    int top = lua_gettop(engine_lua);
    btScalar ret;

    lua_getglobal(engine_lua, CVAR_LUA_TABLE_NAME);
    if (!lua_istable(engine_lua, -1))
    {
        lua_settop(engine_lua, top);
        return 0.0;
    }

    lua_getfield(engine_lua, -1, key);
    ret = lua_tonumber(engine_lua, -1);
    lua_settop(engine_lua, top);
    return ret;
}

const char *CVAR_get_val_s(const char *key)
{
    int top = lua_gettop(engine_lua);
    const char *ret;

    lua_getglobal(engine_lua, CVAR_LUA_TABLE_NAME);
    if (!lua_istable(engine_lua, -1))
    {
        lua_settop(engine_lua, top);
        return NULL;
    }

    lua_getfield(engine_lua, -1, key);
    ret = lua_tostring(engine_lua, -1);
    lua_settop(engine_lua, top);
    return ret;
}

int CVAR_set_val_d(const char *key, btScalar val)
{
    int top = lua_gettop(engine_lua);

    lua_getglobal(engine_lua, CVAR_LUA_TABLE_NAME);
    if (!lua_istable(engine_lua, -1))
    {
        lua_settop(engine_lua, top);
        return 0;
    }

    lua_pushnumber(engine_lua, val);
    lua_setfield(engine_lua, -2, key);
    lua_settop(engine_lua, top);
    return 1;
}

int CVAR_set_val_s(const char *key, const char *val)
{
    int top = lua_gettop(engine_lua);

    lua_getglobal(engine_lua, CVAR_LUA_TABLE_NAME);
    if (!lua_istable(engine_lua, -1))
    {
        lua_settop(engine_lua, top);
        return 0;
    }

    lua_pushstring(engine_lua, val);
    lua_setfield(engine_lua, -2, key);
    lua_settop(engine_lua, top);
    return 1;
}

/*
 * MISK
 */
char *parse_token(char *data, char *token)
{
    //FIXME: Проверять token на переполнение
    int c;
    int len;

    len = 0;
    token[0] = 0;

    if(!data)
    {
        return NULL;
    }

// skip whitespace
    skipwhite:
    while((c = *data) <= ' ')
    {
        if(c == 0)
            return NULL;                    // end of file;
        data++;
    }

// skip // comments
    if (c=='/' && data[1] == '/')
    {
        while (*data && *data != '\n')
            data++;
        goto skipwhite;
    }

// handle quoted strings specially
    if (c == '\"')
    {
        data++;
        while (1)
        {
            c = *data++;
            if (c=='\"' || !c)
            {
                token[len] = 0;
                return data;
            }
            token[len] = c;
            len++;
        }
    }


// parse single characters
    if (c=='{' || c=='}'|| c==')'|| c=='(' || c=='\'' || c==':')
    {
        token[len] = c;
        len++;
        token[len] = 0;
        return data+1;
    }


// parse a regular word
    do
    {
        token[len] = c;
        data++;
        len++;
        c = *data;
        if (c=='{' || c=='}'|| c==')'|| c=='(' || c=='\'' || c==':')
        {
            break;
        }
    } while (c>32);

    token[len] = 0;
    return data;
}

float SC_ParseFloat(char **ch)
{
    char token[64];
    (*ch) = parse_token(*ch, token);
    if(token[0])
    {
        return atof(token);
    }
    return 0.0;
}

int SC_ParseInt(char **ch)
{
    char token[64];
    (*ch) = parse_token(*ch, token);
    if(token[0])
    {
        return atoi(token);
    }
    return 0;
}

/**
 * get tbl[key]
 */
const char *lua_GetStrField(lua_State *lua, const char *key)
{
    int top;
    const char *ret;

    top = lua_gettop(lua);
    if (!lua_istable(lua, -1))
    {
        return NULL;
    }

    lua_getfield(lua, -1, key);
    ret = lua_tostring(lua, -1);
    lua_settop(lua, top);
    return ret;
}


btScalar lua_GetScalarField(lua_State *lua, const char *key)
{
    int top;
    btScalar ret = 0.0;

    top = lua_gettop(lua);
    if (!lua_istable(lua, -1))
    {
        return 0.0;
    }

    lua_getfield(lua, -1, key);
    ret = lua_tonumber(lua, -1);
    lua_settop(lua, top);
    return ret;
}

/**
 * set tbl[key]
 */
int lua_SetScalarField(lua_State *lua, const char *key, btScalar val)
{
    int top;

    top = lua_gettop(lua);
    if (!lua_istable(lua, -1))
    {
        return 0;
    }

    lua_pushnumber(lua, val);
    lua_setfield(lua, -2, key);
    lua_settop(lua, top);
    return 1;
}


int lua_SetStrField(lua_State *lua, const char *key, const char *val)
{
    int top;

    top = lua_gettop(lua);
    if (!lua_istable(lua, -1))
    {
        return 0;
    }

    lua_pushstring(lua, val);
    lua_setfield(lua, -2, key);
    lua_settop(lua, top);
    return 1;
}

/*
 * Gameplay functions
 */
 
int lua_DoTasks(lua_State *lua, btScalar time)
{
    int top;

    top = lua_gettop(lua);
    lua_pushnumber(lua, time);
    lua_setglobal(lua, "frame_time");
    lua_getglobal(lua, "doTasks");
    lua_pcall(lua, 0, 0, 0);
    lua_settop(lua, top);
}
 
int lua_ActivateEntity(lua_State *lua, int id_object, int id_activator)
{
    int top;
    
    top = lua_gettop(lua);
    lua_getglobal(lua, "activateEntity");
    if (!lua_isfunction(lua, -1))
    {
        lua_settop(lua, top);
        //Sys_Warn("Broken \"activateEntity\" script function");
        return -1;
    }
    
    lua_pushinteger(lua, id_object);
    lua_pushinteger(lua, id_activator);
    lua_pcall(lua, 2, 0, 0);
    lua_settop(lua, top);
    return 1;
}


/*
 * Game structures parse
 */
int lua_ParseControlSettings(lua_State *lua, struct control_settings_s *cs)
{
    if(!lua)
    {
        return -1;
    }

    int top = lua_gettop(lua);
    lua_getglobal(lua, "controls");
    cs->mouse_sensitivity = lua_GetScalarField(lua, "mouse_sensitivity");
    cs->use_joy = lua_GetScalarField(lua, "use_joy");
    cs->joy_number = lua_GetScalarField(lua, "joy_number");
    cs->joy_rumble = lua_GetScalarField(lua, "joy_rumble");
    cs->joy_axis_map[AXIS_LOOK_X] = lua_GetScalarField(lua, "joy_look_axis_x");
    cs->joy_axis_map[AXIS_LOOK_Y] = lua_GetScalarField(lua, "joy_look_axis_y");
    cs->joy_look_invert_x = lua_GetScalarField(lua, "joy_look_invert_x");
    cs->joy_look_invert_y = lua_GetScalarField(lua, "joy_look_invert_y");
    cs->joy_look_sensitivity = lua_GetScalarField(lua, "joy_look_sensitivity");
    cs->joy_look_deadzone = lua_GetScalarField(lua, "joy_look_deadzone");
    cs->joy_axis_map[AXIS_MOVE_X] = lua_GetScalarField(lua, "joy_move_axis_x");
    cs->joy_axis_map[AXIS_MOVE_Y] = lua_GetScalarField(lua, "joy_move_axis_y");
    cs->joy_move_invert_x = lua_GetScalarField(lua, "joy_move_invert_x");
    cs->joy_move_invert_y = lua_GetScalarField(lua, "joy_move_invert_y");
    cs->joy_move_sensitivity = lua_GetScalarField(lua, "joy_move_sensitivity");
    cs->joy_move_deadzone = lua_GetScalarField(lua, "joy_move_deadzone");
    lua_settop(lua, top);

    return 1;
}


bool lua_GetSoundtrack(lua_State *lua, int track_index, char *file_path, int *load_method, int *stream_type)
{
    size_t  string_length  = 0;
    int     track_type     = 0;
    int     top;
    
    const char *real_path;
        
    if(lua)
    {
        top = lua_gettop(lua);                                             // save LUA stack
        
        lua_getglobal(lua, "GetTrackInfo");                                // add to the up of stack LUA's function

        if(lua_isfunction(lua, -1))                                        // If function exists...
        {
            lua_pushinteger(lua, engine_world.version);                    // add to stack first argument
            lua_pushinteger(lua, track_index);                             // add to stack second argument

            lua_pcall(lua, 2, 3, 0);                                       // call that function
            
            real_path   = lua_tolstring(lua, -3, &string_length);          // get returned value 1
           *stream_type = lua_tointeger(lua, -2);                          // get returned value 2
           *load_method = lua_tointeger(lua, -1);                          // get returned value 3
           
            // For some reason, Lua returns constant string pointer, which we can't assign to
            // provided argument; so we need to straightly copy it.
        
            strcpy(file_path, real_path);
            
            lua_settop(lua, top);                                          // restore LUA stack
            
            if(*stream_type != -1)
                return true;        // Entry extracted, success!
        }
        else
        {
            lua_settop(lua, top);   // restore LUA stack
        }
    }
    
    // If Lua wasn't able to extract file path from the script, most likely it means
    // that entry is broken or missing, or wrong track ID was specified. So we return
    // FALSE in such cases.
    
    return false;
}


int lua_ParseScreen(lua_State *lua, struct screen_info_s *sc)
{
    if(!lua)
    {
        return -1;
    }

    int top = lua_gettop(lua);
    lua_getglobal(lua, "screen");
    sc->x = lua_GetScalarField(lua, "x");
    sc->y = lua_GetScalarField(lua, "y");
    sc->w = lua_GetScalarField(lua, "width");
    sc->h = lua_GetScalarField(lua, "height");
    sc->FS_flag = lua_GetScalarField(lua, "fullscreen");
    sc->fov = lua_GetScalarField(lua, "fov");
    lua_settop(lua, top);

    return 1;
}

int lua_ParseRender(lua_State *lua, struct render_settings_s *rs)
{
    if(!lua)
    {
        return -1;
    }

    int top = lua_gettop(lua);
    lua_getglobal(lua, "render");
    rs->mipmap_mode = lua_GetScalarField(lua, "mipmap_mode");
    rs->mipmaps = lua_GetScalarField(lua, "mipmaps");
    rs->lod_bias = lua_GetScalarField(lua, "lod_bias");
    rs->anisotropy = lua_GetScalarField(lua, "anisotropy");
    rs->antialias = lua_GetScalarField(lua, "antialias");
    rs->antialias_samples = lua_GetScalarField(lua, "antialias_samples");
    rs->texture_border = lua_GetScalarField(lua, "texture_border");
    rs->z_depth = lua_GetScalarField(lua, "z_depth");
    rs->fog_enabled = lua_GetScalarField(lua, "fog_enabled");
    rs->fog_start_depth = lua_GetScalarField(lua, "fog_start_depth");
    rs->fog_end_depth = lua_GetScalarField(lua, "fog_end_depth");

    lua_getfield(lua, -1, "fog_color");
    if(lua_istable(lua, -1))
    {
        rs->fog_color[0] = (GLfloat)lua_GetScalarField(lua, "r") / 255.0;
        rs->fog_color[1] = (GLfloat)lua_GetScalarField(lua, "g") / 255.0;
        rs->fog_color[2] = (GLfloat)lua_GetScalarField(lua, "b") / 255.0;
    }

    if(rs->z_depth != 8 && rs->z_depth != 16 && rs->z_depth != 24)
    {
        rs->z_depth = 16;
    }

    lua_settop(lua, top);

    return 1;
}

int lua_ParseAudio(lua_State *lua, struct audio_settings_s *as)
{
    if(!lua)
    {
        return -1;
    }

    int top = lua_gettop(lua);
    lua_getglobal(lua, "audio");

    as->music_volume = lua_GetScalarField(lua, "music_volume");
    as->sound_volume = lua_GetScalarField(lua, "sound_volume");
    as->use_effects  = lua_GetScalarField(lua, "use_effects");
    as->listener_is_player = lua_GetScalarField(lua, "listener_is_player");
    as->stream_buffer_size = (lua_GetScalarField(lua, "stream_buffer_size")) * 1024;
    if(as->stream_buffer_size <= 0)
    {
        as->stream_buffer_size = 128 * 1024;
    }
    lua_settop(lua, top);

    return 1;
}

int lua_ParseConsole(lua_State *lua, struct console_info_s *cn)
{
    const char *patch;
    FILE *f;
    int32_t t, i;
    int top;
    float tf;

    if(!lua)
    {
        return -1;
    }

    con_base.inited = 0;

    top = lua_gettop(lua);
    lua_getglobal(lua, "console");
    patch = lua_GetStrField(lua, "font_path");
    if(patch && strncmp(patch, cn->font_patch, 255))
    {
        f = fopen(patch, "rb");
        if(f)
        {
            fclose(f);
            strncpy(cn->font_patch, patch, 255);
            
            delete con_base.font_bitmap;
            con_base.font_bitmap = new FTGLBitmapFont(con_base.font_patch);

            delete con_base.font_texture;
            con_base.font_texture = new FTGLTextureFont(con_base.font_patch);
        }
        else
        {
            Sys_Warn("Console: could not find font = \"%s\"", con_base.font_patch);
        }
    }
    cn->smooth = lua_GetScalarField(lua, "smooth");
    lua_getfield(lua, -1, "font_color");
    if(lua_istable(lua, -1))
    {
        cn->font_color[0] = (GLfloat)lua_GetScalarField(lua, "r") / 255.0;
        cn->font_color[1] = (GLfloat)lua_GetScalarField(lua, "g") / 255.0;
        cn->font_color[2] = (GLfloat)lua_GetScalarField(lua, "b") / 255.0;
        cn->font_color[3] = 1.0;
    }
    lua_pop(lua, 1);

    lua_getfield(lua, -1, "background_color");
    if(lua_istable(lua, -1))
    {
        cn->background_color[0] = (GLfloat)lua_GetScalarField(lua, "r") / 255.0;
        cn->background_color[1] = (GLfloat)lua_GetScalarField(lua, "g") / 255.0;
        cn->background_color[2] = (GLfloat)lua_GetScalarField(lua, "b") / 255.0;
        cn->background_color[3] = (GLfloat)lua_GetScalarField(lua, "a") / 255.0;
    }
    lua_pop(lua, 1);

    t = lua_GetScalarField(lua, "font_size");
    if(t >= 1 && t <= 128)
    {
        cn->font_size = t;
    }
    tf = lua_GetScalarField(lua, "spacing");
    if(tf >= CON_MIN_LINE_INTERVAL && tf <= CON_MAX_LINE_INTERVAL)
    {
        cn->spacing = tf;
    }
    t = lua_GetScalarField(lua, "line_size");
    if(t >= CON_MIN_LINE_SIZE && t <= CON_MAX_LINE_SIZE)
    {
        cn->line_size = t;
    }
    t = lua_GetScalarField(lua, "showing_lines");
    if(t >= CON_MIN_LINES && t <= CON_MAX_LINES)
    {
        cn->showing_lines = t;
    }
    t = lua_GetScalarField(lua, "log_size");
    if(t >= CON_MIN_LOG && t <= CON_MAX_LOG)
    {
        if(t > cn->log_lines_count)
        {
            con_base.log_lines = (char**) realloc(con_base.log_lines, t * sizeof(char*));
            for(i=cn->log_lines_count;i<t;i++)
            {
                con_base.log_lines[i] = (char*) calloc(con_base.line_size * sizeof(char), 1);
            }
        }
        cn->log_lines_count = t;
    }
    t = lua_GetScalarField(lua, "lines_count");
    if(t >= CON_MIN_LOG && t <= CON_MAX_LOG)
    {
        if(t > cn->shown_lines_count)
        {
            con_base.shown_lines = (char**) realloc(con_base.shown_lines, t * sizeof(char*));
            for(i=cn->shown_lines_count;i<t;i++)
            {
                con_base.shown_lines[i] = (char*) calloc(con_base.line_size * sizeof(char), 1);
            }
        }
        cn->shown_lines_count = t;
    }

    cn->show = lua_GetScalarField(lua, "show");
    cn->show_cursor_period = lua_GetScalarField(lua, "show_cursor_period");
    con_base.inited = 1;
    lua_settop(lua, top);

    Con_SetFontSize(con_base.font_size);
    Con_SetLineInterval(con_base.spacing);

    return 1;
}


int SC_ParseEntity(char **ch, struct entity_s *ent)
{
    char token[64];
    int r, x, y, ret = 0;
    btScalar v[3];

    (*ch) = parse_token(*ch, token);
    if(strcmp(token,"{"))                                                       //если не открывающая фигурная скобка, то что-то не так
    {
        Sys_DebugLog(LOG_FILENAME, "Parse error: Expected '{'");
        return -1;
    }

    if(ent == NULL)
    {
        while(strcmp(token,"}")&&(*ch != NULL))                                 //пока не дошли до конца файла, или до закрывающей скобки
        {
            (*ch) = parse_token(*ch, token);
        }
        return 0;
    }

    ent->move_type = MOVE_FREE_FALLING;                                         // дефолтное свойство

    while(strcmp(token, "}")&&(*ch != NULL))                                    //пока не дошли до конца файла, или до закрывающей скобки
    {
        ret++;
        (*ch) = parse_token(*ch, token);
        if(!strcmp(token, "pos"))
        {
            ent->transform[12] = SC_ParseFloat(ch);
            ent->transform[13] = SC_ParseFloat(ch);
            ent->transform[14] = SC_ParseFloat(ch);
        }
        else if(!strcmp(token, "angles"))
        {
            ent->angles[0] = SC_ParseFloat(ch);
            ent->angles[1] = SC_ParseFloat(ch);
            ent->angles[2] = SC_ParseFloat(ch);
        }
        else if(!strcmp(token, "anim"))
        {
            ent->current_animation = SC_ParseInt(ch);
            ent->current_frame = SC_ParseInt(ch);
            Entity_SetAnimation(ent, ent->current_animation, ent->current_frame);
            ent->frame_time = SC_ParseFloat(ch);
        }
        else if(!strcmp(token, "move"))
        {
            ent->move_type = SC_ParseInt(ch);
        }
        else if(!strcmp(token, "speed"))
        {
            v[0] = SC_ParseFloat(ch);
            v[1] = SC_ParseFloat(ch);
            v[2] = SC_ParseFloat(ch);
            vec3_copy(ent->speed.m_floats, v);
        }
        else if(!strcmp(token, "room"))
        {
            r = SC_ParseInt(ch);
            x = SC_ParseInt(ch);
            y = SC_ParseInt(ch);
            if(ent->self->room)
            {
                Room_RemoveEntity(ent->self->room, ent);
            }

            ent->self->room = Room_GetByID(&engine_world, r);
        }
        else
        {
            ret --;
            if(strcmp(token,"{")&&strcmp(token,"}"))
            {
                //Sys_DebugLog(LOG_FILENAME, "\tScript error: Entity have no property: %s", token);
            }
        }
    }

    Entity_UpdateRotation(ent);
    /*if(ent->character)
    {
        btVector3 min, max;
        ent->character->body->getWorldTransform().setFromOpenGLMatrix(ent->transform);
        ent->character->body->getAabb(min, max);
        ent->character->body->getWorldTransform().getOrigin().m_floats[2] += ent->transform[14] - min.m_floats[2];
    }*/

    return ret;
}
