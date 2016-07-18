#include <stdlib.h>
#include <math.h>

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include "core/console.h"
#include "core/vmath.h"
#include "core/obb.h"
#include "render/camera.h"
#include "render/render.h"
#include "vt/tr_versions.h"
#include "audio.h"
#include "mesh.h"
#include "skeletal_model.h"
#include "entity.h"
#include "room.h"
#include "world.h"
#include "engine.h"
#include "physics.h"
#include "script.h"
#include "trigger.h"
#include "anim_state_control.h"
#include "character_controller.h"
#include "gameflow.h"
#include "engine_string.h"


entity_p Entity_Create()
{
    entity_p ret = (entity_p)calloc(1, sizeof(entity_t));

    ret->move_type = MOVE_ON_FLOOR;
    Mat4_E(ret->transform);
    ret->state_flags = ENTITY_STATE_ENABLED | ENTITY_STATE_ACTIVE | ENTITY_STATE_VISIBLE;
    ret->type_flags = ENTITY_TYPE_GENERIC;
    ret->callback_flags = 0x00000000;               // no callbacks by default

    ret->OCB = 0;
    ret->trigger_layout = 0x00;
    ret->timer = 0.0;

    ret->self = (engine_container_p)malloc(sizeof(engine_container_t));
    ret->self->next = NULL;
    ret->self->object = ret;
    ret->self->object_type = OBJECT_ENTITY;
    ret->self->room = NULL;
    ret->self->collision_type = COLLISION_TYPE_KINEMATIC;
    ret->self->collision_shape = COLLISION_SHAPE_TRIMESH;
    ret->obb = OBB_Create();
    ret->obb->transform = ret->transform;

    ret->no_fix_all = 0x00;
    ret->no_fix_z = 0x00;
    ret->no_fix_skeletal_parts = 0x00000000;
    ret->physics = Physics_CreatePhysicsData(ret->self);

    ret->character = NULL;
    ret->current_sector = NULL;

    ret->bf = (ss_bone_frame_p)malloc(sizeof(ss_bone_frame_t));
    SSBoneFrame_CreateFromModel(ret->bf, NULL);
    vec3_set_zero(ret->angles);
    vec3_set_zero(ret->speed);
    vec3_set_one(ret->scaling);

    ret->linear_speed = 0.0f;
    ret->anim_linear_speed = 0.0f;

    ret->activation_offset[0] = 0.0f;
    ret->activation_offset[1] = 256.0f;
    ret->activation_offset[2] = 0.0f;
    ret->activation_offset[3] = 128.0f;

    return ret;
}


void Entity_Clear(entity_p entity)
{
    if(entity)
    {
        if(entity->self->room && (entity != World_GetPlayer()))
        {
            Room_RemoveObject(entity->self->room, entity->self);
        }

        if(entity->obb)
        {
            OBB_Clear(entity->obb);
            free(entity->obb);
            entity->obb = NULL;
        }

        Ragdoll_Delete(entity->physics);
        entity->type_flags &= ~ENTITY_TYPE_DYNAMIC;

        if(entity->character)
        {
            Character_Clean(entity);
        }

        Physics_DeletePhysicsData(entity->physics);
        entity->physics = NULL;

        if(entity->self)
        {
            free(entity->self);
            entity->self = NULL;
        }

        if(entity->bf)
        {
            SSBoneFrame_Clear(entity->bf);
            free(entity->bf);
            entity->bf = NULL;
        }
    }
}


void Entity_Enable(entity_p ent)
{
    if(!(ent->state_flags & ENTITY_STATE_ENABLED))
    {
        Entity_EnableCollision(ent);
        ent->state_flags |= ENTITY_STATE_ENABLED | ENTITY_STATE_ACTIVE | ENTITY_STATE_VISIBLE;
    }
}


void Entity_Disable(entity_p ent)
{
    if(ent->state_flags & ENTITY_STATE_ENABLED)
    {
        Entity_DisableCollision(ent);
        ent->state_flags = 0x0000;
    }
}


void Entity_EnableCollision(entity_p ent)
{
    if(Physics_IsBodyesInited(ent->physics))
    {
        ent->self->collision_type |= 0x0001;
        Physics_EnableCollision(ent->physics);
    }
    else
    {
        ent->self->collision_type = COLLISION_TYPE_KINEMATIC;
        Physics_GenRigidBody(ent->physics, ent->bf);
    }
}


void Entity_DisableCollision(entity_p ent)
{
    if(Physics_IsBodyesInited(ent->physics))
    {
        ent->self->collision_type &= ~0x0001;
        Physics_DisableCollision(ent->physics);
    }
}


void Entity_UpdateRoomPos(entity_p ent)
{
    float pos[3];
    room_p new_room;
    room_sector_p new_sector;

    if(ent->character)
    {
        Mat4_vec3_mul(pos, ent->transform, ent->bf->bone_tags->full_transform+12);
        pos[0] = ent->transform[12+0];
        pos[1] = ent->transform[12+1];
    }
    else
    {
        float v[3];
        vec3_add(v, ent->bf->bb_min, ent->bf->bb_max);
        v[0] /= 2.0;
        v[1] /= 2.0;
        v[2] /= 2.0;
        Mat4_vec3_mul_macro(pos, ent->transform, v);
    }
    new_room = World_FindRoomByPosCogerrence(pos, ent->self->room);
    if(new_room)
    {
        new_sector = Room_GetSectorXYZ(new_room, pos);
        if(new_room != new_sector->owner_room)
        {
            new_room = new_sector->owner_room;
        }

        if(!ent->character && (ent->self->room != new_room))
        {
            if((ent->self->room != NULL) && !Room_IsOverlapped(ent->self->room, new_room))
            {
                if(ent->self->room)
                {
                    Room_RemoveObject(ent->self->room, ent->self);
                }
                if(new_room)
                {
                    Room_AddObject(new_room, ent->self);
                }
            }
        }

        ent->self->room = new_room;
        ent->last_sector = ent->current_sector;

        if(ent->current_sector != new_sector)
        {
            ent->trigger_layout &= (uint8_t)(~ENTITY_TLAYOUT_SSTATUS);          // Reset sector status.
            ent->current_sector = new_sector;
        }
    }
}


void Entity_UpdateTransform(entity_p entity)
{
    int32_t i = entity->angles[0] / 360.0;
    i = (entity->angles[0] < 0.0)?(i-1):(i);
    entity->angles[0] -= 360.0 * i;

    i = entity->angles[1] / 360.0;
    i = (entity->angles[1] < 0.0)?(i-1):(i);
    entity->angles[1] -= 360.0 * i;

    i = entity->angles[2] / 360.0;
    i = (entity->angles[2] < 0.0)?(i-1):(i);
    entity->angles[2] -= 360.0 * i;

    Mat4_SetSelfOrientation(entity->transform, entity->angles);
}


void Entity_UpdateRigidBody(struct entity_s *ent, int force)
{
    if(ent->type_flags & ENTITY_TYPE_DYNAMIC)
    {
        float tr[16];
        Physics_GetBodyWorldTransform(ent->physics, ent->transform, 0);
        Entity_UpdateRoomPos(ent);
        switch(ent->self->collision_shape)
        {
            case COLLISION_SHAPE_SINGLE_BOX:
                return;

            case COLLISION_SHAPE_SINGLE_SPHERE:
                {
                    float centre[3], offset[3];
                    centre[0] = 0.5f * (ent->bf->bb_min[0] + ent->bf->bb_max[0]);
                    centre[1] = 0.5f * (ent->bf->bb_min[1] + ent->bf->bb_max[1]);
                    centre[2] = 0.5f * (ent->bf->bb_min[2] + ent->bf->bb_max[2]);
                    Mat4_vec3_rot_macro(offset, ent->transform, centre);
                    ent->transform[12 + 0] -= offset[0];
                    ent->transform[12 + 1] -= offset[1];
                    ent->transform[12 + 2] -= offset[2];
                }
                return;
        };
        Mat4_E(ent->bf->bone_tags[0].full_transform);
        Physics_GetBodyWorldTransform(ent->physics, tr, 0);
        Physics_SetGhostWorldTransform(ent->physics, tr, 0);
        for(uint16_t i = 1; i < ent->bf->bone_tag_count; i++)
        {
            Physics_GetBodyWorldTransform(ent->physics, tr, i);
            Physics_SetGhostWorldTransform(ent->physics, tr, i);
            Mat4_inv_Mat4_affine_mul(ent->bf->bone_tags[i].full_transform, ent->transform, tr);
        }

        // fill bone frame transformation matrices;
        for(uint16_t i = 0; i < ent->bf->bone_tag_count; i++)
        {
            if(ent->bf->bone_tags[i].parent != NULL)
            {
                Mat4_inv_Mat4_affine_mul(ent->bf->bone_tags[i].transform, ent->bf->bone_tags[i].parent->full_transform, ent->bf->bone_tags[i].full_transform);
            }
            else
            {
                Mat4_Copy(ent->bf->bone_tags[i].transform, ent->bf->bone_tags[i].full_transform);
            }
        }

        // recalculate visibility box
        if(ent->bf->bone_tag_count == 1)
        {
            vec3_copy(ent->bf->bb_min, ent->bf->bone_tags[0].mesh_base->bb_min);
            vec3_copy(ent->bf->bb_max, ent->bf->bone_tags[0].mesh_base->bb_max);
        }
        else
        {
            vec3_copy(ent->bf->bb_min, ent->bf->bone_tags[0].mesh_base->bb_min);
            vec3_copy(ent->bf->bb_max, ent->bf->bone_tags[0].mesh_base->bb_max);
            for(uint16_t i = 0; i < ent->bf->bone_tag_count; i++)
            {
                float *pos = ent->bf->bone_tags[i].full_transform + 12;
                float *bb_min = ent->bf->bone_tags[i].mesh_base->bb_min;
                float *bb_max = ent->bf->bone_tags[i].mesh_base->bb_max;
                float r = bb_max[0] - bb_min[0];
                float t = bb_max[1] - bb_min[1];
                r = (t > r)?(t):(r);
                t = bb_max[2] - bb_min[2];
                r = (t > r)?(t):(r);
                r *= 0.5;

                if(ent->bf->bb_min[0] > pos[0] - r)
                {
                    ent->bf->bb_min[0] = pos[0] - r;
                }
                if(ent->bf->bb_min[1] > pos[1] - r)
                {
                    ent->bf->bb_min[1] = pos[1] - r;
                }
                if(ent->bf->bb_min[2] > pos[2] - r)
                {
                    ent->bf->bb_min[2] = pos[2] - r;
                }

                if(ent->bf->bb_max[0] < pos[0] + r)
                {
                    ent->bf->bb_max[0] = pos[0] + r;
                }
                if(ent->bf->bb_max[1] < pos[1] + r)
                {
                    ent->bf->bb_max[1] = pos[1] + r;
                }
                if(ent->bf->bb_max[2] < pos[2] + r)
                {
                    ent->bf->bb_max[2] = pos[2] + r;
                }
            }
        }
    }
    else
    {
        if((ent->bf->animations.model == NULL) || !Physics_IsBodyesInited(ent->physics) ||
           ((force == 0) && (ent->bf->animations.model->animation_count == 1) && (ent->bf->animations.model->animations->frames_count == 1)))
        {
            return;
        }

        Entity_UpdateRoomPos(ent);
        if(ent->self->collision_type & 0x0001)
        {
            switch(ent->self->collision_shape)
            {
                case COLLISION_SHAPE_SINGLE_BOX:
                    Physics_SetBodyWorldTransform(ent->physics, ent->transform, 0);
                    break;

                case COLLISION_SHAPE_SINGLE_SPHERE:
                    {
                        float centre[3], offset[3];
                        centre[0] = 0.5f * (ent->bf->bb_min[0] + ent->bf->bb_max[0]);
                        centre[1] = 0.5f * (ent->bf->bb_min[1] + ent->bf->bb_max[1]);
                        centre[2] = 0.5f * (ent->bf->bb_min[2] + ent->bf->bb_max[2]);
                        Mat4_vec3_rot_macro(offset, ent->transform, centre);
                        ent->transform[12 + 0] += offset[0];
                        ent->transform[12 + 1] += offset[1];
                        ent->transform[12 + 2] += offset[2];
                        Physics_SetBodyWorldTransform(ent->physics, ent->transform, 0);
                        ent->transform[12 + 0] -= offset[0];
                        ent->transform[12 + 1] -= offset[1];
                        ent->transform[12 + 2] -= offset[2];
                    }
                    break;

                default:
                    {
                        float tr[16];
                        for(uint16_t i = 0; i < ent->bf->bone_tag_count; i++)
                        {
                            Mat4_Mat4_mul(tr, ent->transform, ent->bf->bone_tags[i].full_transform);
                            Physics_SetBodyWorldTransform(ent->physics, tr, i);
                        }
                    }
                    break;
            };
        }
    }

    Entity_RebuildBV(ent);
}


void Entity_GhostUpdate(struct entity_s *ent)
{
    if(Physics_IsGhostsInited(ent->physics))
    {
        float tr[16];
        uint16_t max_index = Physics_GetBodiesCount(ent->physics);
        for(uint16_t i = 0; i < max_index; i++)
        {
            Physics_GetBodyWorldTransform(ent->physics, tr, i);
            Physics_SetGhostWorldTransform(ent->physics, tr, i);
        }
    }
}


///@TODO: make experiment with convexSweepTest with spheres: no more iterative cycles;
int Entity_GetPenetrationFixVector(struct entity_s *ent, float reaction[3], float move_global[3])
{
    int ret = 0;

    vec3_set_zero(reaction);
    if(Physics_IsGhostsInited(ent->physics) && (ent->no_fix_all == 0x00) && (Physics_GetBodiesCount(ent->physics) == ent->bf->bone_tag_count))
    {
        float tmp[3], orig_pos[3];
        float tr[16];
        float from[3], to[3], curr[3], move[3], move_len;

        vec3_copy(orig_pos, ent->transform + 12);
        for(uint16_t i = 0; i < ent->bf->bone_tag_count; i++)
        {
            uint16_t m = ent->bf->animations.model->collision_map[i];
            ss_bone_tag_p btag = ent->bf->bone_tags + m;

            if(btag->body_part & ent->no_fix_skeletal_parts)
            {
                continue;
            }

            // antitunneling condition for main body parts, needs only in move case: ((move != NULL) && (btag->body_part & (BODY_PART_BODY_LOW | BODY_PART_BODY_UPPER)))
            if((btag->parent == NULL) || ((move_global != NULL) && (btag->body_part & (BODY_PART_BODY_LOW | BODY_PART_BODY_UPPER))))
            {
                Physics_GetGhostWorldTransform(ent->physics, tr, m);
                from[0] = tr[12 + 0] + ent->transform[12 + 0] - orig_pos[0];
                from[1] = tr[12 + 1] + ent->transform[12 + 1] - orig_pos[1];
                from[2] = tr[12 + 2] + ent->transform[12 + 2] - orig_pos[2];
            }
            else
            {
                float parent_from[3];
                Mat4_vec3_mul(parent_from, btag->parent->full_transform, btag->parent->mesh_base->centre);
                Mat4_vec3_mul(from, ent->transform, parent_from);
            }

            Mat4_Mat4_mul(tr, ent->transform, btag->full_transform);
            Mat4_vec3_mul(to, tr, btag->mesh_base->centre);
            vec3_copy(curr, from);
            vec3_sub(move, to, from);
            move_len = vec3_abs(move);
            if((i == 0) && (move_len > 1024.0))                                 ///@FIXME: magick const 1024.0!
            {
                break;
            }
            int iter = (float)(2.0 * move_len / btag->mesh_base->R) + 1;        ///@FIXME (not a critical): magick const 2.0!
            move[0] /= (float)iter;
            move[1] /= (float)iter;
            move[2] /= (float)iter;

            for(int j = 0; j <= iter; j++)
            {
                vec3_copy(tr+12, curr);
                Physics_SetGhostWorldTransform(ent->physics, tr, m);
                if(Physics_GetGhostPenetrationFixVector(ent->physics, m, tmp))
                {
                    vec3_add_to(ent->transform + 12, tmp);
                    vec3_add_to(curr, tmp);
                    vec3_add_to(from, tmp);
                    ret++;
                }
                vec3_add_to(curr, move);
            }
        }
        vec3_sub(reaction, ent->transform + 12, orig_pos);
        vec3_copy(ent->transform + 12, orig_pos);
    }

    return ret;
}


/**
 * we check walls and other collision objects reaction. if reaction more then critacal
 * then cmd->horizontal_collide |= 0x01;
 * @param ent - cheked entity
 * @param cmd - here we fill cmd->horizontal_collide field
 * @param move - absolute 3d move vector
 */
int Entity_CheckNextPenetration(struct entity_s *ent, float move[3])
{
    int ret = 0;
    if(Physics_IsGhostsInited(ent->physics))
    {
        float t1, t2, reaction[3], *pos = ent->transform + 12;

        Entity_GhostUpdate(ent);
        vec3_add(pos, pos, move);
        //resp->horizontal_collide = 0x00;
        ret = Entity_GetPenetrationFixVector(ent, reaction, move);
        if((ret > 0) && (ent->character != NULL))
        {
            t1 = reaction[0] * reaction[0] + reaction[1] * reaction[1];
            t2 = move[0] * move[0] + move[1] * move[1];
            if((reaction[2] * reaction[2] < t1) && (move[2] * move[2] < t2))
            {
                t2 *= t1;
                t1 = (reaction[0] * move[0] + reaction[1] * move[1]) / sqrtf(t2);
                if(t1 < ent->character->critical_wall_component)
                {
                    ent->character->resp.horizontal_collide |= 0x01;
                }
            }
        }
        vec3_sub(pos, pos, move);
        Entity_GhostUpdate(ent);
    }

    return ret;
}


void Entity_FixPenetrations(struct entity_s *ent, float move[3])
{
    if(Physics_IsGhostsInited(ent->physics))
    {
        float t1, t2, reaction[3];

        if((move != NULL) && (ent->character != NULL))
        {
            ent->character->resp.horizontal_collide    = 0x00;
            ent->character->resp.vertical_collide      = 0x00;
        }

        if(ent->type_flags & ENTITY_TYPE_DYNAMIC)
        {
            return;
        }

        if(ent->no_fix_all)
        {
            Entity_GhostUpdate(ent);
            return;
        }

        int numPenetrationLoops = Entity_GetPenetrationFixVector(ent, reaction, move);
        vec3_add(ent->transform+12, ent->transform+12, reaction);

        if(ent->character != NULL)
        {
            if((move != NULL) && (numPenetrationLoops > 0))
            {
                t1 = reaction[0] * reaction[0] + reaction[1] * reaction[1];
                t2 = move[0] * move[0] + move[1] * move[1];
                if((reaction[2] * reaction[2] < t1) && (move[2] * move[2] < t2))    // we have horizontal move and horizontal correction
                {
                    t2 *= t1;
                    t1 = (reaction[0] * move[0] + reaction[1] * move[1]) / sqrtf(t2);
                    if(t1 < ent->character->critical_wall_component)
                    {
                        ent->character->resp.horizontal_collide |= 0x01;
                    }
                }
                else if((reaction[2] * reaction[2] > t1) && (move[2] * move[2] > t2))
                {
                    if((reaction[2] > 0.0) && (move[2] < 0.0))
                    {
                        ent->character->resp.vertical_collide |= 0x01;
                    }
                    else if((reaction[2] < 0.0) && (move[2] > 0.0))
                    {
                        ent->character->resp.vertical_collide |= 0x02;
                    }
                }
            }

            if(ent->character->height_info.ceiling_hit.hit && (reaction[2] < -0.1))
            {
                ent->character->resp.vertical_collide |= 0x02;
            }

            if(ent->character->height_info.floor_hit.hit && (reaction[2] > 0.1))
            {
                ent->character->resp.vertical_collide |= 0x01;
            }
        }

        Entity_GhostUpdate(ent);
    }
}


int  Entity_GetSubstanceState(entity_p entity)
{
    if((!entity) || (!entity->character))
    {
        return 0;
    }

    if(entity->self->room->flags & TR_ROOM_FLAG_QUICKSAND)
    {
        if(entity->character->height_info.transition_level > entity->transform[12 + 2] + entity->character->Height)
        {
            return ENTITY_SUBSTANCE_QUICKSAND_CONSUMED;
        }
        else
        {
            return ENTITY_SUBSTANCE_QUICKSAND_SHALLOW;
        }
    }
    else if(!entity->character->height_info.water)
    {
        return ENTITY_SUBSTANCE_NONE;
    }
    else if( entity->character->height_info.water &&
            (entity->character->height_info.transition_level > entity->transform[12 + 2]) &&
            (entity->character->height_info.transition_level < entity->transform[12 + 2] + entity->character->wade_depth) )
    {
        return ENTITY_SUBSTANCE_WATER_SHALLOW;
    }
    else if( entity->character->height_info.water &&
            (entity->character->height_info.transition_level > entity->transform[12 + 2] + entity->character->wade_depth) )
    {
        return ENTITY_SUBSTANCE_WATER_WADE;
    }
    else
    {
        return ENTITY_SUBSTANCE_WATER_SWIM;
    }
}


void Entity_CheckCollisionCallbacks(entity_p ent)
{
    // I do not know why, but without Entity_GhostUpdate(ent); it works pretty slow!
    Entity_GhostUpdate(ent);
    collision_node_p cn = Physics_GetCurrentCollisions(ent->physics);
    for(; cn; cn = cn->next)
    {
        // do callbacks here:
        if(cn->obj->object_type == OBJECT_ENTITY)
        {
            entity_p activator = (entity_p)cn->obj->object;

            if(activator->callback_flags & ENTITY_CALLBACK_COLLISION)
            {
                // Activator and entity IDs are swapped in case of collision callback.
                Script_ExecEntity(engine_lua, ENTITY_CALLBACK_COLLISION, activator->id, ent->id);
                //Con_Printf("collider_bone_index = %d, collider_type = %d", cn->part_self, cn->obj->object_type);
            }
        }
    }
}


void Entity_DoAnimCommands(entity_p entity, struct ss_animation_s *ss_anim, int changing)
{
    if((World_GetAnimCommands() == NULL) || (ss_anim->model == NULL))
    {
        return;  // If no anim commands
    }

    animation_frame_p af  = ss_anim->model->animations + ss_anim->current_animation;
    if(af->num_anim_commands <= 255)
    {
        uint32_t count        = af->num_anim_commands;
        int16_t *pointer      = World_GetAnimCommands() + af->anim_command;
        int8_t   random_value = 0;

        for(uint32_t i = 0; i < count; i++, pointer++)
        {
            switch(*pointer)
            {
                case TR_ANIMCOMMAND_SETPOSITION:
                    // This command executes ONLY at the end of animation.
                    pointer += 3; // Parse through 3 operands.
                    break;

                case TR_ANIMCOMMAND_JUMPDISTANCE:
                    // This command executes ONLY at the end of animation.
                    pointer += 2; // Parse through 2 operands.
                    break;

                case TR_ANIMCOMMAND_EMPTYHANDS:
                    ///@FIXME: Behaviour is yet to be discovered.
                    break;

                case TR_ANIMCOMMAND_KILL:
                    // This command executes ONLY at the end of animation.
                    if(ss_anim->current_frame == af->frames_count - 1)
                    {
                        if(entity->character)
                        {
                            entity->character->resp.kill = 1;
                        }
                    }

                    break;

                case TR_ANIMCOMMAND_PLAYSOUND:
                    int16_t sound_index;

                    if(ss_anim->current_frame == *++pointer)
                    {
                        sound_index = *++pointer & 0x3FFF;

                        // Quick workaround for TR3 quicksand.
                        if((Entity_GetSubstanceState(entity) == ENTITY_SUBSTANCE_QUICKSAND_CONSUMED) ||
                           (Entity_GetSubstanceState(entity) == ENTITY_SUBSTANCE_QUICKSAND_SHALLOW)   )
                        {
                            sound_index = 18;
                        }

                        if(*pointer & TR_ANIMCOMMAND_CONDITION_WATER)
                        {
                            if(Entity_GetSubstanceState(entity) == ENTITY_SUBSTANCE_WATER_SHALLOW)
                                Audio_Send(sound_index, TR_AUDIO_EMITTER_ENTITY, entity->id);
                        }
                        else if(*pointer & TR_ANIMCOMMAND_CONDITION_LAND)
                        {
                            if(Entity_GetSubstanceState(entity) != ENTITY_SUBSTANCE_WATER_SHALLOW)
                                Audio_Send(sound_index, TR_AUDIO_EMITTER_ENTITY, entity->id);
                        }
                        else
                        {
                            Audio_Send(sound_index, TR_AUDIO_EMITTER_ENTITY, entity->id);
                        }
                    }
                    else
                    {
                        pointer++;
                    }
                    break;

                case TR_ANIMCOMMAND_PLAYEFFECT:
                    // Effects (flipeffects) are various non-typical actions which vary
                    // across different TR game engine versions. There are common ones,
                    // however, and currently only these are supported.
                    if(ss_anim->current_frame == *++pointer)
                    {
                        switch(*++pointer & 0x3FFF)
                        {
                            case TR_EFFECT_SHAKESCREEN:
                                if(World_GetPlayer())
                                {
                                    float *pos = World_GetPlayer()->transform + 12;
                                    float dist = vec3_dist(pos, entity->transform + 12);
                                    dist = (dist > TR_CAM_MAX_SHAKE_DISTANCE)?(0):((TR_CAM_MAX_SHAKE_DISTANCE - dist) / 1024.0);
                                    //if(dist > 0)
                                    //    Cam_Shake(&engine_camera, (dist * TR_CAM_DEFAULT_SHAKE_POWER), 0.5);
                                }
                                break;

                            case TR_EFFECT_CHANGEDIRECTION:
                                break;

                            case TR_EFFECT_HIDEOBJECT:
                                entity->state_flags &= ~ENTITY_STATE_VISIBLE;
                                break;

                            case TR_EFFECT_SHOWOBJECT:
                                entity->state_flags |= ENTITY_STATE_VISIBLE;
                                break;

                            case TR_EFFECT_PLAYSTEPSOUND:
                                // Please note that we bypass land/water mask, as TR3-5 tends to ignore
                                // this flag and play step sound in any case on land, ignoring it
                                // completely in water rooms.
                                if(!Entity_GetSubstanceState(entity))
                                {
                                    // TR3-5 footstep map.
                                    // We define it here as a magic numbers array, because TR3-5 versions
                                    // fortunately have no differences in footstep sounds order.
                                    // Also note that some footstep types mutually share same sound IDs
                                    // across different TR versions.
                                    switch(entity->current_sector->material)
                                    {
                                        case SECTOR_MATERIAL_MUD:
                                            Audio_Send(288, TR_AUDIO_EMITTER_ENTITY, entity->id);
                                            break;

                                        case SECTOR_MATERIAL_SNOW:  // TR3 & TR5 only
                                            if(World_GetVersion() != TR_IV)
                                            {
                                                Audio_Send(293, TR_AUDIO_EMITTER_ENTITY, entity->id);
                                            }
                                            break;

                                        case SECTOR_MATERIAL_SAND:  // Same as grass
                                            Audio_Send(291, TR_AUDIO_EMITTER_ENTITY, entity->id);
                                            break;

                                        case SECTOR_MATERIAL_GRAVEL:
                                            Audio_Send(290, TR_AUDIO_EMITTER_ENTITY, entity->id);
                                            break;

                                        case SECTOR_MATERIAL_ICE:   // TR3 & TR5 only
                                            if(World_GetVersion() != TR_IV)
                                            {
                                                Audio_Send(289, TR_AUDIO_EMITTER_ENTITY, entity->id);
                                            }
                                            break;

                                        case SECTOR_MATERIAL_WATER: // BYPASS!
                                            // Audio_Send(17, TR_AUDIO_EMITTER_ENTITY, entity->id);
                                            break;

                                        case SECTOR_MATERIAL_STONE: // DEFAULT SOUND, BYPASS!
                                            // Audio_Send(-1, TR_AUDIO_EMITTER_ENTITY, entity->id);
                                            break;

                                        case SECTOR_MATERIAL_WOOD:
                                            Audio_Send(292, TR_AUDIO_EMITTER_ENTITY, entity->id);
                                            break;

                                        case SECTOR_MATERIAL_METAL:
                                            Audio_Send(294, TR_AUDIO_EMITTER_ENTITY, entity->id);
                                            break;

                                        case SECTOR_MATERIAL_MARBLE:    // TR4 only
                                            if(World_GetVersion() == TR_IV)
                                            {
                                                Audio_Send(293, TR_AUDIO_EMITTER_ENTITY, entity->id);
                                            }
                                            break;

                                        case SECTOR_MATERIAL_GRASS:     // Same as sand
                                            Audio_Send(291, TR_AUDIO_EMITTER_ENTITY, entity->id);
                                            break;

                                        case SECTOR_MATERIAL_CONCRETE:  // DEFAULT SOUND, BYPASS!
                                            Audio_Send(-1, TR_AUDIO_EMITTER_ENTITY, entity->id);
                                            break;

                                        case SECTOR_MATERIAL_OLDWOOD:   // Same as wood
                                            Audio_Send(292, TR_AUDIO_EMITTER_ENTITY, entity->id);
                                            break;

                                        case SECTOR_MATERIAL_OLDMETAL:  // Same as metal
                                            Audio_Send(294, TR_AUDIO_EMITTER_ENTITY, entity->id);
                                            break;
                                    }
                                }
                                break;

                            case TR_EFFECT_BUBBLE:
                                ///@FIXME: Spawn bubble particle here, when particle system is developed.
                                random_value = rand() % 100;
                                if(random_value > 60)
                                {
                                    Audio_Send(TR_AUDIO_SOUND_BUBBLE, TR_AUDIO_EMITTER_ENTITY, entity->id);
                                }
                                break;

                            default:
                                ///@FIXME: TODO ALL OTHER EFFECTS!
                                break;
                        }
                    }
                    else
                    {
                        pointer++;
                    }
                    break;
            }
        }
    }
}


void Entity_ProcessSector(entity_p ent)
{
    if(!ent->current_sector) return;

    // Calculate both above and below sectors for further usage.
    // Sector below is generally needed for getting proper trigger index,
    // as many triggers tend to be called from the lowest room in a row
    // (e.g. first trapdoor in The Great Wall, etc.)
    // Sector above primarily needed for paranoid cases of monkeyswing.

    room_sector_p highest_sector = Sector_GetHighest(ent->current_sector);
    room_sector_p lowest_sector  = Sector_GetLowest(ent->current_sector);

    if(ent->character)
    {
        ent->character->height_info.walls_climb_dir  = 0;
        ent->character->height_info.walls_climb_dir |= lowest_sector->flags & (SECTOR_FLAG_CLIMB_WEST  |
                                                                               SECTOR_FLAG_CLIMB_EAST  |
                                                                               SECTOR_FLAG_CLIMB_NORTH |
                                                                               SECTOR_FLAG_CLIMB_SOUTH );

        ent->character->height_info.walls_climb     = (ent->character->height_info.walls_climb_dir > 0);
        ent->character->height_info.ceiling_climb   = 0x00;

        if((highest_sector->flags & SECTOR_FLAG_CLIMB_CEILING) || (lowest_sector->flags & SECTOR_FLAG_CLIMB_CEILING))
        {
            ent->character->height_info.ceiling_climb = 0x01;
        }

        if(lowest_sector->flags & SECTOR_FLAG_DEATH)
        {
            if((ent->move_type == MOVE_ON_FLOOR)    ||
               (ent->move_type == MOVE_UNDERWATER)  ||
               (ent->move_type == MOVE_WADE)        ||
               (ent->move_type == MOVE_ON_WATER)    ||
               (ent->move_type == MOVE_QUICKSAND))
            {
                Character_SetParam(ent, PARAM_HEALTH, 0.0);
                ent->character->resp.kill = 1;
            }
        }
    }

    // If entity either marked as trigger activator (Lara) or heavytrigger activator (other entities),
    // we try to execute a trigger for this sector.

    if(ent->type_flags & (ENTITY_TYPE_TRIGGER_ACTIVATOR | ENTITY_TYPE_HEAVYTRIGGER_ACTIVATOR))
    {
        // Look up trigger function table and run trigger if it exists.
        Trigger_DoCommands(lowest_sector->trigger, ent);
    }
}

///@FIXME: function did more things than it's name describes;
void Entity_SetAnimation(entity_p entity, int anim_type, int animation, int frame)
{
    if(entity)
    {
        animation = (animation < 0) ? (0) : (animation);
        entity->no_fix_all = 0x00;

        if(anim_type == ANIM_TYPE_BASE)
        {
            entity->anim_linear_speed = entity->bf->animations.model->animations[animation].speed_x;
        }
        SSBoneFrame_SetAnimation(entity->bf, anim_type, animation, frame);
        SSBoneFrame_Update(entity->bf, 0.0f);
    }
}


void Entity_DoAnimMove(entity_p entity, int16_t *anim, int16_t *frame)
{
    if(entity->bf->animations.model != NULL)
    {
        animation_frame_p curr_af = entity->bf->animations.model->animations + entity->bf->animations.current_animation;
        bone_frame_p curr_bf = curr_af->frames + entity->bf->animations.current_frame;

        if(curr_bf->command & ANIM_CMD_JUMP)
        {
            Character_SetToJump(entity, -curr_bf->v_Vertical, curr_bf->v_Horizontal);
        }
        if(curr_bf->command & ANIM_CMD_CHANGE_DIRECTION)
        {
            //Con_Printf("ROTATED: anim = %d, frame = %d of %d", entity->bf->animations.current_animation, entity->bf->animations.current_frame, entity->bf->animations.model->animations[entity->bf->animations.current_animation].frames_count);
            entity->angles[0] += 180.0f;
            if(entity->move_type == MOVE_UNDERWATER)
            {
                entity->angles[1] = -entity->angles[1];                         // for underwater case
            }
            if(entity->dir_flag == ENT_MOVE_BACKWARD)
            {
                entity->dir_flag = ENT_MOVE_FORWARD;
            }
            else if(entity->dir_flag == ENT_MOVE_FORWARD)
            {
                entity->dir_flag = ENT_MOVE_BACKWARD;
            }
            Entity_UpdateTransform(entity);
            Entity_SetAnimation(entity, ANIM_TYPE_BASE, curr_af->next_anim->id, curr_af->next_frame);
            *anim = entity->bf->animations.current_animation;
            *frame = entity->bf->animations.current_frame;
        }
        if(curr_bf->command & ANIM_CMD_MOVE)
        {
            float tr[3];
            Mat4_vec3_rot_macro(tr, entity->transform, curr_bf->move);
            vec3_add(entity->transform+12, entity->transform+12, tr);
        }
    }
}


void Entity_MoveToSink(entity_p entity, uint32_t sink_index)
{
    static_camera_sink_p sink = World_GetstaticCameraSink(sink_index);
    if(sink)
    {
        float sink_pos[3], *ent_pos = entity->transform + 12;
        sink_pos[0] = sink->x;
        sink_pos[1] = sink->y;
        sink_pos[2] = sink->z + 256.0; // Prevents digging into the floor.

        room_sector_p ls = Sector_GetLowest(entity->current_sector);
        room_sector_p hs = Sector_GetHighest(entity->current_sector);

        if((sink_pos[2] > hs->ceiling) ||
           (sink_pos[2] < ls->floor) )
        {
            sink_pos[2] = ent_pos[2];
        }

        float speed[3];
        vec3_sub(speed, sink_pos, ent_pos);
        float t = vec3_abs(speed);
        if(t > 0.001)
        {
            t = 240.0f * engine_frame_time * ((float)(sink->room_or_strength)) / t;

            ent_pos[0] += speed[0] * t;
            ent_pos[1] += speed[1] * t;
            ent_pos[2] += speed[2] * t;

            Entity_UpdateRigidBody(entity, 1);
        }
    }
}


/**
 * In original engine (+ some information from anim_commands) the anim_commands implement in beginning of frame
 */
void Entity_Frame(entity_p entity, float time)
{
    if(entity && !(entity->type_flags & ENTITY_TYPE_DYNAMIC) && (entity->state_flags & ENTITY_STATE_ACTIVE)  && (entity->state_flags & ENTITY_STATE_ENABLED))
    {
        int16_t frame, anim;
        long int t;
        float dt;
        animation_frame_p af;
        state_change_p stc;
        ss_animation_p ss_anim = &entity->bf->animations;
        uint16_t is_base_anim = 1;

        Entity_GhostUpdate(entity);

        while(ss_anim)
        {
            if(ss_anim->enabled)
            {
                if(ss_anim->model && ss_anim->onFrame)
                {
                    int frame_switch_state = ss_anim->onFrame(entity, ss_anim, time);
                    if(ss_anim->onEndFrame != NULL)
                    {
                        ss_anim->onEndFrame(entity, ss_anim, frame_switch_state);
                    }
                }
                else if(ss_anim->model && !(ss_anim->anim_frame_flags & ANIM_FRAME_LOCK) &&
                        ((ss_anim->model->animation_count > 1) || (ss_anim->model->animations->frames_count > 1)))
                {
                    uint16_t frame_switch_state = 0x00;
                    ss_anim->lerp = 0.0;
                    stc = Anim_FindStateChangeByID(ss_anim->model->animations + ss_anim->current_animation, ss_anim->next_state);
                    Anim_GetNextFrame(ss_anim, time, stc, &frame, &anim, ss_anim->anim_frame_flags);
                    if(ss_anim->current_animation != anim)
                    {
                        ss_anim->last_animation = ss_anim->current_animation;

                        frame_switch_state = 0x02;
                        Entity_DoAnimCommands(entity, ss_anim, frame_switch_state);
                        Entity_DoAnimMove(entity, &anim, &frame);

                        Entity_SetAnimation(entity, ANIM_TYPE_BASE, anim, frame);
                        stc = Anim_FindStateChangeByID(ss_anim->model->animations + ss_anim->current_animation, ss_anim->next_state);
                    }
                    else if(ss_anim->current_frame != frame)
                    {
                        if(ss_anim->current_frame == 0)
                        {
                            ss_anim->last_animation = ss_anim->current_animation;
                        }

                        frame_switch_state = 0x01;
                        Entity_DoAnimCommands(entity, ss_anim, frame_switch_state);
                        Entity_DoAnimMove(entity, &anim, &frame);
                    }

                    af = ss_anim->model->animations + ss_anim->current_animation;
                    ss_anim->frame_time += time;

                    t = (ss_anim->frame_time) / ss_anim->period;
                    dt = ss_anim->frame_time - (float)t * ss_anim->period;
                    ss_anim->frame_time = (float)frame * ss_anim->period + dt;
                    ss_anim->lerp = dt / ss_anim->period;
                    Anim_GetNextFrame(ss_anim, ss_anim->period, stc, &ss_anim->next_frame, &ss_anim->next_animation, ss_anim->anim_frame_flags);

                    // Update acceleration.
                    // With variable framerate, we don't know when we'll reach final
                    // frame for sure, so we use native frame number check to increase acceleration.

                    if(is_base_anim && (entity->character) && (ss_anim->current_frame != frame))
                    {
                        // NB!!! For Lara, we update ONLY X-axis speed/accel.
                        if((af->accel_x == 0) || (frame < ss_anim->current_frame))
                        {
                            entity->anim_linear_speed  = af->speed_x;
                        }
                        else
                        {
                            entity->anim_linear_speed += af->accel_x;
                        }
                    }

                    ss_anim->current_frame = frame;

                    if(ss_anim->onEndFrame != NULL)
                    {
                        ss_anim->onEndFrame(entity, ss_anim, frame_switch_state);
                    }
                }
            }
            is_base_anim = 0;
            ss_anim = ss_anim->next;
        }

        SSBoneFrame_Update(entity->bf, time);
        if(entity->character != NULL)
        {
            Entity_FixPenetrations(entity, NULL);
        }
    }
}

/**
 * The function rebuild / renew entity's BV
 */
void Entity_RebuildBV(entity_p ent)
{
    if(ent)
    {
        //get current BB from animation
        OBB_Rebuild(ent->obb, ent->bf->bb_min, ent->bf->bb_max);
        OBB_Transform(ent->obb);
    }
}


void Entity_CheckActivators(struct entity_s *ent)
{
    if((ent != NULL) && (ent->self->room != NULL))
    {
        float ppos[3];

        ppos[0] = ent->transform[12+0] + ent->transform[4+0] * ent->bf->bb_max[1];
        ppos[1] = ent->transform[12+1] + ent->transform[4+1] * ent->bf->bb_max[1];
        ppos[2] = ent->transform[12+2] + ent->transform[4+2] * ent->bf->bb_max[1];
        engine_container_p cont = ent->self->room->content->containers;
        for(; cont; cont = cont->next)
        {
            if((cont->object_type == OBJECT_ENTITY) && (cont->object))
            {
                entity_p e = (entity_p)cont->object;
                if((e->type_flags & ENTITY_TYPE_INTERACTIVE) && (e->state_flags & ENTITY_STATE_ENABLED))
                {
                    //Mat4_vec3_mul_macro(pos, e->transform, e->activation_offset);
                    if((e != ent) && (OBB_OBB_Test(e->obb, ent->obb) == 1))//(vec3_dist_sq(ent->transform+12, pos) < r))
                    {
                        Script_ExecEntity(engine_lua, ENTITY_CALLBACK_ACTIVATE, e->id, ent->id);
                    }
                }
                else if((e->type_flags & ENTITY_TYPE_PICKABLE) && (e->state_flags & ENTITY_STATE_ENABLED))
                {
                    float *v = e->transform + 12;
                    float r = e->activation_offset[3];
                    r *= r;
                    if((e != ent) && ((v[0] - ppos[0]) * (v[0] - ppos[0]) + (v[1] - ppos[1]) * (v[1] - ppos[1]) < r) &&
                                      (v[2] + 32.0 > ent->transform[12+2] + ent->bf->bb_min[2]) && (v[2] - 32.0 < ent->transform[12+2] + ent->bf->bb_max[2]))
                    {
                        Script_ExecEntity(engine_lua, ENTITY_CALLBACK_ACTIVATE, e->id, ent->id);
                    }
                }
            }
        }
    }
}


int  Entity_Activate(struct entity_s *entity_object, struct entity_s *entity_activator, uint16_t trigger_mask, uint16_t trigger_op, uint16_t trigger_lock, uint16_t trigger_timer)
{
    int activation_state = ENTITY_TRIGGERING_NOT_READY;
    if(!((entity_object->trigger_layout & ENTITY_TLAYOUT_LOCK) >> 6))           // Ignore activation, if activity lock is set.
    {
        int activator_id = (entity_activator) ? (entity_activator->id) : (-1);
        // Get current trigger layout.
        uint16_t mask = entity_object->trigger_layout & ENTITY_TLAYOUT_MASK;
        uint16_t event = (entity_object->trigger_layout & ENTITY_TLAYOUT_EVENT) >> 5;

        // Apply trigger mask to entity mask.
        if(trigger_op == TRIGGER_OP_XOR)
        {
            mask ^= trigger_mask;       // Switch cases
        }
        else
        {
            mask |= trigger_mask;       // Other cases
        }

        // Full entity mask (11111) is always a reason to activate an entity.
        // If mask is not full, entity won't activate - no exclusions.

        if((mask == 0x1F) && (event == 0))
        {
            activation_state = Script_ExecEntity(engine_lua, ENTITY_CALLBACK_ACTIVATE, entity_object->id, activator_id);
            event = 1;
        }
        else if((mask != 0x1F) && (event == 1))
        {
            activation_state = Script_ExecEntity(engine_lua, ENTITY_CALLBACK_DEACTIVATE, entity_object->id, activator_id);
            event = 0;
        }

        // Update trigger layout.
        entity_object->trigger_layout &= ~(uint8_t)(ENTITY_TLAYOUT_EVENT);      // event - 00100000
        entity_object->trigger_layout ^= ((uint8_t)event) << 5;
        entity_object->trigger_layout &= ~(uint8_t)(ENTITY_TLAYOUT_MASK);       // mask  - 00011111
        entity_object->trigger_layout ^= (uint8_t)mask;
        if(activation_state == ENTITY_TRIGGERING_ACTIVATED)
        {
            entity_object->trigger_layout &= ~(uint8_t)(ENTITY_TLAYOUT_LOCK);   // lock  - 01000000
            entity_object->trigger_layout ^= ((uint8_t)trigger_lock) << 6;
        }
        entity_object->timer = trigger_timer;                                   // Engage timer.
    }

    return activation_state;
}


int  Entity_Deactivate(struct entity_s *entity_object, struct entity_s *entity_activator)
{
    int activation_state = ENTITY_TRIGGERING_NOT_READY;
    if(!((entity_object->trigger_layout & ENTITY_TLAYOUT_LOCK) >> 6))           // Ignore deactivation, if activity lock is set.
    {
        int top = lua_gettop(engine_lua);
        int activator_id = (entity_activator) ? (entity_activator->id) : (-1);
        // Get current trigger layout.
        uint16_t event = (entity_object->trigger_layout & ENTITY_TLAYOUT_EVENT) >> 5;

        // Execute entity deactivation function, only if activation was previously set.
        if(event == 1)
        {
            activation_state = Script_ExecEntity(engine_lua, ENTITY_CALLBACK_DEACTIVATE, entity_object->id, activator_id);

            // Activation mask and timer are forced to zero when entity is deactivated.
            // Activity lock is ignored, since it can't be raised by antitriggers.
            // Update trigger layout.
            if(activation_state != ENTITY_TRIGGERING_NOT_READY)
            {
                entity_object->trigger_layout = 0x00U;
            }
            entity_object->timer = 0.0f;
        }

        lua_settop(engine_lua, top);
    }

    return activation_state;
}


void Entity_MoveForward(entity_p ent, float dist)
{
    ent->transform[12] += ent->transform[4] * dist;
    ent->transform[13] += ent->transform[5] * dist;
    ent->transform[14] += ent->transform[6] * dist;
}


void Entity_MoveStrafe(entity_p ent, float dist)
{
    ent->transform[12] += ent->transform[0] * dist;
    ent->transform[13] += ent->transform[1] * dist;
    ent->transform[14] += ent->transform[2] * dist;
}


void Entity_MoveVertical(entity_p ent, float dist)
{
    ent->transform[12] += ent->transform[8] * dist;
    ent->transform[13] += ent->transform[9] * dist;
    ent->transform[14] += ent->transform[10] * dist;
}
