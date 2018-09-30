
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
#include "../core/gl_text.h"
#include "../core/console.h"
#include "../core/vmath.h"
#include "../core/polygon.h"
#include "../state_control/state_control.h"
#include "../skeletal_model.h"
#include "../engine.h"
#include "../entity.h"
#include "../world.h"
#include "../character_controller.h"


int lua_CharacterCreate(lua_State *lua)
{
    int top = lua_gettop(lua);
    if(top >= 1)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent && !ent->character)
        {
            Character_Create(ent);
        }
        else
        {
            Con_Warning("no entity with id = %d, or character already created", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("characterCreate: expecting arguments (entity_id)");
    }

    return 0;
}


int lua_SetCharacterBones(lua_State *lua)
{
    int top = lua_gettop(lua);
    if(top >= 7)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent && ent->character)
        {
            ent->character->bone_head = lua_tointeger(lua, 2);
            ent->character->bone_torso = lua_tointeger(lua, 3);
            ent->character->bone_l_hand_start = lua_tointeger(lua, 4);
            ent->character->bone_l_hand_end = lua_tointeger(lua, 5);
            ent->character->bone_r_hand_start = lua_tointeger(lua, 6);
            ent->character->bone_r_hand_end = lua_tointeger(lua, 7);
        }
        else
        {
            Con_Warning("no character with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("setCharacterBones: expecting arguments (entity_id, head_bone, torso_bone, l_hand_first, l_hand_last, r_hand_first, r_hand_last)");
    }
    return 0;
}


int lua_SetCharacterMoveSizes(lua_State *lua)
{
    int top = lua_gettop(lua);
    if(top >= 6)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent && ent->character)
        {
            if(!lua_isnil(lua, 2))
            {
                ent->character->height = lua_tointeger(lua, 2);
            }
            if(!lua_isnil(lua, 3))
            {
                ent->character->min_step_up_height = lua_tointeger(lua, 3);
            }
            if(!lua_isnil(lua, 4))
            {
                ent->character->max_step_up_height = lua_tointeger(lua, 4);
            }
            if(!lua_isnil(lua, 5))
            {
                ent->character->max_climb_height = lua_tointeger(lua, 5);
            }
            if(!lua_isnil(lua, 6))
            {
                ent->character->fall_down_height = lua_tointeger(lua, 6);
            }
        }
        else
        {
            Con_Warning("no character with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("setCharacterMoveSizes: expecting arguments (entity_id, height, min_step_up_height, max_step_up_height, max_climb_height, fall_down_height)");
    }
    return 0;
}


int lua_SetCharacterStateControlFunctions(lua_State *lua)
{
    int top = lua_gettop(lua);
    if(top >= 2)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent && ent->character)
        {
            StateControl_SetStateFunctions(ent, lua_tointeger(lua, 2));
        }
        else
        {
            Con_Warning("no character with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("setCharacterStateControlFunctions: expecting arguments (entity_id, funcs_id)");
    }
    return 0;
}


int lua_SetCharacterKeyAnim(lua_State *lua)
{
    int top = lua_gettop(lua);
    if(top >= 3)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent && ent->character && ent->character->set_key_anim_func)
        {
            ss_animation_p ss_anim = SSBoneFrame_GetOverrideAnim(ent->bf, lua_tointeger(lua, 2));
            if(ss_anim)
            {
                ent->character->set_key_anim_func(ent, ss_anim, lua_tointeger(lua, 3));
            }
        }
        else
        {
            Con_Warning("no suitable character with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("setCharacterKeyAnim: expecting arguments (entity_id, anim_type, anim_key_id)");
    }
    return 0;
}


int lua_SetCharacterTarget(lua_State *lua)
{
    int top = lua_gettop(lua);
    if(top >= 1)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent && ent->character)
        {
            ent->character->target_id = ((top > 1) && !lua_isnil(lua, 2)) ? (lua_tointeger(lua, 2)) : (ENTITY_ID_NONE);
        }
        else
        {
            Con_Warning("no character with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("setCharacterTarget: expecting arguments (entity_id, (target_id))");
    }
    return 0;
}


int lua_SetCharacterAIParams(lua_State *lua)
{
    int top = lua_gettop(lua);
    if(top >= 3)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent && ent->character)
        {
            ent->character->ai_zone = lua_tointeger(lua, 2);
            ent->character->ai_zone_type = lua_tointeger(lua, 3);
        }
        else
        {
            Con_Warning("no character with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("setCharacterAIParams: expecting arguments (entity_id, zone, zone_type)");
    }
    return 0;
}


int lua_SetCharacterPathTarget(lua_State *lua)
{
    int top = lua_gettop(lua);
    if(top >= 2)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        entity_p target = World_GetEntityByID(lua_tointeger(lua, 2));
        if(ent && ent->character)
        {
            ent->character->path_target = (target) ? (target->self->sector) : (NULL);
            Character_UpdatePath(ent, ent->character->path_target);
        }
    }
    else
    {
        Con_Warning("setCharacterPathTarget: expecting arguments (entity_id, target_id)");
    }
    return 0;
}


int lua_GetCharacterTarget(lua_State *lua)
{
    int top = lua_gettop(lua);
    if(top >= 1)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent && ent->character)
        {
            lua_pushinteger(lua, ent->character->target_id);
        }
        else
        {
            lua_pushnil(lua);
            Con_Warning("no character with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        lua_pushnil(lua);
        Con_Warning("getCharacterTarget: expecting arguments (entity_id)");
    }
    return 1;
}


int lua_GetCharacterParam(lua_State *lua)
{
    if(lua_gettop(lua) >= 2)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent && ent->character)
        {
            int parameter = lua_tointeger(lua, 2);
            if(parameter < PARAM_LASTINDEX)
            {
                lua_pushnumber(lua, Character_GetParam(ent, parameter));
                return 1;
            }
            else
            {
                Con_Warning("wrong option index, expecting id < %d", PARAM_LASTINDEX);
            }
        }
        else
        {
            Con_Warning("no character with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("getCharacterParam: expecting arguments (entity_id, param)");
    }

    return 0;
}


int lua_SetCharacterParam(lua_State *lua)
{
    int top = lua_gettop(lua);
    if(top >= 3)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent && ent->character)
        {
            int parameter = lua_tointeger(lua, 2);
            if(parameter < PARAM_LASTINDEX)
            {
                if(top == 3)
                {
                    Character_SetParam(ent, parameter, lua_tonumber(lua, 3));
                }
                else
                {
                    ent->character->parameters.param[parameter] = lua_tonumber(lua, 3);
                    ent->character->parameters.maximum[parameter] = lua_tonumber(lua, 4);
                }
            }
            else
            {
                Con_Warning("wrong option index, expecting id < %d", PARAM_LASTINDEX);
            }
        }
        else
        {
            Con_Warning("no character with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("setCharacterParam: expecting arguments (entity_id, param, value, (max_value))");
    }

    return 0;
}


int lua_GetCharacterCombatMode(lua_State *lua)
{
    if(lua_gettop(lua) >= 1)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent && ent->character)
        {
            lua_pushnumber(lua, ent->character->state.weapon_ready);
            return 1;
        }
    }

    return 0;
}


int lua_ChangeCharacterParam(lua_State *lua)
{
    if(lua_gettop(lua) >= 3)
    {
        entity_p ent   = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent && ent->character)
        {
            int parameter  = lua_tointeger(lua, 2);
            if(parameter < PARAM_LASTINDEX)
            {
                Character_ChangeParam(ent, parameter, lua_tonumber(lua, 3));
            }
            else
            {
                Con_Warning("wrong option index, expecting id < %d", PARAM_LASTINDEX);
            }
        }
        else
        {
            Con_Warning("no character with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("changeCharacterParam: expecting arguments (entity_id, param_id, delta)");
    }

    return 0;
}


int lua_AddCharacterHair(lua_State *lua)
{
    if(lua_gettop(lua) >= 2)
    {
        entity_p ent   = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent && ent->character)
        {
            hair_setup_s *hair_setup = Hair_GetSetup(lua, 2);
            if(hair_setup)
            {
                ent->character->hair_count++;
                ent->character->hairs = (struct hair_s**)realloc(ent->character->hairs, (sizeof(struct hair_s*) * ent->character->hair_count));
                ent->character->hairs[ent->character->hair_count - 1] = Hair_Create(hair_setup, ent->physics);
                if(!ent->character->hairs[ent->character->hair_count - 1])
                {
                    ent->character->hair_count--;
                    Con_Warning("can not create hair for entity_id = %d", lua_tointeger(lua, 1));
                }
                Hair_DeleteSetup(hair_setup);
            }
            else
            {
                Con_Warning("wrong hair setup index = %d", lua_tointeger(lua, 2));
            }
        }
        else
        {
            Con_Warning("no character with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("addCharacterHair: expecting arguments (entity_id, hair_setup_table)");
    }
    return 0;
}


int lua_ResetCharacterHair(lua_State *lua)
{
    if(lua_gettop(lua) == 1)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent && ent->character)
        {
            if(ent->character->hairs)
            {
                for(int i = 0; i < ent->character->hair_count; i++)
                {
                    Hair_Delete(ent->character->hairs[i]);
                    ent->character->hairs[i] = NULL;
                }
                free(ent->character->hairs);
                ent->character->hairs = NULL;
                ent->character->hair_count = 0;
            }
            else
            {
                Con_Warning("can not create hair for entity_id = %d", lua_tointeger(lua, 1));
            }
        }
        else
        {
            Con_Warning("no character with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("resetCharacterHair: expecting arguments (entity_id)");
    }
    return 0;
}


int lua_SetCharacterRagdollSetup(lua_State *lua)
{
    if(lua_gettop(lua) >= 2)
    {
        entity_p ent   = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent && ent->character)
        {
            struct rd_setup_s *ragdoll_setup = Ragdoll_GetSetup(lua, 2);
            if(ragdoll_setup)
            {
                Ragdoll_DeleteSetup(ent->character->ragdoll);
                ent->character->ragdoll = ragdoll_setup;
            }
            else
            {
                Con_Warning("bad ragdoll setup");
            }
        }
        else
        {
            Con_Warning("no character with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("setCharacterRagdollSetup: expecting arguments (entity_id, ragdoll_setup_table)");
    }
    return 0;
}


int lua_SetCharacterDefaultRagdoll(lua_State *lua)
{
    if(lua_gettop(lua) >= 3)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent && ent->character)
        {
            struct rd_setup_s *ragdoll_setup = Ragdoll_AutoCreateSetup(ent->bf->animations.model, lua_tointeger(lua, 2), lua_tointeger(lua, 3));
            if(ragdoll_setup)
            {
                Ragdoll_DeleteSetup(ent->character->ragdoll);
                ent->character->ragdoll = ragdoll_setup;
            }
            else
            {
                Con_Warning("bad ragdoll setup");
            }
        }
        else
        {
            Con_Warning("no character with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("setCharacterDefaultRagdoll: expecting arguments (entity_id, anim, frame)");
    }
    return 0;
}


int lua_SetCharacterRagdollActivity(lua_State *lua)
{
    if(lua_gettop(lua) >= 2)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent && ent->character)
        {
            if(lua_toboolean(lua, 2))
            {
                if(ent->character->ragdoll && Ragdoll_Create(ent->physics, ent->bf, ent->character->ragdoll))
                {
                    ent->type_flags |=  ENTITY_TYPE_DYNAMIC;
                    ent->character->state.ragdoll = 0x01;
                }
                else
                {
                    Con_Warning("can not create ragdoll for entity_id = %d", lua_tointeger(lua, 1));
                }
            }
            else
            {
                if(Ragdoll_Delete(ent->physics))
                {
                   ent->type_flags &= ~ENTITY_TYPE_DYNAMIC;
                   ent->character->state.ragdoll = 0x00;
                }
                else
                {
                    Con_Warning("can not remove ragdoll from entity_id = %d", lua_tointeger(lua, 1));
                }
            }
        }
        else
        {
            Con_Warning("no character with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("setCharacterRagdollActivity: expecting arguments (entity_id, value)");
    }
    return 0;
}


int lua_SetCharacterState(lua_State *lua)
{
    if(lua_gettop(lua) >= 3)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent && ent->character)
        {
            int8_t value = (int8_t)lua_tointeger(lua, 3);
            switch(lua_tointeger(lua, 2))
            {
                case CHARACTER_STATE_DEAD:
                    ent->character->state.dead = value;
                    break;
                case CHARACTER_STATE_WEAPON:
                    ent->character->state.weapon_ready = value;
                    break;
                default:
                    break;
            }
        }
        else
        {
            Con_Warning("no character with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("setCharacterState: expecting arguments (entity_id, state_id, value)");
    }

    return 0;
}


int lua_GetCharacterState(lua_State *lua)
{
    if(lua_gettop(lua) >= 2)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent && ent->character)
        {
            switch(lua_tointeger(lua, 2))
            {
                case CHARACTER_STATE_DEAD:
                    lua_pushinteger(lua, ent->character->state.dead);
                    break;

                case CHARACTER_STATE_WEAPON:
                    lua_pushinteger(lua, ent->character->state.weapon_ready);
                    break;

                default:
                    lua_pushnil(lua);
                    break;
            }
            return 1;
        }
        else
        {
            Con_Warning("no character with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("getCharacterState: expecting arguments (entity_id, response_id)");
    }

    return 0;
}


int lua_SetCharacterClimbPoint(lua_State *lua)
{
    if(lua_gettop(lua) >= 4)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent && ent->character)
        {
            ent->character->climb.point[0] = lua_tonumber(lua, 2);
            ent->character->climb.point[1] = lua_tonumber(lua, 3);
            ent->character->climb.point[2] = lua_tonumber(lua, 4);
        }
        else
        {
            Con_Warning("no character with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Printf("setCharacterClimbPoint: expecting arguments (entity_id, x, y, z)");
    }

    return 0;
}


int lua_GetCharacterClimbPoint(lua_State *lua)
{
    if(lua_gettop(lua) >= 1)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent && ent->character)
        {
            lua_pushnumber(lua, ent->character->climb.point[0]);
            lua_pushnumber(lua, ent->character->climb.point[1]);
            lua_pushnumber(lua, ent->character->climb.point[2]);
            return 3;
        }
        else
        {
            Con_Warning("no character with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Printf("getCharacterClimbPoint: expecting arguments (entity_id)");
    }

    return 0;
}


int lua_GetCharacterCurrentWeapon(lua_State *lua)
{
    if(lua_gettop(lua) >= 1)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent && ent->character)
        {
            lua_pushinteger(lua, ent->character->weapon_id);
            return 1;
        }
        else
        {
            Con_Warning("no character with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("getCharacterCurrentWeapon: expecting arguments (entity_id)");
    }

    return 0;
}


int lua_SetCharacterCurrentWeapon(lua_State *lua)
{
    int top = lua_gettop(lua);
    if(top >= 2)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent && ent->character)
        {
            ent->character->weapon_id_req = lua_tointeger(lua, 2);
            if(ent->character->set_weapon_model_func && (top >= 3))
            {
                ent->character->weapon_id = lua_tointeger(lua, 3);
                ent->character->set_weapon_model_func(ent, 0, -1); // set special anim handlers
            }
        }
        else
        {
            Con_Warning("no character with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Printf("setCharacterCurrentWeapon: expecting arguments (entity_id, req_weapon_id, (curr_weapon_id))");
    }

    return 0;
}


void Script_LuaRegisterCharacterFuncs(lua_State *lua)
{
    lua_register(lua, "setCharacterRagdollSetup", lua_SetCharacterRagdollSetup);
    lua_register(lua, "setCharacterDefaultRagdoll", lua_SetCharacterDefaultRagdoll);
    lua_register(lua, "setCharacterRagdollActivity", lua_SetCharacterRagdollActivity);

    lua_register(lua, "characterCreate", lua_CharacterCreate);
    lua_register(lua, "setCharacterBones", lua_SetCharacterBones);
    lua_register(lua, "setCharacterMoveSizes", lua_SetCharacterMoveSizes);
    lua_register(lua, "setCharacterStateControlFunctions", lua_SetCharacterStateControlFunctions);
    lua_register(lua, "setCharacterKeyAnim", lua_SetCharacterKeyAnim);
    lua_register(lua, "setCharacterAIParams", lua_SetCharacterAIParams);
    lua_register(lua, "setCharacterTarget", lua_SetCharacterTarget);
    lua_register(lua, "setCharacterPathTarget", lua_SetCharacterPathTarget);
    lua_register(lua, "getCharacterTarget", lua_GetCharacterTarget);
    lua_register(lua, "getCharacterParam", lua_GetCharacterParam);
    lua_register(lua, "setCharacterParam", lua_SetCharacterParam);
    lua_register(lua, "changeCharacterParam", lua_ChangeCharacterParam);

    lua_register(lua, "setCharacterState", lua_SetCharacterState);
    lua_register(lua, "getCharacterState", lua_GetCharacterState);
    lua_register(lua, "setCharacterClimbPoint", lua_SetCharacterClimbPoint);
    lua_register(lua, "getCharacterClimbPoint", lua_GetCharacterClimbPoint);

    lua_register(lua, "getCharacterCurrentWeapon", lua_GetCharacterCurrentWeapon);
    lua_register(lua, "setCharacterCurrentWeapon", lua_SetCharacterCurrentWeapon);
    lua_register(lua, "getCharacterCombatMode", lua_GetCharacterCombatMode);
    lua_register(lua, "addCharacterHair", lua_AddCharacterHair);
    lua_register(lua, "resetCharacterHair", lua_ResetCharacterHair);
}