
#include "world.h"

#include <bullet/btBulletCollisionCommon.h>
#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/BulletCollision/NarrowPhaseCollision/btRaycastCallback.h>

#include "character_controller.h"
#include "obb.h"
#include "anim_state_control.h"
#include "engine.h"
#include "entity.h"
#include "gui.h"
#include "mesh.h"
#include "hair.h"
#include "vmath.h"
#include "polygon.h"
#include "resource.h"
#include "console.h"
#include "string.h"

Character::Character(std::shared_ptr<Entity> ent)
    : m_climbSensor(new btSphereShape(ent->m_character->m_climbR))
    , m_rayCb(std::make_shared<BtEngineClosestRayResultCallback>(ent->m_self.get()))
    , m_convexCb(std::make_shared<BtEngineClosestConvexResultCallback>(ent->m_self.get()))
{
    m_rayCb->m_collisionFilterMask = btBroadphaseProxy::StaticFilter | btBroadphaseProxy::KinematicFilter;
    m_convexCb->m_collisionFilterMask = btBroadphaseProxy::StaticFilter | btBroadphaseProxy::KinematicFilter;

    ent->m_dirFlag = ENT_STAY;

    m_heightInfo.cb = m_rayCb;
    m_heightInfo.ccb = m_convexCb;

    ent->createGhosts();
}

int32_t Character_AddItem(std::shared_ptr<Entity> ent, uint32_t item_id, int32_t count)// returns items count after in the function's end
{
    //Con_Notify(SYSNOTE_GIVING_ITEM, item_id, count, ent);
    if(ent->m_character == NULL)
    {
        return 0;
    }

    Gui_NotifierStart(item_id);

    auto item = engine_world.getBaseItemByID(item_id);
    if(!item)
        return 0;


    count = (count == -1) ? item->count : count;

    for(InventoryNode& i : ent->m_character->m_inventory) {
        if(i.id == item_id)
        {
            i.count += count;
            return i.count;
        }
    }

    InventoryNode i;
    i.id = item_id;
    i.count = count;
    ent->m_character->m_inventory.push_back(i);

    return count;
}


int32_t Character_RemoveItem(std::shared_ptr<Entity> ent, uint32_t item_id, int32_t count) // returns items count after in the function's end
{
    if((ent->m_character == NULL) || ent->m_character->m_inventory.empty())
    {
        return 0;
    }

    auto pi = ent->m_character->m_inventory.begin();
    if(pi->id == item_id)
    {
        if(pi->count > count)
        {
            pi->count -= count;
            return pi->count;
        }
        else if(pi->count == count)
        {
            ent->m_character->m_inventory.pop_front();
            return 0;
        }
        else // count_to_remove > current_items_count
        {
            return (int32_t)pi->count - (int32_t)count;
        }
    }

    auto i = std::next(pi);

    while(i != ent->m_character->m_inventory.end()) {
        // i = pi+1
        if(i->id == item_id) {
            if(i->count > count) {
                i->count -= count;
                return i->count;
            }
            else if(i->count == count) {
                ent->m_character->m_inventory.erase(i);
                return 0;
            }
            else { // count_to_remove > current_items_count
                return (int32_t)i->count - (int32_t)count;
            }
        }
        pi = i;
        ++i;
    }

    return -count;
}


int32_t Character_RemoveAllItems(std::shared_ptr<Entity> ent)
{
    if((ent->m_character == NULL) || ent->m_character->m_inventory.empty())
    {
        return 0;
    }
    auto ret = ent->m_character->m_inventory.size();
    ent->m_character->m_inventory.clear();
    return ret;
}


int32_t Character_GetItemsCount(std::shared_ptr<Entity> ent, uint32_t item_id)         // returns items count
{
    if(ent->m_character == NULL)
    {
        return 0;
    }

    return ent->m_character->m_inventory.size();
}

/**
 * Calculates next height info and information about next step
 * @param ent
 */
void Character_UpdateCurrentHeight(std::shared_ptr<Entity> ent)
{
    btVector3 t;
    t[0] = 0.0;
    t[1] = 0.0;
    t[2] = ent->m_bf.bone_tags[0].transform.getOrigin()[2];
    auto pos = ent->m_transform * t;
    Character_GetHeightInfo(pos, &ent->m_character->m_heightInfo, ent->m_character->m_height);
}

/*
 * Move character to the point where to platfom mowes
 */
void Character_UpdatePlatformPreStep(std::shared_ptr<Entity> ent)
{
#if 0
    if(ent->character->platform)
    {
        EngineContainer* cont = (EngineContainer*)ent->character->platform->getUserPointer();
        if(cont && (cont->object_type == OBJECT_ENTITY/* || cont->object_type == OBJECT_BULLET_MISC*/))
        {
            btScalar trpl[16];
            ent->character->platform->getWorldTransform().getOpenGLMatrix(trpl);
#if 0
            new_tr = trpl * ent->character->local_platform;
            vec3_copy(ent->transform.getOrigin(), new_tr + 12);
#else
            ///make something with platform rotation
            ent->transform = trpl * ent->character->local_platform;
#endif
        }
    }
#endif
}

/*
 * Get local character transform relative platfom
 */
void Character_UpdatePlatformPostStep(std::shared_ptr<Entity> ent)
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
        EngineContainer* cont = (EngineContainer*)ent->character->platform->getUserPointer();
        if(cont && (cont->object_type == OBJECT_ENTITY/* || cont->object_type == OBJECT_BULLET_MISC*/))
        {
            btScalar trpl[16];
            ent->character->platform->getWorldTransform().getOpenGLMatrix(trpl);
            /* local_platform = (global_platform ^ -1) x (global_entity); */
            ent->character->local_platform = trpl.inverse() * ent->transform;
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
void Character_GetHeightInfo(const btVector3& pos, struct HeightInfo *fc, btScalar v_offset)
{
    btVector3 from, to;
    auto cb = fc->cb;
    std::shared_ptr<Room> r = (cb->m_container)?(cb->m_container->room):(NULL);
    RoomSector* rs;

    fc->floor_hit = 0x00;
    fc->ceiling_hit = 0x00;
    fc->water = 0x00;
    fc->quicksand = 0x00;
    fc->transition_level = 32512.0;

    r = Room_FindPosCogerrence(pos, r);
    r = r->checkFlip();
    if(r)
    {
        rs = r->getSectorXYZ(pos);                                         // if r != NULL then rs can not been NULL!!!
        if(r->flags & TR_ROOM_FLAG_WATER)                                       // in water - go up
        {
            while(rs->sector_above)
            {
                rs = rs->sector_above->checkFlip();
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
                rs = rs->sector_above->checkFlip();
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
                rs = rs->sector_below->checkFlip();
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
    auto base_pos = pos;
    from = pos;
    to = from;
    to[2] -= 4096.0;
    cb->m_closestHitFraction = 1.0;
    cb->m_collisionObject = NULL;
    bt_engine_dynamicsWorld->rayTest(from, to, *cb);
    fc->floor_hit = (int)cb->hasHit();
    if(fc->floor_hit)
    {
        fc->floor_normale = cb->m_hitNormalWorld;
        fc->floor_point.setInterpolate3(from, to, cb->m_closestHitFraction);
        fc->floor_obj = (btCollisionObject*)cb->m_collisionObject;
    }

    to = from;
    to[2] += 4096.0;
    cb->m_closestHitFraction = 1.0;
    cb->m_collisionObject = NULL;
    //cb->m_flags = btTriangleRaycastCallback::kF_FilterBackfaces;
    bt_engine_dynamicsWorld->rayTest(from, to, *cb);
    fc->ceiling_hit = (int)cb->hasHit();
    if(fc->ceiling_hit)
    {
        fc->ceiling_normale = cb->m_hitNormalWorld;
        fc->ceiling_point.setInterpolate3(from, to, cb->m_closestHitFraction);
        fc->ceiling_obj = (btCollisionObject*)cb->m_collisionObject;
    }
}

/**
 * @function calculates next floor info + fantom filter + returns step info.
 * Current height info must be calculated!
 */
int Character_CheckNextStep(std::shared_ptr<Entity> ent, const btVector3& offset, struct HeightInfo *nfc)
{
    btScalar delta;
    HeightInfo* fc = &ent->m_character->m_heightInfo;
    btVector3 from, to;
    int ret = CHARACTER_STEP_HORIZONTAL;
    ///penetration test?

    auto pos = ent->m_transform.getOrigin() + offset;
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
            if(delta <= ent->m_character->m_minStepUpHeight)
            {
                ret = CHARACTER_STEP_DOWN_LITTLE;
            }
            else if(delta <= ent->m_character->m_maxStepUpHeight)
            {
                ret = CHARACTER_STEP_DOWN_BIG;
            }
            else if(delta <= ent->m_character->m_height)
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
            if(delta <= ent->m_character->m_minStepUpHeight)
            {
                ret = CHARACTER_STEP_UP_LITTLE;
            }
            else if(delta <= ent->m_character->m_maxStepUpHeight)
            {
                ret = CHARACTER_STEP_UP_BIG;
            }
            else if(delta <= ent->m_character->m_maxClimbHeight)
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
    from[2] += ent->m_character->m_climbR;
    to[2] = from[2];
    from[0] = ent->m_transform.getOrigin()[0];
    from[1] = ent->m_transform.getOrigin()[1];
    to[0] = pos[0];
    to[1] = pos[1];
    fc->cb->m_closestHitFraction = 1.0;
    fc->cb->m_collisionObject = NULL;
    bt_engine_dynamicsWorld->rayTest(from, to, *fc->cb);
    if(fc->cb->hasHit())
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
int Character_HasStopSlant(std::shared_ptr<Entity> ent, HeightInfo *next_fc)
{
    const auto& pos = ent->m_transform.getOrigin();
    const auto& v1 = ent->m_transform.getBasis()[1];
    const auto& v2 = next_fc->floor_normale;
    return (next_fc->floor_point[2] > pos[2]) && (next_fc->floor_normale[2] < ent->m_character->m_criticalSlantZComponent) &&
           (v1[0] * v2[0] + v1[1] * v2[2] < 0.0);
}

/**
 * @FIXME: MAGICK CONST!
 * @param ent - entity
 * @param offset - offset, when we check height
 * @param nfc - height info (floor / ceiling)
 */
ClimbInfo Character_CheckClimbability(std::shared_ptr<Entity> ent, btVector3 offset, struct HeightInfo *nfc, btScalar test_height)
{
    btVector3 from, to;
    btScalar d;
    const auto& pos = ent->m_transform.getOrigin();
    btTransform t1, t2;
    char up_founded;
    extern GLfloat cast_ray[6];                                                 // pointer to the test line coordinates
    /*
     * init callbacks functions
     */
    nfc->cb = ent->m_character->m_rayCb;
    nfc->ccb = ent->m_character->m_convexCb;
    auto tmp = pos + offset;                                        // tmp = native offset point
    offset[2] += 128.0;                                                         ///@FIXME: stick for big slant

    ClimbInfo ret;
    ret.height_info = Character_CheckNextStep(ent, offset, nfc);
    offset[2] -= 128.0;
    ret.can_hang = 0;
    ret.edge_hit = 0x00;
    ret.edge_obj = NULL;
    ret.floor_limit = (ent->m_character->m_heightInfo.floor_hit)?(ent->m_character->m_heightInfo.floor_point[2]):(-9E10);
    ret.ceiling_limit = (ent->m_character->m_heightInfo.ceiling_hit)?(ent->m_character->m_heightInfo.ceiling_point[2]):(9E10);
    if(nfc->ceiling_hit && (nfc->ceiling_point[2] < ret.ceiling_limit))
    {
        ret.ceiling_limit = nfc->ceiling_point[2];
    }
    ret.point = ent->m_character->m_climb.point;
    /*
     * check max height
     */
    if(ent->m_character->m_heightInfo.ceiling_hit && (tmp[2] > ent->m_character->m_heightInfo.ceiling_point[2] - ent->m_character->m_climbR - 1.0))
    {
        tmp[2] = ent->m_character->m_heightInfo.ceiling_point[2] - ent->m_character->m_climbR - 1.0;
    }

    /*
    * Let us calculate EDGE
    */
    from[0] = pos[0] - ent->m_transform.getBasis()[1][0] * ent->m_character->m_climbR * 2.0;
    from[1] = pos[1] - ent->m_transform.getBasis()[1][1] * ent->m_character->m_climbR * 2.0;
    from[2] = tmp[2];
    to = tmp;

    //vec3_copy(cast_ray, from);
    //vec3_copy(cast_ray+3, to);

    t1.setIdentity();
    t2.setIdentity();
    up_founded = 0;
    test_height = (test_height >= ent->m_character->m_maxStepUpHeight)?(test_height):(ent->m_character->m_maxStepUpHeight);
    d = pos[2] + ent->m_bf.bb_max[2] - test_height;
    std::copy(to+0, to+3, cast_ray+0);
    std::copy(to+0, to+3, cast_ray+3);
    cast_ray[5] -= d;
    btVector3 n0, n1;
    do
    {
        t1.setOrigin(from);
        t2.setOrigin(to);
        nfc->ccb->m_closestHitFraction = 1.0;
        nfc->ccb->m_hitCollisionObject = NULL;
        bt_engine_dynamicsWorld->convexSweepTest(ent->m_character->m_climbSensor.get(), t1, t2, *nfc->ccb);
        if(nfc->ccb->hasHit())
        {
            if(nfc->ccb->m_hitNormalWorld[2] >= 0.1)
            {
                up_founded = 1;
                n0 = nfc->ccb->m_hitNormalWorld;
                n0[3] = -n0.dot(nfc->ccb->m_hitPointWorld);
            }
            if(up_founded && (nfc->ccb->m_hitNormalWorld[2] < 0.001))
            {
                n1 = nfc->ccb->m_hitNormalWorld;
                n1[3] = -n1.dot(nfc->ccb->m_hitPointWorld);
                ent->m_character->m_climb.edge_obj = (btCollisionObject*)nfc->ccb->m_hitCollisionObject;
                up_founded = 2;
                break;
            }
        }
        else
        {
            tmp[0] = to[0];
            tmp[1] = to[1];
            tmp[2] = d;
            t1.setOrigin(to);
            t2.setOrigin(tmp);
            //vec3_copy(cast_ray, to);
            //vec3_copy(cast_ray+3, tmp);
            nfc->ccb->m_closestHitFraction = 1.0;
            nfc->ccb->m_hitCollisionObject = NULL;
            bt_engine_dynamicsWorld->convexSweepTest(ent->m_character->m_climbSensor.get(), t1, t2, *nfc->ccb);
            if(nfc->ccb->hasHit())
            {
                up_founded = 1;
                auto n0 = nfc->ccb->m_hitNormalWorld;
                n0[3] = -n0.dot(nfc->ccb->m_hitPointWorld);
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
        from[2] -= 0.66 * ent->m_character->m_climbR;
        to[2] -= 0.66 * ent->m_character->m_climbR;
    }
    while(to[2] >= d);                                                 // we can't climb under floor!

    if(up_founded != 2)
    {
        return ret;
    }

    // get the character plane equation
    auto n2 = ent->m_transform.getBasis()[0];
    n2[3] = -n2.dot(pos);

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
    ret.point = ret.edge_point;
    std::copy(ret.point+0, ret.point+3, cast_ray+3);
    /*
     * unclimbable edge slant %)
     */
    n2 = n0.cross(n1);
    d = ent->m_character->m_criticalSlantZComponent;
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
    if(n2[0] * ent->m_transform.getBasis()[1][0] + n2[1] * ent->m_transform.getBasis()[1][1] > 0)       // direction fixing
    {
        n2[0] = -n2[0];
        n2[1] = -n2[1];
    }

    ret.n = n2;
    ret.up[0] = 0.0;
    ret.up[1] = 0.0;
    ret.up[2] = 1.0;
    ret.edge_z_ang = 180.0 * atan2f(n2[0], -n2[1]) / M_PI;
    ret.edge_tan_xy[0] = -n2[1];
    ret.edge_tan_xy[1] = n2[0];
    ret.edge_tan_xy[2] = 0.0;
    ret.edge_tan_xy /= btSqrt(n2[0] * n2[0] + n2[1] * n2[1]);
    ret.t = ret.edge_tan_xy;

    if(!ent->m_character->m_heightInfo.floor_hit || (ret.edge_point[2] - ent->m_character->m_heightInfo.floor_point[2] >= ent->m_character->m_height))
    {
        ret.can_hang = 1;
    }

    ret.next_z_space = 2.0 * ent->m_character->m_height;
    if(nfc->floor_hit && nfc->ceiling_hit)
    {
        ret.next_z_space = nfc->ceiling_point[2] - nfc->floor_point[2];
    }

    return ret;
}


ClimbInfo Character_CheckWallsClimbability(std::shared_ptr<Entity> ent)
{
    btVector3 from, to;
    btTransform tr1, tr2;
    btScalar wn2[2], t, *pos = ent->m_transform.getOrigin();
    auto ccb = ent->m_character->m_convexCb;

    ClimbInfo ret;
    ret.can_hang = 0x00;
    ret.wall_hit = 0x00;
    ret.edge_hit = 0x00;
    ret.edge_obj = NULL;
    ret.floor_limit = (ent->m_character->m_heightInfo.floor_hit)?(ent->m_character->m_heightInfo.floor_point[2]):(-9E10);
    ret.ceiling_limit = (ent->m_character->m_heightInfo.ceiling_hit)?(ent->m_character->m_heightInfo.ceiling_point[2]):(9E10);
    ret.point = ent->m_character->m_climb.point;

    if(ent->m_character->m_heightInfo.walls_climb == 0x00)
    {
        return ret;
    }

    ret.up[0] = 0.0;
    ret.up[1] = 0.0;
    ret.up[2] = 1.0;

    from[0] = pos[0] + ent->m_transform.getBasis()[2][0] * ent->m_bf.bb_max[2] - ent->m_transform.getBasis()[1][0] * ent->m_character->m_climbR;
    from[1] = pos[1] + ent->m_transform.getBasis()[2][1] * ent->m_bf.bb_max[2] - ent->m_transform.getBasis()[1][1] * ent->m_character->m_climbR;
    from[2] = pos[2] + ent->m_transform.getBasis()[2][2] * ent->m_bf.bb_max[2] - ent->m_transform.getBasis()[1][2] * ent->m_character->m_climbR;
    to = from;
    t = ent->m_character->m_forvardSize + ent->m_bf.bb_max[1];
    to[0] += ent->m_transform.getBasis()[1][0] * t;
    to[1] += ent->m_transform.getBasis()[1][1] * t;
    to[2] += ent->m_transform.getBasis()[1][2] * t;

    ccb->m_closestHitFraction = 1.0;
    ccb->m_hitCollisionObject = NULL;
    tr1.setIdentity();
    tr1.setOrigin(from);
    tr2.setIdentity();
    tr2.setOrigin(to);
    bt_engine_dynamicsWorld->convexSweepTest(ent->m_character->m_climbSensor.get(), tr1, tr2, *ccb);
    if(!(ccb->hasHit()))
    {
        return ret;
    }

    ret.point = ccb->m_hitPointWorld;
    ret.n = ccb->m_hitNormalWorld;
    wn2[0] = ret.n[0];
    wn2[1] = ret.n[1];
    t = sqrt(wn2[0] * wn2[0] + wn2[1] * wn2[1]);
    wn2[0] /= t;
    wn2[0] /= t;

    ret.t[0] =-wn2[1];
    ret.t[1] = wn2[0];
    ret.t[2] = 0.0;
    // now we have wall normale in XOY plane. Let us check all flags

    if((ent->m_character->m_heightInfo.walls_climb_dir & SECTOR_FLAG_CLIMB_NORTH) && (wn2[1] < -0.7))
    {
        ret.wall_hit = 0x01;                                                    // nW = (0, -1, 0);
    }
    if((ent->m_character->m_heightInfo.walls_climb_dir & SECTOR_FLAG_CLIMB_EAST) && (wn2[0] < -0.7))
    {
        ret.wall_hit = 0x01;                                                    // nW = (-1, 0, 0);
    }
    if((ent->m_character->m_heightInfo.walls_climb_dir & SECTOR_FLAG_CLIMB_SOUTH) && (wn2[1] > 0.7))
    {
        ret.wall_hit = 0x01;                                                    // nW = (0, 1, 0);
    }
    if((ent->m_character->m_heightInfo.walls_climb_dir & SECTOR_FLAG_CLIMB_WEST) && (wn2[0] > 0.7))
    {
        ret.wall_hit = 0x01;                                                    // nW = (1, 0, 0);
    }

    if(ret.wall_hit)
    {
        t = 0.67 * ent->m_character->m_height;
        from[0] -= ent->m_transform.getBasis()[2][0] * t;
        from[1] -= ent->m_transform.getBasis()[2][1] * t;
        from[2] -= ent->m_transform.getBasis()[2][2] * t;
        to = from;
        t = ent->m_character->m_forvardSize + ent->m_bf.bb_max[1];
        to[0] += ent->m_transform.getBasis()[1][0] * t;
        to[1] += ent->m_transform.getBasis()[1][1] * t;
        to[2] += ent->m_transform.getBasis()[1][2] * t;

        ccb->m_closestHitFraction = 1.0;
        ccb->m_hitCollisionObject = NULL;
        tr1.setIdentity();
        tr1.setOrigin(from);
        tr2.setIdentity();
        tr2.setOrigin(to);
        bt_engine_dynamicsWorld->convexSweepTest(ent->m_character->m_climbSensor.get(), tr1, tr2, *ccb);
        if(ccb->hasHit())
        {
            ret.wall_hit = 0x02;
        }
    }

    // now check ceiling limit (and floor too... may be later)
    /*vec3_add(from, point, ent->transform.getBasis()[1]);
    to = from;
    from[2] += 520.0;                                                  ///@FIXME: magick;
    to[2] -= 520.0;                                                    ///@FIXME: magick... again...
    cb->m_closestHitFraction = 1.0;
    cb->m_collisionObject = NULL;
    bt_engine_dynamicsWorld->rayTest(from, to, *cb);
    if(cb->hasHit())
    {
        point.setInterpolate3(from, to, cb->m_closestHitFraction);
        ret.ceiling_limit = (ret.ceiling_limit > point[2])?(point[2]):(ret.ceiling_limit);
    }*/

    return ret;
}


void Character_SetToJump(std::shared_ptr<Entity> ent, btScalar v_vertical, btScalar v_horizontal)
{
    btScalar t;
    btVector3 spd(0.0, 0.0, 0.0);

    if(!ent->m_character)
    {
        return;
    }

    // Jump length is a speed value multiplied by global speed coefficient.
    t = v_horizontal * ent->m_speedMult;

    // Calculate the direction of jump by vector multiplication.
    if(ent->m_dirFlag & ENT_MOVE_FORWARD)
    {
        spd = ent->m_transform.getBasis()[1] * t;
    }
    else if(ent->m_dirFlag & ENT_MOVE_BACKWARD)
    {
        spd = ent->m_transform.getBasis()[1] * -t;
    }
    else if(ent->m_dirFlag & ENT_MOVE_LEFT)
    {
        spd = ent->m_transform.getBasis()[0] * -t;
    }
    else if(ent->m_dirFlag & ENT_MOVE_RIGHT)
    {
        spd = ent->m_transform.getBasis()[0] * t;
    }
    else
    {
        ent->m_dirFlag = ENT_MOVE_FORWARD;
    }

    ent->m_character->m_response.vertical_collide = 0x00;
    ent->m_character->m_response.slide = 0x00;

    // Jump speed should NOT be added to current speed, as native engine
    // fully replaces current speed with jump speed by anim command.
    ent->m_speed = spd;

    // Apply vertical speed.
    ent->m_speed[2] = v_vertical * ent->m_speedMult;
    ent->m_moveType = MOVE_FREE_FALLING;
}


void Character_Lean(std::shared_ptr<Entity> ent, CharacterCommand *cmd, btScalar max_lean)
{
    btScalar neg_lean   = 360.0 - max_lean;
    btScalar lean_coeff = (max_lean == 0.0)?(48.0):(max_lean * 3);

    // Continously lean character, according to current left/right direction.

    if((cmd->move[1] == 0) || (max_lean == 0.0))       // No direction - restore straight vertical position!
    {
        if(ent->m_angles[2] != 0.0)
        {
            if(ent->m_angles[2] < 180.0)
            {
                ent->m_angles[2] -= ((abs(ent->m_angles[2]) + lean_coeff) / 2) * engine_frame_time;
                if(ent->m_angles[2] < 0.0) ent->m_angles[2] = 0.0;
            }
            else
            {
                ent->m_angles[2] += ((360 - abs(ent->m_angles[2]) + lean_coeff) / 2) * engine_frame_time;
                if(ent->m_angles[2] < 180.0) ent->m_angles[2] = 0.0;
            }
        }
    }
    else if(cmd->move[1] == 1) // Right direction
    {
        if(ent->m_angles[2] != max_lean)
        {
            if(ent->m_angles[2] < max_lean)   // Approaching from center
            {
                ent->m_angles[2] += ((abs(ent->m_angles[2]) + lean_coeff) / 2) * engine_frame_time;
                if(ent->m_angles[2] > max_lean)
                    ent->m_angles[2] = max_lean;
            }
            else if(ent->m_angles[2] > 180.0) // Approaching from left
            {
                ent->m_angles[2] += ((360.0 - abs(ent->m_angles[2]) + (lean_coeff*2) / 2) * engine_frame_time);
                if(ent->m_angles[2] < 180.0) ent->m_angles[2] = 0.0;
            }
            else    // Reduce previous lean
            {
                ent->m_angles[2] -= ((abs(ent->m_angles[2]) + lean_coeff) / 2) * engine_frame_time;
                if(ent->m_angles[2] < 0.0) ent->m_angles[2] = 0.0;
            }
        }
    }
    else if(cmd->move[1] == -1)     // Left direction
    {
        if(ent->m_angles[2] != neg_lean)
        {
            if(ent->m_angles[2] > neg_lean)   // Reduce previous lean
            {
                ent->m_angles[2] -= ((360.0 - abs(ent->m_angles[2]) + lean_coeff) / 2) * engine_frame_time;
                if(ent->m_angles[2] < neg_lean)
                    ent->m_angles[2] = neg_lean;
            }
            else if(ent->m_angles[2] < 180.0) // Approaching from right
            {
                ent->m_angles[2] -= ((abs(ent->m_angles[2]) + (lean_coeff*2)) / 2) * engine_frame_time;
                if(ent->m_angles[2] < 0.0) ent->m_angles[2] += 360.0;
            }
            else    // Approaching from center
            {
                ent->m_angles[2] += ((360.0 - abs(ent->m_angles[2]) + lean_coeff) / 2) * engine_frame_time;
                if(ent->m_angles[2] > 360.0) ent->m_angles[2] -= 360.0;
            }
        }
    }
}


/*
 * Linear inertia is absolutely needed for in-water states, and also it gives
 * more organic feel to land animations.
 */
btScalar Character_InertiaLinear(std::shared_ptr<Entity> ent, btScalar max_speed, btScalar accel, int8_t command)
{
    if((!ent) || (!ent->m_character)) return 0.0;

    if((accel == 0.0) || (accel >= max_speed))
    {
        if(command)
        {
            ent->m_inertiaLinear = max_speed;
        }
        else
        {
            ent->m_inertiaLinear = 0.0;
        }
    }
    else
    {
        if(command)
        {
            if(ent->m_inertiaLinear < max_speed)
            {
                ent->m_inertiaLinear += max_speed * accel * engine_frame_time;
                if(ent->m_inertiaLinear > max_speed) ent->m_inertiaLinear = max_speed;
            }
        }
        else
        {
            if(ent->m_inertiaLinear > 0.0)
            {
                ent->m_inertiaLinear -= max_speed * accel * engine_frame_time;
                if(ent->m_inertiaLinear < 0.0) ent->m_inertiaLinear = 0.0;
            }
        }
    }

    return ent->m_inertiaLinear * ent->m_speedMult;
}

/*
 * Angular inertia is used on keyboard-driven (non-analog) rotational controls.
 */
btScalar Character_InertiaAngular(std::shared_ptr<Entity> ent, btScalar max_angle, btScalar accel, uint8_t axis)
{
    if((!ent) || (!ent->m_character) || (axis > 1)) return 0.0;

    uint8_t curr_rot_dir = 0;
    if     (ent->m_character->m_command.rot[axis] < 0.0) { curr_rot_dir = 1; }
    else if(ent->m_character->m_command.rot[axis] > 0.0) { curr_rot_dir = 2; }

    if((!curr_rot_dir) || (max_angle == 0.0) || (accel == 0.0))
    {
        ent->m_inertiaAngular[axis] = 0.0;
    }
    else
    {
        if(ent->m_inertiaAngular[axis] != max_angle)
        {
            if(curr_rot_dir == 2)
            {
                if(ent->m_inertiaAngular[axis] < 0.0)
                {
                    ent->m_inertiaAngular[axis] = 0.0;
                }
                else
                {
                    ent->m_inertiaAngular[axis] += max_angle * accel * engine_frame_time;
                    if(ent->m_inertiaAngular[axis] > max_angle) ent->m_inertiaAngular[axis] = max_angle;
                }
            }
            else
            {
                if(ent->m_inertiaAngular[axis] > 0.0)
                {
                    ent->m_inertiaAngular[axis] = 0.0;
                }
                else
                {
                    ent->m_inertiaAngular[axis] -= max_angle * accel * engine_frame_time;
                    if(ent->m_inertiaAngular[axis] < -max_angle) ent->m_inertiaAngular[axis] = -max_angle;
                }
            }
        }
    }

    return fabs(ent->m_inertiaAngular[axis]) * ent->m_character->m_command.rot[axis];
}

/*
 * MOVE IN DIFFERENCE CONDITIONS
 */
int Character_MoveOnFloor(std::shared_ptr<Entity> ent)
{
    btVector3 tv, norm_move_xy, move, spd(0.0, 0.0, 0.0);
    btScalar norm_move_xy_len, t, ang;
    auto& pos = ent->m_transform.getOrigin();

    if(!ent->m_character)
    {
        return 0;
    }

    /*
     * init height info structure
     */
    HeightInfo nfc;
    nfc.cb = ent->m_character->m_rayCb;
    nfc.ccb = ent->m_character->m_convexCb;
    ent->m_character->m_response.horizontal_collide = 0x00;
    ent->m_character->m_response.vertical_collide = 0x00;
    // First of all - get information about floor and ceiling!!!
    Character_UpdateCurrentHeight(ent);
    if(ent->m_character->m_heightInfo.floor_hit && (ent->m_character->m_heightInfo.floor_point[2] + 1.0 >= ent->m_transform.getOrigin()[2] + ent->m_bf.bb_min[2]))
    {
        EngineContainer* cont = (EngineContainer*)ent->m_character->m_heightInfo.floor_obj->getUserPointer();
        if((cont != NULL) && (cont->object_type == OBJECT_ENTITY))
        {
            std::shared_ptr<Entity> e = std::static_pointer_cast<Entity>(cont->object);
            if(e->m_callbackFlags & ENTITY_CALLBACK_STAND)
            {
                lua_ExecEntity(engine_lua, ENTITY_CALLBACK_STAND, e->m_id, ent->m_id);
            }
        }
    }

    /*
     * check move type
     */
    if(ent->m_character->m_heightInfo.floor_hit || (ent->m_character->m_response.vertical_collide & 0x01))
    {
        if(ent->m_character->m_heightInfo.floor_point[2] + ent->m_character->m_fallDownHeight < pos[2])
        {
            ent->m_moveType = MOVE_FREE_FALLING;
            ent->m_speed[2] = 0.0;
            return -1;                                                          // nothing to do here
        }
        else
        {
            ent->m_character->m_response.vertical_collide |= 0x01;
        }

        tv = ent->m_character->m_heightInfo.floor_normale;
        if(tv[2] > 0.02 && tv[2] < ent->m_character->m_criticalSlantZComponent)
        {
            tv[2] = -tv[2];
            spd = tv * ent->m_speedMult * DEFAULT_CHARACTER_SLIDE_SPEED_MULT; // slide down direction
            ang = 180.0 * atan2f(tv[0], -tv[1]) / M_PI;       // from -180 deg to +180 deg
            //ang = (ang < 0.0)?(ang + 360.0):(ang);
            t = tv[0] * ent->m_transform.getBasis()[0][1] + tv[1] * ent->m_transform.getBasis()[1][1];
            if(t >= 0.0)
            {
                ent->m_character->m_response.slide = CHARACTER_SLIDE_FRONT;
                ent->m_angles[0] = ang + 180.0;
                // front forward slide down
            }
            else
            {
                ent->m_character->m_response.slide = CHARACTER_SLIDE_BACK;
                ent->m_angles[0] = ang;
                // back forward slide down
            }
            ent->updateRotation();
            ent->m_character->m_response.vertical_collide |= 0x01;
        }
        else    // no slide - free to walk
        {
            t = ent->m_currentSpeed * ent->m_speedMult;
            ent->m_character->m_response.vertical_collide |= 0x01;

            ent->m_angles[0] += Character_InertiaAngular(ent, 1.0, ROT_SPEED_LAND, 0);

            ent->updateRotation(); // apply rotations

            if(ent->m_dirFlag & ENT_MOVE_FORWARD)
            {
                spd = ent->m_transform.getBasis()[1] * t;
            }
            else if(ent->m_dirFlag & ENT_MOVE_BACKWARD)
            {
                spd = ent->m_transform.getBasis()[1] * -t;
            }
            else if(ent->m_dirFlag & ENT_MOVE_LEFT)
            {
                spd = ent->m_transform.getBasis()[0] * -t;
            }
            else if(ent->m_dirFlag & ENT_MOVE_RIGHT)
            {
                spd = ent->m_transform.getBasis()[0] * t;
            }
            else
            {
                //ent->dir_flag = ENT_MOVE_FORWARD;
            }
            ent->m_character->m_response.slide = 0x00;
        }
    }
    else                                                                        // no hit to the floor
    {
        ent->m_character->m_response.slide = 0x00;
        ent->m_character->m_response.vertical_collide = 0x00;
        ent->m_moveType = MOVE_FREE_FALLING;
        ent->m_speed[2] = 0.0;
        return -1;                                                              // nothing to do here
    }

    /*
     * now move normally
     */
    ent->m_speed = spd;
    move = spd * engine_frame_time;
    t = move.length();

    norm_move_xy[0] = move[0];
    norm_move_xy[1] = move[1];
    norm_move_xy[2] = 0.0;
    norm_move_xy_len = norm_move_xy.length();
    if(norm_move_xy_len > 0.2 * t)
    {
        norm_move_xy /= norm_move_xy_len;
    }
    else
    {
        norm_move_xy_len = 32512.0;
        norm_move_xy.setZero();
    }

    ent->ghostUpdate();
    pos += move;
    ent->fixPenetrations(&move);
    if(ent->m_character->m_heightInfo.floor_hit)
    {
        if(ent->m_character->m_heightInfo.floor_point[2] + ent->m_character->m_fallDownHeight > pos[2])
        {
            btScalar dz_to_land = engine_frame_time * 2400.0;                   ///@FIXME: magick
            if(pos[2] > ent->m_character->m_heightInfo.floor_point[2] + dz_to_land)
            {
                pos[2] -= dz_to_land;
                ent->fixPenetrations(nullptr);
            }
            else if(pos[2] > ent->m_character->m_heightInfo.floor_point[2])
            {
                pos[2] = ent->m_character->m_heightInfo.floor_point[2];
                ent->fixPenetrations(nullptr);
            }
        }
        else
        {
            ent->m_moveType = MOVE_FREE_FALLING;
            ent->m_speed[2] = 0.0;
            ent->updateRoomPos();
            return 2;
        }
        if((pos[2] < ent->m_character->m_heightInfo.floor_point[2]) && !ent->m_bt.no_fix_all)
        {
            pos[2] = ent->m_character->m_heightInfo.floor_point[2];
            ent->fixPenetrations(nullptr);
            ent->m_character->m_response.vertical_collide |= 0x01;
        }
    }
    else if(!(ent->m_character->m_response.vertical_collide & 0x01))
    {
        ent->m_moveType = MOVE_FREE_FALLING;
        ent->m_speed[2] = 0.0;
        ent->updateRoomPos();
        return 2;
    }

    ent->updateRoomPos();

    return 1;
}


int Character_FreeFalling(std::shared_ptr<Entity> ent)
{
    btVector3 move;
    auto& pos = ent->m_transform.getOrigin();

    if(!ent->m_character)
    {
        return 0;
    }

    /*
     * init height info structure
     */

    ent->m_character->m_response.slide = 0x00;
    ent->m_character->m_response.horizontal_collide = 0x00;
    ent->m_character->m_response.vertical_collide = 0x00;

    btScalar rot = Character_InertiaAngular(ent, 1.0, ROT_SPEED_FREEFALL, 0);
    ent->m_angles[0] += rot;
    ent->m_angles[1] = 0.0;

    ent->updateRotation();                                                 // apply rotations

    /*btScalar t = ent->current_speed * bf-> ent->speed_mult;        ///@TODO: fix speed update in Entity_Frame function and other;
    if(ent->dir_flag & ENT_MOVE_FORWARD)
    {
        ent->speed[0] = ent->transform.getBasis()[1][0] * t;
        ent->speed[1] = ent->transform.getBasis()[1][1] * t;
    }
    else if(ent->dir_flag & ENT_MOVE_BACKWARD)
    {
        ent->speed[0] =-ent->transform.getBasis()[1][0] * t;
        ent->speed[1] =-ent->transform.getBasis()[1][1] * t;
    }
    else if(ent->dir_flag & ENT_MOVE_LEFT)
    {
        ent->speed[0] =-ent->transform.getBasis()[0 + 0] * t;
        ent->speed[1] =-ent->transform.getBasis()[0 + 1] * t;
    }
    else if(ent->dir_flag & ENT_MOVE_RIGHT)
    {
        ent->speed[0] = ent->transform.getBasis()[0 + 0] * t;
        ent->speed[1] = ent->transform.getBasis()[0 + 1] * t;
    }*/

    move = ent->m_speed + bt_engine_dynamicsWorld->getGravity() * engine_frame_time * 0.5;
    move *= engine_frame_time;
    ent->m_speed += bt_engine_dynamicsWorld->getGravity() * engine_frame_time;
    ent->m_speed[2] = (ent->m_speed[2] < -FREE_FALL_SPEED_MAXIMUM)?(-FREE_FALL_SPEED_MAXIMUM):(ent->m_speed[2]);
    ent->m_speed = ent->m_speed.rotate({0,0,1}, rot * M_PI/180);

    Character_UpdateCurrentHeight(ent);

    if(ent->m_self->room && (ent->m_self->room->flags & TR_ROOM_FLAG_WATER))
    {
        if(ent->m_speed[2] < 0.0)
        {
            ent->m_currentSpeed = 0.0;
            ent->m_speed[0] = 0.0;
            ent->m_speed[1] = 0.0;
        }

        if((engine_world.version < TR_II))//Lara cannot wade in < TRII so when floor < transition level she has to swim
        {
            if(!ent->m_character->m_heightInfo.water || (ent->m_currentSector->floor <= ent->m_character->m_heightInfo.transition_level))
            {
                ent->m_moveType = MOVE_UNDERWATER;
                return 2;
            }
        }
        else
        {
            if(!ent->m_character->m_heightInfo.water || (ent->m_currentSector->floor + ent->m_character->m_height <= ent->m_character->m_heightInfo.transition_level))
            {
                ent->m_moveType = MOVE_UNDERWATER;
                return 2;
            }
        }
    }

    ent->ghostUpdate();
    if(ent->m_character->m_heightInfo.ceiling_hit && ent->m_speed[2] > 0.0)
    {
        if(ent->m_character->m_heightInfo.ceiling_point[2] < ent->m_bf.bb_max[2] + pos[2])
        {
            pos[2] = ent->m_character->m_heightInfo.ceiling_point[2] - ent->m_bf.bb_max[2];
            ent->m_speed[2] = 0.0;
            ent->m_character->m_response.vertical_collide |= 0x02;
            ent->fixPenetrations(nullptr);
            ent->updateRoomPos();
        }
    }
    if(ent->m_character->m_heightInfo.floor_hit && ent->m_speed[2] < 0.0)   // move down
    {
        if(ent->m_character->m_heightInfo.floor_point[2] >= pos[2] + ent->m_bf.bb_min[2] + move[2])
        {
            pos[2] = ent->m_character->m_heightInfo.floor_point[2];
            //ent->speed[2] = 0.0;
            ent->m_moveType = MOVE_ON_FLOOR;
            ent->m_character->m_response.vertical_collide |= 0x01;
            ent->fixPenetrations(nullptr);
            ent->updateRoomPos();
            return 2;
        }
    }

    pos += move;
    ent->fixPenetrations(&move);                           // get horizontal collide

    if(ent->m_character->m_heightInfo.ceiling_hit && ent->m_speed[2] > 0.0)
    {
        if(ent->m_character->m_heightInfo.ceiling_point[2] < ent->m_bf.bb_max[2] + pos[2])
        {
            pos[2] = ent->m_character->m_heightInfo.ceiling_point[2] - ent->m_bf.bb_max[2];
            ent->m_speed[2] = 0.0;
            ent->m_character->m_response.vertical_collide |= 0x02;
        }
    }
    if(ent->m_character->m_heightInfo.floor_hit && ent->m_speed[2] < 0.0)   // move down
    {
        if(ent->m_character->m_heightInfo.floor_point[2] >= pos[2] + ent->m_bf.bb_min[2] + move[2])
        {
            pos[2] = ent->m_character->m_heightInfo.floor_point[2];
            //ent->speed[2] = 0.0;
            ent->m_moveType = MOVE_ON_FLOOR;
            ent->m_character->m_response.vertical_collide |= 0x01;
            ent->fixPenetrations(nullptr);
            ent->updateRoomPos();
            return 2;
        }
    }
    ent->updateRoomPos();

    return 1;
}

/*
 * Monkey CLIMBING - MOVE NO Z LANDING
 */
int Character_MonkeyClimbing(std::shared_ptr<Entity> ent)
{
    btVector3 move, spd(0.0, 0.0, 0.0);
    btScalar t;
    auto& pos = ent->m_transform.getOrigin();

    ent->m_speed[2] = 0.0;
    ent->m_character->m_response.slide = 0x00;
    ent->m_character->m_response.horizontal_collide = 0x00;
    ent->m_character->m_response.vertical_collide = 0x00;

    t = ent->m_currentSpeed * ent->m_speedMult;
    ent->m_character->m_response.vertical_collide |= 0x01;

    ent->m_angles[0] += Character_InertiaAngular(ent, 1.0, ROT_SPEED_MONKEYSWING, 0);
    ent->m_angles[1] = 0.0;
    ent->m_angles[2] = 0.0;
    ent->updateRotation();                                                 // apply rotations

    if(ent->m_dirFlag & ENT_MOVE_FORWARD)
    {
        spd = ent->m_transform.getBasis()[1] * t;
    }
    else if(ent->m_dirFlag & ENT_MOVE_BACKWARD)
    {
        spd = ent->m_transform.getBasis()[1] * -t;
    }
    else if(ent->m_dirFlag & ENT_MOVE_LEFT)
    {
        spd = ent->m_transform.getBasis()[0] * -t;
    }
    else if(ent->m_dirFlag & ENT_MOVE_RIGHT)
    {
        spd = ent->m_transform.getBasis()[0] * t;
    }
    else
    {
        //ent->dir_flag = ENT_MOVE_FORWARD;
    }
    ent->m_character->m_response.slide = 0x00;

    ent->m_speed = spd;
    move = spd * engine_frame_time;
    move[2] = 0.0;

    ent->ghostUpdate();
    Character_UpdateCurrentHeight(ent);
    pos += move;
    ent->fixPenetrations(&move);                              // get horizontal collide
    ///@FIXME: rewrite conditions! or add fixer to update_entity_rigid_body func
    if(ent->m_character->m_heightInfo.ceiling_hit && (pos[2] + ent->m_bf.bb_max[2] - ent->m_character->m_heightInfo.ceiling_point[2] > - 0.33 * ent->m_character->m_minStepUpHeight))
    {
        pos[2] = ent->m_character->m_heightInfo.ceiling_point[2] - ent->m_bf.bb_max[2];
    }
    else
    {
        ent->m_moveType = MOVE_FREE_FALLING;
        ent->updateRoomPos();
        return 2;
    }

    ent->updateRoomPos();

    return 1;
}

/*
 * WALLS CLIMBING - MOVE IN ZT plane
 */
int Character_WallsClimbing(std::shared_ptr<Entity> ent)
{
    ClimbInfo* climb = &ent->m_character->m_climb;
    btVector3 spd, move;
    btScalar t;
    auto& pos = ent->m_transform.getOrigin();

    ent->m_character->m_response.slide = 0x00;
    ent->m_character->m_response.horizontal_collide = 0x00;
    ent->m_character->m_response.vertical_collide = 0x00;

    spd={0,0,0};
    *climb = Character_CheckWallsClimbability(ent);
    ent->m_character->m_climb = *climb;
    if(!(climb->wall_hit))
    {
        ent->m_character->m_heightInfo.walls_climb = 0x00;
        return 2;
    }

    ent->m_angles[0] = 180.0 * atan2f(climb->n[0], -climb->n[1]) / M_PI;
    ent->updateRotation();
    pos[0] = climb->point[0] - ent->m_transform.getBasis()[1][0] * ent->m_bf.bb_max[1];
    pos[1] = climb->point[1] - ent->m_transform.getBasis()[1][1] * ent->m_bf.bb_max[1];

    if(ent->m_dirFlag == ENT_MOVE_FORWARD)
    {
        spd += climb->up;
    }
    else if(ent->m_dirFlag == ENT_MOVE_BACKWARD)
    {
        spd -= climb->up;
    }
    else if(ent->m_dirFlag == ENT_MOVE_RIGHT)
    {
        spd += climb->t;
    }
    else if(ent->m_dirFlag == ENT_MOVE_LEFT)
    {
        spd -= climb->t;
    }
    t = spd.length();
    if(t > 0.01)
    {
        spd /= t;
    }
    ent->m_speed = spd * ent->m_currentSpeed * ent->m_speedMult;
    move = ent->m_speed * engine_frame_time;

    ent->ghostUpdate();
    Character_UpdateCurrentHeight(ent);
    pos += move;
    ent->fixPenetrations(&move); // get horizontal collide
    ent->updateRoomPos();

    *climb = Character_CheckWallsClimbability(ent);
    if(pos[2] + ent->m_bf.bb_max[2] > climb->ceiling_limit)
    {
        pos[2] = climb->ceiling_limit - ent->m_bf.bb_max[2];
    }

    return 1;
}

/*
 * CLIMBING - MOVE NO Z LANDING
 */
int Character_Climbing(std::shared_ptr<Entity> ent)
{
    btVector3 move, spd(0.0, 0.0, 0.0);
    btScalar t;
    auto& pos = ent->m_transform.getOrigin();
    btScalar z = pos[2];

    ent->m_character->m_response.slide = 0x00;
    ent->m_character->m_response.horizontal_collide = 0x00;
    ent->m_character->m_response.vertical_collide = 0x00;

    t = ent->m_currentSpeed * ent->m_speedMult;
    ent->m_character->m_response.vertical_collide |= 0x01;
    ent->m_angles[0] += ent->m_character->m_command.rot[0];
    ent->m_angles[1] = 0.0;
    ent->m_angles[2] = 0.0;
    ent->updateRotation();                                                 // apply rotations

    if(ent->m_dirFlag == ENT_MOVE_FORWARD)
    {
        spd = ent->m_transform.getBasis()[1] * t;
    }
    else if(ent->m_dirFlag == ENT_MOVE_BACKWARD)
    {
        spd = ent->m_transform.getBasis()[1] * -t;
    }
    else if(ent->m_dirFlag == ENT_MOVE_LEFT)
    {
        spd = ent->m_transform.getBasis()[0] * -t;
    }
    else if(ent->m_dirFlag == ENT_MOVE_RIGHT)
    {
        spd = ent->m_transform.getBasis()[0] * t;
    }
    else
    {
        ent->m_character->m_response.slide = 0x00;
        ent->ghostUpdate();
        ent->fixPenetrations(nullptr);
        return 1;
    }

    ent->m_character->m_response.slide = 0x00;
    ent->m_speed = spd;
    move = spd * engine_frame_time;

    ent->ghostUpdate();
    pos += move;
    ent->fixPenetrations(&move);                              // get horizontal collide
    ent->updateRoomPos();
    pos[2] = z;

    return 1;
}

/*
 * underwater and onwater swimming has a big trouble:
 * the speed and acceleration information is absent...
 * I add some sticks to make it work for testing.
 * I thought to make export anim information to LUA script...
 */
int Character_MoveUnderWater(std::shared_ptr<Entity> ent)
{
    btVector3 move, spd(0.0, 0.0, 0.0);
    auto& pos = ent->m_transform.getOrigin();

    // Check current place.

    if(ent->m_self->room && !(ent->m_self->room->flags & TR_ROOM_FLAG_WATER))
    {
        ent->m_moveType = MOVE_FREE_FALLING;
        return 2;
    }

    ent->m_character->m_response.slide = 0x00;
    ent->m_character->m_response.horizontal_collide = 0x00;
    ent->m_character->m_response.vertical_collide = 0x00;

    // Calculate current speed.

    btScalar t = Character_InertiaLinear(ent, MAX_SPEED_UNDERWATER, INERTIA_SPEED_UNDERWATER, ent->m_character->m_command.jump);

    if(!ent->m_character->m_response.kill)   // Block controls if Lara is dead.
    {
        ent->m_angles[0] += Character_InertiaAngular(ent, 1.0, ROT_SPEED_UNDERWATER, 0);
        ent->m_angles[1] -= Character_InertiaAngular(ent, 1.0, ROT_SPEED_UNDERWATER, 1);
        ent->m_angles[2]  = 0.0;

        if((ent->m_angles[1] > 70.0) && (ent->m_angles[1] < 180.0))                 // Underwater angle limiter.
        {
           ent->m_angles[1] = 70.0;
        }
        else if((ent->m_angles[1] > 180.0) && (ent->m_angles[1] < 270.0))
        {
            ent->m_angles[1] = 270.0;
        }

        ent->updateRotation();                                             // apply rotations

        spd = ent->m_transform.getBasis()[1] * t;                     // OY move only!
        ent->m_speed = spd;
    }

    move = spd * engine_frame_time;

    ent->ghostUpdate();
    pos += move;
    ent->fixPenetrations(&move);                              // get horizontal collide

    ent->updateRoomPos();
    if(ent->m_character->m_heightInfo.water && (pos[2] + ent->m_bf.bb_max[2] >= ent->m_character->m_heightInfo.transition_level))
    {
        if(/*(spd[2] > 0.0)*/ent->m_transform.getBasis()[1][2] > 0.67)             ///@FIXME: magick!
        {
            ent->m_moveType = MOVE_ON_WATER;
            //pos[2] = fc.transition_level;
            return 2;
        }
        if(!ent->m_character->m_heightInfo.floor_hit || (ent->m_character->m_heightInfo.transition_level - ent->m_character->m_heightInfo.floor_point[2] >= ent->m_character->m_height))
        {
            pos[2] = ent->m_character->m_heightInfo.transition_level - ent->m_bf.bb_max[2];
        }
    }

    return 1;
}


int Character_MoveOnWater(std::shared_ptr<Entity> ent)
{
    btVector3 move, spd(0.0, 0.0, 0.0);
    auto& pos = ent->m_transform.getOrigin();

    ent->m_character->m_response.slide = 0x00;
    ent->m_character->m_response.horizontal_collide = 0x00;
    ent->m_character->m_response.vertical_collide = 0x00;

    ent->m_angles[0] += Character_InertiaAngular(ent, 1.0, ROT_SPEED_ONWATER, 0);
    ent->m_angles[1] = 0.0;
    ent->m_angles[2] = 0.0;
    ent->updateRotation();     // apply rotations

    // Calculate current speed.

    btScalar t = Character_InertiaLinear(ent, MAX_SPEED_ONWATER, INERTIA_SPEED_ONWATER, ((abs(ent->m_character->m_command.move[0])) | (abs(ent->m_character->m_command.move[1]))));

    if((ent->m_dirFlag & ENT_MOVE_FORWARD) && (ent->m_character->m_command.move[0] == 1))
    {
        spd = ent->m_transform.getBasis()[1] * t;
    }
    else if((ent->m_dirFlag & ENT_MOVE_BACKWARD) && (ent->m_character->m_command.move[0] == -1))
    {
        spd = ent->m_transform.getBasis()[1] * -t;
    }
    else if((ent->m_dirFlag & ENT_MOVE_LEFT) && (ent->m_character->m_command.move[1] == -1))
    {
        spd = ent->m_transform.getBasis()[0] * -t;
    }
    else if((ent->m_dirFlag & ENT_MOVE_RIGHT) && (ent->m_character->m_command.move[1] == 1))
    {
        spd = ent->m_transform.getBasis()[0] * t;
    }
    else
    {
        ent->ghostUpdate();
        ent->fixPenetrations(nullptr);
        ent->updateRoomPos();
        if(ent->m_character->m_heightInfo.water)
        {
            pos[2] = ent->m_character->m_heightInfo.transition_level;
        }
        else
        {
            ent->m_moveType = MOVE_ON_FLOOR;
            return 2;
        }
        return 1;
    }

    /*
     * Prepare to moving
     */
    ent->m_speed = spd;
    move = spd * engine_frame_time;
    ent->ghostUpdate();
    pos += move;
    ent->fixPenetrations(&move);  // get horizontal collide

    ent->updateRoomPos();
    if(ent->m_character->m_heightInfo.water)
    {
        pos[2] = ent->m_character->m_heightInfo.transition_level;
    }
    else
    {
        ent->m_moveType = MOVE_ON_FLOOR;
        return 2;
    }

    return 1;
}

int Character_FindTraverse(std::shared_ptr<Entity> ch)
{
    RoomSector* ch_s, *obj_s = NULL;
    ch_s = ch->m_self->room->getSectorRaw(ch->m_transform.getOrigin());

    if(ch_s == NULL)
    {
        return 0;
    }

    ch->m_character->m_traversedObject = NULL;

    // OX move case
    if(ch->m_transform.getBasis()[1][0] > 0.9)
    {
        obj_s = ch_s->owner_room->getSectorRaw({(btScalar)(ch_s->pos[0] + TR_METERING_SECTORSIZE), (btScalar)(ch_s->pos[1]), (btScalar)0.0});
    }
    else if(ch->m_transform.getBasis()[1][0] < -0.9)
    {
        obj_s = ch_s->owner_room->getSectorRaw({(btScalar)(ch_s->pos[0] - TR_METERING_SECTORSIZE), (btScalar)(ch_s->pos[1]), (btScalar)0.0});
    }
    // OY move case
    else if(ch->m_transform.getBasis()[1][1] > 0.9)
    {
        obj_s = ch_s->owner_room->getSectorRaw({(btScalar)(ch_s->pos[0]), (btScalar)(ch_s->pos[1] + TR_METERING_SECTORSIZE), (btScalar)0.0});
    }
    else if(ch->m_transform.getBasis()[1][1] < -0.9)
    {
        obj_s = ch_s->owner_room->getSectorRaw({(btScalar)(ch_s->pos[0]), (btScalar)(ch_s->pos[1] - TR_METERING_SECTORSIZE), (btScalar)0.0});
    }

    if(obj_s != NULL)
    {
        obj_s = obj_s->checkPortalPointer();
        for(std::shared_ptr<EngineContainer>& cont : obj_s->owner_room->containers)
        {
            if(cont->object_type == OBJECT_ENTITY)
            {
                std::shared_ptr<Entity> e = std::static_pointer_cast<Entity>(cont->object);
                if((e->m_typeFlags & ENTITY_TYPE_TRAVERSE) && (1 == OBB_OBB_Test(e, ch) && (fabs(e->m_transform.getOrigin()[2] - ch->m_transform.getOrigin()[2]) < 1.1)))
                {
                    int oz = (ch->m_angles[0] + 45.0) / 90.0;
                    ch->m_angles[0] = oz * 90.0;
                    ch->m_character->m_traversedObject = e;
                    ch->updateRotation();
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
int Sector_AllowTraverse(struct RoomSector *rs, btScalar floor, const std::shared_ptr<EngineContainer>& cont)
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

    BtEngineClosestRayResultCallback cb(cont.get());
    btVector3 from, to;
    to[0] = from[0] = rs->pos[0];
    to[1] = from[1] = rs->pos[1];
    from[2] = floor + TR_METERING_SECTORSIZE * 0.5;
    to[2] = floor - TR_METERING_SECTORSIZE * 0.5;
    bt_engine_dynamicsWorld->rayTest(from, to, cb);
    if(cb.hasHit())
    {
        btVector3 v;
        v.setInterpolate3(from, to, cb.m_closestHitFraction);
        if(fabs(v[2] - floor) < 1.1)
        {
            EngineContainer* cont = (EngineContainer*)cb.m_collisionObject->getUserPointer();
            if((cont != NULL) && (cont->object_type == OBJECT_ENTITY) && ((std::static_pointer_cast<Entity>(cont->object))->m_typeFlags & ENTITY_TYPE_TRAVERSE_FLOOR))
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
int Character_CheckTraverse(std::shared_ptr<Entity> ch, std::shared_ptr<Entity> obj)
{
    RoomSector* ch_s, *obj_s;

    ch_s = ch->m_self->room->getSectorRaw(ch->m_transform.getOrigin());
    obj_s = obj->m_self->room->getSectorRaw(obj->m_transform.getOrigin());

    if(obj_s == ch_s)
    {
        if(ch->m_transform.getBasis()[1][0] > 0.8)
        {
            ch_s = obj_s->owner_room->getSectorRaw({(btScalar)(obj_s->pos[0] - TR_METERING_SECTORSIZE), (btScalar)(obj_s->pos[1]), (btScalar)0.0});
        }
        else if(ch->m_transform.getBasis()[1][0] < -0.8)
        {
            ch_s = obj_s->owner_room->getSectorRaw({(btScalar)(obj_s->pos[0] + TR_METERING_SECTORSIZE), (btScalar)(obj_s->pos[1]), (btScalar)0.0});
        }
        // OY move case
        else if(ch->m_transform.getBasis()[1][1] > 0.8)
        {
            ch_s = obj_s->owner_room->getSectorRaw({(btScalar)(obj_s->pos[0]), (btScalar)(obj_s->pos[1] - TR_METERING_SECTORSIZE), (btScalar)0.0});
        }
        else if(ch->m_transform.getBasis()[1][1] < -0.8)
        {
            ch_s = obj_s->owner_room->getSectorRaw({(btScalar)(obj_s->pos[0]), (btScalar)(obj_s->pos[1] + TR_METERING_SECTORSIZE), (btScalar)0.0});
        }
        ch_s = ch_s->checkPortalPointer();
    }

    if((ch_s == NULL) || (obj_s == NULL))
    {
        return 0x00;
    }

    btScalar floor = ch->m_transform.getOrigin()[2];
    if((ch_s->floor != obj_s->floor) || (Sector_AllowTraverse(ch_s, floor, ch->m_self) == 0x00) || (Sector_AllowTraverse(obj_s, floor, obj->m_self) == 0x00))
    {
        return 0x00;
    }

    BtEngineClosestRayResultCallback cb(obj->m_self.get());
    btVector3 v0, v1;
    v1[0] = v0[0] = obj_s->pos[0];
    v1[1] = v0[1] = obj_s->pos[1];
    v0[2] = floor + TR_METERING_SECTORSIZE * 0.5;
    v1[2] = floor + TR_METERING_SECTORSIZE * 2.5;
    bt_engine_dynamicsWorld->rayTest(v0, v1, cb);
    if(cb.hasHit())
    {
        EngineContainer* cont = (EngineContainer*)cb.m_collisionObject->getUserPointer();
        if((cont != NULL) && (cont->object_type == OBJECT_ENTITY) && ((std::static_pointer_cast<Entity>(cont->object))->m_typeFlags & ENTITY_TYPE_TRAVERSE))
        {
            return 0x00;
        }
    }

    int ret = 0x00;
    RoomSector* next_s = NULL;

    /*
     * PUSH MOVE CHECK
     */
    // OX move case
    if(ch->m_transform.getBasis()[1][0] > 0.8)
    {
        next_s = obj_s->owner_room->getSectorRaw({(btScalar)(obj_s->pos[0] + TR_METERING_SECTORSIZE), (btScalar)(obj_s->pos[1]), (btScalar)0.0});
    }
    else if(ch->m_transform.getBasis()[1][0] < -0.8)
    {
        next_s = obj_s->owner_room->getSectorRaw({(btScalar)(obj_s->pos[0] - TR_METERING_SECTORSIZE), (btScalar)(obj_s->pos[1]), (btScalar)0.0});
    }
    // OY move case
    else if(ch->m_transform.getBasis()[1][1] > 0.8)
    {
        next_s = obj_s->owner_room->getSectorRaw({(btScalar)(obj_s->pos[0]), (btScalar)(obj_s->pos[1] + TR_METERING_SECTORSIZE), (btScalar)0.0});
    }
    else if(ch->m_transform.getBasis()[1][1] < -0.8)
    {
        next_s = obj_s->owner_room->getSectorRaw({(btScalar)(obj_s->pos[0]), (btScalar)(obj_s->pos[1] - TR_METERING_SECTORSIZE), (btScalar)0.0});
    }

    next_s = next_s->checkPortalPointer();
    if((next_s != NULL) && (Sector_AllowTraverse(next_s, floor, ch->m_self) == 0x01))
    {
        BtEngineClosestConvexResultCallback ccb(obj->m_self.get());
        btSphereShape sp(0.48 * TR_METERING_SECTORSIZE);
        btVector3 v;
        btTransform from, to;
        v[0] = obj_s->pos[0];
        v[1] = obj_s->pos[1];
        v[2] = floor + 0.5 * TR_METERING_SECTORSIZE;
        from.setIdentity();
        from.setOrigin(v);
        v[0] = next_s->pos[0];
        v[1] = next_s->pos[1];
        to.setIdentity();
        to.setOrigin(v);
        bt_engine_dynamicsWorld->convexSweepTest(&sp, from, to, ccb);
        if(!ccb.hasHit())
        {
            ret |= 0x01;                                                        // can traverse forvard
        }
    }

    /*
     * PULL MOVE CHECK
     */
    next_s = NULL;
    // OX move case
    if(ch->m_transform.getBasis()[1][0] > 0.8)
    {
        next_s = obj_s->owner_room->getSectorRaw({(btScalar)(ch_s->pos[0] - TR_METERING_SECTORSIZE), (btScalar)(ch_s->pos[1]), (btScalar)0.0});
    }
    else if(ch->m_transform.getBasis()[1][0] < -0.8)
    {
        next_s = obj_s->owner_room->getSectorRaw({(btScalar)(ch_s->pos[0] + TR_METERING_SECTORSIZE), (btScalar)(ch_s->pos[1]), (btScalar)0.0});
    }
    // OY move case
    else if(ch->m_transform.getBasis()[1][1] > 0.8)
    {
        next_s = obj_s->owner_room->getSectorRaw({(btScalar)(ch_s->pos[0]), (btScalar)(ch_s->pos[1] - TR_METERING_SECTORSIZE), (btScalar)0.0});
    }
    else if(ch->m_transform.getBasis()[1][1] < -0.8)
    {
        next_s = obj_s->owner_room->getSectorRaw({(btScalar)(ch_s->pos[0]), (btScalar)(ch_s->pos[1] + TR_METERING_SECTORSIZE), (btScalar)0.0});
    }

    next_s = next_s->checkPortalPointer();
    if((next_s != NULL) && (Sector_AllowTraverse(next_s, floor, ch->m_self) == 0x01))
    {
        BtEngineClosestConvexResultCallback ccb(ch->m_self.get());
        btSphereShape sp(0.48 * TR_METERING_SECTORSIZE);
        btVector3 v;
        btTransform from, to;
        v[0] = ch_s->pos[0];
        v[1] = ch_s->pos[1];
        v[2] = floor + 0.5 * TR_METERING_SECTORSIZE;
        from.setIdentity();
        from.setOrigin(v);
        v[0] = next_s->pos[0];
        v[1] = next_s->pos[1];
        to.setIdentity();
        to.setOrigin(v);
        bt_engine_dynamicsWorld->convexSweepTest(&sp, from, to, ccb);
        if(!ccb.hasHit())
        {
            ret |= 0x02;                                                        // can traverse backvard
        }
    }

    return ret;
}

/**
 * Main character frame function
 */
void Character_ApplyCommands(std::shared_ptr<Entity> ent)
{
    if(ent->m_typeFlags & ENTITY_TYPE_DYNAMIC)
    {
        return;
    }

    Character_UpdatePlatformPreStep(ent);

    if(ent->m_character->state_func)
    {
        ent->m_character->state_func(ent, &ent->m_bf.animations);
    }

    switch(ent->m_moveType)
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
            ent->m_moveType = MOVE_ON_FLOOR;
            break;
    };

    ent->updateRigidBody(true);
    Character_UpdatePlatformPostStep(ent);
}

void Character_UpdateParams(std::shared_ptr<Entity> ent)
{
    switch(ent->m_moveType)
    {
        case MOVE_ON_FLOOR:
        case MOVE_FREE_FALLING:
        case MOVE_CLIMBING:
        case MOVE_MONKEYSWING:
        case MOVE_WALLS_CLIMB:

            if((ent->m_character->m_heightInfo.quicksand == 0x02) &&
               (ent->m_moveType == MOVE_ON_FLOOR))
            {
                if(!Character_ChangeParam(ent, PARAM_AIR, -3.0))
                    Character_ChangeParam(ent, PARAM_HEALTH, -3.0);
            }
            else if(ent->m_character->m_heightInfo.quicksand == 0x01)
            {
                Character_ChangeParam(ent, PARAM_AIR, 3.0);
            }
            else
            {
                Character_SetParam(ent, PARAM_AIR, PARAM_ABSOLUTE_MAX);
            }


            if((ent->m_bf.animations.last_state == TR_STATE_LARA_SPRINT) ||
               (ent->m_bf.animations.last_state == TR_STATE_LARA_SPRINT_ROLL))
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
                    ent->m_character->m_response.kill = 1;
                }
            }
            break;

        default:
            break;  // Add quicksand later...
    }
}

bool IsCharacter(std::shared_ptr<Entity> ent)
{
    return (ent != NULL) && (ent->m_character != NULL);
}

int Character_SetParamMaximum(std::shared_ptr<Entity> ent, int parameter, float max_value)
{
    if((!IsCharacter(ent)) || (parameter >= PARAM_SENTINEL))
        return 0;

    max_value = (max_value < 0)?(0):(max_value);    // Clamp max. to at least zero
    ent->m_character->m_parameters.maximum[parameter] = max_value;
    return 1;
}

int Character_SetParam(std::shared_ptr<Entity> ent, int parameter, float value)
{
    if((!IsCharacter(ent)) || (parameter >= PARAM_SENTINEL))
        return 0;

    float maximum = ent->m_character->m_parameters.maximum[parameter];

    value = (value >= 0)?(value):(maximum); // Char params can't be less than zero.
    value = (value <= maximum)?(value):(maximum);

    ent->m_character->m_parameters.param[parameter] = value;
    return 1;
}

float Character_GetParam(std::shared_ptr<Entity> ent, int parameter)
{
    if((!IsCharacter(ent)) || (parameter >= PARAM_SENTINEL))
        return 0;

    return ent->m_character->m_parameters.param[parameter];
}

int Character_ChangeParam(std::shared_ptr<Entity> ent, int parameter, float value)
{
    if((!IsCharacter(ent)) || (parameter >= PARAM_SENTINEL))
        return 0;

    float maximum = ent->m_character->m_parameters.maximum[parameter];
    float current = ent->m_character->m_parameters.param[parameter];

    if((current == maximum) && (value > 0))
        return 0;

    current += value;

    if(current < 0)
    {
        ent->m_character->m_parameters.param[parameter] = 0;
        return 0;
    }
    else if(current > maximum)
    {
        ent->m_character->m_parameters.param[parameter] = ent->m_character->m_parameters.maximum[parameter];
    }
    else
    {
        ent->m_character->m_parameters.param[parameter] = current;
    }

    return 1;
}

// overrided == 0x00: no overriding;
// overrided == 0x01: overriding mesh in armed state;
// overrided == 0x02: add mesh to slot in armed state;
// overrided == 0x03: overriding mesh in disarmed state;
// overrided == 0x04: add mesh to slot in disarmed state;
///@TODO: separate mesh replacing control and animation disabling / enabling
int Character_SetWeaponModel(std::shared_ptr<Entity> ent, int weapon_model, int armed)
{
    SkeletalModel* sm = engine_world.getModelByID(weapon_model);

    if((sm != NULL) && (ent->m_bf.bone_tags.size() == sm->mesh_count) && (sm->animations.size() >= 4))
    {
        SkeletalModel* bm = ent->m_bf.animations.model;
        if(ent->m_bf.animations.next == NULL)
        {
            ent->addOverrideAnim(weapon_model);
        }
        else
        {
            ent->m_bf.animations.next->model = sm;
        }

        for(int i=0;i<bm->mesh_count;i++)
        {
            ent->m_bf.bone_tags[i].mesh_base = bm->mesh_tree[i].mesh_base;
            ent->m_bf.bone_tags[i].mesh_slot = NULL;
        }

        if(armed != 0)
        {
            for(int i=0;i<bm->mesh_count;i++)
            {
                if(sm->mesh_tree[i].replace_mesh == 0x01)
                {
                    ent->m_bf.bone_tags[i].mesh_base = sm->mesh_tree[i].mesh_base;
                }
                else if(sm->mesh_tree[i].replace_mesh == 0x02)
                {
                    ent->m_bf.bone_tags[i].mesh_slot = sm->mesh_tree[i].mesh_base;
                }
            }
        }
        else
        {
            for(int i=0;i<bm->mesh_count;i++)
            {
                if(sm->mesh_tree[i].replace_mesh == 0x03)
                {
                    ent->m_bf.bone_tags[i].mesh_base = sm->mesh_tree[i].mesh_base;
                }
                else if(sm->mesh_tree[i].replace_mesh == 0x04)
                {
                    ent->m_bf.bone_tags[i].mesh_slot = sm->mesh_tree[i].mesh_base;
                }
            }
            ent->m_bf.animations.next->model = NULL;
        }

        return 1;
    }
    else
    {
        // do unarmed default model
        SkeletalModel* bm = ent->m_bf.animations.model;
        for(int i=0;i<bm->mesh_count;i++)
        {
            ent->m_bf.bone_tags[i].mesh_base = bm->mesh_tree[i].mesh_base;
            ent->m_bf.bone_tags[i].mesh_slot = NULL;
        }
        if(ent->m_bf.animations.next != NULL)
        {
            ent->m_bf.animations.next->model = NULL;
        }
    }

    return 0;
}
