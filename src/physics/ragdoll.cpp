
#include <stdlib.h>
#include <string.h>
extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include "ragdoll.h"

struct rd_setup_s *Ragdoll_GetSetup(struct lua_State *lua, int stack_pos)
{
    struct rd_setup_s *setup = NULL;
    int top = lua_gettop(lua);

    if(!lua_istable(lua, stack_pos))
    {
        lua_settop(lua, top);
        return NULL;
    }

    lua_getfield(lua, stack_pos, "hit_callback");
    if(!lua_isstring(lua, -1))
    {
        lua_settop(lua, top);
        return NULL;
    }

    setup = (rd_setup_p)malloc(sizeof(rd_setup_t));
    setup->body_count = 0;
    setup->joint_count = 0;
    setup->body_setup = NULL;
    setup->joint_setup = NULL;
    setup->hit_func = NULL;
    setup->joint_cfm = 0.0;
    setup->joint_erp = 0.0;

    size_t string_length = 0;
    const char* func_name = lua_tolstring(lua, -1, &string_length);
    setup->hit_func = (char*)calloc(string_length, sizeof(char));
    memcpy(setup->hit_func, func_name, string_length * sizeof(char));
    lua_pop(lua, 1);

    lua_getfield(lua, stack_pos, "joint_count");
    setup->joint_count = (uint32_t)lua_tonumber(lua, -1);
    lua_pop(lua, 1);

    lua_getfield(lua, stack_pos, "body_count");
    setup->body_count  = (uint32_t)lua_tonumber(lua, -1);
    lua_pop(lua, 1);


    lua_getfield(lua, stack_pos, "joint_cfm");
    setup->joint_cfm   = lua_tonumber(lua, -1);
    lua_pop(lua, 1);

    lua_getfield(lua, stack_pos, "joint_erp");
    setup->joint_erp   = lua_tonumber(lua, -1);
    lua_pop(lua, 1);

    if((setup->body_count <= 0) || (setup->joint_count <= 0))
    {
        Ragdoll_DeleteSetup(setup);
        setup = NULL;
        lua_settop(lua, top);
        return NULL;
    }

    lua_getfield(lua, stack_pos, "body");
    if(!lua_istable(lua, -1))
    {
        Ragdoll_DeleteSetup(setup);
        setup = NULL;
        lua_settop(lua, top);
        return NULL;
    }

    setup->body_setup  = (rd_body_setup_p)calloc(setup->body_count, sizeof(rd_body_setup_t));
    setup->joint_setup = (rd_joint_setup_p)calloc(setup->joint_count, sizeof(rd_joint_setup_t));

    for(uint32_t i = 0; i < setup->body_count; i++)
    {
        lua_rawgeti(lua, -1, i + 1);
        if(!lua_istable(lua, -1))
        {
            Ragdoll_DeleteSetup(setup);
            setup = NULL;
            lua_settop(lua, top);
            return NULL;
        }
        lua_getfield(lua, -1, "mass");
        setup->body_setup[i].mass = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "restitution");
        setup->body_setup[i].restitution = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "friction");
        setup->body_setup[i].friction = lua_tonumber(lua, -1);
        lua_pop(lua, 1);


        lua_getfield(lua, -1, "damping");
        if(!lua_istable(lua, -1))
        {
            Ragdoll_DeleteSetup(setup);
            setup = NULL;
            lua_settop(lua, top);
            return NULL;
        }

        lua_rawgeti(lua, -1, 1);
        setup->body_setup[i].damping[0] = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_rawgeti(lua, -1, 2);
        setup->body_setup[i].damping[1] = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_pop(lua, 1);
        lua_pop(lua, 1);
    }
    lua_pop(lua, 1);

    lua_getfield(lua, stack_pos, "joint");
    if(!lua_istable(lua, -1))
    {
        Ragdoll_DeleteSetup(setup);
        setup = NULL;
        lua_settop(lua, top);
        return NULL;
    }

    for(uint32_t i = 0; i < setup->joint_count; i++)
    {
        lua_rawgeti(lua, -1, i + 1);
        if(!lua_istable(lua, -1))
        {
            Ragdoll_DeleteSetup(setup);
            setup = NULL;
            lua_settop(lua, top);
            return NULL;
        }
        lua_getfield(lua, -1, "body_index");
        setup->joint_setup[i].body_index = (uint16_t)lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "joint_type");
        setup->joint_setup[i].joint_type = (uint16_t)lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "body1_offset");
        if(!lua_istable(lua, -1))
        {
            Ragdoll_DeleteSetup(setup);
            setup = NULL;
            lua_settop(lua, top);
            return NULL;
        }
        lua_rawgeti(lua, -1, 1);
        setup->joint_setup[i].body1_offset[0] = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_rawgeti(lua, -1, 2);
        setup->joint_setup[i].body1_offset[1] = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_rawgeti(lua, -1, 3);
        setup->joint_setup[i].body1_offset[2] = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_pop(lua, 1);

        lua_getfield(lua, -1, "body2_offset");
        if(!lua_istable(lua, -1))
        {
            Ragdoll_DeleteSetup(setup);
            setup = NULL;
            lua_settop(lua, top);
            return NULL;
        }
        lua_rawgeti(lua, -1, 1);
        setup->joint_setup[i].body2_offset[0] = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_rawgeti(lua, -1, 2);
        setup->joint_setup[i].body2_offset[1] = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_rawgeti(lua, -1, 3);
        setup->joint_setup[i].body2_offset[2] = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_pop(lua, 1);

        lua_getfield(lua, -1, "body1_angle");
        if(!lua_istable(lua, -1))
        {
            Ragdoll_DeleteSetup(setup);
            setup = NULL;
            lua_settop(lua, top);
            return NULL;
        }
        lua_rawgeti(lua, -1, 1);
        setup->joint_setup[i].body1_angle[0] = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_rawgeti(lua, -1, 2);
        setup->joint_setup[i].body1_angle[1] = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_rawgeti(lua, -1, 3);
        setup->joint_setup[i].body1_angle[2] = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_pop(lua, 1);

        lua_getfield(lua, -1, "body2_angle");
        if(!lua_istable(lua, -1))
        {
            Ragdoll_DeleteSetup(setup);
            setup = NULL;
            lua_settop(lua, top);
            return NULL;
        }
        lua_rawgeti(lua, -1, 1);
        setup->joint_setup[i].body2_angle[0] = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_rawgeti(lua, -1, 2);
        setup->joint_setup[i].body2_angle[1] = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_rawgeti(lua, -1, 3);
        setup->joint_setup[i].body2_angle[2] = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_pop(lua, 1);

        lua_getfield(lua, -1, "joint_limit");
        if(!lua_istable(lua, -1))
        {
            Ragdoll_DeleteSetup(setup);
            setup = NULL;
            lua_settop(lua, top);
            return NULL;
        }
        lua_rawgeti(lua, -1, 1);
        setup->joint_setup[i].joint_limit[0] = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_rawgeti(lua, -1, 2);
        setup->joint_setup[i].joint_limit[1] = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_rawgeti(lua, -1, 3);
        setup->joint_setup[i].joint_limit[2] = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_pop(lua, 1);
        lua_pop(lua, 1);
    }

    lua_settop(lua, top);
    return setup;
}


void Ragdoll_DeleteSetup(struct rd_setup_s *setup)
{
    if(setup)
    {
        free(setup->body_setup);
        setup->body_setup = NULL;
        setup->body_count = 0;

        free(setup->joint_setup);
        setup->joint_setup = NULL;
        setup->joint_count = 0;

        free(setup->hit_func);
        setup->hit_func = NULL;

        free(setup);
    }
}
