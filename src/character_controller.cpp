
#include <stdlib.h>

#include "core/vmath.h"
#include "core/console.h"
#include "core/polygon.h"
#include "core/obb.h"
#include "render/render.h"

#include "vt/tr_versions.h"
#include "audio.h"
#include "room.h"
#include "world.h"
#include "character_controller.h"
#include "anim_state_control.h"
#include "script.h"
#include "engine.h"
#include "physics.h"
#include "entity.h"
#include "skeletal_model.h"
#include "resource.h"
#include "engine_string.h"
#include "inventory.h"
#include "game.h"
#include "controls.h"

void Character_Create(struct entity_s *ent)
{
    if(ent && !ent->character)
    {
        character_p ret;
        const collision_result_t zero_result = {0};

        ret = (character_p)malloc(sizeof(character_t));
        //ret->platform = NULL;
        ret->state_func = NULL;
        ret->inventory = NULL;
        ret->ent = ent;
        ent->character = ret;
        ret->height_info.self = ent->self;
        ent->dir_flag = ENT_STAY;

        ret->target_id = ENTITY_ID_NONE;
        ret->hair_count = 0;
        ret->hairs = NULL;

        ret->weapon_current_state = 0x00;
        ret->current_weapon = 0;

        ret->resp.vertical_collide = 0x00;
        ret->resp.horizontal_collide = 0x00;
        ret->resp.kill = 0x00;
        ret->resp.burn = 0x00;
        ret->resp.slide = 0x00;

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

        ret->sphere = CHARACTER_BASE_RADIUS;
        ret->climb_sensor = ent->character->climb_r;
        ret->height_info.ceiling_hit = zero_result;
        ret->height_info.floor_hit = zero_result;
        ret->height_info.leg_l_floor = zero_result;
        ret->height_info.leg_r_floor = zero_result;
        ret->height_info.hand_l_floor = zero_result;
        ret->height_info.hand_r_floor = zero_result;
        ret->height_info.water = 0x00;

        ret->height_info.leg_l_index = -1;
        ret->height_info.leg_r_index = -1;
        ret->height_info.hand_l_index = -1;
        ret->height_info.hand_r_index = -1;

        ret->climb.edge_obj = NULL;
        ret->climb.edge_z_ang = 0.0f;
        ret->climb.can_hang = 0x00;
        ret->climb.next_z_space = 0.0;
        ret->climb.height_info = 0x00;
        ret->climb.edge_hit = 0x00;
        ret->climb.wall_hit = 0x00;
        ret->forvard_size = 48.0;                                               ///@FIXME: magick number
        ret->Height = CHARACTER_BASE_HEIGHT;

        ret->traversed_object = NULL;

        Physics_CreateGhosts(ent->physics, ent->bf);
    }
}

void Character_Clean(struct entity_s *ent)
{
    character_p actor = ent->character;

    if(actor == NULL)
    {
        return;
    }

    actor->ent = NULL;

    Inventory_RemoveAllItems(&actor->inventory);

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

    actor->height_info.water = 0x00;
    actor->climb.edge_hit = 0x00;

    free(ent->character);
    ent->character = NULL;
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
        vec3_mul_scalar(ent->speed, ent->transform+4, t);
    }
    else if(ent->dir_flag & ENT_MOVE_BACKWARD)
    {
        vec3_mul_scalar(ent->speed, ent->transform+4,-t);
    }
    else if(ent->dir_flag & ENT_MOVE_LEFT)
    {
        vec3_mul_scalar(ent->speed, ent->transform+0,-t);
    }
    else if(ent->dir_flag & ENT_MOVE_RIGHT)
    {
        vec3_mul_scalar(ent->speed, ent->transform+0, t);
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
    float from[3], to[3], base_z;
    height_info_p hi = &ent->character->height_info;

    hi->leg_l_floor.hit = 0x00;
    hi->leg_r_floor.hit = 0x00;
    hi->hand_l_floor.hit = 0x00;
    hi->hand_r_floor.hit = 0x00;
    to[0] = 0.0;
    to[1] = 0.0;
    to[2] = ent->bf->bone_tags[0].transform[12 + 2];
    Mat4_vec3_mul_macro(from, ent->transform, to);
    base_z = from[2];
    Character_GetHeightInfo(from, hi, ent->character->Height);

    if((hi->leg_l_index >= 0) && (hi->leg_l_index < ent->bf->bone_tag_count))
    {
        Mat4_vec3_mul(from, ent->transform, ent->bf->bone_tags[hi->leg_l_index].full_transform + 12);
        from[2] = base_z;
        vec3_copy(to, from);
        to[2] -= ent->character->Height;
        vec3_copy(hi->leg_l_floor.point, from);
        Physics_RayTest(&hi->leg_l_floor, from ,to, ent->self);
    }

    if((hi->leg_r_index >= 0) && (hi->leg_r_index < ent->bf->bone_tag_count))
    {
        Mat4_vec3_mul(from, ent->transform, ent->bf->bone_tags[hi->leg_r_index].full_transform + 12);
        from[2] = base_z;
        vec3_copy(to, from);
        to[2] -= ent->character->Height;
        vec3_copy(hi->leg_r_floor.point, from);
        Physics_RayTest(&hi->leg_r_floor, from ,to, ent->self);
    }

    if((hi->hand_l_index >= 0) && (hi->hand_l_index < ent->bf->bone_tag_count))
    {
        Mat4_vec3_mul(from, ent->transform, ent->bf->bone_tags[hi->hand_l_index].full_transform + 12);
        from[2] = base_z;
        vec3_copy(to, from);
        to[2] -= ent->character->Height;
        Physics_RayTest(&hi->hand_l_floor, from ,to, ent->self);
        vec3_copy(hi->hand_l_floor.point, from);
    }

    if((hi->hand_r_index >= 0) && (hi->hand_r_index < ent->bf->bone_tag_count))
    {
        Mat4_vec3_mul(from, ent->transform, ent->bf->bone_tags[hi->hand_r_index].full_transform + 12);
        from[2] = base_z;
        vec3_copy(to, from);
        to[2] -= ent->character->Height;
        Physics_RayTest(&hi->hand_r_floor, from ,to, ent->self);
        vec3_copy(hi->hand_r_floor.point, from);
    }
}

/*
 * Move character to the point where to platfom mowes
 */
void Character_UpdatePlatformPreStep(struct entity_s *ent)
{
#if 0
    if(ent->character->platform)
    {
        engine_container_p cont = (engine_container_p)ent->character->platform->getUserPointer();
        if(cont && (cont->object_type == OBJECT_ENTITY/* || cont->object_type == OBJECT_BULLET_MISC*/))
        {
            float trpl[16];
            ent->character->platform->getWorldTransform().getOpenGLMatrix(trpl);
#if 0
            Mat4_Mat4_mul(new_tr, trpl, ent->character->local_platform);
            vec3_copy(ent->transform + 12, new_tr + 12);
#else
            ///make something with platform rotation
            Mat4_Mat4_mul(ent->transform, trpl, ent->character->local_platform);
#endif
        }
    }
#endif
}

/*
 * Get local character transform relative platfom
 */
void Character_UpdatePlatformPostStep(struct entity_s *ent)
{
#if 0
    switch(ent->move_type)
    {
        case MOVE_ON_FLOOR:
            if(ent->character->height_info.floor_hit)
            {
                ent->character->platform = ent->character->height_info.floor_obj;
            }
            break;

        case MOVE_CLIMBING:
            if(ent->character->climb.edge_hit)
            {
                ent->character->platform = ent->character->climb.edge_obj;
            }
            break;

        default:
            ent->character->platform = NULL;
            break;
    };

    if(ent->character->platform)
    {
        engine_container_p cont = (engine_container_p)ent->character->platform->getUserPointer();
        if(cont && (cont->object_type == OBJECT_ENTITY/* || cont->object_type == OBJECT_BULLET_MISC*/))
        {
            float trpl[16];
            ent->character->platform->getWorldTransform().getOpenGLMatrix(trpl);
            /* local_platform = (global_platform ^ -1) x (global_entity); */
            Mat4_inv_Mat4_affine_mul(ent->character->local_platform, trpl, ent->transform);
        }
        else
        {
            ent->character->platform = NULL;
        }
    }
#endif
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
    r = Room_CheckFlip(r);
    if(r)
    {
        rs = Room_GetSectorXYZ(r, pos);                                         // if r != NULL then rs can not been NULL!!!
        if(r->flags & TR_ROOM_FLAG_WATER)                                       // in water - go up
        {
            while(rs->sector_above)
            {
                rs = Sector_CheckFlip(rs->sector_above);
                if((rs->owner_room->flags & TR_ROOM_FLAG_WATER) == 0x00)        // find air
                {
                    fc->transition_level = (float)rs->floor;
                    fc->water = 0x01;
                    break;
                }
            }
        }
        else if(r->flags & TR_ROOM_FLAG_QUICKSAND)
        {
            while(rs->sector_above)
            {
                rs = Sector_CheckFlip(rs->sector_above);
                if((rs->owner_room->flags & TR_ROOM_FLAG_QUICKSAND) == 0x00)    // find air
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
            while(rs->sector_below)
            {
                rs = Sector_CheckFlip(rs->sector_below);
                if((rs->owner_room->flags & TR_ROOM_FLAG_WATER) != 0x00)        // find water
                {
                    fc->transition_level = (float)rs->ceiling;
                    fc->water = 0x01;
                    break;
                }
                else if((rs->owner_room->flags & TR_ROOM_FLAG_QUICKSAND) != 0x00)// find water
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

    Physics_RayTest(&fc->floor_hit, from ,to, fc->self);

    to[2] = from[2] + 4096.0f;
    Physics_RayTest(&fc->ceiling_hit, from ,to, fc->self);
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
            else if(delta <= ent->character->Height)
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

    if(Physics_RayTest(NULL, from, to, fc->self))
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


void Character_FixPosByFloorInfoUnderLegs(struct entity_s *ent)
{
    if(ent->no_fix_all == 0)
    {
        height_info_p hi = &ent->character->height_info;
        float *pos = ent->transform + 12;
        if((hi->leg_l_index >= 0) && (hi->leg_r_index >= 0))
        {
            bool valid_l = hi->leg_l_floor.hit &&
                (hi->leg_l_floor.point[2] >= pos[2] - ent->character->fall_down_height) &&
                (hi->leg_l_floor.point[2] <= pos[2] + ent->character->max_step_up_height);

            bool valid_r = hi->leg_r_floor.hit &&
                (hi->leg_r_floor.point[2] >= pos[2] - ent->character->fall_down_height) &&
                (hi->leg_r_floor.point[2] <= pos[2] + ent->character->max_step_up_height);

            float fix[2] = {0.0f, 0.0f};
            //const float red[3] = {1.0f, 0.0f, 0.0f};
            const float R = 16.0f;
            if(!valid_r)
            {
                collision_result_t cb;
                float from[3], to[3];
                from[0] = hi->leg_r_floor.point[0];
                from[1] = hi->leg_r_floor.point[1];
                from[2] = pos[2] + ent->character->max_step_up_height;
                to[0] = from[0];
                to[1] = from[1];
                to[2] = pos[2] - ent->character->max_step_up_height;
                //renderer.debugDrawer->DrawLine(from, to, red, red);
                while((from[0] + 4.0f * R * ent->transform[0 + 0] - pos[0]) * ent->transform[0 + 0] + (from[1] + 4.0f *R * ent->transform[0 + 1] - pos[1]) * ent->transform[0 + 1] > 0.0f)
                {
                    if(Physics_SphereTest(&cb, from, to, R, ent->self))
                    {
                        fix[0] += (cb.point[0] - hi->leg_r_floor.point[0]);
                        fix[1] += (cb.point[1] - hi->leg_r_floor.point[1]);
                        break;
                    }
                    from[0] -= 1.5f * R * ent->transform[0 + 0];
                    from[1] -= 1.5f * R * ent->transform[0 + 1];
                    to[0] = from[0];
                    to[1] = from[1];
                }
            }
            if(!valid_l)
            {
                collision_result_t cb;
                float from[3], to[3];
                from[0] = hi->leg_l_floor.point[0];
                from[1] = hi->leg_l_floor.point[1];
                from[2] = ent->transform[12 + 2] + ent->character->max_step_up_height;
                to[0] = from[0];
                to[1] = from[1];
                to[2] = ent->transform[12 + 2] - ent->character->max_step_up_height;
                //renderer.debugDrawer->DrawLine(from, to, red, red);
                while((from[0] - 4.0f * R * ent->transform[0 + 0] - pos[0]) * ent->transform[0 + 0] + (from[1] - 4.0f *R * ent->transform[0 + 1] - pos[1]) * ent->transform[0 + 1] < 0.0f)
                {
                    if(Physics_SphereTest(&cb, from, to, R, ent->self))
                    {
                        fix[0] += (cb.point[0] - hi->leg_l_floor.point[0]);
                        fix[1] += (cb.point[1] - hi->leg_l_floor.point[1]);
                        break;
                    }
                    from[0] += 1.5f * R * ent->transform[0 + 0];
                    from[1] += 1.5f * R * ent->transform[0 + 1];
                    to[0] = from[0];
                    to[1] = from[1];
                }
            }
            ent->transform[12 + 0] += fix[0];
            ent->transform[12 + 1] += fix[1];
        }
    }
}


void Character_GetMiddleHandsPos(const struct entity_s *ent, float pos[3])
{
    float temp[3];
    const float *v1 = ent->bf->bone_tags[10].full_transform + 12;
    const float *v2 = ent->bf->bone_tags[13].full_transform + 12;

    vec3_add(temp, v1, v2);
    vec3_mul_scalar(temp, temp, 0.5f);
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
    if(Physics_SphereTest(&cb, from, to, ent->character->climb_r, ent->self) && (cb.fraction > 0))
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
            if(Physics_RayTestFiltered(&cb, from, to, ent->self))
            {
                vec3_copy(n1, cb.normale);
                n1[3] = -vec3_dot(n1, cb.point);
                up_founded = 2;
                if(vec3_dot(n0, n1) >= 0.98f)
                {
                    from[0] = test_from[0];
                    from[1] = test_from[1];
                    from[2] = cb.point[2];
                    to[0] = test_to[0];
                    to[1] = test_to[1];
                    to[2] = from[2];
                    while(to[2] > test_to[2])
                    {
                        if(Physics_SphereTest(&cb, from, to, ent->character->climb_r, ent->self) && (vec3_dot(cb.normale, n1) < 0.98f))
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
    }
    else
    {
        // IN FLY CASE
        from[0] = to[0] = test_to[0];
        from[1] = to[1] = test_to[1];
        from[2] = test_from[2];
        to[2] = test_to[2];
        //renderer.debugDrawer->DrawLine(from, to, color, color);
        if(Physics_SphereTest(&cb, from, to, ent->character->climb_r, ent->self) && (cb.fraction > 0.0f))
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
                if(Physics_SphereTest(&cb, from, to, ent->character->climb_r, ent->self) && (cb.fraction > 0.0f))
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
        d = sqrt(n2[0] * n2[0] + n2[1] * n2[1]);
        climb->edge_tan_xy[0] /= d;
        climb->edge_tan_xy[1] /= d;
        climb->t[0] = climb->edge_tan_xy[0];
        climb->t[1] = climb->edge_tan_xy[1];

        // Calc hang info
        climb->next_z_space = 2.0f * ent->character->Height;
        vec3_copy(from, climb->edge_point);
        vec3_copy(to, from);
        to[2] += climb->next_z_space;
        if(Physics_RayTestFiltered(&cb, from, to, ent->self))
        {
            climb->next_z_space = cb.point[2] - climb->edge_point[2];
            if(climb->next_z_space < 0.01f)
            {
                climb->next_z_space = 2.0 * ent->character->Height;
                from[2] += fabs(climb->next_z_space);
                if(Physics_RayTestFiltered(&cb, from, to, ent->self))
                {
                    climb->next_z_space = cb.point[2] - climb->edge_point[2];
                }
            }
        }

        from[0] = to[0] = test_from[0];
        from[1] = to[1] = test_from[1];
        from[2] = test_from[2];
        to[2] = climb->edge_point[2] - ent->character->Height;
        climb->can_hang = (Physics_RayTestFiltered(NULL, from, to, ent->self) ? (0x00) : (0x01));
    }
}


void Character_CheckWallsClimbability(struct entity_s *ent, struct climb_info_s *climb)
{
    float from[3], to[3];
    float wn2[2], t, *pos = ent->transform + 12;
    collision_result_t cb;

    climb->can_hang = 0x00;
    climb->wall_hit = 0x00;
    climb->edge_hit = 0x00;
    climb->edge_obj = NULL;
    vec3_copy(climb->point, ent->character->climb.point);

    if(ent->character->height_info.walls_climb == 0x00)
    {
        return;
    }

    climb->up[0] = 0.0;
    climb->up[1] = 0.0;
    climb->up[2] = 1.0;

    from[0] = pos[0] + ent->transform[8 + 0] * ent->bf->bb_max[2] - ent->transform[4 + 0] * ent->character->climb_r;
    from[1] = pos[1] + ent->transform[8 + 1] * ent->bf->bb_max[2] - ent->transform[4 + 1] * ent->character->climb_r;
    from[2] = pos[2] + ent->transform[8 + 2] * ent->bf->bb_max[2] - ent->transform[4 + 2] * ent->character->climb_r;
    vec3_copy(to, from);
    t = ent->character->forvard_size + ent->bf->bb_max[1];
    to[0] += ent->transform[4 + 0] * t;
    to[1] += ent->transform[4 + 1] * t;
    to[2] += ent->transform[4 + 2] * t;

    if(!Physics_SphereTest(&cb, from, to, ent->character->climb_r, ent->self))
    {
        return;
    }

    vec3_copy(climb->point, cb.point);
    vec3_copy(climb->n, cb.normale);
    wn2[0] = climb->n[0];
    wn2[1] = climb->n[1];
    t = sqrt(wn2[0] * wn2[0] + wn2[1] * wn2[1]);
    wn2[0] /= t;
    wn2[0] /= t;

    climb->t[0] =-wn2[1];
    climb->t[1] = wn2[0];
    climb->t[2] = 0.0;
    // now we have wall normale in XOY plane. Let us check all flags

    if((ent->character->height_info.walls_climb_dir & SECTOR_FLAG_CLIMB_NORTH) && (wn2[1] < -0.7))
    {
        climb->wall_hit = 0x01;                                                    // nW = (0, -1, 0);
    }
    if((ent->character->height_info.walls_climb_dir & SECTOR_FLAG_CLIMB_EAST) && (wn2[0] < -0.7))
    {
        climb->wall_hit = 0x01;                                                    // nW = (-1, 0, 0);
    }
    if((ent->character->height_info.walls_climb_dir & SECTOR_FLAG_CLIMB_SOUTH) && (wn2[1] > 0.7))
    {
        climb->wall_hit = 0x01;                                                    // nW = (0, 1, 0);
    }
    if((ent->character->height_info.walls_climb_dir & SECTOR_FLAG_CLIMB_WEST) && (wn2[0] > 0.7))
    {
        climb->wall_hit = 0x01;                                                    // nW = (1, 0, 0);
    }

    if(climb->wall_hit)
    {
        t = 0.67 * ent->character->Height;
        from[0] -= ent->transform[8 + 0] * t;
        from[1] -= ent->transform[8 + 1] * t;
        from[2] -= ent->transform[8 + 2] * t;
        vec3_copy(to, from);
        t = ent->character->forvard_size + ent->bf->bb_max[1];
        to[0] += ent->transform[4 + 0] * t;
        to[1] += ent->transform[4 + 1] * t;
        to[2] += ent->transform[4 + 2] * t;

        if(Physics_SphereTest(NULL, from, to, ent->character->climb_r, ent->self))
        {
            climb->wall_hit = 0x02;
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
        vec3_mul_scalar(ent->speed, ent->transform+4,  t);
    }
    else if(ent->dir_flag & ENT_MOVE_BACKWARD)
    {
        vec3_mul_scalar(ent->speed, ent->transform+4, -t);
    }
    else if(ent->dir_flag & ENT_MOVE_LEFT)
    {
        vec3_mul_scalar(ent->speed, ent->transform+0, -t);
    }
    else if(ent->dir_flag & ENT_MOVE_RIGHT)
    {
        vec3_mul_scalar(ent->speed, ent->transform+0,  t);
    }
    else
    {
        // Jump speed should NOT be added to current speed, as native engine
        // fully replaces current speed with jump speed by anim command.
        //vec3_set_zero(spd);
        ent->dir_flag = ENT_MOVE_FORWARD;
    }

    ent->character->resp.vertical_collide = 0x00;
    ent->character->resp.slide = 0x00;

    // Apply vertical speed.
    ent->speed[2] = v_vertical * ent->character->linear_speed_mult;
    ent->move_type = MOVE_FREE_FALLING;
}


void Character_Lean(struct entity_s *ent, character_command_p cmd, float max_lean)
{
    float neg_lean   = 360.0 - max_lean;
    float lean_coeff = (max_lean == 0.0) ? (48.0) : (max_lean * 3.0f);

    // Continously lean character, according to current left/right direction.

    if((cmd->move[1] == 0) || (max_lean == 0.0))       // No direction - restore straight vertical position!
    {
        if(ent->angles[2] != 0.0)
        {
            if(ent->angles[2] < 180.0)
            {
                ent->angles[2] -= 0.5f * (fabs(ent->angles[2]) + lean_coeff) * engine_frame_time;
                if(ent->angles[2] < 0.0) ent->angles[2] = 0.0;
            }
            else
            {
                ent->angles[2] += 0.5f * (360 - fabs(ent->angles[2]) + lean_coeff) * engine_frame_time;
                if(ent->angles[2] < 180.0) ent->angles[2] = 0.0;
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
                if(ent->angles[2] > max_lean)
                    ent->angles[2] = max_lean;
            }
            else if(ent->angles[2] > 180.0) // Approaching from left
            {
                ent->angles[2] += 0.5f * (360.0 - fabs(ent->angles[2]) + lean_coeff) * engine_frame_time;
                if(ent->angles[2] < 180.0) ent->angles[2] = 0.0;
            }
            else    // Reduce previous lean
            {
                ent->angles[2] -= 0.5f * (fabs(ent->angles[2]) + lean_coeff) * engine_frame_time;
                if(ent->angles[2] < 0.0) ent->angles[2] = 0.0;
            }
        }
    }
    else if(cmd->move[1] == -1)     // Left direction
    {
        if(ent->angles[2] != neg_lean)
        {
            if(ent->angles[2] > neg_lean)   // Reduce previous lean
            {
                ent->angles[2] -= 0.5f * (360.0 - fabs(ent->angles[2]) + lean_coeff) * engine_frame_time;
                if(ent->angles[2] < neg_lean)
                    ent->angles[2] = neg_lean;
            }
            else if(ent->angles[2] < 180.0) // Approaching from right
            {
                ent->angles[2] -= 0.5f * (fabs(ent->angles[2]) + lean_coeff) * engine_frame_time;
                if(ent->angles[2] < 0.0) ent->angles[2] += 360.0;
            }
            else    // Approaching from center
            {
                ent->angles[2] += 0.5f * (360.0 - fabs(ent->angles[2]) + lean_coeff) * engine_frame_time;
                if(ent->angles[2] > 360.0) ent->angles[2] -= 360.0;
            }
        }
    }
}


void Character_LookAt(struct entity_s *ent, float target[3])
{
    const float bone_dir[] = {0.0f, 1.0f, 0.0f};
    ss_animation_p anim_head_track = SSBoneFrame_GetOverrideAnim(ent->bf, ANIM_TYPE_HEAD_TRACK);
    ss_animation_p  base_anim = &ent->bf->animations;

    base_anim->anim_ext_flags &= ~ANIM_EXT_TARGET_TO;

    if(!anim_head_track)
    {
        anim_head_track = SSBoneFrame_AddOverrideAnim(ent->bf, NULL, ANIM_TYPE_HEAD_TRACK);
    }

    anim_head_track->targeting_bone = 14;
    vec3_copy(anim_head_track->target, target);
    vec3_copy(anim_head_track->bone_direction, bone_dir);
    anim_head_track->targeting_flags = 0x0000;
    anim_head_track->targeting_limit[0] = 0.0f;
    anim_head_track->targeting_limit[1] = 1.0f;
    anim_head_track->targeting_limit[2] = 0.0f;
    anim_head_track->targeting_limit[3] = 0.273f;

    if(SSBoneFrame_CheckTargetBoneLimit(ent->bf, anim_head_track))
    {
        anim_head_track->anim_ext_flags |= ANIM_EXT_TARGET_TO;
        if((ent->move_type == MOVE_ON_FLOOR) || (ent->move_type == MOVE_FREE_FALLING))
        {
            base_anim->targeting_bone = 7;
            vec3_copy(base_anim->target, target);
            vec3_copy(base_anim->bone_direction, bone_dir);
            base_anim->targeting_flags = 0x0000;
            base_anim->targeting_limit[0] = 0.0f;
            base_anim->targeting_limit[1] = 1.0f;
            base_anim->targeting_limit[2] = 0.0f;
            base_anim->targeting_limit[3] = 0.883f;

            base_anim->targeting_axis_mod[0] = 0.5f;
            base_anim->targeting_axis_mod[1] = 0.5f;
            base_anim->targeting_axis_mod[2] = 1.0f;
            base_anim->targeting_flags |= ANIM_TARGET_USE_AXIS_MOD;
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
    float norm_move_xy_len, t, ang, *pos = ent->transform + 12;
    float tv[3], move[3], norm_move_xy[2];

    if(!ent->character)
    {
        return 0;
    }

    /*
     * init height info structure
     */
    ent->character->resp.horizontal_collide = 0x00;
    ent->character->resp.vertical_collide = 0x00;
    /*
     * check move type
     */

    if(!ent->character->height_info.floor_hit.hit || (ent->character->height_info.floor_hit.point[2] + ent->character->fall_down_height < pos[2]))
    {
        tv[0] = pos[0];
        tv[1] = pos[1];
        tv[2] = pos[2] - ent->character->max_step_up_height;
        move[0] = pos[0];
        move[1] = pos[1];
        move[2] = pos[2] + 24.0f;
        Physics_SphereTest(&ent->character->height_info.floor_hit, move, tv, 16.0f, ent->self);
        ent->character->height_info.floor_hit.normale[0] = 0.0f;
        ent->character->height_info.floor_hit.normale[1] = 0.0f;
        ent->character->height_info.floor_hit.normale[2] = 1.0f;
    }

    if(ent->character->height_info.floor_hit.hit || (ent->character->resp.vertical_collide & 0x01))
    {
        if(ent->character->height_info.floor_hit.point[2] + ent->character->fall_down_height < pos[2])
        {
            ent->move_type = MOVE_FREE_FALLING;
            ent->speed[2] = 0.0;
            return -1;                                                          // nothing to do here
        }
        else
        {
            ent->character->resp.vertical_collide |= 0x01;
        }

        vec3_copy(tv, ent->character->height_info.floor_hit.normale);
        if(ent->character->height_info.floor_hit.hit && tv[2] > 0.02 && tv[2] < ent->character->critical_slant_z_component)
        {
            tv[2] = -tv[2];
            t = ent->character->linear_speed_mult * DEFAULT_CHARACTER_SLIDE_SPEED_MULT;
            vec3_mul_scalar(ent->speed, tv, t);                                 // slide down direction
            ang = 180.0 * atan2f(tv[0], -tv[1]) / M_PI;                         // from -180 deg to +180 deg
            //ang = (ang < 0.0) ? (ang + 360.0) : (ang);
            t = tv[0] * ent->transform[4 + 0] + tv[1] * ent->transform[4 + 1];
            if(t >= 0.0)
            {
                ent->character->resp.slide = CHARACTER_SLIDE_FRONT;
                ent->angles[0] = ang + 180.0;
                // front forward slide down
            }
            else
            {
                ent->character->resp.slide = CHARACTER_SLIDE_BACK;
                ent->angles[0] = ang;
                // back forward slide down
            }
            Entity_UpdateTransform(ent);
            ent->character->resp.vertical_collide |= 0x01;
        }
        else    // no slide - free to walk
        {
            t = ent->anim_linear_speed * ent->character->linear_speed_mult;
            ent->character->resp.vertical_collide |= 0x01;

            ent->angles[0] += ROT_SPEED_LAND * 60.0f * ent->character->rotate_speed_mult * engine_frame_time * ent->character->cmd.rot[0];

            Entity_UpdateTransform(ent); // apply rotations

            if(ent->dir_flag & ENT_MOVE_FORWARD)
            {
                vec3_mul_scalar(ent->speed, ent->transform+4, t);
            }
            else if(ent->dir_flag & ENT_MOVE_BACKWARD)
            {
                vec3_mul_scalar(ent->speed, ent->transform+4,-t);
            }
            else if(ent->dir_flag & ENT_MOVE_LEFT)
            {
                vec3_mul_scalar(ent->speed, ent->transform+0,-t);
            }
            else if(ent->dir_flag & ENT_MOVE_RIGHT)
            {
                vec3_mul_scalar(ent->speed, ent->transform+0, t);
            }
            else
            {
                vec3_set_zero(ent->speed);
                //ent->dir_flag = ENT_MOVE_FORWARD;
            }
            ent->character->resp.slide = 0x00;
        }
    }
    else                                                                        // no hit to the floor
    {
        ent->character->resp.slide = 0x00;
        ent->character->resp.vertical_collide = 0x00;
        ent->move_type = MOVE_FREE_FALLING;
        ent->speed[2] = 0.0;
        return -1;                                                              // nothing to do here
    }

    /*
     * now move normally
     */
    if(ent->character->height_info.floor_hit.hit && (ent->character->height_info.floor_hit.point[2] + 1.0 >= ent->transform[12+2] + ent->bf->bb_min[2]))
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

    vec3_mul_scalar(move, ent->speed, engine_frame_time);
    t = vec3_abs(move);

    norm_move_xy[0] = move[0];
    norm_move_xy[1] = move[1];
    norm_move_xy_len = sqrt(move[0] * move[0] + move[1] * move[1]);
    if(norm_move_xy_len > 0.2 * t)
    {
        norm_move_xy[0] /= norm_move_xy_len;
        norm_move_xy[1] /= norm_move_xy_len;
    }
    else
    {
        norm_move_xy_len = 32512.0;
        norm_move_xy[0] = 0.0;
        norm_move_xy[1] = 0.0;
    }

    Entity_GhostUpdate(ent);
    vec3_add(pos, pos, move);
    Entity_FixPenetrations(ent, move);
    if(ent->character->height_info.floor_hit.hit)
    {
        if(ent->character->height_info.floor_hit.point[2] + ent->character->fall_down_height > pos[2])
        {
            float dz_to_land = engine_frame_time * 2400.0;                      ///@FIXME: magick
            if(pos[2] > ent->character->height_info.floor_hit.point[2] + dz_to_land)
            {
                pos[2] -= dz_to_land;
                Entity_FixPenetrations(ent, NULL);
            }
            else if(pos[2] > ent->character->height_info.floor_hit.point[2])
            {
                pos[2] = ent->character->height_info.floor_hit.point[2];
                Entity_FixPenetrations(ent, NULL);
            }
        }
        else
        {
            ent->move_type = MOVE_FREE_FALLING;
            ent->speed[2] = 0.0;
            Entity_UpdateRoomPos(ent);
            return 2;
        }
        if((pos[2] < ent->character->height_info.floor_hit.point[2]) && (ent->no_fix_all == 0x00))
        {
            pos[2] = ent->character->height_info.floor_hit.point[2];
            Entity_FixPenetrations(ent, NULL);
            ent->character->resp.vertical_collide |= 0x01;
        }
    }
    else if(!(ent->character->resp.vertical_collide & 0x01))
    {
        ent->move_type = MOVE_FREE_FALLING;
        ent->speed[2] = 0.0;
        Entity_UpdateRoomPos(ent);
        return 2;
    }

    Entity_UpdateRoomPos(ent);

    return 1;
}


int Character_FreeFalling(struct entity_s *ent)
{
    float move[3], g[3], *pos = ent->transform + 12;

    if(!ent->character)
    {
        return 0;
    }

    /*
     * init height info structure
     */

    ent->character->resp.slide = 0x00;
    ent->character->resp.horizontal_collide = 0x00;
    ent->character->resp.vertical_collide = 0x00;

    float rot = ROT_SPEED_FREEFALL * 60.0f * ent->character->rotate_speed_mult * engine_frame_time * ent->character->cmd.rot[0];
    ent->angles[0] += rot;
    ent->angles[1] = 0.0;

    Entity_UpdateTransform(ent);                                                 // apply rotations

    /*float t = ent->current_speed * bf-> ent->character->speed_mult;        ///@TODO: fix speed update in Entity_Frame function and other;
    if(ent->dir_flag & ENT_MOVE_FORWARD)
    {
        ent->speed.m_floats[0] = ent->transform[4 + 0] * t;
        ent->speed.m_floats[1] = ent->transform[4 + 1] * t;
    }
    else if(ent->dir_flag & ENT_MOVE_BACKWARD)
    {
        ent->speed.m_floats[0] =-ent->transform[4 + 0] * t;
        ent->speed.m_floats[1] =-ent->transform[4 + 1] * t;
    }
    else if(ent->dir_flag & ENT_MOVE_LEFT)
    {
        ent->speed.m_floats[0] =-ent->transform[0 + 0] * t;
        ent->speed.m_floats[1] =-ent->transform[0 + 1] * t;
    }
    else if(ent->dir_flag & ENT_MOVE_RIGHT)
    {
        ent->speed.m_floats[0] = ent->transform[0 + 0] * t;
        ent->speed.m_floats[1] = ent->transform[0 + 1] * t;
    }*/


    Physics_GetGravity(g);
    vec3_add_mul(move, ent->speed, g, engine_frame_time * 0.5);
    move[0] *= engine_frame_time;
    move[1] *= engine_frame_time;
    move[2] *= engine_frame_time;
    ent->speed[0] += g[0] * engine_frame_time;
    ent->speed[1] += g[1] * engine_frame_time;
    ent->speed[2] += g[2] * engine_frame_time;
    ent->speed[2] = (ent->speed[2] < -FREE_FALL_SPEED_MAXIMUM) ? (-FREE_FALL_SPEED_MAXIMUM) : (ent->speed[2]);
    vec3_RotateZ(ent->speed, ent->speed, rot);

    if(ent->self->room && (ent->self->room->flags & TR_ROOM_FLAG_WATER))
    {
        if(ent->speed[2] < 0.0)
        {
            ent->anim_linear_speed = 0.0;
            ent->speed[0] = 0.0;
            ent->speed[1] = 0.0;
        }

        if((World_GetVersion() < TR_II))//Lara cannot wade in < TRII so when floor < transition level she has to swim
        {
            if(!ent->character->height_info.water || (ent->current_sector->floor <= ent->character->height_info.transition_level))
            {
                ent->move_type = MOVE_UNDERWATER;
                return 2;
            }
        }
        else
        {
            if(!ent->character->height_info.water || (ent->current_sector->floor + ent->character->Height <= ent->character->height_info.transition_level))
            {
                ent->move_type = MOVE_UNDERWATER;
                return 2;
            }
        }
    }

    Entity_GhostUpdate(ent);
    if(ent->character->height_info.ceiling_hit.hit && ent->speed[2] > 0.0)
    {
        if(ent->character->height_info.ceiling_hit.point[2] < ent->bf->bb_max[2] + pos[2])
        {
            pos[2] = ent->character->height_info.ceiling_hit.point[2] - ent->bf->bb_max[2];
            ent->speed[2] = 0.0;
            ent->character->resp.vertical_collide |= 0x02;
            Entity_FixPenetrations(ent, NULL);
            Entity_UpdateRoomPos(ent);
        }
    }
    if(ent->character->height_info.floor_hit.hit && ent->speed[2] < 0.0)        // move down
    {
        if(ent->character->height_info.floor_hit.point[2] >= pos[2] + ent->bf->bb_min[2] + move[2])
        {
            pos[2] = ent->character->height_info.floor_hit.point[2];
            //ent->speed.m_floats[2] = 0.0;
            ent->move_type = MOVE_ON_FLOOR;
            ent->character->resp.vertical_collide |= 0x01;
            Entity_FixPenetrations(ent, NULL);
            Entity_UpdateRoomPos(ent);
            return 2;
        }
    }

    vec3_add(pos, pos, move);
    Entity_FixPenetrations(ent, move);                                          // get horizontal collide

    if(ent->character->height_info.ceiling_hit.hit && ent->speed[2] > 0.0)
    {
        if(ent->character->height_info.ceiling_hit.point[2] < ent->bf->bb_max[2] + pos[2])
        {
            pos[2] = ent->character->height_info.ceiling_hit.point[2] - ent->bf->bb_max[2];
            ent->speed[2] = 0.0;
            ent->character->resp.vertical_collide |= 0x02;
        }
    }
    if(ent->character->height_info.floor_hit.hit && ent->speed[2] < 0.0)        // move down
    {
        if(ent->character->height_info.floor_hit.point[2] >= pos[2] + ent->bf->bb_min[2] + move[2])
        {
            pos[2] = ent->character->height_info.floor_hit.point[2];
            //ent->speed.m_floats[2] = 0.0;
            ent->move_type = MOVE_ON_FLOOR;
            ent->character->resp.vertical_collide |= 0x01;
            Entity_FixPenetrations(ent, NULL);
            Entity_UpdateRoomPos(ent);
            return 2;
        }
    }
    Entity_UpdateRoomPos(ent);

    return 1;
}

/*
 * Monkey CLIMBING - MOVE NO Z LANDING
 */
int Character_MonkeyClimbing(struct entity_s *ent)
{
    float move[3];
    float t, *pos = ent->transform + 12;

    ent->character->resp.slide = 0x00;
    ent->character->resp.horizontal_collide = 0x00;
    ent->character->resp.vertical_collide = 0x00;

    t = ent->anim_linear_speed * ent->character->linear_speed_mult;
    ent->character->resp.vertical_collide |= 0x01;

    ent->angles[0] += ROT_SPEED_MONKEYSWING * 60.0f * ent->character->rotate_speed_mult * engine_frame_time * ent->character->cmd.rot[0];
    ent->angles[1] = 0.0;
    ent->angles[2] = 0.0;
    Entity_UpdateTransform(ent);                                                // apply rotations

    if(ent->dir_flag & ENT_MOVE_FORWARD)
    {
        vec3_mul_scalar(ent->speed, ent->transform+4, t);
    }
    else if(ent->dir_flag & ENT_MOVE_BACKWARD)
    {
        vec3_mul_scalar(ent->speed, ent->transform+4,-t);
    }
    else if(ent->dir_flag & ENT_MOVE_LEFT)
    {
        vec3_mul_scalar(ent->speed, ent->transform+0,-t);
    }
    else if(ent->dir_flag & ENT_MOVE_RIGHT)
    {
        vec3_mul_scalar(ent->speed, ent->transform+0, t);
    }
    else
    {
        vec3_set_zero(ent->speed);
        //ent->dir_flag = ENT_MOVE_FORWARD;
    }
    ent->speed[2] = 0.0;
    ent->character->resp.slide = 0x00;
    vec3_mul_scalar(move, ent->speed, engine_frame_time);

    Entity_GhostUpdate(ent);
    vec3_add(pos, pos, move);
    Entity_FixPenetrations(ent, move);                                          // get horizontal collide
    ///@FIXME: rewrite conditions! or add fixer to update_entity_rigid_body func
    if(ent->character->height_info.ceiling_hit.hit && (pos[2] + ent->bf->bb_max[2] - ent->character->height_info.ceiling_hit.point[2] > - 0.33 * ent->character->min_step_up_height))
    {
        pos[2] = ent->character->height_info.ceiling_hit.point[2] - ent->bf->bb_max[2];
    }
    else
    {
        ent->move_type = MOVE_FREE_FALLING;
        Entity_UpdateRoomPos(ent);
        return 2;
    }

    Entity_UpdateRoomPos(ent);

    return 1;
}

/*
 * WALLS CLIMBING - MOVE IN ZT plane
 */
int Character_WallsClimbing(struct entity_s *ent)
{
    climb_info_t *climb = &ent->character->climb;
    float move[3], t, *pos = ent->transform + 12;

    ent->character->resp.slide = 0x00;
    ent->character->resp.horizontal_collide = 0x00;
    ent->character->resp.vertical_collide = 0x00;

    Character_CheckWallsClimbability(ent, climb);
    ent->character->climb = *climb;
    if(!(climb->wall_hit))
    {
        ent->character->height_info.walls_climb = 0x00;
        return 2;
    }

    ent->angles[0] = 180.0 * atan2f(climb->n[0], -climb->n[1]) / M_PI;
    Entity_UpdateTransform(ent);
    pos[0] = climb->point[0] - ent->transform[4 + 0] * ent->bf->bb_max[1];
    pos[1] = climb->point[1] - ent->transform[4 + 1] * ent->bf->bb_max[1];

    if(ent->dir_flag == ENT_MOVE_FORWARD)
    {
        vec3_copy(move, climb->up);
    }
    else if(ent->dir_flag == ENT_MOVE_BACKWARD)
    {
        vec3_copy_inv(move, climb->up);
    }
    else if(ent->dir_flag == ENT_MOVE_RIGHT)
    {
        vec3_copy(move, climb->t);
    }
    else if(ent->dir_flag == ENT_MOVE_LEFT)
    {
        vec3_copy_inv(move, climb->t);
    }
    else
    {
        vec3_set_zero(move);
    }
    t = vec3_abs(move);
    if(t > 0.01)
    {
        move[0] /= t;
        move[1] /= t;
        move[2] /= t;
    }

    t = ent->anim_linear_speed * ent->character->linear_speed_mult;
    vec3_mul_scalar(ent->speed, move, t);
    vec3_mul_scalar(move, ent->speed, engine_frame_time);

    Entity_GhostUpdate(ent);
    vec3_add(pos, pos, move);
    Entity_FixPenetrations(ent, move);                                          // get horizontal collide
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
    float z = pos[2];

    ent->character->resp.slide = 0x00;
    ent->character->resp.horizontal_collide = 0x00;
    ent->character->resp.vertical_collide = 0x00;

    t = ent->anim_linear_speed * ent->character->linear_speed_mult;
    ent->character->resp.vertical_collide |= 0x01;
    ent->angles[0] += ROT_SPEED_MONKEYSWING * 60.0f * ent->character->rotate_speed_mult * engine_frame_time * ent->character->cmd.rot[0];
    ent->angles[1] = 0.0;
    ent->angles[2] = 0.0;
    Entity_UpdateTransform(ent);                                                // apply rotations

    if(ent->dir_flag == ENT_MOVE_FORWARD)
    {
        vec3_mul_scalar(ent->speed, ent->transform+4, t);
    }
    else if(ent->dir_flag == ENT_MOVE_BACKWARD)
    {
        vec3_mul_scalar(ent->speed, ent->transform+4,-t);
    }
    else if(ent->dir_flag == ENT_MOVE_LEFT)
    {
        vec3_mul_scalar(ent->speed, ent->transform+0,-t);
    }
    else if(ent->dir_flag == ENT_MOVE_RIGHT)
    {
        vec3_mul_scalar(ent->speed, ent->transform+0, t);
    }
    else
    {
        vec3_set_zero(ent->speed);
        ent->character->resp.slide = 0x00;
        Entity_GhostUpdate(ent);
        Entity_FixPenetrations(ent, NULL);
        return 1;
    }

    ent->character->resp.slide = 0x00;
    vec3_mul_scalar(move, ent->speed, engine_frame_time);

    Entity_GhostUpdate(ent);
    vec3_add(pos, pos, move);
    Entity_FixPenetrations(ent, move);                                          // get horizontal collide
    Entity_UpdateRoomPos(ent);
    pos[2] = z;

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

    if(ent->self->room && !(ent->self->room->flags & TR_ROOM_FLAG_WATER))
    {
        ent->move_type = MOVE_FREE_FALLING;
        return 2;
    }

    ent->character->resp.slide = 0x00;
    ent->character->resp.horizontal_collide = 0x00;
    ent->character->resp.vertical_collide = 0x00;

    if(!ent->character->resp.kill)   // Block controls if Lara is dead.
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
        ent->angles[2]  = 0.0;

        if((ent->angles[1] > 70.0) && (ent->angles[1] < 180.0))                 // Underwater angle limiter.
        {
           ent->angles[1] = 70.0;
        }
        else if((ent->angles[1] > 180.0) && (ent->angles[1] < 270.0))
        {
            ent->angles[1] = 270.0;
        }

        Entity_UpdateTransform(ent);                                            // apply rotations
        vec3_mul_scalar(ent->speed, ent->transform + 4, ent->linear_speed * ent->character->linear_speed_mult);    // OY move only!
    }
    vec3_mul_scalar(move, ent->speed, engine_frame_time);

    Entity_GhostUpdate(ent);
    vec3_add(pos, pos, move);
    Entity_FixPenetrations(ent, move);                                          // get horizontal collide

    Entity_UpdateRoomPos(ent);
    if(ent->character->height_info.water && (pos[2] + ent->bf->bb_max[2] >= ent->character->height_info.transition_level))
    {
        if(ent->transform[4 + 2] > 0.67)             ///@FIXME: magick!
        {
            ent->move_type = MOVE_ON_WATER;
            //pos[2] = fc.transition_level;
            return 2;
        }
        if(!ent->character->height_info.floor_hit.hit || (ent->character->height_info.transition_level - ent->character->height_info.floor_hit.point[2] >= ent->character->Height))
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

    ent->character->resp.slide = 0x00;
    ent->character->resp.horizontal_collide = 0x00;
    ent->character->resp.vertical_collide = 0x00;

    ent->angles[0] += ROT_SPEED_ONWATER * 60.0f * ent->character->rotate_speed_mult * engine_frame_time * ent->character->cmd.rot[0];
    ent->angles[1] = 0.0;
    ent->angles[2] = 0.0;
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
        vec3_mul_scalar(ent->speed, ent->transform+4, t);
    }
    else if((ent->dir_flag & ENT_MOVE_BACKWARD) && (ent->character->cmd.move[0] == -1))
    {
        vec3_mul_scalar(ent->speed, ent->transform+4,-t);
    }
    else if((ent->dir_flag & ENT_MOVE_LEFT) && (ent->character->cmd.move[1] == -1))
    {
        vec3_mul_scalar(ent->speed, ent->transform+0,-t);
    }
    else if((ent->dir_flag & ENT_MOVE_RIGHT) && (ent->character->cmd.move[1] == 1))
    {
        vec3_mul_scalar(ent->speed, ent->transform+0, t);
    }
    else
    {
        vec3_set_zero(ent->speed);
        Entity_GhostUpdate(ent);
        Entity_FixPenetrations(ent, NULL);
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
    Entity_GhostUpdate(ent);
    vec3_add(pos, pos, move);
    Entity_FixPenetrations(ent, move);  // get horizontal collide

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

int Character_FindTraverse(struct entity_s *ch)
{
    room_sector_p ch_s, obj_s = NULL;
    ch_s = Room_GetSectorRaw(ch->self->room, ch->transform + 12);

    if(ch_s == NULL)
    {
        return 0;
    }

    ch->character->traversed_object = NULL;

    // OX move case
    if(ch->transform[4 + 0] > 0.9)
    {
        float pos[] = {(float)(ch_s->pos[0] + TR_METERING_SECTORSIZE), (float)(ch_s->pos[1]), (float)0.0};
        obj_s = Room_GetSectorRaw(ch_s->owner_room, pos);
    }
    else if(ch->transform[4 + 0] < -0.9)
    {
        float pos[] = {(float)(ch_s->pos[0] - TR_METERING_SECTORSIZE), (float)(ch_s->pos[1]), (float)0.0};
        obj_s = Room_GetSectorRaw(ch_s->owner_room, pos);
    }
    // OY move case
    else if(ch->transform[4 + 1] > 0.9)
    {
        float pos[] = {(float)(ch_s->pos[0]), (float)(ch_s->pos[1] + TR_METERING_SECTORSIZE), (float)0.0};
        obj_s = Room_GetSectorRaw(ch_s->owner_room, pos);
    }
    else if(ch->transform[4 + 1] < -0.9)
    {
        float pos[] = {(float)(ch_s->pos[0]), (float)(ch_s->pos[1] - TR_METERING_SECTORSIZE), (float)0.0};
        obj_s = Room_GetSectorRaw(ch_s->owner_room, pos);
    }

    if(obj_s != NULL)
    {
        obj_s = Sector_GetPortalSectorTarget(obj_s);
        for(engine_container_p cont = obj_s->owner_room->content->containers; cont; cont = cont->next)
        {
            if(cont->object_type == OBJECT_ENTITY)
            {
                entity_p e = (entity_p)cont->object;
                if((e->type_flags & ENTITY_TYPE_TRAVERSE) && OBB_OBB_Test(e->obb, ch->obb) && (fabs(e->transform[12 + 2] - ch->transform[12 + 2]) < 1.1f))
                {
                    int oz = (ch->angles[0] + 45.0) / 90.0;
                    ch->angles[0] = oz * 90.0;
                    ch->character->traversed_object = e;
                    Entity_UpdateTransform(ch);
                    return 1;
                }
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
int Sector_AllowTraverse(struct room_sector_s *rs, float floor, struct engine_container_s *cont)
{
    float f0 = rs->floor_corners[0][2];
    if((rs->floor_corners[0][2] != f0) || (rs->floor_corners[1][2] != f0) ||
       (rs->floor_corners[2][2] != f0) || (rs->floor_corners[3][2] != f0))
    {
        return 0x00;
    }

    if((fabs(floor - f0) < 1.1) && (rs->ceiling - rs->floor >= TR_METERING_SECTORSIZE))
    {
        return 0x01;
    }

    float from[3], to[3];
    collision_result_t cb;

    to[0] = from[0] = rs->pos[0];
    to[1] = from[1] = rs->pos[1];
    from[2] = floor + TR_METERING_SECTORSIZE * 0.5;
    to[2]   = floor - TR_METERING_SECTORSIZE * 0.5;

    if(Physics_RayTest(&cb, from, to, cont))
    {
        if(fabs(cb.point[2] - floor) < 1.1)
        {
            if((cb.obj != NULL) && (cb.obj->object_type == OBJECT_ENTITY) && (((entity_p)cb.obj->object)->type_flags & ENTITY_TYPE_TRAVERSE_FLOOR))
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
    room_sector_p ch_s  = Room_GetSectorRaw(ch->self->room, ch->transform + 12);
    room_sector_p obj_s = Room_GetSectorRaw(obj->self->room, obj->transform + 12);

    if(obj_s == ch_s)
    {
        if(ch->transform[4 + 0] > 0.8)
        {
            float pos[] = {(float)(obj_s->pos[0] - TR_METERING_SECTORSIZE), (float)(obj_s->pos[1]), (float)0.0};
            ch_s = Room_GetSectorRaw(obj_s->owner_room, pos);
        }
        else if(ch->transform[4 + 0] < -0.8)
        {
            float pos[] = {(float)(obj_s->pos[0] + TR_METERING_SECTORSIZE), (float)(obj_s->pos[1]), (float)0.0};
            ch_s = Room_GetSectorRaw(obj_s->owner_room, pos);
        }
        // OY move case
        else if(ch->transform[4 + 1] > 0.8)
        {
            float pos[] = {(float)(obj_s->pos[0]), (float)(obj_s->pos[1] - TR_METERING_SECTORSIZE), (float)0.0};
            ch_s = Room_GetSectorRaw(obj_s->owner_room, pos);
        }
        else if(ch->transform[4 + 1] < -0.8)
        {
            float pos[] = {(float)(obj_s->pos[0]), (float)(obj_s->pos[1] + TR_METERING_SECTORSIZE), (float)0.0};
            ch_s = Room_GetSectorRaw(obj_s->owner_room, pos);
        }
        ch_s = Sector_GetPortalSectorTarget(ch_s);
    }

    if((ch_s == NULL) || (obj_s == NULL))
    {
        return 0x00;
    }

    float floor = ch->transform[12 + 2];
    if((ch_s->floor != obj_s->floor) || (Sector_AllowTraverse(ch_s, floor, ch->self) == 0x00) || (Sector_AllowTraverse(obj_s, floor, obj->self) == 0x00))
    {
        return 0x00;
    }

    collision_result_t cb;
    float from[3], to[3];

    to[0] = from[0] = obj_s->pos[0];
    to[1] = from[1] = obj_s->pos[1];
    from[2] = floor + TR_METERING_SECTORSIZE * 0.5;
    to[2] = floor + TR_METERING_SECTORSIZE * 2.5;
    if(Physics_RayTest(&cb, from, to, obj->self))
    {
        if((cb.obj != NULL) && (cb.obj->object_type == OBJECT_ENTITY) && (((entity_p)cb.obj->object)->type_flags & ENTITY_TYPE_TRAVERSE))
        {
            return 0x00;
        }
    }

    int ret = 0x00;
    room_sector_p next_s = NULL;

    /*
     * PUSH MOVE CHECK
     */
    // OX move case
    if(ch->transform[4 + 0] > 0.8)
    {
        float pos[] = {(float)(obj_s->pos[0] + TR_METERING_SECTORSIZE), (float)(obj_s->pos[1]), (float)0.0};
        next_s = Room_GetSectorRaw(obj_s->owner_room, pos);
    }
    else if(ch->transform[4 + 0] < -0.8)
    {
        float pos[] = {(float)(obj_s->pos[0] - TR_METERING_SECTORSIZE), (float)(obj_s->pos[1]), (float)0.0};
        next_s = Room_GetSectorRaw(obj_s->owner_room, pos);
    }
    // OY move case
    else if(ch->transform[4 + 1] > 0.8)
    {
        float pos[] = {(float)(obj_s->pos[0]), (float)(obj_s->pos[1] + TR_METERING_SECTORSIZE), (float)0.0};
        next_s = Room_GetSectorRaw(obj_s->owner_room, pos);
    }
    else if(ch->transform[4 + 1] < -0.8)
    {
        float pos[] = {(float)(obj_s->pos[0]), (float)(obj_s->pos[1] - TR_METERING_SECTORSIZE), (float)0.0};
        next_s = Room_GetSectorRaw(obj_s->owner_room, pos);
    }

    next_s = Sector_GetPortalSectorTarget(next_s);
    if((next_s != NULL) && (Sector_AllowTraverse(next_s, floor, ch->self) == 0x01))
    {
        from[0] = obj_s->pos[0];
        from[1] = obj_s->pos[1];
        from[2] = floor + 0.5 * TR_METERING_SECTORSIZE;

        to[0] = next_s->pos[0];
        to[1] = next_s->pos[1];
        to[2] = from[2];
        if(!Physics_SphereTest(NULL, from ,to, 0.48 * TR_METERING_SECTORSIZE, obj->self))
        {
            ret |= 0x01;                                                        // can traverse forvard
        }
    }

    /*
     * PULL MOVE CHECK
     */
    next_s = NULL;
    // OX move case
    if(ch->transform[4 + 0] > 0.8)
    {
        float pos[] = {(float)(ch_s->pos[0] - TR_METERING_SECTORSIZE), (float)(ch_s->pos[1]), (float)0.0};
        next_s = Room_GetSectorRaw(ch_s->owner_room, pos);
    }
    else if(ch->transform[4 + 0] < -0.8)
    {
        float pos[] = {(float)(ch_s->pos[0] + TR_METERING_SECTORSIZE), (float)(ch_s->pos[1]), (float)0.0};
        next_s = Room_GetSectorRaw(ch_s->owner_room, pos);
    }
    // OY move case
    else if(ch->transform[4 + 1] > 0.8)
    {
        float pos[] = {(float)(ch_s->pos[0]), (float)(ch_s->pos[1] - TR_METERING_SECTORSIZE), (float)0.0};
        next_s = Room_GetSectorRaw(ch_s->owner_room, pos);
    }
    else if(ch->transform[4 + 1] < -0.8)
    {
        float pos[] = {(float)(ch_s->pos[0]), (float)(ch_s->pos[1] + TR_METERING_SECTORSIZE), (float)0.0};
        next_s = Room_GetSectorRaw(ch_s->owner_room, pos);
    }

    next_s = Sector_GetPortalSectorTarget(next_s);
    if((next_s != NULL) && (Sector_AllowTraverse(next_s, floor, ch->self) == 0x01))
    {
        from[0] = ch_s->pos[0];
        from[1] = ch_s->pos[1];
        from[2] = floor + 0.5 * TR_METERING_SECTORSIZE;

        to[0] = next_s->pos[0];
        to[1] = next_s->pos[1];
        to[2] = from[2];
        if(!Physics_SphereTest(NULL, from ,to, 0.48 * TR_METERING_SECTORSIZE, ch->self))
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
    Character_UpdatePlatformPreStep(ent);

    if((ent->character->cmd.ready_weapon != 0x00) && (ent->character->current_weapon > 0) && (ent->character->weapon_current_state == WEAPON_STATE_HIDE))
    {
        Character_SetWeaponModel(ent, ent->character->current_weapon, 1);
    }

    if(ent->character->state_func)
    {
        ent->character->state_func(ent, &ent->bf->animations);
    }

    switch(ent->move_type)
    {
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

        default:
            ent->move_type = MOVE_ON_FLOOR;
            break;
    };

    Entity_UpdateRigidBody(ent, 1);
    Character_UpdatePlatformPostStep(ent);
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
                if(!Character_ChangeParam(ent, PARAM_AIR, -3.0 * speed))
                    Character_ChangeParam(ent, PARAM_HEALTH, -3.0 * speed);
            }
            else if(ent->character->height_info.quicksand == 0x01)
            {
                Character_ChangeParam(ent, PARAM_AIR, 3.0 * speed);
            }
            else
            {
                Character_SetParam(ent, PARAM_AIR, PARAM_ABSOLUTE_MAX);
            }

            if((ent->bf->animations.last_state == TR_STATE_LARA_SPRINT) ||
               (ent->bf->animations.last_state == TR_STATE_LARA_SPRINT_ROLL))
            {
                Character_ChangeParam(ent, PARAM_STAMINA, -0.5 * speed);
            }
            else
            {
                Character_ChangeParam(ent, PARAM_STAMINA,  0.5 * speed);
            }
            break;

        case MOVE_ON_WATER:
            Character_ChangeParam(ent, PARAM_AIR, 3.0 * speed);
            break;

        case MOVE_UNDERWATER:
            if(!Character_ChangeParam(ent, PARAM_AIR, -1.0 * speed))
            {
                if(!Character_ChangeParam(ent, PARAM_HEALTH, -3.0 * speed))
                {
                    ent->character->resp.kill = 1;
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
    if(!ent || !ent->character || (parameter >= PARAM_LASTINDEX))
    {
        return 0;
    }

    return ent->character->parameters.param[parameter];
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
int Character_SetWeaponModel(struct entity_s *ent, int weapon_model, int armed)
{
    skeletal_model_p sm = World_GetModelByID(weapon_model);

    if((sm != NULL) && (ent->bf->bone_tag_count == sm->mesh_count) && (sm->animation_count >= 4))
    {
        skeletal_model_p bm = ent->bf->animations.model;
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
            anim_rh->last_state = WEAPON_STATE_HIDE;
            anim_rh->next_state = WEAPON_STATE_HIDE;

            ss_animation_p anim_lh = SSBoneFrame_GetOverrideAnim(ent->bf, ANIM_TYPE_WEAPON_LH);
            if(!anim_lh)
            {
                anim_lh = SSBoneFrame_AddOverrideAnim(ent->bf, World_GetModelByID(weapon_model), ANIM_TYPE_WEAPON_LH);
            }
            anim_lh->model = sm;
            anim_lh->onEndFrame = NULL;
            anim_lh->onFrame = Character_DoOneHandWeponFrame;
            anim_lh->last_state = WEAPON_STATE_HIDE;
            anim_lh->next_state = WEAPON_STATE_HIDE;

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
            anim_th->last_state = WEAPON_STATE_HIDE;
            anim_th->next_state = WEAPON_STATE_HIDE;
            SSBoneFrame_EnableOverrideAnim(ent->bf, anim_th);
        }

        for(uint16_t i = 0; i < bm->mesh_count; i++)
        {
            ent->bf->bone_tags[i].mesh_base = bm->mesh_tree[i].mesh_base;
            ent->bf->bone_tags[i].mesh_slot = NULL;
        }

        if(armed != 0)
        {
            for(uint16_t i = 0; i < bm->mesh_count; i++)
            {
                if(sm->mesh_tree[i].replace_mesh == 0x01)
                {
                    ent->bf->bone_tags[i].mesh_base = sm->mesh_tree[i].mesh_base;
                }
                else if(sm->mesh_tree[i].replace_mesh == 0x02)
                {
                    ent->bf->bone_tags[i].mesh_slot = sm->mesh_tree[i].mesh_base;
                }
            }
        }
        else
        {
            for(uint16_t i = 0; i < bm->mesh_count; i++)
            {
                ss_animation_p alt_anim = ent->bf->bone_tags[i].alt_anim;
                if(sm->mesh_tree[i].replace_mesh == 0x03)
                {
                    ent->bf->bone_tags[i].mesh_base = sm->mesh_tree[i].mesh_base;
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
        }

        return 1;
    }
    else
    {
        // do unarmed default model
        skeletal_model_p bm = ent->bf->animations.model;
        for(uint16_t i = 0; i < bm->mesh_count; i++)
        {
            ss_animation_p alt_anim = ent->bf->bone_tags[i].alt_anim;
            ent->bf->bone_tags[i].mesh_base = bm->mesh_tree[i].mesh_base;
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
    if(ss_anim->model->animation_count == 4)
    {
        const float bone_dir[] = {0.0f, 1.0f, 0.0f};
        float dt;
        int32_t t;
        uint16_t targeted_bone = (ss_anim->type == ANIM_TYPE_WEAPON_LH) ? (11) : (8);
        entity_p target = (ent->character->target_id != ENTITY_ID_NONE) ? World_GetEntityByID(ent->character->target_id) : (NULL);
        bool silent = false;
        if(target)
        {
            ss_anim->targeting_bone = targeted_bone;
            vec3_copy(ss_anim->target, target->obb->centre);
            vec3_copy(ss_anim->bone_direction, bone_dir);
            ss_anim->targeting_flags = 0x0000;
            ss_anim->targeting_limit[0] = 0.0f;
            ss_anim->targeting_limit[1] = 1.0f;
            ss_anim->targeting_limit[2] = 0.0f;
            ss_anim->targeting_limit[3] = 0.224f;

            if(ss_anim->type == ANIM_TYPE_WEAPON_LH)
            {
                vec3_RotateZ(ss_anim->targeting_limit, ss_anim->targeting_limit, 40.0f);
            }
            else
            {
                vec3_RotateZ(ss_anim->targeting_limit, ss_anim->targeting_limit, -40.0f);
            }

            if(!SSBoneFrame_CheckTargetBoneLimit(ent->bf, ss_anim))
            {
                target = NULL;
                silent = true;
            }
        }

        ss_anim->anim_ext_flags &= ~ANIM_EXT_TARGET_TO;
        switch(ss_anim->last_state)
        {
            case WEAPON_STATE_HIDE:
                if(ent->character->cmd.ready_weapon)   // ready weapon
                {
                    ss_anim->current_animation = 2;
                    ss_anim->next_animation = 2;
                    ss_anim->current_frame = 0;
                    ss_anim->next_frame = 0;
                    ss_anim->frame_time = 0.0;
                    ss_anim->last_state = WEAPON_STATE_HIDE_TO_READY;
                    ent->character->weapon_current_state = WEAPON_STATE_IDLE;
                }
                break;

            case WEAPON_STATE_HIDE_TO_READY:
                ss_anim->frame_time += time;
                ss_anim->current_frame = (ss_anim->frame_time) / ss_anim->period;
                dt = ss_anim->frame_time - (float)ss_anim->current_frame * ss_anim->period;
                ss_anim->lerp = dt / ss_anim->period;
                t = ss_anim->model->animations[ss_anim->current_animation].frames_count;

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
                    ss_anim->last_state = WEAPON_STATE_IDLE;
                }
                break;

            case WEAPON_STATE_IDLE:
                ss_anim->current_frame = 0;
                ss_anim->current_animation = 0;
                ss_anim->next_frame = 0;
                ss_anim->next_animation = 0;
                ss_anim->frame_time = 0.0;
                if(ent->character->cmd.ready_weapon)
                {
                    ss_anim->current_animation = 2;
                    ss_anim->next_animation = 2;
                    ss_anim->current_frame = ss_anim->next_frame = ss_anim->model->animations[ss_anim->current_animation].frames_count - 1;
                    ss_anim->frame_time = 0.0;
                    ss_anim->last_state = WEAPON_STATE_IDLE_TO_HIDE;
                }
                else if((!silent && ent->character->cmd.action) || target)
                {
                    ss_anim->last_state = WEAPON_STATE_IDLE_TO_FIRE;
                }
                else
                {
                    // do nothing here, may be;
                }
                break;

            case WEAPON_STATE_FIRE_TO_IDLE:
                // Yes, same animation, reverse frames order;
                t = ss_anim->model->animations[ss_anim->current_animation].frames_count;
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
                    ss_anim->last_state = WEAPON_STATE_IDLE;
                }
                break;

            case WEAPON_STATE_IDLE_TO_FIRE:
                ss_anim->frame_time += time;
                ss_anim->current_frame = (ss_anim->frame_time) / ss_anim->period;
                dt = ss_anim->frame_time - (float)ss_anim->current_frame * ss_anim->period;
                ss_anim->lerp = dt / ss_anim->period;
                t = ss_anim->model->animations[ss_anim->current_animation].frames_count;

                if(ent->character->cmd.ready_weapon)
                {
                    ss_anim->current_animation = 2;
                    ss_anim->next_animation = 2;
                    ss_anim->frame_time = 0.0;
                    ss_anim->last_state = WEAPON_STATE_IDLE_TO_HIDE;
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
                    ss_anim->last_state = WEAPON_STATE_FIRE;
                }
                else
                {
                    ss_anim->frame_time = 0.0;
                    ss_anim->current_frame = ss_anim->model->animations[ss_anim->current_animation].frames_count - 1;
                    ss_anim->last_state = WEAPON_STATE_FIRE_TO_IDLE;
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
                    t = ss_anim->model->animations[ss_anim->current_animation].frames_count;

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
                    ss_anim->current_frame = ss_anim->model->animations[ss_anim->current_animation].frames_count - 1;
                    ss_anim->next_frame = (ss_anim->current_frame > 0) ? (ss_anim->current_frame - 1) : (0);
                    ss_anim->last_state = WEAPON_STATE_FIRE_TO_IDLE;
                }
                break;

            case WEAPON_STATE_IDLE_TO_HIDE:
                // Yes, same animation, reverse frames order;
                t = ss_anim->model->animations[ss_anim->current_animation].frames_count;
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
                    ss_anim->last_state = WEAPON_STATE_HIDE;
                    ent->character->weapon_current_state = WEAPON_STATE_HIDE;
                    Character_SetWeaponModel(ent, ent->character->current_weapon, 0);
                }
                break;
        };
    }
    return 1;
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
    if(ss_anim->model->animation_count > 4)
    {
        float dt;
        int32_t t;
        uint16_t targeted_bone = 7;
        entity_p target = (ent->character->target_id != ENTITY_ID_NONE) ? World_GetEntityByID(ent->character->target_id) : (NULL);
        if(target)
        {
            const float bone_dir[] = {0.0f, 1.0f, 0.0f};
            ss_anim->targeting_bone = targeted_bone;
            vec3_copy(ss_anim->target, target->obb->centre);
            vec3_copy(ss_anim->bone_direction, bone_dir);
            ss_anim->targeting_flags = 0x0000;
            ss_anim->targeting_limit[0] = 0.0f;
            ss_anim->targeting_limit[1] = 1.0f;
            ss_anim->targeting_limit[2] = 0.0f;
            ss_anim->targeting_limit[3] = 0.624f;

            if(!SSBoneFrame_CheckTargetBoneLimit(ent->bf, ss_anim))
            {
                target = NULL;
            }
        }
        ss_anim->anim_ext_flags &= ~ANIM_EXT_TARGET_TO;
        Character_ClearLookAt(ent);

        switch(ss_anim->last_state)
        {
            case WEAPON_STATE_HIDE:
                if(ent->character->cmd.ready_weapon)   // ready weapon
                {
                    ss_anim->current_animation = 1;
                    ss_anim->next_animation = 1;
                    ss_anim->current_frame = 0;
                    ss_anim->next_frame = 0;
                    ss_anim->frame_time = 0.0;
                    ss_anim->last_state = WEAPON_STATE_HIDE_TO_READY;
                    ent->character->weapon_current_state = WEAPON_STATE_IDLE;
                }
                break;

            case WEAPON_STATE_HIDE_TO_READY:
                ss_anim->frame_time += time;
                ss_anim->current_frame = (ss_anim->frame_time) / ss_anim->period;
                dt = ss_anim->frame_time - (float)ss_anim->current_frame * ss_anim->period;
                ss_anim->lerp = dt / ss_anim->period;
                t = ss_anim->model->animations[ss_anim->current_animation].frames_count;

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
                    ss_anim->last_state = WEAPON_STATE_IDLE;
                }
                break;

            case WEAPON_STATE_IDLE:
                ss_anim->current_frame = 0;
                ss_anim->current_animation = 0;
                ss_anim->next_frame = 0;
                ss_anim->next_animation = 0;
                ss_anim->frame_time = 0.0;
                if(ent->character->cmd.ready_weapon)
                {
                    ss_anim->current_animation = 3;
                    ss_anim->next_animation = 3;
                    ss_anim->current_frame = ss_anim->next_frame = 0;
                    ss_anim->frame_time = 0.0;
                    ss_anim->last_state = WEAPON_STATE_IDLE_TO_HIDE;
                }
                else if(ent->character->cmd.action || target)
                {
                    ss_anim->last_state = WEAPON_STATE_IDLE_TO_FIRE;
                }
                else
                {
                    // do nothing here, may be;
                }
                break;

            case WEAPON_STATE_FIRE_TO_IDLE:
                // Yes, same animation, reverse frames order;
                t = ss_anim->model->animations[ss_anim->current_animation].frames_count;
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
                    ss_anim->last_state = WEAPON_STATE_IDLE;
                }
                break;

            case WEAPON_STATE_IDLE_TO_FIRE:
                ss_anim->frame_time += time;
                ss_anim->current_frame = (ss_anim->frame_time) / ss_anim->period;
                dt = ss_anim->frame_time - (float)ss_anim->current_frame * ss_anim->period;
                ss_anim->lerp = dt / ss_anim->period;
                t = ss_anim->model->animations[ss_anim->current_animation].frames_count;

                if(ent->character->cmd.ready_weapon)
                {
                    ss_anim->current_animation = 3;
                    ss_anim->next_animation = 3;
                    ss_anim->frame_time = 0.0;
                    ss_anim->last_state = WEAPON_STATE_IDLE_TO_HIDE;
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
                    ss_anim->last_state = WEAPON_STATE_FIRE;
                }
                else
                {
                    ss_anim->frame_time = 0.0;
                    ss_anim->current_frame = ss_anim->model->animations[ss_anim->current_animation].frames_count - 1;
                    ss_anim->last_state = WEAPON_STATE_FIRE_TO_IDLE;
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
                    t = ss_anim->model->animations[ss_anim->current_animation].frames_count;

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
                    ss_anim->current_frame = ss_anim->model->animations[ss_anim->current_animation].frames_count - 1;
                    ss_anim->next_frame = (ss_anim->current_frame > 0) ? (ss_anim->current_frame - 1) : (0);
                    ss_anim->last_state = WEAPON_STATE_FIRE_TO_IDLE;
                }
                break;

            case WEAPON_STATE_IDLE_TO_HIDE:
                t = ss_anim->model->animations[ss_anim->current_animation].frames_count;
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
                    ss_anim->last_state = WEAPON_STATE_HIDE;
                    ent->character->weapon_current_state = WEAPON_STATE_HIDE;
                    Character_SetWeaponModel(ent, ent->character->current_weapon, 0);
                }
                ss_anim->anim_ext_flags &= ~ANIM_EXT_TARGET_TO;
                break;
        };
    }
    return 1;
}
