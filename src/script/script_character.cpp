
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
#include "../inventory.h"
#include "../entity.h"
#include "../world.h"
#include "../character_controller.h"
#include "../gui.h"


int lua_SetCharacterTarget(lua_State * lua)
{
    int top = lua_gettop(lua);

    if(top == 0)
    {
        Con_Warning("setCharacterTarget: expecting arguments (entity_id, (target_id))");
        return 0;
    }

    uint32_t character_id = lua_tointeger(lua, 1);
    entity_p ent     = World_GetEntityByID(character_id);

    if(!ent || !ent->character)
    {
        Con_Warning("no character with id = %d", character_id);
        return 0;
    }

    ent->character->target_id = (top > 1) ? (lua_tointeger(lua, 2)) : (ENTITY_ID_NONE);

    return 0;
}


int lua_GetCharacterParam(lua_State * lua)
{
    if(lua_gettop(lua) < 2)
    {
        Con_Warning("getCharacterParam: expecting arguments (entity_id, param)");
        return 0;
    }

    int id         = lua_tointeger(lua, 1);
    int parameter  = lua_tointeger(lua, 2);
    entity_p ent   = World_GetEntityByID(id);

    if(parameter >= PARAM_LASTINDEX)
    {
        Con_Warning("wrong option index, expecting id < %d", PARAM_LASTINDEX);
        return 0;
    }

    if(ent && ent->character)
    {
        lua_pushnumber(lua, Character_GetParam(ent, parameter));
        return 1;
    }
    else
    {
        Con_Warning("no character with id = %d", id);
        return 0;
    }
}


int lua_SetCharacterParam(lua_State * lua)
{
    int top = lua_gettop(lua);

    if(top < 3)
    {
        Con_Warning("setCharacterParam: expecting arguments (entity_id, param, value, (max_value))");
        return 0;
    }

    int id           = lua_tointeger(lua, 1);
    int parameter    = lua_tointeger(lua, 2);
    entity_p ent     = World_GetEntityByID(id);

    if(parameter >= PARAM_LASTINDEX)
    {
        Con_Warning("wrong option index, expecting id < %d", PARAM_LASTINDEX);
        return 0;
    }

    if(!ent || !ent->character)
    {
        Con_Warning("no character with id = %d", id);
        return 0;
    }
    else if(top == 3)
    {
        Character_SetParam(ent, parameter, lua_tonumber(lua, 3));
    }
    else
    {
        ent->character->parameters.param[parameter] = lua_tonumber(lua, 3);
        ent->character->parameters.maximum[parameter] = lua_tonumber(lua, 4);
    }

    return 0;
}


int lua_GetCharacterCombatMode(lua_State * lua)
{
    if(lua_gettop(lua) >= 1)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent && ent->character)
        {
            lua_pushnumber(lua, ent->character->weapon_current_state);
            return 1;
        }
    }

    return 0;
}


int lua_ChangeCharacterParam(lua_State * lua)
{
    if(lua_gettop(lua) < 3)
    {
        Con_Warning("changeCharacterParam: expecting arguments (entity_id, param, value)");
        return 0;
    }

    int id         = lua_tointeger(lua, 1);
    int parameter  = lua_tointeger(lua, 2);
    int value      = lua_tonumber(lua, 3);
    entity_p ent   = World_GetEntityByID(id);

    if(parameter >= PARAM_LASTINDEX)
    {
        Con_Warning("wrong option index, expecting id < %d", PARAM_LASTINDEX);
        return 0;
    }

    if(ent && ent->character)
    {
        Character_ChangeParam(ent, parameter, value);
    }
    else
    {
        Con_Warning("no character with id = %d", id);
    }

    return 0;
}


int lua_AddCharacterHair(lua_State *lua)
{
    if(lua_gettop(lua) != 2)
    {
        Con_Warning("addCharacterHair: expecting arguments (entity_id, hair_setup_index)");
    }
    else
    {
        int ent_id       = lua_tointeger(lua, 1);
        int setup_index  = lua_tointeger(lua, 2);

        entity_p ent   = World_GetEntityByID(ent_id);

        if(ent && ent->character)
        {
            hair_setup_s *hair_setup = Hair_GetSetup(lua, setup_index);

            if(hair_setup)
            {
                ent->character->hair_count++;
                ent->character->hairs = (struct hair_s**)realloc(ent->character->hairs, (sizeof(struct hair_s*) * ent->character->hair_count));
                ent->character->hairs[ent->character->hair_count-1] = Hair_Create(hair_setup, ent->physics);
                if(!ent->character->hairs[ent->character->hair_count - 1])
                {
                    ent->character->hair_count--;
                    Con_Warning("can not create hair for entity_id = %d", ent_id);
                }
            }
            else
            {
                Con_Warning("wrong hair setup index = %d", setup_index);
            }
            free(hair_setup);
        }
        else
        {
            Con_Warning("no character with id = %d", ent_id);
        }
    }
    return 0;
}


int lua_ResetCharacterHair(lua_State *lua)
{
    if(lua_gettop(lua) == 1)
    {
        int ent_id   = lua_tointeger(lua, 1);
        entity_p ent = World_GetEntityByID(ent_id);

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
                Con_Warning("can not create hair for entity_id = %d", ent_id);
            }
        }
        else
        {
            Con_Warning("no character with id = %d", ent_id);
        }
    }
    else
    {
        Con_Warning("resetCharacterHair: expecting arguments (entity_id)");
    }
    return 0;
}


int lua_AddEntityRagdoll(lua_State *lua)
{
    if(lua_gettop(lua) >= 2)
    {
        int ent_id       = lua_tointeger(lua, 1);
        int setup_index  = lua_tointeger(lua, 2);

        entity_p ent   = World_GetEntityByID(ent_id);

        if(ent)
        {
            struct rd_setup_s *ragdoll_setup = Ragdoll_GetSetup(lua, setup_index);
            if(ragdoll_setup)
            {
                if(!Ragdoll_Create(ent->physics, ent->bf, ragdoll_setup))
                {
                    Con_Warning("can not create ragdoll for entity_id = %d", ent_id);
                }
                ent->type_flags |=  ENTITY_TYPE_DYNAMIC;
                Ragdoll_DeleteSetup(ragdoll_setup);
            }
            else
            {
                Con_Warning("no ragdoll setup with id = %d", setup_index);
            }
        }
        else
        {
            Con_Warning("no entity with id = %d", ent_id);
        }
    }
    else
    {
        Con_Warning("addEntityRagdoll: expecting arguments (entity_id, ragdoll_setup_index)");
    }
    return 0;
}


int lua_RemoveEntityRagdoll(lua_State *lua)
{
    if(lua_gettop(lua) >= 1)
    {
        int ent_id   = lua_tointeger(lua, 1);
        entity_p ent = World_GetEntityByID(ent_id);

        if(ent)
        {
            if(!Ragdoll_Delete(ent->physics))
            {
                Con_Warning("can not remove ragdoll for entity_id = %d", ent_id);
            }
            ent->type_flags &= ~ENTITY_TYPE_DYNAMIC;
        }
        else
        {
            Con_Warning("no entity with id = %d", ent_id);
        }
    }
    else
    {
        Con_Warning("removeEntityRagdoll: expecting arguments (entity_id)");
    }
    return 0;
}


int lua_AddItem(lua_State * lua)
{
    int top, count;
    top = lua_gettop(lua);

    if(top < 2)
    {
        Con_Warning("addItem: expecting arguments (entity_id, item_id, items_count)");
        return 0;
    }

    int entity_id = lua_tointeger(lua, 1);
    int item_id = lua_tointeger(lua, 2);
    count = (top >= 3) ? (lua_tointeger(lua, 3)) : (-1);
    entity_p ent = World_GetEntityByID(entity_id);

    if(ent && ent->character)
    {
        entity_p player = World_GetPlayer();
        lua_pushinteger(lua, Inventory_AddItem(&ent->character->inventory, item_id, count));
        if(!player || ent->id == player->id)
        {
            Gui_NotifierStart(item_id);
        }
        return 1;
    }

    Con_Warning("no entity with id = %d", entity_id);
    return 0;
}


int lua_RemoveItem(lua_State * lua)
{
    if(lua_gettop(lua) < 3)
    {
        Con_Warning("removeItem: expecting arguments (entity_id, item_id, items_count)");
        return 0;
    }

    int entity_id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(entity_id);

    if(ent && ent->character)
    {
        int item_id = lua_tointeger(lua, 2);
        int count = lua_tointeger(lua, 3);
        lua_pushinteger(lua, Inventory_RemoveItem(&ent->character->inventory, item_id, count));
        return 1;
    }

    Con_Warning("no entity with id = %d", entity_id);
    return 0;
}


int lua_RemoveAllItems(lua_State * lua)
{
    if(lua_gettop(lua) < 1)
    {
        Con_Warning("removeAllItems: expecting arguments (entity_id)");
        return 0;
    }

    int entity_id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(entity_id);

    if(ent && ent->character)
    {
        Inventory_RemoveAllItems(&ent->character->inventory);
    }
    else
    {
        Con_Warning("no entity with id = %d", entity_id);
    }

    return 0;
}


int lua_GetItemsCount(lua_State * lua)
{
    if(lua_gettop(lua) < 2)
    {
        Con_Warning("getItemsCount: expecting arguments (entity_id, item_id)");
        return 0;
    }

    int entity_id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(entity_id);
    if(ent && ent->character)
    {
        int item_id = lua_tointeger(lua, 2);
        lua_pushinteger(lua, Inventory_GetItemsCount(ent->character->inventory, item_id));
        return 1;
    }
    else
    {
        Con_Warning("no entity with id = %d", entity_id);
        return 0;
    }
}


int lua_PrintItems(lua_State * lua)
{
    if(lua_gettop(lua) < 1)
    {
        Con_Warning("printItems: expecting arguments (entity_id)");
        return 0;
    }

    int entity_id = lua_tointeger(lua, 1);
    entity_p  ent = World_GetEntityByID(entity_id);
    if(ent && ent->character)
    {
        inventory_node_p i = ent->character->inventory;
        for(; i; i = i->next)
        {
            Con_Printf("item[id = %d]: count = %d", i->id, i->count);
        }
    }
    else
    {
        Con_Warning("no character with id = %d", entity_id);
    }

    return 0;
}


int lua_SetCharacterResponse(lua_State * lua)
{
    if(lua_gettop(lua) < 3)
    {
        Con_Warning("setEntityResponse: expecting arguments (entity_id, response_id, value)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent && ent->character)
    {
        int8_t value = (int8_t)lua_tointeger(lua, 3);

        switch(lua_tointeger(lua, 2))
        {
            case RESP_KILL:
                ent->character->resp.kill = value;
                break;

            case RESP_VERT_COLLIDE:
                ent->character->resp.vertical_collide = value;
                break;

            case RESP_HOR_COLLIDE:
                ent->character->resp.horizontal_collide = value;
                break;

            case RESP_SLIDE:
                ent->character->resp.slide = value;
                break;

            default:
                break;
        }
    }
    else
    {
        Con_Warning("no character with id = %d", id);
    }

    return 0;
}


int lua_GetCharacterResponse(lua_State * lua)
{
    if(lua_gettop(lua) < 2)
    {
        Con_Warning("getEntityResponse: expecting arguments (entity_id, response_id)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent && ent->character)
    {
        switch(lua_tointeger(lua, 2))
        {
            case 0:
                lua_pushinteger(lua, ent->character->resp.kill);
                break;
            case 1:
                lua_pushinteger(lua, ent->character->resp.vertical_collide);
                break;
            case 2:
                lua_pushinteger(lua, ent->character->resp.horizontal_collide);
                break;
            case 3:
                lua_pushinteger(lua, ent->character->resp.slide);
                break;
            default:
                lua_pushinteger(lua, 0);
                break;
        }
        return 1;
    }
    else
    {
        Con_Warning("no character with id = %d", id);
        return 0;
    }
}


int lua_SetCharacterWeaponModel(lua_State *lua)
{
    if(lua_gettop(lua) < 3)
    {
        Con_Printf("setCharacterWeaponModel: expecting arguments (id_entity, id_weapon_model, armed_state)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent && ent->character)
    {
        Character_SetWeaponModel(ent, lua_tointeger(lua, 2), lua_tointeger(lua, 3));
    }
    else
    {
        Con_Warning("no character with id = %d", id);
    }

    return 0;
}


int lua_GetCharacterCurrentWeapon(lua_State *lua)
{
    if(lua_gettop(lua) < 1)
    {
        Con_Warning("getCharacterCurrentWeapon: expecting arguments (entity_id)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent && ent->character)
    {
        lua_pushinteger(lua, ent->character->current_weapon);
        return 1;
    }
    else
    {
        Con_Warning("no character with id = %d", id);
        return 0;
    }
}


int lua_SetCharacterCurrentWeapon(lua_State *lua)
{
    if(lua_gettop(lua) < 2)
    {
        Con_Printf("setCharacterCurrentWeapon: expecting arguments (id_entity, id_weapon)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent && ent->character)
    {
        ent->character->current_weapon = lua_tointeger(lua, 2);
    }
    else
    {
        Con_Warning("no character with id = %d", id);
    }

    return 0;
}


void Script_LuaRegisterCharacterFuncs(lua_State *lua)
{
    lua_register(lua, "addItem", lua_AddItem);
    lua_register(lua, "removeItem", lua_RemoveItem);
    lua_register(lua, "removeAllItems", lua_RemoveAllItems);
    lua_register(lua, "getItemsCount", lua_GetItemsCount);
    lua_register(lua, "printItems", lua_PrintItems);

    lua_register(lua, "addEntityRagdoll", lua_AddEntityRagdoll);
    lua_register(lua, "removeEntityRagdoll", lua_RemoveEntityRagdoll);

    lua_register(lua, "setCharacterTarget", lua_SetCharacterTarget);
    lua_register(lua, "getCharacterParam", lua_GetCharacterParam);
    lua_register(lua, "setCharacterParam", lua_SetCharacterParam);
    lua_register(lua, "changeCharacterParam", lua_ChangeCharacterParam);

    lua_register(lua, "getCharacterResponse", lua_GetCharacterResponse);
    lua_register(lua, "setCharacterResponse", lua_SetCharacterResponse);
    lua_register(lua, "getCharacterCurrentWeapon", lua_GetCharacterCurrentWeapon);
    lua_register(lua, "setCharacterCurrentWeapon", lua_SetCharacterCurrentWeapon);
    lua_register(lua, "setCharacterWeaponModel", lua_SetCharacterWeaponModel);
    lua_register(lua, "getCharacterCombatMode", lua_GetCharacterCombatMode);
    lua_register(lua, "addCharacterHair", lua_AddCharacterHair);
    lua_register(lua, "resetCharacterHair", lua_ResetCharacterHair);
}