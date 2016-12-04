
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
#include "../mesh.h"
#include "../skeletal_model.h"
#include "../trigger.h"
#include "../room.h"
#include "../entity.h"
#include "../world.h"
#include "../engine.h"
#include "../physics.h"


int Script_ExecEntity(lua_State *lua, int id_callback, int id_object, int id_activator)
{
    int top = lua_gettop(lua);
    int ret = ENTITY_TRIGGERING_NOT_READY;

    lua_getglobal(lua, "execEntity");
    if(lua_isfunction(lua, -1))
    {
        int argn = 0;
        lua_pushinteger(lua, id_callback);  argn++;
        lua_pushinteger(lua, id_object);    argn++;

        if(id_activator >= 0)
        {
            lua_pushinteger(lua, id_activator);
            argn++;
        }

        if(lua_pcall(lua, argn, 1, 0) == LUA_OK)
        {
            ret = lua_tointeger(lua, -1);
        }
    }
    lua_settop(lua, top);
    //Sys_Warn("Broken \"execEntity\" script function");
    return ret;
}


void Script_LoopEntity(lua_State *lua, int object_id)
{
    entity_p ent = World_GetEntityByID(object_id);
    if((lua) && (ent->state_flags & ENTITY_STATE_ACTIVE))
    {
        int top = lua_gettop(lua);
        lua_getglobal(lua, "loopEntity");
        if(lua_isfunction(lua, -1))
        {
            lua_pushinteger(lua, object_id);
            lua_CallAndLog(lua, 1, 0, 0);
        }
        lua_settop(lua, top);
    }
}

/*
 * Base Entity trigger functions
 */
int lua_ActivateEntity(lua_State *lua)
{
    if(lua_gettop(lua) >= 6)
    {
        entity_p object = World_GetEntityByID(lua_tointeger(lua, 1));
        if(object)
        {
            entity_p activator          = World_GetEntityByID(lua_tointeger(lua, 2));
            uint16_t trigger_mask       = lua_tointeger(lua, 3);
            uint16_t trigger_op         = lua_tointeger(lua, 4);
            uint16_t trigger_lock       = lua_tointeger(lua, 5);
            uint16_t trigger_timer      = lua_tointeger(lua, 6);
            uint16_t ret = Entity_Activate(object, activator, trigger_mask, trigger_op, trigger_lock, trigger_timer);
            lua_pushinteger(lua, ret);
            return 1;
        }
    }
    else
    {
        Con_Warning("activateEntity: expecting arguments (object_id, activator_id, trigger_mask, trigger_op, trigger_lock, trigger_timer)");
    }

    return 0;
}


int lua_DeactivateEntity(lua_State *lua)
{
    if(lua_gettop(lua) >= 2)
    {
        entity_p object = World_GetEntityByID(lua_tointeger(lua, 1));
        if(object)
        {
            entity_p activator = World_GetEntityByID(lua_tointeger(lua, 2));
            uint16_t ret = Entity_Deactivate(object, activator);
            lua_pushinteger(lua, ret);
            return 1;
        }
    }
    else
    {
        Con_Warning("deactivateEntity: expecting arguments (object_id, activator_id)");
    }

    return 0;
}


int lua_NoFixEntityCollision(lua_State *lua)
{
    if(lua_gettop(lua) >= 1)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            ent->no_fix_all = 0x01;
        }
    }
    else
    {
        Con_Warning("noFixEntityCollision: Expecting arguments (entity_id)");
    }

    return 0;
}


int lua_EnableEntity(lua_State * lua)
{
    if(lua_gettop(lua) >= 1)
    {
        entity_p ent = World_GetEntityByID(lua_tonumber(lua, 1));
        if(ent)
        {
            Entity_Enable(ent);
        }
    }
    else
    {
        Con_Warning("enableEntity: Expecting arguments (entity_id)");
    }

    return 0;
}


int lua_DisableEntity(lua_State * lua)
{
    if(lua_gettop(lua) >= 1)
    {
        entity_p ent = World_GetEntityByID(lua_tonumber(lua, 1));
        if(ent)
        {
            Entity_Disable(ent);
        }
    }
    else
    {
        Con_Warning("disableEntity: Expecting arguments (entity_id)");
    }

    return 0;
}


int lua_SetEntityCollision(lua_State * lua)
{
    int top = lua_gettop(lua);

    if(top >= 1)
    {
        entity_p ent = World_GetEntityByID(lua_tonumber(lua, 1));
        if(ent)
        {
            if((top >= 2) && (lua_tointeger(lua, 2)))
            {
                Entity_EnableCollision(ent);
            }
            else
            {
                Entity_DisableCollision(ent);
            }
        }
    }
    else
    {
        Con_Warning("setEntityCollision: Expecting arguments (entity_id)");
    }

    return 0;
}


int lua_SetEntityGhostCollisionShape(lua_State * lua)
{
    if(lua_gettop(lua) >= 8)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            uint16_t ghost_index = lua_tointeger(lua, 2);
            ghost_shape_t shape;
            shape.bb_min[0] = lua_tonumber(lua, 3);
            shape.bb_min[1] = lua_tonumber(lua, 4);
            shape.bb_min[2] = lua_tonumber(lua, 5);
            shape.bb_max[0] = lua_tonumber(lua, 6);
            shape.bb_max[1] = lua_tonumber(lua, 7);
            shape.bb_max[2] = lua_tonumber(lua, 8);

            Physics_SetGhostCollisionShape(ent->physics, ghost_index, &shape);
        }
        else
        {
            Con_Warning("no entity with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("setEntityGhostCollisionShape: expecting arguments (entity_id, shape_index, min_x, min_y, min_z, max_x, max_y, max_z)");
    }

    return 0;
}


int lua_SetEntityCollisionFlags(lua_State * lua)
{
    if(lua_gettop(lua) >= 4)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            if(!lua_isnil(lua, 2))
            {
                ent->self->collision_group = lua_tointeger(lua, 2);
            }
            if(!lua_isnil(lua, 3))
            {
                ent->self->collision_shape = lua_tointeger(lua, 3);
            }
            if(!lua_isnil(lua, 4))
            {
                ent->self->collision_mask = lua_tointeger(lua, 4);
            }

            if(Physics_GetBodiesCount(ent->physics) != ent->bf->bone_tag_count)
            {
                ent->self->collision_shape = COLLISION_SHAPE_SINGLE_BOX;
            }
        }
        else
        {
            Con_Warning("no entity with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("setEntityCollisionFlags: expecting arguments (entity_id, collision_group, collision_shape, collision_mask)");
    }

    return 0;
}


int lua_GetEntitySectorFlags(lua_State *lua)
{
    if(lua_gettop(lua) >= 1)
    {
        entity_p ent = World_GetEntityByID(lua_tonumber(lua, 1));
        if(ent && (ent->current_sector))
        {
            lua_pushinteger(lua, ent->current_sector->flags);
            return 1;
        }
    }

    return 0;
}


int lua_GetEntitySectorIndex(lua_State *lua)
{
    if(lua_gettop(lua) >= 1)
    {
        entity_p ent = World_GetEntityByID(lua_tonumber(lua, 1));
        if(ent && (ent->current_sector))
        {
            lua_pushinteger(lua, ent->current_sector->trig_index);
            return 1;
        }
    }

    return 0;
}


int lua_GetEntitySectorMaterial(lua_State *lua)
{
    if(lua_gettop(lua) >= 1)
    {
        entity_p ent = World_GetEntityByID(lua_tonumber(lua, 1));
        if(ent && (ent->current_sector))
        {
            lua_pushinteger(lua, ent->current_sector->material);
            return 1;
        }
    }

    return 0;
}


int lua_GetEntityActivationOffset(lua_State * lua)
{
    if(lua_gettop(lua) >= 1)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            lua_pushnumber(lua, ent->activation_offset[0]);
            lua_pushnumber(lua, ent->activation_offset[1]);
            lua_pushnumber(lua, ent->activation_offset[2]);
            lua_pushnumber(lua, ent->activation_offset[3]);
            return 4;
        }
    }

    return 0;
}


int lua_SetEntityActivationOffset(lua_State * lua)
{
    int top = lua_gettop(lua);

    if(top >= 1)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            if(top >= 4)
            {
                ent->activation_offset[0] = lua_tonumber(lua, 2);
                ent->activation_offset[1] = lua_tonumber(lua, 3);
                ent->activation_offset[2] = lua_tonumber(lua, 4);
            }
            if(top >= 5)
            {
                ent->activation_offset[3] = lua_tonumber(lua, 5);
            }
        }
        else
        {
            Con_Warning("no entity with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("setEntityActivationOffset: Expecting arguments (entity_id)");
    }

    return 0;
}


int lua_GetEntityActivationDirection(lua_State * lua)
{
    if(lua_gettop(lua) >= 1)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            lua_pushnumber(lua, ent->activation_direction[0]);
            lua_pushnumber(lua, ent->activation_direction[1]);
            lua_pushnumber(lua, ent->activation_direction[2]);
            lua_pushnumber(lua, ent->activation_direction[3]);
            return 4;
        }
    }

    return 0;
}


int lua_SetEntityActivationDirection(lua_State * lua)
{
    int top = lua_gettop(lua);

    if(top >= 4)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            ent->activation_direction[0] = lua_tonumber(lua, 2);
            ent->activation_direction[1] = lua_tonumber(lua, 3);
            ent->activation_direction[2] = lua_tonumber(lua, 4);
            if(top >= 5)
            {
                ent->activation_direction[3] = lua_tonumber(lua, 5);
            }
        }
        else
        {
            Con_Warning("no entity with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("setEntityActivationDirection: Expecting arguments (entity_id, x, y, z, (w))");
    }

    return 0;
}


/*
 * Moveables script control section
 */
int lua_GetEntityVector(lua_State * lua)
{
    if(lua_gettop(lua) >= 2)
    {
        entity_p e1 = World_GetEntityByID(lua_tointeger(lua, 1));
        entity_p e2 = World_GetEntityByID(lua_tointeger(lua, 2));
        if(e1 && e2)
        {
            lua_pushnumber(lua, e2->transform[12 + 0] - e1->transform[12 + 0]);
            lua_pushnumber(lua, e2->transform[12 + 1] - e1->transform[12 + 1]);
            lua_pushnumber(lua, e2->transform[12 + 2] - e1->transform[12 + 2]);
            return 3;
        }
    }
    else
    {
        Con_Warning("getEntityVector: expecting arguments (entity_id1, entity_id2)");
    }

    return 0;
}


int lua_GetEntityDistance(lua_State * lua)
{
    if(lua_gettop(lua) >= 2)
    {
        entity_p e1 = World_GetEntityByID(lua_tointeger(lua, 1));
        entity_p e2 = World_GetEntityByID(lua_tointeger(lua, 2));
        if(e1 && e2)
        {
            lua_pushnumber(lua, vec3_dist(e1->transform + 12, e2->transform + 12));
            return 1;
        }
    }
    else
    {
        Con_Warning("getEntityDistance: expecting arguments (entity_id1, entity_id2)");
    }

    return 0;
}


int lua_GetEntityDirDot(lua_State * lua)
{
    if(lua_gettop(lua) >= 2)
    {
        entity_p e1 = World_GetEntityByID(lua_tointeger(lua, 1));
        entity_p e2 = World_GetEntityByID(lua_tointeger(lua, 2));
        if(e1 && e2)
        {
            lua_pushnumber(lua, vec3_dot(e1->transform + 4, e2->transform + 4));
            return 1;
        }
    }
    else
    {
        Con_Warning("getEntityDirDot: expecting arguments (id1, id2)");
    }

    return 0;
}


int lua_GetEntityPosition(lua_State * lua)
{
    if(lua_gettop(lua) >= 1)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            lua_pushnumber(lua, ent->transform[12 + 0]);
            lua_pushnumber(lua, ent->transform[12 + 1]);
            lua_pushnumber(lua, ent->transform[12 + 2]);
            lua_pushnumber(lua, ent->angles[0]);
            lua_pushnumber(lua, ent->angles[1]);
            lua_pushnumber(lua, ent->angles[2]);
            return 6;
        }
        else
        {
            Con_Warning("no entity with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("getEntityPosition: expecting arguments (entity_id)");
    }

    return 0;
}


int lua_GetEntityAngles(lua_State * lua)
{
    if(lua_gettop(lua) >= 1)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            lua_pushnumber(lua, ent->angles[0]);
            lua_pushnumber(lua, ent->angles[1]);
            lua_pushnumber(lua, ent->angles[2]);
            return 3;
        }
        else
        {
            Con_Warning("no entity with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("getEntityAngles: expecting arguments (entity_id)");
    }

    return 0;
}


int lua_GetEntityScaling(lua_State * lua)
{
    if(lua_gettop(lua) >= 1)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            lua_pushnumber(lua, ent->scaling[0]);
            lua_pushnumber(lua, ent->scaling[1]);
            lua_pushnumber(lua, ent->scaling[2]);
            return 3;
        }
        else
        {
            Con_Warning("no entity with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("getEntityScaling: expecting arguments (entity_id)");
    }

    return 0;
}


int lua_SetEntityScaling(lua_State * lua)
{
    if(lua_gettop(lua) >= 4)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            ent->scaling[0] = lua_tonumber(lua, 2);
            ent->scaling[1] = lua_tonumber(lua, 3);
            ent->scaling[2] = lua_tonumber(lua, 4);

            Physics_SetCollisionScale(ent->physics, ent->scaling);
            Entity_UpdateRigidBody(ent, 1);
        }
        else
        {
            Con_Warning("no entity with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("setEntityScaling: expecting arguments (entity_id, x_scaling, y_scaling, z_scaling)");
    }

    return 0;
}


int lua_SetEntityPosition(lua_State * lua)
{
    switch(lua_gettop(lua))
    {
        case 4:
            {
                entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
                if(ent)
                {
                    ent->transform[12 + 0] = lua_tonumber(lua, 2);
                    ent->transform[12 + 1] = lua_tonumber(lua, 3);
                    ent->transform[12 + 2] = lua_tonumber(lua, 4);
                    Entity_UpdateRigidBody(ent, 1);
                }
                else
                {
                    Con_Warning("no entity with id = %d", lua_tointeger(lua, 1));
                }
            }
            return 0;

        case 7:
            {
                entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
                if(ent)
                {
                    ent->transform[12 + 0] = lua_tonumber(lua, 2);
                    ent->transform[12 + 1] = lua_tonumber(lua, 3);
                    ent->transform[12 + 2] = lua_tonumber(lua, 4);
                    ent->angles[0] = lua_tonumber(lua, 5);
                    ent->angles[1] = lua_tonumber(lua, 6);
                    ent->angles[2] = lua_tonumber(lua, 7);
                    Entity_UpdateTransform(ent);
                    Entity_UpdateRigidBody(ent, 1);
                }
                else
                {
                    Con_Warning("no entity with id = %d", lua_tointeger(lua, 1));
                }
            }
            return 0;

        default:
            Con_Warning("setEntityPosition: expecting arguments (entity_id, x, y, z, (fi_x, fi_y, fi_z))");
            return 0;
    }

    return 0;
}


int lua_SetEntityAngles(lua_State * lua)
{
    if(lua_gettop(lua) >= 4)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            ent->angles[0] = lua_tonumber(lua, 2);
            ent->angles[1] = lua_tonumber(lua, 3);
            ent->angles[2] = lua_tonumber(lua, 4);
            Entity_UpdateTransform(ent);
        }
        else
        {
            Con_Warning("no entity with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("setEntityAngles: expecting arguments (entity_id, fi_x, (fi_y, fi_z))");
    }

    return 0;
}


int lua_MoveEntityGlobal(lua_State * lua)
{
    switch(lua_gettop(lua))
    {
        case 4:
            {
                entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
                if(ent)
                {
                    ent->transform[12 + 0] += lua_tonumber(lua, 2);
                    ent->transform[12 + 1] += lua_tonumber(lua, 3);
                    ent->transform[12 + 2] += lua_tonumber(lua, 4);
                    Entity_UpdateRigidBody(ent, 1);
                }
                else
                {
                    Con_Printf("can not find entity with id = %d", lua_tointeger(lua, 1));
                }
            }
            return 0;

        default:
            Con_Warning("moveEntityGlobal: expecting arguments (entity_id, x, y, z)");
            return 0;
    }

    return 0;
}


int lua_MoveEntityLocal(lua_State * lua)
{
    if(lua_gettop(lua) >= 4)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            float dx = lua_tonumber(lua, 2);
            float dy = lua_tonumber(lua, 3);
            float dz = lua_tonumber(lua, 4);

            ent->transform[12 + 0] += dx * ent->transform[0 + 0] + dy * ent->transform[4 + 0] + dz * ent->transform[8 + 0];
            ent->transform[12 + 1] += dx * ent->transform[0 + 1] + dy * ent->transform[4 + 1] + dz * ent->transform[8 + 1];
            ent->transform[12 + 2] += dx * ent->transform[0 + 2] + dy * ent->transform[4 + 2] + dz * ent->transform[8 + 2];

            Entity_UpdateRigidBody(ent, 1);
        }
        else
        {
            Con_Warning("no entity with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("moveEntityLocal: expecting arguments (entity_id, dx, dy, dz)");
    }

    return 0;
}


int lua_MoveEntityToSink(lua_State * lua)
{
    if(lua_gettop(lua) >= 2)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            Entity_MoveToSink(ent, lua_tointeger(lua, 2));
        }
    }
    else
    {
        Con_Warning("moveEntityToSink: expecting arguments (entity_id, sink_id)");
    }

    return 0;
}


int lua_MoveEntityToEntity(lua_State * lua)
{
    int top = lua_gettop(lua);

    if(top >= 3)
    {
        entity_p ent1 = World_GetEntityByID(lua_tointeger(lua, 1));
        entity_p ent2 = World_GetEntityByID(lua_tointeger(lua, 2));
        if(ent1 && ent2)
        {
            float speed_mult = lua_tonumber(lua, 3);
            float *ent1_pos = ent1->transform + 12;
            float *ent2_pos = ent2->transform + 12;
            float t, speed[3];

            vec3_sub(speed, ent2_pos, ent1_pos);
            t = vec3_abs(speed);
            t = (t == 0.0f) ? 1.0f : t; // Prevents division by zero.
            t = speed_mult / t;

            ent1->transform[12 + 0] += speed[0] * t;
            ent1->transform[12 + 1] += speed[1] * t;
            if((top == 3) || !lua_toboolean(lua, 4))
            {
                ent1->transform[12 + 2] += speed[2] * t;
            }

            Entity_UpdateRigidBody(ent1, 1);
        }
    }
    else
    {
        Con_Warning("moveEntityToEntity: expecting arguments (moved_entity_id, target_entity_id, speed, (ignore_z))");
    }

    return 0;
}


int lua_RotateEntity(lua_State *lua)
{
    int top = lua_gettop(lua);

    if(top >= 2)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            ent->angles[0] += lua_tonumber(lua, 2);
            if(top >= 4)
            {
                 ent->angles[1] += lua_tonumber(lua, 3);
                 ent->angles[2] += lua_tonumber(lua, 4);
            }
            Entity_UpdateTransform(ent);
            Entity_UpdateRigidBody(ent, 1);
        }
        else
        {
            Con_Warning("no entity with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("rotateEntity: expecting arguments (ent_id, rot_x, (rot_y, rot_z))");
    }

    return 0;
}


int lua_GetEntitySpeed(lua_State * lua)
{
    if(lua_gettop(lua) >= 1)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            lua_pushnumber(lua, ent->speed[0]);
            lua_pushnumber(lua, ent->speed[1]);
            lua_pushnumber(lua, ent->speed[2]);
            return 3;
        }
        else
        {
            Con_Warning("no entity with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("getEntitySpeed: expecting arguments (entity_id)");
    }

    return 0;
}


int lua_GetEntitySpeedLinear(lua_State * lua)
{
    if(lua_gettop(lua) >= 1)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            lua_pushnumber(lua, vec3_abs(ent->speed));
            return 1;
        }
        else
        {
            Con_Warning("no entity with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("getEntitySpeedLinear: expecting arguments (entity_id)");
    }

    return 0;
}


int lua_SetEntitySpeed(lua_State * lua)
{
    if(lua_gettop(lua) >= 4)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            ent->speed[0] = lua_tonumber(lua, 2);
            ent->speed[1] = lua_tonumber(lua, 3);
            ent->speed[2] = lua_tonumber(lua, 4);
        }
        else
        {
            Con_Warning("no entity with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("setEntitySpeed: expecting arguments (id, speed_x, speed_y, speed_z)");
    }

    return 0;
}


int lua_SetEntityLinearSpeed(lua_State * lua)
{
    if(lua_gettop(lua) >= 2)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            ent->linear_speed = lua_tonumber(lua, 2);
        }
        else
        {
            Con_Warning("no entity with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("setEntityLinearSpeed: expecting arguments (id, speed");
    }

    return 0;
}


int lua_SetEntityBodyPartFlag(lua_State * lua)
{
    if(lua_gettop(lua) >= 3)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            int bone_id = lua_tointeger(lua, 2);
            if((bone_id >= 0) && (bone_id < ent->bf->bone_tag_count))
            {
                ent->bf->bone_tags[bone_id].body_part = lua_tointeger(lua, 3);
            }
        }
        else
        {
            Con_Warning("no entity with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("setEntityBodyPartFlag: expecting arguments (entity_id, bone_id, body_part_flag)");
    }

    return 0;
}


int lua_CanTriggerEntity(lua_State * lua)
{
    if(lua_gettop(lua) >= 2)
    {
        lua_pushboolean(lua, Entity_CanTrigger(World_GetEntityByID(lua_tointeger(lua, 1)),
                                               World_GetEntityByID(lua_tointeger(lua, 2))));
    }
    else
    {
        lua_pushboolean(lua, false);
    }

    return 1;
}


int lua_EntityRotateToTriggerZ(lua_State * lua)
{
    if(lua_gettop(lua) >= 2)
    {
        Entity_RotateToTriggerZ(World_GetEntityByID(lua_tointeger(lua, 1)),
                                World_GetEntityByID(lua_tointeger(lua, 2)));
    }

    return 0;
}


int lua_EntityRotateToTrigger(lua_State * lua)
{
    if(lua_gettop(lua) >= 2)
    {
        Entity_RotateToTrigger(World_GetEntityByID(lua_tointeger(lua, 1)),
                               World_GetEntityByID(lua_tointeger(lua, 2)));
    }

    return 0;
}


int lua_EntityMoveToTriggerActivationPoint(lua_State * lua)
{
    if(lua_gettop(lua) >= 2)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        entity_p trigger = World_GetEntityByID(lua_tointeger(lua, 2));
        if(ent && trigger)
        {
            float *pos = ent->transform + 12;
            Mat4_vec3_mul_macro(pos, trigger->transform, trigger->activation_offset);
        }
    }

    return 0;
}


int lua_GetEntityVisibility(lua_State * lua)
{
    if(lua_gettop(lua) >= 1)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            lua_pushinteger(lua, (ent->state_flags & ENTITY_STATE_VISIBLE) != 0);
            return 1;
        }
        else
        {
            Con_Warning("no entity with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("getEntityVisibility: expecting arguments (entity_id)");
    }

    return 0;
}


int lua_SetEntityVisibility(lua_State * lua)
{
    if(lua_gettop(lua) >= 2)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            if(lua_tointeger(lua, 2) != 0)
            {
                ent->state_flags |= ENTITY_STATE_VISIBLE;
            }
            else
            {
                ent->state_flags &= ~ENTITY_STATE_VISIBLE;
            }
        }
        else
        {
            Con_Warning("no entity with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("setEntityVisibility: expecting arguments (entity_id, value)");
    }

    return 0;
}


int lua_GetEntityEnability(lua_State * lua)
{
    if(lua_gettop(lua) >= 1)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            lua_pushboolean(lua, (ent->state_flags & ENTITY_STATE_ENABLED) != 0);
            return 1;
        }
        else
        {
            Con_Warning("no entity with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("getEntityEnability: expecting arguments (entity_id)");
    }

    return 0;
}


int lua_GetEntityActivity(lua_State * lua)
{
    if(lua_gettop(lua) >= 1)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            lua_pushboolean(lua, (ent->state_flags & ENTITY_STATE_ACTIVE) != 0);
            return 1;
        }
        else
        {
            Con_Warning("no entity with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("getEntityActivity: expecting arguments (entity_id)");
    }

    return 0;
}


int lua_SetEntityActivity(lua_State * lua)
{
    if(lua_gettop(lua) >= 2)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            if(lua_toboolean(lua, 2))
            {
                ent->state_flags |= ENTITY_STATE_ACTIVE;
            }
            else
            {
                ent->state_flags &= ~ENTITY_STATE_ACTIVE;
            }
        }
        else
        {
            Con_Warning("no entity with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("setEntityActivity: expecting arguments (entity_id, value)");
    }

    return 0;
}


int lua_GetEntityTriggerLayout(lua_State *lua)
{
    if(lua_gettop(lua) >= 1)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            lua_pushinteger(lua, (ent->trigger_layout & ENTITY_TLAYOUT_MASK));          // mask
            lua_pushinteger(lua, (ent->trigger_layout & ENTITY_TLAYOUT_EVENT) >> 5);    // event
            lua_pushinteger(lua, (ent->trigger_layout & ENTITY_TLAYOUT_LOCK) >> 6);     // lock
            return 3;
        }
    }

    return 0;
}


int lua_SetEntityTriggerLayout(lua_State *lua)
{
    int top = lua_gettop(lua);

    if(top >= 2)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            if(top == 2)
            {
                ent->trigger_layout = (uint8_t)lua_tointeger(lua, 2);
            }
            else if(top == 4)
            {
                uint8_t trigger_layout = ent->trigger_layout;
                trigger_layout &= ~(uint8_t)(ENTITY_TLAYOUT_MASK);  trigger_layout ^= (uint8_t)lua_tointeger(lua, 2);          // mask  - 00011111
                trigger_layout &= ~(uint8_t)(ENTITY_TLAYOUT_EVENT); trigger_layout ^= ((uint8_t)lua_tointeger(lua, 3)) << 5;   // event - 00100000
                trigger_layout &= ~(uint8_t)(ENTITY_TLAYOUT_LOCK);  trigger_layout ^= ((uint8_t)lua_tointeger(lua, 4)) << 6;   // lock  - 01000000
                ent->trigger_layout = trigger_layout;
            }
        }
        else
        {
            Con_Warning("no entity with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("setEntityTriggerLayout: expecting arguments (entity_id, layout) or (entity_id, mask, event, once)");
    }

    return 0;
}


int lua_SetEntityLock(lua_State * lua)
{
    if(lua_gettop(lua) >= 2)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            uint8_t trigger_layout = ent->trigger_layout;
            trigger_layout &= ~(uint8_t)(ENTITY_TLAYOUT_LOCK);  trigger_layout ^= ((uint8_t)lua_tointeger(lua, 2)) << 6;   // lock  - 01000000
            ent->trigger_layout = trigger_layout;
        }
    }

    return 0;
}


int lua_GetEntityLock(lua_State * lua)
{
    if(lua_gettop(lua) >= 1)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            lua_pushinteger(lua, ((ent->trigger_layout & ENTITY_TLAYOUT_LOCK) >> 6));      // lock
            return 1;
        }
    }

    return 0;
}


int lua_SetEntityEvent(lua_State * lua)
{
    if(lua_gettop(lua) >= 2)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            uint8_t trigger_layout = ent->trigger_layout;
            trigger_layout &= ~(uint8_t)(ENTITY_TLAYOUT_EVENT);
            trigger_layout ^= ((uint8_t)lua_tointeger(lua, 2)) << 5;   // event - 00100000
            ent->trigger_layout = trigger_layout;
        }
    }

    return 0;
}


int lua_GetEntityEvent(lua_State *lua)
{
    if(lua_gettop(lua) >= 1)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            lua_pushinteger(lua, ((ent->trigger_layout & ENTITY_TLAYOUT_EVENT) >> 5));    // event
            return 1;
        }
    }

    return 0;
}


int lua_GetEntityMask(lua_State * lua)
{
    if(lua_gettop(lua) >= 1)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            lua_pushinteger(lua, (ent->trigger_layout & ENTITY_TLAYOUT_MASK));      // mask
            return 1;
        }
    }

    return 0;
}


int lua_SetEntityMask(lua_State * lua)
{
    if(lua_gettop(lua) >= 2)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            uint8_t trigger_layout = ent->trigger_layout;
            trigger_layout &= ~(uint8_t)(ENTITY_TLAYOUT_MASK);  trigger_layout ^= (uint8_t)lua_tointeger(lua, 2);   // mask  - 00011111
            ent->trigger_layout = trigger_layout;
        }
    }

    return 0;
}


int lua_GetEntitySectorStatus(lua_State *lua)
{
    if(lua_gettop(lua) >= 1)
    {
        entity_p ent = World_GetEntityByID(lua_tonumber(lua, 1));
        if(ent)
        {
            lua_pushinteger(lua, ((ent->trigger_layout & ENTITY_TLAYOUT_SSTATUS) >> 7));
            return 1;
        }
    }

    return 0;
}


int lua_SetEntitySectorStatus(lua_State *lua)
{
    if(lua_gettop(lua) >= 2)
    {
        entity_p ent = World_GetEntityByID(lua_tonumber(lua, 1));
        if(ent)
        {
            uint8_t trigger_layout = ent->trigger_layout;
            trigger_layout &= ~(uint8_t)(ENTITY_TLAYOUT_SSTATUS);
            trigger_layout ^=  ((uint8_t)lua_tointeger(lua, 2)) << 7;   // sector_status  - 10000000
            ent->trigger_layout = trigger_layout;
        }
    }

    return 0;
}


int lua_GetEntityOCB(lua_State * lua)
{
    if(lua_gettop(lua) >= 1)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            lua_pushinteger(lua, ent->OCB);
            return 1;
        }
    }

    return 0;   // No entity found - return.
}


int lua_SetEntityOCB(lua_State * lua)
{
    if(lua_gettop(lua) >= 2)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            ent->OCB = lua_tointeger(lua, 2);
        }
    }

    return 0;
}


int lua_GetEntityFlags(lua_State * lua)
{
    if(lua_gettop(lua) >= 1)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            lua_pushinteger(lua, ent->state_flags);
            lua_pushinteger(lua, ent->type_flags);
            lua_pushinteger(lua, ent->callback_flags);
            return 3;
        }
        else
        {
            Con_Warning("no entity with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("getEntityFlags: expecting arguments (entity_id)");
    }

    return 0;
}


int lua_SetEntityFlags(lua_State * lua)
{
    if(lua_gettop(lua) >= 4)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            if(!lua_isnil(lua, 2))
            {
                ent->state_flags = lua_tointeger(lua, 2);
            }
            if(!lua_isnil(lua, 3))
            {
                ent->type_flags = lua_tointeger(lua, 3);
            }
            if(!lua_isnil(lua, 4))
            {
                ent->callback_flags = lua_tointeger(lua, 4);
            }
        }
        else
        {
            Con_Warning("no entity with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("setEntityFlags: expecting arguments (entity_id, state_flags, type_flags, callback_flags)");
    }

    return 0;
}


int lua_GetEntityTypeFlag(lua_State *lua)
{
    int top = lua_gettop(lua);

    if(top >= 1)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            if(top == 1)
            {
                lua_pushinteger(lua, ent->type_flags);
            }
            else
            {
                lua_pushinteger(lua, (ent->type_flags & (uint16_t)(lua_tointeger(lua, 2))));
            }
            return 1;
        }
        else
        {
            Con_Warning("no entity with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("getEntityTypeFlag: expecting arguments (entity_id, (type_flag))");
    }

    return 0;
}


int lua_SetEntityTypeFlag(lua_State *lua)
{
    int top = lua_gettop(lua);

    if(top >= 2)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            if(top == 2)
            {
                ent->type_flags ^= (uint16_t)lua_tointeger(lua, 2);
            }
            else
            {
                if(lua_tointeger(lua, 3) == 1)
                {
                    ent->type_flags |=  (uint16_t)lua_tointeger(lua, 2);
                }
                else
                {
                    ent->type_flags &= ~(uint16_t)lua_tointeger(lua, 2);
                }
            }
        }
        else
        {
            Con_Warning("no entity with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("setEntityTypeFlag: expecting arguments (entity_id, type_flag, (value))");
    }

    return 0;
}


int lua_GetEntityStateFlag(lua_State *lua)
{
    int top = lua_gettop(lua);

    if(top >= 1)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            if(top == 1)
            {
                lua_pushinteger(lua, ent->state_flags);
            }
            else
            {
                lua_pushinteger(lua, (ent->state_flags & (uint16_t)(lua_tointeger(lua, 2))));
            }
            return 1;
        }
        else
        {
            Con_Warning("no entity with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("getEntityStateFlag: expecting arguments (entity_id, (state_flag))");
    }

    return 0;
}


int lua_SetEntityStateFlag(lua_State *lua)
{
    int top = lua_gettop(lua);

    if(top >= 2)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            if(top == 2)
            {
                ent->state_flags ^= (uint16_t)lua_tointeger(lua, 2);
            }
            else
            {
                if(lua_tointeger(lua, 3) == 1)
                {
                    ent->state_flags |=  (uint16_t)lua_tointeger(lua, 2);
                }
                else
                {
                    ent->state_flags &= ~(uint16_t)lua_tointeger(lua, 2);
                }
            }
        }
        else
        {
            Con_Warning("no entity with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("setEntityStateFlag: expecting arguments (entity_id, state_flag, (value))");
    }

    return 0;
}


int lua_GetEntityCallbackFlag(lua_State *lua)
{
    int top = lua_gettop(lua);

    if(top >= 1)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            if(top == 1)
            {
                lua_pushinteger(lua, ent->callback_flags);
            }
            else
            {
                lua_pushinteger(lua, (ent->callback_flags & (uint32_t)(lua_tointeger(lua, 2))));
            }
            return 1;
        }
        else
        {
            Con_Warning("no entity with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("getEntityCallbackFlag: expecting arguments (entity_id, (callback_flag))");
    }

    return 0;
}


int lua_SetEntityCallbackFlag(lua_State *lua)
{
    int top = lua_gettop(lua);

    if(top >= 2)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            if(top == 2)
            {
                ent->callback_flags ^= (uint32_t)lua_tointeger(lua, 2);
            }
            else
            {
                if(lua_tointeger(lua, 3) == 1)
                {
                    ent->callback_flags |=  (uint16_t)lua_tointeger(lua, 2);
                }
                else
                {
                    ent->callback_flags &= ~(uint32_t)lua_tointeger(lua, 2);
                }
            }
        }
        else
        {
            Con_Warning("no entity with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("setEntityCallbackFlag: expecting arguments (entity_id, callback_flag, (value))");
    }

    return 0;
}


int lua_GetEntityTimer(lua_State * lua)
{
    if(lua_gettop(lua) >= 1)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            lua_pushnumber(lua, ent->timer);
            return 1;
        }
    }

    return 0;
}


int lua_SetEntityTimer(lua_State * lua)
{
    if(lua_gettop(lua) >= 2)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            ent->timer = lua_tonumber(lua, 2);
        }
    }

    return 0;
}


int lua_GetEntityMoveType(lua_State * lua)
{
    if(lua_gettop(lua) >= 1)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            lua_pushinteger(lua, ent->move_type);
            return 1;
        }
        else
        {
            Con_Warning("no entity with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("getEntityMoveType: expecting arguments (entity_id)");
    }

    return 0;
}


int lua_SetEntityMoveType(lua_State * lua)
{
    if(lua_gettop(lua) >= 2)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            ent->move_type = lua_tointeger(lua, 2);
        }
    }
    else
    {
        Con_Warning("setEntityMoveType: expecting arguments (entity_id, move_type)");
    }

    return 0;
}


int lua_SetEntityRoomMove(lua_State * lua)
{
    if(lua_gettop(lua) >= 4)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            room_p room = NULL;
            if(!lua_isnil(lua, 2) && (room = World_GetRoomByID(lua_tointeger(lua, 2))))
            {
                if(ent == World_GetPlayer())
                {
                    ent->self->room = room;
                }
                else if(ent->self->room != room)
                {
                    if(ent->self->room != NULL)
                    {
                        Room_RemoveObject(ent->self->room, ent->self);
                    }
                    Room_AddObject(room, ent->self);
                }
            }
            Entity_UpdateRoomPos(ent);

            if(!lua_isnil(lua, 3))
            {
                ent->move_type = lua_tointeger(lua, 3);
            }
            if(!lua_isnil(lua, 4))
            {
                ent->dir_flag = lua_tointeger(lua, 4);
            }
        }
        else
        {
            Con_Warning("no entity with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("setEntityRoomMove: expecting arguments (entity_id, room_id, move_type, dir_flag)");
    }

    return 0;
}

/*
 * physics routine
 */
int lua_CreateGhosts(lua_State * lua)
{
    if(lua_gettop(lua) >= 1)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent && !Physics_IsGhostsInited(ent->physics))
        {
            Physics_CreateGhosts(ent->physics, ent->bf, NULL);
        }
        else
        {
            Con_Warning("no entity with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("createGhosts: expecting arguments (entity_id)");
    }

    return 0;
}


int lua_GetEntityGlobalMove(lua_State * lua)
{
    if(lua_gettop(lua) >= 4)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            float move[3], gmove[3];
            move[0] = lua_tonumber(lua, 2);
            move[1] = lua_tonumber(lua, 3);
            move[2] = lua_tonumber(lua, 4);

            Mat4_vec3_rot_macro(gmove, ent->transform, move);

            lua_pushnumber(lua, gmove[0]);
            lua_pushnumber(lua, gmove[1]);
            lua_pushnumber(lua, gmove[2]);
            return 3;
        }
        else
        {
            Con_Warning("no entity with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("getEntityGlobalMove: expecting arguments (entity_id, dx, dy, dz)");
    }

    return 0;
}


int lua_GetEntityCollisionFix(lua_State * lua)
{
    if(lua_gettop(lua) >= 2)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            int16_t filter = lua_tointeger(lua, 2);
            float reaction[3] = {0.0f, 0.0f, 0.0f};
            Entity_GetPenetrationFixVector(ent, reaction, filter);

            bool result = (reaction[0] != 0.0f) || (reaction[1] != 0.0f) || (reaction[2] != 0.0f);
            lua_pushboolean(lua, result);
            lua_pushnumber(lua, reaction[0]);
            lua_pushnumber(lua, reaction[1]);
            lua_pushnumber(lua, reaction[2]);
            return 4;
        }
        else
        {
            Con_Warning("no entity with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("getEntityCollisionFix: expecting arguments (entity_id, filter)");
    }

    return 0;
}


int lua_GetEntityMoveCollisionFix(lua_State * lua)
{
    if(lua_gettop(lua) >= 5)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            int16_t filter = lua_tointeger(lua, 2);
            float reaction[3] = {0.0f, 0.0f, 0.0f};
            float move[3];
            move[0] = lua_tonumber(lua, 3);
            move[1] = lua_tonumber(lua, 4);
            move[2] = lua_tonumber(lua, 5);

            Entity_CheckNextPenetration(ent, move, reaction, filter);

            bool result = (reaction[0] != 0.0f) || (reaction[1] != 0.0f) || (reaction[2] != 0.0f);
            lua_pushboolean(lua, result);
            lua_pushnumber(lua, reaction[0]);
            lua_pushnumber(lua, reaction[1]);
            lua_pushnumber(lua, reaction[2]);
            return 4;
        }
        else
        {
            Con_Warning("no entity with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("getEntityMoveCollisionFix: expecting arguments (entity_id, filter, dx, dy, dz)");
    }

    return 0;
}


int lua_DropEntity(lua_State * lua)
{
    int top = lua_gettop(lua);

    if(top >= 2)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            bool only_room = (top > 2) ? (lua_toboolean(lua, 3)) : (false);
            int16_t filter = ((only_room) ? (COLLISION_GROUP_STATIC_ROOM) : ent->self->collision_mask);
            float move[3], g[3], t, time = lua_tonumber(lua, 2);
            float from[3], to[3];
            collision_result_t cb;

            Physics_GetGravity(g);
            vec3_mul_scalar(move, ent->speed, time);
            t = 0.5 * time * time;
            vec3_add_mul(move, move, g, t);
            ent->speed[0] += g[0] * time;
            ent->speed[1] += g[1] * time;
            ent->speed[2] += g[2] * time;

            Mat4_vec3_mul_macro(from, ent->transform, ent->bf->centre);
            vec3_add(to, from, move);
            to[2] -= (ent->bf->bb_max[2] - ent->bf->bb_min[2]);

            if(Physics_RayTest(&cb, from, to, ent->self, filter))
            {
                ent->transform[12 + 2] = cb.point[2];
                vec3_set_zero(ent->speed);
                lua_pushboolean(lua, true);
            }
            else
            {
                vec3_add_to(ent->transform + 12, move);
                lua_pushboolean(lua, false);
            }
            Entity_UpdateRigidBody(ent, 1);
            return 1;
        }
        else
        {
            Con_Warning("no entity with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("dropEntity: expecting arguments (entity_id, time, (only_room))");
    }

    return 0;
}


int lua_PushEntityBody(lua_State *lua)
{
    if(lua_gettop(lua) >= 5)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        int body_number = lua_tointeger(lua, 2);
        if(ent && (body_number < ent->bf->bone_tag_count) && (ent->type_flags & ENTITY_TYPE_DYNAMIC))
        {
            float h_force = lua_tonumber(lua, 3);
            float t       = ent->angles[0] * M_PI / 180.0;
            float speed[3];

            speed[0] = -sinf(t) * h_force;
            speed[1] =  cosf(t) * h_force;
            speed[2] =  lua_tonumber(lua, 4);

            Physics_PushBody(ent->physics, speed, body_number);
        }
        else
        {
            Con_Printf("Can't apply force to entity %d - no entity, body, or entity is not kinematic!", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Printf("pushEntityBody: expecting arguments (entity_id, body_number, h_force, v_force, reset_flag)");
    }

    return 0;
}


int lua_SetEntityBodyMass(lua_State *lua)
{
    int top = lua_gettop(lua);

    if(lua_gettop(lua) >= 3)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        int body_number = lua_tointeger(lua, 2);
        body_number = (body_number < 1) ? (1) : (body_number);

        if(ent && (ent->bf->bone_tag_count >= body_number))
        {
            uint16_t argn  = 3;
            bool dynamic = false;
            float mass = 0.0;

            for(int i = 0; i < body_number; i++)
            {
                if(top >= argn) mass = lua_tonumber(lua, argn++);
                if(mass > 0.0) dynamic = true;
                Physics_SetBodyMass(ent->physics, mass, i);
            }
            Entity_UpdateRigidBody(ent, 1);

            if(dynamic)
            {
                ent->type_flags |=  ENTITY_TYPE_DYNAMIC;
            }
            else
            {
                ent->type_flags &= ~ENTITY_TYPE_DYNAMIC;
            }
        }
        else
        {
            Con_Printf("Can't find entity %d or body number is more than %d", lua_tointeger(lua, 1), body_number);
        }
    }
    else
    {
        Con_Printf("setEntityBodyMass: expecting arguments (entity_id, body_number, (mass / each body mass))");
    }

    return 0;
}


int lua_LockEntityBodyLinearFactor(lua_State *lua)
{
    int top = lua_gettop(lua);

    if(top >= 2)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        int body_number = lua_tointeger(lua, 2);
        if(ent && (body_number < ent->bf->bone_tag_count) && (ent->type_flags & ENTITY_TYPE_DYNAMIC))
        {
            float factor[3], t    = ent->angles[0] * M_PI / 180.0;
            factor[0] = fabs(sinf(t));
            factor[1] = fabs(cosf(t));
            factor[2] = 1.0;

            if(top >= 3)
            {
                factor[2] = fabs(lua_tonumber(lua, 3));
                factor[2] = (factor[2] > 1.0) ? (1.0) : (factor[2]);
            }
            Physics_SetLinearFactor(ent->physics, factor, body_number);
        }
        else
        {
            Con_Printf("Can't apply force to entity %d - no entity, body, or entity is not dynamic!", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Printf("lockEntityBodyLinearFactor: expecting arguments (entity_id, body_number, (vertical_factor))");
    }

    return 0;
}


void Script_LuaRegisterEntityFuncs(lua_State *lua)
{
    lua_register(lua, "canTriggerEntity", lua_CanTriggerEntity);
    lua_register(lua, "entityRotateToTriggerZ", lua_EntityRotateToTriggerZ);
    lua_register(lua, "entityRotateToTrigger", lua_EntityRotateToTrigger);
    lua_register(lua, "entityMoveToTriggerActivationPoint", lua_EntityMoveToTriggerActivationPoint);
    lua_register(lua, "enableEntity", lua_EnableEntity);
    lua_register(lua, "disableEntity", lua_DisableEntity);

    lua_register(lua, "activateEntity", lua_ActivateEntity);
    lua_register(lua, "deactivateEntity", lua_DeactivateEntity);
    lua_register(lua, "noFixEntityCollision", lua_NoFixEntityCollision);

    lua_register(lua, "moveEntityGlobal", lua_MoveEntityGlobal);
    lua_register(lua, "moveEntityLocal", lua_MoveEntityLocal);
    lua_register(lua, "moveEntityToSink", lua_MoveEntityToSink);
    lua_register(lua, "moveEntityToEntity", lua_MoveEntityToEntity);
    lua_register(lua, "rotateEntity", lua_RotateEntity);

    lua_register(lua, "getEntityVector", lua_GetEntityVector);
    lua_register(lua, "getEntityDirDot", lua_GetEntityDirDot);
    lua_register(lua, "getEntityDistance", lua_GetEntityDistance);
    lua_register(lua, "getEntityPos", lua_GetEntityPosition);
    lua_register(lua, "setEntityPos", lua_SetEntityPosition);
    lua_register(lua, "getEntityAngles", lua_GetEntityAngles);
    lua_register(lua, "setEntityAngles", lua_SetEntityAngles);
    lua_register(lua, "getEntityScaling", lua_GetEntityScaling);
    lua_register(lua, "setEntityScaling", lua_SetEntityScaling);
    lua_register(lua, "getEntitySpeed", lua_GetEntitySpeed);
    lua_register(lua, "setEntitySpeed", lua_SetEntitySpeed);
    lua_register(lua, "setEntityLinearSpeed", lua_SetEntityLinearSpeed);
    lua_register(lua, "getEntitySpeedLinear", lua_GetEntitySpeedLinear);
    lua_register(lua, "setEntityCollision", lua_SetEntityCollision);
    lua_register(lua, "setEntityGhostCollisionShape", lua_SetEntityGhostCollisionShape);
    lua_register(lua, "setEntityCollisionFlags", lua_SetEntityCollisionFlags);
    lua_register(lua, "setEntityBodyPartFlag", lua_SetEntityBodyPartFlag);
    lua_register(lua, "getEntityVisibility", lua_GetEntityVisibility);
    lua_register(lua, "setEntityVisibility", lua_SetEntityVisibility);
    lua_register(lua, "getEntityActivity", lua_GetEntityActivity);
    lua_register(lua, "setEntityActivity", lua_SetEntityActivity);
    lua_register(lua, "getEntityEnability", lua_GetEntityEnability);
    lua_register(lua, "getEntityOCB", lua_GetEntityOCB);
    lua_register(lua, "setEntityOCB", lua_SetEntityOCB);
    lua_register(lua, "getEntityTimer", lua_GetEntityTimer);
    lua_register(lua, "setEntityTimer", lua_SetEntityTimer);
    lua_register(lua, "getEntityFlags", lua_GetEntityFlags);
    lua_register(lua, "setEntityFlags", lua_SetEntityFlags);
    lua_register(lua, "getEntityTypeFlag", lua_GetEntityTypeFlag);
    lua_register(lua, "setEntityTypeFlag", lua_SetEntityTypeFlag);
    lua_register(lua, "getEntityStateFlag", lua_GetEntityStateFlag);
    lua_register(lua, "setEntityStateFlag", lua_SetEntityStateFlag);
    lua_register(lua, "getEntityCallbackFlag", lua_GetEntityCallbackFlag);
    lua_register(lua, "setEntityCallbackFlag", lua_SetEntityCallbackFlag);
    lua_register(lua, "setEntityRoomMove", lua_SetEntityRoomMove);
    lua_register(lua, "getEntityMoveType", lua_GetEntityMoveType);
    lua_register(lua, "setEntityMoveType", lua_SetEntityMoveType);

    lua_register(lua, "getEntityTriggerLayout", lua_GetEntityTriggerLayout);
    lua_register(lua, "setEntityTriggerLayout", lua_SetEntityTriggerLayout);
    lua_register(lua, "getEntityMask", lua_GetEntityMask);
    lua_register(lua, "setEntityMask", lua_SetEntityMask);
    lua_register(lua, "getEntityEvent", lua_GetEntityEvent);
    lua_register(lua, "setEntityEvent", lua_SetEntityEvent);
    lua_register(lua, "getEntityLock", lua_GetEntityLock);
    lua_register(lua, "setEntityLock", lua_SetEntityLock);
    lua_register(lua, "getEntitySectorStatus", lua_GetEntitySectorStatus);
    lua_register(lua, "setEntitySectorStatus", lua_SetEntitySectorStatus);

    lua_register(lua, "getEntityActivationOffset", lua_GetEntityActivationOffset);
    lua_register(lua, "setEntityActivationOffset", lua_SetEntityActivationOffset);
    lua_register(lua, "getEntityActivationDirection", lua_GetEntityActivationDirection);
    lua_register(lua, "setEntityActivationDirection", lua_SetEntityActivationDirection);
    lua_register(lua, "getEntitySectorIndex", lua_GetEntitySectorIndex);
    lua_register(lua, "getEntitySectorFlags", lua_GetEntitySectorFlags);
    lua_register(lua, "getEntitySectorMaterial", lua_GetEntitySectorMaterial);

    lua_register(lua, "createGhosts", lua_CreateGhosts);
    lua_register(lua, "getEntityGlobalMove", lua_GetEntityGlobalMove);
    lua_register(lua, "getEntityCollisionFix", lua_GetEntityCollisionFix);
    lua_register(lua, "getEntityMoveCollisionFix", lua_GetEntityMoveCollisionFix);
    lua_register(lua, "dropEntity", lua_DropEntity);
    lua_register(lua, "setEntityBodyMass", lua_SetEntityBodyMass);
    lua_register(lua, "pushEntityBody", lua_PushEntityBody);
    lua_register(lua, "lockEntityBodyLinearFactor", lua_LockEntityBodyLinearFactor);
}
