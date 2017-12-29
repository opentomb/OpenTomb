
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

    if(top >= 6)
    {
        skeletal_model_p model = World_GetModelByID(lua_tointeger(lua, 1));
        if(model)
        {
            int anim = lua_tointeger(lua, 2);
            if((anim >= 0) && (anim < model->animation_count))
            {
                int state = lua_tointeger(lua, 3);
                animation_frame_p af = model->animations + anim;
                for(uint16_t i = 0; i < af->state_change_count; i++)
                {
                    if(af->state_change[i].id == (uint32_t)state)
                    {
                        int dispatch = lua_tointeger(lua, 4);
                        if((dispatch >= 0) && (dispatch < af->state_change[i].anim_dispatch_count))
                        {
                            af->state_change[i].anim_dispatch[dispatch].frame_low = lua_tointeger(lua, 5);
                            af->state_change[i].anim_dispatch[dispatch].frame_high = lua_tointeger(lua, 6);
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
            }
            else
            {
                Con_Warning("wrong anim number = %d", anim);
            }
        }
        else
        {
            Con_Warning("no skeletal model with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("setStateChangeRange: expecting arguments (model_id, anim_num, state_id, dispatch_num, start_frame, end_frame, (next_anim, next_frame))");
    }

    return 0;
}


int lua_GetEntityModelID(lua_State * lua)
{
    int top = lua_gettop(lua);
    if(top >= 1)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            uint16_t anim_type = (top > 1) ? (lua_tointeger(lua, 2)) : (ANIM_TYPE_BASE);
            ss_animation_p ss_anim = SSBoneFrame_GetOverrideAnim(ent->bf, anim_type);
            if(ss_anim && ss_anim->model)
            {
                lua_pushinteger(lua, ss_anim->model->id);
                return 1;
            }
        }
    }
    return 0;
}


int lua_GetEntityAnimState(lua_State * lua)
{
    if(lua_gettop(lua) >= 2)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            int anim_type_id = lua_tointeger(lua, 2);
            for(ss_animation_p ss_anim = &ent->bf->animations; ss_anim; ss_anim = ss_anim->next)
            {
                if(ss_anim->type == anim_type_id)
                {
                    lua_pushinteger(lua, Anim_GetCurrentState(ss_anim));
                    return 1;
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
        Con_Warning("getEntityAnimState: expecting arguments (entity_id, anim_type_id)");
    }

    return 0;
}


/*
 * Base engine functions
 */
int lua_SetEntityBaseAnimModel(lua_State * lua)
{
    if(lua_gettop(lua) >= 2)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            skeletal_model_p model = World_GetModelByID(lua_tointeger(lua, 2));
            if(model && ent->bf->animations.model && (ent->bf->animations.model->mesh_count == model->mesh_count))
            {
                ent->bf->animations.model = model;
                ent->bf->animations.current_animation = 0;
                ent->bf->animations.current_frame = 0;
                ent->bf->animations.next_animation = 0;
                ent->bf->animations.next_frame = 0;
            }
        }
        else
        {
            Con_Warning("no entity with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("setEntityBaseAnimModel: expecting arguments (entity_id, model_id)");
    }

    return 0;
}


int lua_SetModelCollisionMap(lua_State * lua)
{
    if(lua_gettop(lua) >= 3)
    {
        skeletal_model_p model = World_GetModelByID(lua_tointeger(lua, 1));
        if(model)
        {
            int arg = lua_tointeger(lua, 2);
            int val = lua_tointeger(lua, 3);
            if((arg >= 0) && (arg < model->mesh_count) &&
               (val >= 0) && (val < model->mesh_count))
            {
                model->collision_map[arg] = val;
            }
        }
        else
        {
            Con_Warning("wrong model id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("wrong arguments number, shoul be (model_id, map_index, value)");
    }

    return 0;
}


int lua_SetModelBodyPartFlag(lua_State * lua)
{
    int top = lua_gettop(lua);

    if(top >= 3)
    {
        skeletal_model_p model = World_GetModelByID(lua_tointeger(lua, 1));
        if(model)
        {
            int bone_id = lua_tointeger(lua, 2);
            if((bone_id >= 0) && (bone_id < model->mesh_count))
            {
                model->mesh_tree[bone_id].body_part = lua_tointeger(lua, 3);
            }
            else
            {
                Con_Warning("wrong bone index = %d", bone_id);
            }
        }
        else
        {
            Con_Warning("no skeletal model with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("setModelBodyPartFlag: expecting arguments (model_id, bone_id, body_part_flag)");
    }

    return 0;
}


int lua_CopyModelAnimations(lua_State * lua)
{
    int top = lua_gettop(lua);

    if(top >= 2)
    {
        skeletal_model_p dst = World_GetModelByID(lua_tointeger(lua, 1));
        skeletal_model_p src = World_GetModelByID(lua_tointeger(lua, 2));
        if(src && dst && (src != dst) && (src->mesh_count == dst->mesh_count))
        {
            SkeletalModel_CopyAnims(dst, src);
        }
    }
    else
    {
        Con_Warning("copyModelAnimations: expecting arguments (model_dst_id, model_src_id)");
    }

    return 0;
}


int lua_SetEntityAnimState(lua_State * lua)
{
    if(lua_gettop(lua) >= 3)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            int anim_type_id = lua_tointeger(lua, 2);
            for(ss_animation_p ss_anim = &ent->bf->animations; ss_anim; ss_anim = ss_anim->next)
            {
                if(ss_anim->type == anim_type_id)
                {
                    ss_anim->next_state = lua_tointeger(lua, 3);
                    break;
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
        Con_Warning("setEntityAnimState: expecting arguments (entity_id, anim_type_id, next_state)");
    }

    return 0;
}


int lua_SetEntityAnimStateHeavy(lua_State * lua)
{
    if(lua_gettop(lua) >= 3)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
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
        }
        else
        {
            Con_Warning("no entity with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("setEntityAnimStateHeavy: expecting arguments (entity_id, anim_type_id, next_state)");
    }

    return 0;
}


int lua_GetModelMeshCount(lua_State *lua)
{
    if(lua_gettop(lua) >= 1)
    {
        skeletal_model_p sm = World_GetModelByID(lua_tointeger(lua, 1));
        if(sm)
        {
            lua_pushinteger(lua, sm->mesh_count);
            return 1;
        }
        else
        {
            Con_Warning("no model with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("getModelMeshCount: expecting arguments (model_id)");
    }

    lua_pushinteger(lua, 0);
    return 1;
}


int lua_GetEntityMeshCount(lua_State *lua)
{
    if(lua_gettop(lua) >= 1)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            lua_pushinteger(lua, ent->bf->bone_tag_count);
            return 1;
        }
        else
        {
            Con_Warning("no entity with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("getEntityMeshCount: expecting arguments (entity_id)");
    }

    lua_pushinteger(lua, 0);
    return 1;
}


int lua_SetEntityMeshes(lua_State * lua)
{
    if(lua_gettop(lua) >= 4)
    {
        entity_p         ent_dest = World_GetEntityByID(lua_tointeger(lua, 1));
        skeletal_model_p model_src = World_GetModelByID(lua_tointeger(lua, 2));
        if(ent_dest && model_src)
        {
            int index = lua_tointeger(lua, 3);
            int to = (lua_isnil(lua, 4)) ? (ent_dest->bf->bone_tag_count) : (lua_tointeger(lua, 4));
            index = (index >= 0) ? (index) : (0);
            to = (to <= ent_dest->bf->bone_tag_count) ? (to) : (ent_dest->bf->bone_tag_count);
            to = (to <= model_src->mesh_count) ? (to) : (model_src->mesh_count);
            for(; index <= to; ++index)
            {
                ent_dest->bf->bone_tags[index].mesh_base = model_src->mesh_tree[index].mesh_base;
            }
        }
    }
    else
    {
        Con_Warning("setEntityMeshes: expecting arguments (id_dest, id_src)");
    }

    return 0;
}


int lua_SetEntitySkinMeshes(lua_State * lua)
{
    if(lua_gettop(lua) >= 4)
    {
        entity_p         ent_dest = World_GetEntityByID(lua_tointeger(lua, 1));
        skeletal_model_p model_src = World_GetModelByID(lua_tointeger(lua, 2));
        if(ent_dest && model_src)
        {
            int index = lua_tointeger(lua, 3);
            int to = (lua_isnil(lua, 4)) ? (ent_dest->bf->bone_tag_count) : (lua_tointeger(lua, 4));
            index = (index >= 0) ? (index) : (0);
            to = (to <= ent_dest->bf->bone_tag_count) ? (to) : (ent_dest->bf->bone_tag_count);
            to = (to <= model_src->mesh_count) ? (to) : (model_src->mesh_count);
            for(; index <= to; ++index)
            {
                ent_dest->bf->bone_tags[index].mesh_skin = model_src->mesh_tree[index].mesh_base;
            }
            SSBoneFrame_FillSkinnedMeshMap(ent_dest->bf);
        }
    }
    else
    {
        Con_Warning("setEntitySkinMeshes: expecting arguments (id_dest, id_src)");
    }

    return 0;
}


int lua_SetModelMeshReplaceFlag(lua_State *lua)
{
    if(lua_gettop(lua) >= 3)
    {
        skeletal_model_p sm = World_GetModelByID(lua_tointeger(lua, 1));
        if(sm)
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
    }
    else
    {
        Con_Printf("setModelMeshReplaceFlag: Wrong arguments count. Must be (id_model, bone_num, flag)");
    }

    return 0;
}


int lua_SetEntityAnimFlag(lua_State * lua)
{
    int top = lua_gettop(lua);

    if(top >= 3)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            int anim_type_id = lua_tointeger(lua, 2);
            for(ss_animation_p  ss_anim_it = &ent->bf->animations; ss_anim_it; ss_anim_it = ss_anim_it->next)
            {
                if(ss_anim_it->type == anim_type_id)
                {
                    ss_anim_it->anim_frame_flags = lua_tointeger(lua, 3);
                    break;
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
        Con_Warning("setEntityAnimFlag: expecting arguments (entity_id, anim_type_id, anim_flag)");
    }

    return 0;
}


int lua_SetModelAnimReplaceFlag(lua_State *lua)
{
    if(lua_gettop(lua) >= 3)
    {
        skeletal_model_p sm = World_GetModelByID(lua_tointeger(lua, 1));
        if(sm)
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
    }
    else
    {
        Con_Printf("setModelAnimReplaceFlag: Wrong arguments count. Must be (id_model, bone_num, flag)");
    }

    return 0;
}


int lua_CopyMeshFromModelToModel(lua_State *lua)
{
    if(lua_gettop(lua) >= 4)
    {
        skeletal_model_p sm1 = World_GetModelByID(lua_tointeger(lua, 1));
        skeletal_model_p sm2 = World_GetModelByID(lua_tointeger(lua, 2));

        if(sm1 && sm2)
        {
            int bone1 = lua_tointeger(lua, 3);
            int bone2 = lua_tointeger(lua, 4);
            if((bone1 >= 0) && (bone1 < sm1->mesh_count) && (bone2 >= 0) && (bone2 < sm2->mesh_count))
            {
                sm1->mesh_tree[bone1].mesh_base = sm2->mesh_tree[bone2].mesh_base;
            }
        }
    }
    else
    {
        Con_Printf("copyMeshFromModelToModel: expecting arguments (id_model1, id_model2, bone_num1, bone_num2)");
    }

    return 0;
}


int lua_SetEntityBoneVisibility(lua_State * lua)
{
    int top = lua_gettop(lua);

    if(top >= 3)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            uint16_t bone_id = lua_tointeger(lua, 2);
            if(bone_id < ent->bf->bone_tag_count)
            {
                ent->bf->bone_tags[bone_id].is_hidden = !lua_toboolean(lua, 3);
            }
        }
        else
        {
            Con_Warning("no entity with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("setEntityBoneVisibility: expecting arguments (entity_id, bone_id, value");
    }

    return 0;
}



int lua_SetEntityAnim(lua_State * lua)
{
    int top = lua_gettop(lua);

    if(top >= 4)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            uint16_t anim_type_id = lua_tointeger(lua, 2);
            ss_animation_p ss_anim = SSBoneFrame_GetOverrideAnim(ent->bf, anim_type_id);
            if(ss_anim && ss_anim->model)
            {
                Anim_SetAnimation(ss_anim, lua_tointeger(lua, 3), lua_tointeger(lua, 4));
                if(top >= 6)
                {
                    int16_t anim = lua_tointeger(lua, 5);
                    int16_t frame = lua_tointeger(lua, 6);
                    if((anim < ss_anim->model->animation_count) && (frame < ss_anim->model->animations[anim].frames_count))
                    {
                        ss_anim->current_animation = anim;
                        ss_anim->current_frame = frame;
                    }
                }
            }
            SSBoneFrame_Update(ent->bf, 0.0f);
        }
        else
        {
            Con_Warning("no entity with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("setEntityAnim: expecting arguments (entity_id, anim_type_id, anim_num, frame_number, (curr_anim, curr_frame))");
    }

    return 0;
}


int lua_GetEntityAnim(lua_State * lua)
{
    if(lua_gettop(lua) >= 2)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
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
        }
        else
        {
            Con_Warning("no entity with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("getEntityAnim: expecting arguments (entity_id, anim_type_id)");
    }

    return 0;
}


int lua_EntitySSAnimCopy(lua_State * lua)
{
    if(lua_gettop(lua) >= 2)
    {
        entity_p ent_dest = World_GetEntityByID(lua_tointeger(lua, 1));
        entity_p ent_src = World_GetEntityByID(lua_tointeger(lua, 2));
        if(ent_dest && ent_src)
        {
            SSBoneFrame_Copy(ent_dest->bf, ent_src->bf);
        }
    }
    else
    {
        Con_Warning("entitySSAnimCopy: expecting arguments (id_dest, id_src)");
    }

    return 0;
}


int lua_EntitySSAnimEnsureExists(lua_State * lua)
{
    if(lua_gettop(lua) >= 3)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
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
        }
        else
        {
            Con_Warning("no entity with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("entitySSAnimEnsureExists: expecting arguments (entity_id, anim_type_id, model_id)");
    }

    return 0;
}


int lua_EntitySSAnimSetTarget(lua_State * lua)
{
    if(lua_gettop(lua) >= 8)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        int ind = lua_tointeger(lua, 2);
        if(ent && (ind >= 0) && (ind < ent->bf->bone_tag_count))
        {
            ss_bone_tag_p b_tag = ent->bf->bone_tags + ind;
            float pos[3], dir[3];
            pos[0] = lua_tonumber(lua, 3);
            pos[1] = lua_tonumber(lua, 4);
            pos[2] = lua_tonumber(lua, 5);
            dir[0] = lua_tonumber(lua, 6);
            dir[1] = lua_tonumber(lua, 7);
            dir[2] = lua_tonumber(lua, 8);

            SSBoneFrame_SetTarget(b_tag, pos, dir);
        }
        else
        {
            Con_Warning("no entity with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("entitySSAnimSetTarget: expecting arguments (entity_id, targeted_bone, target_x, target_y, target_z, bone_dir_x, bone_dir_y, bone_dir_z)");
    }

    return 0;
}


int lua_EntitySSAnimSetAxisMod(lua_State * lua)
{
    if(lua_gettop(lua) >= 5)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        int ind = lua_tointeger(lua, 2);
        if(ent && (ind >= 0) && (ind < ent->bf->bone_tag_count))
        {
            ss_bone_tag_p b_tag = ent->bf->bone_tags + ind;
            float mod[3];
            mod[0] = lua_tonumber(lua, 3);
            mod[1] = lua_tonumber(lua, 4);
            mod[2] = lua_tonumber(lua, 5);
            SSBoneFrame_SetTargetingAxisMod(b_tag, mod);
        }
        else
        {
            Con_Warning("no entity with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("entitySSAnimSetAxisMod: expecting arguments (entity_id, bone_id, mod_x, mod_y, mod_z)");
    }

    return 0;
}


int lua_EntitySSAnimSetTargetingLimit(lua_State * lua)
{
    if(lua_gettop(lua) >= 6)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        int ind = lua_tointeger(lua, 2);
        if(ent && (ind >= 0) && (ind < ent->bf->bone_tag_count))
        {
            ss_bone_tag_p b_tag = ent->bf->bone_tags + ind;
            float q[4];
            q[0] = lua_tonumber(lua, 3);
            q[1] = lua_tonumber(lua, 4);
            q[2] = lua_tonumber(lua, 5);
            q[3] = lua_tonumber(lua, 6);
            SSBoneFrame_SetTargetingLimit(b_tag, q);
        }
        else
        {
            Con_Warning("no entity with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("entitySSAnimSetTargetingLimit: expecting arguments (entity_id, bone_id, q_x, q_y, q_z, q_w)");
    }

    return 0;
}


int lua_EntitySSAnimSetCurrentRotation(lua_State * lua)
{
    if(lua_gettop(lua) >= 6)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        int ind = lua_tointeger(lua, 2);
        if(ent && (ind >= 0) && (ind < ent->bf->bone_tag_count))
        {
            ss_bone_tag_p b_tag = ent->bf->bone_tags + ind;
            b_tag->mod.current[0] = lua_tonumber(lua, 3);
            b_tag->mod.current[1] = lua_tonumber(lua, 4);
            b_tag->mod.current[2] = lua_tonumber(lua, 5);
            b_tag->mod.current[3] = lua_tonumber(lua, 6);
        }
        else
        {
            Con_Warning("no entity with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("entitySSAnimSetCurrentRotation: expecting arguments (entity_id, bone_id, q_x, q_y, q_z, q_w)");
    }

    return 0;
}


int lua_EntitySSAnimSetExtFlags(lua_State * lua)
{
    if(lua_gettop(lua) >= 4)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            ss_animation_p ss_anim = SSBoneFrame_GetOverrideAnim(ent->bf, lua_tointeger(lua, 2));
            if(ss_anim)
            {
                ss_anim->enabled = 0x01 & (lua_tointeger(lua, 3));
                ss_anim->anim_ext_flags = lua_tointeger(lua, 4);
            }
        }
        else
        {
            Con_Warning("no entity with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("entitySSAnimSetCurrentRotation: expecting arguments (entity_id, anim_type_id, enabled, anim_ext_flags)");
    }

    return 0;
}


int lua_EntitySSAnimSetEnable(lua_State * lua)
{
    if(lua_gettop(lua) >= 3)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            int anim_type_id = lua_tointeger(lua, 2);
            if(lua_tointeger(lua, 3))
            {
                SSBoneFrame_EnableOverrideAnimByType(ent->bf, anim_type_id);
            }
            else
            {
                SSBoneFrame_DisableOverrideAnim(ent->bf, anim_type_id);
            }
        }
        else
        {
            Con_Warning("no entity with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("entitySSAnimSetEnable: expecting arguments (entity_id, anim_type_id, enabled)");
    }

    return 0;
}


int lua_EntitySSAnimGetEnable(lua_State * lua)
{
    if(lua_gettop(lua) >= 2)
    {
        entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
        if(ent)
        {
            int anim_type_id = lua_tointeger(lua, 2);
            ss_animation_p ss_anim = SSBoneFrame_GetOverrideAnim(ent->bf, anim_type_id);
            lua_pushboolean(lua, (ss_anim && ss_anim->enabled));
            return 1;
        }
        else
        {
            Con_Warning("no entity with id = %d", lua_tointeger(lua, 1));
        }
    }
    else
    {
        Con_Warning("entitySSAnimGetEnable: expecting arguments (entity_id, anim_type_id)");
    }

    return 0;
}


void Script_LuaRegisterAnimFuncs(lua_State *lua)
{
    lua_register(lua, "setEntityBaseAnimModel", lua_SetEntityBaseAnimModel);
    lua_register(lua, "setModelCollisionMap", lua_SetModelCollisionMap);
    lua_register(lua, "setModelBodyPartFlag", lua_SetModelBodyPartFlag);
    lua_register(lua, "copyModelAnimations", lua_CopyModelAnimations);
    lua_register(lua, "setStateChangeRange", lua_SetStateChangeRange);

    lua_register(lua, "getEntityModelID", lua_GetEntityModelID);
    lua_register(lua, "getEntityAnimState", lua_GetEntityAnimState);
    lua_register(lua, "setEntityAnimState", lua_SetEntityAnimState);
    lua_register(lua, "setEntityAnimStateHeavy", lua_SetEntityAnimStateHeavy);
    lua_register(lua, "getModelMeshCount", lua_GetModelMeshCount);
    lua_register(lua, "getEntityMeshCount", lua_GetEntityMeshCount);
    lua_register(lua, "setEntityMeshes", lua_SetEntityMeshes);
    lua_register(lua, "setEntitySkinMeshes", lua_SetEntitySkinMeshes);
    lua_register(lua, "setModelMeshReplaceFlag", lua_SetModelMeshReplaceFlag);
    lua_register(lua, "setEntityAnimFlag", lua_SetEntityAnimFlag);
    lua_register(lua, "setModelAnimReplaceFlag", lua_SetModelAnimReplaceFlag);
    lua_register(lua, "copyMeshFromModelToModel", lua_CopyMeshFromModelToModel);

    lua_register(lua, "getEntityAnim", lua_GetEntityAnim);
    lua_register(lua, "setEntityBoneVisibility", lua_SetEntityBoneVisibility);
    lua_register(lua, "setEntityAnim", lua_SetEntityAnim);
    lua_register(lua, "entitySSAnimCopy", lua_EntitySSAnimCopy);
    lua_register(lua, "entitySSAnimEnsureExists", lua_EntitySSAnimEnsureExists);
    lua_register(lua, "entitySSAnimSetTarget", lua_EntitySSAnimSetTarget);
    lua_register(lua, "entitySSAnimSetAxisMod", lua_EntitySSAnimSetAxisMod);
    lua_register(lua, "entitySSAnimSetTargetingLimit", lua_EntitySSAnimSetTargetingLimit);
    lua_register(lua, "entitySSAnimSetCurrentRotation", lua_EntitySSAnimSetCurrentRotation);
    lua_register(lua, "entitySSAnimSetExtFlags", lua_EntitySSAnimSetExtFlags);
    lua_register(lua, "entitySSAnimSetEnable", lua_EntitySSAnimSetEnable);
    lua_register(lua, "entitySSAnimGetEnable", lua_EntitySSAnimGetEnable);
}
