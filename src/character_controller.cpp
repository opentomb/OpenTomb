
#include <stdlib.h>

#include "core/vmath.h"
#include "core/system.h"
#include "core/console.h"
#include "core/polygon.h"
#include "core/obb.h"
#include "render/render.h"
#include "script/script.h"
#include "physics/ragdoll.h"

#include "vt/tr_versions.h"
#include "audio.h"
#include "room.h"
#include "world.h"
#include "character_controller.h"
#include "engine.h"
#include "entity.h"
#include "skeletal_model.h"
#include "resource.h"
#include "engine_string.h"
#include "game.h"
#include "controls.h"
#include "mesh.h"

void Character_CollisionCallback(struct entity_s *ent, struct collision_node_s *cn);

void Character_Create(struct entity_s *ent)
{
    if(ent && !ent->character)
    {
        character_p ret;
        const collision_result_t zero_result = {0};

        ret = (character_p)malloc(sizeof(character_t));
        ret->state_func = NULL;
        ret->set_key_anim_func = NULL;
        ret->ent = ent;
        ent->character = ret;
        ret->height_info.self = ent->self;
        ent->dir_flag = ENT_STAY;
        ent->no_anim_pos_autocorrection = 0x00;

        ret->target_id = ENTITY_ID_NONE;
        ret->hair_count = 0;
        ret->path_dist = 0;
        ret->path_target = NULL;
        ret->hairs = NULL;
        ret->ragdoll = NULL;
        ret->ai_zone = 0;
        ret->ai_zone_type = ZONE_TYPE_ALL;

        ret->bone_head = 0x00;
        ret->bone_torso = 0x00;
        ret->bone_l_hand_start = 0x00;
        ret->bone_l_hand_end = 0x00;
        ret->bone_r_hand_start = 0x00;
        ret->bone_r_hand_end = 0x00;
        ret->weapon_current_state = 0x00;
        ret->current_weapon = 0;

        ret->state.floor_collide = 0x00;
        ret->state.ceiling_collide = 0x00;
        ret->state.wall_collide = 0x00;
        ret->state.slide = 0x00;
        ret->state.step_z = 0x00;
        ret->state.uw_current = 0x00;
        ret->state.attack = 0x00;
        ret->state.dead = 0x00;
        ret->state.ragdoll = 0x00;
        ret->state.burn = 0x00;
        ret->state.crouch = 0x00;
        ret->state.sprint = 0x00;
        ret->state.tightrope = 0x00;

        ret->cmd.action = 0x00;
        ret->cmd.crouch = 0x00;
        ret->cmd.flags = 0x00;
        ret->cmd.jump = 0x00;
        ret->cmd.roll = 0x00;
        ret->cmd.shift = 0x00;
        vec3_set_zero(ret->cmd.move);
        vec3_set_zero(ret->cmd.rot);

        ret->cam_follow_center = 0x00;
        ret->linear_speed_mult = DEFAULT_CHARACTER_SPEED_MULT;
        ret->rotate_speed_mult = 1.0f;
        ret->min_step_up_height = DEFAULT_MIN_STEP_UP_HEIGHT;
        ret->max_climb_height = DEFAULT_CLIMB_UP_HEIGHT;
        ret->max_step_up_height = DEFAULT_MAX_STEP_UP_HEIGHT;
        ret->fall_down_height = DEFAULT_FALL_DOWN_HEIGHT;
        ret->critical_slant_z_component = DEFAULT_CRITICAL_SLANT_Z_COMPONENT;
        ret->critical_wall_component = DEFAULT_CRITICAL_WALL_COMPONENT;
        ret->climb_r = DEFAULT_CHARACTER_CLIMB_R;
        ret->wade_depth = DEFAULT_CHARACTER_WADE_DEPTH;
        ret->swim_depth = DEFAULT_CHARACTER_SWIM_DEPTH;

        for(int i = 0; i < PARAM_LASTINDEX; i++)
        {
            ret->parameters.param[i] = 0.0;
            ret->parameters.maximum[i] = 0.0;
        }
        ret->parameters.maximum[PARAM_HIT_DAMAGE] = 9999.0;

        ret->sphere = CHARACTER_BASE_RADIUS;
        ret->climb_sensor = ent->character->climb_r;
        ret->height_info.ceiling_hit = zero_result;
        ret->height_info.floor_hit = zero_result;
        ret->height_info.water = 0x00;

        ret->climb.edge_obj = NULL;
        ret->climb.edge_z_ang = 0.0f;
        ret->climb.can_hang = 0x00;
        ret->climb.next_z_space = 0.0;
        ret->climb.height_info = 0x00;
        ret->climb.edge_hit = 0x00;
        ret->climb.wall_hit = 0x00;
        ret->forvard_size = 48.0;                                               ///@FIXME: magick number
        ret->height = CHARACTER_BASE_HEIGHT;

        ret->traversed_object = NULL;

        ent->self->collision_group = COLLISION_GROUP_CHARACTERS;
        ent->self->collision_mask = COLLISION_GROUP_STATIC_ROOM | COLLISION_GROUP_STATIC_OBLECT | COLLISION_GROUP_KINEMATIC |
                                    COLLISION_GROUP_CHARACTERS | COLLISION_GROUP_DYNAMICS | COLLISION_GROUP_DYNAMICS_NI | COLLISION_GROUP_TRIGGERS;
        Physics_SetCollisionGroupAndMask(ent->physics, ent->self->collision_group, ent->self->collision_mask);
        Physics_CreateGhosts(ent->physics, ent->bf, NULL);
        Entity_GhostUpdate(ent);
    }
}

void Character_Delete(struct entity_s *ent)
{
    character_p actor = ent->character;

    if(actor == NULL)
    {
        return;
    }

    actor->path_dist = 0;
    actor->path[0] = NULL;
    actor->path_target = NULL;
    actor->ent = NULL;
    if(actor->hairs)
    {
        for(int i = 0; i < actor->hair_count; i++)
        {
            Hair_Delete(actor->hairs[i]);
            actor->hairs[i] = NULL;
        }
        free(actor->hairs);
        actor->hairs = NULL;
        actor->hair_count = 0;
    }

    if(actor->ragdoll)
    {
        Ragdoll_DeleteSetup(actor->ragdoll);
        actor->ragdoll = NULL;
    }

    actor->height_info.water = 0x00;
    actor->climb.edge_hit = 0x00;

    free(ent->character);
    ent->character = NULL;
}


void Character_Update(struct entity_s *ent)
{
    const uint16_t mask = ENTITY_STATE_ENABLED | ENTITY_STATE_ACTIVE;
    if(mask == (ent->state_flags & mask))
    {
        if(ent->character->cmd.action && (ent->type_flags & ENTITY_TYPE_TRIGGER_ACTIVATOR))
        {
            Entity_CheckActivators(ent);
        }
        if(Character_GetParam(ent, PARAM_HEALTH) <= 0.0f)
        {
            ent->character->state.dead = 1;                                     // Kill, if no HP.
        }
        Character_GoToPathTarget(ent);
        Character_ApplyCommands(ent);

        Entity_ProcessSector(ent);
        Character_UpdateParams(ent);
        Entity_CheckCollisionCallbacks(ent);

        for(int h = 0; h < ent->character->hair_count; h++)
        {
            Hair_Update(ent->character->hairs[h], ent->physics);
        }

        if(ent->character->state.ragdoll && ent->character->ragdoll &&
           !(ent->type_flags & ENTITY_TYPE_DYNAMIC) &&
            Ragdoll_Create(ent->physics, ent->bf, ent->character->ragdoll))
        {
            ent->type_flags |= ENTITY_TYPE_DYNAMIC;
        }
    }
}


void Character_UpdatePath(struct entity_s *ent, struct room_sector_s *target)
{
    if(ent->character && ent->current_sector && ent->current_sector->box && target && target->box)
    {
        const int buf_size = sizeof(room_box_p) * World_GetRoomBoxesCount();
        room_box_p *path = (room_box_p*)Sys_GetTempMem(buf_size);
        box_validition_options_t op;
        op.zone = ent->character->ai_zone;
        op.zone_type = (ent->move_type == MOVE_FLY) ? (ZONE_TYPE_FLY) : (ent->character->ai_zone_type);
        op.zone_alt = ent->self->room->is_swapped;
        op.step_up = (ent->character->max_step_up_height > ent->character->max_climb_height) ? (ent->character->max_step_up_height) : (ent->character->max_climb_height);
        op.step_down = ent->character->fall_down_height;
        int dist = Room_FindPath(path, World_GetRoomBoxesCount(), ent->current_sector, target, &op);
        const int max_dist = sizeof(ent->character->path) / sizeof(ent->character->path[0]);
        ent->character->path_dist = (dist > max_dist) ? (max_dist) : dist;

        for(int i = 0; i < ent->character->path_dist; ++i)
        {
            ent->character->path[i] = path[dist - i - 1];
        }

        Sys_ReturnTempMem(buf_size);
    }
}


void Character_FixByBox(struct entity_s *ent, struct room_box_s *curr_box, struct room_box_s *next_box)
{
    float r = ent->bf->bone_tags->mesh_base->radius;
    int32_t fix_x = 0;
    int32_t fix_y = 0;

    if(ent->transform[12 + 0] + r > curr_box->bb_max[0])
    {
        fix_x = curr_box->bb_max[0] - ent->transform[12 + 0] - r;
    }
    else if(ent->transform[12 + 0] - r < curr_box->bb_min[0])
    {
        fix_x = curr_box->bb_min[0] - ent->transform[12 + 0] + r;
    }

    if(ent->transform[12 + 1] + r > curr_box->bb_max[1])
    {
        fix_y = curr_box->bb_max[1] - ent->transform[12 + 1] - r;
    }
    else if(ent->transform[12 + 1] - r < curr_box->bb_min[1])
    {
        fix_y = curr_box->bb_min[1] - ent->transform[12 + 1] + r;
    }

    if(!next_box)
    {
        ent->transform[12 + 0] += fix_x;
        ent->transform[12 + 1] += fix_y;
    }
    else if(next_box && (fix_x || fix_y))
    {
        float min = (curr_box->bb_min[0] < next_box->bb_min[0]) ? (curr_box->bb_min[0]) : (next_box->bb_min[0]);
        float max = (curr_box->bb_max[0] > next_box->bb_max[0]) ? (curr_box->bb_max[0]) : (next_box->bb_max[0]);
        if(fix_x && ((ent->transform[12 + 0] + r > max) || (ent->transform[12 + 0] - r < min)))
        {
            ent->transform[12 + 0] += fix_x;
        }
        
        min = (curr_box->bb_min[1] < next_box->bb_min[1]) ? (curr_box->bb_min[1]) : (next_box->bb_min[1]);
        max = (curr_box->bb_max[1] > next_box->bb_max[1]) ? (curr_box->bb_max[1]) : (next_box->bb_max[1]);
        if(fix_y && ((ent->transform[12 + 1] + r > max) || (ent->transform[12 + 1] - r < min)))
        {
            ent->transform[12 + 1] += fix_y;
        }
    }
}


void Character_GoToPathTarget(struct entity_s *ent)
{
    if(ent->character && ent->current_sector && ent->current_sector->box &&
       ent->character->path_target && (ent->character->path_dist > 0))
    {
        float dir[4];
        ent->character->cmd.rot[0] = 0;
        ent->character->cmd.move[0] = 0;
        ent->character->cmd.move[1] = 0;
        ent->character->cmd.move[2] = 0;
        ent->character->cmd.shift = 0;

        if(ent->character->path_target == ent->current_sector)
        {
            ent->character->path_target = NULL;
            return;
        }

        Character_FixByBox(ent, ent->character->path[0], (ent->character->path_dist > 1) ? (ent->character->path[1]) : (NULL));

        if(ent->current_sector->box->id != ent->character->path[0]->id)
        {
            Character_UpdatePath(ent, ent->character->path_target);
            if(ent->character->path_dist == 0)
            {
                return;
            }
        }

        if(ent->character->path_dist == 1)
        {
            vec3_copy(dir, ent->character->path_target->pos);
        }
        else
        {
            if((ent->character->path_dist >= 3) && Room_IsInBox(ent->character->path[1], ent->transform + 12))
            {
                Room_GetOverlapCenter(ent->character->path[1], ent->character->path[2], dir);
            }
            else
            {
                Room_GetOverlapCenter(ent->character->path[0], ent->character->path[1], dir);
            }
        }

        vec3_sub(dir, dir, ent->transform + 12);
        vec3_norm(dir, dir[3]);

        float z = dir[0] * ent->transform[4 + 1] - dir[1] * ent->transform[4 + 0];
        ent->character->cmd.rot[0] = 0;
        if(z > 0.01f)
        {
            ent->character->cmd.rot[0] = -1;
        }
        else if(z < -0.01f)
        {
            ent->character->cmd.rot[0] = 1;
        }

        if(ent->move_type == MOVE_FLY)
        {
            float target_z = ent->character->path_target->floor + 600.0f;
            room_sector_p next_sector = Sector_GetNextSector(ent->current_sector, ent->transform + 4);
            target_z = (target_z < next_sector->floor + TR_METERING_STEP) ? (next_sector->floor + TR_METERING_STEP) : target_z;
            target_z = (target_z > next_sector->ceiling - TR_METERING_STEP) ? (next_sector->ceiling - TR_METERING_STEP) : target_z;
            if(ent->transform[12 + 2] < target_z - 64.0f)
            {
                ent->character->cmd.move[2] = 0x01;
            }
            else if(ent->transform[12 + 2] > target_z + 64.0f)
            {
                ent->character->cmd.move[2] = -0x01;
            }
        }
        else
        {
            ent->character->cmd.shift = (fabs(z) > 0.4f) || (dir[3] < 4096.0f);
        }
        ent->character->cmd.move[0] = (dir[3] > 32.0f) ? (0x01) : (0x00);
    }
}


void Character_CollisionCallback(struct entity_s *ent, struct collision_node_s *cn)
{
    for(; cn && cn->obj; cn = cn->next)
    {
        if(cn->obj->object_type == OBJECT_ENTITY)
        {
            entity_p trigger = (entity_p)cn->obj->object;
            if(trigger->callback_flags & ENTITY_CALLBACK_COLLISION)
            {
                // Activator and entity IDs are swapped in case of collision callback.
                Script_ExecEntity(engine_lua, ENTITY_CALLBACK_COLLISION, trigger->id, ent->id);
            }
            if(trigger->character && trigger->character->state.attack)
            {
                Script_EntityUpdateCollisionInfo(engine_lua, trigger->id, cn);
                Script_ExecEntity(engine_lua, ENTITY_CALLBACK_ATTACK, trigger->id, ent->id);
            }
        }
    }
}


/**
 * Calculates character speed, based on direction flag and anim linear speed
 * @param ent
 */
void Character_UpdateCurrentSpeed(struct entity_s *ent, int zeroVz)
{
    float t  = ent->anim_linear_speed;
    float vz = (zeroVz) ? (0.0) : (ent->speed[2]);

    t *= (ent->character) ? (ent->character->linear_speed_mult) : (DEFAULT_CHARACTER_SPEED_MULT);
    if(ent->dir_flag & ENT_MOVE_FORWARD)
    {
        vec3_mul_scalar(ent->speed, ent->transform + 4, t);
    }
    else if(ent->dir_flag & ENT_MOVE_BACKWARD)
    {
        vec3_mul_scalar(ent->speed, ent->transform + 4,-t);
    }
    else if(ent->dir_flag & ENT_MOVE_LEFT)
    {
        vec3_mul_scalar(ent->speed, ent->transform + 0,-t);
    }
    else if(ent->dir_flag & ENT_MOVE_RIGHT)
    {
        vec3_mul_scalar(ent->speed, ent->transform + 0, t);
    }
    else
    {
        vec3_set_zero(ent->speed);
    }

    ent->speed[2] = vz;
}

/**
 * Calculates next height info and information about next step
 * @param ent
 */
void Character_UpdateCurrentHeight(struct entity_s *ent)
{
    float from[3], *v;
    height_info_p hi = &ent->character->height_info;

    v = ent->bf->bone_tags[0].transform + 12;
    Mat4_vec3_mul_macro(from, ent->transform, v);
    Character_GetHeightInfo(from, hi, ent->character->height);
}

/**
 * Start position are taken from ent->transform
 */
void Character_GetHeightInfo(float pos[3], struct height_info_s *fc, float v_offset)
{
    float from[3], to[3];
    room_p r = (fc->self) ? (fc->self->room) : (NULL);
    room_sector_p rs;

    fc->floor_hit.hit = 0x00;
    fc->ceiling_hit.hit = 0x00;
    fc->water = 0x00;
    fc->quicksand = 0x00;
    fc->transition_level = 32512.0;

    r = World_FindRoomByPosCogerrence(pos, r);
    if(r)
    {
        rs = Room_GetSectorXYZ(r, pos);                                         // if r != NULL then rs can not been NULL!!!
        if(r->content->room_flags & TR_ROOM_FLAG_WATER)                         // in water - go up
        {
            while(rs->room_above)
            {
                rs = Room_GetSectorRaw(rs->room_above->real_room, rs->pos);
                if((rs->owner_room->content->room_flags & TR_ROOM_FLAG_WATER) == 0x00)        // find air
                {
                    fc->transition_level = (float)rs->floor;
                    fc->water = 0x01;
                    break;
                }
            }
        }
        else if(r->content->room_flags & TR_ROOM_FLAG_QUICKSAND)
        {
            while(rs->room_above)
            {
                rs = Room_GetSectorRaw(rs->room_above->real_room, rs->pos);
                if((rs->owner_room->content->room_flags & TR_ROOM_FLAG_QUICKSAND) == 0x00)    // find air
                {
                    fc->transition_level = (float)rs->floor;
                    if(fc->transition_level - fc->floor_hit.point[2] > v_offset)
                    {
                        fc->quicksand = 0x02;
                    }
                    else
                    {
                        fc->quicksand = 0x01;
                    }
                    break;
                }
            }
        }
        else                                                                    // in air - go down
        {
            while(rs->room_below)
            {
                rs = Room_GetSectorRaw(rs->room_below->real_room, rs->pos);
                if((rs->owner_room->content->room_flags & TR_ROOM_FLAG_WATER) != 0x00)        // find water
                {
                    fc->transition_level = (float)rs->ceiling;
                    fc->water = 0x01;
                    break;
                }
                else if((rs->owner_room->content->room_flags & TR_ROOM_FLAG_QUICKSAND) != 0x00)// find water
                {
                    fc->transition_level = (float)rs->ceiling;
                    if(fc->transition_level - fc->floor_hit.point[2] > v_offset)
                    {
                        fc->quicksand = 0x02;
                    }
                    else
                    {
                        fc->quicksand = 0x01;
                    }
                    break;
                }
            }
        }
    }

    /*
     * GET HEIGHTS
     */
    vec3_copy(from, pos);
    to[0] = from[0];
    to[1] = from[1];
    to[2] = from[2] - 8192.0f;

    Physics_RayTest(&fc->floor_hit, from ,to, fc->self, COLLISION_FILTER_HEIGHT_TEST);

    to[2] = from[2] + 4096.0f;
    Physics_RayTest(&fc->ceiling_hit, from ,to, fc->self, COLLISION_FILTER_HEIGHT_TEST);
}

/**
 * @function calculates next floor info + fantom filter + returns step info.
 * Current height info must be calculated!
 */
int Character_CheckNextStep(struct entity_s *ent, float offset[3], struct height_info_s *nfc)
{
    float pos[3], from[3], to[3], delta;
    height_info_p fc = &ent->character->height_info;
    int ret = CHARACTER_STEP_HORIZONTAL;
    ///penetration test?

    vec3_add(pos, ent->transform + 12, offset);
    Character_GetHeightInfo(pos, nfc);

    if(fc->floor_hit.hit && nfc->floor_hit.hit)
    {
        delta = nfc->floor_hit.point[2] - fc->floor_hit.point[2];
        if(fabs(delta) < SPLIT_EPSILON)
        {
            from[2] = fc->floor_hit.point[2];
            ret = CHARACTER_STEP_HORIZONTAL;                                    // horizontal
        }
        else if(delta < 0.0)                                                    // down way
        {
            delta = -delta;
            from[2] = fc->floor_hit.point[2];
            if(delta <= ent->character->min_step_up_height)
            {
                ret = CHARACTER_STEP_DOWN_LITTLE;
            }
            else if(delta <= ent->character->max_step_up_height)
            {
                ret = CHARACTER_STEP_DOWN_BIG;
            }
            else if(delta <= ent->character->height)
            {
                ret = CHARACTER_STEP_DOWN_DROP;
            }
            else
            {
                ret = CHARACTER_STEP_DOWN_CAN_HANG;
            }
        }
        else                                                                    // up way
        {
            from[2] = nfc->floor_hit.point[2];
            if(delta <= ent->character->min_step_up_height)
            {
                ret = CHARACTER_STEP_UP_LITTLE;
            }
            else if(delta <= ent->character->max_step_up_height)
            {
                ret = CHARACTER_STEP_UP_BIG;
            }
            else if(delta <= ent->character->max_climb_height)
            {
                ret = CHARACTER_STEP_UP_CLIMB;
            }
            else
            {
                ret = CHARACTER_STEP_UP_IMPOSSIBLE;
            }
        }
    }
    else if(!fc->floor_hit.hit && !nfc->floor_hit.hit)
    {
        from[2] = pos[2];
        ret = CHARACTER_STEP_HORIZONTAL;                                        // horizontal? yes no maybe...
    }
    else if(!fc->floor_hit.hit && nfc->floor_hit.hit)                           // strange case
    {
        from[2] = nfc->floor_hit.point[2];
        ret = 0x00;
    }
    else //if(fc->floor_hit && !nfc->floor_hit)                                 // bottomless
    {
        from[2] = fc->floor_hit.point[2];
        ret = CHARACTER_STEP_DOWN_CAN_HANG;
    }

    /*
     * check walls! If test is positive, than CHARACTER_STEP_UP_IMPOSSIBLE - can not go next!
     */
    from[0] = ent->transform[12 + 0];
    from[1] = ent->transform[12 + 1];
    from[2] += ent->character->climb_r;

    to[0] = pos[0];
    to[1] = pos[1];
    to[2] = from[2];

    if(Physics_RayTest(NULL, from, to, fc->self, COLLISION_FILTER_HEIGHT_TEST))
    {
        ret = CHARACTER_STEP_UP_IMPOSSIBLE;
    }

    return ret;
}

/**
 *
 * @param ent - entity
 * @param next_fc - next step floor / ceiling information
 * @return 1 if character can't run / walk next; in other cases returns 0
 */
int Character_HasStopSlant(struct entity_s *ent, height_info_p next_fc)
{
    float *pos = ent->transform + 12;
    float *v1 = ent->transform + 4;
    float *v2 = next_fc->floor_hit.normale;

    return next_fc->floor_hit.hit && (next_fc->floor_hit.point[2] > pos[2]) && (next_fc->floor_hit.normale[2] < ent->character->critical_slant_z_component) &&
           (v1[0] * v2[0] + v1[1] * v2[1] < 0.0f);
}


void Character_GetMiddleHandsPos(const struct entity_s *ent, float pos[3])
{
    float temp[3];
    const float *v1 = ent->bf->bone_tags[ent->character->bone_l_hand_end].full_transform + 12;
    const float *v2 = ent->bf->bone_tags[ent->character->bone_r_hand_end].full_transform + 12;

    temp[0] = 0.0f;
    temp[1] = 0.5f * (v1[1] + v2[1]);
    temp[2] = ((v1[2] > v2[2]) ? (v1[2]) : (v2[2]));
    Mat4_vec3_mul_macro(pos, ent->transform, temp);
}


/**
 * @param ent - entity
 * @param climb - returned climb information
 * @param test_from - where we start check height (i.e. middle hands position)
 * @param test_to - test area parameters (next pos xy, z_min)
 */
void Character_CheckClimbability(struct entity_s *ent, struct climb_info_s *climb, float test_from[3], float test_to[3])
{
    const float z_step = -0.66 * ent->character->climb_r;
    float from[3], to[3];
    float *pos = ent->transform + 12;
    float n0[4], n1[4];                                                         // planes equations
    char up_founded = 0;
    collision_result_t cb;
    //const float color[3] = {1.0, 0.0, 0.0};

    if(ent->current_sector && ent->current_sector->room_above &&
       ent->current_sector->room_above->bb_min[2] < test_from[2] + 256.0f)
    {
        Entity_MoveToRoom(ent, ent->current_sector->room_above->real_room);
    }

    climb->height_info = CHARACTER_STEP_HORIZONTAL;
    climb->can_hang = 0x00;
    climb->edge_hit = 0x00;
    climb->edge_obj = NULL;

    from[0] = test_from[0];
    from[1] = test_from[1];
    from[2] = test_from[2];
    to[0] = test_to[0];
    to[1] = test_to[1];
    to[2] = test_from[2];
    //renderer.debugDrawer->DrawLine(from, to, color, color);
    if(Physics_SphereTest(&cb, from, to, ent->character->climb_r, ent->self, COLLISION_FILTER_HEIGHT_TEST))
    {
        // NEAR WALL CASE
        if(cb.fraction > 0.0f)
        {
            climb->edge_obj = cb.obj;
            vec3_copy(n0, cb.normale);
            n0[3] = -vec3_dot(n0, cb.point);
            from[0] = to[0] = cb.point[0] - cb.normale[0];
            from[1] = to[1] = cb.point[1] - cb.normale[1];
            from[2] = cb.point[2] + ent->character->max_step_up_height;
            to[2] = test_to[2];
            //renderer.debugDrawer->DrawLine(from, to, color, color);
            if(Physics_RayTestFiltered(&cb, from, to, ent->self, COLLISION_FILTER_HEIGHT_TEST))
            {
                vec3_copy(n1, cb.normale);
                n1[3] = -vec3_dot(n1, cb.point);
                up_founded = 2;
                from[0] = test_from[0];
                from[1] = test_from[1];
                from[2] = cb.point[2];
                to[0] = test_to[0];
                to[1] = test_to[1];
                to[2] = from[2];
                while(to[2] > test_to[2])
                {
                    if(Physics_SphereTest(&cb, from, to, ent->character->climb_r, ent->self, COLLISION_FILTER_HEIGHT_TEST) && (vec3_dot(cb.normale, n1) < 0.98f))
                    {
                        vec3_copy(n0, cb.normale);
                        n0[3] = -vec3_dot(n0, cb.point);
                        break;
                    }
                    from[2] += z_step;
                    to[2] += z_step;
                }
            }
        }
    }
    else
    {
        // IN FLY CASE
        from[0] = to[0] = test_to[0];
        from[1] = to[1] = test_to[1];
        from[2] = test_from[2];
        to[2] = test_to[2];
        //renderer.debugDrawer->DrawLine(from, to, color, color);
        if(Physics_SphereTest(&cb, from, to, ent->character->climb_r, ent->self, COLLISION_FILTER_HEIGHT_TEST) && (cb.fraction > 0.0f))
        {
            vec3_copy(n0, cb.normale);
            n0[3] = -vec3_dot(n0, cb.point);
            up_founded = 1;

            // mult 0.66 is magick, but it must be less than 1.0 and greater than 0.0;
            // close to 1.0 - bad precision, good speed;
            // close to 0.0 - bad speed, bad precision;
            // close to 0.5 - middle speed, good precision
            from[0] = test_from[0];
            from[1] = test_from[1];
            from[2] = to[2] = cb.point[2];
            for(; to[2] >= test_to[2]; from[2] += z_step, to[2] += z_step)      // we can't climb under floor!
            {
                //renderer.debugDrawer->DrawLine(from, to, color, color);
                if(Physics_SphereTest(&cb, from, to, ent->character->climb_r, ent->self, COLLISION_FILTER_HEIGHT_TEST) && (cb.fraction > 0.0f))
                {
                    if(up_founded && (vec3_dist_sq(cb.normale, n0) > 0.05f))
                    {
                        vec3_copy(n1, cb.normale);
                        n1[3] = -vec3_dot(n1, cb.point);
                        climb->edge_obj = cb.obj;
                        up_founded = 2;
                        break;
                    }
                }
            }
        }
    }

    if((up_founded == 2) && ((n0[2] > 0.0f) || (n1[2] > 0.0f)))
    {
        float d, n2[4];
        // get the character plane equation
        vec3_copy(n2, ent->transform + 0);
        n2[3] = -vec3_dot(n2, pos);

        /*
         * Solve system of the linear equations by Kramer method!
         * I know - It may be slow, but it has a good precision!
         * The root is point of 3 planes intersection.
         */
        d =-n0[0] * (n1[1] * n2[2] - n1[2] * n2[1]) +
            n1[0] * (n0[1] * n2[2] - n0[2] * n2[1]) -
            n2[0] * (n0[1] * n1[2] - n0[2] * n1[1]);

        if(fabs(d) < 0.005)
        {
            return;
        }

        climb->edge_point[0] = n0[3] * (n1[1] * n2[2] - n1[2] * n2[1]) -
                               n1[3] * (n0[1] * n2[2] - n0[2] * n2[1]) +
                               n2[3] * (n0[1] * n1[2] - n0[2] * n1[1]);
        climb->edge_point[0] /= d;

        climb->edge_point[1] = n0[0] * (n1[3] * n2[2] - n1[2] * n2[3]) -
                               n1[0] * (n0[3] * n2[2] - n0[2] * n2[3]) +
                               n2[0] * (n0[3] * n1[2] - n0[2] * n1[3]);
        climb->edge_point[1] /= d;

        climb->edge_point[2] = n0[0] * (n1[1] * n2[3] - n1[3] * n2[1]) -
                               n1[0] * (n0[1] * n2[3] - n0[3] * n2[1]) +
                               n2[0] * (n0[1] * n1[3] - n0[3] * n1[1]);
        climb->edge_point[2] /= d;
        vec3_copy(climb->point, climb->edge_point);
        //renderer.debugDrawer->DrawLine(to, climb->point, color, color);
        /*
         * unclimbable edge slant %)
         */
        vec3_cross(n2, n0, n1);
        d = ent->character->critical_slant_z_component;
        d *= d * (n2[0] * n2[0] + n2[1] * n2[1] + n2[2] * n2[2]);
        if(n2[2] * n2[2] > d)
        {
            return;
        }

        /*
         * Now, let us calculate z_angle
         */
        climb->edge_hit = 0x01;

        n2[2] = n2[0];
        n2[0] = n2[1];
        n2[1] =-n2[2];
        n2[2] = 0.0;
        if(n2[0] * ent->transform[4 + 0] + n2[1] * ent->transform[4 + 1] > 0)   // direction fixing
        {
            n2[0] = -n2[0];
            n2[1] = -n2[1];
        }

        vec3_copy(climb->n, n2);
        climb->up[0] = 0.0;
        climb->up[1] = 0.0;
        climb->up[2] = 1.0;
        climb->edge_z_ang = 180.0f * atan2f(n2[0], -n2[1]) / M_PI;
        climb->edge_tan_xy[0] = -n2[1];
        climb->edge_tan_xy[1] = n2[0];
        d = sqrtf(n2[0] * n2[0] + n2[1] * n2[1]);
        climb->edge_tan_xy[0] /= d;
        climb->edge_tan_xy[1] /= d;
        climb->t[0] = climb->edge_tan_xy[0];
        climb->t[1] = climb->edge_tan_xy[1];

        // Calc hang info
        climb->next_z_space = 2.0f * ent->character->height;
        vec3_copy(from, climb->edge_point);
        vec3_copy(to, from);
        to[2] += climb->next_z_space;
        if(Physics_RayTestFiltered(&cb, from, to, ent->self, COLLISION_FILTER_HEIGHT_TEST))
        {
            climb->next_z_space = cb.point[2] - climb->edge_point[2];
            if(climb->next_z_space < 0.01f)
            {
                climb->next_z_space = 2.0 * ent->character->height;
                from[2] += fabs(climb->next_z_space);
                if(Physics_RayTestFiltered(&cb, from, to, ent->self, COLLISION_FILTER_HEIGHT_TEST))
                {
                    climb->next_z_space = cb.point[2] - climb->edge_point[2];
                }
            }
        }

        from[0] = to[0] = test_from[0];
        from[1] = to[1] = test_from[1];
        from[2] = test_from[2];
        to[2] = climb->edge_point[2] - ent->character->height;
        climb->can_hang = (Physics_RayTestFiltered(NULL, from, to, ent->self, COLLISION_FILTER_HEIGHT_TEST) ? (0x00) : (0x01));
    }
}


void Character_CheckWallsClimbability(struct entity_s *ent, struct climb_info_s *climb)
{
    float from[3], to[3], t;
    collision_result_t cb;

    climb->can_hang = 0x00;
    climb->wall_hit = 0x00;
    climb->edge_hit = 0x00;
    climb->edge_obj = NULL;

    if(ent->character->height_info.walls_climb == 0x00)
    {
        return;
    }

    // now we have wall normale in XOY plane. Let us check all flags
    if(!((ent->character->height_info.walls_climb_dir & SECTOR_FLAG_CLIMB_NORTH) && (ent->transform[4 + 1] >  0.7)) &&
       !((ent->character->height_info.walls_climb_dir & SECTOR_FLAG_CLIMB_EAST)  && (ent->transform[4 + 0] >  0.7)) &&
       !((ent->character->height_info.walls_climb_dir & SECTOR_FLAG_CLIMB_SOUTH) && (ent->transform[4 + 1] < -0.7)) &&
       !((ent->character->height_info.walls_climb_dir & SECTOR_FLAG_CLIMB_WEST)  && (ent->transform[4 + 0] < -0.7)))
    {
        return;
    }

    climb->up[0] = 0.0f;
    climb->up[1] = 0.0f;
    climb->up[2] = 1.0f;

    Character_GetMiddleHandsPos(ent, from);
    vec3_copy(to, from);
    t = ent->character->climb_r * 2.0f;
    from[0] -= ent->transform[4 + 0] * t;
    from[1] -= ent->transform[4 + 1] * t;

    t += ent->character->forvard_size + 64.0f;      //@WORKAROUND! stupid useless anim move command usages!
    to[0] += ent->transform[4 + 0] * t;
    to[1] += ent->transform[4 + 1] * t;

    to[2] -= ent->character->min_step_up_height;
    Character_CheckClimbability(ent, climb, from, to);
    to[2] += ent->character->min_step_up_height;
    if(Physics_SphereTest(&cb, from, to, ent->character->climb_r, ent->self, COLLISION_FILTER_HEIGHT_TEST))
    {
        float wn2[2] = {cb.normale[0], cb.normale[1]};

        climb->wall_hit = 0x01;
        vec3_copy(climb->point, cb.point);
        vec3_copy(climb->n, cb.normale);
        t = sqrtf(wn2[0] * wn2[0] + wn2[1] * wn2[1]);
        wn2[0] /= t;
        wn2[1] /= t;

        climb->t[0] =-wn2[1];
        climb->t[1] = wn2[0];
        climb->t[2] = 0.0f;

        if(climb->wall_hit)
        {
            from[2] -= 0.67f * ent->character->height;
            to[2] = from[2];

            if(Physics_SphereTest(NULL, from, to, ent->character->climb_r, ent->self, COLLISION_FILTER_HEIGHT_TEST))
            {
                climb->wall_hit = 0x02;
            }
        }
    }
}


void Character_SetToJump(struct entity_s *ent, float v_vertical, float v_horizontal)
{
    float t;

    if(!ent->character)
    {
        return;
    }

    // Jump length is a speed value multiplied by global speed coefficient.
    t = v_horizontal * ent->character->linear_speed_mult;

    // Calculate the direction of jump by vector multiplication.
    if(ent->dir_flag & ENT_MOVE_FORWARD)
    {
        vec3_mul_scalar(ent->speed, ent->transform + 4,  t);
    }
    else if(ent->dir_flag & ENT_MOVE_BACKWARD)
    {
        vec3_mul_scalar(ent->speed, ent->transform + 4, -t);
    }
    else if(ent->dir_flag & ENT_MOVE_LEFT)
    {
        vec3_mul_scalar(ent->speed, ent->transform + 0, -t);
    }
    else if(ent->dir_flag & ENT_MOVE_RIGHT)
    {
        vec3_mul_scalar(ent->speed, ent->transform + 0,  t);
    }
    else
    {
        // Jump speed should NOT be added to current speed, as native engine
        // fully replaces current speed with jump speed by anim command.
        //vec3_set_zero(spd);
        ent->dir_flag = ENT_MOVE_FORWARD;
    }

    ent->character->state.floor_collide = 0x00;
    ent->character->state.ceiling_collide = 0x00;
    ent->character->state.slide = 0x00;
    ent->character->state.step_z = 0x00;

    // Apply vertical speed.
    ent->speed[2] = v_vertical * ent->character->linear_speed_mult;
    ent->move_type = MOVE_FREE_FALLING;
}


void Character_Lean(struct entity_s *ent, character_command_p cmd, float max_lean)
{
    float neg_lean   = 360.0f - max_lean;
    float lean_coeff = (max_lean == 0.0f) ? (48.0f) : (max_lean * 3.0f);

    // Continously lean character, according to current left/right direction.
    if((cmd->move[1] == 0) || (max_lean == 0.0f))                               // No direction - restore straight vertical position!
    {
        if(ent->angles[2] != 0.0f)
        {
            if(ent->angles[2] < 180.0f)
            {
                ent->angles[2] -= 0.5f * (fabs(ent->angles[2]) + lean_coeff) * engine_frame_time;
                ent->angles[2] = (ent->angles[2] >= 0.0f) ? (ent->angles[2]) : (0.0f);
            }
            else
            {
                ent->angles[2] += 0.5f * (360.0f - fabs(ent->angles[2]) + lean_coeff) * engine_frame_time;
                ent->angles[2] = (ent->angles[2] >= 180.0f) ? (ent->angles[2]) : (0.0f);
            }
        }
    }
    else if(cmd->move[1] == 1) // Right direction
    {
        if(ent->angles[2] != max_lean)
        {
            if(ent->angles[2] < max_lean)   // Approaching from center
            {
                ent->angles[2] += 0.5f * (fabs(ent->angles[2]) + lean_coeff) * engine_frame_time;
                ent->angles[2] = (ent->angles[2] <= max_lean) ? (ent->angles[2]) : (max_lean);
            }
            else if(ent->angles[2] > 180.0f) // Approaching from left
            {
                ent->angles[2] += 0.5f * (360.0f - fabs(ent->angles[2]) + lean_coeff) * engine_frame_time;
                ent->angles[2] = (ent->angles[2] >= 180.0f) ? (ent->angles[2]) : (0.0f);
            }
            else    // Reduce previous lean
            {
                ent->angles[2] -= 0.5f * (fabs(ent->angles[2]) + lean_coeff) * engine_frame_time;
                ent->angles[2] = (ent->angles[2] >= 0.0f) ? (ent->angles[2]) : (0.0f);
            }
        }
    }
    else if(cmd->move[1] == -1)     // Left direction
    {
        if(ent->angles[2] != neg_lean)
        {
            if(ent->angles[2] > neg_lean)   // Reduce previous lean
            {
                ent->angles[2] -= 0.5f * (360.0f - fabs(ent->angles[2]) + lean_coeff) * engine_frame_time;
                ent->angles[2] = (ent->angles[2] >= neg_lean) ? (ent->angles[2]) : (neg_lean);
            }
            else if(ent->angles[2] < 180.0f) // Approaching from right
            {
                ent->angles[2] -= 0.5f * (fabs(ent->angles[2]) + lean_coeff) * engine_frame_time;
                if(ent->angles[2] < 0.0) ent->angles[2] += 360.0f;
            }
            else    // Approaching from center
            {
                ent->angles[2] += 0.5f * (360.0f - fabs(ent->angles[2]) + lean_coeff) * engine_frame_time;
                if(ent->angles[2] > 360.0f) ent->angles[2] -= 360.0f;
            }
        }
    }
}


void Character_LookAt(struct entity_s *ent, float target[3])
{
    const float bone_dir[3] = {0.0f, 1.0f, 0.0f};
    const float head_target_limit[4] = {0.0f, 1.0f, 0.0f, 0.273f};
    ss_animation_p anim_head_track = SSBoneFrame_GetOverrideAnim(ent->bf, ANIM_TYPE_HEAD_TRACK);
    ss_animation_p  base_anim = &ent->bf->animations;

    base_anim->anim_ext_flags &= ~ANIM_EXT_TARGET_TO;

    if(!anim_head_track)
    {
        anim_head_track = SSBoneFrame_AddOverrideAnim(ent->bf, NULL, ANIM_TYPE_HEAD_TRACK);
    }

    anim_head_track->targeting_flags = 0x0000;
    SSBoneFrame_SetTarget(anim_head_track, ent->character->bone_head, target, bone_dir);
    SSBoneFrame_SetTargetingLimit(anim_head_track, head_target_limit);

    if(SSBoneFrame_CheckTargetBoneLimit(ent->bf, anim_head_track))
    {
        anim_head_track->anim_ext_flags |= ANIM_EXT_TARGET_TO;
        if((ent->move_type == MOVE_ON_FLOOR) || (ent->move_type == MOVE_FREE_FALLING))
        {
            const float axis_mod[3] = {0.33f, 0.33f, 1.0f};
            const float target_limit[4] = {0.0f, 1.0f, 0.0f, 0.883f};

            base_anim->targeting_flags = 0x0000;
            SSBoneFrame_SetTarget(base_anim, base_anim->model->mesh_tree[ent->character->bone_head].parent, target, bone_dir);
            SSBoneFrame_SetTargetingLimit(base_anim, target_limit);
            SSBoneFrame_SetTargetingAxisMod(base_anim, axis_mod);
            base_anim->anim_ext_flags |= ANIM_EXT_TARGET_TO;
        }
    }
    else
    {
        anim_head_track->anim_ext_flags &= ~ANIM_EXT_TARGET_TO;
    }
}


void Character_ClearLookAt(struct entity_s *ent)
{
    ss_animation_p anim_head_track = SSBoneFrame_GetOverrideAnim(ent->bf, ANIM_TYPE_HEAD_TRACK);
    ent->bf->animations.anim_ext_flags &= ~ANIM_EXT_TARGET_TO;
    if(anim_head_track)
    {
        anim_head_track->anim_ext_flags &= ~ANIM_EXT_TARGET_TO;
    }
}


/*
 * MOVE IN DIFFERENCE CONDITIONS
 */
int Character_MoveOnFloor(struct entity_s *ent)
{
    float norm_move_xy_len, t, *pos = ent->transform + 12;
    float tv[3], move[3], norm_move_xy[2];

    if(!ent->character)
    {
        return 0;
    }

    /*
     * init height info structure
     */
    ent->character->state.floor_collide = 0x00;
    ent->character->state.ceiling_collide = 0x00;
    ent->character->state.wall_collide = 0x00;
    ent->character->state.slide = 0x00;
    ent->character->state.step_z = 0x00;

    t = ent->anim_linear_speed * ent->character->linear_speed_mult;
    ent->angles[0] += ROT_SPEED_LAND * 60.0f * ent->character->rotate_speed_mult * engine_frame_time * (float)ent->character->cmd.rot[0];
    Entity_UpdateTransform(ent); // apply rotations

    if(ent->dir_flag & ENT_MOVE_FORWARD)
    {
        vec3_mul_scalar(ent->speed, ent->transform + 4, t);
    }
    else if(ent->dir_flag & ENT_MOVE_BACKWARD)
    {
        vec3_mul_scalar(ent->speed, ent->transform + 4,-t);
    }
    else if(ent->dir_flag & ENT_MOVE_LEFT)
    {
        vec3_mul_scalar(ent->speed, ent->transform + 0,-t);
    }
    else if(ent->dir_flag & ENT_MOVE_RIGHT)
    {
        vec3_mul_scalar(ent->speed, ent->transform + 0, t);
    }
    else
    {
        vec3_set_zero(ent->speed);
        //ent->dir_flag = ENT_MOVE_FORWARD;
    }

    /*
     * do on floor move
     */
    vec3_mul_scalar(move, ent->speed, engine_frame_time);
    t = vec3_abs(move);

    norm_move_xy[0] = move[0];
    norm_move_xy[1] = move[1];
    norm_move_xy_len = sqrtf(move[0] * move[0] + move[1] * move[1]);
    if(norm_move_xy_len > 0.2f * t)
    {
        norm_move_xy[0] /= norm_move_xy_len;
        norm_move_xy[1] /= norm_move_xy_len;
    }
    else
    {
        norm_move_xy_len = 32512.0f;
        norm_move_xy[0] = 0.0f;
        norm_move_xy[1] = 0.0f;
    }

    Entity_GhostUpdate(ent);
    vec3_add(pos, pos, move);
    Entity_FixPenetrations(ent, Character_CollisionCallback, move, COLLISION_FILTER_CHARACTER);
    Character_UpdateCurrentHeight(ent);

    // check micro gaps cases
    if(!ent->character->height_info.floor_hit.hit || (pos[2] >= ent->character->height_info.floor_hit.point[2] + ent->character->fall_down_height))
    {
        tv[0] = pos[0];
        tv[1] = pos[1];
        tv[2] = pos[2] - ent->character->fall_down_height;
        move[0] = pos[0];
        move[1] = pos[1];
        move[2] = pos[2] + 24.0f;
        if(Physics_SphereTest(&ent->character->height_info.floor_hit, move, tv, 16.0f, ent->self, COLLISION_FILTER_HEIGHT_TEST))
        {
            ent->character->height_info.floor_hit.normale[0] = 0.0f;
            ent->character->height_info.floor_hit.normale[1] = 0.0f;
            ent->character->height_info.floor_hit.normale[2] = 1.0f;
        }
    }

    // check move type
    if(ent->character->height_info.floor_hit.hit &&
       (pos[2] < ent->character->height_info.floor_hit.point[2] + ent->character->fall_down_height) &&
       (ent->speed[2] <= 0.0f))
    {
        ent->character->state.floor_collide = 0x01;
        vec3_copy(tv, ent->character->height_info.floor_hit.normale);

        if((tv[2] > 0.02) && (tv[2] < ent->character->critical_slant_z_component))
        {
            float ang;
            tv[2] = -tv[2];
            t = ent->character->linear_speed_mult * DEFAULT_CHARACTER_SLIDE_SPEED_MULT;
            vec3_mul_scalar(ent->speed, tv, t);                                 // slide down direction
            ang = 180.0f * atan2f(tv[0], -tv[1]) / M_PI;                        // from -180 deg to +180 deg
            //ang = (ang < 0.0) ? (ang + 360.0) : (ang);
            t = tv[0] * ent->transform[4 + 0] + tv[1] * ent->transform[4 + 1];
            if(t >= 0.0f)
            {
                ent->character->state.slide = CHARACTER_SLIDE_FRONT;
                ent->angles[0] = ang + 180.0f;
                // front forward slide down
            }
            else
            {
                ent->character->state.slide = CHARACTER_SLIDE_BACK;
                ent->angles[0] = ang;
                // back forward slide down
            }
            Entity_UpdateTransform(ent);
        }

        {
            t = pos[2] - ent->character->height_info.floor_hit.point[2];
            if(t < 0.0f)
            {
                pos[2] = ent->character->height_info.floor_hit.point[2];
                ent->character->state.floor_collide = 0x01;
                ent->character->state.step_z = (t < -ent->character->min_step_up_height) ? (0x01) : (0x00);
                pos[2] = ent->character->height_info.floor_hit.point[2];
            }
            else if(t > ent->character->min_step_up_height)
            {
                ent->character->state.step_z = 0x02;
                pos[2] -= engine_frame_time * 2400.0f;                          ///@FIXME: magick
                pos[2] = (pos[2] >= ent->character->height_info.floor_hit.point[2]) ? (pos[2]) : (ent->character->height_info.floor_hit.point[2]);
            }
            else
            {
                pos[2] = ent->character->height_info.floor_hit.point[2];
            }
        }

        if(ent->character->height_info.floor_hit.hit && (ent->character->height_info.floor_hit.point[2] + 1.0f >= pos[2] + ent->bf->bb_min[2]))
        {
            engine_container_p cont = ent->character->height_info.floor_hit.obj;
            if((cont != NULL) && (cont->object_type == OBJECT_ENTITY))
            {
                entity_p e = (entity_p)cont->object;
                if(e->callback_flags & ENTITY_CALLBACK_STAND)
                {
                    Script_ExecEntity(engine_lua, ENTITY_CALLBACK_STAND, e->id, ent->id);
                }
            }
        }
    }
    else                                                                        // no hit to the floor
    {
        ent->character->state.slide = 0x00;
        ent->character->state.floor_collide = 0x00;
        ent->move_type = MOVE_FREE_FALLING;
        ent->speed[2] = (ent->speed[2] < 0.0f) ? (0.0) : (ent->speed[2]);
        return 2;
    }

    Entity_UpdateRoomPos(ent);

    return 1;
}


int Character_MoveFly(struct entity_s *ent)
{
    float move[3], *pos = ent->transform + 12;

    ent->character->state.slide = 0x00;
    ent->character->state.floor_collide = 0x00;
    ent->character->state.ceiling_collide = 0x00;
    ent->character->state.wall_collide = 0x00;

    if(!ent->character->state.dead)   // Block controls if character is dead.
    {
        character_command_p cmd = &ent->character->cmd;
        float dir[3] = {0.0f, 0.0f, 0.0f};
        // Calculate current speed.
        if(cmd->move[0] || cmd->move[1] || cmd->move[2])
        {
            ent->linear_speed += MAX_SPEED_UNDERWATER * INERTIA_SPEED_UNDERWATER * engine_frame_time;
            if(ent->linear_speed > MAX_SPEED_UNDERWATER)
            {
                ent->linear_speed = MAX_SPEED_UNDERWATER;
            }
        }
        else if(ent->linear_speed > 0.0f)
        {
            ent->linear_speed -= MAX_SPEED_UNDERWATER * INERTIA_SPEED_UNDERWATER * engine_frame_time;
            if(ent->linear_speed < 0.0f)
            {
                ent->linear_speed = 0.0f;
            }
        }

        if(!cmd->shift)
        {
            ent->angles[0] += ROT_SPEED_UNDERWATER * 60.0f * ent->character->rotate_speed_mult * engine_frame_time * cmd->rot[0];
            ent->angles[1]  = 0.0f;
            ent->angles[2]  = 0.0f;
            Entity_UpdateTransform(ent);
        }
        dir[0] = (cmd->shift) ? (cmd->move[1]) : (0.0f);
        dir[1] = cmd->move[0];
        dir[2] = cmd->move[2];
        Mat4_vec3_rot_macro(ent->speed, ent->transform, dir);
        vec3_mul_scalar(ent->speed, ent->speed, ent->linear_speed * ent->character->linear_speed_mult);    // OY move only!
    }
    else
    {
        ent->move_type = MOVE_FREE_FALLING;
    }

    vec3_mul_scalar(move, ent->speed, engine_frame_time);
    vec3_add(pos, pos, move);
    Entity_FixPenetrations(ent, Character_CollisionCallback, move, COLLISION_FILTER_CHARACTER);              // get horizontal collide
    Entity_UpdateRoomPos(ent);
    Character_UpdateCurrentHeight(ent);

    return 1;
}


int Character_FreeFalling(struct entity_s *ent)
{
    float move[3], g[3], *pos = ent->transform + 12;

    if(!ent->character)
    {
        return 0;
    }

    ent->character->state.slide = 0x00;
    ent->character->state.floor_collide = 0x00;
    ent->character->state.ceiling_collide = 0x00;
    ent->character->state.wall_collide = 0x00;

    float rot = ROT_SPEED_FREEFALL * 60.0f * ent->character->rotate_speed_mult * engine_frame_time * ent->character->cmd.rot[0];
    ent->angles[0] += rot;
    ent->angles[1] = 0.0f;

    Entity_UpdateTransform(ent);                                                // apply rotations

    Physics_GetGravity(g);
    vec3_add_mul(move, ent->speed, g, engine_frame_time * 0.5f);
    move[0] *= engine_frame_time;
    move[1] *= engine_frame_time;
    move[2] *= engine_frame_time;
    ent->speed[0] += g[0] * engine_frame_time;
    ent->speed[1] += g[1] * engine_frame_time;
    ent->speed[2] += g[2] * engine_frame_time;
    ent->speed[2] = (ent->speed[2] >= -FREE_FALL_SPEED_MAXIMUM) ? (ent->speed[2]) : (-FREE_FALL_SPEED_MAXIMUM);
    vec3_RotateZ(ent->speed, ent->speed, rot);

    vec3_add(pos, pos, move);
    Entity_FixPenetrations(ent, Character_CollisionCallback, move, COLLISION_FILTER_CHARACTER);              // get horizontal collide
    Entity_GhostUpdate(ent);
    Entity_UpdateRoomPos(ent);
    Character_UpdateCurrentHeight(ent);

    if(ent->self->room && (ent->self->room->content->room_flags & TR_ROOM_FLAG_WATER))
    {
        if(ent->speed[2] < 0.0f)
        {
            ent->anim_linear_speed = 0.0f;
            ent->speed[0] = 0.0f;
            ent->speed[1] = 0.0f;
        }

        float transition_level = ent->character->height_info.transition_level;
        transition_level = (World_GetVersion() < TR_II) ? (transition_level) : (transition_level - ent->character->height);
        if(!ent->character->height_info.water || (ent->current_sector->floor <= transition_level))
        {
            ent->move_type = MOVE_UNDERWATER;
            return 2;
        }
    }

    if(ent->character->height_info.ceiling_hit.hit && (ent->speed[2] > 0.0f))
    {
        if(ent->character->height_info.ceiling_hit.point[2] < ent->bf->bb_max[2] + pos[2])
        {
            pos[2] = ent->character->height_info.ceiling_hit.point[2] - ent->bf->bb_max[2];
            ent->speed[2] = (ent->speed[2] > 0.0f) ? (0.0f) : (ent->speed[2]);
            ent->character->state.ceiling_collide = 0x01;
        }
    }
    if(ent->character->height_info.floor_hit.hit && (ent->speed[2] < 0.0f))     // move down
    {
        if(ent->character->height_info.floor_hit.point[2] >= pos[2] + ent->bf->bb_min[2] + move[2])
        {
            pos[2] = ent->character->height_info.floor_hit.point[2];
            ent->move_type = MOVE_ON_FLOOR;
            ent->character->state.floor_collide = 0x01;
            return 2;
        }
    }

    return 1;
}

/*
 * Monkey CLIMBING - MOVE NO Z LANDING
 */
int Character_MonkeyClimbing(struct entity_s *ent)
{
    float move[3];
    float t, *pos = ent->transform + 12;
    int ret = 1;

    ent->character->state.slide = 0x00;
    ent->character->state.floor_collide = 0x00;
    ent->character->state.ceiling_collide = 0x00;
    ent->character->state.wall_collide = 0x00;

    t = ent->anim_linear_speed * ent->character->linear_speed_mult;

    ent->angles[0] += ROT_SPEED_MONKEYSWING * 60.0f * ent->character->rotate_speed_mult * engine_frame_time * ent->character->cmd.rot[0];
    ent->angles[1] = 0.0f;
    ent->angles[2] = 0.0f;
    Entity_UpdateTransform(ent);                                                // apply rotations

    if(ent->dir_flag & ENT_MOVE_FORWARD)
    {
        vec3_mul_scalar(ent->speed, ent->transform + 4, t);
    }
    else if(ent->dir_flag & ENT_MOVE_BACKWARD)
    {
        vec3_mul_scalar(ent->speed, ent->transform + 4,-t);
    }
    else if(ent->dir_flag & ENT_MOVE_LEFT)
    {
        vec3_mul_scalar(ent->speed, ent->transform + 0,-t);
    }
    else if(ent->dir_flag & ENT_MOVE_RIGHT)
    {
        vec3_mul_scalar(ent->speed, ent->transform + 0, t);
    }
    else
    {
        vec3_set_zero(ent->speed);
    }
    ent->speed[2] = 0.0f;
    vec3_mul_scalar(move, ent->speed, engine_frame_time);

    vec3_add(pos, pos, move);
    Entity_FixPenetrations(ent, Character_CollisionCallback, move, COLLISION_FILTER_CHARACTER);              // get horizontal collide
    if(ent->character->height_info.ceiling_hit.hit && (pos[2] + ent->bf->bb_max[2] - ent->character->height_info.ceiling_hit.point[2] > - 0.33 * ent->character->min_step_up_height))
    {
        pos[2] = ent->character->height_info.ceiling_hit.point[2] - ent->bf->bb_max[2];
    }
    else
    {
        ent->move_type = MOVE_FREE_FALLING;
        ret = 2;
    }

    Entity_UpdateRoomPos(ent);

    return ret;
}

/*
 * WALLS CLIMBING - MOVE IN ZT plane
 */
int Character_WallsClimbing(struct entity_s *ent)
{
    climb_info_t *climb = &ent->character->climb;
    float move[3], t, *pos = ent->transform + 12;

    ent->character->state.slide = 0x00;
    ent->character->state.floor_collide = 0x00;
    ent->character->state.ceiling_collide = 0x00;
    ent->character->state.wall_collide = 0x00;

    Character_CheckWallsClimbability(ent, climb);
    Character_GetMiddleHandsPos(ent, move);
    if(!climb->wall_hit || (climb->edge_hit && (climb->edge_point[2] < move[2])))
    {
        if(climb->edge_hit && (ent->dir_flag != ENT_MOVE_BACKWARD))
        {
            pos[2] += climb->edge_point[2] - move[2];
        }
        return 2;
    }

    ent->angles[0] = atan2f(climb->n[0], -climb->n[1]) * 180.0f / M_PI;
    Entity_UpdateTransform(ent);
    pos[0] = climb->point[0] + climb->n[0] * ent->bf->bb_max[1];
    pos[1] = climb->point[1] + climb->n[1] * ent->bf->bb_max[1];

    if(ent->dir_flag == ENT_MOVE_RIGHT)
    {
        vec3_copy(move, climb->t);
        t = ent->anim_linear_speed * ent->character->linear_speed_mult;
    }
    else if(ent->dir_flag == ENT_MOVE_LEFT)
    {
        vec3_copy_inv(move, climb->t);
        t = ent->anim_linear_speed * ent->character->linear_speed_mult;
    }
    else
    {
        vec3_set_zero(move);
        t = 0.0f;
    }

    if(t != 0.0f)
    {
        vec3_mul_scalar(ent->speed, move, t);
        vec3_mul_scalar(move, ent->speed, engine_frame_time);
        vec3_add(pos, pos, move);
        Entity_FixPenetrations(ent, Character_CollisionCallback, move, COLLISION_FILTER_CHARACTER);          // get horizontal collide
    }

    Entity_UpdateRoomPos(ent);

    return 1;
}

/*
 * CLIMBING - MOVE NO Z LANDING
 */
int Character_Climbing(struct entity_s *ent)
{
    float move[3];
    float t, *pos = ent->transform + 12;

    ent->character->state.slide = 0x00;
    ent->character->state.floor_collide = 0x00;
    ent->character->state.ceiling_collide = 0x00;
    ent->character->state.wall_collide = 0x00;

    t = ent->anim_linear_speed * ent->character->linear_speed_mult;
    ent->angles[0] += ROT_SPEED_MONKEYSWING * 60.0f * ent->character->rotate_speed_mult * engine_frame_time * ent->character->cmd.rot[0];
    ent->angles[1] = 0.0f;
    ent->angles[2] = 0.0f;
    Entity_UpdateTransform(ent);                                                // apply rotations

    if(ent->dir_flag == ENT_MOVE_FORWARD)
    {
        vec3_mul_scalar(ent->speed, ent->transform + 4, t);
    }
    else if(ent->dir_flag == ENT_MOVE_BACKWARD)
    {
        vec3_mul_scalar(ent->speed, ent->transform + 4,-t);
    }
    else if(ent->dir_flag == ENT_MOVE_LEFT)
    {
        vec3_mul_scalar(ent->speed, ent->transform + 0,-t);
    }
    else if(ent->dir_flag == ENT_MOVE_RIGHT)
    {
        vec3_mul_scalar(ent->speed, ent->transform + 0, t);
    }
    else
    {
        vec3_set_zero(ent->speed);
        Entity_GhostUpdate(ent);
        Entity_FixPenetrations(ent, Character_CollisionCallback, NULL, COLLISION_FILTER_CHARACTER);
        return 1;
    }

    vec3_mul_scalar(move, ent->speed, engine_frame_time);
    vec3_add(pos, pos, move);
    Entity_FixPenetrations(ent, Character_CollisionCallback, move, COLLISION_FILTER_CHARACTER);              // get horizontal collide
    Entity_UpdateRoomPos(ent);

    return 1;
}

/*
 * underwater and onwater swimming has a big trouble:
 * the speed and acceleration information is absent...
 * I add some sticks to make it work for testing.
 * I thought to make export anim information to LUA script...
 */
int Character_MoveUnderWater(struct entity_s *ent)
{
    float move[3], *pos = ent->transform + 12;

    // Check current place.

    if(ent->self->room && !(ent->self->room->content->room_flags & TR_ROOM_FLAG_WATER))
    {
        ent->move_type = MOVE_FREE_FALLING;
        return 2;
    }

    ent->character->state.slide = 0x00;
    ent->character->state.floor_collide = 0x00;
    ent->character->state.ceiling_collide = 0x00;
    ent->character->state.wall_collide = 0x00;

    if(!ent->character->state.dead)   // Block controls if Lara is dead.
    {
        // Calculate current speed.
        if(ent->character->cmd.jump)
        {
            ent->linear_speed += MAX_SPEED_UNDERWATER * INERTIA_SPEED_UNDERWATER * engine_frame_time;
            if(ent->linear_speed > MAX_SPEED_UNDERWATER)
            {
                ent->linear_speed = MAX_SPEED_UNDERWATER;
            }
        }
        else if(ent->linear_speed > 0.0f)
        {
            ent->linear_speed -= MAX_SPEED_UNDERWATER * INERTIA_SPEED_UNDERWATER * engine_frame_time;
            if(ent->linear_speed < 0.0f)
            {
                ent->linear_speed = 0.0f;
            }
        }

        ent->angles[0] += ROT_SPEED_UNDERWATER * 60.0f * ent->character->rotate_speed_mult * engine_frame_time * ent->character->cmd.rot[0];
        ent->angles[1] -= ROT_SPEED_UNDERWATER * 60.0f * ent->character->rotate_speed_mult * engine_frame_time * ent->character->cmd.rot[1];
        ent->angles[2]  = 0.0f;

        if((ent->angles[1] > 70.0f) && (ent->angles[1] < 180.0f))               // Underwater angle limiter.
        {
           ent->angles[1] = 70.0f;
        }
        else if((ent->angles[1] > 180.0f) && (ent->angles[1] < 270.0f))
        {
            ent->angles[1] = 270.0f;
        }

        Entity_UpdateTransform(ent);                                            // apply rotations
        vec3_mul_scalar(ent->speed, ent->transform + 4, ent->linear_speed * ent->character->linear_speed_mult);    // OY move only!
    }
    vec3_mul_scalar(move, ent->speed, engine_frame_time);
    vec3_add(pos, pos, move);
    Entity_FixPenetrations(ent, Character_CollisionCallback, move, COLLISION_FILTER_CHARACTER);              // get horizontal collide
    Entity_UpdateRoomPos(ent);
    Character_UpdateCurrentHeight(ent);
    if(ent->character->height_info.water && (pos[2] + ent->bf->bb_max[2] >= ent->character->height_info.transition_level))
    {
        if(ent->transform[4 + 2] > 0.67f)             ///@FIXME: magick!
        {
            ent->move_type = MOVE_ON_WATER;
            //pos[2] = fc.transition_level;
            return 2;
        }
        if(!ent->character->height_info.floor_hit.hit || (ent->character->height_info.transition_level - ent->character->height_info.floor_hit.point[2] >= ent->character->height))
        {
            pos[2] = ent->character->height_info.transition_level - ent->bf->bb_max[2];
        }
    }

    return 1;
}


int Character_MoveOnWater(struct entity_s *ent)
{
    float move[3];
    float *pos = ent->transform + 12;

    ent->character->state.slide = 0x00;
    ent->character->state.floor_collide = 0x00;
    ent->character->state.ceiling_collide = 0x00;
    ent->character->state.wall_collide = 0x00;

    ent->angles[0] += ROT_SPEED_ONWATER * 60.0f * ent->character->rotate_speed_mult * engine_frame_time * ent->character->cmd.rot[0];
    ent->angles[1] = 0.0f;
    ent->angles[2] = 0.0f;
    Entity_UpdateTransform(ent);     // apply rotations

    // Calculate current speed.
    if(ent->character->cmd.move[0] || ent->character->cmd.move[1])
    {
        ent->linear_speed += MAX_SPEED_ONWATER * INERTIA_SPEED_ONWATER * engine_frame_time;
        if(ent->linear_speed > MAX_SPEED_ONWATER)
        {
            ent->linear_speed = MAX_SPEED_ONWATER;
        }
    }
    else if(ent->linear_speed > 0.0f)
    {
        ent->linear_speed -= MAX_SPEED_ONWATER * INERTIA_SPEED_ONWATER * engine_frame_time;
        if(ent->linear_speed < 0.0f)
        {
            ent->linear_speed = 0.0f;
        }
    }

    float t = ent->linear_speed * ent->linear_speed;
    if((ent->dir_flag & ENT_MOVE_FORWARD) && (ent->character->cmd.move[0] == 1))
    {
        vec3_mul_scalar(ent->speed, ent->transform + 4, t);
    }
    else if((ent->dir_flag & ENT_MOVE_BACKWARD) && (ent->character->cmd.move[0] == -1))
    {
        vec3_mul_scalar(ent->speed, ent->transform + 4,-t);
    }
    else if((ent->dir_flag & ENT_MOVE_LEFT) && (ent->character->cmd.move[1] == -1))
    {
        vec3_mul_scalar(ent->speed, ent->transform + 0,-t);
    }
    else if((ent->dir_flag & ENT_MOVE_RIGHT) && (ent->character->cmd.move[1] == 1))
    {
        vec3_mul_scalar(ent->speed, ent->transform + 0, t);
    }
    else
    {
        vec3_set_zero(ent->speed);
        Entity_GhostUpdate(ent);
        Entity_FixPenetrations(ent, Character_CollisionCallback, NULL, COLLISION_FILTER_CHARACTER);
        Entity_UpdateRoomPos(ent);
        if(ent->character->height_info.water)
        {
            pos[2] = ent->character->height_info.transition_level;
        }
        else
        {
            ent->move_type = MOVE_ON_FLOOR;
            return 2;
        }
        return 1;
    }

    /*
     * Prepare to moving
     */
    vec3_mul_scalar(move, ent->speed, engine_frame_time);
    vec3_add(pos, pos, move);
    Entity_FixPenetrations(ent, Character_CollisionCallback, move, COLLISION_FILTER_CHARACTER);              // get horizontal collide
    Entity_UpdateRoomPos(ent);
    Character_UpdateCurrentHeight(ent);
    if(ent->character->height_info.water)
    {
        pos[2] = ent->character->height_info.transition_level;
    }
    else
    {
        ent->move_type = MOVE_ON_FLOOR;
        return 2;
    }

    return 1;
}

int Character_FindTraverse(struct entity_s *ch)
{
    room_sector_p ch_s, obj_s = NULL;
    ch_s = ch->current_sector;

    if(ch_s == NULL)
    {
        return 0;
    }

    ch->character->traversed_object = NULL;

    // OX move case
    if(ch->transform[4 + 0] > 0.9f)
    {
        float pos[] = {(float)(ch_s->pos[0] + TR_METERING_SECTORSIZE), (float)(ch_s->pos[1]), 0.0f};
        obj_s = Room_GetSectorRaw(ch->self->room->real_room, pos);
    }
    else if(ch->transform[4 + 0] < -0.9f)
    {
        float pos[] = {(float)(ch_s->pos[0] - TR_METERING_SECTORSIZE), (float)(ch_s->pos[1]), 0.0f};
        obj_s = Room_GetSectorRaw(ch->self->room->real_room, pos);
    }
    // OY move case
    else if(ch->transform[4 + 1] > 0.9f)
    {
        float pos[] = {(float)(ch_s->pos[0]), (float)(ch_s->pos[1] + TR_METERING_SECTORSIZE), 0.0f};
        obj_s = Room_GetSectorRaw(ch->self->room->real_room, pos);
    }
    else if(ch->transform[4 + 1] < -0.9f)
    {
        float pos[] = {(float)(ch_s->pos[0]), (float)(ch_s->pos[1] - TR_METERING_SECTORSIZE), 0.0f};
        obj_s = Room_GetSectorRaw(ch->self->room->real_room, pos);
    }

    if(obj_s != NULL)
    {
        obj_s = Sector_GetPortalSectorTargetReal(obj_s);
        for(engine_container_p cont = obj_s->owner_room->containers; cont; cont = cont->next)
        {
            if(cont->object_type == OBJECT_ENTITY)
            {
                entity_p e = (entity_p)cont->object;
                if((e->type_flags & ENTITY_TYPE_TRAVERSE) && OBB_OBB_Test(e->obb, ch->obb, 32.0f) && (fabs(e->transform[12 + 2] - ch->transform[12 + 2]) < 1.1f))
                {
                    int oz = (ch->angles[0] + 45.0f) / 90.0f;
                    ch->angles[0] = oz * 90.0f;
                    ch->character->traversed_object = e;
                    Entity_UpdateTransform(ch);
                    return 1;
                }
            }
        }
    }

    for(engine_container_p cont = ch_s->owner_room->containers; cont; cont = cont->next)
    {
        if(cont->object_type == OBJECT_ENTITY)
        {
            entity_p e = (entity_p)cont->object;
            if((e->type_flags & ENTITY_TYPE_TRAVERSE) && OBB_OBB_Test(e->obb, ch->obb, 32.0f) && (fabs(e->transform[12 + 2] - ch->transform[12 + 2]) < 1.1f))
            {
                int oz = (ch->angles[0] + 45.0f) / 90.0f;
                ch->angles[0] = oz * 90.0f;
                ch->character->traversed_object = e;
                Entity_UpdateTransform(ch);
                return 1;
            }
        }
    }

    return 0;
}

/**
 *
 * @param rs: room sector pointer
 * @param floor: floor height
 * @return 0x01: can traverse, 0x00 can not;
 */
int Sector_AllowTraverse(struct room_sector_s *rs, float floor)
{
    float f0 = rs->floor_corners[0][2];
    if((rs->floor_corners[0][2] != f0) || (rs->floor_corners[1][2] != f0) ||
       (rs->floor_corners[2][2] != f0) || (rs->floor_corners[3][2] != f0) ||
       (rs->floor_penetration_config != TR_PENETRATION_CONFIG_SOLID) ||
       (rs->ceiling - floor < TR_METERING_SECTORSIZE))
    {
        return 0x00;
    }

    if(fabs(f0 - floor) < 1.1f)
    {
        return 0x01;
    }

    for(engine_container_p cont = rs->owner_room->real_room->containers; cont; cont = cont->next)
    {
        if(cont->object_type == OBJECT_ENTITY)
        {
            entity_p ent = (entity_p)cont->object;
            if((ent->type_flags & ENTITY_TYPE_TRAVERSE_FLOOR) &&
               (fabs(ent->transform[12 + 2] + TR_METERING_SECTORSIZE - floor) < 1.1f) &&
               (fabs(ent->transform[12 + 0] - rs->pos[0]) < 1.1f) &&
               (fabs(ent->transform[12 + 1] - rs->pos[1]) < 1.1f))
            {
                return 0x01;
            }
        }
    }

    return 0x00;
}

/**
 *
 * @param ch: character pointer
 * @param obj: traversed object pointer
 * @return: 0x01 if can traverse forvard; 0x02 if can traverse backvard; 0x03 can traverse in both directions; 0x00 - can't traverse
 */
int Character_CheckTraverse(struct entity_s *ch, struct entity_s *obj)
{
    room_sector_p ch_s  = ch->current_sector;
    room_sector_p obj_s = obj->current_sector;

    if((ch_s == NULL) || (obj_s == NULL))
    {
        return 0x00;
    }

    float floor = ch->transform[12 + 2];
    if((Sector_AllowTraverse(ch_s, floor) == 0x00) || (Sector_AllowTraverse(obj_s, floor) == 0x00))
    {
        return 0x00;
    }

    for(engine_container_p cont = obj_s->owner_room->real_room->containers; cont; cont = cont->next)
    {
        if(cont->object_type == OBJECT_ENTITY)
        {
            entity_p ent = (entity_p)cont->object;
            if((ent->type_flags & (ENTITY_TYPE_TRAVERSE | ENTITY_TYPE_TRAVERSE_FLOOR)) &&
               (fabs(ent->transform[12 + 2] - TR_METERING_SECTORSIZE - floor) < 1.1f) &&
               (fabs(ent->transform[12 + 0] - obj_s->pos[0]) < 1.1f) &&
               (fabs(ent->transform[12 + 1] - obj_s->pos[1]) < 1.1f))
            {
                return 0x00;
            }
        }
    }

    int ret = 0x00;
    room_sector_p next_s = NULL;
    const float r_test = 0.44f * TR_METERING_SECTORSIZE;
    /*
     * PUSH MOVE CHECK
     */
    // OX move case
    if(ch->transform[4 + 0] > 0.8f)
    {
        float pos[] = {(float)(obj_s->pos[0] + TR_METERING_SECTORSIZE), (float)(obj_s->pos[1]), 0.0f};
        next_s = Room_GetSectorRaw(obj_s->owner_room->real_room, pos);
    }
    else if(ch->transform[4 + 0] < -0.8f)
    {
        float pos[] = {(float)(obj_s->pos[0] - TR_METERING_SECTORSIZE), (float)(obj_s->pos[1]), 0.0f};
        next_s = Room_GetSectorRaw(obj_s->owner_room->real_room, pos);
    }
    // OY move case
    else if(ch->transform[4 + 1] > 0.8f)
    {
        float pos[] = {(float)(obj_s->pos[0]), (float)(obj_s->pos[1] + TR_METERING_SECTORSIZE), 0.0f};
        next_s = Room_GetSectorRaw(obj_s->owner_room->real_room, pos);
    }
    else if(ch->transform[4 + 1] < -0.8f)
    {
        float pos[] = {(float)(obj_s->pos[0]), (float)(obj_s->pos[1] - TR_METERING_SECTORSIZE), 0.0f};
        next_s = Room_GetSectorRaw(obj_s->owner_room->real_room, pos);
    }

    next_s = Sector_GetPortalSectorTargetReal(next_s);
    if((next_s != NULL) && (Sector_AllowTraverse(next_s, floor) == 0x01))
    {
        float from[3], to[3];
        from[0] = obj_s->pos[0];
        from[1] = obj_s->pos[1];
        from[2] = floor + 0.5f * TR_METERING_SECTORSIZE;

        to[0] = next_s->pos[0];
        to[1] = next_s->pos[1];
        to[2] = from[2];
        if(!Physics_SphereTest(NULL, from, to, r_test, obj->self, COLLISION_FILTER_HEIGHT_TEST))
        {
            ret |= 0x01;                                                        // can traverse forvard
        }
    }

    /*
     * PULL MOVE CHECK
     */
    next_s = NULL;
    // OX move case
    if(ch->transform[4 + 0] > 0.8f)
    {
        float pos[] = {(float)(ch_s->pos[0] - TR_METERING_SECTORSIZE), (float)(ch_s->pos[1]), 0.0f};
        next_s = Room_GetSectorRaw(ch_s->owner_room->real_room, pos);
    }
    else if(ch->transform[4 + 0] < -0.8f)
    {
        float pos[] = {(float)(ch_s->pos[0] + TR_METERING_SECTORSIZE), (float)(ch_s->pos[1]), 0.0f};
        next_s = Room_GetSectorRaw(ch_s->owner_room->real_room, pos);
    }
    // OY move case
    else if(ch->transform[4 + 1] > 0.8f)
    {
        float pos[] = {(float)(ch_s->pos[0]), (float)(ch_s->pos[1] - TR_METERING_SECTORSIZE), 0.0f};
        next_s = Room_GetSectorRaw(ch_s->owner_room->real_room, pos);
    }
    else if(ch->transform[4 + 1] < -0.8f)
    {
        float pos[] = {(float)(ch_s->pos[0]), (float)(ch_s->pos[1] + TR_METERING_SECTORSIZE), 0.0f};
        next_s = Room_GetSectorRaw(ch_s->owner_room->real_room, pos);
    }

    next_s = Sector_GetPortalSectorTargetReal(next_s);
    if((next_s != NULL) && (Sector_AllowTraverse(next_s, floor) == 0x01))
    {
        float from[3], to[3];
        from[0] = ch_s->pos[0];
        from[1] = ch_s->pos[1];
        from[2] = floor + 0.5f * TR_METERING_SECTORSIZE;

        to[0] = next_s->pos[0];
        to[1] = next_s->pos[1];
        to[2] = from[2];
        if(!Physics_SphereTest(NULL, from ,to, r_test, ch->self, COLLISION_FILTER_HEIGHT_TEST))
        {
            ret |= 0x02;                                                        // can traverse backvard
        }
    }

    return ret;
}

/**
 * Main character frame function
 */
void Character_ApplyCommands(struct entity_s *ent)
{
    if(ent->type_flags & ENTITY_TYPE_DYNAMIC)
    {
        return;
    }

    Character_UpdateCurrentHeight(ent);

    if((ent->character->cmd.ready_weapon != 0x00) && (ent->character->current_weapon > 0) && (ent->character->weapon_current_state == WEAPON_STATE_HIDE))
    {
        Character_SetWeaponModel(ent, ent->character->current_weapon, WEAPON_STATE_HIDE_TO_READY);
    }

    if(ent->character->state_func)
    {
        ent->character->state_func(ent, &ent->bf->animations);
    }

    if(!ent->no_move)
    {
        switch(ent->move_type)
        {
            case MOVE_KINEMATIC:
            case MOVE_STATIC_POS:
                //Entity_FixPenetrations(ent, NULL, COLLISION_FILTER_CHARACTER);
                break;

            case MOVE_ON_FLOOR:
                Character_MoveOnFloor(ent);
                break;

            case MOVE_FREE_FALLING:
                Character_FreeFalling(ent);
                break;

            case MOVE_CLIMBING:
                Character_Climbing(ent);
                break;

            case MOVE_MONKEYSWING:
                Character_MonkeyClimbing(ent);
                break;

            case MOVE_WALLS_CLIMB:
                Character_WallsClimbing(ent);
                break;

            case MOVE_UNDERWATER:
                Character_MoveUnderWater(ent);
                break;

            case MOVE_ON_WATER:
                Character_MoveOnWater(ent);
                break;

            case MOVE_FLY:
                Character_MoveFly(ent);
                break;

            default:
                ent->move_type = MOVE_ON_FLOOR;
                break;
        };
    }
}

void Character_UpdateParams(struct entity_s *ent)
{
    float speed = engine_frame_time / GAME_LOGIC_REFRESH_INTERVAL;

    if(ent->character->weapon_current_state != WEAPON_STATE_HIDE)
    {
        entity_p target = World_GetEntityByID(ent->character->target_id);
        if(target)
        {
            Character_LookAt(ent, target->obb->centre);
        }
        else
        {
            Character_ClearLookAt(ent);
        }
    }

    switch(ent->move_type)
    {
        case MOVE_ON_FLOOR:
        case MOVE_FREE_FALLING:
        case MOVE_CLIMBING:
        case MOVE_MONKEYSWING:
        case MOVE_WALLS_CLIMB:

            if((ent->character->height_info.quicksand == 0x02) &&
               (ent->move_type == MOVE_ON_FLOOR))
            {
                if(!Character_ChangeParam(ent, PARAM_AIR, -3.0f * speed))
                    Character_ChangeParam(ent, PARAM_HEALTH, -3.0f * speed);
            }
            else if(ent->character->height_info.quicksand == 0x01)
            {
                Character_ChangeParam(ent, PARAM_AIR, 3.0f * speed);
            }
            else
            {
                Character_SetParam(ent, PARAM_AIR, PARAM_ABSOLUTE_MAX);
            }

            if(ent->character->state.sprint)
            {
                Character_ChangeParam(ent, PARAM_STAMINA, -0.5f * speed);
            }
            else
            {
                Character_ChangeParam(ent, PARAM_STAMINA,  0.5f * speed);
            }
            break;

        case MOVE_ON_WATER:
            Character_ChangeParam(ent, PARAM_AIR, 3.0f * speed);
            break;

        case MOVE_UNDERWATER:
            if(!Character_ChangeParam(ent, PARAM_AIR, -speed))
            {
                if(!Character_ChangeParam(ent, PARAM_HEALTH, -3.0f * speed))
                {
                    ent->character->state.dead = 1;
                }
            }
            break;

        default:
            break;  // Add quicksand later...
    }
}


int Character_SetParamMaximum(struct entity_s *ent, int parameter, float max_value)
{
    if(!ent || !ent->character || (parameter >= PARAM_LASTINDEX))
    {
        return 0;
    }

    max_value = (max_value < 0) ? (0) : (max_value);    // Clamp max. to at least zero
    ent->character->parameters.maximum[parameter] = max_value;
    return 1;
}

int Character_SetParam(struct entity_s *ent, int parameter, float value)
{
    if(!ent || !ent->character || (parameter >= PARAM_LASTINDEX))
    {
        return 0;
    }

    float maximum = ent->character->parameters.maximum[parameter];

    value = (value >= 0) ? (value) : (maximum); // Char params can't be less than zero.
    value = (value <= maximum) ? (value) : (maximum);

    ent->character->parameters.param[parameter] = value;
    return 1;
}

float Character_GetParam(struct entity_s *ent, int parameter)
{
    if(ent && ent->character && (parameter < PARAM_LASTINDEX))
    {
        return ent->character->parameters.param[parameter];
    }

    return 0;
}

int Character_ChangeParam(struct entity_s *ent, int parameter, float value)
{
    if(!ent || !ent->character || (parameter >= PARAM_LASTINDEX))
    {
        return 0;
    }

    float maximum = ent->character->parameters.maximum[parameter];
    float current = ent->character->parameters.param[parameter];

    if((current == maximum) && (value > 0))
        return 0;

    current += value;

    if(current < 0)
    {
        ent->character->parameters.param[parameter] = 0;
        return 0;
    }
    else if(current > maximum)
    {
        ent->character->parameters.param[parameter] = ent->character->parameters.maximum[parameter];
    }
    else
    {
        ent->character->parameters.param[parameter] = current;
    }

    return 1;
}


int Character_IsTargetAccessible(struct entity_s *character, struct entity_s *target)
{
    int ret = 0;
    if(target && (target->state_flags & ENTITY_STATE_ACTIVE))
    {
        collision_result_t cs;
        float dir[3], t;
        vec3_sub(dir, target->transform + 12, character->transform + 12);
        vec3_norm(dir, t);
        t = vec3_dot(character->transform + 4, dir);
        ret = (t > 0.0f) && (!Physics_RayTest(&cs, character->obb->centre, target->obb->centre, character->self, COLLISION_FILTER_CHARACTER) || (cs.obj == target->self));
    }

    return ret;
}


struct entity_s *Character_FindTarget(struct entity_s *ent)
{
    entity_p ret = NULL;
    float max_dot = 0.0f;
    collision_result_t cs;

    for(int ri = -1; ri < ent->self->room->content->near_room_list_size; ++ri)
    {
        room_p r = (ri >= 0) ? (ent->self->room->content->near_room_list[ri]) : (ent->self->room);
        for(engine_container_p cont = r->containers; cont; cont = cont->next)
        {
            if(cont->object_type == OBJECT_ENTITY)
            {
                entity_p target = (entity_p)cont->object;
                if((target->type_flags & ENTITY_TYPE_ACTOR) && (target->state_flags & ENTITY_STATE_ACTIVE) &&
                   (!target->character || (target->character->parameters.param[PARAM_HEALTH] > 0.0f)))
                {
                    float dir[3], t;
                    vec3_sub(dir, target->transform + 12, ent->transform + 12);
                    vec3_norm(dir, t);
                    t = vec3_dot(ent->transform + 4, dir);
                    if((t > max_dot) && (!Physics_RayTest(&cs, ent->obb->centre, target->obb->centre, ent->self, COLLISION_FILTER_CHARACTER) || (cs.obj == target->self)))
                    {
                        max_dot = t;
                        ret = target;
                    }
                }
            }
        }
    }

    return ret;
}


void  Character_SetTarget(struct entity_s *ent, uint32_t target_id)
{
    if(ent && ent->character)
    {
        ent->character->target_id = target_id;
    }
}


// overrided == 0x00: no overriding;
// overrided == 0x01: overriding mesh in armed state;
// overrided == 0x02: add mesh to slot in armed state;
// overrided == 0x03: overriding mesh in disarmed state;
// overrided == 0x04: add mesh to slot in disarmed state;
///@TODO: separate mesh replacing control and animation disabling / enabling
int Character_SetWeaponModel(struct entity_s *ent, int weapon_model, int weapon_state)
{
    skeletal_model_p sm = World_GetModelByID(weapon_model);

    if(sm && (ent->bf->bone_tag_count == sm->mesh_count) && (sm->animation_count >= 4))
    {
        skeletal_model_p base_model = ent->bf->animations.model;
        ent->character->weapon_current_state = weapon_state;
        ent->character->current_weapon = weapon_model;
        if(sm->animation_count == 4)
        {
            ss_animation_p anim_rh = SSBoneFrame_GetOverrideAnim(ent->bf, ANIM_TYPE_WEAPON_RH);
            if(!anim_rh)
            {
                anim_rh = SSBoneFrame_AddOverrideAnim(ent->bf, World_GetModelByID(weapon_model), ANIM_TYPE_WEAPON_RH);
            }
            anim_rh->model = sm;
            anim_rh->onEndFrame = NULL;
            anim_rh->onFrame = Character_DoOneHandWeponFrame;
            anim_rh->next_state = WEAPON_STATE_HIDE;

            ss_animation_p anim_lh = SSBoneFrame_GetOverrideAnim(ent->bf, ANIM_TYPE_WEAPON_LH);
            if(!anim_lh)
            {
                anim_lh = SSBoneFrame_AddOverrideAnim(ent->bf, World_GetModelByID(weapon_model), ANIM_TYPE_WEAPON_LH);
            }
            anim_lh->model = sm;
            anim_lh->onEndFrame = NULL;
            anim_lh->onFrame = Character_DoOneHandWeponFrame;
            anim_lh->next_state = WEAPON_STATE_HIDE;

            anim_rh->enabled = 1;
            anim_lh->enabled = 1;
            ent->bf->bone_tags[8].alt_anim = anim_rh;
            ent->bf->bone_tags[9].alt_anim = anim_rh;
            ent->bf->bone_tags[10].alt_anim = anim_rh;
            ent->bf->bone_tags[11].alt_anim = anim_lh;
            ent->bf->bone_tags[12].alt_anim = anim_lh;
            ent->bf->bone_tags[13].alt_anim = anim_lh;
        }
        else
        {
            ss_animation_p anim_th = SSBoneFrame_GetOverrideAnim(ent->bf, ANIM_TYPE_WEAPON_TH);
            if(!anim_th)
            {
                anim_th = SSBoneFrame_AddOverrideAnim(ent->bf, World_GetModelByID(weapon_model), ANIM_TYPE_WEAPON_TH);
            }
            anim_th->model = sm;
            anim_th->onEndFrame = NULL;
            anim_th->onFrame = Character_DoTwoHandWeponFrame;
            anim_th->next_state = WEAPON_STATE_HIDE;
            SSBoneFrame_EnableOverrideAnim(ent->bf, anim_th);
        }

        for(uint16_t i = 0; i < base_model->mesh_count; i++)
        {
            ent->bf->bone_tags[i].mesh_replace = NULL;
            ent->bf->bone_tags[i].mesh_slot = NULL;
        }

        if(weapon_state != WEAPON_STATE_HIDE)
        {
            for(uint16_t i = 0; i < base_model->mesh_count; i++)
            {
                if(sm->mesh_tree[i].replace_mesh == 0x01)
                {
                    ent->bf->bone_tags[i].mesh_replace = sm->mesh_tree[i].mesh_base;
                }
                else if(sm->mesh_tree[i].replace_mesh == 0x02)
                {
                    ent->bf->bone_tags[i].mesh_slot = sm->mesh_tree[i].mesh_base;
                }
            }
        }
        else
        {
            for(uint16_t i = 0; i < base_model->mesh_count; i++)
            {
                ss_animation_p alt_anim = ent->bf->bone_tags[i].alt_anim;
                if(sm->mesh_tree[i].replace_mesh == 0x03)
                {
                    ent->bf->bone_tags[i].mesh_replace = sm->mesh_tree[i].mesh_base;
                }
                else if(sm->mesh_tree[i].replace_mesh == 0x04)
                {
                    ent->bf->bone_tags[i].mesh_slot = sm->mesh_tree[i].mesh_base;
                }
                if(alt_anim && ((alt_anim->type == ANIM_TYPE_WEAPON_TH) || (alt_anim->type == ANIM_TYPE_WEAPON_LH) || (alt_anim->type == ANIM_TYPE_WEAPON_RH)))
                {
                    ent->bf->bone_tags[i].alt_anim = NULL;
                }
            }
            for(ss_animation_p ss_anim = &ent->bf->animations; ss_anim; ss_anim = ss_anim->next)
            {
                if((ss_anim->type == ANIM_TYPE_WEAPON_TH) || (ss_anim->type == ANIM_TYPE_WEAPON_LH) || (ss_anim->type == ANIM_TYPE_WEAPON_RH))
                {
                    ss_anim->enabled = 0;
                }
            }
        }

        return 1;
    }
    else if(ent->bf->animations.model)
    {
        // do unarmed default model
        skeletal_model_p bm = ent->bf->animations.model;
        for(uint16_t i = 0; i < bm->mesh_count; i++)
        {
            ss_animation_p alt_anim = ent->bf->bone_tags[i].alt_anim;
            ent->bf->bone_tags[i].mesh_replace = NULL;
            ent->bf->bone_tags[i].mesh_slot = NULL;
            if(alt_anim && ((alt_anim->type == ANIM_TYPE_WEAPON_TH) || (alt_anim->type == ANIM_TYPE_WEAPON_LH) || (alt_anim->type == ANIM_TYPE_WEAPON_RH)))
            {
                ent->bf->bone_tags[i].alt_anim = NULL;
            }
        }
    }

    return 0;
}


int Character_DoOneHandWeponFrame(struct entity_s *ent, struct  ss_animation_s *ss_anim, float time)
{
   /* anims (TR_I - TR_V):
    * pistols:
    * 0: idle to fire;
    * 1: draw weapon (short?);
    * 2: draw weapon (full);
    * 3: fire process;
    */
    int16_t old_anim = ss_anim->next_animation;
    int16_t old_frame = ss_anim->next_frame;

    /*static float d_from[3] = {0.0f, 0.0f, 0.0f};
    static float d_to[3] = {0.0f, 0.0f, 0.0f};
    static float color[4] = {1.0f, 0.0f, 0.0f, 1.0f};
    renderer.debugDrawer->DrawLine(d_from, d_to, color, color);*/

    if(ss_anim->model->animation_count == 4)
    {
        const float bone_dir[] = {0.0f, 1.0f, 0.0f};
        float dt;
        int32_t t;
        uint16_t targeted_bone_start = (ss_anim->type == ANIM_TYPE_WEAPON_LH) ? (ent->character->bone_l_hand_start) : (ent->character->bone_r_hand_start);
        uint16_t targeted_bone_end = (ss_anim->type == ANIM_TYPE_WEAPON_LH) ? (ent->character->bone_l_hand_end) : (ent->character->bone_r_hand_end);
        entity_p target = (ent->character->target_id != ENTITY_ID_NONE) ? World_GetEntityByID(ent->character->target_id) : (NULL);
        bool silent = false;
        if(target)
        {
            float targeting_limit[4] = {0.0f, 1.0f, 0.0f, 0.224f};
            ss_anim->targeting_flags = 0x0000;
            SSBoneFrame_SetTarget(ss_anim, targeted_bone_start, target->obb->centre, bone_dir);
            if(ss_anim->type == ANIM_TYPE_WEAPON_LH)
            {
                vec3_RotateZ(targeting_limit, targeting_limit, 40.0f);
            }
            else
            {
                vec3_RotateZ(targeting_limit, targeting_limit, -40.0f);
            }
            SSBoneFrame_SetTargetingLimit(ss_anim, targeting_limit);

            if(!SSBoneFrame_CheckTargetBoneLimit(ent->bf, ss_anim))
            {
                target = NULL;
                silent = true;
            }
        }

        ss_anim->anim_ext_flags &= ~ANIM_EXT_TARGET_TO;
        switch(ss_anim->next_state)
        {
            case WEAPON_STATE_HIDE:
                if(ent->character->cmd.ready_weapon)   // ready weapon
                {
                    ss_anim->current_animation = 2;
                    ss_anim->current_frame = 0;
                    ss_anim->frame_time = 0.0f;
                    ss_anim->next_state = WEAPON_STATE_HIDE_TO_READY;
                    ent->character->weapon_current_state = WEAPON_STATE_IDLE;
                }
                break;

            case WEAPON_STATE_HIDE_TO_READY:
                ss_anim->frame_time += time;
                ss_anim->current_frame = (ss_anim->frame_time) / ss_anim->period;
                dt = ss_anim->frame_time - (float)ss_anim->current_frame * ss_anim->period;
                ss_anim->lerp = dt / ss_anim->period;
                t = ss_anim->model->animations[ss_anim->current_animation].max_frame;

                if(ss_anim->current_frame < t - 1)
                {
                    ss_anim->next_frame = (ss_anim->current_frame + 1) % t;
                    ss_anim->next_animation = ss_anim->current_animation;
                }
                else if(ss_anim->current_frame < t)
                {
                    ss_anim->next_frame = 0;
                    ss_anim->next_animation = 0;
                }
                else
                {
                    ss_anim->current_frame = 0;
                    ss_anim->current_animation = 0;
                    ss_anim->next_frame = 0;
                    ss_anim->next_animation = 0;
                    ss_anim->frame_time = 0.0;
                    ss_anim->next_state = WEAPON_STATE_IDLE;
                }
                break;

            case WEAPON_STATE_IDLE:
                ss_anim->current_frame = 0;
                ss_anim->current_animation = 0;
                ss_anim->next_frame = 0;
                ss_anim->next_animation = 0;
                ss_anim->frame_time = 0.0f;
                if(ent->character->cmd.ready_weapon)
                {
                    ss_anim->current_animation = 2;
                    ss_anim->next_animation = 2;
                    ss_anim->current_frame = ss_anim->next_frame = ss_anim->model->animations[ss_anim->current_animation].max_frame - 1;
                    ss_anim->frame_time = 0.0f;
                    ss_anim->next_state = WEAPON_STATE_IDLE_TO_HIDE;
                }
                else if((!silent && ent->character->cmd.action) || target)
                {
                    ss_anim->next_state = WEAPON_STATE_IDLE_TO_FIRE;
                }
                else
                {
                    // do nothing here, may be;
                }
                break;

            case WEAPON_STATE_FIRE_TO_IDLE:
                // Yes, same animation, reverse frames order;
                t = ss_anim->model->animations[ss_anim->current_animation].max_frame;
                ss_anim->frame_time += time;
                ss_anim->current_frame = (ss_anim->frame_time) / ss_anim->period;
                dt = ss_anim->frame_time - (float)ss_anim->current_frame * ss_anim->period;
                ss_anim->lerp = dt / ss_anim->period;
                ss_anim->current_frame = t - 1 - ss_anim->current_frame;
                if(ss_anim->current_frame > 0)
                {
                    ss_anim->next_frame = ss_anim->current_frame - 1;
                    ss_anim->next_animation = ss_anim->current_animation;
                }
                else
                {
                    ss_anim->next_frame = ss_anim->current_frame = 0;
                    ss_anim->next_animation = ss_anim->current_animation;
                    ss_anim->next_state = WEAPON_STATE_IDLE;
                }
                break;

            case WEAPON_STATE_IDLE_TO_FIRE:
                ss_anim->frame_time += time;
                ss_anim->current_frame = (ss_anim->frame_time) / ss_anim->period;
                dt = ss_anim->frame_time - (float)ss_anim->current_frame * ss_anim->period;
                ss_anim->lerp = dt / ss_anim->period;
                t = ss_anim->model->animations[ss_anim->current_animation].max_frame;

                if(ent->character->cmd.ready_weapon)
                {
                    ss_anim->current_animation = 2;
                    ss_anim->next_animation = 2;
                    ss_anim->frame_time = 0.0f;
                    ss_anim->next_state = WEAPON_STATE_IDLE_TO_HIDE;
                    break;
                }

                if(target)
                {
                    ss_anim->anim_ext_flags |= ANIM_EXT_TARGET_TO;
                    if(!ent->character->cmd.action && !ent->character->cmd.ready_weapon)
                    {
                        float max_time = (float)ss_anim->current_frame * ss_anim->period;
                        ss_anim->current_frame = t - 1;
                        if(ss_anim->frame_time > max_time)
                        {
                            ss_anim->frame_time = max_time;
                        }
                    }
                }

                if(ss_anim->current_frame < t - 1)
                {
                    ss_anim->next_frame = ss_anim->current_frame + 1;
                    ss_anim->next_animation = ss_anim->current_animation;
                }
                else if(ss_anim->current_frame < t)
                {
                    ss_anim->next_frame = 0;
                    ss_anim->next_animation = 3;
                }
                else if(!silent && ent->character->cmd.action)
                {
                    ss_anim->current_frame = 0;
                    ss_anim->next_frame = 1;
                    ss_anim->current_animation = 3;
                    ss_anim->next_animation = ss_anim->current_animation;
                    ss_anim->next_state = WEAPON_STATE_FIRE;
                }
                else
                {
                    ss_anim->frame_time = 0.0;
                    ss_anim->current_frame = ss_anim->model->animations[ss_anim->current_animation].max_frame - 1;
                    ss_anim->next_state = WEAPON_STATE_FIRE_TO_IDLE;
                }
                break;

            case WEAPON_STATE_FIRE:
                if(target)
                {
                    ss_anim->anim_ext_flags |= ANIM_EXT_TARGET_TO;
                }
                if(!silent && ent->character->cmd.action)
                {
                    // inc time, loop;
                    ss_anim->frame_time += time;
                    ss_anim->current_frame = (ss_anim->frame_time) / ss_anim->period;
                    dt = ss_anim->frame_time - (float)ss_anim->current_frame * ss_anim->period;
                    ss_anim->lerp = dt / ss_anim->period;
                    t = ss_anim->model->animations[ss_anim->current_animation].max_frame;

                    if(ss_anim->current_frame < t - 1)
                    {
                        ss_anim->next_frame = ss_anim->current_frame + 1;
                        ss_anim->next_animation = ss_anim->current_animation;
                    }
                    else if(ss_anim->current_frame < t)
                    {
                        ss_anim->next_frame = 0;
                        ss_anim->next_animation = ss_anim->current_animation;
                    }
                    else
                    {
                        if(target)
                        {
                            Script_ExecEntity(engine_lua, ENTITY_CALLBACK_SHOOT, ent->id, target->id);
                        }
                        else
                        {
                            collision_result_t cs;
                            float from[3], to[3], tr[16];
                            ss_bone_tag_p bt = ent->bf->bone_tags + targeted_bone_end;
                            Mat4_Mat4_mul(tr, ent->transform, bt->full_transform);
                            vec3_copy(from, tr + 12);
                            vec3_add_mul(to, from, tr + 8, -32768.0f);

                            //vec3_copy(d_from, from);
                            //vec3_copy(d_to, to);

                            if(Physics_RayTest(&cs, from, to, ent->self, COLLISION_FILTER_CHARACTER) && cs.obj && (cs.obj->object_type == OBJECT_ENTITY))
                            {
                                target = (entity_p)cs.obj->object;
                                Script_ExecEntity(engine_lua, ENTITY_CALLBACK_SHOOT, ent->id, target->id);
                            }
                        }

                        ss_anim->frame_time = dt;
                        ss_anim->current_frame = 0;
                        ss_anim->next_frame = 1;
                    }
                }
                else
                {
                    ss_anim->frame_time = 0.0f;
                    ss_anim->current_animation = 0;
                    ss_anim->next_animation = ss_anim->current_animation;
                    ss_anim->current_frame = ss_anim->model->animations[ss_anim->current_animation].max_frame - 1;
                    ss_anim->next_frame = (ss_anim->current_frame > 0) ? (ss_anim->current_frame - 1) : (0);
                    ss_anim->next_state = WEAPON_STATE_FIRE_TO_IDLE;
                }
                break;

            case WEAPON_STATE_IDLE_TO_HIDE:
                // Yes, same animation, reverse frames order;
                t = ss_anim->model->animations[ss_anim->current_animation].max_frame;
                ss_anim->frame_time += time;
                ss_anim->current_frame = (ss_anim->frame_time) / ss_anim->period;
                dt = ss_anim->frame_time - (float)ss_anim->current_frame * ss_anim->period;
                ss_anim->lerp = dt / ss_anim->period;
                ss_anim->current_frame = t - 1 - ss_anim->current_frame;
                if(ss_anim->current_frame > 0)
                {
                    ss_anim->next_frame = ss_anim->current_frame - 1;
                    ss_anim->next_animation = ss_anim->current_animation;
                }
                else
                {
                    ss_anim->next_frame = ss_anim->current_frame = 0;
                    ss_anim->next_animation = ss_anim->current_animation;
                    ss_anim->next_state = WEAPON_STATE_HIDE;
                    ent->character->weapon_current_state = WEAPON_STATE_HIDE;
                    Character_SetWeaponModel(ent, ent->character->current_weapon, WEAPON_STATE_HIDE);
                }
                break;
        };
    }

    if(old_anim != ss_anim->next_animation)
    {
        return 0x03;
    }
    if(old_frame != ss_anim->next_frame)
    {
        return 0x01;
    }
    return 0x00;
}


int Character_DoTwoHandWeponFrame(struct entity_s *ent, struct  ss_animation_s *ss_anim, float time)
{
   /* anims (TR_I - TR_V):
    * shotgun, rifles, crossbow, harpoon, launchers (2 handed weapons):
    * 0: idle to fire;
    * 1: draw weapon;
    * 2: fire process;
    * 3: hide weapon;
    * 4: idle to fire (targeted);
    */
    int16_t old_anim = ss_anim->next_animation;
    int16_t old_frame = ss_anim->next_frame;

    /*static float d_from[3] = {0.0f, 0.0f, 0.0f};
    static float d_to[3] = {0.0f, 0.0f, 0.0f};
    static float color[4] = {1.0f, 0.0f, 0.0f, 1.0f};
    renderer.debugDrawer->DrawLine(d_from, d_to, color, color);*/

    if(ss_anim->model->animation_count > 4)
    {
        float dt;
        int32_t t;
        entity_p target = (ent->character->target_id != ENTITY_ID_NONE) ? World_GetEntityByID(ent->character->target_id) : (NULL);
        if(target)
        {
            const float bone_dir[3] = {0.0f, 1.0f, 0.0f};
            const float targeting_limit[4] = {0.0f, 1.0f, 0.0f, 0.624f};
            ss_anim->targeting_flags = 0x0000;
            SSBoneFrame_SetTarget(ss_anim, ent->character->bone_torso, target->obb->centre, bone_dir);
            SSBoneFrame_SetTargetingLimit(ss_anim, targeting_limit);

            if(!SSBoneFrame_CheckTargetBoneLimit(ent->bf, ss_anim))
            {
                target = NULL;
            }
        }
        ss_anim->anim_ext_flags &= ~ANIM_EXT_TARGET_TO;
        Character_ClearLookAt(ent);
        //ss_anim->model->animations[ss_anim->current_animation].state_id;
        switch(ss_anim->next_state)
        {
            case WEAPON_STATE_HIDE:
                if(ent->character->cmd.ready_weapon)   // ready weapon
                {
                    ss_anim->current_animation = 1;
                    ss_anim->next_animation = 1;
                    ss_anim->current_frame = 0;
                    ss_anim->next_frame = 0;
                    ss_anim->frame_time = 0.0f;
                    ss_anim->next_state = WEAPON_STATE_HIDE_TO_READY;
                    ent->character->weapon_current_state = WEAPON_STATE_IDLE;
                }
                break;

            case WEAPON_STATE_HIDE_TO_READY:
                ss_anim->frame_time += time;
                ss_anim->current_frame = (ss_anim->frame_time) / ss_anim->period;
                dt = ss_anim->frame_time - (float)ss_anim->current_frame * ss_anim->period;
                ss_anim->lerp = dt / ss_anim->period;
                t = ss_anim->model->animations[ss_anim->current_animation].max_frame;

                if(ss_anim->current_frame < t - 1)
                {
                    ss_anim->next_frame = (ss_anim->current_frame + 1) % t;
                    ss_anim->next_animation = ss_anim->current_animation;
                }
                else if(ss_anim->current_frame < t)
                {
                    ss_anim->next_frame = 0;
                    ss_anim->next_animation = 0;
                }
                else
                {
                    ss_anim->current_frame = 0;
                    ss_anim->current_animation = 0;
                    ss_anim->next_frame = 0;
                    ss_anim->next_animation = 0;
                    ss_anim->frame_time = 0.0f;
                    ss_anim->next_state = WEAPON_STATE_IDLE;
                }
                break;

            case WEAPON_STATE_IDLE:
                ss_anim->current_frame = 0;
                ss_anim->current_animation = 0;
                ss_anim->next_frame = 0;
                ss_anim->next_animation = 0;
                ss_anim->frame_time = 0.0f;
                if(ent->character->cmd.ready_weapon)
                {
                    ss_anim->current_animation = 3;
                    ss_anim->next_animation = 3;
                    //ss_anim->current_frame = ss_anim->next_frame = 0;
                    ss_anim->frame_time = 0.0f;
                    ss_anim->next_state = WEAPON_STATE_IDLE_TO_HIDE;
                }
                else if(ent->character->cmd.action || target)
                {
                    ss_anim->next_state = WEAPON_STATE_IDLE_TO_FIRE;
                }
                else
                {
                    // do nothing here, may be;
                }
                break;

            case WEAPON_STATE_FIRE_TO_IDLE:
                // Yes, same animation, reverse frames order;
                t = ss_anim->model->animations[ss_anim->current_animation].max_frame;
                ss_anim->frame_time += time;
                ss_anim->current_frame = (ss_anim->frame_time) / ss_anim->period;
                dt = ss_anim->frame_time - (float)ss_anim->current_frame * ss_anim->period;
                ss_anim->lerp = dt / ss_anim->period;
                ss_anim->current_frame = t - 1 - ss_anim->current_frame;
                if(ss_anim->current_frame > 0)
                {
                    ss_anim->next_frame = ss_anim->current_frame - 1;
                    ss_anim->next_animation = ss_anim->current_animation;
                }
                else
                {
                    ss_anim->next_frame = ss_anim->current_frame = 0;
                    ss_anim->next_animation = ss_anim->current_animation;
                    ss_anim->next_state = WEAPON_STATE_IDLE;
                }
                break;

            case WEAPON_STATE_IDLE_TO_FIRE:
                ss_anim->frame_time += time;
                ss_anim->current_frame = (ss_anim->frame_time) / ss_anim->period;
                dt = ss_anim->frame_time - (float)ss_anim->current_frame * ss_anim->period;
                ss_anim->lerp = dt / ss_anim->period;
                t = ss_anim->model->animations[ss_anim->current_animation].max_frame;

                if(ent->character->cmd.ready_weapon)
                {
                    ss_anim->current_animation = 3;
                    ss_anim->next_animation = 3;
                    ss_anim->frame_time = 0.0;
                    ss_anim->next_state = WEAPON_STATE_IDLE_TO_HIDE;
                    break;
                }

                if(target)
                {
                    ss_anim->anim_ext_flags |= ANIM_EXT_TARGET_TO;
                    if(!ent->character->cmd.action && !ent->character->cmd.ready_weapon)
                    {
                        float max_time = (float)ss_anim->current_frame * ss_anim->period;
                        ss_anim->current_frame = t - 1;
                        if(ss_anim->frame_time > max_time)
                        {
                            ss_anim->frame_time = max_time;
                        }
                    }
                }

                if(ss_anim->current_frame < t - 1)
                {
                    ss_anim->next_frame = ss_anim->current_frame + 1;
                    ss_anim->next_animation = ss_anim->current_animation;
                }
                else if(ss_anim->current_frame < t)
                {
                    ss_anim->next_frame = 0;
                    ss_anim->next_animation = 2;
                }
                else if(ent->character->cmd.action)
                {
                    ss_anim->current_frame = 0;
                    ss_anim->next_frame = 1;
                    ss_anim->current_animation = 2;
                    ss_anim->next_animation = ss_anim->current_animation;
                    ss_anim->next_state = WEAPON_STATE_FIRE;
                }
                else
                {
                    ss_anim->frame_time = 0.0f;
                    ss_anim->current_frame = ss_anim->model->animations[ss_anim->current_animation].max_frame - 1;
                    ss_anim->next_state = WEAPON_STATE_FIRE_TO_IDLE;
                }
                break;

            case WEAPON_STATE_FIRE:
                if(target)
                {
                    ss_anim->anim_ext_flags |= ANIM_EXT_TARGET_TO;
                }
                if(ent->character->cmd.action)
                {
                    // inc time, loop;
                    ss_anim->frame_time += time;
                    ss_anim->current_frame = (ss_anim->frame_time) / ss_anim->period;
                    dt = ss_anim->frame_time - (float)ss_anim->current_frame * ss_anim->period;
                    ss_anim->lerp = dt / ss_anim->period;
                    t = ss_anim->model->animations[ss_anim->current_animation].max_frame;

                    if(ss_anim->current_frame < t - 1)
                    {
                        ss_anim->next_frame = ss_anim->current_frame + 1;
                        ss_anim->next_animation = ss_anim->current_animation;
                    }
                    else if(ss_anim->current_frame < t)
                    {
                        ss_anim->next_frame = 0;
                        ss_anim->next_animation = ss_anim->current_animation;
                    }
                    else
                    {
                        if(target)
                        {
                            Script_ExecEntity(engine_lua, ENTITY_CALLBACK_SHOOT, ent->id, target->id);
                        }
                        else
                        {
                            collision_result_t cs;
                            float from[3], to[3], tr[16];
                            ss_bone_tag_p bt = ent->bf->bone_tags + ent->character->bone_r_hand_end;
                            Mat4_Mat4_mul(tr, ent->transform, bt->full_transform);
                            vec3_copy(from, tr + 12);
                            vec3_add_mul(to, from, tr + 8, -32768.0f);

                            //vec3_copy(d_from, from);
                            //vec3_copy(d_to, to);

                            if(Physics_RayTest(&cs, from, to, ent->self, COLLISION_FILTER_CHARACTER) && cs.obj && (cs.obj->object_type == OBJECT_ENTITY))
                            {
                                target = (entity_p)cs.obj->object;
                                Script_ExecEntity(engine_lua, ENTITY_CALLBACK_SHOOT, ent->id, target->id);
                            }
                        }
                        ss_anim->frame_time = dt;
                        ss_anim->current_frame = 0;
                        ss_anim->next_frame = 1;
                    }
                }
                else
                {
                    ss_anim->frame_time = 0.0;
                    ss_anim->current_animation = 0;
                    ss_anim->next_animation = ss_anim->current_animation;
                    ss_anim->current_frame = ss_anim->model->animations[ss_anim->current_animation].max_frame - 1;
                    ss_anim->next_frame = (ss_anim->current_frame > 0) ? (ss_anim->current_frame - 1) : (0);
                    ss_anim->next_state = WEAPON_STATE_FIRE_TO_IDLE;
                }
                break;

            case WEAPON_STATE_IDLE_TO_HIDE:
                t = ss_anim->model->animations[ss_anim->current_animation].max_frame;
                ss_anim->frame_time += time;
                ss_anim->current_frame = (ss_anim->frame_time) / ss_anim->period;
                dt = ss_anim->frame_time - (float)ss_anim->current_frame * ss_anim->period;
                ss_anim->lerp = dt / ss_anim->period;
                if(ss_anim->current_frame < t - 1)
                {
                    ss_anim->next_frame = ss_anim->current_frame + 1;
                    ss_anim->next_animation = ss_anim->current_animation;
                }
                else
                {
                    ss_anim->next_frame = ss_anim->current_frame = 0;
                    ss_anim->next_animation = ss_anim->current_animation;
                    ss_anim->next_state = WEAPON_STATE_HIDE;
                    ent->character->weapon_current_state = WEAPON_STATE_HIDE;
                    Character_SetWeaponModel(ent, ent->character->current_weapon, WEAPON_STATE_HIDE);
                }
                ss_anim->anim_ext_flags &= ~ANIM_EXT_TARGET_TO;
                break;
        };
    }

    if(old_anim != ss_anim->next_animation)
    {
        return 0x03;
    }
    if(old_frame != ss_anim->next_frame)
    {
        return 0x01;
    }
    return 0x00;
}
