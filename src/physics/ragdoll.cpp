
#include <stdlib.h>
#include <string.h>

#include "../core/lua.h"
#include "../core/vmath.h"
#include "ragdoll.h"
#include "../mesh.h"
#include "../skeletal_model.h"
#include "../character_controller.h"

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


struct rd_setup_s *Ragdoll_AutoCreateSetup(struct skeletal_model_s *model, uint16_t anim, uint16_t frame)
{
    struct rd_setup_s *setup = NULL;
    if(model && (anim < model->animation_count) && (frame < model->animations[anim].frames_count))
    {
        bone_frame_p bf = model->animations[anim].frames + frame;
        rd_joint_setup_p js;
        float tr[16], t;
        setup = (rd_setup_p)malloc(sizeof(rd_setup_t));

        setup->body_count  = model->mesh_count;
        setup->joint_count = model->mesh_count - 1;
        setup->body_setup  = (rd_body_setup_p)calloc(setup->body_count, sizeof(rd_body_setup_t));
        setup->joint_setup = (rd_joint_setup_p)calloc(setup->joint_count, sizeof(rd_joint_setup_t));
        setup->hit_func = NULL;
        setup->joint_cfm = 0.7f;
        setup->joint_erp = 0.1f;

        for(uint32_t i = 0; i < setup->body_count; i++)
        {
            base_mesh_p mesh = model->mesh_tree[i].mesh_base;
            t  = (mesh->bb_max[0] - mesh->bb_min[0]);
            t *= (mesh->bb_max[1] - mesh->bb_min[1]);
            t *= (mesh->bb_max[2] - mesh->bb_min[2]);
            setup->body_setup[i].mass = t / 1024.0f;
            setup->body_setup[i].restitution = 0.5f;
            setup->body_setup[i].friction = 5.0f;
            setup->body_setup[i].damping[0] = 0.6f;
            setup->body_setup[i].damping[1] = 0.6f;
        }

        js = setup->joint_setup;
        uint32_t part = 0x0000;
        for(uint32_t i = 0; i < setup->body_count; i++)
        {
            if(model->mesh_tree[i].parent != i)
            {
                bone_tag_p b_tag = bf->bone_tags + i;
                part = (model->mesh_tree[i].body_part) ? (model->mesh_tree[i].body_part) : (part);
                js->body_index = i;
                js->body1_offset[0] = 0.0f;
                js->body1_offset[1] = 0.0f;
                js->body1_offset[2] = 0.0f;
                js->body2_offset[0] = b_tag->offset[0];
                js->body2_offset[1] = b_tag->offset[1];
                js->body2_offset[2] = b_tag->offset[2];
                js->body1_angle[0] = 0.0f;
                js->body1_angle[1] = 0.0f;
                js->body1_angle[2] = 0.0f;
                Mat4_E_macro(tr);
                Mat4_RotateQuaternion(tr, b_tag->qrotate);
                Mat4_GetAnglesZXY(js->body2_angle, tr);
                SWAPT(js->body2_angle[1], js->body2_angle[2], t);
                
                if(part & BODY_PART_HEAD)
                {
                    js->joint_type = RD_CONSTRAINT_CONE;
                    js->joint_limit[0] = M_PI * 0.25f;
                    js->joint_limit[1] = M_PI * 0.25f;
                    js->joint_limit[2] = M_PI * 0.25f;
                }
                else if(part & BODY_PART_BODY_UPPER)
                {
                    js->joint_type = RD_CONSTRAINT_HINGE;
                    js->joint_limit[0] = - M_PI * 0.25f;
                    js->joint_limit[1] =   M_PI * 0.5f;
                    js->joint_limit[2] =   M_PI * 0.0f;
                }
                else if(part & (BODY_PART_LEFT_HAND_1 | BODY_PART_RIGHT_HAND_1))
                {
                    js->joint_type = RD_CONSTRAINT_CONE;
                    js->joint_limit[0] = M_PI * 0.5f;
                    js->joint_limit[1] = M_PI * 0.25f;
                    js->joint_limit[2] = M_PI * 0.15f;
                }
                else if(part & (BODY_PART_LEFT_HAND_2 | BODY_PART_RIGHT_HAND_2))
                {
                    js->joint_type = RD_CONSTRAINT_HINGE;
                    js->joint_limit[0] = - M_PI * 0.25;
                    js->joint_limit[1] =   M_PI * 0.0f;
                    js->joint_limit[2] =   M_PI * 0.0f;
                }
                else if(part & (BODY_PART_LEFT_HAND_3 | BODY_PART_RIGHT_HAND_3))
                {
                    js->joint_type = RD_CONSTRAINT_HINGE;
                    js->joint_limit[0] = - M_PI * 0.25;
                    js->joint_limit[1] =   M_PI * 0.25f;
                    js->joint_limit[2] =   M_PI * 0.0f;
                }
                else if(part & (BODY_PART_LEFT_LEG_1 | BODY_PART_RIGHT_LEG_1))
                {
                    js->joint_type = RD_CONSTRAINT_CONE;
                    js->joint_limit[0] = M_PI * 0.5f;
                    js->joint_limit[1] = M_PI * 0.25f;
                    js->joint_limit[2] = M_PI * 0.0f;
                }
                else if(part & (BODY_PART_LEFT_LEG_2 | BODY_PART_RIGHT_LEG_2))
                {
                    js->joint_type = RD_CONSTRAINT_HINGE;
                    js->joint_limit[0] = M_PI * 0.0f;
                    js->joint_limit[1] = M_PI * 0.5f;
                    js->joint_limit[2] = M_PI * 0.0f;
                }
                else if(part & (BODY_PART_LEFT_LEG_3 | BODY_PART_RIGHT_LEG_3))
                {
                    js->joint_type = RD_CONSTRAINT_HINGE;
                    js->joint_limit[0] = M_PI * 0.0f;
                    js->joint_limit[1] = M_PI * 0.25f;
                    js->joint_limit[2] = M_PI * 0.0f;
                }
                else
                {
                    js->joint_type = RD_CONSTRAINT_CONE;
                    js->joint_limit[0] = 0.2f;
                    js->joint_limit[1] = 0.2f;
                    js->joint_limit[2] = 0.0f;
                }
                js++;
            }
        }
    }

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
