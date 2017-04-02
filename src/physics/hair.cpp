
#include <stdlib.h>
extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include "hair.h"

struct hair_setup_s *Hair_GetSetup(struct lua_State *lua, int stack_pos)
{
    struct hair_setup_s *hair_setup = NULL;
    int top = lua_gettop(lua);

    if(!lua_istable(lua, stack_pos))
    {
        lua_settop(lua, top);
        return NULL;
    }

    hair_setup = (struct hair_setup_s*)malloc(sizeof(struct hair_setup_s));
    lua_getfield(lua, stack_pos, "model");
    hair_setup->model_id = (uint32_t)lua_tonumber(lua, -1);
    lua_pop(lua, 1);

    lua_getfield(lua, stack_pos, "link_body");
    hair_setup->link_body = (uint32_t)lua_tonumber(lua, -1);
    lua_pop(lua, 1);

    lua_getfield(lua, stack_pos, "v_count");
    hair_setup->vertex_map_count = (uint32_t)lua_tonumber(lua, -1);
    lua_pop(lua, 1);

    lua_getfield(lua, stack_pos, "props");
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

    lua_getfield(lua, stack_pos, "v_index");
    if(!lua_istable(lua, -1))
    {
        free(hair_setup);
        lua_settop(lua, top);
        return NULL;
    }
    for(uint32_t i = 0; i < hair_setup->vertex_map_count; i++)
    {
        lua_rawgeti(lua, -1, i + 1);
        hair_setup->head_vertex_map[i] = (uint32_t)lua_tonumber(lua, -1);
        lua_pop(lua, 1);
    }
    lua_pop(lua, 1);

    lua_getfield(lua, stack_pos, "offset");
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

    lua_getfield(lua, stack_pos, "root_angle");
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
