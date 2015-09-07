#include "character_controller.h"

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

#include "anim_state_control.h"
#include "engine.h"
#include "entity.h"
#include "gui.h"
#include "hair.h"
#include "mesh.h"
#include "obb.h"
#include "polygon.h"
#include "resource.h"
#include "script.h"
#include "vmath.h"
#include "world.h"

Character::Character(uint32_t id)
    : Entity(id)
{
    m_sphere->setMargin(COLLISION_MARGIN_DEFAULT);

    m_climbSensor.reset(new btSphereShape(m_climbR));
    m_climbSensor->setMargin(COLLISION_MARGIN_DEFAULT);

    m_rayCb = std::make_shared<BtEngineClosestRayResultCallback>(m_self, true);
    m_rayCb->m_collisionFilterMask = btBroadphaseProxy::StaticFilter | btBroadphaseProxy::KinematicFilter;
    m_heightInfo.cb = m_rayCb;

    m_convexCb = std::make_shared<BtEngineClosestConvexResultCallback>(m_self, true);
    m_convexCb->m_collisionFilterMask = btBroadphaseProxy::StaticFilter | btBroadphaseProxy::KinematicFilter;
    m_heightInfo.ccb = m_convexCb;

    m_dirFlag = ENT_STAY;
}

Character::~Character()
{
    if((m_self->room != nullptr) && (this != engine_world.character.get()))
    {
        m_self->room->removeEntity(this);
    }
}

int32_t Character::addItem(uint32_t item_id, int32_t count)// returns items count after in the function's end
{
    Gui_NotifierStart(item_id);

    auto item = engine_world.getBaseItemByID(item_id);
    if(!item)
        return 0;

    count = (count < 0) ? item->count : count;

    for(InventoryNode& i : m_inventory)
    {
        if(i.id == item_id)
        {
            i.count += count;
            return i.count;
        }
    }

    InventoryNode i;
    i.id = item_id;
    i.count = count;
    m_inventory.push_back(i);

    return count;
}

int32_t Character::removeItem(uint32_t item_id, int32_t count) // returns items count after in the function's end
{
    if(m_inventory.empty())
    {
        return 0;
    }

    for(auto it = std::begin(m_inventory); it != std::end(m_inventory); ++it)
    {
        if(it->id == item_id)
        {
            if(it->count > count)
            {
                it->count -= count;
                return it->count;
            }
            else if(it->count == count)
            {
                m_inventory.erase(it);
                return 0;
            }
            else // count_to_remove > current_items_count
            {
                return it->count - count;
            }
        }
    }

    return -count;
}

int32_t Character::removeAllItems()
{
    if(m_inventory.empty())
    {
        return 0;
    }
    auto ret = m_inventory.size();
    m_inventory.clear();
    return static_cast<int32_t>( ret );
}

int32_t Character::getItemsCount(uint32_t item_id)         // returns items count
{
    for(const auto& item : m_inventory)
    {
        if(item.id == item_id)
        {
            return item.count;
        }
    }

    return 0;
}

/**
 * Calculates next height info and information about next step
 * @param ent
 */
void Character::updateCurrentHeight()
{
    btVector3 t;
    t[0] = 0.0;
    t[1] = 0.0;
    t[2] = m_bf.bone_tags[0].transform.getOrigin()[2];
    auto pos = m_transform * t;
    Character::getHeightInfo(pos, &m_heightInfo, m_height);
}

/*
 * Move character to the point where to platfom mowes
 */
void Character::updatePlatformPreStep()
{
#if 0
    if(character->platform)
    {
        EngineContainer* cont = (EngineContainer*)character->platform->getUserPointer();
        if(cont && (cont->object_type == OBJECT_ENTITY/* || cont->object_type == OBJECT_BULLET_MISC*/))
        {
            btScalar trpl[16];
            character->platform->getWorldTransform().getOpenGLMatrix(trpl);
#if 0
            new_tr = trpl * character->local_platform;
            vec3_copy(transform.getOrigin(), new_tr + 12);
#else
            ///make something with platform rotation
            transform = trpl * character->local_platform;
#endif
        }
    }
#endif
}

/*
 * Get local character transform relative platfom
 */
void Character::updatePlatformPostStep()
{
#if 0
    switch(move_type)
    {
        case MoveType::OnFloor:
            if(character->height_info.floor_hit)
            {
                character->platform = character->height_info.floor_obj;
            }
            break;

        case MoveType::Climbing:
            if(character->climb.edge_hit)
            {
                character->platform = character->climb.edge_obj;
            }
            break;

        default:
            character->platform = nullptr;
            break;
    };

    if(character->platform)
    {
        EngineContainer* cont = (EngineContainer*)character->platform->getUserPointer();
        if(cont && (cont->object_type == OBJECT_ENTITY/* || cont->object_type == OBJECT_BULLET_MISC*/))
        {
            btScalar trpl[16];
            character->platform->getWorldTransform().getOpenGLMatrix(trpl);
            /* local_platform = (global_platform ^ -1) x (global_entity); */
            character->local_platform = trpl.inverse() * transform;
        }
        else
        {
            character->platform = nullptr;
        }
    }
#endif
}

/**
 * Start position are taken from transform
 */
void Character::getHeightInfo(const btVector3& pos, struct HeightInfo *fc, btScalar v_offset)
{
    btVector3 from, to;
    auto cb = fc->cb;
    Room* r = (cb->m_container) ? (cb->m_container->room) : (nullptr);
    RoomSector* rs;

    fc->floor_hit = false;
    fc->ceiling_hit = false;
    fc->water = false;
    fc->quicksand = QuicksandPosition::None;
    fc->transition_level = 32512.0;

    r = Room_FindPosCogerrence(pos, r);
    if(r)
        r = r->checkFlip();
    if(r)
    {
        rs = r->getSectorXYZ(pos);                                         // if r != nullptr then rs can not been nullptr!!!
        if(r->flags & TR_ROOM_FLAG_WATER)                                       // in water - go up
        {
            while(rs->sector_above)
            {
                assert(rs->sector_above != nullptr);
                rs = rs->sector_above->checkFlip();
                assert(rs != nullptr && rs->owner_room != nullptr);
                if((rs->owner_room->flags & TR_ROOM_FLAG_WATER) == 0x00)        // find air
                {
                    fc->transition_level = static_cast<btScalar>(rs->floor);
                    fc->water = true;
                    break;
                }
            }
        }
        else if(r->flags & TR_ROOM_FLAG_QUICKSAND)
        {
            while(rs->sector_above)
            {
                assert(rs->sector_above != nullptr);
                rs = rs->sector_above->checkFlip();
                assert(rs != nullptr && rs->owner_room != nullptr);
                if((rs->owner_room->flags & TR_ROOM_FLAG_QUICKSAND) == 0x00)    // find air
                {
                    fc->transition_level = static_cast<btScalar>(rs->floor);
                    if(fc->transition_level - fc->floor_point[2] > v_offset)
                    {
                        fc->quicksand = QuicksandPosition::Drowning;
                    }
                    else
                    {
                        fc->quicksand = QuicksandPosition::Sinking;
                    }
                    break;
                }
            }
        }
        else                                                                    // in air - go down
        {
            while(rs->sector_below)
            {
                assert(rs->sector_below != nullptr);
                rs = rs->sector_below->checkFlip();
                assert(rs != nullptr && rs->owner_room != nullptr);
                if((rs->owner_room->flags & TR_ROOM_FLAG_WATER) != 0x00)        // find water
                {
                    fc->transition_level = static_cast<btScalar>(rs->ceiling);
                    fc->water = true;
                    break;
                }
                else if((rs->owner_room->flags & TR_ROOM_FLAG_QUICKSAND) != 0x00)        // find water
                {
                    fc->transition_level = static_cast<btScalar>(rs->ceiling);
                    if(fc->transition_level - fc->floor_point[2] > v_offset)
                    {
                        fc->quicksand = QuicksandPosition::Drowning;
                    }
                    else
                    {
                        fc->quicksand = QuicksandPosition::Sinking;
                    }
                    break;
                }
            }
        }
    }

    /*
     * GET HEIGHTS
     */
    from = pos;
    to = from;
    to[2] -= 4096.0;
    cb->m_closestHitFraction = 1.0;
    cb->m_collisionObject = nullptr;
    bt_engine_dynamicsWorld->rayTest(from, to, *cb);
    fc->floor_hit = cb->hasHit();
    if(fc->floor_hit)
    {
        fc->floor_normale = cb->m_hitNormalWorld;
        fc->floor_point.setInterpolate3(from, to, cb->m_closestHitFraction);
        fc->floor_obj = cb->m_collisionObject;
    }

    to = from;
    to[2] += 4096.0;
    cb->m_closestHitFraction = 1.0;
    cb->m_collisionObject = nullptr;
    //cb->m_flags = btTriangleRaycastCallback::kF_FilterBackfaces;
    bt_engine_dynamicsWorld->rayTest(from, to, *cb);
    fc->ceiling_hit = cb->hasHit();
    if(fc->ceiling_hit)
    {
        fc->ceiling_normale = cb->m_hitNormalWorld;
        fc->ceiling_point.setInterpolate3(from, to, cb->m_closestHitFraction);
        fc->ceiling_obj = cb->m_collisionObject;
    }
}

/**
 * Calculates next floor info + phantom filter + returns step info.
 * Current height info must be calculated!
 */
StepType Character::checkNextStep(const btVector3& offset, struct HeightInfo *nfc) const
{
    btScalar delta;
    btVector3 from, to;
    StepType ret;
    ///penetration test?

    auto pos = m_transform.getOrigin() + offset;
    Character::getHeightInfo(pos, nfc);

    if(m_heightInfo.floor_hit && nfc->floor_hit)
    {
        delta = nfc->floor_point[2] - m_heightInfo.floor_point[2];
        if(std::abs(delta) < SPLIT_EPSILON)
        {
            from[2] = m_heightInfo.floor_point[2];
            ret = StepType::Horizontal;                                    // horizontal
        }
        else if(delta < 0.0)                                                    // down way
        {
            delta = -delta;
            from[2] = m_heightInfo.floor_point[2];
            if(delta <= m_minStepUpHeight)
            {
                ret = StepType::DownLittle;
            }
            else if(delta <= m_maxStepUpHeight)
            {
                ret = StepType::DownBig;
            }
            else if(delta <= m_height)
            {
                ret = StepType::DownDrop;
            }
            else
            {
                ret = StepType::DownCanHang;
            }
        }
        else                                                                    // up way
        {
            from[2] = nfc->floor_point[2];
            if(delta <= m_minStepUpHeight)
            {
                ret = StepType::UpLittle;
            }
            else if(delta <= m_maxStepUpHeight)
            {
                ret = StepType::UpBig;
            }
            else if(delta <= m_maxClimbHeight)
            {
                ret = StepType::UpClimb;
            }
            else
            {
                ret = StepType::UpImpossible;
            }
        }
    }
    else if(!m_heightInfo.floor_hit && !nfc->floor_hit)
    {
        from[2] = pos[2];
        ret = StepType::Horizontal;                                        // horizontal? yes no maybe...
    }
    else if(!m_heightInfo.floor_hit && nfc->floor_hit)                                   // strange case
    {
        from[2] = nfc->floor_point[2];
        ret = StepType::Horizontal;
    }
    else //if(m_heightInfo.floor_hit && !nfc->floor_hit)                                 // bottomless
    {
        from[2] = m_heightInfo.floor_point[2];
        ret = StepType::DownCanHang;
    }

    /*
     * check walls! If test is positive, than CHARACTER_STEP_UP_IMPOSSIBLE - can not go next!
     */
    from[2] += m_climbR;
    to[2] = from[2];
    from[0] = m_transform.getOrigin()[0];
    from[1] = m_transform.getOrigin()[1];
    to[0] = pos[0];
    to[1] = pos[1];
    m_heightInfo.cb->m_closestHitFraction = 1.0;
    m_heightInfo.cb->m_collisionObject = nullptr;
    bt_engine_dynamicsWorld->rayTest(from, to, *m_heightInfo.cb);
    if(m_heightInfo.cb->hasHit())
    {
        ret = StepType::UpImpossible;
    }

    return ret;
}

/**
 * @param next_fc  next step floor / ceiling information
 * @retval @c true if character can't run / walk next; in other cases returns @c false
 */
bool Character::hasStopSlant(const HeightInfo& next_fc)
{
    const btVector3& pos = m_transform.getOrigin();
    const btVector3 forward = m_transform.getBasis().getColumn(1);
    const btVector3& floor = next_fc.floor_normale;

    return    next_fc.floor_point[2] > pos[2]
        && next_fc.floor_normale[2] < m_criticalSlantZComponent
        && (forward[0] * floor[0] + forward[1] * floor[1]) < 0.0;
}

/**
 * @FIXME: MAGICK CONST!
 * @param ent - entity
 * @param offset - offset, when we check height
 * @param nfc - height info (floor / ceiling)
 */
ClimbInfo Character::checkClimbability(const btVector3& offset, struct HeightInfo *nfc, btScalar test_height)
{
    btVector3 from, to;
    btScalar d;
    const auto& pos = m_transform.getOrigin();
    btTransform t1, t2;
    char up_founded;
    extern GLfloat cast_ray[6];                                                 // pointer to the test line coordinates
    /*
     * init callbacks functions
     */
    nfc->cb = m_rayCb;
    nfc->ccb = m_convexCb;
    auto tmp = pos + offset;                                        // tmp = native offset point

    ClimbInfo ret;
    ret.height_info = checkNextStep(offset + btVector3{0, 0, 128}, nfc); ///@FIXME: stick for big slant
    ret.can_hang = false;
    ret.edge_hit = false;
    ret.edge_obj = nullptr;
    ret.floor_limit = (m_heightInfo.floor_hit) ? (m_heightInfo.floor_point[2]) : (-9E10);
    ret.ceiling_limit = (m_heightInfo.ceiling_hit) ? (m_heightInfo.ceiling_point[2]) : (9E10);
    if(nfc->ceiling_hit && (nfc->ceiling_point[2] < ret.ceiling_limit))
    {
        ret.ceiling_limit = nfc->ceiling_point[2];
    }
    ret.point = m_climb.point;
    /*
     * check max height
     */
    if(m_heightInfo.ceiling_hit && (tmp[2] > m_heightInfo.ceiling_point[2] - m_climbR - 1.0))
    {
        tmp[2] = m_heightInfo.ceiling_point[2] - m_climbR - 1.0f;
    }

    /*
    * Let us calculate EDGE
    */
    from[0] = pos[0] - m_transform.getBasis().getColumn(1)[0] * m_climbR * 2.0f;
    from[1] = pos[1] - m_transform.getBasis().getColumn(1)[1] * m_climbR * 2.0f;
    from[2] = tmp[2];
    to = tmp;

    //vec3_copy(cast_ray, from);
    //vec3_copy(cast_ray+3, to);

    t1.setIdentity();
    t2.setIdentity();
    up_founded = 0;
    test_height = (test_height >= m_maxStepUpHeight) ? (test_height) : (m_maxStepUpHeight);
    d = pos[2] + m_bf.bb_max[2] - test_height;
    std::copy(to + 0, to + 3, cast_ray + 0);
    std::copy(to + 0, to + 3, cast_ray + 3);
    cast_ray[5] -= d;
    btVector3 n0{ 0,0,0 }, n1{ 0,0,0 };
    btScalar n0d{ 0 }, n1d{ 0 };
    do
    {
        t1.setOrigin(from);
        t2.setOrigin(to);
        nfc->ccb->m_closestHitFraction = 1.0;
        nfc->ccb->m_hitCollisionObject = nullptr;
        bt_engine_dynamicsWorld->convexSweepTest(m_climbSensor.get(), t1, t2, *nfc->ccb);
        if(nfc->ccb->hasHit())
        {
            if(nfc->ccb->m_hitNormalWorld[2] >= 0.1)
            {
                up_founded = 1;
                n0 = nfc->ccb->m_hitNormalWorld;
                n0d = -n0.dot(nfc->ccb->m_hitPointWorld);
            }
            if(up_founded && (nfc->ccb->m_hitNormalWorld[2] < 0.001))
            {
                n1 = nfc->ccb->m_hitNormalWorld;
                n1d = -n1.dot(nfc->ccb->m_hitPointWorld);
                m_climb.edge_obj = const_cast<btCollisionObject*>(nfc->ccb->m_hitCollisionObject);
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
            nfc->ccb->m_hitCollisionObject = nullptr;
            bt_engine_dynamicsWorld->convexSweepTest(m_climbSensor.get(), t1, t2, *nfc->ccb);
            if(nfc->ccb->hasHit())
            {
                up_founded = 1;
                n0 = nfc->ccb->m_hitNormalWorld;
                n0d = -n0.dot(nfc->ccb->m_hitPointWorld);
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
        from[2] -= 0.66f * m_climbR;
        to[2] -= 0.66f * m_climbR;
    } while(to[2] >= d);                                                 // we can't climb under floor!

    if(up_founded != 2)
    {
        return ret;
    }

    // get the character plane equation
    btVector3 n2 = m_transform.getBasis().getColumn(0);
    btScalar n2d = -n2.dot(pos);

    assert(!n0.fuzzyZero());
    assert(!n1.fuzzyZero());
    assert(!n2.fuzzyZero());
    /*
     * Solve system of the linear equations by Kramer method!
     * I know - It may be slow, but it has a good precision!
     * The root is point of 3 planes intersection.
     */
    d = -n0[0] * (n1[1] * n2[2] - n1[2] * n2[1]) +
        n1[0] * (n0[1] * n2[2] - n0[2] * n2[1]) -
        n2[0] * (n0[1] * n1[2] - n0[2] * n1[1]);

    if(std::abs(d) < 0.005)
    {
        return ret;
    }

    ret.edge_point[0] = n0d * (n1[1] * n2[2] - n1[2] * n2[1]) -
        n1d * (n0[1] * n2[2] - n0[2] * n2[1]) +
        n2d * (n0[1] * n1[2] - n0[2] * n1[1]);
    ret.edge_point[0] /= d;

    ret.edge_point[1] = n0[0] * (n1d * n2[2] - n1[2] * n2d) -
        n1[0] * (n0d * n2[2] - n0[2] * n2d) +
        n2[0] * (n0d * n1[2] - n0[2] * n1d);
    ret.edge_point[1] /= d;

    ret.edge_point[2] = n0[0] * (n1[1] * n2d - n1d * n2[1]) -
        n1[0] * (n0[1] * n2d - n0d * n2[1]) +
        n2[0] * (n0[1] * n1d - n0d * n1[1]);
    ret.edge_point[2] /= d;
    ret.point = ret.edge_point;
    std::copy(ret.point + 0, ret.point + 3, cast_ray + 3);
    /*
     * unclimbable edge slant %)
     */
    n2 = n0.cross(n1);
    d = m_criticalSlantZComponent;
    d *= d * (n2[0] * n2[0] + n2[1] * n2[1] + n2[2] * n2[2]);
    if(n2[2] * n2[2] > d)
    {
        return ret;
    }

    /*
     * Now, let us calculate z_angle
     */
    ret.edge_hit = true;

    n2[2] = n2[0];
    n2[0] = n2[1];
    n2[1] = -n2[2];
    n2[2] = 0.0;
    if(n2[0] * m_transform.getBasis().getColumn(1)[0] + n2[1] * m_transform.getBasis().getColumn(1)[1] > 0)       // direction fixing
    {
        n2[0] = -n2[0];
        n2[1] = -n2[1];
    }

    ret.n = n2;
    ret.up[0] = 0.0;
    ret.up[1] = 0.0;
    ret.up[2] = 1.0;
    ret.edge_z_ang = std::atan2(n2[0], -n2[1]) * DegPerRad;
    ret.edge_tan_xy[0] = -n2[1];
    ret.edge_tan_xy[1] = n2[0];
    ret.edge_tan_xy[2] = 0.0;
    ret.edge_tan_xy /= btSqrt(n2[0] * n2[0] + n2[1] * n2[1]);
    ret.right = ret.edge_tan_xy;

    if(!m_heightInfo.floor_hit || (ret.edge_point[2] - m_heightInfo.floor_point[2] >= m_height))
    {
        ret.can_hang = true;
    }

    ret.next_z_space = 2.0f * m_height;
    if(nfc->floor_hit && nfc->ceiling_hit)
    {
        ret.next_z_space = nfc->ceiling_point[2] - nfc->floor_point[2];
    }

    return ret;
}

ClimbInfo Character::checkWallsClimbability()
{
    ClimbInfo ret;
    ret.can_hang = false;
    ret.wall_hit = ClimbType::None;
    ret.edge_hit = false;
    ret.edge_obj = nullptr;
    ret.floor_limit = (m_heightInfo.floor_hit) ? (m_heightInfo.floor_point[2]) : (-9E10);
    ret.ceiling_limit = (m_heightInfo.ceiling_hit) ? (m_heightInfo.ceiling_point[2]) : (9E10);
    ret.point = m_climb.point;

    if(!m_heightInfo.walls_climb)
    {
        return ret;
    }

    ret.up = { 0,0,1 };

    btVector3& pos = m_transform.getOrigin();
    btVector3 from = pos + m_transform.getBasis().getColumn(2) * m_bf.bb_max[2] - m_transform.getBasis().getColumn(1) * m_climbR;
    btVector3 to = from;
    btScalar t = m_forwardSize + m_bf.bb_max[1];
    to += m_transform.getBasis().getColumn(1) * t;

    auto ccb = m_convexCb;
    ccb->m_closestHitFraction = 1.0;
    ccb->m_hitCollisionObject = nullptr;

    btTransform tr1;
    tr1.setIdentity();
    tr1.setOrigin(from);

    btTransform tr2;
    tr2.setIdentity();
    tr2.setOrigin(to);

    bt_engine_dynamicsWorld->convexSweepTest(m_climbSensor.get(), tr1, tr2, *ccb);
    if(!ccb->hasHit())
    {
        return ret;
    }

    ret.point = ccb->m_hitPointWorld;
    ret.n = ccb->m_hitNormalWorld;
    btScalar wn2[2] = { ret.n[0], ret.n[1] };
    t = sqrt(wn2[0] * wn2[0] + wn2[1] * wn2[1]);
    wn2[0] /= t;
    wn2[1] /= t;

    ret.right[0] = -wn2[1];
    ret.right[1] = wn2[0];
    ret.right[2] = 0.0;
    // now we have wall normale in XOY plane. Let us check all flags

    if((m_heightInfo.walls_climb_dir & SECTOR_FLAG_CLIMB_NORTH) && (wn2[1] < -0.7))
    {
        ret.wall_hit = ClimbType::HandsOnly;                                    // nW = (0, -1, 0);
    }
    if((m_heightInfo.walls_climb_dir & SECTOR_FLAG_CLIMB_EAST) && (wn2[0] < -0.7))
    {
        ret.wall_hit = ClimbType::HandsOnly;                                    // nW = (-1, 0, 0);
    }
    if((m_heightInfo.walls_climb_dir & SECTOR_FLAG_CLIMB_SOUTH) && (wn2[1] > 0.7))
    {
        ret.wall_hit = ClimbType::HandsOnly;                                    // nW = (0, 1, 0);
    }
    if((m_heightInfo.walls_climb_dir & SECTOR_FLAG_CLIMB_WEST) && (wn2[0] > 0.7))
    {
        ret.wall_hit = ClimbType::HandsOnly;                                    // nW = (1, 0, 0);
    }

    if(ret.wall_hit != ClimbType::None)
    {
        t = 0.67f * m_height;
        from -= m_transform.getBasis().getColumn(2) * t;
        to = from;
        t = m_forwardSize + m_bf.bb_max[1];
        to += m_transform.getBasis().getColumn(1) * t;

        ccb->m_closestHitFraction = 1.0;
        ccb->m_hitCollisionObject = nullptr;
        tr1.setIdentity();
        tr1.setOrigin(from);
        tr2.setIdentity();
        tr2.setOrigin(to);
        bt_engine_dynamicsWorld->convexSweepTest(m_climbSensor.get(), tr1, tr2, *ccb);
        if(ccb->hasHit())
        {
            ret.wall_hit = ClimbType::FullBody;
        }
    }

    // now check ceiling limit (and floor too... may be later)
    /*vec3_add(from, point, transform.getBasis().getColumn(1));
    to = from;
    from[2] += 520.0;                                                  ///@FIXME: magick;
    to[2] -= 520.0;                                                    ///@FIXME: magick... again...
    cb->m_closestHitFraction = 1.0;
    cb->m_collisionObject = nullptr;
    bt_engine_dynamicsWorld->rayTest(from, to, *cb);
    if(cb->hasHit())
    {
        point.setInterpolate3(from, to, cb->m_closestHitFraction);
        ret.ceiling_limit = (ret.ceiling_limit > point[2])?(point[2]):(ret.ceiling_limit);
    }*/

    return ret;
}

void Character::lean(CharacterCommand *cmd, btScalar max_lean)
{
    btScalar neg_lean = 360.0 - max_lean;
    btScalar lean_coeff = (max_lean == 0.0) ? (48.0) : (max_lean * 3);

    // Continously lean character, according to current left/right direction.

    if((cmd->move[1] == 0) || (max_lean == 0.0))       // No direction - restore straight vertical position!
    {
        if(m_angles[2] != 0.0)
        {
            if(m_angles[2] < 180.0)
            {
                m_angles[2] -= ((std::abs(m_angles[2]) + lean_coeff) / 2) * engine_frame_time;
                if(m_angles[2] < 0.0) m_angles[2] = 0.0;
            }
            else
            {
                m_angles[2] += ((360 - std::abs(m_angles[2]) + lean_coeff) / 2) * engine_frame_time;
                if(m_angles[2] < 180.0) m_angles[2] = 0.0;
            }
        }
    }
    else if(cmd->move[1] == 1) // Right direction
    {
        if(m_angles[2] != max_lean)
        {
            if(m_angles[2] < max_lean)   // Approaching from center
            {
                m_angles[2] += ((std::abs(m_angles[2]) + lean_coeff) / 2) * engine_frame_time;
                if(m_angles[2] > max_lean)
                    m_angles[2] = max_lean;
            }
            else if(m_angles[2] > 180.0) // Approaching from left
            {
                m_angles[2] += ((360.0f - std::abs(m_angles[2]) + (lean_coeff*2) / 2) * engine_frame_time);
                if(m_angles[2] < 180.0) m_angles[2] = 0.0;
            }
            else    // Reduce previous lean
            {
                m_angles[2] -= ((std::abs(m_angles[2]) + lean_coeff) / 2) * engine_frame_time;
                if(m_angles[2] < 0.0) m_angles[2] = 0.0;
            }
        }
    }
    else if(cmd->move[1] == -1)     // Left direction
    {
        if(m_angles[2] != neg_lean)
        {
            if(m_angles[2] > neg_lean)   // Reduce previous lean
            {
                m_angles[2] -= ((360.0f - std::abs(m_angles[2]) + lean_coeff) / 2) * engine_frame_time;
                if(m_angles[2] < neg_lean)
                    m_angles[2] = neg_lean;
            }
            else if(m_angles[2] < 180.0) // Approaching from right
            {
                m_angles[2] -= ((std::abs(m_angles[2]) + (lean_coeff * 2)) / 2) * engine_frame_time;
                if(m_angles[2] < 0.0) m_angles[2] += 360.0;
            }
            else    // Approaching from center
            {
                m_angles[2] += ((360.0f - std::abs(m_angles[2]) + lean_coeff) / 2) * engine_frame_time;
                if(m_angles[2] > 360.0) m_angles[2] -= 360.0f;
            }
        }
    }
}

/*
 * Linear inertia is absolutely needed for in-water states, and also it gives
 * more organic feel to land animations.
 */
btScalar Character::inertiaLinear(btScalar max_speed, btScalar accel, bool command)
{
    if((accel == 0.0) || (accel >= max_speed))
    {
        if(command)
        {
            m_inertiaLinear = max_speed;
        }
        else
        {
            m_inertiaLinear = 0.0;
        }
    }
    else
    {
        if(command)
        {
            if(m_inertiaLinear < max_speed)
            {
                m_inertiaLinear += max_speed * accel * engine_frame_time;
                if(m_inertiaLinear > max_speed) m_inertiaLinear = max_speed;
            }
        }
        else
        {
            if(m_inertiaLinear > 0.0)
            {
                m_inertiaLinear -= max_speed * accel * engine_frame_time;
                if(m_inertiaLinear < 0.0) m_inertiaLinear = 0.0;
            }
        }
    }

    return m_inertiaLinear * m_speedMult;
}

/*
 * Angular inertia is used on keyboard-driven (non-analog) rotational controls.
 */
btScalar Character::inertiaAngular(btScalar max_angle, btScalar accel, uint8_t axis)
{
    if(axis > 1) return 0.0;

    uint8_t curr_rot_dir = 0;
    if(m_command.rot[axis] < 0.0)
    {
        curr_rot_dir = 1;
    }
    else if(m_command.rot[axis] > 0.0)
    {
        curr_rot_dir = 2;
    }

    if((!curr_rot_dir) || (max_angle == 0.0) || (accel == 0.0))
    {
        m_inertiaAngular[axis] = 0.0;
    }
    else
    {
        if(m_inertiaAngular[axis] != max_angle)
        {
            if(curr_rot_dir == 2)
            {
                if(m_inertiaAngular[axis] < 0.0)
                {
                    m_inertiaAngular[axis] = 0.0;
                }
                else
                {
                    m_inertiaAngular[axis] += max_angle * accel * engine_frame_time;
                    if(m_inertiaAngular[axis] > max_angle) m_inertiaAngular[axis] = max_angle;
                }
            }
            else
            {
                if(m_inertiaAngular[axis] > 0.0)
                {
                    m_inertiaAngular[axis] = 0.0;
                }
                else
                {
                    m_inertiaAngular[axis] -= max_angle * accel * engine_frame_time;
                    if(m_inertiaAngular[axis] < -max_angle) m_inertiaAngular[axis] = -max_angle;
                }
            }
        }
    }

    return std::abs(m_inertiaAngular[axis]) * m_command.rot[axis];
}

/*
 * MOVE IN DIFFERENT CONDITIONS
 */
int Character::moveOnFloor()
{
    /*
     * init height info structure
     */
    m_response.horizontal_collide = 0x00;
    m_response.vertical_collide = 0x00;
    // First of all - get information about floor and ceiling!!!
    updateCurrentHeight();
    if(m_heightInfo.floor_hit && (m_heightInfo.floor_point[2] + 1.0 >= m_transform.getOrigin()[2] + m_bf.bb_min[2]))
    {
        EngineContainer* cont = static_cast<EngineContainer*>(m_heightInfo.floor_obj->getUserPointer());
        if((cont != nullptr) && (cont->object_type == OBJECT_ENTITY))
        {
            Entity* e = static_cast<Entity*>(cont->object);
            if(e->m_callbackFlags & ENTITY_CALLBACK_STAND)
            {
                engine_lua.execEntity(ENTITY_CALLBACK_STAND, e->id(), id());
            }
        }
    }

    btVector3& position = m_transform.getOrigin();
    btVector3 speed(0.0, 0.0, 0.0);

    /*
     * check move type
     */
    if(m_heightInfo.floor_hit || (m_response.vertical_collide & 0x01))
    {
        if(m_heightInfo.floor_point[2] + m_fallDownHeight < position[2])
        {
            m_moveType = MoveType::FreeFalling;
            m_speed[2] = 0.0;
            return -1;                                                          // nothing to do here
        }
        else
        {
            m_response.vertical_collide |= 0x01;
        }

        btVector3 floorNormal = m_heightInfo.floor_normale;
        if(floorNormal[2] > 0.02 && floorNormal[2] < m_criticalSlantZComponent)
        {
            floorNormal[2] = -floorNormal[2];
            speed = floorNormal * m_speedMult * DEFAULT_CHARACTER_SLIDE_SPEED_MULT; // slide down direction
            const btScalar zAngle = std::atan2(floorNormal[0], -floorNormal[1]) * DegPerRad;       // from -180 deg to +180 deg
            //ang = (ang < 0.0)?(ang + 360.0):(ang);
            btScalar t = floorNormal[0] * m_transform.getBasis().getColumn(1)[0]
                + floorNormal[1] * m_transform.getBasis().getColumn(1)[1];
            if(t >= 0.0)
            {
                // front forward slide down
                m_response.slide = SlideType::Front;
                m_angles[0] = zAngle + 180;
            }
            else
            {
                // back forward slide down
                m_response.slide = SlideType::Back;
                m_angles[0] = zAngle;
            }
            updateTransform();
            m_response.vertical_collide |= 0x01;
        }
        else    // no slide - free to walk
        {
            const btScalar fullSpeed = m_currentSpeed * m_speedMult;
            m_response.vertical_collide |= 0x01;

            m_angles[0] += inertiaAngular(1.0, ROT_SPEED_LAND, 0);

            updateTransform(); // apply rotations

            if(m_dirFlag & ENT_MOVE_FORWARD)
            {
                speed = m_transform.getBasis().getColumn(1) * fullSpeed;
            }
            else if(m_dirFlag & ENT_MOVE_BACKWARD)
            {
                speed = m_transform.getBasis().getColumn(1) * -fullSpeed;
            }
            else if(m_dirFlag & ENT_MOVE_LEFT)
            {
                speed = m_transform.getBasis().getColumn(0) * -fullSpeed;
            }
            else if(m_dirFlag & ENT_MOVE_RIGHT)
            {
                speed = m_transform.getBasis().getColumn(0) * fullSpeed;
            }
            else
            {
                //dir_flag = ENT_MOVE_FORWARD;
            }
            m_response.slide = SlideType::None;
        }
    }
    else                                                                        // no hit to the floor
    {
        m_response.slide = SlideType::None;
        m_response.vertical_collide = 0x00;
        m_moveType = MoveType::FreeFalling;
        m_speed[2] = 0.0;
        return -1;                                                              // nothing to do here
    }

    /*
     * now move normally
     */
    m_speed = speed;
    btVector3 positionDelta = speed * engine_frame_time;
    const btScalar distance = positionDelta.length();

    btVector3 norm_move_xy(positionDelta[0], positionDelta[1], 0.0);
    btScalar norm_move_xy_len = norm_move_xy.length();
    if(norm_move_xy_len > 0.2 * distance)
    {
        norm_move_xy /= norm_move_xy_len;
    }
    else
    {
        norm_move_xy.setZero();
    }

    ghostUpdate();
    position += positionDelta;
    fixPenetrations(&positionDelta);
    if(m_heightInfo.floor_hit)
    {
        if(m_heightInfo.floor_point[2] + m_fallDownHeight > position[2])
        {
            btScalar dz_to_land = engine_frame_time * 2400.0f;                   ///@FIXME: magick
            if(position[2] > m_heightInfo.floor_point[2] + dz_to_land)
            {
                position[2] -= dz_to_land;
                fixPenetrations(nullptr);
            }
            else if(position[2] > m_heightInfo.floor_point[2])
            {
                position[2] = m_heightInfo.floor_point[2];
                fixPenetrations(nullptr);
            }
        }
        else
        {
            m_moveType = MoveType::FreeFalling;
            m_speed[2] = 0.0;
            updateRoomPos();
            return 2;
        }
        if((position[2] < m_heightInfo.floor_point[2]) && !m_bt.no_fix_all)
        {
            position[2] = m_heightInfo.floor_point[2];
            fixPenetrations(nullptr);
            m_response.vertical_collide |= 0x01;
        }
    }
    else if(!(m_response.vertical_collide & 0x01))
    {
        m_moveType = MoveType::FreeFalling;
        m_speed[2] = 0.0;
        updateRoomPos();
        return 2;
    }

    updateRoomPos();

    return 1;
}

int Character::freeFalling()
{
    btVector3& pos = m_transform.getOrigin();

    /*
     * init height info structure
     */

    m_response.slide = SlideType::None;
    m_response.horizontal_collide = 0x00;
    m_response.vertical_collide = 0x00;

    btScalar rot = inertiaAngular(1.0, ROT_SPEED_FREEFALL, 0);
    m_angles[0] += rot;
    m_angles[1] = 0.0;

    updateTransform();  // apply rotations

    btVector3 move = applyGravity(engine_frame_time);
    m_speed[2] = (m_speed[2] < -FREE_FALL_SPEED_MAXIMUM) ? (-FREE_FALL_SPEED_MAXIMUM) : (m_speed[2]);
    m_speed = m_speed.rotate({ 0,0,1 }, rot * RadPerDeg);

    updateCurrentHeight();

    if(m_self->room && (m_self->room->flags & TR_ROOM_FLAG_WATER))
    {
        if(m_speed[2] < 0.0)
        {
            m_currentSpeed = 0.0;
            m_speed[0] = 0.0;
            m_speed[1] = 0.0;
        }

        if((engine_world.engineVersion < loader::Engine::TR2))//Lara cannot wade in < TRII so when floor < transition level she has to swim
        {
            if(!m_heightInfo.water || (m_currentSector->floor <= m_heightInfo.transition_level))
            {
                m_moveType = MoveType::Underwater;
                return 2;
            }
        }
        else
        {
            if(!m_heightInfo.water || (m_currentSector->floor + m_height <= m_heightInfo.transition_level))
            {
                m_moveType = MoveType::Underwater;
                return 2;
            }
        }
    }

    ghostUpdate();

    if(m_heightInfo.ceiling_hit && m_speed[2] > 0.0)
    {
        if(m_heightInfo.ceiling_point[2] < m_bf.bb_max[2] + pos[2])
        {
            pos[2] = m_heightInfo.ceiling_point[2] - m_bf.bb_max[2];
            m_speed[2] = 1.0;   // As in original.
            m_response.vertical_collide |= 0x02;
            fixPenetrations(nullptr);
            updateRoomPos();
        }
    }
    if(m_heightInfo.floor_hit && m_speed[2] < 0.0)   // move down
    {
        if(m_heightInfo.floor_point[2] >= pos[2] + m_bf.bb_min[2] + move[2])
        {
            pos[2] = m_heightInfo.floor_point[2];
            //speed[2] = 0.0;
            m_moveType = MoveType::OnFloor;
            m_response.vertical_collide |= 0x01;
            fixPenetrations(nullptr);
            updateRoomPos();
            return 2;
        }
    }

    pos += move;
    fixPenetrations(&move);                           // get horizontal collide

    if(m_heightInfo.ceiling_hit && m_speed[2] > 0.0)
    {
        if(m_heightInfo.ceiling_point[2] < m_bf.bb_max[2] + pos[2])
        {
            pos[2] = m_heightInfo.ceiling_point[2] - m_bf.bb_max[2];
            m_speed[2] = 1.0;   // As in original.
            m_response.vertical_collide |= 0x02;
        }
    }
    if(m_heightInfo.floor_hit && m_speed[2] < 0.0)   // move down
    {
        if(m_heightInfo.floor_point[2] >= pos[2] + m_bf.bb_min[2] + move[2])
        {
            pos[2] = m_heightInfo.floor_point[2];
            //speed[2] = 0.0;
            m_moveType = MoveType::OnFloor;
            m_response.vertical_collide |= 0x01;
            fixPenetrations(nullptr);
            updateRoomPos();
            return 2;
        }
    }
    updateRoomPos();

    return 1;
}

/*
 * Monkey CLIMBING - MOVE NO Z LANDING
 */
int Character::monkeyClimbing()
{
    btVector3 move, spd(0.0, 0.0, 0.0);
    btScalar t;
    auto& pos = m_transform.getOrigin();

    m_speed[2] = 0.0;

    m_response.slide = SlideType::None;
    m_response.lean  = LeanType::None;

    m_response.horizontal_collide = 0x00;
    m_response.vertical_collide = 0x00;

    t = m_currentSpeed * m_speedMult;
    m_response.vertical_collide |= 0x01;

    m_angles[0] += inertiaAngular(1.0, ROT_SPEED_MONKEYSWING, 0);
    m_angles[1] = 0.0;
    m_angles[2] = 0.0;
    updateTransform();                                                 // apply rotations

    if(m_dirFlag & ENT_MOVE_FORWARD)
    {
        spd = m_transform.getBasis().getColumn(1) * t;
    }
    else if(m_dirFlag & ENT_MOVE_BACKWARD)
    {
        spd = m_transform.getBasis().getColumn(1) * -t;
    }
    else if(m_dirFlag & ENT_MOVE_LEFT)
    {
        spd = m_transform.getBasis().getColumn(0) * -t;
    }
    else if(m_dirFlag & ENT_MOVE_RIGHT)
    {
        spd = m_transform.getBasis().getColumn(0) * t;
    }
    else
    {
        //dir_flag = ENT_MOVE_FORWARD;
    }

    m_speed = spd;
    move = spd * engine_frame_time;
    move[2] = 0.0;

    ghostUpdate();
    updateCurrentHeight();
    pos += move;
    fixPenetrations(&move);                              // get horizontal collide
    ///@FIXME: rewrite conditions! or add fixer to update_entity_rigid_body func
    if(m_heightInfo.ceiling_hit && (pos[2] + m_bf.bb_max[2] - m_heightInfo.ceiling_point[2] > -0.33 * m_minStepUpHeight))
    {
        pos[2] = m_heightInfo.ceiling_point[2] - m_bf.bb_max[2];
    }
    else
    {
        m_moveType = MoveType::FreeFalling;
        updateRoomPos();
        return 2;
    }

    updateRoomPos();

    return 1;
}

/*
 * WALLS CLIMBING - MOVE IN ZT plane
 */
int Character::wallsClimbing()
{
    ClimbInfo* climb = &m_climb;
    btVector3 spd, move;
    btScalar t;
    auto& pos = m_transform.getOrigin();

    m_response.slide = SlideType::None;
    m_response.horizontal_collide = 0x00;
    m_response.vertical_collide = 0x00;

    spd = { 0,0,0 };
    *climb = checkWallsClimbability();
    m_climb = *climb;
    if(climb->wall_hit == ClimbType::None)
    {
        m_heightInfo.walls_climb = false;
        return 2;
    }

    m_angles[0] = std::atan2(climb->n[0], -climb->n[1]) * DegPerRad;
    updateTransform();
    pos[0] = climb->point[0] - m_transform.getBasis().getColumn(1)[0] * m_bf.bb_max[1];
    pos[1] = climb->point[1] - m_transform.getBasis().getColumn(1)[1] * m_bf.bb_max[1];

    if(m_dirFlag == ENT_MOVE_FORWARD)
    {
        spd += climb->up;
    }
    else if(m_dirFlag == ENT_MOVE_BACKWARD)
    {
        spd -= climb->up;
    }
    else if(m_dirFlag == ENT_MOVE_RIGHT)
    {
        spd += climb->right;
    }
    else if(m_dirFlag == ENT_MOVE_LEFT)
    {
        spd -= climb->right;
    }
    t = spd.length();
    if(t > 0.01)
    {
        spd /= t;
    }
    m_speed = spd * m_currentSpeed * m_speedMult;
    move = m_speed * engine_frame_time;

    ghostUpdate();
    updateCurrentHeight();
    pos += move;
    fixPenetrations(&move); // get horizontal collide
    updateRoomPos();

    *climb = checkWallsClimbability();
    if(pos[2] + m_bf.bb_max[2] > climb->ceiling_limit)
    {
        pos[2] = climb->ceiling_limit - m_bf.bb_max[2];
    }

    return 1;
}

/*
 * CLIMBING - MOVE NO Z LANDING
 */
int Character::climbing()
{
    btVector3 move, spd;
    btScalar t;
    auto& pos = m_transform.getOrigin();
    btScalar z = pos[2];

    m_response.slide = SlideType::None;
    m_response.horizontal_collide = 0x00;
    m_response.vertical_collide = 0x00;

    t = m_currentSpeed * m_speedMult;
    m_response.vertical_collide |= 0x01;
    m_angles[0] += m_command.rot[0];
    m_angles[1] = 0.0;
    m_angles[2] = 0.0;
    updateTransform();                                                 // apply rotations

    if(m_dirFlag == ENT_MOVE_FORWARD)
    {
        spd = m_transform.getBasis().getColumn(1) * t;
    }
    else if(m_dirFlag == ENT_MOVE_BACKWARD)
    {
        spd = m_transform.getBasis().getColumn(1) * -t;
    }
    else if(m_dirFlag == ENT_MOVE_LEFT)
    {
        spd = m_transform.getBasis().getColumn(0) * -t;
    }
    else if(m_dirFlag == ENT_MOVE_RIGHT)
    {
        spd = m_transform.getBasis().getColumn(0) * t;
    }
    else
    {
        m_response.slide = SlideType::None;
        ghostUpdate();
        fixPenetrations(nullptr);
        return 1;
    }

    m_response.slide = SlideType::None;

    m_speed = spd;
    move = spd * engine_frame_time;

    ghostUpdate();
    pos += move;
    fixPenetrations(&move);                              // get horizontal collide
    updateRoomPos();
    pos[2] = z;

    return 1;
}

/*
 * underwater and onwater swimming has a big trouble:
 * the speed and acceleration information is absent...
 * I add some sticks to make it work for testing.
 * I thought to make export anim information to LUA script...
 */
int Character::moveUnderWater()
{
    btVector3 move, spd(0.0, 0.0, 0.0);
    auto& pos = m_transform.getOrigin();

    // Check current place.

    if(m_self->room && !(m_self->room->flags & TR_ROOM_FLAG_WATER))
    {
        m_moveType = MoveType::FreeFalling;
        return 2;
    }

    m_response.slide = SlideType::None;
    m_response.lean  = LeanType::None;

    m_response.horizontal_collide = 0x00;
    m_response.vertical_collide   = 0x00;

    // Calculate current speed.

    btScalar t = inertiaLinear(MAX_SPEED_UNDERWATER, INERTIA_SPEED_UNDERWATER, m_command.jump);

    if(!m_response.killed)   // Block controls if Lara is dead.
    {
        m_angles[0] += inertiaAngular(1.0, ROT_SPEED_UNDERWATER, 0);
        m_angles[1] -= inertiaAngular(1.0, ROT_SPEED_UNDERWATER, 1);
        m_angles[2] = 0.0;

        if((m_angles[1] > 70.0) && (m_angles[1] < 180.0))                 // Underwater angle limiter.
        {
            m_angles[1] = 70.0;
        }
        else if((m_angles[1] > 180.0) && (m_angles[1] < 270.0))
        {
            m_angles[1] = 270.0;
        }

        updateTransform();                                             // apply rotations

        spd = m_transform.getBasis().getColumn(1) * t;                     // OY move only!
        m_speed = spd;
    }

    move = spd * engine_frame_time;

    ghostUpdate();
    pos += move;
    fixPenetrations(&move);                              // get horizontal collide

    updateRoomPos();
    if(m_heightInfo.water && (pos[2] + m_bf.bb_max[2] >= m_heightInfo.transition_level))
    {
        if(/*(spd[2] > 0.0)*/m_transform.getBasis().getColumn(1)[2] > 0.67)             ///@FIXME: magick!
        {
            m_moveType = MoveType::OnWater;
            //pos[2] = fc.transition_level;
            return 2;
        }
        if(!m_heightInfo.floor_hit || (m_heightInfo.transition_level - m_heightInfo.floor_point[2] >= m_height))
        {
            pos[2] = m_heightInfo.transition_level - m_bf.bb_max[2];
        }
    }

    return 1;
}

int Character::moveOnWater()
{
    btVector3 move, spd;
    auto& pos = m_transform.getOrigin();

    m_response.slide = SlideType::None;
    m_response.lean  = LeanType::None;

    m_response.horizontal_collide = 0x00;
    m_response.vertical_collide   = 0x00;

    m_angles[0] += inertiaAngular(1.0, ROT_SPEED_ONWATER, 0);
    m_angles[1] = 0.0;
    m_angles[2] = 0.0;
    updateTransform();     // apply rotations

    // Calculate current speed.

    btScalar t = inertiaLinear(MAX_SPEED_ONWATER, INERTIA_SPEED_ONWATER, std::abs(m_command.move[0]) != 0 || std::abs(m_command.move[1]) != 0);

    if((m_dirFlag & ENT_MOVE_FORWARD) && (m_command.move[0] == 1))
    {
        spd = m_transform.getBasis().getColumn(1) * t;
    }
    else if((m_dirFlag & ENT_MOVE_BACKWARD) && (m_command.move[0] == -1))
    {
        spd = m_transform.getBasis().getColumn(1) * -t;
    }
    else if((m_dirFlag & ENT_MOVE_LEFT) && (m_command.move[1] == -1))
    {
        spd = m_transform.getBasis().getColumn(0) * -t;
    }
    else if((m_dirFlag & ENT_MOVE_RIGHT) && (m_command.move[1] == 1))
    {
        spd = m_transform.getBasis().getColumn(0) * t;
    }
    else
    {
        ghostUpdate();
        fixPenetrations(nullptr);
        updateRoomPos();
        if(m_heightInfo.water)
        {
            pos[2] = m_heightInfo.transition_level;
        }
        else
        {
            m_moveType = MoveType::OnFloor;
            return 2;
        }
        return 1;
    }

    /*
     * Prepare to moving
     */
    m_speed = spd;
    move = spd * engine_frame_time;
    ghostUpdate();
    pos += move;
    fixPenetrations(&move);  // get horizontal collide

    updateRoomPos();
    if(m_heightInfo.water)
    {
        pos[2] = m_heightInfo.transition_level;
    }
    else
    {
        m_moveType = MoveType::OnFloor;
        return 2;
    }

    return 1;
}

int Character::findTraverse()
{
    RoomSector* ch_s, *obj_s = nullptr;
    ch_s = m_self->room->getSectorRaw(m_transform.getOrigin());

    if(ch_s == nullptr)
    {
        return 0;
    }

    m_traversedObject = nullptr;

    // OX move case
    if(m_transform.getBasis().getColumn(1)[0] > 0.9)
    {
        obj_s = ch_s->owner_room->getSectorRaw({ static_cast<btScalar>(ch_s->pos[0] + TR_METERING_SECTORSIZE), static_cast<btScalar>(ch_s->pos[1]), static_cast<btScalar>(0.0) });
    }
    else if(m_transform.getBasis().getColumn(1)[0] < -0.9)
    {
        obj_s = ch_s->owner_room->getSectorRaw({ static_cast<btScalar>(ch_s->pos[0] - TR_METERING_SECTORSIZE), static_cast<btScalar>(ch_s->pos[1]), static_cast<btScalar>(0.0) });
    }
    // OY move case
    else if(m_transform.getBasis().getColumn(1)[1] > 0.9)
    {
        obj_s = ch_s->owner_room->getSectorRaw({ static_cast<btScalar>(ch_s->pos[0]), static_cast<btScalar>(ch_s->pos[1] + TR_METERING_SECTORSIZE), static_cast<btScalar>(0.0) });
    }
    else if(m_transform.getBasis().getColumn(1)[1] < -0.9)
    {
        obj_s = ch_s->owner_room->getSectorRaw({ static_cast<btScalar>(ch_s->pos[0]), static_cast<btScalar>(ch_s->pos[1] - TR_METERING_SECTORSIZE), static_cast<btScalar>(0.0) });
    }

    if(obj_s != nullptr)
    {
        obj_s = obj_s->checkPortalPointer();
        for(std::shared_ptr<EngineContainer>& cont : obj_s->owner_room->containers)
        {
            if(cont->object_type == OBJECT_ENTITY)
            {
                Entity* e = static_cast<Entity*>(cont->object);
                if((e->m_typeFlags & ENTITY_TYPE_TRAVERSE) && (1 == OBB_OBB_Test(*e, *this) && (std::abs(e->m_transform.getOrigin()[2] - m_transform.getOrigin()[2]) < 1.1)))
                {
                    int oz = (m_angles[0] + 45.0f) / 90.0f;
                    m_angles[0] = oz * 90.0f;
                    m_traversedObject = e;
                    updateTransform();
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
int Sector_AllowTraverse(struct RoomSector *rs, btScalar floor, const std::shared_ptr<EngineContainer>& container)
{
    btScalar f0 = rs->floor_corners[0][2];
    if((rs->floor_corners[0][2] != f0) || (rs->floor_corners[1][2] != f0) ||
       (rs->floor_corners[2][2] != f0) || (rs->floor_corners[3][2] != f0))
    {
        return 0x00;
    }

    if((std::abs(floor - f0) < 1.1) && (rs->ceiling - rs->floor >= TR_METERING_SECTORSIZE))
    {
        return 0x01;
    }

    BtEngineClosestRayResultCallback cb(container);
    btVector3 from, to;
    to[0] = from[0] = rs->pos[0];
    to[1] = from[1] = rs->pos[1];
    from[2] = floor + TR_METERING_SECTORSIZE * 0.5f;
    to[2] = floor - TR_METERING_SECTORSIZE * 0.5f;
    bt_engine_dynamicsWorld->rayTest(from, to, cb);
    if(cb.hasHit())
    {
        btVector3 v;
        v.setInterpolate3(from, to, cb.m_closestHitFraction);
        if(std::abs(v[2] - floor) < 1.1)
        {
            EngineContainer* cont = static_cast<EngineContainer*>(cb.m_collisionObject->getUserPointer());
            if((cont != nullptr) && (cont->object_type == OBJECT_ENTITY) && ((static_cast<Entity*>(cont->object))->m_typeFlags & ENTITY_TYPE_TRAVERSE_FLOOR))
            {
                return 0x01;
            }
        }
    }

    return 0x00;
}

/**
 *
 * @param obj Traversed object pointer
 * @see TraverseNone TraverseForward TraverseBackward
 */
int Character::checkTraverse(const Entity& obj)
{
    RoomSector* ch_s = m_self->room->getSectorRaw(m_transform.getOrigin());
    RoomSector* obj_s = obj.m_self->room->getSectorRaw(obj.m_transform.getOrigin());

    if(obj_s == ch_s)
    {
        if(m_transform.getBasis().getColumn(1)[0] > 0.8)
        {
            ch_s = obj_s->owner_room->getSectorRaw({ static_cast<btScalar>(obj_s->pos[0] - TR_METERING_SECTORSIZE), static_cast<btScalar>(obj_s->pos[1]), static_cast<btScalar>(0.0) });
        }
        else if(m_transform.getBasis().getColumn(1)[0] < -0.8)
        {
            ch_s = obj_s->owner_room->getSectorRaw({ static_cast<btScalar>(obj_s->pos[0] + TR_METERING_SECTORSIZE), static_cast<btScalar>(obj_s->pos[1]), static_cast<btScalar>(0.0) });
        }
        // OY move case
        else if(m_transform.getBasis().getColumn(1)[1] > 0.8)
        {
            ch_s = obj_s->owner_room->getSectorRaw({ static_cast<btScalar>(obj_s->pos[0]), static_cast<btScalar>(obj_s->pos[1] - TR_METERING_SECTORSIZE), static_cast<btScalar>(0.0) });
        }
        else if(m_transform.getBasis().getColumn(1)[1] < -0.8)
        {
            ch_s = obj_s->owner_room->getSectorRaw({ static_cast<btScalar>(obj_s->pos[0]), static_cast<btScalar>(obj_s->pos[1] + TR_METERING_SECTORSIZE), static_cast<btScalar>(0.0) });
        }
        ch_s = ch_s->checkPortalPointer();
    }

    if((ch_s == nullptr) || (obj_s == nullptr))
    {
        return TraverseNone;
    }

    btScalar floor = m_transform.getOrigin()[2];
    if((ch_s->floor != obj_s->floor) || (Sector_AllowTraverse(ch_s, floor, m_self) == 0x00) || (Sector_AllowTraverse(obj_s, floor, obj.m_self) == 0x00))
    {
        return TraverseNone;
    }

    BtEngineClosestRayResultCallback cb(obj.m_self);
    btVector3 v0, v1;
    v1[0] = v0[0] = obj_s->pos[0];
    v1[1] = v0[1] = obj_s->pos[1];
    v0[2] = floor + TR_METERING_SECTORSIZE * 0.5f;
    v1[2] = floor + TR_METERING_SECTORSIZE * 2.5f;
    bt_engine_dynamicsWorld->rayTest(v0, v1, cb);
    if(cb.hasHit())
    {
        EngineContainer* cont = static_cast<EngineContainer*>(cb.m_collisionObject->getUserPointer());
        if((cont != nullptr) && (cont->object_type == OBJECT_ENTITY) && ((static_cast<Entity*>(cont->object))->m_typeFlags & ENTITY_TYPE_TRAVERSE))
        {
            return TraverseNone;
        }
    }

    int ret = TraverseNone;
    RoomSector* next_s = nullptr;

    /*
     * PUSH MOVE CHECK
     */
     // OX move case
    if(m_transform.getBasis().getColumn(1)[0] > 0.8)
    {
        next_s = obj_s->owner_room->getSectorRaw({ static_cast<btScalar>(obj_s->pos[0] + TR_METERING_SECTORSIZE), static_cast<btScalar>(obj_s->pos[1]), static_cast<btScalar>(0.0) });
    }
    else if(m_transform.getBasis().getColumn(1)[0] < -0.8)
    {
        next_s = obj_s->owner_room->getSectorRaw({ static_cast<btScalar>(obj_s->pos[0] - TR_METERING_SECTORSIZE), static_cast<btScalar>(obj_s->pos[1]), static_cast<btScalar>(0.0) });
    }
    // OY move case
    else if(m_transform.getBasis().getColumn(1)[1] > 0.8)
    {
        next_s = obj_s->owner_room->getSectorRaw({ static_cast<btScalar>(obj_s->pos[0]), static_cast<btScalar>(obj_s->pos[1] + TR_METERING_SECTORSIZE), static_cast<btScalar>(0.0) });
    }
    else if(m_transform.getBasis().getColumn(1)[1] < -0.8)
    {
        next_s = obj_s->owner_room->getSectorRaw({ static_cast<btScalar>(obj_s->pos[0]), static_cast<btScalar>(obj_s->pos[1] - TR_METERING_SECTORSIZE), static_cast<btScalar>(0.0) });
    }

    if(next_s)
        next_s = next_s->checkPortalPointer();

    if((next_s != nullptr) && (Sector_AllowTraverse(next_s, floor, m_self) == 0x01))
    {
        btTransform from;
        from.setIdentity();
        from.setOrigin(btVector3(obj_s->pos[0], obj_s->pos[1], floor + 0.5 * TR_METERING_SECTORSIZE));

        btTransform to;
        to.setIdentity();
        to.setOrigin(btVector3(next_s->pos[0], next_s->pos[1], floor + 0.5 * TR_METERING_SECTORSIZE));

        btSphereShape sp(COLLISION_TRAVERSE_TEST_RADIUS * TR_METERING_SECTORSIZE);
        sp.setMargin(COLLISION_MARGIN_DEFAULT);
        BtEngineClosestConvexResultCallback ccb(obj.m_self);
        bt_engine_dynamicsWorld->convexSweepTest(&sp, from, to, ccb);

        if(!ccb.hasHit())
        {
            ret |= TraverseForward;
        }
    }

    /*
     * PULL MOVE CHECK
     */
    next_s = nullptr;
    // OX move case
    if(m_transform.getBasis().getColumn(1)[0] > 0.8)
    {
        next_s = ch_s->owner_room->getSectorRaw({ static_cast<btScalar>(ch_s->pos[0] - TR_METERING_SECTORSIZE), static_cast<btScalar>(ch_s->pos[1]), static_cast<btScalar>(0.0) });
    }
    else if(m_transform.getBasis().getColumn(1)[0] < -0.8)
    {
        next_s = ch_s->owner_room->getSectorRaw({ static_cast<btScalar>(ch_s->pos[0] + TR_METERING_SECTORSIZE), static_cast<btScalar>(ch_s->pos[1]), static_cast<btScalar>(0.0) });
    }
    // OY move case
    else if(m_transform.getBasis().getColumn(1)[1] > 0.8)
    {
        next_s = ch_s->owner_room->getSectorRaw({ static_cast<btScalar>(ch_s->pos[0]), static_cast<btScalar>(ch_s->pos[1] - TR_METERING_SECTORSIZE), static_cast<btScalar>(0.0) });
    }
    else if(m_transform.getBasis().getColumn(1)[1] < -0.8)
    {
        next_s = ch_s->owner_room->getSectorRaw({ static_cast<btScalar>(ch_s->pos[0]), static_cast<btScalar>(ch_s->pos[1] + TR_METERING_SECTORSIZE), static_cast<btScalar>(0.0) });
    }

    if(next_s)
        next_s = next_s->checkPortalPointer();

    if((next_s != nullptr) && (Sector_AllowTraverse(next_s, floor, m_self) == 0x01))
    {
        btTransform from;
        from.setIdentity();
        from.setOrigin(btVector3(ch_s->pos[0], ch_s->pos[1], floor + 0.5f * TR_METERING_SECTORSIZE));

        btTransform to;
        to.setIdentity();
        to.setOrigin(btVector3(next_s->pos[0], next_s->pos[1], floor + 0.5f * TR_METERING_SECTORSIZE));

        btSphereShape sp(COLLISION_TRAVERSE_TEST_RADIUS * TR_METERING_SECTORSIZE);
        sp.setMargin(COLLISION_MARGIN_DEFAULT);
        BtEngineClosestConvexResultCallback ccb(m_self);
        bt_engine_dynamicsWorld->convexSweepTest(&sp, from, to, ccb);

        if(!ccb.hasHit())
        {
            ret |= TraverseBackward;
        }
    }

    return ret;
}

/**
 * Main character frame function
 */
void Character::applyCommands()
{
    if(m_typeFlags & ENTITY_TYPE_DYNAMIC)
    {
        return;
    }

    updatePlatformPreStep();

    if(state_func)
    {
        state_func(this, &m_bf.animations);
    }

    switch(m_moveType)
    {
        case MoveType::OnFloor:
            moveOnFloor();
            break;

        case MoveType::FreeFalling:
            freeFalling();
            break;

        case MoveType::Climbing:
            climbing();
            break;

        case MoveType::Monkeyswing:
            monkeyClimbing();
            break;

        case MoveType::WallsClimb:
            wallsClimbing();
            break;

        case MoveType::Underwater:
            moveUnderWater();
            break;

        case MoveType::OnWater:
            moveOnWater();
            break;

        default:
            m_moveType = MoveType::OnFloor;
            break;
    };

    m_command.rot.setZero();

    updateRigidBody(true);
    updatePlatformPostStep();
}

void Character::updateParams()
{
    // Poisoning is always global

    float poison = getParam(PARAM_POISON);

    if(poison)
    {
        changeParam(PARAM_POISON, 0.0001f);
        changeParam(PARAM_HEALTH, -poison);
    }

    switch(m_moveType)
    {
        case MoveType::OnFloor:
        case MoveType::FreeFalling:
        case MoveType::Climbing:
        case MoveType::Monkeyswing:
        case MoveType::WallsClimb:

            if(m_heightInfo.quicksand == QuicksandPosition::Drowning &&
               m_moveType == MoveType::OnFloor)
            {
                if(!changeParam(PARAM_AIR, -3.0))
                    changeParam(PARAM_HEALTH, -3.0);
            }
            else if(m_heightInfo.quicksand == QuicksandPosition::Sinking)
            {
                changeParam(PARAM_AIR, 3.0);
            }
            else
            {
                setParam(PARAM_AIR, PARAM_ABSOLUTE_MAX);
            }

            if((m_bf.animations.last_state == TR_STATE_LARA_SPRINT) ||
               (m_bf.animations.last_state == TR_STATE_LARA_SPRINT_ROLL))
            {
                changeParam(PARAM_STAMINA, -0.5);
            }
            else
            {
                changeParam(PARAM_STAMINA, 0.5);
            }
            break;

        case MoveType::OnWater:
            changeParam(PARAM_AIR, 3.0);;
            break;

        case MoveType::Underwater:
            if(!changeParam(PARAM_AIR, -1.0))
            {
                if(!changeParam(PARAM_HEALTH, -3.0))
                {
                    m_response.killed = true;
                }
            }
            break;

        default:
            break;  // Add quicksand later...
    }
}

bool IsCharacter(std::shared_ptr<Entity> ent)
{
    return std::dynamic_pointer_cast<Character>(ent) != nullptr;
}

int Character::setParamMaximum(int parameter, float max_value)
{
    if(parameter >= PARAM_SENTINEL)
        return 0;

    max_value = (max_value < 0) ? (0) : (max_value);    // Clamp max. to at least zero
    m_parameters.maximum[parameter] = max_value;
    return 1;
}

int Character::setParam(int parameter, float value)
{
    if(parameter >= PARAM_SENTINEL)
        return 0;

    if(value == m_parameters.param[parameter])
        return 0;

    float maximum = m_parameters.maximum[parameter];

    value = (value >= 0) ? (value) : (maximum); // Char params can't be less than zero.
    value = (value <= maximum) ? (value) : (maximum);

    m_parameters.param[parameter] = value;
    return 1;
}

float Character::getParam(int parameter)
{
    if(parameter >= PARAM_SENTINEL)
        return 0;

    return m_parameters.param[parameter];
}

int Character::changeParam(int parameter, float value)
{
    if(parameter >= PARAM_SENTINEL)
        return 0;

    float maximum = m_parameters.maximum[parameter];
    float current = m_parameters.param[parameter];

    if((current == maximum) && (value > 0))
        return 0;

    current += value;

    if(current < 0)
    {
        m_parameters.param[parameter] = 0;
        return 0;
    }
    else if(current > maximum)
    {
        m_parameters.param[parameter] = m_parameters.maximum[parameter];
    }
    else
    {
        m_parameters.param[parameter] = current;
    }

    return 1;
}

// overrided == 0x00: no overriding;
// overrided == 0x01: overriding mesh in armed state;
// overrided == 0x02: add mesh to slot in armed state;
// overrided == 0x03: overriding mesh in disarmed state;
// overrided == 0x04: add mesh to slot in disarmed state;
///@TODO: separate mesh replacing control and animation disabling / enabling
int Character::setWeaponModel(int weapon_model, int armed)
{
    SkeletalModel* sm = engine_world.getModelByID(weapon_model);

    if((sm != nullptr) && (m_bf.bone_tags.size() == sm->mesh_count) && (sm->animations.size() >= 4))
    {
        SkeletalModel* bm = m_bf.animations.model;
        if(m_bf.animations.next == nullptr)
        {
            addOverrideAnim(weapon_model);
        }
        else
        {
            m_bf.animations.next->model = sm;
        }

        for(int i = 0; i < bm->mesh_count; i++)
        {
            m_bf.bone_tags[i].mesh_base = bm->mesh_tree[i].mesh_base;
            m_bf.bone_tags[i].mesh_slot = nullptr;
        }

        if(armed != 0)
        {
            for(int i = 0; i < bm->mesh_count; i++)
            {
                if(sm->mesh_tree[i].replace_mesh == 0x01)
                {
                    m_bf.bone_tags[i].mesh_base = sm->mesh_tree[i].mesh_base;
                }
                else if(sm->mesh_tree[i].replace_mesh == 0x02)
                {
                    m_bf.bone_tags[i].mesh_slot = sm->mesh_tree[i].mesh_base;
                }
            }
        }
        else
        {
            for(int i = 0; i < bm->mesh_count; i++)
            {
                if(sm->mesh_tree[i].replace_mesh == 0x03)
                {
                    m_bf.bone_tags[i].mesh_base = sm->mesh_tree[i].mesh_base;
                }
                else if(sm->mesh_tree[i].replace_mesh == 0x04)
                {
                    m_bf.bone_tags[i].mesh_slot = sm->mesh_tree[i].mesh_base;
                }
            }
            m_bf.animations.next->model = nullptr;
        }

        return 1;
    }
    else
    {
        // do unarmed default model
        SkeletalModel* bm = m_bf.animations.model;
        for(int i = 0; i < bm->mesh_count; i++)
        {
            m_bf.bone_tags[i].mesh_base = bm->mesh_tree[i].mesh_base;
            m_bf.bone_tags[i].mesh_slot = nullptr;
        }
        if(m_bf.animations.next != nullptr)
        {
            m_bf.animations.next->model = nullptr;
        }
    }

    return 0;
}

void Character::fixPenetrations(const btVector3* move)
{
    if(m_bt.ghostObjects.empty())
        return;

    if(move != nullptr)
    {
        m_response.horizontal_collide = 0x00;
        m_response.vertical_collide = 0x00;
    }

    if(m_typeFlags & ENTITY_TYPE_DYNAMIC)
    {
        return;
    }

    if(m_bt.no_fix_all)
    {
        ghostUpdate();
        return;
    }

    btVector3 reaction;
    int numPenetrationLoops = getPenetrationFixVector(&reaction, move != nullptr);
    m_transform.getOrigin() += reaction;

    updateCurrentHeight();
    if((move != nullptr) && (numPenetrationLoops > 0))
    {
        btScalar t1 = reaction[0] * reaction[0] + reaction[1] * reaction[1];
        btScalar t2 = move->x() * move->x() + move->y() * move->y();
        if((reaction[2] * reaction[2] < t1) && (move->z() * move->z() < t2))    // we have horizontal move and horizontal correction
        {
            t2 *= t1;
            t1 = (reaction[0] * move->x() + reaction[1] * move->y()) / sqrtf(t2);
            if(t1 < m_criticalWallComponent)
            {
                m_response.horizontal_collide |= 0x01;
            }
        }
        else if((reaction[2] * reaction[2] > t1) && (move->z() * move->z() > t2))
        {
            if((reaction[2] > 0.0) && (move->z() < 0.0))
            {
                m_response.vertical_collide |= 0x01;
            }
            else if((reaction[2] < 0.0) && (move->z() > 0.0))
            {
                m_response.vertical_collide |= 0x02;
            }
        }
    }

    if(m_heightInfo.ceiling_hit && (reaction[2] < -0.1))
    {
        m_response.vertical_collide |= 0x02;
    }

    if(m_heightInfo.floor_hit && (reaction[2] > 0.1))
    {
        m_response.vertical_collide |= 0x01;
    }

    ghostUpdate();
}

/**
 * we check walls and other collision objects reaction. if reaction more than critical
 * then cmd->horizontal_collide |= 0x01;
 * @param move absolute 3d move vector
 */
int Character::checkNextPenetration(const btVector3& move)
{
    if(m_bt.ghostObjects.empty())
        return 0;

    ghostUpdate();
    m_transform.getOrigin() += move;
    //resp->horizontal_collide = 0x00;
    btVector3 reaction;
    int ret = getPenetrationFixVector(&reaction, true);
    if(ret > 0)
    {
        btScalar t1 = reaction[0] * reaction[0] + reaction[1] * reaction[1];
        btScalar t2 = move[0] * move[0] + move[1] * move[1];
        if((reaction[2] * reaction[2] < t1) && (move[2] * move[2] < t2))
        {
            t2 *= t1;
            t1 = (reaction[0] * move[0] + reaction[1] * move[1]) / sqrtf(t2);
            if(t1 < m_criticalWallComponent)
            {
                m_response.horizontal_collide |= 0x01;
            }
        }
    }
    m_transform.getOrigin() -= move;
    ghostUpdate();
    cleanCollisionAllBodyParts();

    return ret;
}

void Character::updateHair()
{
    if(m_hairs.empty())
        return;

    for(std::shared_ptr<Hair> hair : m_hairs)
    {
        if(!hair || hair->m_elements.empty())
            continue;

        /*btScalar new_transform[16];

        new_transform = transform * bf.bone_tags[hair->owner_body].full_transform;

        // Calculate mixed velocities.
        btVector3 mix_vel(new_transform[12+0] - hair->owner_body_transform[12+0],
                          new_transform[12+1] - hair->owner_body_transform[12+1],
                          new_transform[12+2] - hair->owner_body_transform[12+2]);
        mix_vel *= 1.0 / engine_frame_time;

        if(0)
        {
            btScalar sub_tr[16];
            btTransform ang_tr;
            btVector3 mix_ang;
            sub_tr = hair->owner_body_transform.inverse() * new_transform;
            ang_tr.setFromOpenGLMatrix(sub_tr);
            ang_tr.getBasis().getEulerYPR(mix_ang[2], mix_ang[1], mix_ang[0]);
            mix_ang *= 1.0 / engine_frame_time;

            // Looks like angular velocity breaks up constraints on VERY fast moves,
            // like mid-air turn. Probably, I've messed up with multiplier value...

            hair->elements[hair->root_index].body->setAngularVelocity(mix_ang);
            hair->owner_char->bt_body[hair->owner_body]->setAngularVelocity(mix_ang);
        }
        Mat4_Copy(hair->owner_body_transform, new_transform);*/

        // Set mixed velocities to both parent body and first hair body.

        //hair->elements[hair->root_index].body->setLinearVelocity(mix_vel);
        //hair->owner_char->bt_body[hair->owner_body]->setLinearVelocity(mix_vel);

        /*mix_vel *= -10.0;                                                     ///@FIXME: magick speed coefficient (force air hair friction!);
        for(int j=0;j<hair->element_count;j++)
        {
            hair->elements[j].body->applyCentralForce(mix_vel);
        }*/

        if(auto ownerChar = hair->m_ownerChar.lock())
        {
            hair->m_container->room = ownerChar->m_self->room;
        }
    }
}

/**
 * Character framestep actions
 * @param time      frame time
 */
void Character::frame(btScalar time)
{
    int animStepResult = ENTITY_ANIM_NONE;

    if(!m_enabled && !isPlayer())
    {
        return;
    }
    if(m_typeFlags & ENTITY_TYPE_DYNAMIC)
    {
        // Ragdoll
        updateRigidBody(false); // bbox update, room update, m_transform from btBody...
        return;
    }
    if(isPlayer() && (control_states.noclip || control_states.free_look))
    {
        updateCurrentBoneFrame(&m_bf);
        updateRigidBody(false);     // bbox update, room update, m_transform from btBody...
        return;
    }

    fixPenetrations(nullptr);
    m_bt.no_fix_all = false;    // FIXME

    processSector();

    if(isPlayer())  // Player:
    {
        updateParams();
        checkCollisionCallbacks();  // physics collision checks, lua callbacks
    }
    else            // Other Character entities:
    {
        engine_lua.loopEntity(id());
        if(m_typeFlags & ENTITY_TYPE_COLLCHECK)
            checkCollisionCallbacks();
    }

    animStepResult = stepAnimation(time);
    if(m_bf.animations.onFrame != nullptr)
    {
        m_bf.animations.onFrame(this, &m_bf.animations, animStepResult);
    }

    applyCommands();    // state_func()

    if(m_command.action && (m_typeFlags & ENTITY_TYPE_TRIGGER_ACTIVATOR))
    {
        checkActivators();          // bbox check f. interact/pickup, lua callbacks
    }
    if(getParam(PARAM_HEALTH) <= 0.0)
    {
        m_response.killed = true;   // Kill, if no HP.
    }
    updateHair();

    doWeaponFrame(time);

    // Update acceleration/speed, it is calculated per anim frame index
    auto af = &m_bf.animations.model->animations[m_bf.animations.current_animation];
    m_currentSpeed = (af->speed_x + m_bf.animations.current_frame * af->accel_x) / (1<<16); //Decompiled from TOMB5.EXE


    // TODO: check rigidbody update requirements.
    //if(animStepResult != ENTITY_ANIM_NONE)
    //{ }
    updateCurrentBoneFrame(&m_bf);
    updateRigidBody(false);     // bbox update, room update, m_transform from btBody...
    return;
}

void Character::processSectorImpl()
{
    assert(m_currentSector != nullptr);
    RoomSector* highest_sector = m_currentSector->getHighestSector();
    assert(highest_sector != nullptr);
    RoomSector* lowest_sector = m_currentSector->getLowestSector();
    assert(lowest_sector != nullptr);

    m_heightInfo.walls_climb_dir = 0;
    m_heightInfo.walls_climb_dir |= lowest_sector->flags & (SECTOR_FLAG_CLIMB_WEST |
                                                            SECTOR_FLAG_CLIMB_EAST |
                                                            SECTOR_FLAG_CLIMB_NORTH |
                                                            SECTOR_FLAG_CLIMB_SOUTH);

    m_heightInfo.walls_climb = (m_heightInfo.walls_climb_dir > 0);
    m_heightInfo.ceiling_climb = false;

    if((highest_sector->flags & SECTOR_FLAG_CLIMB_CEILING) || (lowest_sector->flags & SECTOR_FLAG_CLIMB_CEILING))
    {
        m_heightInfo.ceiling_climb = true;
    }

    if(lowest_sector->flags & SECTOR_FLAG_DEATH)
    {
        if((m_moveType == MoveType::OnFloor) ||
           (m_moveType == MoveType::Wade)    ||
           (m_moveType == MoveType::Quicksand))
        {
            if(m_heightInfo.floor_hit)
            {
                EngineContainer* cont = static_cast<EngineContainer*>(m_heightInfo.floor_obj->getUserPointer());

                if((cont != nullptr) && (cont->object_type == OBJECT_ROOM_BASE))
                {
                    setParam(PARAM_HEALTH, 0.0);
                    m_response.killed = true;
                }
            }
        }
        else if((m_moveType == MoveType::Underwater) ||
                (m_moveType == MoveType::OnWater))
        {
            setParam(PARAM_HEALTH, 0.0);
            m_response.killed = true;
        }
    }
}

void Character::jump(btScalar v_vertical, btScalar v_horizontal)
{
    btScalar t;
    btVector3 spd(0.0, 0.0, 0.0);

    // Jump length is a speed value multiplied by global speed coefficient.
    t = v_horizontal * m_speedMult;

    // Calculate the direction of jump by vector multiplication.
    if(m_dirFlag & ENT_MOVE_FORWARD)
    {
        spd = m_transform.getBasis().getColumn(1) * t;
    }
    else if(m_dirFlag & ENT_MOVE_BACKWARD)
    {
        spd = m_transform.getBasis().getColumn(1) * -t;
    }
    else if(m_dirFlag & ENT_MOVE_LEFT)
    {
        spd = m_transform.getBasis().getColumn(0) * -t;
    }
    else if(m_dirFlag & ENT_MOVE_RIGHT)
    {
        spd = m_transform.getBasis().getColumn(0) * t;
    }
    else
    {
        m_dirFlag = ENT_MOVE_FORWARD;
    }

    m_response.vertical_collide = 0x00;

    m_response.slide = SlideType::None;
    m_response.lean  = LeanType::None;

    // Jump speed should NOT be added to current speed, as native engine
    // fully replaces current speed with jump speed by anim command.
    m_speed = spd;

    // Apply vertical speed.
    m_speed[2] = v_vertical * m_speedMult;
    m_moveType = MoveType::FreeFalling;
}

Substance Character::getSubstanceState() const
{
    if(m_self->room->flags & TR_ROOM_FLAG_QUICKSAND)
    {
        if(m_heightInfo.transition_level > m_transform.getOrigin()[2] + m_height)
        {
            return Substance::QuicksandConsumed;
        }
        else
        {
            return Substance::QuicksandShallow;
        }
    }
    else if(!m_heightInfo.water)
    {
        return Substance::None;
    }
    else if(m_heightInfo.water &&
            (m_heightInfo.transition_level > m_transform.getOrigin()[2]) &&
            (m_heightInfo.transition_level < m_transform.getOrigin()[2] + m_wadeDepth))
    {
        return Substance::WaterShallow;
    }
    else if(m_heightInfo.water &&
            (m_heightInfo.transition_level > m_transform.getOrigin()[2] + m_wadeDepth))
    {
        return Substance::WaterWade;
    }
    else
    {
        return Substance::WaterSwim;
    }
}

void Character::updateGhostRigidBody()
{
    if(!m_bt.ghostObjects.empty())
    {
        assert(m_bf.bone_tags.size() == m_bt.ghostObjects.size());
        for(size_t i = 0; i < m_bf.bone_tags.size(); i++)
        {
            auto tr = m_bt.bt_body[i]->getWorldTransform();
            tr.setOrigin(tr * m_bf.bone_tags[i].mesh_base->m_center);
            m_bt.ghostObjects[i]->getWorldTransform() = tr;
        }
    }
}

btVector3 Character::camPosForFollowing(btScalar dz)
{
    if(m_camFollowCenter > 0)
    {
        m_camFollowCenter--;
        return m_obb.centre;
    }
    return Entity::camPosForFollowing(dz);
}

/* There are stick code for multianimation (weapon mode) testing
 * Model replacing will be upgraded too, I have to add override
 * flags to model manually in the script*/
void Character::doWeaponFrame(btScalar time)
{
    /* anims (TR1 - TR5):
     * pistols:
     * 0: idle to fire;
     * 1: draw weapon (short?);
     * 2: draw weapon (full);
     * 3: fire process;
     *
     * shotgun, rifles, crossbow, harpoon, launchers (2 handed weapons):
     * 0: idle to fire;
     * 1: draw weapon;
     * 2: fire process;
     * 3: hide weapon;
     * 4: idle to fire (targeted);
     */
    if(m_command.ready_weapon && (m_currentWeapon > 0) && (m_weaponCurrentState == WeaponState::Hide))
    {
        setWeaponModel(m_currentWeapon, 1);
    }

    for(SSAnimation* ss_anim = m_bf.animations.next; ss_anim != nullptr; ss_anim = ss_anim->next)
    {
        if((ss_anim->model != nullptr) && (ss_anim->model->animations.size() > 4))
        {
            // fixme: set weapon combat flag depending on specific weapon versions (pistols, uzi, revolver)
            ss_anim->anim_flags = ANIM_NORMAL_CONTROL;
            switch(m_weaponCurrentState)
            {
                case WeaponState::Hide:
                    if(m_command.ready_weapon)   // ready weapon
                    {
                        ss_anim->setAnimation(1);  // draw from holster
                        // fixme: reset lerp:
                        ss_anim->lerp_last_animation = ss_anim->current_animation;
                        ss_anim->lerp_last_frame = ss_anim->current_frame;
                        m_weaponCurrentState = WeaponState::HideToReady;
                    }
                    break;

                case WeaponState::HideToReady:
                    if(ss_anim->stepAnimation(time, this) == ENTITY_ANIM_NEWANIM)
                    {
                        ss_anim->setAnimation(0);  // hold drawn weapon to aim at target transition
                        m_weaponCurrentState = WeaponState::Idle;
                    }
                    break;

                case WeaponState::Idle:
                    if(m_command.ready_weapon)
                    {
                        ss_anim->setAnimation(3);  // holster weapon
                        m_weaponCurrentState = WeaponState::IdleToHide;
                    }
                    else if(m_command.action)
                    {
                        m_weaponCurrentState = WeaponState::IdleToFire;
                    }
                    else
                    {
                        // stay at frame 0
                        ss_anim->setAnimation(0);  // hold drawn weapon to aim at target transition
                    }
                    break;

                case WeaponState::FireToIdle:
                    // reverse stepping:
                    // (there is a separate animation (4) for this, hence the original shotgun/bow don't reverse mid-anim)
                    if(ss_anim->stepAnimation(-time, this) == ENTITY_ANIM_NEWANIM)
                    {
                        ss_anim->setAnimation(0);  // hold drawn weapon to aim at target transition
                        m_weaponCurrentState = WeaponState::Idle;
                    }
                    break;

                case WeaponState::IdleToFire:
                    if(m_command.action) {
                        if(ss_anim->stepAnimation(time, this) == ENTITY_ANIM_NEWANIM )
                        {
                            ss_anim->setAnimation(2);  // shooting cycle
                            m_weaponCurrentState = WeaponState::Fire;
                        }
                    } else {
                        m_weaponCurrentState = WeaponState::FireToIdle;
                    }
                    break;

                case WeaponState::Fire:
                    if(m_command.action)
                    {
                        if(ss_anim->stepAnimation(time, this) == ENTITY_ANIM_NEWANIM )
                        {
                            ss_anim->setAnimation(2);  // shooting cycle
                            // bang
                        }
                    }
                    else
                    {
                        ss_anim->setAnimation(0,-1);  // hold drawn weapon to aim at target transition
                        m_weaponCurrentState = WeaponState::FireToIdle;
                    }
                    break;

                case WeaponState::IdleToHide:
                    if( ss_anim->stepAnimation(time, this) == ENTITY_ANIM_NEWANIM)
                    {
                        m_weaponCurrentState = WeaponState::Hide;
                        setWeaponModel(m_currentWeapon, 0);
                    }
                    break;
            };
        }
        else if((ss_anim->model != nullptr) && (ss_anim->model->animations.size() == 4))
        {
            // fixme: set weapon combat flag depending on specific weapon versions (pistols, uzi, revolver)
            ss_anim->anim_flags = ANIM_WEAPON_COMPAT;
            switch(m_weaponCurrentState)
            {
                case WeaponState::Hide:
                    if(m_command.ready_weapon)   // ready weapon
                    {
                        ss_anim->setAnimation(2);  // draw from holster
                        // fixme: reset lerp:
                        ss_anim->lerp_last_animation = ss_anim->current_animation;
                        ss_anim->lerp_last_frame = ss_anim->current_frame;
                        m_weaponCurrentState = WeaponState::HideToReady;
                    }
                    break;

                case WeaponState::HideToReady:
                    if(ss_anim->stepAnimation(time, this) == ENTITY_ANIM_NEWANIM)
                    {
                        ss_anim->setAnimation(0);  // hold drawn weapon to aim at target transition
                        m_weaponCurrentState = WeaponState::Idle;
                    }
                    break;

                case WeaponState::Idle:
                    if(m_command.ready_weapon)
                    {
                        ss_anim->setAnimation(2,-1);  // draw weapon, end for reverse
                        m_weaponCurrentState = WeaponState::IdleToHide;
                    }
                    else if(m_command.action)
                    {
                        m_weaponCurrentState = WeaponState::IdleToFire;
                    }
                    else
                    {
                        // stay
                        ss_anim->setAnimation(0);  // hold drawn weapon to aim at target transition
                    }
                    break;

                case WeaponState::FireToIdle:
                    // reverse stepping:
                    if(ss_anim->stepAnimation(-time, this) == ENTITY_ANIM_NEWANIM)
                    {
                        ss_anim->setAnimation(0);  // hold drawn weapon to aim at target transition
                        m_weaponCurrentState = WeaponState::Idle;
                    }
                    break;

                case WeaponState::IdleToFire:
                    if(m_command.action) {
                        if(ss_anim->stepAnimation(time, this) == ENTITY_ANIM_NEWANIM )
                        {
                            ss_anim->setAnimation(3);  // shooting cycle
                            m_weaponCurrentState = WeaponState::Fire;
                        }
                    } else {
                        m_weaponCurrentState = WeaponState::FireToIdle;
                    }
                    break;

                case WeaponState::Fire:
                    if(m_command.action)
                    {
                        if(ss_anim->stepAnimation(time, this) == ENTITY_ANIM_NEWANIM )
                        {
                            ss_anim->setAnimation(3);  // shooting cycle
                            // bang
                        }
                    }
                    else
                    {
                        ss_anim->setAnimation(0,-1);  // hold drawn weapon to aim at target transition
                        m_weaponCurrentState = WeaponState::FireToIdle;
                    }
                    break;

                case WeaponState::IdleToHide:
                    // reverse stepping:
                    if( ss_anim->stepAnimation(-time, this) == ENTITY_ANIM_NEWANIM)
                    {
                        m_weaponCurrentState = WeaponState::Hide;
                        setWeaponModel(m_currentWeapon, 0);
                    }
                    break;
            };
        }
    }
}
