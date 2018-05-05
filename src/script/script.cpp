
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
#include "../core/gl_text.h"
#include "../vt/tr_versions.h"
#include "../room.h"
#include "../engine.h"
#include "../controls.h"
#include "../game.h"
#include "../gameflow.h"
#include "../skeletal_model.h"
#include "../entity.h"
#include "../trigger.h"
#include "../inventory.h"
#include "../character_controller.h"
#include "../state_control/state_control.h"


#define LUA_EXPOSE(lua, x) do { lua_pushinteger(lua, x); lua_setglobal(lua, #x); } while(false)


/*
 * MISK
 */
char *SC_ParseToken(char *data, char *token, size_t token_size)
{
    int c;
    int len = 0;

    if(!data || token_size < 2)
    {
        return NULL;
    }
    token[0] = 0;

// skip whitespace
    skipwhite:
    while((c = *data) <= ' ')
    {
        if(c == 0)
        {
            return NULL;
        }
        data++;
    }

// skip // comments
    if(data[0] == '/' && data[1] == '/')
    {
        while(*data && *data != '\n')
        {
            data++;
        }
        goto skipwhite;
    }

// handle quoted strings specially
    if(c == '\"')
    {
        data++;
        while(len + 1 < token_size)
        {
            c = *data++;
            if (c == '\"' || !c)
            {
                token[len] = 0;
                return data;
            }
            token[len] = c;
            len++;
        }
    }

// parse single characters
    if(c=='{' || c=='}'|| c==')'|| c=='(' || c=='\'' || c==':')
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
        if(c == '{' || c == '}' || c == ')' || c == '(' || c == '\'' || c == '\"' || c == ':')
        {
            break;
        }
    }
    while((c > ' ') && (len + 1 < token_size));

    token[len] = 0;
    return data;
}

float SC_ParseFloat(char **ch)
{
    char token[64];
    (*ch) = SC_ParseToken(*ch, token, sizeof(token));
    if(token[0])
    {
        return atof(token);
    }
    return 0.0;
}

int SC_ParseInt(char **ch)
{
    char token[64];
    (*ch) = SC_ParseToken(*ch, token, sizeof(token));
    if(token[0])
    {
        return atoi(token);
    }
    return 0;
}


bool Script_GetString(lua_State *lua, int string_index, size_t string_size, char *buffer)
{
    bool result = false;

    if(lua)
    {
        int top = lua_gettop(lua);

        lua_getglobal(lua, "getString");

        if(lua_isfunction(lua, -1))
        {
            size_t *string_length = NULL;

            lua_pushinteger(lua, string_index);
            if (lua_CallAndLog(lua, 1, 1, 0))
            {
                const char* lua_str = lua_tolstring(lua, -1, string_length);
                strncpy(buffer, lua_str, string_size);
                result = true;
            }
        }
        lua_settop(lua, top);
    }

    return result;
}


/*
 * Gameplay functions
 */

int Script_UseItem(lua_State *lua, int item_id, int activator_id)
{
    int top = lua_gettop(lua);
    int ret = -1;

    lua_getglobal(lua, "items_funcs");
    if(!lua_istable(lua, -1))
    {
        lua_settop(lua, top);
        return -1;
    }

    lua_rawgeti(lua, -1, item_id);
    if(!lua_istable(lua, -1))
    {
        lua_settop(lua, top);
        return -1;
    }

    lua_pushstring(lua, "onUse");
    lua_rawget(lua, -2);
    if(!lua_isfunction(lua, -1))
    {
        lua_settop(lua, top);
        return -1;
    }

    lua_pushinteger(lua, activator_id);
    if(lua_pcall(lua, 1, 1, 0) == LUA_OK)
    {
        ret = lua_tointeger(lua, -1);
    }
    lua_settop(lua, top);

    return ret;
}


int Script_DoTasks(lua_State *lua, float time)
{
    lua_pushnumber(lua, time);
    lua_setglobal(lua, "frame_time");

    Script_CallVoidFunc(lua, "doTasks");
    Script_CallVoidFunc(lua, "clearKeys");

    return 0;
}

void Script_AddKey(lua_State *lua, int keycode, int state)
{
    int top = lua_gettop(lua);

    lua_getglobal(lua, "addKey");

    if(!lua_isfunction(lua, -1))
    {
        lua_settop(lua, top);
        return;
    }

    lua_pushinteger(lua, keycode);
    lua_pushinteger(lua, state);
    lua_CallAndLog(lua, 2, 0, 0);

    lua_settop(lua, top);
}

bool Script_CallVoidFunc(lua_State *lua, const char* func_name, bool destroy_after_call)
{
    int top = lua_gettop(lua);

    lua_getglobal(lua, func_name);

    if(!lua_isfunction(lua, -1))
    {
        lua_settop(lua, top);
        return false;
    }

    lua_CallAndLog(lua, 0, 0, 0);

    if(destroy_after_call)
    {
        lua_pushnil(engine_lua);
        lua_setglobal(lua, func_name);
    }

    lua_settop(lua, top);
    return true;
}


bool lua_CallWithError(lua_State *lua, int nargs, int nresults, int errfunc, const char *cfile, int cline)
{
    if(lua_pcall(lua, nargs, nresults, errfunc) != LUA_OK)
    {
        if (lua_gettop(lua) > 0 && lua_isstring(lua, -1))
        {
            const char *luaErrorDescription = lua_tostring(lua, -1);
            Con_Warning("Lua error: %s (called from %s:%d)", luaErrorDescription, cfile, cline);
            lua_pop(lua, 1);
        }
        else
        {
            Con_Warning("Lua error without message (called from %s:%d)", cfile, cline);
        }
        return false;
    }
    return true;
}


/*
 * debug functions
 */
int lua_print(lua_State * lua)
{
    int top = lua_gettop(lua);

    if(top == 0)
    {
       Con_AddLine("nil", FONTSTYLE_CONSOLE_EVENT);
    }

    for(int i = 1; i <= top; i++)
    {
        switch(lua_type(lua, i))
        {
            case LUA_TNUMBER:
            case LUA_TSTRING:
               Con_AddLine(lua_tostring(lua, i), FONTSTYLE_CONSOLE_EVENT);
               break;

            case LUA_TBOOLEAN:
               Con_AddLine(lua_toboolean(lua, i) ? ("true") : ("false"), FONTSTYLE_CONSOLE_EVENT);
               break;

            case LUA_TFUNCTION:
                Con_AddLine("function", FONTSTYLE_CONSOLE_EVENT);
                break;

            case LUA_TTABLE:
                Con_AddLine("table", FONTSTYLE_CONSOLE_EVENT);
                break;

            case LUA_TTHREAD:
                Con_AddLine("thread", FONTSTYLE_CONSOLE_EVENT);
                break;

            case LUA_TLIGHTUSERDATA:
                Con_AddLine("light user data", FONTSTYLE_CONSOLE_EVENT);
                break;

            case LUA_TNIL:
                Con_AddLine("nil", FONTSTYLE_CONSOLE_EVENT);
                break;

            case LUA_TNONE:
                Con_AddLine("none", FONTSTYLE_CONSOLE_EVENT);
                break;

            default:
                Con_AddLine("none or nil", FONTSTYLE_CONSOLE_EVENT);
                break;
        };
    }

    return 0;
}


int lua_ls(lua_State *lua)
{
    int top = lua_gettop(lua);
    char path[1024] = { 0 };
    const char *wild = NULL;

    strncpy(path, Engine_GetBasePath(), sizeof(path));
    if((top > 0) && lua_isstring(lua, 1))
    {
        strncat(path, lua_tostring(lua, 1), sizeof(path) - strlen(path) - 1);
    }

    if((top > 1) && lua_isstring(lua, 2))
    {
        wild = lua_tostring(lua, 2);
    }

    file_info_p fi = Sys_ListDir(path, wild);
    if(fi)
    {
        for(file_info_p i = fi; i; i = i->next)
        {
            Con_Printf((i->is_dir) ? ("[%s]") : ("%s"), i->name);
        }
        Sys_ListDirFree(fi);
    }
    return 0;
}


int lua_BindKey(lua_State *lua)
{
    int top = lua_gettop(lua);
    int act = lua_tointeger(lua, 1);

    if(top < 1 || act < 0 || act >= ACT_LASTINDEX)
    {
        Con_Warning("wrong action number");
    }

    else if(top == 2)
    {
        control_states.actions[act].primary = lua_tointeger(lua, 2);
    }
    else if(top == 3)
    {
        control_states.actions[act].primary   = lua_tointeger(lua, 2);
        control_states.actions[act].secondary = lua_tointeger(lua, 3);
    }
    else
    {
        Con_Warning("bindKey: expecting arguments (action_id, key_id1, (key_id2))");
    }

    return 0;
}


 int lua_GetActionState(lua_State *lua)
{
    if(lua_gettop(lua) >= 1)
    {
        int act = lua_tointeger(lua, 1);

        if((act >= 0) && (act < ACT_LASTINDEX))
        {
            lua_pushinteger(lua, (int)(control_states.actions[act].state));
            return 1;
        }

        Con_Warning("wrong action number");
        return 0;
    }

    Con_Warning("getActionState: expecting arguments (action_id)");
    return 0;
}


int lua_GetActionChange(lua_State *lua)
{
    if(lua_gettop(lua) >= 1)
    {
        int index = lua_tointeger(lua, 1);
        if((index >= 0) && (index < ACT_LASTINDEX))
        {
            control_action_p act = control_states.actions + index;
            lua_pushinteger(lua, (int)(act->state != act->prev_state));
            return 1;
        }

        Con_Warning("wrong action number");
        return 0;
    }

    Con_Warning("getActionChange: expecting arguments (action_id)");
    return 0;
}


int lua_AddFont(lua_State *lua)
{
    if(lua_gettop(lua) != 3)
    {
        Con_Warning("addFont: expecting arguments (font_index, font_path, font_size)");
        return 0;
    }

    if(!GLText_AddFont(lua_tointeger(lua, 1), lua_tointeger(lua, 3), lua_tostring(lua, 2)))
    {
        Con_Warning("can't create font with id = %d", lua_tointeger(lua, 1));
    }

    return 0;
}


int lua_AddFontStyle(lua_State *lua)
{
    if(lua_gettop(lua) < 6)
    {
        Con_Warning("addFontStyle: expecting arguments (index, R, G, B, A, shadow)");
        return 0;
    }

    font_Style  style_index = (font_Style)lua_tointeger(lua, 1);
    GLfloat     color_R     = (GLfloat)lua_tonumber(lua, 2);
    GLfloat     color_G     = (GLfloat)lua_tonumber(lua, 3);
    GLfloat     color_B     = (GLfloat)lua_tonumber(lua, 4);
    GLfloat     color_A     = (GLfloat)lua_tonumber(lua, 5);
    int         shadowed    = lua_toboolean(lua, 6);

    if(!GLText_AddFontStyle(style_index,
                         color_R, color_G, color_B, color_A,
                         shadowed))
    {
        Con_Warning("can't create fontstyle with id = %d", style_index);
    }

    return 0;
}


int lua_RemoveFont(lua_State *lua)
{
    if(lua_gettop(lua) != 1)
    {
        Con_Warning("removeFont: expecting arguments (font_index)");
        return 0;
    }

    if(!GLText_RemoveFont(lua_tointeger(lua, 1)))
    {
        Con_Warning("can't remove font");
    }

    return 0;
}


int lua_RemoveFontStyle(lua_State *lua)
{
    if(lua_gettop(lua) != 1)
    {
        Con_Warning("removeFontStyle: expecting arguments (style_index)");
        return 0;
    }

    if(!GLText_RemoveFontStyle(lua_tointeger(lua, 1)))
    {
        Con_Warning("can't remove font style");
    }

    return 0;
}


// Called when something goes absolutely horribly wrong in Lua, and tries
// to produce some debug output. Lua calls abort afterwards, so sending
// the output to the internal console is not an option.
static int lua_LuaPanic(lua_State *lua)
{
    if(lua_gettop(lua) < 1)
    {
        fprintf(stderr, "Fatal lua error (no details provided).\n");
    }
    else
    {
        fprintf(stderr, "Fatal lua error: %s\n", lua_tostring(lua, 1));
    }
    fflush(stderr);
    return 0;
}


void Script_LoadConstants(lua_State *lua)
{
    if(lua)
    {
        int top = lua_gettop(lua);

        LUA_EXPOSE(lua, TR_I);
        LUA_EXPOSE(lua, TR_I_DEMO);
        LUA_EXPOSE(lua, TR_I_UB);
        LUA_EXPOSE(lua, TR_II);
        LUA_EXPOSE(lua, TR_II_DEMO);
        LUA_EXPOSE(lua, TR_III);
        LUA_EXPOSE(lua, TR_IV);
        LUA_EXPOSE(lua, TR_IV_DEMO);
        LUA_EXPOSE(lua, TR_V);
        LUA_EXPOSE(lua, TR_UNKNOWN);

        LUA_EXPOSE(lua, GAME_1);
        LUA_EXPOSE(lua, GAME_1_1);
        LUA_EXPOSE(lua, GAME_1_5);
        LUA_EXPOSE(lua, GAME_2);
        LUA_EXPOSE(lua, GAME_2_1);
        LUA_EXPOSE(lua, GAME_2_5);
        LUA_EXPOSE(lua, GAME_3);
        LUA_EXPOSE(lua, GAME_3_5);
        LUA_EXPOSE(lua, GAME_4);
        LUA_EXPOSE(lua, GAME_4_1);
        LUA_EXPOSE(lua, GAME_5);

        LUA_EXPOSE(lua, GF_OP_PICTURE);
        LUA_EXPOSE(lua, GF_OP_LISTSTART);
        LUA_EXPOSE(lua, GF_OP_LISTEND);
        LUA_EXPOSE(lua, GF_OP_STARTFMV);
        LUA_EXPOSE(lua, GF_OP_STARTLEVEL);
        LUA_EXPOSE(lua, GF_OP_STARTCINE);
        LUA_EXPOSE(lua, GF_OP_LEVELCOMPLETE);
        LUA_EXPOSE(lua, GF_OP_STARTDEMO);
        LUA_EXPOSE(lua, GF_OP_JUMPTOSEQUENCE);
        LUA_EXPOSE(lua, GF_OP_ENDSEQUENCE);
        LUA_EXPOSE(lua, GF_OP_SETTRACK);
        LUA_EXPOSE(lua, GF_OP_ENABLESUNSET);
        LUA_EXPOSE(lua, GF_OP_LOADINGPIC);
        LUA_EXPOSE(lua, GF_OP_DEADLYWATER);
        LUA_EXPOSE(lua, GF_OP_REMOVEWEAPONS);
        LUA_EXPOSE(lua, GF_OP_GAMECOMPLETE);
        LUA_EXPOSE(lua, GF_OP_CUTANGLE);
        LUA_EXPOSE(lua, GF_OP_NOFLOOR);
        LUA_EXPOSE(lua, GF_OP_ADDTOINVENTORY);
        LUA_EXPOSE(lua, GF_OP_LARASTARTANIM);
        LUA_EXPOSE(lua, GF_OP_NUMSECRETS);
        LUA_EXPOSE(lua, GF_OP_KILLTOCOMPLETE);
        LUA_EXPOSE(lua, GF_OP_REMOVEAMMO);

        LUA_EXPOSE(lua, ITEM_TYPE_SYSTEM);
        LUA_EXPOSE(lua, ITEM_TYPE_SUPPLY);
        LUA_EXPOSE(lua, ITEM_TYPE_QUEST);

        LUA_EXPOSE(lua, ITEM_COMPASS);
        LUA_EXPOSE(lua, ITEM_PASSPORT);
        LUA_EXPOSE(lua, ITEM_LARAHOME);
        LUA_EXPOSE(lua, ITEM_VIDEO);
        LUA_EXPOSE(lua, ITEM_AUDIO);
        LUA_EXPOSE(lua, ITEM_CONTROLS);
        LUA_EXPOSE(lua, ITEM_LOAD);
        LUA_EXPOSE(lua, ITEM_SAVE);
        LUA_EXPOSE(lua, ITEM_MAP);

        LUA_EXPOSE(lua, TR_FD_TRIGTYPE_TRIGGER);
        LUA_EXPOSE(lua, TR_FD_TRIGTYPE_PAD);
        LUA_EXPOSE(lua, TR_FD_TRIGTYPE_SWITCH);
        LUA_EXPOSE(lua, TR_FD_TRIGTYPE_KEY);
        LUA_EXPOSE(lua, TR_FD_TRIGTYPE_PICKUP);
        LUA_EXPOSE(lua, TR_FD_TRIGTYPE_HEAVY);
        LUA_EXPOSE(lua, TR_FD_TRIGTYPE_ANTIPAD);
        LUA_EXPOSE(lua, TR_FD_TRIGTYPE_COMBAT);
        LUA_EXPOSE(lua, TR_FD_TRIGTYPE_DUMMY);
        LUA_EXPOSE(lua, TR_FD_TRIGTYPE_ANTITRIGGER);
        LUA_EXPOSE(lua, TR_FD_TRIGTYPE_HEAVYSWITCH);
        LUA_EXPOSE(lua, TR_FD_TRIGTYPE_HEAVYANTITRIGGER);
        LUA_EXPOSE(lua, TR_FD_TRIGTYPE_MONKEY);
        LUA_EXPOSE(lua, TR_FD_TRIGTYPE_SKELETON);
        LUA_EXPOSE(lua, TR_FD_TRIGTYPE_TIGHTROPE);
        LUA_EXPOSE(lua, TR_FD_TRIGTYPE_CRAWLDUCK);
        LUA_EXPOSE(lua, TR_FD_TRIGTYPE_CLIMB);

        LUA_EXPOSE(lua, TR_FD_TRIGFUNC_OBJECT);
        LUA_EXPOSE(lua, TR_FD_TRIGFUNC_SET_CAMERA);
        LUA_EXPOSE(lua, TR_FD_TRIGFUNC_UWCURRENT);
        LUA_EXPOSE(lua, TR_FD_TRIGFUNC_FLIPMAP);
        LUA_EXPOSE(lua, TR_FD_TRIGFUNC_FLIPON);
        LUA_EXPOSE(lua, TR_FD_TRIGFUNC_FLIPOFF);
        LUA_EXPOSE(lua, TR_FD_TRIGFUNC_SET_TARGET);
        LUA_EXPOSE(lua, TR_FD_TRIGFUNC_ENDLEVEL);
        LUA_EXPOSE(lua, TR_FD_TRIGFUNC_PLAYTRACK);
        LUA_EXPOSE(lua, TR_FD_TRIGFUNC_FLIPEFFECT);
        LUA_EXPOSE(lua, TR_FD_TRIGFUNC_SECRET);
        LUA_EXPOSE(lua, TR_FD_TRIGFUNC_CLEARBODIES);
        LUA_EXPOSE(lua, TR_FD_TRIGFUNC_FLYBY);
        LUA_EXPOSE(lua, TR_FD_TRIGFUNC_CUTSCENE);

        LUA_EXPOSE(lua, ENTITY_STATE_ENABLED);
        LUA_EXPOSE(lua, ENTITY_STATE_ACTIVE);
        LUA_EXPOSE(lua, ENTITY_STATE_VISIBLE);
        LUA_EXPOSE(lua, ENTITY_STATE_COLLIDABLE);
        LUA_EXPOSE(lua, ENTITY_STATE_DELETED);

        LUA_EXPOSE(lua, ENTITY_TYPE_SPAWNED);
        LUA_EXPOSE(lua, ENTITY_TYPE_GENERIC);
        LUA_EXPOSE(lua, ENTITY_TYPE_INTERACTIVE);
        LUA_EXPOSE(lua, ENTITY_TYPE_TRIGGER_ACTIVATOR);
        LUA_EXPOSE(lua, ENTITY_TYPE_HEAVYTRIGGER_ACTIVATOR);
        LUA_EXPOSE(lua, ENTITY_TYPE_PICKABLE);
        LUA_EXPOSE(lua, ENTITY_TYPE_TRAVERSE);
        LUA_EXPOSE(lua, ENTITY_TYPE_TRAVERSE_FLOOR);
        LUA_EXPOSE(lua, ENTITY_TYPE_DYNAMIC);
        LUA_EXPOSE(lua, ENTITY_TYPE_ACTOR);

        LUA_EXPOSE(lua, ENTITY_CALLBACK_NONE);
        LUA_EXPOSE(lua, ENTITY_CALLBACK_ACTIVATE);
        LUA_EXPOSE(lua, ENTITY_CALLBACK_DEACTIVATE);
        LUA_EXPOSE(lua, ENTITY_CALLBACK_COLLISION);
        LUA_EXPOSE(lua, ENTITY_CALLBACK_STAND);
        LUA_EXPOSE(lua, ENTITY_CALLBACK_HIT);
        LUA_EXPOSE(lua, ENTITY_CALLBACK_ATTACK);
        LUA_EXPOSE(lua, ENTITY_CALLBACK_SHOOT);

        LUA_EXPOSE(lua, PARAM_HEALTH);
        LUA_EXPOSE(lua, PARAM_AIR);
        LUA_EXPOSE(lua, PARAM_STAMINA);
        LUA_EXPOSE(lua, PARAM_WARMTH);
        LUA_EXPOSE(lua, PARAM_HIT_DAMAGE);
        LUA_EXPOSE(lua, PARAM_EXTRA1);
        LUA_EXPOSE(lua, PARAM_EXTRA2);
        LUA_EXPOSE(lua, PARAM_EXTRA3);
        LUA_EXPOSE(lua, PARAM_EXTRA4);
        LUA_EXPOSE(lua, PARAM_LASTINDEX);

        LUA_EXPOSE(lua, ANIMATION_KEY_INIT);
        LUA_EXPOSE(lua, ANIMATION_KEY_DEAD);

        LUA_EXPOSE(lua, STATE_FUNCTIONS_LARA);
        LUA_EXPOSE(lua, STATE_FUNCTIONS_BAT);
        LUA_EXPOSE(lua, STATE_FUNCTIONS_WOLF);
        LUA_EXPOSE(lua, STATE_FUNCTIONS_BEAR);
        LUA_EXPOSE(lua, STATE_FUNCTIONS_RAPTOR);
        LUA_EXPOSE(lua, STATE_FUNCTIONS_TREX);
        LUA_EXPOSE(lua, STATE_FUNCTIONS_LARSON);
        LUA_EXPOSE(lua, STATE_FUNCTIONS_PIERRE);
        LUA_EXPOSE(lua, STATE_FUNCTIONS_LION);
        LUA_EXPOSE(lua, STATE_FUNCTIONS_GORILLA);
        LUA_EXPOSE(lua, STATE_FUNCTIONS_CROCODILE);
        LUA_EXPOSE(lua, STATE_FUNCTIONS_RAT);
        LUA_EXPOSE(lua, STATE_FUNCTIONS_CENTAUR);
        LUA_EXPOSE(lua, STATE_FUNCTIONS_PUMA);
        LUA_EXPOSE(lua, STATE_FUNCTIONS_WINGED_MUTANT);
        LUA_EXPOSE(lua, STATE_FUNCTIONS_COWBOY);
        LUA_EXPOSE(lua, STATE_FUNCTIONS_SKATEBOARDIST);
        LUA_EXPOSE(lua, STATE_FUNCTIONS_MRT);
        LUA_EXPOSE(lua, STATE_FUNCTIONS_TORSO_BOSS);
        LUA_EXPOSE(lua, STATE_FUNCTIONS_NATLA);

        LUA_EXPOSE(lua, ANIM_NORMAL_CONTROL);
        LUA_EXPOSE(lua, ANIM_LOOP_LAST_FRAME);
        LUA_EXPOSE(lua, ANIM_FRAME_LOCK);
        LUA_EXPOSE(lua, ANIM_FRAME_REVERSE);

        LUA_EXPOSE(lua, ANIM_TYPE_BASE);
        LUA_EXPOSE(lua, ANIM_TYPE_WEAPON_LH);
        LUA_EXPOSE(lua, ANIM_TYPE_WEAPON_RH);
        LUA_EXPOSE(lua, ANIM_TYPE_WEAPON_TH);
        LUA_EXPOSE(lua, ANIM_TYPE_MISK_1);
        LUA_EXPOSE(lua, ANIM_TYPE_MISK_2);
        LUA_EXPOSE(lua, ANIM_TYPE_MISK_3);
        LUA_EXPOSE(lua, ANIM_TYPE_MISK_4);

        LUA_EXPOSE(lua, BODY_PART_BODY_LOW);
        LUA_EXPOSE(lua, BODY_PART_BODY_UPPER);
        LUA_EXPOSE(lua, BODY_PART_HEAD);

        LUA_EXPOSE(lua, BODY_PART_LEFT_HAND_1);
        LUA_EXPOSE(lua, BODY_PART_LEFT_HAND_2);
        LUA_EXPOSE(lua, BODY_PART_LEFT_HAND_3);
        LUA_EXPOSE(lua, BODY_PART_RIGHT_HAND_1);
        LUA_EXPOSE(lua, BODY_PART_RIGHT_HAND_2);
        LUA_EXPOSE(lua, BODY_PART_RIGHT_HAND_3);

        LUA_EXPOSE(lua, BODY_PART_LEFT_LEG_1);
        LUA_EXPOSE(lua, BODY_PART_LEFT_LEG_2);
        LUA_EXPOSE(lua, BODY_PART_LEFT_LEG_3);
        LUA_EXPOSE(lua, BODY_PART_RIGHT_LEG_1);
        LUA_EXPOSE(lua, BODY_PART_RIGHT_LEG_2);
        LUA_EXPOSE(lua, BODY_PART_RIGHT_LEG_3);
        LUA_EXPOSE(lua, BODY_PART_TAIL);

        LUA_EXPOSE(lua, ZONE_TYPE_ALL);
        LUA_EXPOSE(lua, ZONE_TYPE_1);
        LUA_EXPOSE(lua, ZONE_TYPE_2);
        LUA_EXPOSE(lua, ZONE_TYPE_3);
        LUA_EXPOSE(lua, ZONE_TYPE_4);
        LUA_EXPOSE(lua, ZONE_TYPE_FLY);

        LUA_EXPOSE(lua, MOVE_STATIC_POS);
        LUA_EXPOSE(lua, MOVE_KINEMATIC);
        LUA_EXPOSE(lua, MOVE_ON_FLOOR);
        LUA_EXPOSE(lua, MOVE_WADE);
        LUA_EXPOSE(lua, MOVE_QUICKSAND);
        LUA_EXPOSE(lua, MOVE_ON_WATER);
        LUA_EXPOSE(lua, MOVE_UNDERWATER);
        LUA_EXPOSE(lua, MOVE_FREE_FALLING);
        LUA_EXPOSE(lua, MOVE_CLIMBING);
        LUA_EXPOSE(lua, MOVE_MONKEYSWING);
        LUA_EXPOSE(lua, MOVE_WALLS_CLIMB);
        LUA_EXPOSE(lua, MOVE_DOZY);
        LUA_EXPOSE(lua, MOVE_FLY);

        LUA_EXPOSE(lua, TRIGGER_OP_OR);
        LUA_EXPOSE(lua, TRIGGER_OP_XOR);
        LUA_EXPOSE(lua, TRIGGER_OP_AND_INV);

        LUA_EXPOSE(lua, CHARACTER_STATE_DEAD);
        LUA_EXPOSE(lua, CHARACTER_STATE_WEAPON);

        LUA_EXPOSE(lua, TICK_IDLE);
        LUA_EXPOSE(lua, TICK_STOPPED);
        LUA_EXPOSE(lua, TICK_ACTIVE);

        LUA_EXPOSE(lua, COLLISION_SHAPE_BOX);
        LUA_EXPOSE(lua, COLLISION_SHAPE_BOX_BASE);
        LUA_EXPOSE(lua, COLLISION_SHAPE_SPHERE);
        LUA_EXPOSE(lua, COLLISION_SHAPE_TRIMESH);
        LUA_EXPOSE(lua, COLLISION_SHAPE_TRIMESH_CONVEX);
        LUA_EXPOSE(lua, COLLISION_SHAPE_SINGLE_BOX);
        LUA_EXPOSE(lua, COLLISION_SHAPE_SINGLE_SPHERE);

        LUA_EXPOSE(lua, COLLISION_NONE);
        LUA_EXPOSE(lua, COLLISION_GROUP_ALL);
        LUA_EXPOSE(lua, COLLISION_MASK_ALL);
        LUA_EXPOSE(lua, COLLISION_GROUP_STATIC_ROOM);
        LUA_EXPOSE(lua, COLLISION_GROUP_STATIC_OBLECT);
        LUA_EXPOSE(lua, COLLISION_GROUP_KINEMATIC);
        //LUA_EXPOSE(lua, COLLISION_GROUP_GHOST);
        LUA_EXPOSE(lua, COLLISION_GROUP_TRIGGERS);
        LUA_EXPOSE(lua, COLLISION_GROUP_CHARACTERS);
        LUA_EXPOSE(lua, COLLISION_GROUP_VEHICLE);
        LUA_EXPOSE(lua, COLLISION_GROUP_BULLETS);
        LUA_EXPOSE(lua, COLLISION_GROUP_DYNAMICS);
        LUA_EXPOSE(lua, COLLISION_GROUP_DYNAMICS_NI);

        LUA_EXPOSE(lua, ENTITY_TRIGGERING_ACTIVATED);
        LUA_EXPOSE(lua, ENTITY_TRIGGERING_DEACTIVATED);
        LUA_EXPOSE(lua, ENTITY_TRIGGERING_NOT_READY);

        lua_pushstring(lua, Engine_GetBasePath());
        lua_setglobal(lua, "base_path");

        lua_settop(lua, top);
    }
}


bool Script_LuaInit()
{
    bool ret = false;
    engine_lua = luaL_newstate();

    if(engine_lua)
    {
        luaL_openlibs(engine_lua);
        Script_LoadConstants(engine_lua);
        Script_LuaRegisterFuncs(engine_lua);
        lua_atpanic(engine_lua, lua_LuaPanic);

        // Load script loading order (sic!)
        Script_DoLuaFile(engine_lua, "scripts/loadscript.lua");
        ret = true;
    }

    return ret;
}


int Script_DoLuaFile(lua_State *lua, const char *local_path)
{
    char script_path[1024];
    size_t script_path_base_len = sizeof(script_path) - 1;
    strncpy(script_path, Engine_GetBasePath(), script_path_base_len);
    script_path[script_path_base_len] = 0;
    strncat(script_path, local_path, script_path_base_len - 1);
    return luaL_dofile(lua, script_path);
}


void Script_LuaClearTasks()
{
    if(engine_lua)
    {
        int top = lua_gettop(engine_lua);

        Script_CallVoidFunc(engine_lua, "clearTasks");

        lua_getglobal(engine_lua, "tlist_Clear");
        if(lua_isfunction(engine_lua, -1))
        {
            lua_CallAndLog(engine_lua, 0, 1, 0);
        }

        lua_getglobal(engine_lua, "entfuncs_Clear");
        if(lua_isfunction(engine_lua, -1))
        {
            lua_CallAndLog(engine_lua, 0, 1, 0);
        }

        lua_settop(engine_lua, top);
    }
}


void Script_LuaRegisterAnimFuncs(lua_State *lua);
void Script_LuaRegisterEntityFuncs(lua_State *lua);
void Script_LuaRegisterCharacterFuncs(lua_State *lua);
void Script_LuaRegisterWorldFuncs(lua_State *lua);
void Script_LuaRegisterAudioFuncs(lua_State *lua);


void Script_LuaRegisterFuncs(lua_State *lua)
{
    luaL_dostring(lua, CVAR_LUA_TABLE_NAME " = {};");

    lua_register(lua, "print", lua_print);
    lua_register(lua, "ls", lua_ls);

    lua_register(lua, "getActionState", lua_GetActionState);
    lua_register(lua, "getActionChange", lua_GetActionChange);

    lua_register(lua, "bind", lua_BindKey);
    lua_register(lua, "addFont", lua_AddFont);
    lua_register(lua, "removeFont", lua_RemoveFont);
    lua_register(lua, "addFontStyle", lua_AddFontStyle);
    lua_register(lua, "removeFontStyle", lua_RemoveFontStyle);

    Game_RegisterLuaFunctions(lua);

    Script_LuaRegisterConfigFuncs(lua);
    Script_LuaRegisterAnimFuncs(lua);
    Script_LuaRegisterEntityFuncs(lua);
    Script_LuaRegisterCharacterFuncs(lua);
    Script_LuaRegisterWorldFuncs(lua);
    Script_LuaRegisterAudioFuncs(lua);
}
