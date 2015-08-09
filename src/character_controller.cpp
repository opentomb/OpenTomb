
#include "core/vmath.h"
#include "core/console.h"
#include "core/polygon.h"
#include "core/obb.h"

#include "audio.h"
#include "world.h"
#include "character_controller.h"
#include "anim_state_control.h"
#include "engine.h"
#include "engine_physics.h"
#include "entity.h"
#include "gui.h"
#include "mesh.h"
#include "hair.h"
#include "resource.h"
#include "engine_string.h"

void Character_Create(struct entity_s *ent)
{
    character_p ret;

    if(ent == NULL || ent->character != NULL)
    {
        return;
    }

    ret = (character_p)malloc(sizeof(character_t));
    //ret->platform = NULL;
    ret->state_func = NULL;
    ret->inventory = NULL;
    ret->ent = ent;
    ent->character = ret;
    ret->height_info.self = ent->self;
    ent->dir_flag = ENT_STAY;

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
    ret->min_step_up_height = DEFAULT_MIN_STEP_UP_HEIGHT;
    ret->max_climb_height = DEFAULT_CLIMB_UP_HEIGHT;
    ret->max_step_up_height = DEFAULT_MAX_STEP_UP_HEIGHT;
    ret->fall_down_height = DEFAULT_FALL_DOWN_HEIGHT;
    ret->critical_slant_z_component = DEFAULT_CRITICAL_SLANT_Z_COMPONENT;
    ret->critical_wall_component = DEFAULT_CRITICAL_WALL_COMPONENT;
    ret->climb_r = DEFAULT_CHARACTER_CLIMB_R;
    ret->wade_depth = DEFAULT_CHARACTER_WADE_DEPTH;
    ret->swim_depth = DEFAULT_CHARACTER_SWIM_DEPTH;

    for(int i=0;i<PARAM_LASTINDEX;i++)
    {
        ret->parameters.param[i] = 0.0;
        ret->parameters.maximum[i] = 0.0;
    }

    ret->sphere = CHARACTER_BASE_RADIUS;
    ret->climb_sensor = ent->character->climb_r;
    ret->height_info.ceiling_hit = 0x00;
    ret->height_info.floor_hit = 0x00;
    ret->height_info.water = 0x00;

    ret->climb.edge_obj = NULL;
    ret->climb.can_hang = 0x00;
    ret->climb.next_z_space = 0.0;
    ret->climb.height_info = 0x00;
    ret->climb.edge_hit = 0x00;
    ret->climb.wall_hit = 0x00;
    ret->forvard_size = 48.0;                                                   ///@FIXME: magick number
    ret->Height = CHARACTER_BASE_HEIGHT;

    ret->traversed_object = NULL;

    Entity_CreateGhosts(ent);
}

void Character_Clean(struct entity_s *ent)
{
    character_p actor = ent->character;

    if(actor == NULL)
    {
        return;
    }

    actor->ent = NULL;

    inventory_node_p in, rn;
    in = actor->inventory;
    while(in)
    {
        rn = in;
        in = in->next;
        free(rn);
    }

    if(actor->hairs)
    {
        for(int i=0;i<actor->hair_count;i++)
        {
            Hair_Delete(actor->hairs[i]);
            actor->hairs[i] = NULL;
        }
        free(actor->hairs);
        actor->hairs = NULL;
        actor->hair_count = 0;
    }

    actor->height_info.ceiling_hit = 0x00;
    actor->height_info.floor_hit = 0x00;
    actor->height_info.water = 0x00;
    actor->climb.edge_hit = 0x00;

    free(ent->character);
    ent->character = NULL;
}


int32_t Character_AddItem(struct entity_s *ent, uint32_t item_id, int32_t count)// returns items count after in the function's end
{
    //Con_Notify(SYSNOTE_GIVING_ITEM, item_id, count, ent);
    if(ent->character == NULL)
    {
        return 0;
    }

    Gui_NotifierStart(item_id);

    base_item_p item = World_GetBaseItemByID(&engine_world, item_id);
    if(item == NULL) return 0;

    inventory_node_p last, i  = ent->character->inventory;

    count = (count == -1)?(item->count):(count);
    last = i;
    while(i)
    {
        if(i->id == item_id)
        {
            i->count += count;
            return i->count;
        }
        last = i;
        i = i->next;
    }

    i = (inventory_node_p)malloc(sizeof(inventory_node_t));
    i->id = item_id;
    i->count = count;
    i->next = NULL;
    if(last != NULL)
    {
        last->next = i;
    }
    else
    {
        ent->character->inventory = i;
    }

    return count;
}


int32_t Character_RemoveItem(struct entity_s *ent, uint32_t item_id, int32_t count) // returns items count after in the function's end
{
    if((ent->character == NULL) || (ent->character->inventory == NULL))
    {
        return 0;
    }

    inventory_node_p pi, i;
    pi = ent->character->inventory;
    i = pi->next;
    if(pi->id == item_id)
    {
        if(pi->count > count)
        {
            pi->count -= count;
            return pi->count;
        }
        else if(pi->count == count)
        {
            ent->character->inventory = pi->next;
            free(pi);
            return 0;
        }
        else // count_to_remove > current_items_count
        {
            return (int32_t)pi->count - (int32_t)count;
        }
    }

    while(i)
    {
        if(i->id == item_id)
        {
            if(i->count > count)
            {
                i->count -= count;
                return i->count;
            }
            else if(i->count == count)
            {
                pi->next = i->next;
                free(i);
                return 0;
            }
            else // count_to_remove > current_items_count
            {
                return (int32_t)i->count - (int32_t)count;
            }
        }
        pi = i;
        i = i->next;
    }

    return -count;
}


int32_t Character_RemoveAllItems(struct entity_s *ent)
{
    if((ent->character == NULL) || (ent->character->inventory == NULL))
    {
        return 0;
    }

    inventory_node_p curr_i = ent->character->inventory, next_i;
    int32_t ret = 0;

    while(curr_i != NULL)
    {
        next_i = curr_i->next;
        free(curr_i);
        curr_i = next_i;
        ret++;
    }
    ent->character->inventory = NULL;

    return ret;
}


int32_t Character_GetItemsCount(struct entity_s *ent, uint32_t item_id)         // returns items count
{
    if(ent->character == NULL)
    {
        return 0;
    }

    inventory_node_p i = ent->character->inventory;
    while(i)
    {
        if(i->id == item_id)
        {
            return i->count;
        }
        i = i->next;
    }

    return 0;
}

/**
 * Calculates next height info and information about next step
 * @param ent
 */
void Character_UpdateCurrentHeight(struct entity_s *ent)
{
    btScalar pos[3], t[3];
    t[0] = 0.0;
    t[1] = 0.0;
    t[2] = ent->bf->bone_tags[0].transform[12+2];
    Mat4_vec3_mul_macro(pos, ent->transform, t);
    Character_GetHeightInfo(pos, &ent->character->height_info, ent->character->Height);
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
            btScalar trpl[16];
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
            btScalar trpl[16];
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
void Character_GetHeightInfo(btScalar pos[3], struct height_info_s *fc, btScalar v_offset)
{
    btScalar from[3], to[3], base_pos[3];
    room_p r = (fc->self)?(fc->self->room):(NULL);
    collision_result_t cb;
    room_sector_p rs;

    fc->floor_hit = 0x00;
    fc->ceiling_hit = 0x00;
    fc->water = 0x00;
    fc->quicksand = 0x00;
    fc->transition_level = 32512.0;

    r = Room_FindPosCogerrence(pos, r);
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
                    fc->transition_level = (btScalar)rs->floor;
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
                    fc->transition_level = (btScalar)rs->floor;
                    if(fc->transition_level - fc->floor_point[2] > v_offset)
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
                    fc->transition_level = (btScalar)rs->ceiling;
                    fc->water = 0x01;
                    break;
                }
                else if((rs->owner_room->flags & TR_ROOM_FLAG_QUICKSAND) != 0x00)        // find water
                {
                    fc->transition_level = (btScalar)rs->ceiling;
                    if(fc->transition_level - fc->floor_point[2] > v_offset)
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
    vec3_copy(base_pos, pos);
    vec3_copy(from, pos);
    to[0] = from[0];
    to[1] = from[1];
    to[2] = from[2] - 4096.0f;

    fc->floor_hit = Physics_RayTest(&cb, from ,to, fc->self);
    if(fc->floor_hit)
    {
        vec3_copy(fc->floor_normale, cb.normale);
        vec3_copy(fc->floor_point, cb.point);
        fc->floor_obj = cb.obj;
    }

    to[2] = from[2] + 4096.0f;
    fc->ceiling_hit = Physics_RayTest(&cb, from ,to, fc->self);
    if(fc->ceiling_hit)
    {
        vec3_copy(fc->ceiling_normale, cb.normale);
        vec3_copy(fc->ceiling_point, cb.point);
        fc->ceiling_obj = cb.obj;
    }
}

/**
 * @function calculates next floor info + fantom filter + returns step info.
 * Current height info must be calculated!
 */
int Character_CheckNextStep(struct entity_s *ent, btScalar offset[3], struct height_info_s *nfc)
{
    btScalar pos[3], from[3], to[3], delta;
    height_info_p fc = &ent->character->height_info;
    collision_result_t cb;
    int ret = CHARACTER_STEP_HORIZONTAL;
    ///penetration test?

    vec3_add(pos, ent->transform + 12, offset);
    Character_GetHeightInfo(pos, nfc);

    if(fc->floor_hit && nfc->floor_hit)
    {
        delta = nfc->floor_point[2] - fc->floor_point[2];
        if(fabs(delta) < SPLIT_EPSILON)
        {
            from[2] = fc->floor_point[2];
            ret = CHARACTER_STEP_HORIZONTAL;                                    // horizontal
        }
        else if(delta < 0.0)                                                    // down way
        {
            delta = -delta;
            from[2] = fc->floor_point[2];
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
            from[2] = nfc->floor_point[2];
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
    else if(!fc->floor_hit && !nfc->floor_hit)
    {
        from[2] = pos[2];
        ret = CHARACTER_STEP_HORIZONTAL;                                        // horizontal? yes no maybe...
    }
    else if(!fc->floor_hit && nfc->floor_hit)                                   // strange case
    {
        from[2] = nfc->floor_point[2];
        ret = 0x00;
    }
    else //if(fc->floor_hit && !nfc->floor_hit)                                 // bottomless
    {
        from[2] = fc->floor_point[2];
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

    if(Physics_RayTest(&cb, from, to, fc->self))
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
    btScalar *pos = ent->transform + 12, *v1 = ent->transform + 4, *v2 = next_fc->floor_normale;

    return (next_fc->floor_point[2] > pos[2]) && (next_fc->floor_normale[2] < ent->character->critical_slant_z_component) &&
           (v1[0] * v2[0] + v1[1] * v2[1] < 0.0);
}

/**
 * @FIXME: MAGICK CONST!
 * @param ent - entity
 * @param offset - offset, when we check height
 * @param nfc - height info (floor / ceiling)
 */
climb_info_t Character_CheckClimbability(struct entity_s *ent, btScalar offset[3], struct height_info_s *nfc, btScalar test_height)
{
    climb_info_t ret;
    btScalar from[3], to[3], tmp[3];
    btScalar d, *pos = ent->transform + 12;
    btScalar n0[4], n1[4], n2[4];                                               // planes equations
    char up_founded;
    collision_result_t cb;
    extern GLfloat cast_ray[6];                                                 // pointer to the test line coordinates

    /*
     * init callbacks functions
     */
    vec3_add(tmp, pos, offset);                                                 // tmp = native offset point
    offset[2] += 128.0;                                                         ///@FIXME: stick for big slant
    ret.height_info = Character_CheckNextStep(ent, offset, nfc);
    offset[2] -= 128.0;
    ret.can_hang = 0;
    ret.edge_hit = 0x00;
    ret.edge_obj = NULL;
    ret.floor_limit = (ent->character->height_info.floor_hit)?(ent->character->height_info.floor_point[2]):(-9E10);
    ret.ceiling_limit = (ent->character->height_info.ceiling_hit)?(ent->character->height_info.ceiling_point[2]):(9E10);
    if(nfc->ceiling_hit && (nfc->ceiling_point[2] < ret.ceiling_limit))
    {
        ret.ceiling_limit = nfc->ceiling_point[2];
    }
    vec3_copy(ret.point, ent->character->climb.point);
    /*
     * check max height
     */
    if(ent->character->height_info.ceiling_hit && (tmp[2] > ent->character->height_info.ceiling_point[2] - ent->character->climb_r - 1.0))
    {
        tmp[2] = ent->character->height_info.ceiling_point[2] - ent->character->climb_r - 1.0;
    }

    /*
    * Let us calculate EDGE
    */
    from[0] = pos[0] - ent->transform[4 + 0] * ent->character->climb_r * 2.0;
    from[1] = pos[1] - ent->transform[4 + 1] * ent->character->climb_r * 2.0;
    from[2] = tmp[2];
    vec3_copy(to, tmp);

    //vec3_copy(cast_ray, from.m_floats);
    //vec3_copy(cast_ray+3, to.m_floats);

    up_founded = 0;
    test_height = (test_height >= ent->character->max_step_up_height)?(test_height):(ent->character->max_step_up_height);
    d = pos[2] + ent->bf->bb_max[2] - test_height;
    vec3_copy(cast_ray, to);
    vec3_copy(cast_ray+3, cast_ray);
    cast_ray[5] -= d;
    do
    {
        if(Physics_SphereTest(&cb, from, to, ent->character->climb_r, nfc->self))
        {
            if(cb.normale[2] >= 0.1)
            {
                up_founded = 1;
                vec3_copy(n0, cb.normale);
                n0[3] = -vec3_dot(n0, cb.point);
            }
            if(up_founded && (cb.normale[2] < 0.001))
            {
                vec3_copy(n1, cb.normale);
                n1[3] = -vec3_dot(n1, cb.point);
                ent->character->climb.edge_obj = cb.obj;
                up_founded = 2;
                break;
            }
        }
        else
        {
            tmp[0] = to[0];
            tmp[1] = to[1];
            tmp[2] = d;
            //vec3_copy(cast_ray, to.m_floats);
            //vec3_copy(cast_ray+3, tmp.m_floats);
            if(Physics_SphereTest(&cb, to, tmp, ent->character->climb_r, nfc->self))
            {
                up_founded = 1;
                vec3_copy(n0, cb.normale);
                n0[3] = -vec3_dot(n0, cb.point);
            }
            else
            {
                return ret;
            }
        }

        // mult 0.66 is magick, but it must be less than 1.0 and greater than 0.0;
        // close to 1.0 - bad precision, good speed;
        // close to 0.0 - bad speed, bad precision;
        // close to 0.5 - middle speed, good precision
        from[2] -= 0.66 * ent->character->climb_r;
        to[2] -= 0.66 * ent->character->climb_r;
    }
    while(to[2] >= d);                                                          // we can't climb under floor!

    if(up_founded != 2)
    {
        return ret;
    }

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
        return ret;
    }

    ret.edge_point[0] = n0[3] * (n1[1] * n2[2] - n1[2] * n2[1]) -
                        n1[3] * (n0[1] * n2[2] - n0[2] * n2[1]) +
                        n2[3] * (n0[1] * n1[2] - n0[2] * n1[1]);
    ret.edge_point[0] /= d;

    ret.edge_point[1] = n0[0] * (n1[3] * n2[2] - n1[2] * n2[3]) -
                        n1[0] * (n0[3] * n2[2] - n0[2] * n2[3]) +
                        n2[0] * (n0[3] * n1[2] - n0[2] * n1[3]);
    ret.edge_point[1] /= d;

    ret.edge_point[2] = n0[0] * (n1[1] * n2[3] - n1[3] * n2[1]) -
                        n1[0] * (n0[1] * n2[3] - n0[3] * n2[1]) +
                        n2[0] * (n0[1] * n1[3] - n0[3] * n1[1]);
    ret.edge_point[2] /= d;
    vec3_copy(ret.point, ret.edge_point);
    vec3_copy(cast_ray+3, ret.point);
    /*
     * unclimbable edge slant %)
     */
    vec3_cross(n2, n0, n1);
    d = ent->character->critical_slant_z_component;
    d *= d * (n2[0] * n2[0] + n2[1] * n2[1] + n2[2] * n2[2]);
    if(n2[2] * n2[2] > d)
    {
        return ret;
    }

    /*
     * Now, let us calculate z_angle
     */
    ret.edge_hit = 0x01;

    n2[2] = n2[0];
    n2[0] = n2[1];
    n2[1] =-n2[2];
    n2[2] = 0.0;
    if(n2[0] * ent->transform[4 + 0] + n2[1] * ent->transform[4 + 1] > 0)       // direction fixing
    {
        n2[0] = -n2[0];
        n2[1] = -n2[1];
    }

    vec3_copy(ret.n, n2);
    ret.up[0] = 0.0;
    ret.up[1] = 0.0;
    ret.up[2] = 1.0;
    ret.edge_z_ang = 180.0 * atan2f(n2[0], -n2[1]) / M_PI;
    ret.edge_tan_xy[0] = -n2[1];
    ret.edge_tan_xy[1] = n2[0];
    d = sqrt(n2[0] * n2[0] + n2[1] * n2[1]);
    ret.edge_tan_xy[0] /= d;
    ret.edge_tan_xy[1] /= d;
    vec3_copy(ret.t, ret.edge_tan_xy);

    if(!ent->character->height_info.floor_hit || (ret.edge_point[2] - ent->character->height_info.floor_point[2] >= ent->character->Height))
    {
        ret.can_hang = 1;
    }

    ret.next_z_space = 2.0 * ent->character->Height;
    if(nfc->floor_hit && nfc->ceiling_hit)
    {
        ret.next_z_space = nfc->ceiling_point[2] - nfc->floor_point[2];
    }

    return ret;
}


climb_info_t Character_CheckWallsClimbability(struct entity_s *ent)
{
    climb_info_t ret;
    btScalar from[3], to[3];
    btScalar wn2[2], t, *pos = ent->transform + 12;
    collision_result_t cb;

    ret.can_hang = 0x00;
    ret.wall_hit = 0x00;
    ret.edge_hit = 0x00;
    ret.edge_obj = NULL;
    ret.floor_limit = (ent->character->height_info.floor_hit)?(ent->character->height_info.floor_point[2]):(-9E10);
    ret.ceiling_limit = (ent->character->height_info.ceiling_hit)?(ent->character->height_info.ceiling_point[2]):(9E10);
    vec3_copy(ret.point, ent->character->climb.point);

    if(ent->character->height_info.walls_climb == 0x00)
    {
        return ret;
    }

    ret.up[0] = 0.0;
    ret.up[1] = 0.0;
    ret.up[2] = 1.0;

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
        return ret;
    }

    vec3_copy(ret.point, cb.point);
    vec3_copy(ret.n, cb.normale);
    wn2[0] = ret.n[0];
    wn2[1] = ret.n[1];
    t = sqrt(wn2[0] * wn2[0] + wn2[1] * wn2[1]);
    wn2[0] /= t;
    wn2[0] /= t;

    ret.t[0] =-wn2[1];
    ret.t[1] = wn2[0];
    ret.t[2] = 0.0;
    // now we have wall normale in XOY plane. Let us check all flags

    if((ent->character->height_info.walls_climb_dir & SECTOR_FLAG_CLIMB_NORTH) && (wn2[1] < -0.7))
    {
        ret.wall_hit = 0x01;                                                    // nW = (0, -1, 0);
    }
    if((ent->character->height_info.walls_climb_dir & SECTOR_FLAG_CLIMB_EAST) && (wn2[0] < -0.7))
    {
        ret.wall_hit = 0x01;                                                    // nW = (-1, 0, 0);
    }
    if((ent->character->height_info.walls_climb_dir & SECTOR_FLAG_CLIMB_SOUTH) && (wn2[1] > 0.7))
    {
        ret.wall_hit = 0x01;                                                    // nW = (0, 1, 0);
    }
    if((ent->character->height_info.walls_climb_dir & SECTOR_FLAG_CLIMB_WEST) && (wn2[0] > 0.7))
    {
        ret.wall_hit = 0x01;                                                    // nW = (1, 0, 0);
    }

    if(ret.wall_hit)
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

        if(Physics_SphereTest(&cb, from, to, ent->character->climb_r, ent->self))
        {
            ret.wall_hit = 0x02;
        }
    }

    // now check ceiling limit (and floor too... may be later)
    /*vec3_add(from.m_floats, point.m_floats, ent->transform+4);
    to = from;
    from.m_floats[2] += 520.0;                                                  ///@FIXME: magick;
    to.m_floats[2] -= 520.0;                                                    ///@FIXME: magick... again...
    cb->m_closestHitFraction = 1.0;
    cb->m_collisionObject = NULL;
    bt_engine_dynamicsWorld->rayTest(from, to, *cb);
    if(cb->hasHit())
    {
        point.setInterpolate3(from, to, cb->m_closestHitFraction);
        ret.ceiling_limit = (ret.ceiling_limit > point.m_floats[2])?(point.m_floats[2]):(ret.ceiling_limit);
    }*/

    return ret;
}


void Character_SetToJump(struct entity_s *ent, btScalar v_vertical, btScalar v_horizontal)
{
    btScalar t;

    if(!ent->character)
    {
        return;
    }

    // Jump length is a speed value multiplied by global speed coefficient.
    t = v_horizontal * ent->speed_mult;

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
    ent->speed[2] = v_vertical * ent->speed_mult;
    ent->move_type = MOVE_FREE_FALLING;
}


void Character_Lean(struct entity_s *ent, character_command_p cmd, btScalar max_lean)
{
    btScalar neg_lean   = 360.0 - max_lean;
    btScalar lean_coeff = (max_lean == 0.0)?(48.0):(max_lean * 3);

    // Continously lean character, according to current left/right direction.

    if((cmd->move[1] == 0) || (max_lean == 0.0))       // No direction - restore straight vertical position!
    {
        if(ent->angles[2] != 0.0)
        {
            if(ent->angles[2] < 180.0)
            {
                ent->angles[2] -= ((abs(ent->angles[2]) + lean_coeff) / 2) * engine_frame_time;
                if(ent->angles[2] < 0.0) ent->angles[2] = 0.0;
            }
            else
            {
                ent->angles[2] += ((360 - abs(ent->angles[2]) + lean_coeff) / 2) * engine_frame_time;
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
                ent->angles[2] += ((abs(ent->angles[2]) + lean_coeff) / 2) * engine_frame_time;
                if(ent->angles[2] > max_lean)
                    ent->angles[2] = max_lean;
            }
            else if(ent->angles[2] > 180.0) // Approaching from left
            {
                ent->angles[2] += ((360.0 - abs(ent->angles[2]) + (lean_coeff*2) / 2) * engine_frame_time);
                if(ent->angles[2] < 180.0) ent->angles[2] = 0.0;
            }
            else    // Reduce previous lean
            {
                ent->angles[2] -= ((abs(ent->angles[2]) + lean_coeff) / 2) * engine_frame_time;
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
                ent->angles[2] -= ((360.0 - abs(ent->angles[2]) + lean_coeff) / 2) * engine_frame_time;
                if(ent->angles[2] < neg_lean)
                    ent->angles[2] = neg_lean;
            }
            else if(ent->angles[2] < 180.0) // Approaching from right
            {
                ent->angles[2] -= ((abs(ent->angles[2]) + (lean_coeff*2)) / 2) * engine_frame_time;
                if(ent->angles[2] < 0.0) ent->angles[2] += 360.0;
            }
            else    // Approaching from center
            {
                ent->angles[2] += ((360.0 - abs(ent->angles[2]) + lean_coeff) / 2) * engine_frame_time;
                if(ent->angles[2] > 360.0) ent->angles[2] -= 360.0;
            }
        }
    }
}


/*
 * Linear inertia is absolutely needed for in-water states, and also it gives
 * more organic feel to land animations.
 */
btScalar Character_InertiaLinear(struct entity_s *ent, btScalar max_speed, btScalar accel, int8_t command)
{
    if((!ent) || (!ent->character)) return 0.0;

    if((accel == 0.0) || (accel >= max_speed))
    {
        if(command)
        {
            ent->inertia_linear = max_speed;
        }
        else
        {
            ent->inertia_linear = 0.0;
        }
    }
    else
    {
        if(command)
        {
            if(ent->inertia_linear < max_speed)
            {
                ent->inertia_linear += max_speed * accel * engine_frame_time;
                if(ent->inertia_linear > max_speed) ent->inertia_linear = max_speed;
            }
        }
        else
        {
            if(ent->inertia_linear > 0.0)
            {
                ent->inertia_linear -= max_speed * accel * engine_frame_time;
                if(ent->inertia_linear < 0.0) ent->inertia_linear = 0.0;
            }
        }
    }

    return ent->inertia_linear * ent->speed_mult;
}

/*
 * Angular inertia is used on keyboard-driven (non-analog) rotational controls.
 */
btScalar Character_InertiaAngular(struct entity_s *ent, btScalar max_angle, btScalar accel, uint8_t axis)
{
    if((!ent) || (!ent->character) || (axis > 1)) return 0.0;

    uint8_t curr_rot_dir = 0;
    if     (ent->character->cmd.rot[axis] < 0.0) { curr_rot_dir = 1; }
    else if(ent->character->cmd.rot[axis] > 0.0) { curr_rot_dir = 2; }

    if((!curr_rot_dir) || (max_angle == 0.0) || (accel == 0.0))
    {
        ent->inertia_angular[axis] = 0.0;
    }
    else
    {
        if(ent->inertia_angular[axis] != max_angle)
        {
            if(curr_rot_dir == 2)
            {
                if(ent->inertia_angular[axis] < 0.0)
                {
                    ent->inertia_angular[axis] = 0.0;
                }
                else
                {
                    ent->inertia_angular[axis] += max_angle * accel * engine_frame_time;
                    if(ent->inertia_angular[axis] > max_angle) ent->inertia_angular[axis] = max_angle;
                }
            }
            else
            {
                if(ent->inertia_angular[axis] > 0.0)
                {
                    ent->inertia_angular[axis] = 0.0;
                }
                else
                {
                    ent->inertia_angular[axis] -= max_angle * accel * engine_frame_time;
                    if(ent->inertia_angular[axis] < -max_angle) ent->inertia_angular[axis] = -max_angle;
                }
            }
        }
    }

    return fabs(ent->inertia_angular[axis]) * ent->character->cmd.rot[axis];
}

/*
 * MOVE IN DIFFERENCE CONDITIONS
 */
int Character_MoveOnFloor(struct entity_s *ent)
{
    btScalar norm_move_xy_len, t, ang, *pos = ent->transform + 12;
    btScalar tv[3], move[3], norm_move_xy[2];

    if(!ent->character)
    {
        return 0;
    }

    /*
     * init height info structure
     */
    ent->character->resp.horizontal_collide = 0x00;
    ent->character->resp.vertical_collide = 0x00;
    // First of all - get information about floor and ceiling!!!
    Character_UpdateCurrentHeight(ent);
    if(ent->character->height_info.floor_hit && (ent->character->height_info.floor_point[2] + 1.0 >= ent->transform[12+2] + ent->bf->bb_min[2]))
    {
        engine_container_p cont = ent->character->height_info.floor_obj;
        if((cont != NULL) && (cont->object_type == OBJECT_ENTITY))
        {
            entity_p e = (entity_p)cont->object;
            if(e->callback_flags & ENTITY_CALLBACK_STAND)
            {
                lua_ExecEntity(engine_lua, ENTITY_CALLBACK_STAND, e->id, ent->id);
            }
        }
    }

    /*
     * check move type
     */
    if(ent->character->height_info.floor_hit || (ent->character->resp.vertical_collide & 0x01))
    {
        if(ent->character->height_info.floor_point[2] + ent->character->fall_down_height < pos[2])
        {
            ent->move_type = MOVE_FREE_FALLING;
            ent->speed[2] = 0.0;
            return -1;                                                          // nothing to do here
        }
        else
        {
            ent->character->resp.vertical_collide |= 0x01;
        }

        vec3_copy(tv, ent->character->height_info.floor_normale);
        if(tv[2] > 0.02 && tv[2] < ent->character->critical_slant_z_component)
        {
            tv[2] = -tv[2];
            t = ent->speed_mult * DEFAULT_CHARACTER_SLIDE_SPEED_MULT;
            vec3_mul_scalar(ent->speed, tv, t);                                 // slide down direction
            ang = 180.0 * atan2f(tv[0], -tv[1]) / M_PI;                         // from -180 deg to +180 deg
            //ang = (ang < 0.0)?(ang + 360.0):(ang);
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
            t = ent->current_speed * ent->speed_mult;
            ent->character->resp.vertical_collide |= 0x01;

            ent->angles[0] += Character_InertiaAngular(ent, 1.0, ROT_SPEED_LAND, 0);

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
    if(ent->character->height_info.floor_hit)
    {
        if(ent->character->height_info.floor_point[2] + ent->character->fall_down_height > pos[2])
        {
            btScalar dz_to_land = engine_frame_time * 2400.0;                   ///@FIXME: magick
            if(pos[2] > ent->character->height_info.floor_point[2] + dz_to_land)
            {
                pos[2] -= dz_to_land;
                Entity_FixPenetrations(ent, NULL);
            }
            else if(pos[2] > ent->character->height_info.floor_point[2])
            {
                pos[2] = ent->character->height_info.floor_point[2];
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
        if((pos[2] < ent->character->height_info.floor_point[2]) && (Entity_GetNoFixAllFlag(ent) == 0x00))
        {
            pos[2] = ent->character->height_info.floor_point[2];
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
    btScalar move[3], g[3], *pos = ent->transform + 12;

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

    btScalar rot = Character_InertiaAngular(ent, 1.0, ROT_SPEED_FREEFALL, 0);
    ent->angles[0] += rot;
    ent->angles[1] = 0.0;

    Entity_UpdateTransform(ent);                                                 // apply rotations

    /*btScalar t = ent->current_speed * bf-> ent->character->speed_mult;        ///@TODO: fix speed update in Entity_Frame function and other;
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
    ent->speed[2] = (ent->speed[2] < -FREE_FALL_SPEED_MAXIMUM)?(-FREE_FALL_SPEED_MAXIMUM):(ent->speed[2]);
    vec3_RotateZ(ent->speed, ent->speed, rot);

    Character_UpdateCurrentHeight(ent);

    if(ent->self->room && (ent->self->room->flags & TR_ROOM_FLAG_WATER))
    {
        if(ent->speed[2] < 0.0)
        {
            ent->current_speed = 0.0;
            ent->speed[0] = 0.0;
            ent->speed[1] = 0.0;
        }

        if((engine_world.version < TR_II))//Lara cannot wade in < TRII so when floor < transition level she has to swim
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
    if(ent->character->height_info.ceiling_hit && ent->speed[2] > 0.0)
    {
        if(ent->character->height_info.ceiling_point[2] < ent->bf->bb_max[2] + pos[2])
        {
            pos[2] = ent->character->height_info.ceiling_point[2] - ent->bf->bb_max[2];
            ent->speed[2] = 0.0;
            ent->character->resp.vertical_collide |= 0x02;
            Entity_FixPenetrations(ent, NULL);
            Entity_UpdateRoomPos(ent);
        }
    }
    if(ent->character->height_info.floor_hit && ent->speed[2] < 0.0)   // move down
    {
        if(ent->character->height_info.floor_point[2] >= pos[2] + ent->bf->bb_min[2] + move[2])
        {
            pos[2] = ent->character->height_info.floor_point[2];
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

    if(ent->character->height_info.ceiling_hit && ent->speed[2] > 0.0)
    {
        if(ent->character->height_info.ceiling_point[2] < ent->bf->bb_max[2] + pos[2])
        {
            pos[2] = ent->character->height_info.ceiling_point[2] - ent->bf->bb_max[2];
            ent->speed[2] = 0.0;
            ent->character->resp.vertical_collide |= 0x02;
        }
    }
    if(ent->character->height_info.floor_hit && ent->speed[2] < 0.0)   // move down
    {
        if(ent->character->height_info.floor_point[2] >= pos[2] + ent->bf->bb_min[2] + move[2])
        {
            pos[2] = ent->character->height_info.floor_point[2];
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
    btScalar move[3];
    btScalar t, *pos = ent->transform + 12;

    ent->character->resp.slide = 0x00;
    ent->character->resp.horizontal_collide = 0x00;
    ent->character->resp.vertical_collide = 0x00;

    t = ent->current_speed * ent->speed_mult;
    ent->character->resp.vertical_collide |= 0x01;

    ent->angles[0] += Character_InertiaAngular(ent, 1.0, ROT_SPEED_MONKEYSWING, 0);
    ent->angles[1] = 0.0;
    ent->angles[2] = 0.0;
    Entity_UpdateTransform(ent);                                                 // apply rotations

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
    Character_UpdateCurrentHeight(ent);
    vec3_add(pos, pos, move);
    Entity_FixPenetrations(ent, move);                                          // get horizontal collide
    ///@FIXME: rewrite conditions! or add fixer to update_entity_rigid_body func
    if(ent->character->height_info.ceiling_hit && (pos[2] + ent->bf->bb_max[2] - ent->character->height_info.ceiling_point[2] > - 0.33 * ent->character->min_step_up_height))
    {
        pos[2] = ent->character->height_info.ceiling_point[2] - ent->bf->bb_max[2];
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
    btScalar move[3], t, *pos = ent->transform + 12;

    ent->character->resp.slide = 0x00;
    ent->character->resp.horizontal_collide = 0x00;
    ent->character->resp.vertical_collide = 0x00;

    *climb = Character_CheckWallsClimbability(ent);
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

    t = ent->current_speed * ent->speed_mult;
    vec3_mul_scalar(ent->speed, move, t);
    vec3_mul_scalar(move, ent->speed, engine_frame_time);

    Entity_GhostUpdate(ent);
    Character_UpdateCurrentHeight(ent);
    vec3_add(pos, pos, move);
    Entity_FixPenetrations(ent, move);                                          // get horizontal collide
    Entity_UpdateRoomPos(ent);

    *climb = Character_CheckWallsClimbability(ent);
    if(pos[2] + ent->bf->bb_max[2] > climb->ceiling_limit)
    {
        pos[2] = climb->ceiling_limit - ent->bf->bb_max[2];
    }

    return 1;
}

/*
 * CLIMBING - MOVE NO Z LANDING
 */
int Character_Climbing(struct entity_s *ent)
{
    btScalar move[3];
    btScalar t, *pos = ent->transform + 12;
    btScalar z = pos[2];

    ent->character->resp.slide = 0x00;
    ent->character->resp.horizontal_collide = 0x00;
    ent->character->resp.vertical_collide = 0x00;

    t = ent->current_speed * ent->speed_mult;
    ent->character->resp.vertical_collide |= 0x01;
    ent->angles[0] += ent->character->cmd.rot[0];
    ent->angles[1] = 0.0;
    ent->angles[2] = 0.0;
    Entity_UpdateTransform(ent);                                                 // apply rotations

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
    btScalar move[3], *pos = ent->transform + 12;

    // Check current place.

    if(ent->self->room && !(ent->self->room->flags & TR_ROOM_FLAG_WATER))
    {
        ent->move_type = MOVE_FREE_FALLING;
        return 2;
    }

    ent->character->resp.slide = 0x00;
    ent->character->resp.horizontal_collide = 0x00;
    ent->character->resp.vertical_collide = 0x00;

    // Calculate current speed.

    btScalar t = Character_InertiaLinear(ent, MAX_SPEED_UNDERWATER, INERTIA_SPEED_UNDERWATER, ent->character->cmd.jump);

    if(!ent->character->resp.kill)   // Block controls if Lara is dead.
    {
        ent->angles[0] += Character_InertiaAngular(ent, 1.0, ROT_SPEED_UNDERWATER, 0);
        ent->angles[1] -= Character_InertiaAngular(ent, 1.0, ROT_SPEED_UNDERWATER, 1);
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
        vec3_mul_scalar(ent->speed, ent->transform +4 , t);                     // OY move only!
    }
    vec3_mul_scalar(move, ent->speed, engine_frame_time);

    Entity_GhostUpdate(ent);
    vec3_add(pos, pos, move);
    Entity_FixPenetrations(ent, move);                                          // get horizontal collide

    Entity_UpdateRoomPos(ent);
    if(ent->character->height_info.water && (pos[2] + ent->bf->bb_max[2] >= ent->character->height_info.transition_level))
    {
        if(/*(spd.m_floats[2] > 0.0)*/ent->transform[4 + 2] > 0.67)             ///@FIXME: magick!
        {
            ent->move_type = MOVE_ON_WATER;
            //pos[2] = fc.transition_level;
            return 2;
        }
        if(!ent->character->height_info.floor_hit || (ent->character->height_info.transition_level - ent->character->height_info.floor_point[2] >= ent->character->Height))
        {
            pos[2] = ent->character->height_info.transition_level - ent->bf->bb_max[2];
        }
    }

    return 1;
}


int Character_MoveOnWater(struct entity_s *ent)
{
    btScalar move[3];
    btScalar *pos = ent->transform + 12;

    ent->character->resp.slide = 0x00;
    ent->character->resp.horizontal_collide = 0x00;
    ent->character->resp.vertical_collide = 0x00;

    ent->angles[0] += Character_InertiaAngular(ent, 1.0, ROT_SPEED_ONWATER, 0);
    ent->angles[1] = 0.0;
    ent->angles[2] = 0.0;
    Entity_UpdateTransform(ent);     // apply rotations

    // Calculate current speed.

    btScalar t = Character_InertiaLinear(ent, MAX_SPEED_ONWATER, INERTIA_SPEED_ONWATER, ((abs(ent->character->cmd.move[0])) | (abs(ent->character->cmd.move[1]))));

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
        btScalar pos[] = {(btScalar)(ch_s->pos[0] + TR_METERING_SECTORSIZE), (btScalar)(ch_s->pos[1]), (btScalar)0.0};
        obj_s = Room_GetSectorRaw(ch_s->owner_room, pos);
    }
    else if(ch->transform[4 + 0] < -0.9)
    {
        btScalar pos[] = {(btScalar)(ch_s->pos[0] - TR_METERING_SECTORSIZE), (btScalar)(ch_s->pos[1]), (btScalar)0.0};
        obj_s = Room_GetSectorRaw(ch_s->owner_room, pos);
    }
    // OY move case
    else if(ch->transform[4 + 1] > 0.9)
    {
        btScalar pos[] = {(btScalar)(ch_s->pos[0]), (btScalar)(ch_s->pos[1] + TR_METERING_SECTORSIZE), (btScalar)0.0};
        obj_s = Room_GetSectorRaw(ch_s->owner_room, pos);
    }
    else if(ch->transform[4 + 1] < -0.9)
    {
        btScalar pos[] = {(btScalar)(ch_s->pos[0]), (btScalar)(ch_s->pos[1] - TR_METERING_SECTORSIZE), (btScalar)0.0};
        obj_s = Room_GetSectorRaw(ch_s->owner_room, pos);
    }

    if(obj_s != NULL)
    {
        obj_s = Sector_CheckPortalPointer(obj_s);
        for(engine_container_p cont = obj_s->owner_room->containers;cont!=NULL;cont=cont->next)
        {
            if(cont->object_type == OBJECT_ENTITY)
            {
                entity_p e = (entity_p)cont->object;
                if((e->type_flags & ENTITY_TYPE_TRAVERSE) && (1 == OBB_OBB_Test(e->obb, ch->obb) && (fabs(e->transform[12 + 2] - ch->transform[12 + 2]) < 1.1)))
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
int Sector_AllowTraverse(struct room_sector_s *rs, btScalar floor, struct engine_container_s *cont)
{
    btScalar f0 = rs->floor_corners[0][2];
    if((rs->floor_corners[0][2] != f0) || (rs->floor_corners[1][2] != f0) ||
       (rs->floor_corners[2][2] != f0) || (rs->floor_corners[3][2] != f0))
    {
        return 0x00;
    }

    if((fabs(floor - f0) < 1.1) && (rs->ceiling - rs->floor >= TR_METERING_SECTORSIZE))
    {
        return 0x01;
    }

    btScalar from[3], to[3];
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
            btScalar pos[] = {(btScalar)(obj_s->pos[0] - TR_METERING_SECTORSIZE), (btScalar)(obj_s->pos[1]), (btScalar)0.0};
            ch_s = Room_GetSectorRaw(obj_s->owner_room, pos);
        }
        else if(ch->transform[4 + 0] < -0.8)
        {
            btScalar pos[] = {(btScalar)(obj_s->pos[0] + TR_METERING_SECTORSIZE), (btScalar)(obj_s->pos[1]), (btScalar)0.0};
            ch_s = Room_GetSectorRaw(obj_s->owner_room, pos);
        }
        // OY move case
        else if(ch->transform[4 + 1] > 0.8)
        {
            btScalar pos[] = {(btScalar)(obj_s->pos[0]), (btScalar)(obj_s->pos[1] - TR_METERING_SECTORSIZE), (btScalar)0.0};
            ch_s = Room_GetSectorRaw(obj_s->owner_room, pos);
        }
        else if(ch->transform[4 + 1] < -0.8)
        {
            btScalar pos[] = {(btScalar)(obj_s->pos[0]), (btScalar)(obj_s->pos[1] + TR_METERING_SECTORSIZE), (btScalar)0.0};
            ch_s = Room_GetSectorRaw(obj_s->owner_room, pos);
        }
        ch_s = Sector_CheckPortalPointer(ch_s);
    }

    if((ch_s == NULL) || (obj_s == NULL))
    {
        return 0x00;
    }

    btScalar floor = ch->transform[12 + 2];
    if((ch_s->floor != obj_s->floor) || (Sector_AllowTraverse(ch_s, floor, ch->self) == 0x00) || (Sector_AllowTraverse(obj_s, floor, obj->self) == 0x00))
    {
        return 0x00;
    }

    collision_result_t cb;
    btScalar from[3], to[3];

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
        btScalar pos[] = {(btScalar)(obj_s->pos[0] + TR_METERING_SECTORSIZE), (btScalar)(obj_s->pos[1]), (btScalar)0.0};
        next_s = Room_GetSectorRaw(obj_s->owner_room, pos);
    }
    else if(ch->transform[4 + 0] < -0.8)
    {
        btScalar pos[] = {(btScalar)(obj_s->pos[0] - TR_METERING_SECTORSIZE), (btScalar)(obj_s->pos[1]), (btScalar)0.0};
        next_s = Room_GetSectorRaw(obj_s->owner_room, pos);
    }
    // OY move case
    else if(ch->transform[4 + 1] > 0.8)
    {
        btScalar pos[] = {(btScalar)(obj_s->pos[0]), (btScalar)(obj_s->pos[1] + TR_METERING_SECTORSIZE), (btScalar)0.0};
        next_s = Room_GetSectorRaw(obj_s->owner_room, pos);
    }
    else if(ch->transform[4 + 1] < -0.8)
    {
        btScalar pos[] = {(btScalar)(obj_s->pos[0]), (btScalar)(obj_s->pos[1] - TR_METERING_SECTORSIZE), (btScalar)0.0};
        next_s = Room_GetSectorRaw(obj_s->owner_room, pos);
    }

    next_s = Sector_CheckPortalPointer(next_s);
    if((next_s != NULL) && (Sector_AllowTraverse(next_s, floor, ch->self) == 0x01))
    {
        from[0] = obj_s->pos[0];
        from[1] = obj_s->pos[1];
        from[2] = floor + 0.5 * TR_METERING_SECTORSIZE;

        to[0] = next_s->pos[0];
        to[1] = next_s->pos[1];
        to[2] = from[2];
        if(!Physics_SphereTest(&cb, from ,to, 0.48 * TR_METERING_SECTORSIZE, obj->self))
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
        btScalar pos[] = {(btScalar)(ch_s->pos[0] - TR_METERING_SECTORSIZE), (btScalar)(ch_s->pos[1]), (btScalar)0.0};
        next_s = Room_GetSectorRaw(ch_s->owner_room, pos);
    }
    else if(ch->transform[4 + 0] < -0.8)
    {
        btScalar pos[] = {(btScalar)(ch_s->pos[0] + TR_METERING_SECTORSIZE), (btScalar)(ch_s->pos[1]), (btScalar)0.0};
        next_s = Room_GetSectorRaw(ch_s->owner_room, pos);
    }
    // OY move case
    else if(ch->transform[4 + 1] > 0.8)
    {
        btScalar pos[] = {(btScalar)(ch_s->pos[0]), (btScalar)(ch_s->pos[1] - TR_METERING_SECTORSIZE), (btScalar)0.0};
        next_s = Room_GetSectorRaw(ch_s->owner_room, pos);
    }
    else if(ch->transform[4 + 1] < -0.8)
    {
        btScalar pos[] = {(btScalar)(ch_s->pos[0]), (btScalar)(ch_s->pos[1] + TR_METERING_SECTORSIZE), (btScalar)0.0};
        next_s = Room_GetSectorRaw(ch_s->owner_room, pos);
    }

    next_s = Sector_CheckPortalPointer(next_s);
    if((next_s != NULL) && (Sector_AllowTraverse(next_s, floor, ch->self) == 0x01))
    {
        from[0] = ch_s->pos[0];
        from[1] = ch_s->pos[1];
        from[2] = floor + 0.5 * TR_METERING_SECTORSIZE;

        to[0] = next_s->pos[0];
        to[1] = next_s->pos[1];
        to[2] = from[2];
        if(!Physics_SphereTest(&cb, from ,to, 0.48 * TR_METERING_SECTORSIZE, ch->self))
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

    Character_UpdatePlatformPreStep(ent);

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
                if(!Character_ChangeParam(ent, PARAM_AIR, -3.0))
                    Character_ChangeParam(ent, PARAM_HEALTH, -3.0);
            }
            else if(ent->character->height_info.quicksand == 0x01)
            {
                Character_ChangeParam(ent, PARAM_AIR, 3.0);
            }
            else
            {
                Character_SetParam(ent, PARAM_AIR, PARAM_ABSOLUTE_MAX);
            }


            if((ent->bf->animations.last_state == TR_STATE_LARA_SPRINT) ||
               (ent->bf->animations.last_state == TR_STATE_LARA_SPRINT_ROLL))
            {
                Character_ChangeParam(ent, PARAM_STAMINA, -0.5);
            }
            else
            {
                Character_ChangeParam(ent, PARAM_STAMINA,  0.5);
            }
            break;

        case MOVE_ON_WATER:
            Character_ChangeParam(ent, PARAM_AIR, 3.0);;
            break;

        case MOVE_UNDERWATER:
            if(!Character_ChangeParam(ent, PARAM_AIR, -1.0))
            {
                if(!Character_ChangeParam(ent, PARAM_HEALTH, -3.0))
                {
                    ent->character->resp.kill = 1;
                }
            }
            break;

        default:
            break;  // Add quicksand later...
    }
}

bool IsCharacter(struct entity_s *ent)
{
    return (ent != NULL) && (ent->character != NULL);
}

int Character_SetParamMaximum(struct entity_s *ent, int parameter, float max_value)
{
    if((!IsCharacter(ent)) || (parameter >= PARAM_LASTINDEX))
        return 0;

    max_value = (max_value < 0)?(0):(max_value);    // Clamp max. to at least zero
    ent->character->parameters.maximum[parameter] = max_value;
    return 1;
}

int Character_SetParam(struct entity_s *ent, int parameter, float value)
{
    if((!IsCharacter(ent)) || (parameter >= PARAM_LASTINDEX))
        return 0;

    float maximum = ent->character->parameters.maximum[parameter];

    value = (value >= 0)?(value):(maximum); // Char params can't be less than zero.
    value = (value <= maximum)?(value):(maximum);

    ent->character->parameters.param[parameter] = value;
    return 1;
}

float Character_GetParam(struct entity_s *ent, int parameter)
{
    if((!IsCharacter(ent)) || (parameter >= PARAM_LASTINDEX))
        return 0;

    return ent->character->parameters.param[parameter];
}

int Character_ChangeParam(struct entity_s *ent, int parameter, float value)
{
    if((!IsCharacter(ent)) || (parameter >= PARAM_LASTINDEX))
        return 0;

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

// overrided == 0x00: no overriding;
// overrided == 0x01: overriding mesh in armed state;
// overrided == 0x02: add mesh to slot in armed state;
// overrided == 0x03: overriding mesh in disarmed state;
// overrided == 0x04: add mesh to slot in disarmed state;
///@TODO: separate mesh replacing control and animation disabling / enabling
int Character_SetWeaponModel(struct entity_s *ent, int weapon_model, int armed)
{
    skeletal_model_p sm = World_GetModelByID(&engine_world, weapon_model);

    if((sm != NULL) && (ent->bf->bone_tag_count == sm->mesh_count) && (sm->animation_count >= 4))
    {
        skeletal_model_p bm = ent->bf->animations.model;
        if(ent->bf->animations.next == NULL)
        {
            Entity_AddOverrideAnim(ent, weapon_model);
        }
        else
        {
            ent->bf->animations.next->model = sm;
        }

        for(int i=0;i<bm->mesh_count;i++)
        {
            ent->bf->bone_tags[i].mesh_base = bm->mesh_tree[i].mesh_base;
            ent->bf->bone_tags[i].mesh_slot = NULL;
        }

        if(armed != 0)
        {
            for(int i=0;i<bm->mesh_count;i++)
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
            for(int i=0;i<bm->mesh_count;i++)
            {
                if(sm->mesh_tree[i].replace_mesh == 0x03)
                {
                    ent->bf->bone_tags[i].mesh_base = sm->mesh_tree[i].mesh_base;
                }
                else if(sm->mesh_tree[i].replace_mesh == 0x04)
                {
                    ent->bf->bone_tags[i].mesh_slot = sm->mesh_tree[i].mesh_base;
                }
            }
            ent->bf->animations.next->model = NULL;
        }

        return 1;
    }
    else
    {
        // do unarmed default model
        skeletal_model_p bm = ent->bf->animations.model;
        for(int i=0;i<bm->mesh_count;i++)
        {
            ent->bf->bone_tags[i].mesh_base = bm->mesh_tree[i].mesh_base;
            ent->bf->bone_tags[i].mesh_slot = NULL;
        }
        if(ent->bf->animations.next != NULL)
        {
            ent->bf->animations.next->model = NULL;
        }
    }

    return 0;
}
