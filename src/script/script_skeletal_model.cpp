
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
#include "../skeletal_model.h"
#include "../entity.h"
#include "../world.h"


int lua_SetStateChangeRange(lua_State * lua)
{
    int top = lua_gettop(lua);

    if(top < 6)
    {
        Con_Warning("setStateChangeRange: expecting arguments (model_id, anim_num, state_id, dispatch_num, start_frame, end_frame, (next_anim), (next_frame))");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    skeletal_model_p model = World_GetModelByID(id);

    if(model == NULL)
    {
        Con_Warning("no skeletal model with id = %d", id);
        return 0;
    }

    int anim = lua_tointeger(lua, 2);
    int state = lua_tointeger(lua, 3);
    int dispatch = lua_tointeger(lua, 4);
    int frame_low = lua_tointeger(lua, 5);
    int frame_high = lua_tointeger(lua, 6);

    if((anim < 0) || (anim + 1 > model->animation_count))
    {
        Con_Warning("wrong anim number = %d", anim);
        return 0;
    }

    animation_frame_p af = model->animations + anim;
    for(uint16_t i = 0; i < af->state_change_count; i++)
    {
        if(af->state_change[i].id == (uint32_t)state)
        {
            if((dispatch >= 0) && (dispatch < af->state_change[i].anim_dispatch_count))
            {
                af->state_change[i].anim_dispatch[dispatch].frame_low = frame_low;
                af->state_change[i].anim_dispatch[dispatch].frame_high = frame_high;
                if(top >= 8)
                {
                    af->state_change[i].anim_dispatch[dispatch].next_anim = lua_tointeger(lua, 7);
                    af->state_change[i].anim_dispatch[dispatch].next_frame = lua_tointeger(lua, 8);
                }
            }
            else
            {
                Con_Warning("wrong anim dispatch number = %d", dispatch);
            }
            break;
        }
    }

    return 0;
}


int lua_GetEntityModelID(lua_State * lua)
{
    if(lua_gettop(lua) >= 1)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent && ent->bf->animations.model)
        {
            lua_pushinteger(lua, ent->bf->animations.model->id);
            return 1;
        }
    }
    return 0;
}


int lua_GetEntityAnimState(lua_State * lua)
{
    if(lua_gettop(lua) < 2)
    {
        Con_Warning("getEntityAnimState: expecting arguments (entity_id, anim_type_id)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent == NULL)
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }

    int anim_type_id = lua_tointeger(lua, 2);
    for(ss_animation_p ss_anim = &ent->bf->animations; ss_anim; ss_anim = ss_anim->next)
    {
        if(ss_anim->type == anim_type_id)
        {
            lua_pushinteger(lua, ss_anim->next_state);
            return 1;
        }
    }

    return 0;
}


/*
 * Base engine functions
 */
int lua_SetModelCollisionMap(lua_State * lua)
{
    if(lua_gettop(lua) < 3)
    {
        Con_Warning("wrong arguments number, shoul be (model_id, map_index, value)");
        return 0;
    }

    skeletal_model_p model = World_GetModelByID(lua_tointeger(lua, 1));
    if(model == NULL)
    {
        Con_Warning("wrong model id = %d", lua_tointeger(lua, 1));
        return 0;
    }

    int arg = lua_tointeger(lua, 2);
    int val = lua_tointeger(lua, 3);
    if((arg >= 0) && (arg < model->mesh_count) &&
       (val >= 0) && (val < model->mesh_count))
    {
        model->collision_map[arg] = val;
    }

    return 0;
}


int lua_SetEntityAnimState(lua_State * lua)
{
    int top = lua_gettop(lua);
    if(top < 3)
    {
        Con_Warning("setEntityAnimState: expecting arguments (entity_id, anim_type_id, next_state)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent == NULL)
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }

    int anim_type_id = lua_tointeger(lua, 2);
    for(ss_animation_p ss_anim = &ent->bf->animations; ss_anim; ss_anim = ss_anim->next)
    {
        if(ss_anim->type == anim_type_id)
        {
            ss_anim->next_state = lua_tointeger(lua, 3);
            break;
        }
    }

    return 0;
}


int lua_SetEntityAnimStateHeavy(lua_State * lua)
{
    int top = lua_gettop(lua);
    if(top < 3)
    {
        Con_Warning("setEntityAnimStateHeavy: expecting arguments (entity_id, anim_type_id, next_state)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent == NULL)
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }

    int anim_type_id = lua_tointeger(lua, 2);
    for(ss_animation_p ss_anim = &ent->bf->animations; ss_anim; ss_anim = ss_anim->next)
    {
        if(ss_anim->type == anim_type_id)
        {
            ss_anim->next_state_heavy = lua_tointeger(lua, 3);
            ss_anim->next_state = ss_anim->next_state_heavy;
            break;
        }
    }

    return 0;
}


int lua_GetEntityMeshCount(lua_State *lua)
{
    if(lua_gettop(lua) < 1)
    {
        Con_Warning("getEntityMeshCount: expecting arguments (entity_id)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent == NULL)
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }

    lua_pushinteger(lua, ent->bf->bone_tag_count);
    return 1;
}


int lua_SetEntityMeshswap(lua_State * lua)
{
    if(lua_gettop(lua) < 2)
    {
        Con_Warning("setEntityMeshswap: expecting arguments (id_dest, id_src)");
        return 0;
    }

    int id_dest = lua_tointeger(lua, 1);
    int id_src = lua_tointeger(lua, 2);

    entity_p         ent_dest;
    skeletal_model_p model_src;

    ent_dest   = World_GetEntityByID(id_dest);
    model_src  = World_GetModelByID(id_src);

    int meshes_to_copy = (ent_dest->bf->bone_tag_count > model_src->mesh_count) ? (model_src->mesh_count) : (ent_dest->bf->bone_tag_count);

    for(int i = 0; i < meshes_to_copy; i++)
    {
        ent_dest->bf->bone_tags[i].mesh_base = model_src->mesh_tree[i].mesh_base;
        ent_dest->bf->bone_tags[i].mesh_skin = model_src->mesh_tree[i].mesh_skin;
    }

    return 0;
}


int lua_SetModelMeshReplaceFlag(lua_State *lua)
{
    if(lua_gettop(lua) < 3)
    {
        Con_Printf("setModelMeshReplaceFlag: Wrong arguments count. Must be (id_model, bone_num, flag)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    skeletal_model_p sm = World_GetModelByID(id);
    if(sm != NULL)
    {
        int bone = lua_tointeger(lua, 2);
        if((bone >= 0) && (bone < sm->mesh_count))
        {
            sm->mesh_tree[bone].replace_mesh = lua_tointeger(lua, 3);
        }
        else
        {
            Con_Printf("wrong bone number = %d", bone);
        }
    }
    else
    {
        Con_Printf("can not find model with id = %d", id);
    }

    return 0;
}


int lua_SetEntityAnimFlag(lua_State * lua)
{
    int top = lua_gettop(lua);

    if(top < 3)
    {
        Con_Warning("setEntityAnimFlag: expecting arguments (entity_id, anim_type_id, anim_flag)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent == NULL)
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }

    int anim_type_id = lua_tointeger(lua, 2);
    for(ss_animation_p  ss_anim_it = &ent->bf->animations; ss_anim_it; ss_anim_it = ss_anim_it->next)
    {
        if(ss_anim_it->type == anim_type_id)
        {
            ss_anim_it->anim_frame_flags = lua_tointeger(lua, 3);
            break;
        }
    }

    return 0;
}


int lua_SetModelAnimReplaceFlag(lua_State *lua)
{
    if(lua_gettop(lua) < 3)
    {
        Con_Printf("setModelAnimReplaceFlag: Wrong arguments count. Must be (id_model, bone_num, flag)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    skeletal_model_p sm = World_GetModelByID(id);
    if(sm != NULL)
    {
        int bone = lua_tointeger(lua, 2);
        if((bone >= 0) && (bone < sm->mesh_count))
        {
            sm->mesh_tree[bone].replace_anim = lua_tointeger(lua, 3);
        }
        else
        {
            Con_Printf("wrong bone number = %d", bone);
        }
    }
    else
    {
        Con_Printf("can not find model with id = %d", id);
    }

    return 0;
}


int lua_CopyMeshFromModelToModel(lua_State *lua)
{
    if(lua_gettop(lua) < 4)
    {
        Con_Printf("copyMeshFromModelToModel: expecting arguments (id_model1, id_model2, bone_num1, bone_num2)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    skeletal_model_p sm1 = World_GetModelByID(id);
    if(sm1 == NULL)
    {
        Con_Printf("can not find model with id = %d", id);
        return 0;
    }

    id = lua_tointeger(lua, 2);
    skeletal_model_p sm2 = World_GetModelByID(id);
    if(sm2 == NULL)
    {
        Con_Printf("can not find model with id = %d", id);
        return 0;
    }

    int bone1 = lua_tointeger(lua, 3);
    int bone2 = lua_tointeger(lua, 4);

    if((bone1 >= 0) && (bone1 < sm1->mesh_count) && (bone2 >= 0) && (bone2 < sm2->mesh_count))
    {
        sm1->mesh_tree[bone1].mesh_base = sm2->mesh_tree[bone2].mesh_base;
    }
    else
    {
        Con_AddLine("wrong bone number = %d", FONTSTYLE_GENERIC);
    }

    return 0;
}


int lua_SetEntityAnim(lua_State * lua)
{
    int top = lua_gettop(lua);

    if(top < 4)
    {
        Con_Warning("setEntityAnim: expecting arguments (entity_id, anim_type_id, anim_num, frame_number, (next_anim, next_frame))");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent == NULL)
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }

    uint16_t anim_type_id = lua_tointeger(lua, 2);
    ss_animation_p ss_anim = SSBoneFrame_GetOverrideAnim(ent->bf, anim_type_id);
    if(ss_anim)
    {
        Anim_SetAnimation(ss_anim, lua_tointeger(lua, 3), lua_tointeger(lua, 4));
        if(top >= 6)
        {
            ss_anim->next_animation = lua_tointeger(lua, 5);
            ss_anim->next_frame = lua_tointeger(lua, 6);
        }
    }

    return 0;
}


int lua_GetEntityAnim(lua_State * lua)
{
    if(lua_gettop(lua) < 2)
    {
        Con_Warning("getEntityAnim: expecting arguments (entity_id, anim_type_id)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent == NULL)
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }

    int anim_id = lua_tointeger(lua, 2);
    ss_animation_p ss_anim = SSBoneFrame_GetOverrideAnim(ent->bf, anim_id);
    if(ss_anim && ss_anim->model)
    {
        animation_frame_p af = ss_anim->model->animations + ss_anim->next_animation;
        lua_pushinteger(lua, ss_anim->next_animation);
        lua_pushinteger(lua, ss_anim->next_frame);
        lua_pushinteger(lua, af->max_frame);
        lua_pushinteger(lua, af->next_anim->id);
        lua_pushinteger(lua, af->next_frame);
        lua_pushinteger(lua, af->next_anim->state_id);
        return 6;
    }

    return 0;
}


int lua_EntitySSAnimEnsureExists(lua_State * lua)
{
    if(lua_gettop(lua) < 1)
    {
        Con_Warning("entitySSAnimEnsureExists: expecting arguments (entity_id, anim_type_id, model_id)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent == NULL)
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }

    int anim_type_id = lua_tointeger(lua, 2);
    if(!SSBoneFrame_GetOverrideAnim(ent->bf, anim_type_id))
    {
        if(!lua_isnil(lua, 3))
        {
            SSBoneFrame_AddOverrideAnim(ent->bf, World_GetModelByID(lua_tointeger(lua, 3)), anim_type_id);
        }
        else
        {
            SSBoneFrame_AddOverrideAnim(ent->bf, NULL, anim_type_id);
        }
    }

    return 0;
}


int lua_EntitySSAnimSetTarget(lua_State * lua)
{
    if(lua_gettop(lua) < 9)
    {
        Con_Warning("entitySSAnimSetTarget: expecting arguments (entity_id, anim_type_id, targeted_bone, target_x, target_y, target_z, bone_dir_x, bone_dir_y, bone_dir_z)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent == NULL)
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }

    int anim_type_id = lua_tointeger(lua, 2);
    ss_animation_p ss_anim = SSBoneFrame_GetOverrideAnim(ent->bf, anim_type_id);
    if(ss_anim)
    {
        float pos[3], dir[3];
        uint16_t targeted_bone = lua_tointeger(lua, 3);
        pos[0] = lua_tonumber(lua, 4);
        pos[1] = lua_tonumber(lua, 5);
        pos[2] = lua_tonumber(lua, 6);
        dir[0] = lua_tonumber(lua, 7);
        dir[1] = lua_tonumber(lua, 8);
        dir[2] = lua_tonumber(lua, 9);

        SSBoneFrame_SetTrget(ss_anim, targeted_bone, pos, dir);
    }

    return 0;
}


int lua_EntitySSAnimSetAxisMod(lua_State * lua)
{
    if(lua_gettop(lua) < 5)
    {
        Con_Warning("entitySSAnimSetAxisMod: expecting arguments (entity_id, anim_type_id, mod_x, mod_y, mod_z)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent == NULL)
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }

    int anim_type_id = lua_tointeger(lua, 2);
    ss_animation_p ss_anim = SSBoneFrame_GetOverrideAnim(ent->bf, anim_type_id);
    if(ss_anim)
    {
        float mod[3];
        mod[0] = lua_tonumber(lua, 3);
        mod[1] = lua_tonumber(lua, 4);
        mod[2] = lua_tonumber(lua, 5);
        SSBoneFrame_SetTargetingAxisMod(ss_anim, mod);
    }

    return 0;
}


int lua_EntitySSAnimSetTargetingLimit(lua_State * lua)
{
    if(lua_gettop(lua) < 6)
    {
        Con_Warning("entitySSAnimSetTargetingLimit: expecting arguments (entity_id, anim_type_id, q_x, q_y, q_z, q_w)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent == NULL)
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }

    int anim_type_id = lua_tointeger(lua, 2);
    ss_animation_p ss_anim = SSBoneFrame_GetOverrideAnim(ent->bf, anim_type_id);
    if(ss_anim)
    {
        float q[4];
        q[0] = lua_tonumber(lua, 3);
        q[1] = lua_tonumber(lua, 4);
        q[2] = lua_tonumber(lua, 5);
        q[3] = lua_tonumber(lua, 6);
        SSBoneFrame_SetTargetingLimit(ss_anim, q);
    }

    return 0;
}


int lua_EntitySSAnimSetCurrentRotation(lua_State * lua)
{
    if(lua_gettop(lua) < 6)
    {
        Con_Warning("entitySSAnimSetCurrentRotation: expecting arguments (entity_id, anim_type_id, q_x, q_y, q_z, q_w)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent == NULL)
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }

    int anim_type_id = lua_tointeger(lua, 2);
    ss_animation_p ss_anim = SSBoneFrame_GetOverrideAnim(ent->bf, anim_type_id);
    if(ss_anim)
    {
        float q[4];
        q[0] = lua_tonumber(lua, 3);
        q[1] = lua_tonumber(lua, 4);
        q[2] = lua_tonumber(lua, 5);
        q[3] = lua_tonumber(lua, 6);
        vec4_copy(ss_anim->current_mod, q);
    }

    return 0;
}


int lua_EntitySSAnimSetExtFlags(lua_State * lua)
{
    if(lua_gettop(lua) < 5)
    {
        Con_Warning("entitySSAnimSetCurrentRotation: expecting arguments (entity_id, anim_type_id, enabled, anim_ext_flags, anim_targeting_flags)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent == NULL)
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }

    int anim_type_id = lua_tointeger(lua, 2);
    ss_animation_p ss_anim = SSBoneFrame_GetOverrideAnim(ent->bf, anim_type_id);
    if(ss_anim)
    {
        ss_anim->enabled = 0x01 & (lua_tointeger(lua, 3));
        ss_anim->anim_ext_flags = lua_tointeger(lua, 4);
        ss_anim->targeting_flags = lua_tointeger(lua, 5);
    }

    return 0;
}


int lua_EntitySSAnimSetEnable(lua_State * lua)
{
    if(lua_gettop(lua) < 3)
    {
        Con_Warning("entitySSAnimEnable: expecting arguments (entity_id, anim_type_id, enabled)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent == NULL)
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }

    int anim_type_id = lua_tointeger(lua, 2);
    if(lua_tointeger(lua, 3))
    {
        SSBoneFrame_EnableOverrideAnimByType(ent->bf, anim_type_id);
    }
    else
    {
        SSBoneFrame_DisableOverrideAnim(ent->bf, anim_type_id);
    }
    return 0;
}


void Script_LuaRegisterAnimFuncs(lua_State *lua)
{
    lua_register(lua, "setModelCollisionMap", lua_SetModelCollisionMap);
    lua_register(lua, "setStateChangeRange", lua_SetStateChangeRange);

    lua_register(lua, "getEntityModelID", lua_GetEntityModelID);
    lua_register(lua, "getEntityAnimState", lua_GetEntityAnimState);
    lua_register(lua, "setEntityAnimState", lua_SetEntityAnimState);
    lua_register(lua, "setEntityAnimStateHeavy", lua_SetEntityAnimStateHeavy);
    lua_register(lua, "getEntityMeshCount", lua_GetEntityMeshCount);
    lua_register(lua, "setEntityMeshswap", lua_SetEntityMeshswap);
    lua_register(lua, "setModelMeshReplaceFlag", lua_SetModelMeshReplaceFlag);
    lua_register(lua, "setEntityAnimFlag", lua_SetEntityAnimFlag);
    lua_register(lua, "setModelAnimReplaceFlag", lua_SetModelAnimReplaceFlag);
    lua_register(lua, "copyMeshFromModelToModel", lua_CopyMeshFromModelToModel);

    lua_register(lua, "getEntityAnim", lua_GetEntityAnim);
    lua_register(lua, "setEntityAnim", lua_SetEntityAnim);
    lua_register(lua, "entitySSAnimEnsureExists", lua_EntitySSAnimEnsureExists);
    lua_register(lua, "entitySSAnimSetTarget", lua_EntitySSAnimSetTarget);
    lua_register(lua, "entitySSAnimSetAxisMod", lua_EntitySSAnimSetAxisMod);
    lua_register(lua, "entitySSAnimSetTargetingLimit", lua_EntitySSAnimSetTargetingLimit);
    lua_register(lua, "entitySSAnimSetCurrentRotation", lua_EntitySSAnimSetCurrentRotation);
    lua_register(lua, "entitySSAnimSetExtFlags", lua_EntitySSAnimSetExtFlags);
    lua_register(lua, "entitySSAnimSetEnable", lua_EntitySSAnimSetEnable);
}
