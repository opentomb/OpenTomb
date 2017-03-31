
#include <stdlib.h>
extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include "hair.h"
#include "../script/script.h"

struct hair_setup_s *Hair_GetSetup(struct lua_State *lua, uint32_t hair_entry_index)
{
    struct hair_setup_s *hair_setup = NULL;
    int top = lua_gettop(lua);

    lua_getglobal(lua, "getHairSetup");
    if(!lua_isfunction(lua, -1))
    {
        lua_settop(lua, top);
        return NULL;
    }

    lua_pushinteger(lua, hair_entry_index);
    if(!lua_CallAndLog(lua, 1, 1, 0) || !lua_istable(lua, -1))
    {
        lua_settop(lua, top);
        return NULL;
    }

    hair_setup = (struct hair_setup_s*)malloc(sizeof(struct hair_setup_s));
    lua_getfield(lua, -1, "model");
    hair_setup->model_id = (uint32_t)lua_tonumber(lua, -1);
    lua_pop(lua, 1);

    lua_getfield(lua, -1, "link_body");
    hair_setup->link_body = (uint32_t)lua_tonumber(lua, -1);
    lua_pop(lua, 1);

    lua_getfield(lua, -1, "v_count");
    hair_setup->vertex_map_count = (uint32_t)lua_tonumber(lua, -1);
    lua_pop(lua, 1);


    lua_getfield(lua, -1, "props");
    if(!lua_istable(lua, -1))
    {
        free(hair_setup);
        lua_settop(lua, top);
        return NULL;
    }
    lua_getfield(lua, -1, "root_weight");
    hair_setup->root_weight      = lua_tonumber(lua, -1);
    lua_pop(lua, 1);

    lua_getfield(lua, -1, "tail_weight");
    hair_setup->tail_weight      = lua_tonumber(lua, -1);
    lua_pop(lua, 1);

    lua_getfield(lua, -1, "hair_inertia");
    hair_setup->hair_inertia     = lua_tonumber(lua, -1);
    lua_pop(lua, 1);

    lua_getfield(lua, -1, "hair_friction");
    hair_setup->hair_friction    = lua_tonumber(lua, -1);
    lua_pop(lua, 1);

    lua_getfield(lua, -1, "hair_bouncing");
    hair_setup->hair_restitution = lua_tonumber(lua, -1);
    lua_pop(lua, 1);

    lua_getfield(lua, -1, "joint_overlap");
    hair_setup->joint_overlap    = lua_tonumber(lua, -1);
    lua_pop(lua, 1);

    lua_getfield(lua, -1, "joint_cfm");
    hair_setup->joint_cfm        = lua_tonumber(lua, -1);
    lua_pop(lua, 1);

    lua_getfield(lua, -1, "joint_erp");
    hair_setup->joint_erp        = lua_tonumber(lua, -1);
    lua_pop(lua, 1);


    lua_getfield(lua, -1, "hair_damping");
    if(!lua_istable(lua, -1))
    {
        free(hair_setup);
        lua_settop(lua, top);
        return NULL;
    }
    lua_rawgeti(lua, -1, 1);
    hair_setup->hair_damping[0] = lua_tonumber(lua, -1);
    lua_pop(lua, 1);

    lua_rawgeti(lua, -1, 2);
    hair_setup->hair_damping[1] = lua_tonumber(lua, -1);
    lua_pop(lua, 1);

    lua_pop(lua, 1);
    lua_pop(lua, 1);

    lua_getfield(lua, -1, "v_index");
    if(!lua_istable(lua, -1))
    {
        free(hair_setup);
        lua_settop(lua, top);
        return NULL;
    }
    for(uint32_t i = 1; i <= hair_setup->vertex_map_count; i++)
    {
        lua_rawgeti(lua, -1, i);
        hair_setup->head_vertex_map[i-1] = (uint32_t)lua_tonumber(lua, -1);
        lua_pop(lua, 1);

    }
    lua_pop(lua, 1);

    lua_getfield(lua, -1, "offset");
    if(!lua_istable(lua, -1))
    {
        free(hair_setup);
        lua_settop(lua, top);
        return NULL;
    }
    lua_rawgeti(lua, -1, 1);
    hair_setup->head_offset[0] = lua_tonumber(lua, -1);
    lua_pop(lua, 1);

    lua_rawgeti(lua, -1, 2);
    hair_setup->head_offset[1] = lua_tonumber(lua, -1);
    lua_pop(lua, 1);

    lua_rawgeti(lua, -1, 3);
    hair_setup->head_offset[2] = lua_tonumber(lua, -1);
    lua_pop(lua, 1);

    lua_pop(lua, 1);

    lua_getfield(lua, -1, "root_angle");
    if(!lua_istable(lua, -1))
    {
        free(hair_setup);
        lua_settop(lua, top);
        return NULL;
    }
    lua_rawgeti(lua, -1, 1);
    hair_setup->root_angle[0] = lua_tonumber(lua, -1);
    lua_pop(lua, 1);

    lua_rawgeti(lua, -1, 2);
    hair_setup->root_angle[1] = lua_tonumber(lua, -1);
    lua_pop(lua, 1);

    lua_rawgeti(lua, -1, 3);
    hair_setup->root_angle[2] = lua_tonumber(lua, -1);
    lua_pop(lua, 1);


    lua_settop(lua, top);
    return hair_setup;
}
