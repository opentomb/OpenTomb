
#include "world.h"

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/NarrowPhaseCollision/btRaycastCallback.h>

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
#include "strings.h"

#define LEFT_LEG                    (3)
#define RIGHT_LEG                   (6)

#define PENETRATION_TEST_OFFSET     (48.0)        ///@TODO: tune it!
#define WALK_FORWARD_OFFSET         (96.0)        ///@FIXME: find real offset
#define WALK_BACK_OFFSET            (16.0)
#define WALK_FORWARD_STEP_UP        (256.0)       // by bone frame bb
#define RUN_FORWARD_OFFSET          (128.0)       ///@FIXME: find real offset
#define RUN_FORWARD_STEP_UP         (320.0)       // by bone frame bb
#define CRAWL_FORWARD_OFFSET        (256.0)
#define LARA_HANG_WALL_DISTANCE     (128.0 - 24.0)
#define LARA_HANG_VERTICAL_EPSILON  (64.0)
#define LARA_HANG_VERTICAL_OFFSET   (12.0)        // in original is 0, in real life hands are little more higher than edge
#define LARA_TRY_HANG_WALL_OFFSET   (72.0)        // It works more stable than 32 or 128
#define LARA_HANG_SENSOR_Z          (800.0)       // It works more stable than 1024 (after collision critical fix, of course)

#define OSCILLATE_HANG_USE 0

Character::Character(uint32_t id)
    : Entity(id)
{
    m_sphere->setMargin(COLLISION_MARGIN_DEFAULT);

    m_climbSensor.setMargin(COLLISION_MARGIN_DEFAULT);

    m_rayCb = std::make_shared<BtEngineClosestRayResultCallback>(m_self, true);
    m_rayCb->m_collisionFilterMask = btBroadphaseProxy::StaticFilter | btBroadphaseProxy::KinematicFilter;
    m_heightInfo.cb = m_rayCb;

    m_convexCb = std::make_shared<BtEngineClosestConvexResultCallback>(m_self, true);
    m_convexCb->m_collisionFilterMask = btBroadphaseProxy::StaticFilter | btBroadphaseProxy::KinematicFilter;
    m_heightInfo.ccb = m_convexCb;

    m_dirFlag = ENT_STAY;
}

Character::~Character() {
    if((m_self->room != NULL) && (this != engine_world.character.get()))
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

    for(InventoryNode& i : m_inventory) {
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
    return ret;
}


int32_t Character::getItemsCount(uint32_t item_id)         // returns items count
{
    for (const auto& item : m_inventory)
    {
        if (item.id == item_id)
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
    if(platform)
    {
        EngineContainer* cont = (EngineContainer*)platform->getUserPointer();
        if(cont && (cont->object_type == OBJECT_ENTITY/* || cont->object_type == OBJECT_BULLET_MISC*/))
        {
            btScalar trpl[16];
            platform->getWorldTransform().getOpenGLMatrix(trpl);
#if 0
            new_tr = trpl * local_platform;
            vec3_copy(transform.getOrigin(), new_tr + 12);
#else
            ///make something with platform rotation
            transform = trpl * local_platform;
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
        case MOVE_ON_FLOOR:
            if(height_info.floor_hit)
            {
                platform = height_info.floor_obj;
            }
            break;

        case MOVE_CLIMBING:
            if(climb.edge_hit != ClimbInfo::NoClimb)
            {
                platform = climb.edge_obj;
            }
            break;

        default:
            platform = NULL;
            break;
    };

    if(platform)
    {
        EngineContainer* cont = (EngineContainer*)platform->getUserPointer();
        if(cont && (cont->object_type == OBJECT_ENTITY/* || cont->object_type == OBJECT_BULLET_MISC*/))
        {
            btScalar trpl[16];
            platform->getWorldTransform().getOpenGLMatrix(trpl);
            /* local_platform = (global_platform ^ -1) x (global_entity); */
            local_platform = trpl.inverse() * transform;
        }
        else
        {
            platform = NULL;
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
    Room* r = (cb->m_container)?(cb->m_container->room):(NULL);
    RoomSector* rs;

    fc->floor_hit = false;
    fc->ceiling_hit = false;
    fc->water = false;
    fc->quicksand = 0x00;
    fc->transition_level = 32512.0;

    r = Room_FindPosCogerrence(pos, r);
    if(r)
        r = r->checkFlip();
    if(r)
    {
        rs = r->getSectorXYZ(pos);                                         // if r != NULL then rs can not been NULL!!!
        if(r->flags & TR_ROOM_FLAG_WATER)                                       // in water - go up
        {
            while(rs->sector_above)
            {
                assert( rs->sector_above != nullptr );
                rs = rs->sector_above->checkFlip();
                assert( rs != nullptr && rs->owner_room != nullptr );
                if((rs->owner_room->flags & TR_ROOM_FLAG_WATER) == 0x00)        // find air
                {
                    fc->transition_level = (btScalar)rs->floor;
                    fc->water = true;
                    break;
                }
            }
        }
        else if(r->flags & TR_ROOM_FLAG_QUICKSAND)
        {
            while(rs->sector_above)
            {
                assert( rs->sector_above != nullptr );
                rs = rs->sector_above->checkFlip();
                assert( rs != nullptr && rs->owner_room != nullptr );
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
                assert( rs->sector_below != nullptr );
                rs = rs->sector_below->checkFlip();
                assert( rs != nullptr && rs->owner_room != nullptr );
                if((rs->owner_room->flags & TR_ROOM_FLAG_WATER) != 0x00)        // find water
                {
                    fc->transition_level = (btScalar)rs->ceiling;
                    fc->water = true;
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
    from = pos;
    to = from;
    to[2] -= 4096.0;
    cb->m_closestHitFraction = 1.0;
    cb->m_collisionObject = NULL;
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
    cb->m_collisionObject = NULL;
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
NextStepInfo Character::checkNextStep(const btVector3& offset, struct HeightInfo *nfc)
{
    btScalar delta;
    HeightInfo* fc = &m_heightInfo;
    btVector3 from, to;
    NextStepInfo ret = NextStepInfo::Horizontal;
    ///penetration test?

    auto pos = m_transform.getOrigin() + offset;
    Character::getHeightInfo(pos, nfc);

    if(fc->floor_hit && nfc->floor_hit)
    {
        delta = nfc->floor_point[2] - fc->floor_point[2];
        if(std::abs(delta) < SPLIT_EPSILON)
        {
            from[2] = fc->floor_point[2];
            ret = NextStepInfo::Horizontal;                                    // horizontal
        }
        else if(delta < 0.0)                                                    // down way
        {
            delta = -delta;
            from[2] = fc->floor_point[2];
            if(delta <= m_minStepUpHeight)
            {
                ret = NextStepInfo::DownLittle;
            }
            else if(delta <= m_maxStepUpHeight)
            {
                ret = NextStepInfo::DownBig;
            }
            else if(delta <= m_height)
            {
                ret = NextStepInfo::DownDrop;
            }
            else
            {
                ret = NextStepInfo::DownCanHang;
            }
        }
        else                                                                    // up way
        {
            from[2] = nfc->floor_point[2];
            if(delta <= m_minStepUpHeight)
            {
                ret = NextStepInfo::UpLittle;
            }
            else if(delta <= m_maxStepUpHeight)
            {
                ret = NextStepInfo::UpBig;
            }
            else if(delta <= m_maxClimbHeight)
            {
                ret = NextStepInfo::UpClimb;
            }
            else
            {
                ret = NextStepInfo::UpImpossible;
            }
        }
    }
    else if(!fc->floor_hit && !nfc->floor_hit)
    {
        from[2] = pos[2];
        ret = NextStepInfo::Horizontal;                                        // horizontal? yes no maybe...
    }
    else if(!fc->floor_hit && nfc->floor_hit)                                   // strange case
    {
        from[2] = nfc->floor_point[2];
        ret = NextStepInfo::Horizontal;
    }
    else //if(fc->floor_hit && !nfc->floor_hit)                                 // bottomless
    {
        from[2] = fc->floor_point[2];
        ret = NextStepInfo::DownCanHang;
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
    fc->cb->m_closestHitFraction = 1.0;
    fc->cb->m_collisionObject = NULL;
    bt_engine_dynamicsWorld->rayTest(from, to, *fc->cb);
    if(fc->cb->hasHit())
    {
        ret = NextStepInfo::UpImpossible;
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
    const btVector3 entityPos = m_transform.getOrigin();
    extern GLfloat cast_ray[6];                                                 // pointer to the test line coordinates
    /*
     * init callbacks functions
     */
    nfc->cb = m_rayCb;
    nfc->ccb = m_convexCb;
    btVector3 to = entityPos + offset; // native offset point

    ClimbInfo result;
    result.height_info = checkNextStep(offset + btVector3{0, 0, 128.0}, nfc);
    result.floor_limit = m_heightInfo.floor_hit
                       ? m_heightInfo.floor_point[2]
                       : -9E10;
    result.ceiling_limit = m_heightInfo.ceiling_hit
                         ? m_heightInfo.ceiling_point[2]
                         : 9E10;

    if(nfc->ceiling_hit && (nfc->ceiling_point[2] < result.ceiling_limit))
    {
        result.ceiling_limit = nfc->ceiling_point[2];
    }

    result.point = m_climb.point;
    /*
     * check max height
     */
    if(m_heightInfo.ceiling_hit && (to[2] > m_heightInfo.ceiling_point[2] - m_climbR - 1.0))
    {
        to[2] = m_heightInfo.ceiling_point[2] - m_climbR - 1.0;
    }

    /*
    * Let us calculate EDGE
    */
    btVector3 from;
    from[0] = entityPos[0] - m_transform.getBasis().getColumn(1)[0] * m_climbR * 2.0;
    from[1] = entityPos[1] - m_transform.getBasis().getColumn(1)[1] * m_climbR * 2.0;
    from[2] = to[2];

    btTransform fromTransform;
    fromTransform.setIdentity();

    btTransform toTransform;
    toTransform.setIdentity();

    btScalar d = entityPos[2] + m_bf.bb_max[2] - std::max(test_height, m_maxStepUpHeight);
    std::copy(&to[0], &to[3], cast_ray+0);
    std::copy(&to[0], &to[3], cast_ray+3);
    cast_ray[5] -= d;
    int up_founded = 0;
    btVector3 n0{0,0,0}, n1{0,0,0};
    btScalar n0d{0}, n1d{0};
    do
    {
        fromTransform.setOrigin(from);
        toTransform.setOrigin(to);
        nfc->ccb->m_closestHitFraction = 1.0;
        nfc->ccb->m_hitCollisionObject = nullptr;
        bt_engine_dynamicsWorld->convexSweepTest(&m_climbSensor, fromTransform, toTransform, *nfc->ccb);
        if(nfc->ccb->hasHit())
        {
            if(nfc->ccb->m_hitNormalWorld[2] >= 0.1)
            {
                up_founded = 1;
                n0 = nfc->ccb->m_hitNormalWorld;
                n0d = -n0.dot(nfc->ccb->m_hitPointWorld);
            }
            if(up_founded!=0 && (nfc->ccb->m_hitNormalWorld[2] < 0.001))
            {
                n1 = nfc->ccb->m_hitNormalWorld;
                n1d = -n1.dot(nfc->ccb->m_hitPointWorld);
                m_climb.edge_obj = (btCollisionObject*)nfc->ccb->m_hitCollisionObject;
                up_founded = 2;
                break;
            }
        }
        else
        {
            fromTransform.setOrigin(to);
            toTransform.setOrigin({to[0], to[1], d});
            nfc->ccb->m_closestHitFraction = 1.0;
            nfc->ccb->m_hitCollisionObject = NULL;
            bt_engine_dynamicsWorld->convexSweepTest(&m_climbSensor, fromTransform, toTransform, *nfc->ccb);
            if(nfc->ccb->hasHit())
            {
                up_founded = 1;
                n0 = nfc->ccb->m_hitNormalWorld;
                n0d = -n0.dot(nfc->ccb->m_hitPointWorld);
            }
            else
            {
                return result;
            }
        }

        // mult 0.66 is magick, but it must be less than 1.0 and greater than 0.0;
        // close to 1.0 - bad precision, good speed;
        // close to 0.0 - bad speed, bad precision;
        // close to 0.5 - middle speed, good precision
        from[2] -= 0.66 * m_climbR;
        to[2] -= 0.66 * m_climbR;
    } while(to[2] >= d);                                                 // we can't climb under floor!

    if(up_founded != 2)
    {
        return result;
    }

    // get the character plane equation
    btVector3 n2 = m_transform.getBasis().getColumn(0);
    btScalar n2d = -n2.dot(entityPos);

    assert( !n0.fuzzyZero() );
    assert( !n1.fuzzyZero() );
    assert( !n2.fuzzyZero() );
    /*
     * Solve system of the linear equations by Kramer method!
     * I know - It may be slow, but it has a good precision!
     * The root is point of 3 planes intersection.
     */
    d =-n0[0] * (n1[1] * n2[2] - n1[2] * n2[1]) +
        n1[0] * (n0[1] * n2[2] - n0[2] * n2[1]) -
        n2[0] * (n0[1] * n1[2] - n0[2] * n1[1]);

    if(std::abs(d) < 0.005)
    {
        return result;
    }

    result.edge_point[0] = n0d * (n1[1] * n2[2] - n1[2] * n2[1]) -
                           n1d * (n0[1] * n2[2] - n0[2] * n2[1]) +
                           n2d * (n0[1] * n1[2] - n0[2] * n1[1]);
    result.edge_point[0] /= d;

    result.edge_point[1] = n0[0] * (n1d * n2[2] - n1[2] * n2d) -
                           n1[0] * (n0d * n2[2] - n0[2] * n2d) +
                           n2[0] * (n0d * n1[2] - n0[2] * n1d);
    result.edge_point[1] /= d;

    result.edge_point[2] = n0[0] * (n1[1] * n2d - n1d * n2[1]) -
                           n1[0] * (n0[1] * n2d - n0d * n2[1]) +
                           n2[0] * (n0[1] * n1d - n0d * n1[1]);
    result.edge_point[2] /= d;

    result.point = result.edge_point;
    std::copy(result.point+0, result.point+3, cast_ray+3);
    /*
     * unclimbable edge slant %)
     */
    n2 = n0.cross(n1);
    d = m_criticalSlantZComponent;
    d *= d * n2.length2();
    if(n2[2] * n2[2] > d)
    {
        return result;
    }

    /*
     * Now, let us calculate z_angle
     */
    result.edge_hit = ClimbType::HandsOnly;

    // $0,$1,$2 => $1,-$0,0.0
    n2[2] = n2[0];
    n2[0] = n2[1];
    n2[1] =-n2[2];
    n2[2] = 0.0;
    if(n2[0] * m_transform.getBasis().getColumn(1)[0] + n2[1] * m_transform.getBasis().getColumn(1)[1] > 0)       // direction fixing
    {
        n2[0] = -n2[0];
        n2[1] = -n2[1];
    }

    result.n = n2;
    result.up_dir = {0,0,1};
    result.edge_z_ang = std::atan2(n2[0], -n2[1]) * DegPerRad;
    result.edge_tan_xy = {-n2[1], n2[0], 0.0};
    result.edge_tan_xy.normalize();
    result.right_dir = result.edge_tan_xy;

    if(!m_heightInfo.floor_hit || (result.edge_point[2] - m_heightInfo.floor_point[2] >= m_height))
    {
        result.can_hang = true;
    }

    result.next_z_space = 2.0 * m_height;
    if(nfc->floor_hit && nfc->ceiling_hit)
    {
        result.next_z_space = nfc->ceiling_point[2] - nfc->floor_point[2];
    }

    return result;
}


ClimbInfo Character::checkWallsClimbability()
{

    ClimbInfo result;
    if(m_heightInfo.floor_hit)
        result.floor_limit = m_heightInfo.floor_point[2];
    if(m_heightInfo.ceiling_hit)
        result.ceiling_limit = m_heightInfo.ceiling_point[2];
    result.point = m_climb.point;

    if(!m_heightInfo.walls_climb)
    {
        return result;
    }

    result.up_dir = {0,0,1};

    const btVector3 entityPosition = m_transform.getOrigin();
    btVector3 from = entityPosition + m_transform.getBasis().getColumn(2) * m_bf.bb_max[2] - m_transform.getBasis().getColumn(1) * m_climbR;
    btVector3 to = from;
    to += m_transform.getBasis().getColumn(1) * (m_forwardSize + m_bf.bb_max[1]);

    auto ccb = m_convexCb;
    ccb->m_closestHitFraction = 1.0;
    ccb->m_hitCollisionObject = nullptr;

    btTransform fromTransform;
    fromTransform.setIdentity();
    fromTransform.setOrigin(from);

    btTransform toTransform;
    toTransform.setIdentity();
    toTransform.setOrigin(to);

    bt_engine_dynamicsWorld->convexSweepTest(&m_climbSensor, fromTransform, toTransform, *ccb);
    if(!ccb->hasHit())
    {
        return result;
    }

    result.point = ccb->m_hitPointWorld;
    result.n = ccb->m_hitNormalWorld;
    btVector3 wn2 = {result.n[0], result.n[1], 0};
    wn2.normalize();

    result.right_dir = {-wn2[1], wn2[0], 0.0};
    // now we have wall normale in XOY plane. Let us check all flags

    if((m_heightInfo.walls_climb_dir & SECTOR_FLAG_CLIMB_NORTH) && (wn2[1] < -0.7))
    {
        result.wall_hit = ClimbType::HandsOnly;                                                    // nW = (0, -1, 0);
    }
    if((m_heightInfo.walls_climb_dir & SECTOR_FLAG_CLIMB_EAST) && (wn2[0] < -0.7))
    {
        result.wall_hit = ClimbType::HandsOnly;                                                    // nW = (-1, 0, 0);
    }
    if((m_heightInfo.walls_climb_dir & SECTOR_FLAG_CLIMB_SOUTH) && (wn2[1] > 0.7))
    {
        result.wall_hit = ClimbType::HandsOnly;                                                    // nW = (0, 1, 0);
    }
    if((m_heightInfo.walls_climb_dir & SECTOR_FLAG_CLIMB_WEST) && (wn2[0] > 0.7))
    {
        result.wall_hit = ClimbType::HandsOnly;                                                    // nW = (1, 0, 0);
    }

    if(result.wall_hit != ClimbType::NoClimb)
    {
        from -= m_transform.getBasis().getColumn(2) * (0.67 * m_height);
        to = from;
        to += m_transform.getBasis().getColumn(1) * (m_forwardSize + m_bf.bb_max[1]);

        ccb->m_closestHitFraction = 1.0;
        ccb->m_hitCollisionObject = NULL;
        fromTransform.setIdentity();
        fromTransform.setOrigin(from);
        toTransform.setIdentity();
        toTransform.setOrigin(to);
        bt_engine_dynamicsWorld->convexSweepTest(&m_climbSensor, fromTransform, toTransform, *ccb);
        if(ccb->hasHit())
        {
            result.wall_hit = ClimbType::FullClimb;
        }
    }

    // now check ceiling limit (and floor too... may be later)
    /*vec3_add(from, point, transform.getBasis().getColumn(1));
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

    return result;
}


void Character::lean(btScalar max_lean)
{
    btScalar neg_lean   = 360.0 - max_lean;
    btScalar lean_coeff = (max_lean == 0.0)?(48.0):(max_lean * 3);

    // Continously lean character, according to current left/right direction.

    if((m_command.move[1] == 0) || (max_lean == 0.0))       // No direction - restore straight vertical position!
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
    else if(m_command.move[1] == 1) // Right direction
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
                m_angles[2] += ((360.0 - std::abs(m_angles[2]) + (lean_coeff*2) / 2) * engine_frame_time);
                if(m_angles[2] < 180.0) m_angles[2] = 0.0;
            }
            else    // Reduce previous lean
            {
                m_angles[2] -= ((std::abs(m_angles[2]) + lean_coeff) / 2) * engine_frame_time;
                if(m_angles[2] < 0.0) m_angles[2] = 0.0;
            }
        }
    }
    else if(m_command.move[1] == -1)     // Left direction
    {
        if(m_angles[2] != neg_lean)
        {
            if(m_angles[2] > neg_lean)   // Reduce previous lean
            {
                m_angles[2] -= ((360.0 - std::abs(m_angles[2]) + lean_coeff) / 2) * engine_frame_time;
                if(m_angles[2] < neg_lean)
                    m_angles[2] = neg_lean;
            }
            else if(m_angles[2] < 180.0) // Approaching from right
            {
                m_angles[2] -= ((std::abs(m_angles[2]) + (lean_coeff*2)) / 2) * engine_frame_time;
                if(m_angles[2] < 0.0) m_angles[2] += 360.0;
            }
            else    // Approaching from center
            {
                m_angles[2] += ((360.0 - std::abs(m_angles[2]) + lean_coeff) / 2) * engine_frame_time;
                if(m_angles[2] > 360.0) m_angles[2] -= 360.0;
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
    if     (m_command.rot[axis] < 0.0) { curr_rot_dir = 1; }
    else if(m_command.rot[axis] > 0.0) { curr_rot_dir = 2; }

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
    HeightInfo nfc;
    nfc.cb = m_rayCb;
    nfc.ccb = m_convexCb;
    m_response.horizontal_collision = false;
    m_response.ceiling_collision = false;
    m_response.floor_collision = false;
    // First of all - get information about floor and ceiling!!!
    updateCurrentHeight();
    if(m_heightInfo.floor_hit && (m_heightInfo.floor_point[2] + 1.0 >= m_transform.getOrigin()[2] + m_bf.bb_min[2]))
    {
        EngineContainer* cont = (EngineContainer*)m_heightInfo.floor_obj->getUserPointer();
        if((cont != NULL) && (cont->object_type == OBJECT_ENTITY))
        {
            Entity* e = static_cast<Entity*>(cont->object);
            if(e->m_callbackFlags & ENTITY_CALLBACK_STAND)
            {
                lua_ExecEntity(engine_lua, ENTITY_CALLBACK_STAND, e->id(), id());
            }
        }
    }

    btVector3& position = m_transform.getOrigin();
    btVector3 speed(0.0, 0.0, 0.0);

    /*
     * check move type
     */
    if(m_heightInfo.floor_hit || m_response.floor_collision)
    {
        if(m_heightInfo.floor_point[2] + m_fallDownHeight < position[2])
        {
            m_moveType = MOVE_FREE_FALLING;
            m_speed[2] = 0.0;
            return -1;                                                          // nothing to do here
        }
        else
        {
            m_response.floor_collision = true;
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
            m_response.floor_collision = true;
        }
        else    // no slide - free to walk
        {
            const btScalar fullSpeed = m_currentSpeed * m_speedMult;
            m_response.floor_collision = true;

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
        m_response.ceiling_collision = false;
        m_response.floor_collision = false;
        m_moveType = MOVE_FREE_FALLING;
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
        norm_move_xy_len = 32512.0;
        norm_move_xy.setZero();
    }

    ghostUpdate();
    position += positionDelta;
    fixPenetrations(&positionDelta);
    if(m_heightInfo.floor_hit)
    {
        if(m_heightInfo.floor_point[2] + m_fallDownHeight > position[2])
        {
            btScalar dz_to_land = engine_frame_time * 2400.0;                   ///@FIXME: magick
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
            m_moveType = MOVE_FREE_FALLING;
            m_speed[2] = 0.0;
            updateRoomPos();
            return 2;
        }
        if((position[2] < m_heightInfo.floor_point[2]) && !m_bt.no_fix_all)
        {
            position[2] = m_heightInfo.floor_point[2];
            fixPenetrations(nullptr);
            m_response.floor_collision = true;
        }
    }
    else if(!m_response.floor_collision)
    {
        m_moveType = MOVE_FREE_FALLING;
        m_speed[2] = 0.0;
        updateRoomPos();
        return 2;
    }

    updateRoomPos();

    return 1;
}


int Character::freeFalling()
{
    btVector3 move;
    auto& pos = m_transform.getOrigin();

    /*
     * init height info structure
     */

    m_response.slide = SlideType::None;
    m_response.horizontal_collision = false;
    m_response.ceiling_collision = false;
    m_response.floor_collision = false;

    btScalar rot = inertiaAngular(1.0, ROT_SPEED_FREEFALL, 0);
    m_angles[0] += rot;
    m_angles[1] = 0.0;

    updateTransform();                                                 // apply rotations

    const btVector3 grav = bt_engine_dynamicsWorld->getGravity();
    move = m_speed + grav * engine_frame_time * 0.5;
    move *= engine_frame_time;
    m_speed += grav * engine_frame_time;
    m_speed[2] = (m_speed[2] < -FREE_FALL_SPEED_MAXIMUM)?(-FREE_FALL_SPEED_MAXIMUM):(m_speed[2]);
    m_speed = m_speed.rotate({0,0,1}, rot * RadPerDeg);

    updateCurrentHeight();

    if(m_self->room && (m_self->room->flags & TR_ROOM_FLAG_WATER))
    {
        if(m_speed[2] < 0.0)
        {
            m_currentSpeed = 0.0;
            m_speed[0] = 0.0;
            m_speed[1] = 0.0;
        }

        if((engine_world.version < TR_II))//Lara cannot wade in < TRII so when floor < transition level she has to swim
        {
            if(!m_heightInfo.water || (m_currentSector->floor <= m_heightInfo.transition_level))
            {
                m_moveType = MOVE_UNDERWATER;
                return 2;
            }
        }
        else
        {
            if(!m_heightInfo.water || (m_currentSector->floor + m_height <= m_heightInfo.transition_level))
            {
                m_moveType = MOVE_UNDERWATER;
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
            m_speed[2] = 0.0;
            m_response.ceiling_collision = true;
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
            m_moveType = MOVE_ON_FLOOR;
            m_response.floor_collision = true;
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
            m_speed[2] = 0.0;
            m_response.ceiling_collision = true;
        }
    }
    if(m_heightInfo.floor_hit && m_speed[2] < 0.0)   // move down
    {
        if(m_heightInfo.floor_point[2] >= pos[2] + m_bf.bb_min[2] + move[2])
        {
            pos[2] = m_heightInfo.floor_point[2];
            //speed[2] = 0.0;
            m_moveType = MOVE_ON_FLOOR;
            m_response.floor_collision = true;
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
    m_response.horizontal_collision = false;
    m_response.ceiling_collision = false;
    m_response.floor_collision = true; //! @todo Monkey climbing with floor collision?

    t = m_currentSpeed * m_speedMult;

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
    m_response.slide = SlideType::None;

    m_speed = spd;
    move = spd * engine_frame_time;
    move[2] = 0.0;

    ghostUpdate();
    updateCurrentHeight();
    pos += move;
    fixPenetrations(&move);                              // get horizontal collide
    ///@FIXME: rewrite conditions! or add fixer to update_entity_rigid_body func
    if(m_heightInfo.ceiling_hit && (pos[2] + m_bf.bb_max[2] - m_heightInfo.ceiling_point[2] > - 0.33 * m_minStepUpHeight))
    {
        pos[2] = m_heightInfo.ceiling_point[2] - m_bf.bb_max[2];
    }
    else
    {
        m_moveType = MOVE_FREE_FALLING;
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
    m_response.horizontal_collision = false;
    m_response.ceiling_collision = false;
    m_response.floor_collision = false;

    spd={0,0,0};
    *climb = checkWallsClimbability();
    m_climb = *climb;
    if(climb->wall_hit == ClimbType::NoClimb)
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
        spd += climb->up_dir;
    }
    else if(m_dirFlag == ENT_MOVE_BACKWARD)
    {
        spd -= climb->up_dir;
    }
    else if(m_dirFlag == ENT_MOVE_RIGHT)
    {
        spd += climb->right_dir;
    }
    else if(m_dirFlag == ENT_MOVE_LEFT)
    {
        spd -= climb->right_dir;
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
    m_response.slide = SlideType::None;
    m_response.horizontal_collision = false;
    m_response.ceiling_collision = false;
    m_response.floor_collision = true; //! @todo Climbing with floor collision?

    const btScalar fullSpeed = m_currentSpeed * m_speedMult;
    m_angles[0] += m_command.rot[0];
    m_angles[1] = 0.0;
    m_angles[2] = 0.0;
    updateTransform();                                                 // apply rotations

    btVector3 spd(0.0, 0.0, 0.0);
    if(m_dirFlag == ENT_MOVE_FORWARD)
    {
        spd = m_transform.getBasis().getColumn(1) * fullSpeed;
    }
    else if(m_dirFlag == ENT_MOVE_BACKWARD)
    {
        spd = m_transform.getBasis().getColumn(1) * -fullSpeed;
    }
    else if(m_dirFlag == ENT_MOVE_LEFT)
    {
        spd = m_transform.getBasis().getColumn(0) * -fullSpeed;
    }
    else if(m_dirFlag == ENT_MOVE_RIGHT)
    {
        spd = m_transform.getBasis().getColumn(0) * fullSpeed;
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
    btVector3 move = spd * engine_frame_time;

    ghostUpdate();
    const btScalar origZ = m_transform.getOrigin()[2];
    m_transform.getOrigin() += move;
    fixPenetrations(&move);                              // get horizontal collide
    updateRoomPos();
    m_transform.getOrigin()[2] = origZ;

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
        m_moveType = MOVE_FREE_FALLING;
        return 2;
    }

    m_response.slide = SlideType::None;
    m_response.horizontal_collision = false;
    m_response.ceiling_collision = false;
    m_response.floor_collision = false;

    // Calculate current speed.

    btScalar t = inertiaLinear(MAX_SPEED_UNDERWATER, INERTIA_SPEED_UNDERWATER, m_command.jump);

    if(!m_response.killed)   // Block controls if Lara is dead.
    {
        m_angles[0] += inertiaAngular(1.0, ROT_SPEED_UNDERWATER, 0);
        m_angles[1] -= inertiaAngular(1.0, ROT_SPEED_UNDERWATER, 1);
        m_angles[2]  = 0.0;

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
            m_moveType = MOVE_ON_WATER;
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
    btVector3 move, spd(0.0, 0.0, 0.0);
    auto& pos = m_transform.getOrigin();

    m_response.slide = SlideType::None;
    m_response.horizontal_collision = false;
    m_response.ceiling_collision = false;
    m_response.floor_collision = false;

    m_angles[0] += inertiaAngular(1.0, ROT_SPEED_ONWATER, 0);
    m_angles[1] = 0.0;
    m_angles[2] = 0.0;
    updateTransform();     // apply rotations

    // Calculate current speed.

    btScalar t = inertiaLinear(MAX_SPEED_ONWATER, INERTIA_SPEED_ONWATER, std::abs(m_command.move[0])!=0 || std::abs(m_command.move[1])!=0);

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
            m_moveType = MOVE_ON_FLOOR;
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
        m_moveType = MOVE_ON_FLOOR;
        return 2;
    }

    return 1;
}

int Character::findTraverse()
{
    RoomSector* ch_s, *obj_s = NULL;
    ch_s = m_self->room->getSectorRaw(m_transform.getOrigin());

    if(ch_s == NULL)
    {
        return 0;
    }

    m_traversedObject = NULL;

    // OX move case
    if(m_transform.getBasis().getColumn(1)[0] > 0.9)
    {
        obj_s = ch_s->owner_room->getSectorRaw({(btScalar)(ch_s->pos[0] + TR_METERING_SECTORSIZE), (btScalar)(ch_s->pos[1]), (btScalar)0.0});
    }
    else if(m_transform.getBasis().getColumn(1)[0] < -0.9)
    {
        obj_s = ch_s->owner_room->getSectorRaw({(btScalar)(ch_s->pos[0] - TR_METERING_SECTORSIZE), (btScalar)(ch_s->pos[1]), (btScalar)0.0});
    }
    // OY move case
    else if(m_transform.getBasis().getColumn(1)[1] > 0.9)
    {
        obj_s = ch_s->owner_room->getSectorRaw({(btScalar)(ch_s->pos[0]), (btScalar)(ch_s->pos[1] + TR_METERING_SECTORSIZE), (btScalar)0.0});
    }
    else if(m_transform.getBasis().getColumn(1)[1] < -0.9)
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
                Entity* e = static_cast<Entity*>(cont->object);
                if((e->m_typeFlags & ENTITY_TYPE_TRAVERSE) && (1 == OBB_OBB_Test(*e, *this) && (std::abs(e->m_transform.getOrigin()[2] - m_transform.getOrigin()[2]) < 1.1)))
                {
                    int oz = (m_angles[0] + 45.0) / 90.0;
                    m_angles[0] = oz * 90.0;
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
int Sector_AllowTraverse(struct RoomSector *rs, btScalar floor, const std::shared_ptr<EngineContainer>& cont)
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

    BtEngineClosestRayResultCallback cb(cont);
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
        if(std::abs(v[2] - floor) < 1.1)
        {
            EngineContainer* cont = (EngineContainer*)cb.m_collisionObject->getUserPointer();
            if((cont != NULL) && (cont->object_type == OBJECT_ENTITY) && ((static_cast<Entity*>(cont->object))->m_typeFlags & ENTITY_TYPE_TRAVERSE_FLOOR))
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
    RoomSector* ch_s  =     m_self->room->getSectorRaw(    m_transform.getOrigin());
    RoomSector* obj_s = obj.m_self->room->getSectorRaw(obj.m_transform.getOrigin());

    if(obj_s == ch_s)
    {
        if(m_transform.getBasis().getColumn(1)[0] > 0.8)
        {
            ch_s = obj_s->owner_room->getSectorRaw({(btScalar)(obj_s->pos[0] - TR_METERING_SECTORSIZE), (btScalar)(obj_s->pos[1]), (btScalar)0.0});
        }
        else if(m_transform.getBasis().getColumn(1)[0] < -0.8)
        {
            ch_s = obj_s->owner_room->getSectorRaw({(btScalar)(obj_s->pos[0] + TR_METERING_SECTORSIZE), (btScalar)(obj_s->pos[1]), (btScalar)0.0});
        }
        // OY move case
        else if(m_transform.getBasis().getColumn(1)[1] > 0.8)
        {
            ch_s = obj_s->owner_room->getSectorRaw({(btScalar)(obj_s->pos[0]), (btScalar)(obj_s->pos[1] - TR_METERING_SECTORSIZE), (btScalar)0.0});
        }
        else if(m_transform.getBasis().getColumn(1)[1] < -0.8)
        {
            ch_s = obj_s->owner_room->getSectorRaw({(btScalar)(obj_s->pos[0]), (btScalar)(obj_s->pos[1] + TR_METERING_SECTORSIZE), (btScalar)0.0});
        }
        ch_s = ch_s->checkPortalPointer();
    }

    if((ch_s == NULL) || (obj_s == NULL))
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
    v0[2] = floor + TR_METERING_SECTORSIZE * 0.5;
    v1[2] = floor + TR_METERING_SECTORSIZE * 2.5;
    bt_engine_dynamicsWorld->rayTest(v0, v1, cb);
    if(cb.hasHit())
    {
        EngineContainer* cont = (EngineContainer*)cb.m_collisionObject->getUserPointer();
        if((cont != NULL) && (cont->object_type == OBJECT_ENTITY) && ((static_cast<Entity*>(cont->object))->m_typeFlags & ENTITY_TYPE_TRAVERSE))
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
        next_s = obj_s->owner_room->getSectorRaw({(btScalar)(obj_s->pos[0] + TR_METERING_SECTORSIZE), (btScalar)(obj_s->pos[1]), (btScalar)0.0});
    }
    else if(m_transform.getBasis().getColumn(1)[0] < -0.8)
    {
        next_s = obj_s->owner_room->getSectorRaw({(btScalar)(obj_s->pos[0] - TR_METERING_SECTORSIZE), (btScalar)(obj_s->pos[1]), (btScalar)0.0});
    }
    // OY move case
    else if(m_transform.getBasis().getColumn(1)[1] > 0.8)
    {
        next_s = obj_s->owner_room->getSectorRaw({(btScalar)(obj_s->pos[0]), (btScalar)(obj_s->pos[1] + TR_METERING_SECTORSIZE), (btScalar)0.0});
    }
    else if(m_transform.getBasis().getColumn(1)[1] < -0.8)
    {
        next_s = obj_s->owner_room->getSectorRaw({(btScalar)(obj_s->pos[0]), (btScalar)(obj_s->pos[1] - TR_METERING_SECTORSIZE), (btScalar)0.0});
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
        next_s = ch_s->owner_room->getSectorRaw({(btScalar)(ch_s->pos[0] - TR_METERING_SECTORSIZE), (btScalar)(ch_s->pos[1]), (btScalar)0.0});
    }
    else if(m_transform.getBasis().getColumn(1)[0] < -0.8)
    {
        next_s = ch_s->owner_room->getSectorRaw({(btScalar)(ch_s->pos[0] + TR_METERING_SECTORSIZE), (btScalar)(ch_s->pos[1]), (btScalar)0.0});
    }
    // OY move case
    else if(m_transform.getBasis().getColumn(1)[1] > 0.8)
    {
        next_s = ch_s->owner_room->getSectorRaw({(btScalar)(ch_s->pos[0]), (btScalar)(ch_s->pos[1] - TR_METERING_SECTORSIZE), (btScalar)0.0});
    }
    else if(m_transform.getBasis().getColumn(1)[1] < -0.8)
    {
        next_s = ch_s->owner_room->getSectorRaw({(btScalar)(ch_s->pos[0]), (btScalar)(ch_s->pos[1] + TR_METERING_SECTORSIZE), (btScalar)0.0});
    }

    if(next_s)
        next_s = next_s->checkPortalPointer();

    if((next_s != nullptr) && (Sector_AllowTraverse(next_s, floor, m_self) == 0x01))
    {
        btTransform from;
        from.setIdentity();
        from.setOrigin(btVector3(ch_s->pos[0], ch_s->pos[1], floor + 0.5 * TR_METERING_SECTORSIZE));

        btTransform to;
        to.setIdentity();
        to.setOrigin(btVector3(next_s->pos[0], next_s->pos[1], floor + 0.5 * TR_METERING_SECTORSIZE));

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
        (this->*state_func)(&m_bf.animations);
    }

    switch(m_moveType)
    {
        case MOVE_ON_FLOOR:
            moveOnFloor();
            break;

        case MOVE_FREE_FALLING:
            freeFalling();
            break;

        case MOVE_CLIMBING:
            climbing();
            break;

        case MOVE_MONKEYSWING:
            monkeyClimbing();
            break;

        case MOVE_WALLS_CLIMB:
            wallsClimbing();
            break;

        case MOVE_UNDERWATER:
            moveUnderWater();
            break;

        case MOVE_ON_WATER:
            moveOnWater();
            break;

        default:
            m_moveType = MOVE_ON_FLOOR;
            break;
    };

    updateRigidBody(true);
    updatePlatformPostStep();
}

void Character::updateParams()
{
    switch(m_moveType)
    {
        case MOVE_ON_FLOOR:
        case MOVE_FREE_FALLING:
        case MOVE_CLIMBING:
        case MOVE_MONKEYSWING:
        case MOVE_WALLS_CLIMB:

            if((m_heightInfo.quicksand == 0x02) &&
               (m_moveType == MOVE_ON_FLOOR))
            {
                if(!changeParam(PARAM_AIR, -3.0))
                    changeParam(PARAM_HEALTH, -3.0);
            }
            else if(m_heightInfo.quicksand == 0x01)
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
                changeParam(PARAM_STAMINA,  0.5);
            }
            break;

        case MOVE_ON_WATER:
            changeParam(PARAM_AIR, 3.0);;
            break;

        case MOVE_UNDERWATER:
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

    max_value = (max_value < 0)?(0):(max_value);    // Clamp max. to at least zero
    m_parameters.maximum[parameter] = max_value;
    return 1;
}

int Character::setParam(int parameter, float value)
{
    if(parameter >= PARAM_SENTINEL)
        return 0;

    float maximum = m_parameters.maximum[parameter];

    value = (value >= 0)?(value):(maximum); // Char params can't be less than zero.
    value = (value <= maximum)?(value):(maximum);

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

    if((sm != NULL) && (m_bf.bone_tags.size() == sm->mesh_count) && (sm->animations.size() >= 4))
    {
        SkeletalModel* bm = m_bf.animations.model;
        if(m_bf.animations.next == NULL)
        {
            addOverrideAnim(weapon_model);
        }
        else
        {
            m_bf.animations.next->model = sm;
        }

        for(int i=0;i<bm->mesh_count;i++)
        {
            m_bf.bone_tags[i].mesh_base = bm->mesh_tree[i].mesh_base;
            m_bf.bone_tags[i].mesh_slot = NULL;
        }

        if(armed != 0)
        {
            for(int i=0;i<bm->mesh_count;i++)
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
            for(int i=0;i<bm->mesh_count;i++)
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
            m_bf.animations.next->model = NULL;
        }

        return 1;
    }
    else
    {
        // do unarmed default model
        SkeletalModel* bm = m_bf.animations.model;
        for(int i=0;i<bm->mesh_count;i++)
        {
            m_bf.bone_tags[i].mesh_base = bm->mesh_tree[i].mesh_base;
            m_bf.bone_tags[i].mesh_slot = NULL;
        }
        if(m_bf.animations.next != NULL)
        {
            m_bf.animations.next->model = NULL;
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
        m_response.horizontal_collision = false;
        m_response.ceiling_collision = false;
        m_response.floor_collision = false;
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
    int numPenetrationLoops = getPenetrationFixVector(&reaction, move!=nullptr);
    m_transform.getOrigin() += reaction;

    updateCurrentHeight();
    if((move != NULL) && (numPenetrationLoops > 0))
    {
        btScalar t1 = reaction[0] * reaction[0] + reaction[1] * reaction[1];
        btScalar t2 = move->x() * move->x() + move->y() * move->y();
        if((reaction[2] * reaction[2] < t1) && (move->z() * move->z() < t2))    // we have horizontal move and horizontal correction
        {
            t2 *= t1;
            t1 = (reaction[0] * move->x() + reaction[1] * move->y()) / sqrtf(t2);
            if(t1 < m_criticalWallComponent)
            {
                m_response.horizontal_collision = true;
            }
        }
        else if((reaction[2] * reaction[2] > t1) && (move->z() * move->z() > t2))
        {
            if((reaction[2] > 0.0) && (move->z() < 0.0))
            {
                m_response.floor_collision = true;
            }
            else if((reaction[2] < 0.0) && (move->z() > 0.0))
            {
                m_response.ceiling_collision = true;
            }
        }
    }

    if(m_heightInfo.ceiling_hit && (reaction[2] < -0.1))
    {
        m_response.ceiling_collision = true;
    }

    if(m_heightInfo.floor_hit && (reaction[2] > 0.1))
    {
        m_response.floor_collision = true;
    }

    ghostUpdate();
}

/**
 * we check walls and other collision objects reaction. if reaction more than critical
 * then m_command.horizontal_collision=true;
 * @param move absolute 3d move vector
 */
int Character::checkNextPenetration(const btVector3& move)
{
    if(m_bt.ghostObjects.empty())
        return 0;

    ghostUpdate();
    m_transform.getOrigin() += move;
    //m_response.horizontal_collision = false;
    btVector3 reaction;
    int ret = getPenetrationFixVector(&reaction, true);
    if(ret > 0) {
        btScalar t1 = reaction[0] * reaction[0] + reaction[1] * reaction[1];
        btScalar t2 = move[0] * move[0] + move[1] * move[1];
        if((reaction[2] * reaction[2] < t1) && (move[2] * move[2] < t2)) {
            t2 *= t1;
            t1 = (reaction[0] * move[0] + reaction[1] * move[1]) / sqrtf(t2);
            if(t1 < m_criticalWallComponent) {
                m_response.horizontal_collision = true;
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

        if (auto ownerChar = hair->m_ownerChar.lock())
        {
            hair->m_container->room = ownerChar->m_self->room;
        }
    }
}

void Character::frameImpl(btScalar time, int16_t frame, int state) {
    // Update acceleration.
    // With variable framerate, we don't know when we'll reach final
    // frame for sure, so we use native frame number check to increase acceleration.

    if(m_bf.animations.current_frame != frame)
    {

        // NB!!! For Lara, we update ONLY X-axis speed/accel.

        auto af = &m_bf.animations.model->animations[ m_bf.animations.current_animation ];
        if((af->accel_x == 0) || (frame < m_bf.animations.current_frame))
        {
            m_currentSpeed  = af->speed_x;
        }
        else
        {
            m_currentSpeed += af->accel_x;
        }
    }

    m_bf.animations.current_frame = frame;

    doWeaponFrame(time);

    if(m_bf.animations.onFrame != nullptr)
    {
        (this->*m_bf.animations.onFrame)(&m_bf.animations, state);
    }
}

void Character::processSectorImpl() {
    assert( m_currentSector != nullptr );
    RoomSector* highest_sector = m_currentSector->getHighestSector();
    assert( highest_sector != nullptr );
    RoomSector* lowest_sector  = m_currentSector->getLowestSector();
    assert( lowest_sector != nullptr );

    m_heightInfo.walls_climb_dir  = 0;
    m_heightInfo.walls_climb_dir |= lowest_sector->flags & (SECTOR_FLAG_CLIMB_WEST  |
                                                            SECTOR_FLAG_CLIMB_EAST  |
                                                            SECTOR_FLAG_CLIMB_NORTH |
                                                            SECTOR_FLAG_CLIMB_SOUTH );

    m_heightInfo.walls_climb     = (m_heightInfo.walls_climb_dir > 0);
    m_heightInfo.ceiling_climb   = false;

    if((highest_sector->flags & SECTOR_FLAG_CLIMB_CEILING) || (lowest_sector->flags & SECTOR_FLAG_CLIMB_CEILING))
    {
        m_heightInfo.ceiling_climb = true;
    }

    if(lowest_sector->flags & SECTOR_FLAG_DEATH)
    {
        if((m_moveType == MOVE_ON_FLOOR)    ||
                (m_moveType == MOVE_UNDERWATER) ||
                (m_moveType == MOVE_WADE)        ||
                (m_moveType == MOVE_ON_WATER)    ||
                (m_moveType == MOVE_QUICKSAND))
        {
            setParam(PARAM_HEALTH, 0.0);
            m_response.killed = true;
        }
    }
}

void Character::jump(btScalar v_vertical, btScalar v_horizontal) {
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

    m_response.ceiling_collision = false;
    m_response.floor_collision = false;
    m_response.slide = SlideType::None;

    // Jump speed should NOT be added to current speed, as native engine
    // fully replaces current speed with jump speed by anim command.
    m_speed = spd;

    // Apply vertical speed.
    m_speed[2] = v_vertical * m_speedMult;
    m_moveType = MOVE_FREE_FALLING;
}

Substance Character::getSubstanceState() const {
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
    else if( m_heightInfo.water &&
             (m_heightInfo.transition_level > m_transform.getOrigin()[2]) &&
             (m_heightInfo.transition_level < m_transform.getOrigin()[2] + m_wadeDepth) )
    {
        return Substance::WaterShallow;
    }
    else if( m_heightInfo.water &&
             (m_heightInfo.transition_level > m_transform.getOrigin()[2] + m_wadeDepth) )
    {
        return Substance::WaterWade;
    }
    else
    {
        return Substance::WaterSwim;
    }
}

void Character::updateGhostRigidBody() {
    if(!m_bt.ghostObjects.empty()) {
        assert( m_bf.bone_tags.size() == m_bt.ghostObjects.size() );
        for(size_t i=0; i<m_bf.bone_tags.size(); i++) {
            auto tr = m_bt.bt_body[i]->getWorldTransform();
            tr.setOrigin(tr * m_bf.bone_tags[i].mesh_base->m_center);
            m_bt.ghostObjects[i]->getWorldTransform() = tr;
        }
    }
}

btVector3 Character::camPosForFollowing(btScalar dz) {
    if(m_camFollowCenter > 0) {
        m_camFollowCenter--;
        return m_obb->centre;
    }
    return Entity::camPosForFollowing(dz);
}

/* There are stick code for multianimation (weapon mode) testing
 * Model replacing will be upgraded too, I have to add override
 * flags to model manually in the script*/
void Character::doWeaponFrame(btScalar time)
{
    /* anims (TR_I - TR_V):
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

    btScalar dt;
    int t;

    for(SSAnimation* ss_anim=m_bf.animations.next;ss_anim!=NULL;ss_anim=ss_anim->next)
    {
        if((ss_anim->model != NULL) && (ss_anim->model->animations.size() > 4))
        {
            switch(m_weaponCurrentState)
            {
            case WeaponState::Hide:
                if(m_command.ready_weapon)   // ready weapon
                {
                    ss_anim->current_animation = 1;
                    ss_anim->next_animation = 1;
                    ss_anim->current_frame = 0;
                    ss_anim->next_frame = 0;
                    ss_anim->frame_time = 0.0;
                    m_weaponCurrentState = WeaponState::HideToReady;
                }
                break;

            case WeaponState::HideToReady:
                ss_anim->frame_time += time;
                ss_anim->current_frame = (ss_anim->frame_time) / ss_anim->period;
                dt = ss_anim->frame_time - (btScalar)ss_anim->current_frame * ss_anim->period;
                ss_anim->lerp = dt / ss_anim->period;
                t = ss_anim->model->animations[ss_anim->current_animation].frames.size();

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
                    m_weaponCurrentState = WeaponState::Idle;
                }
                break;

            case WeaponState::Idle:
                ss_anim->current_frame = 0;
                ss_anim->current_animation = 0;
                ss_anim->next_frame = 0;
                ss_anim->next_animation = 0;
                ss_anim->frame_time = 0.0;
                if(m_command.ready_weapon)
                {
                    ss_anim->current_animation = 3;
                    ss_anim->next_animation = 3;
                    ss_anim->current_frame = ss_anim->next_frame = 0;
                    ss_anim->frame_time = 0.0;
                    m_weaponCurrentState = WeaponState::IdleToHide;
                }
                else if(m_command.action)
                {
                    m_weaponCurrentState = WeaponState::IdleToFire;
                }
                else
                {
                    // do nothing here, may be;
                }
                break;

            case WeaponState::FireToIdle:
                // Yes, same animation, reverse frames order;
                t = ss_anim->model->animations[ss_anim->current_animation].frames.size();
                ss_anim->frame_time += time;
                ss_anim->current_frame = (ss_anim->frame_time) / ss_anim->period;
                dt = ss_anim->frame_time - (btScalar)ss_anim->current_frame * ss_anim->period;
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
                    m_weaponCurrentState = WeaponState::Idle;
                }
                break;

            case WeaponState::IdleToFire:
                ss_anim->frame_time += time;
                ss_anim->current_frame = (ss_anim->frame_time) / ss_anim->period;
                dt = ss_anim->frame_time - (btScalar)ss_anim->current_frame * ss_anim->period;
                ss_anim->lerp = dt / ss_anim->period;
                t = ss_anim->model->animations[ss_anim->current_animation].frames.size();

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
                else if(m_command.action)
                {
                    ss_anim->current_frame = 0;
                    ss_anim->next_frame = 1;
                    ss_anim->current_animation = 2;
                    ss_anim->next_animation = ss_anim->current_animation;
                    m_weaponCurrentState = WeaponState::Fire;
                }
                else
                {
                    ss_anim->frame_time = 0.0;
                    ss_anim->current_frame = ss_anim->model->animations[ss_anim->current_animation].frames.size() - 1;
                    m_weaponCurrentState = WeaponState::FireToIdle;
                }
                break;

            case WeaponState::Fire:
                if(m_command.action)
                {
                    // inc time, loop;
                    ss_anim->frame_time += time;
                    ss_anim->current_frame = (ss_anim->frame_time) / ss_anim->period;
                    dt = ss_anim->frame_time - (btScalar)ss_anim->current_frame * ss_anim->period;
                    ss_anim->lerp = dt / ss_anim->period;
                    t = ss_anim->model->animations[ss_anim->current_animation].frames.size();

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
                    ss_anim->current_frame = ss_anim->model->animations[ss_anim->current_animation].frames.size() - 1;
                    ss_anim->next_frame = (ss_anim->current_frame > 0)?(ss_anim->current_frame - 1):(0);
                    m_weaponCurrentState = WeaponState::FireToIdle;
                }
                break;

            case WeaponState::IdleToHide:
                t = ss_anim->model->animations[ss_anim->current_animation].frames.size();
                ss_anim->frame_time += time;
                ss_anim->current_frame = (ss_anim->frame_time) / ss_anim->period;
                dt = ss_anim->frame_time - (btScalar)ss_anim->current_frame * ss_anim->period;
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
                    m_weaponCurrentState = WeaponState::Hide;
                    setWeaponModel(m_currentWeapon, 0);
                }
                break;
            };
        }
        else if((ss_anim->model != NULL) && (ss_anim->model->animations.size() == 4))
        {
            switch(m_weaponCurrentState)
            {
            case WeaponState::Hide:
                if(m_command.ready_weapon)   // ready weapon
                {
                    ss_anim->current_animation = 2;
                    ss_anim->next_animation = 2;
                    ss_anim->current_frame = 0;
                    ss_anim->next_frame = 0;
                    ss_anim->frame_time = 0.0;
                    m_weaponCurrentState = WeaponState::HideToReady;
                }
                break;

            case WeaponState::HideToReady:
                ss_anim->frame_time += time;
                ss_anim->current_frame = (ss_anim->frame_time) / ss_anim->period;
                dt = ss_anim->frame_time - (btScalar)ss_anim->current_frame * ss_anim->period;
                ss_anim->lerp = dt / ss_anim->period;
                t = ss_anim->model->animations[ss_anim->current_animation].frames.size();

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
                    m_weaponCurrentState = WeaponState::Idle;
                }
                break;

            case WeaponState::Idle:
                ss_anim->current_frame = 0;
                ss_anim->current_animation = 0;
                ss_anim->next_frame = 0;
                ss_anim->next_animation = 0;
                ss_anim->frame_time = 0.0;
                if(m_command.ready_weapon)
                {
                    ss_anim->current_animation = 2;
                    ss_anim->next_animation = 2;
                    ss_anim->current_frame = ss_anim->next_frame = ss_anim->model->animations[ss_anim->current_animation].frames.size() - 1;
                    ss_anim->frame_time = 0.0;
                    m_weaponCurrentState = WeaponState::IdleToHide;
                }
                else if(m_command.action)
                {
                    m_weaponCurrentState = WeaponState::IdleToFire;
                }
                else
                {
                    // do nothing here, may be;
                }
                break;

            case WeaponState::FireToIdle:
                // Yes, same animation, reverse frames order;
                t = ss_anim->model->animations[ss_anim->current_animation].frames.size();
                ss_anim->frame_time += time;
                ss_anim->current_frame = (ss_anim->frame_time) / ss_anim->period;
                dt = ss_anim->frame_time - (btScalar)ss_anim->current_frame * ss_anim->period;
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
                    m_weaponCurrentState = WeaponState::Idle;
                }
                break;

            case WeaponState::IdleToFire:
                ss_anim->frame_time += time;
                ss_anim->current_frame = (ss_anim->frame_time) / ss_anim->period;
                dt = ss_anim->frame_time - (btScalar)ss_anim->current_frame * ss_anim->period;
                ss_anim->lerp = dt / ss_anim->period;
                t = ss_anim->model->animations[ss_anim->current_animation].frames.size();

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
                else if(m_command.action)
                {
                    ss_anim->current_frame = 0;
                    ss_anim->next_frame = 1;
                    ss_anim->current_animation = 3;
                    ss_anim->next_animation = ss_anim->current_animation;
                    m_weaponCurrentState = WeaponState::Fire;
                }
                else
                {
                    ss_anim->frame_time = 0.0;
                    ss_anim->current_frame = ss_anim->model->animations[ss_anim->current_animation].frames.size() - 1;
                    m_weaponCurrentState = WeaponState::FireToIdle;
                }
                break;

            case WeaponState::Fire:
                if(m_command.action)
                {
                    // inc time, loop;
                    ss_anim->frame_time += time;
                    ss_anim->current_frame = (ss_anim->frame_time) / ss_anim->period;
                    dt = ss_anim->frame_time - (btScalar)ss_anim->current_frame * ss_anim->period;
                    ss_anim->lerp = dt / ss_anim->period;
                    t = ss_anim->model->animations[ss_anim->current_animation].frames.size();

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
                    ss_anim->current_frame = ss_anim->model->animations[ss_anim->current_animation].frames.size() - 1;
                    ss_anim->next_frame = (ss_anim->current_frame > 0)?(ss_anim->current_frame - 1):(0);
                    m_weaponCurrentState = WeaponState::FireToIdle;
                }
                break;

            case WeaponState::IdleToHide:
                // Yes, same animation, reverse frames order;
                t = ss_anim->model->animations[ss_anim->current_animation].frames.size();
                ss_anim->frame_time += time;
                ss_anim->current_frame = (ss_anim->frame_time) / ss_anim->period;
                dt = ss_anim->frame_time - (btScalar)ss_anim->current_frame * ss_anim->period;
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
                    m_weaponCurrentState = WeaponState::Hide;
                    setWeaponModel(m_currentWeapon, 0);
                }
                break;
            };
        }

        doAnimCommands(ss_anim, 0);
    }
}

void Character::stateLaraStop(SSAnimation* ss_anim, HeightInfo &next_fc, bool low_vertical_space)
{
    // Reset directional flag only on intermediate animation!

    if(ss_anim->current_animation == TR_ANIMATION_LARA_STAY_SOLID)
    {
        m_dirFlag = ENT_STAY;
    }

    m_command.rot[0] = 0;
    m_command.crouch |= low_vertical_space;
    lean(0.0);

    if( (m_climb.can_hang &&
        (m_climb.next_z_space >= m_height - LARA_HANG_VERTICAL_EPSILON) &&
        (m_moveType == MOVE_CLIMBING)) ||
        (ss_anim->current_animation == TR_ANIMATION_LARA_STAY_SOLID) )
    {
        m_moveType = MOVE_ON_FLOOR;
    }

    if(m_moveType == MOVE_ON_FLOOR)
    {
        m_bt.no_fix_body_parts = BODY_PART_LEGS_2 | BODY_PART_LEGS_3;
    }

    if(m_moveType == MOVE_FREE_FALLING)
    {
        setAnimation(TR_ANIMATION_LARA_START_FREE_FALL, 0);
        m_dirFlag = ENT_STAY;
    }
    else if(m_response.killed)
    {
        ss_anim->next_state = TR_STATE_LARA_DEATH;
    }
    else if(m_response.slide == SlideType::Front)
    {
        Audio_Send(TR_AUDIO_SOUND_LANDING, TR_AUDIO_EMITTER_ENTITY, id());

        if(m_command.jump)
        {
            m_dirFlag = ENT_MOVE_FORWARD;
            setAnimation(TR_ANIMATION_LARA_JUMP_FORWARD_BEGIN, 0);
        }
        else
        {
            setAnimation(TR_ANIMATION_LARA_SLIDE_FORWARD, 0);
        }
    }
    else if(m_response.slide == SlideType::Back)
    {
        if(m_command.jump)
        {
            m_dirFlag = ENT_MOVE_BACKWARD;
            setAnimation(TR_ANIMATION_LARA_JUMP_BACK_BEGIN, 0);
            Audio_Send(TR_AUDIO_SOUND_LANDING, TR_AUDIO_EMITTER_ENTITY, id());
        }
        else
        {
            setAnimation(TR_ANIMATION_LARA_START_SLIDE_BACKWARD, 0);
        }
    }
    else if(m_command.jump)
    {
        if(!m_heightInfo.quicksand)
            ss_anim->next_state = TR_STATE_LARA_JUMP_PREPARE;       // jump sideways
    }
    else if(m_command.roll)
    {
        if(!m_heightInfo.quicksand && ss_anim->current_animation != TR_ANIMATION_LARA_CLIMB_2CLICK)
        {
            m_dirFlag = ENT_MOVE_FORWARD;
            setAnimation(TR_ANIMATION_LARA_ROLL_BEGIN, 0);
        }
    }
    else if(m_command.crouch)
    {
        if(!m_heightInfo.quicksand)
            ss_anim->next_state = TR_STATE_LARA_CROUCH_IDLE;
    }
    else if(m_command.action && findTraverse())
    {
        ss_anim->next_state = TR_STATE_LARA_PUSHABLE_GRAB;
        btScalar t;
        if(m_transform.getBasis().getColumn(1).x() > 0.9)
        {
            t = -m_traversedObject->m_bf.bb_min[0] + 72.0;
        }
        else if(m_transform.getBasis().getColumn(1).x() < -0.9)
        {
            t = m_traversedObject->m_bf.bb_max[0] + 72.0;
        }
        else if(m_transform.getBasis().getColumn(1).y() > 0.9)
        {
            t = -m_traversedObject->m_bf.bb_min[1] + 72.0;
        }
        else if(m_transform.getBasis().getColumn(1).y() < -0.9)
        {
            t = m_traversedObject->m_bf.bb_max[1] + 72.0;
        }
        else
        {
            t = 512.0 + 72.0;                                           ///@PARANOID
        }
        const btVector3& v = m_traversedObject->m_transform.getOrigin();
        m_transform.getOrigin()[0] = v[0] - m_transform.getBasis().getColumn(1).x() * t;
        m_transform.getOrigin()[1] = v[1] - m_transform.getBasis().getColumn(1).y() * t;
    }
    else if(m_command.move[0] == 1)
    {
        if(m_command.shift)
        {
            btVector3 move = m_transform.getBasis().getColumn(1) * PENETRATION_TEST_OFFSET;
            btVector3 global_offset = m_transform.getBasis().getColumn(1) * WALK_FORWARD_OFFSET;
            global_offset[2] += m_bf.bb_max[2];
            global_offset += m_transform.getOrigin();
            Character::getHeightInfo(global_offset, &next_fc);
            if(((checkNextPenetration(move) == 0) || !m_response.horizontal_collision) &&
               (next_fc.floor_hit && (next_fc.floor_point[2] > m_transform.getOrigin()[2] - m_maxStepUpHeight) && (next_fc.floor_point[2] <= m_transform.getOrigin()[2] + m_maxStepUpHeight)))
            {
                m_moveType = MOVE_ON_FLOOR;
                m_dirFlag = ENT_MOVE_FORWARD;
                if((m_heightInfo.water || m_heightInfo.quicksand) && m_heightInfo.floor_hit && (m_heightInfo.transition_level - m_heightInfo.floor_point[2] > m_wadeDepth))
                {
                    ss_anim->next_state = TR_STATE_LARA_WADE_FORWARD;
                }
                else
                {
                    ss_anim->next_state = TR_STATE_LARA_WALK_FORWARD;
                }
            }
        }       // end IF CMD->SHIFT
        else
        {
            btVector3 move = m_transform.getBasis().getColumn(1) * PENETRATION_TEST_OFFSET;
            btVector3 global_offset = m_transform.getBasis().getColumn(1) * RUN_FORWARD_OFFSET;
            global_offset[2] += m_bf.bb_max[2];
            checkNextStep(global_offset, &next_fc);
            if(((checkNextPenetration(move) == 0) || !m_response.horizontal_collision) && !hasStopSlant(next_fc))
            {
                m_moveType = MOVE_ON_FLOOR;
                m_dirFlag = ENT_MOVE_FORWARD;
                if((m_heightInfo.water || m_heightInfo.quicksand) && m_heightInfo.floor_hit && (m_heightInfo.transition_level - m_heightInfo.floor_point[2] > m_wadeDepth))
                {
                    ss_anim->next_state = TR_STATE_LARA_WADE_FORWARD;
                }
                else
                {
                    ss_anim->next_state = TR_STATE_LARA_RUN_FORWARD;
                }
            }
        }

        if( m_command.action &&
            ((ss_anim->current_animation == TR_ANIMATION_LARA_STAY_IDLE)        ||
             (ss_anim->current_animation == TR_ANIMATION_LARA_STAY_SOLID)       ||
             (ss_anim->current_animation == TR_ANIMATION_LARA_WALL_SMASH_LEFT)  ||
             (ss_anim->current_animation == TR_ANIMATION_LARA_WALL_SMASH_RIGHT)) )
        {
            btScalar t = m_forwardSize + LARA_TRY_HANG_WALL_OFFSET;
            btVector3 global_offset = m_transform.getBasis().getColumn(1) * t;

            global_offset[2] += 0.5 * DEFAULT_CLIMB_UP_HEIGHT;
            m_climb = checkClimbability(global_offset, &next_fc, 0.5 * DEFAULT_CLIMB_UP_HEIGHT);
            if(  m_climb.edge_hit != ClimbType::NoClimb                                       &&
                (m_climb.next_z_space >= m_height - LARA_HANG_VERTICAL_EPSILON)    &&
                (m_transform.getOrigin()[2] + m_maxStepUpHeight < next_fc.floor_point[2])             &&
                (m_transform.getOrigin()[2] + 2944.0 >= next_fc.floor_point[2])                                  &&
                (next_fc.floor_normale[2] >= m_criticalSlantZComponent)  ) // trying to climb on
            {
                if(m_transform.getOrigin()[2] + 640.0 >= next_fc.floor_point[2])
                {
                    m_angles[0] = m_climb.edge_z_ang;
                    m_transform.getOrigin()[2] = next_fc.floor_point[2] - 512.0;
                    m_climb.point = next_fc.floor_point;
                    setAnimation(TR_ANIMATION_LARA_CLIMB_2CLICK, 0);
                    m_bt.no_fix_all = true;
                    ss_anim->onFrame = &Character::setOnFloorAfterClimb;
                    return;
                }
                else if(m_transform.getOrigin()[2] + 896.0 >= next_fc.floor_point[2])
                {
                    m_angles[0] = m_climb.edge_z_ang;
                    m_transform.getOrigin()[2] = next_fc.floor_point[2] - 768.0;
                    m_climb.point = next_fc.floor_point;
                    setAnimation(TR_ANIMATION_LARA_CLIMB_3CLICK, 0);
                    m_bt.no_fix_all = true;
                    ss_anim->onFrame = &Character::setOnFloorAfterClimb;
                    return;
                }
            }   // end IF MOVE_LITTLE_CLIMBING

            global_offset[2] += 0.5 * DEFAULT_CLIMB_UP_HEIGHT;
            m_climb = checkClimbability(global_offset, &next_fc, DEFAULT_CLIMB_UP_HEIGHT);
            if(  m_climb.edge_hit != ClimbType::NoClimb                                       &&
                (m_climb.next_z_space >= m_height - LARA_HANG_VERTICAL_EPSILON)    &&
                (m_transform.getOrigin()[2] + m_maxStepUpHeight < next_fc.floor_point[2])             &&
                (m_transform.getOrigin()[2] + 2944.0 >= next_fc.floor_point[2])                                  &&
                (next_fc.floor_normale[2] >= m_criticalSlantZComponent)  ) // trying to climb on
            {
                if(m_transform.getOrigin()[2] + 1920.0 >= next_fc.floor_point[2])
                {
                    ss_anim->next_state = TR_STATE_LARA_JUMP_UP;
                    return;
                }
            }   // end IF MOVE_BIG_CLIMBING

            m_climb = checkWallsClimbability();
            if(m_climb.wall_hit != ClimbType::NoClimb)
            {
                ss_anim->next_state = TR_STATE_LARA_JUMP_UP;
                return;
            }
        }
    }       // end CMD->MOVE FORWARD
    else if(m_command.move[0] == -1)
    {
        if(m_command.shift)
        {
            btVector3 move = m_transform.getBasis().getColumn(1) * -PENETRATION_TEST_OFFSET;
            if((checkNextPenetration(move) == 0) || !m_response.horizontal_collision)
            {
                btVector3 global_offset = m_transform.getBasis().getColumn(1) * -WALK_BACK_OFFSET;
                global_offset[2] += m_bf.bb_max[2];
                global_offset += m_transform.getOrigin();
                Character::getHeightInfo(global_offset, &next_fc);
                if((next_fc.floor_hit && (next_fc.floor_point[2] > m_transform.getOrigin()[2] - m_maxStepUpHeight) && (next_fc.floor_point[2] <= m_transform.getOrigin()[2] + m_maxStepUpHeight)))
                {
                    m_dirFlag = ENT_MOVE_BACKWARD;
                    ss_anim->next_state = TR_STATE_LARA_WALK_BACK;
                }
            }
        }
        else    // RUN BACK
        {
            btVector3 move = m_transform.getBasis().getColumn(1) * -PENETRATION_TEST_OFFSET;
            if((checkNextPenetration(move) == 0) || !m_response.horizontal_collision)
            {
                m_dirFlag = ENT_MOVE_BACKWARD;
                if((m_heightInfo.water || m_heightInfo.quicksand) && m_heightInfo.floor_hit && (m_heightInfo.transition_level - m_heightInfo.floor_point[2] > m_wadeDepth))
                {
                    ss_anim->next_state = TR_STATE_LARA_WALK_BACK;
                }
                else
                {
                    ss_anim->next_state = TR_STATE_LARA_RUN_BACK;
                }
            }
        }
    }       // end CMD->MOVE BACK
    else if(m_command.move[1] == 1)
    {
        if(m_command.shift)
        {
            btVector3 move = m_transform.getBasis().getColumn(0) * PENETRATION_TEST_OFFSET;
            if((checkNextPenetration(move) == 0) || !m_response.horizontal_collision)
            {
                btVector3 global_offset = m_transform.getBasis().getColumn(0) * RUN_FORWARD_OFFSET;
                global_offset[2] += m_bf.bb_max[2];
                NextStepInfo i = checkNextStep(global_offset, &next_fc);
                if(!m_response.horizontal_collision && (i >= NextStepInfo::DownLittle && i <= NextStepInfo::UpLittle))
                {
                    m_command.rot[0] = 0.0;
                    m_dirFlag = ENT_MOVE_RIGHT;
                    ss_anim->next_state = TR_STATE_LARA_WALK_RIGHT;
                }
            }
        }       //end IF CMD->SHIFT
        else
        {
            ss_anim->next_state = TR_STATE_LARA_TURN_RIGHT_SLOW;
        }
    }       // end MOVE RIGHT
    else if(m_command.move[1] == -1)
    {
        if(m_command.shift)
        {
            btVector3 move = m_transform.getBasis().getColumn(0) * -PENETRATION_TEST_OFFSET;
            if((checkNextPenetration(move) == 0) || !m_response.horizontal_collision)
            {
                btVector3 global_offset = m_transform.getBasis().getColumn(0) * -RUN_FORWARD_OFFSET;
                global_offset[2] += m_bf.bb_max[2];
                NextStepInfo i = checkNextStep(global_offset, &next_fc);
                if(!m_response.horizontal_collision && (i >= NextStepInfo::DownLittle && i <= NextStepInfo::UpLittle))
                {
                    m_command.rot[0] = 0.0;
                    m_dirFlag = ENT_MOVE_LEFT;
                    ss_anim->next_state = TR_STATE_LARA_WALK_LEFT;
                }
            }
        }       //end IF CMD->SHIFT
        else
        {
            ss_anim->next_state = TR_STATE_LARA_TURN_LEFT_SLOW;
        }
    }       // end MOVE LEFT
}

void Character::stateLaraJumpPrepare(SSAnimation* ss_anim)
{
    m_command.rot[0] = 0;
    lean(0.0);

    if(m_response.slide == SlideType::Back)      // Slide checking is only for jumps direction correction!
    {
        setAnimation(TR_ANIMATION_LARA_JUMP_BACK_BEGIN, 0);
        m_command.move[0] = -1;
    }
    else if(m_response.slide == SlideType::Front)
    {
        setAnimation(TR_ANIMATION_LARA_JUMP_FORWARD_BEGIN, 0);
        m_command.move[0] = 1;
    }
    if((m_heightInfo.water || m_heightInfo.quicksand) && m_heightInfo.floor_hit && (m_heightInfo.transition_level - m_heightInfo.floor_point[2] > m_wadeDepth))
    {
        //Stay, directional jumps are not allowed whilst in wade depth
    }
    else if(m_command.move[0] == 1)
    {
        m_dirFlag = ENT_MOVE_FORWARD;
        btVector3 move = m_transform.getBasis().getColumn(1) * PENETRATION_TEST_OFFSET;
        if(checkNextPenetration(move) == 0)
        {
            ss_anim->next_state = TR_STATE_LARA_JUMP_FORWARD;           // jump forward
        }
    }
    else if(m_command.move[0] ==-1)
    {
        m_dirFlag = ENT_MOVE_BACKWARD;
        btVector3 move = m_transform.getBasis().getColumn(1) * -PENETRATION_TEST_OFFSET;
        if(checkNextPenetration(move) == 0)
        {
            ss_anim->next_state = TR_STATE_LARA_JUMP_BACK;              // jump backward
        }
    }
    else if(m_command.move[1] == 1)
    {
        m_dirFlag = ENT_MOVE_RIGHT;
        btVector3 move = m_transform.getBasis().getColumn(0) * PENETRATION_TEST_OFFSET;
        if(checkNextPenetration(move) == 0)
        {
            ss_anim->next_state = TR_STATE_LARA_JUMP_LEFT;              // jump right
        }
    }
    else if(m_command.move[1] ==-1)
    {
        m_dirFlag = ENT_MOVE_LEFT;
        btVector3 move = m_transform.getBasis().getColumn(0) * -PENETRATION_TEST_OFFSET;
        if(checkNextPenetration(move) == 0)
        {
            ss_anim->next_state = TR_STATE_LARA_JUMP_RIGHT;             // jump left
        }
    }
}

void Character::stateLaraJumpBack(SSAnimation* ss_anim)
{
    m_command.rot[0] = 0.0;
    if(m_response.floor_collision || m_moveType == MOVE_ON_FLOOR)
    {
        if(m_heightInfo.quicksand)
        {
            setAnimation(TR_ANIMATION_LARA_STAY_IDLE, 0);
        }
        else
        {
            ss_anim->next_state = TR_STATE_LARA_STOP;       // landing
        }
    }
    else if(m_response.horizontal_collision)
    {
        Controls_JoyRumble(200.0, 200);
        setAnimation(TR_ANIMATION_LARA_SMASH_JUMP, 0);
        m_dirFlag = ENT_MOVE_FORWARD;
        updateCurrentSpeed(true);
    }
    else if((m_moveType == MOVE_UNDERWATER) || (m_speed[2] <= -FREE_FALL_SPEED_2))
    {
        ss_anim->next_state = TR_STATE_LARA_FREEFALL;                   // free falling
    }
    else if(m_command.roll)
    {
        ss_anim->next_state = TR_STATE_LARA_JUMP_ROLL;
    }
}

void Character::stateLaraJumpLeftRight(SSAnimation* ss_anim)
{
    m_command.rot[0] = 0.0;
    if(m_response.floor_collision || m_moveType == MOVE_ON_FLOOR)
    {
        if(m_heightInfo.quicksand)
        {
            setAnimation(TR_ANIMATION_LARA_STAY_IDLE, 0);
        }
        else
        {
            ss_anim->next_state = TR_STATE_LARA_STOP;       // landing
        }
    }
    else if(m_response.horizontal_collision)
    {
        Controls_JoyRumble(200.0, 200);
        setAnimation(TR_ANIMATION_LARA_SMASH_JUMP, 0);
        m_dirFlag = ss_anim->last_state==TR_STATE_LARA_JUMP_LEFT
                  ? ENT_MOVE_RIGHT
                  : ENT_MOVE_LEFT;
        updateCurrentSpeed(true);
    }
    else
    {
        ss_anim->next_state = TR_STATE_LARA_FREEFALL;
    }
}

void Character::stateLaraRunBack()
{
    m_dirFlag = ENT_MOVE_BACKWARD;

    if(m_moveType == MOVE_FREE_FALLING)
    {
        m_dirFlag = ENT_MOVE_FORWARD;
        setAnimation(TR_ANIMATION_LARA_FREE_FALL_BACK, 0);
    }
    else if(m_response.horizontal_collision)
    {
        setAnimation(TR_ANIMATION_LARA_CLIMB_2CLICK_END, 0);
    }
}

void Character::stateLaraTurnSlow(SSAnimation* ss_anim, bool last_frame)
{
    m_command.rot[0] *= 0.7;
    m_dirFlag = ENT_STAY;
    lean(0.0);
    m_bt.no_fix_body_parts = BODY_PART_LEGS_2 | BODY_PART_LEGS_3;

    if(m_command.move[0] == 1)
    {
        Substance substance_state = getSubstanceState();
        if((substance_state == Substance::None) ||
           (substance_state == Substance::WaterShallow))
        {
            if(m_command.shift)
            {
                ss_anim->next_state = TR_STATE_LARA_WALK_FORWARD;
                m_dirFlag = ENT_MOVE_FORWARD;
            }
            else
            {
                ss_anim->next_state = TR_STATE_LARA_RUN_FORWARD;
                m_dirFlag = ENT_MOVE_FORWARD;
            }
        }
        else
        {
            ss_anim->next_state = TR_STATE_LARA_WADE_FORWARD;
            m_dirFlag = ENT_MOVE_FORWARD;
        }

    }
    else if(((ss_anim->last_state == TR_STATE_LARA_TURN_LEFT_SLOW ) && (m_command.move[1] == -1)) ||
            ((ss_anim->last_state == TR_STATE_LARA_TURN_RIGHT_SLOW) && (m_command.move[1] ==  1))  )
    {
        Substance substance_state = getSubstanceState();
        if(last_frame &&
           (substance_state != Substance::WaterWade) &&
           (substance_state != Substance::QuicksandConsumed) &&
           (substance_state != Substance::QuicksandShallow))
         {
             ss_anim->next_state = TR_STATE_LARA_TURN_FAST;
         }
    }
    else
    {
        ss_anim->next_state = TR_STATE_LARA_STOP;
    }
}

void Character::stateLaraTurnFast(SSAnimation* ss_anim)
{
    // 65 - wade
    m_dirFlag = ENT_STAY;
    m_bt.no_fix_body_parts = BODY_PART_LEGS_2 | BODY_PART_LEGS_3;
    lean(0.0);

    if(m_moveType == MOVE_FREE_FALLING)
    {
        setAnimation(TR_ANIMATION_LARA_START_FREE_FALL, 0);
    }
    else if(m_command.move[0] == 1 && !m_command.jump && !m_command.crouch && m_command.shift)
    {
        ss_anim->next_state = TR_STATE_LARA_WALK_FORWARD;
        m_dirFlag = ENT_MOVE_FORWARD;
    }
    else if(m_command.move[0] == 1 && !m_command.jump && !m_command.crouch && !m_command.shift)
    {
        ss_anim->next_state = TR_STATE_LARA_RUN_FORWARD;
        m_dirFlag = ENT_MOVE_FORWARD;
    }
    else if(m_command.move[1] == 0)
    {
        ss_anim->next_state = TR_STATE_LARA_STOP;
    }
}

void Character::stateLaraRunForward(SSAnimation* ss_anim, HeightInfo& next_fc, bool low_vertical_space)
{
    btVector3 global_offset = m_transform.getBasis().getColumn(1) * RUN_FORWARD_OFFSET;
    global_offset[2] += m_bf.bb_max[2];
    NextStepInfo i = checkNextStep(global_offset, &next_fc);
    m_dirFlag = ENT_MOVE_FORWARD;
    m_command.crouch |= low_vertical_space;

    if(m_moveType == MOVE_ON_FLOOR)
    {
        m_bt.no_fix_body_parts = BODY_PART_HANDS | BODY_PART_LEGS;;
    }
    lean(6.0);

    if(m_moveType == MOVE_FREE_FALLING)
    {
        setAnimation(TR_ANIMATION_LARA_FREE_FALL_FORWARD, 0);
    }
    else if(m_response.killed)
    {
        ss_anim->next_state = TR_STATE_LARA_DEATH;
    }
    else if(m_response.slide == SlideType::Front)
    {
        setAnimation(TR_ANIMATION_LARA_SLIDE_FORWARD, 0);
    }
    else if(m_response.slide == SlideType::Back)
    {
        setAnimation(TR_ANIMATION_LARA_START_SLIDE_BACKWARD, 0);
        m_dirFlag = ENT_MOVE_BACKWARD;
    }
    else if(hasStopSlant(next_fc))
    {
        m_dirFlag = ENT_STAY;
        setAnimation(TR_ANIMATION_LARA_STAY_IDLE, 0);
    }
    else if(m_command.crouch)
    {
        ss_anim->next_state = TR_STATE_LARA_CROUCH_IDLE;
    }
    else if((m_command.move[0] == 1) && !m_command.crouch && (next_fc.floor_normale[2] >= m_criticalSlantZComponent) && (i == NextStepInfo::UpBig))
    {
        m_dirFlag = ENT_STAY;
        int dispCase = getAnimDispatchCase(2);                         // MOST CORRECT STATECHANGE!!!
        if(dispCase == 0)
        {
            setAnimation(TR_ANIMATION_LARA_RUN_UP_STEP_RIGHT, 0);
            m_transform.getOrigin()[2] = next_fc.floor_point[2];
            m_dirFlag = ENT_MOVE_FORWARD;
        }
        else //if(i == 1)
        {
            setAnimation(TR_ANIMATION_LARA_RUN_UP_STEP_LEFT, 0);
            m_transform.getOrigin()[2] = next_fc.floor_point[2];
            m_dirFlag = ENT_MOVE_FORWARD;
        }
    }
    else if(m_response.horizontal_collision)
    {
        global_offset = m_transform.getBasis().getColumn(1) * RUN_FORWARD_OFFSET;
        global_offset[2] += 1024.0;
        if(ss_anim->current_animation == TR_ANIMATION_LARA_STAY_TO_RUN)
        {
            setAnimation(TR_ANIMATION_LARA_STAY_IDLE, 0);
        }
        else
        {
            Controls_JoyRumble(200.0, 200);

            if(m_command.move[0] == 1)
            {
                int dispCase = getAnimDispatchCase(2);
                if(dispCase == 1)
                {
                    setAnimation(TR_ANIMATION_LARA_WALL_SMASH_LEFT, 0);
                }
                else
                {
                    setAnimation(TR_ANIMATION_LARA_WALL_SMASH_RIGHT, 0);
                }
            }
            else
            {
                setAnimation(TR_ANIMATION_LARA_STAY_SOLID, 0);
            }
        }
        updateCurrentSpeed(false);
    }
    else if(m_command.move[0] == 1)                                          // If we continue running...
    {
        if((m_heightInfo.water || m_heightInfo.quicksand) && m_heightInfo.floor_hit && (m_heightInfo.transition_level - m_heightInfo.floor_point[2] > m_wadeDepth))
        {
            ss_anim->next_state = TR_STATE_LARA_WADE_FORWARD;
        }
        else if(m_command.shift)
        {
            ss_anim->next_state = TR_STATE_LARA_WALK_FORWARD;
        }
        else if(m_command.jump && (ss_anim->last_animation != TR_ANIMATION_LARA_STAY_TO_RUN))
        {
            ss_anim->next_state = TR_STATE_LARA_JUMP_FORWARD;
        }
        else if(m_command.roll)
        {
            m_dirFlag = ENT_MOVE_FORWARD;
            setAnimation(TR_ANIMATION_LARA_ROLL_BEGIN, 0);
        }
        else if(m_command.sprint)
        {
            ss_anim->next_state = TR_STATE_LARA_SPRINT;
        }
    }
    else
    {
        ss_anim->next_state = TR_STATE_LARA_STOP;
    }
}

void Character::stateLaraSprint(SSAnimation* ss_anim, HeightInfo& next_fc, bool low_vertical_space)
{
    btVector3 global_offset = m_transform.getBasis().getColumn(1) * RUN_FORWARD_OFFSET;
    lean(12.0);
    global_offset[2] += m_bf.bb_max[2];
    NextStepInfo i = checkNextStep(global_offset, &next_fc);
    m_command.crouch |= low_vertical_space;

    if(m_moveType == MOVE_ON_FLOOR)
    {
        m_bt.no_fix_body_parts = BODY_PART_LEGS_1 | BODY_PART_LEGS_2 | BODY_PART_LEGS_3;
    }

    if(!getParam(PARAM_STAMINA))
    {
        ss_anim->next_state = TR_STATE_LARA_RUN_FORWARD;
    }
    else if(m_moveType == MOVE_FREE_FALLING)
    {
        setAnimation(TR_ANIMATION_LARA_FREE_FALL_FORWARD, 0);
    }
    else if(m_response.killed)
    {
        ss_anim->next_state = TR_STATE_LARA_RUN_FORWARD;    // Normal run then die
    }
    else if(m_response.slide == SlideType::Front)
    {
        setAnimation(TR_ANIMATION_LARA_SLIDE_FORWARD, 0);
    }
    else if(m_response.slide == SlideType::Back)
    {
        setAnimation(TR_ANIMATION_LARA_START_SLIDE_BACKWARD, 0);
    }
    else if((next_fc.floor_normale[2] < m_criticalSlantZComponent) && (i > NextStepInfo::Horizontal))
    {
        m_currentSpeed = 0.0;
        setAnimation(TR_ANIMATION_LARA_STAY_IDLE, 0);       ///@FIXME: maybe RUN_TO_STAY
    }
    else if((next_fc.floor_normale[2] >= m_criticalSlantZComponent) && (i == NextStepInfo::UpBig))
    {
        ss_anim->next_state = TR_STATE_LARA_RUN_FORWARD;     // Interrupt sprint
    }
    else if(m_response.horizontal_collision)
    {
        Controls_JoyRumble(200.0, 200);

        int dispCase = getAnimDispatchCase(2);                         // tested!
        if(dispCase == 1)
        {
            setAnimation(TR_ANIMATION_LARA_WALL_SMASH_LEFT, 0);
        }
        else if(dispCase == 0)
        {
            setAnimation(TR_ANIMATION_LARA_WALL_SMASH_RIGHT, 0);
        }
        updateCurrentSpeed(false);
    }
    else if(!m_command.sprint)
    {
        if(m_command.move[0] == 1)
        {
            ss_anim->next_state = TR_STATE_LARA_RUN_FORWARD;
        }
        else
        {
            ss_anim->next_state = TR_STATE_LARA_STOP;
        }
    }
    else
    {
        if(m_command.jump == 1)
        {
            ss_anim->next_state = TR_STATE_LARA_SPRINT_ROLL;
        }
        else if(m_command.roll == 1)
        {
            m_dirFlag = ENT_MOVE_FORWARD;
            setAnimation(TR_ANIMATION_LARA_ROLL_BEGIN, 0);
        }
        else if(m_command.crouch)
        {
            ss_anim->next_state = TR_STATE_LARA_CROUCH_IDLE;
        }
        else if(m_command.move[0] == 0)
        {
            ss_anim->next_state = TR_STATE_LARA_STOP;
        }
    }
}

void Character::stateLaraWalkForward(SSAnimation* ss_anim, HeightInfo& next_fc, bool low_vertical_space)
{
    m_command.rot[0] *= 0.4;
    lean(0.0);

    btVector3 global_offset = m_transform.getBasis().getColumn(1) * WALK_FORWARD_OFFSET;
    global_offset[2] += m_bf.bb_max[2];
    NextStepInfo i = checkNextStep(global_offset, &next_fc);
    m_dirFlag = ENT_MOVE_FORWARD;

    if(m_moveType == MOVE_ON_FLOOR)
    {
        m_bt.no_fix_body_parts = BODY_PART_LEGS_1 | BODY_PART_LEGS_2 | BODY_PART_LEGS_3;
    }

    if(m_moveType == MOVE_FREE_FALLING)
    {
        setAnimation(TR_ANIMATION_LARA_START_FREE_FALL, 0);
    }
    else if(m_response.killed)
    {
        ss_anim->next_state = TR_STATE_LARA_STOP;
    }
    else if((next_fc.floor_normale[2] >= m_criticalSlantZComponent) && (i == NextStepInfo::UpBig))
    {
        /*
         * climb up
         */
        m_dirFlag = ENT_STAY;
        int dispCase = getAnimDispatchCase(2);
        if(dispCase == 1)
        {
            setAnimation(TR_ANIMATION_LARA_WALK_UP_STEP_RIGHT, 0);
            m_transform.getOrigin() = next_fc.floor_point;
            m_moveType = MOVE_ON_FLOOR;
            m_dirFlag = ENT_MOVE_FORWARD;
        }
        else
        {
            setAnimation(TR_ANIMATION_LARA_WALK_UP_STEP_LEFT, 0);
            m_transform.getOrigin() = next_fc.floor_point;
            m_moveType = MOVE_ON_FLOOR;
            m_dirFlag = ENT_MOVE_FORWARD;
        }
    }
    else if((next_fc.floor_normale[2] >= m_criticalSlantZComponent) && (i == NextStepInfo::DownBig))
    {
        /*
         * climb down
         */
        m_dirFlag = ENT_STAY;
        int dispCase = getAnimDispatchCase(2);
        if(dispCase == 1)
        {
            setAnimation(TR_ANIMATION_LARA_WALK_DOWN_RIGHT, 0);
            m_climb.point = next_fc.floor_point;
            m_transform.getOrigin() = next_fc.floor_point;
            m_moveType = MOVE_ON_FLOOR;
            m_dirFlag = ENT_MOVE_FORWARD;
        }
        else //if(i == 0)
        {
            setAnimation(TR_ANIMATION_LARA_WALK_DOWN_LEFT, 0);
            m_climb.point = next_fc.floor_point;
            m_transform.getOrigin() = next_fc.floor_point;
            m_moveType = MOVE_ON_FLOOR;
            m_dirFlag = ENT_MOVE_FORWARD;
        }
    }
    else if(m_response.horizontal_collision || (i < NextStepInfo::DownBig || i > NextStepInfo::UpBig) || (low_vertical_space))
    {
        /*
         * too high
         */
        m_dirFlag = ENT_STAY;
        setAnimation(TR_ANIMATION_LARA_STAY_IDLE, 0);
    }
    else if(m_command.move[0] != 1)
    {
        ss_anim->next_state = TR_STATE_LARA_STOP;
    }
    else if((m_heightInfo.water || m_heightInfo.quicksand) && m_heightInfo.floor_hit && (m_heightInfo.transition_level - m_heightInfo.floor_point[2] > m_wadeDepth))
    {
        ss_anim->next_state = TR_STATE_LARA_WADE_FORWARD;
    }
    else if(m_command.move[0] == 1 && !m_command.crouch && !m_command.shift)
    {
        ss_anim->next_state = TR_STATE_LARA_RUN_FORWARD;
    }
}

void Character::stateLaraWadeForward(SSAnimation* ss_anim)
{
    m_command.rot[0] *= 0.4;
    m_dirFlag = ENT_MOVE_FORWARD;

    if(m_heightInfo.quicksand)
    {
        m_currentSpeed = (m_currentSpeed > MAX_SPEED_QUICKSAND)?MAX_SPEED_QUICKSAND:m_currentSpeed;
    }

    if(m_command.move[0] == 1)
    {
        btVector3 move = m_transform.getBasis().getColumn(1) * PENETRATION_TEST_OFFSET;
        checkNextPenetration(move);
    }

    if(m_response.killed)
    {
        ss_anim->next_state = TR_STATE_LARA_STOP;
    }

    if(!m_heightInfo.floor_hit || m_moveType == MOVE_FREE_FALLING)      // free fall, next swim
    {
        setAnimation(TR_ANIMATION_LARA_START_FREE_FALL, 0);
    }
    else if(m_heightInfo.water)
    {
        if((m_heightInfo.transition_level - m_heightInfo.floor_point[2] <= m_wadeDepth))
        {
            // run / walk case
            if((m_command.move[0] == 1) && !m_response.horizontal_collision)
            {
                ss_anim->next_state = TR_STATE_LARA_RUN_FORWARD;
            }
            else
            {
                ss_anim->next_state = TR_STATE_LARA_STOP;
            }
        }
        else if(m_heightInfo.transition_level - m_heightInfo.floor_point[2] > (m_height - m_swimDepth))
        {
            // swim case
            if(m_heightInfo.transition_level - m_heightInfo.floor_point[2] > m_height + m_maxStepUpHeight)
            {
                setAnimation(TR_ANIMATION_LARA_START_FREE_FALL, 0);                                    // swim underwater
            }
            else
            {
                setAnimation(TR_ANIMATION_LARA_ONWATER_IDLE, 0);                                       // swim onwater
                m_moveType = MOVE_ON_WATER;
                m_transform.getOrigin()[2] = m_heightInfo.transition_level;
            }
        }
        else if(m_heightInfo.transition_level - m_heightInfo.floor_point[2] > m_wadeDepth)              // wade case
        {
            if((m_command.move[0] != 1) || m_response.horizontal_collision)
            {
                ss_anim->next_state = TR_STATE_LARA_STOP;
            }
        }
    }
    else                                                                // no water, stay or run / walk
    {
        if((m_command.move[0] == 1) && !m_response.horizontal_collision)
        {
            if(!m_heightInfo.quicksand)
            {
                ss_anim->next_state = TR_STATE_LARA_RUN_FORWARD;
            }
        }
        else
        {
            ss_anim->next_state = TR_STATE_LARA_STOP;
        }
    }
}

void Character::stateLaraWalkBack(SSAnimation* ss_anim, HeightInfo& next_fc)
{
    m_command.rot[0] *= 0.4;
    m_dirFlag = ENT_MOVE_BACKWARD;

    if(m_heightInfo.quicksand)
    {
        m_currentSpeed = (m_currentSpeed > MAX_SPEED_QUICKSAND)?MAX_SPEED_QUICKSAND:m_currentSpeed;
    }

    btVector3 global_offset = m_transform.getBasis().getColumn(1) * -WALK_BACK_OFFSET;
    global_offset[2] += m_bf.bb_max[2];
    NextStepInfo i = checkNextStep(global_offset, &next_fc);
    if(m_moveType == MOVE_FREE_FALLING)
    {
        setAnimation(TR_ANIMATION_LARA_START_FREE_FALL, 0);
    }
    else if(m_heightInfo.water && (m_heightInfo.floor_point[2] + m_height < m_heightInfo.transition_level))
    {
        setAnimation(TR_ANIMATION_LARA_ONWATER_SWIM_BACK, 0);
        ss_anim->next_state = TR_STATE_LARA_ONWATER_BACK;
        m_moveType = MOVE_ON_WATER;
    }
    else if((i < NextStepInfo::DownBig) || (i > NextStepInfo::UpBig))
    {
        m_dirFlag = ENT_STAY;
        setAnimation(TR_ANIMATION_LARA_CLIMB_2CLICK_END, 0);
    }
    else if((next_fc.floor_normale[2] >= m_criticalSlantZComponent) && (i == NextStepInfo::DownBig))
    {
        if(!m_bt.no_fix_all)
        {
            int frames_count = ss_anim->model->animations[TR_ANIMATION_LARA_WALK_DOWN_BACK_LEFT].frames.size();
            int frames_count2 = (frames_count + 1) / 2;
            if((ss_anim->current_frame >= 0) && (ss_anim->current_frame <= frames_count2))
            {
                setAnimation(TR_ANIMATION_LARA_WALK_DOWN_BACK_LEFT, ss_anim->current_frame);
                m_dirFlag = ENT_MOVE_BACKWARD;
                m_transform.getOrigin()[2] -= (m_heightInfo.floor_point[2] - next_fc.floor_point[2]);
                m_bt.no_fix_all = true;
            }
            else if((ss_anim->current_frame >= frames_count) && (ss_anim->current_frame <= frames_count + frames_count2))
            {
                setAnimation(TR_ANIMATION_LARA_WALK_DOWN_BACK_RIGHT, ss_anim->current_frame - frames_count);
                m_dirFlag = ENT_MOVE_BACKWARD;
                m_transform.getOrigin()[2] -= (m_heightInfo.floor_point[2] - next_fc.floor_point[2]);
                m_bt.no_fix_all = true;
            }
            else
            {
                m_dirFlag = ENT_STAY;                               // waiting for correct frame
            }
        }
    }
    else if((m_command.move[0] == -1) && (m_command.shift || m_heightInfo.quicksand))
    {
        m_dirFlag = ENT_MOVE_BACKWARD;
        ss_anim->next_state = TR_STATE_LARA_WALK_BACK;
    }
    else
    {
        ss_anim->next_state = TR_STATE_LARA_STOP;
    }
}

void Character::stateLaraWalkLeftRight(SSAnimation* ss_anim, HeightInfo& next_fc)
{
    m_command.rot[0] = 0;
    m_dirFlag = ss_anim->last_state == TR_STATE_LARA_WALK_LEFT
              ? ENT_MOVE_LEFT
              : ENT_MOVE_RIGHT;
    const int8_t moveCommand = ss_anim->last_state == TR_STATE_LARA_WALK_LEFT
                         ? -1
                         : 1;
    if(m_moveType == MOVE_FREE_FALLING)
    {
        setAnimation(TR_ANIMATION_LARA_START_FREE_FALL, 0);
    }
    else if(m_command.move[1] == moveCommand && m_command.shift)
    {
        btVector3 global_offset = moveCommand * m_transform.getBasis().getColumn(0) * RUN_FORWARD_OFFSET;  // not an error - RUN_... more correct here
        global_offset[2] += m_bf.bb_max[2];
        global_offset += m_transform.getOrigin();
        Character::getHeightInfo(global_offset, &next_fc);
        if(next_fc.floor_hit && (next_fc.floor_point[2] > m_transform.getOrigin()[2] - m_maxStepUpHeight) && (next_fc.floor_point[2] <= m_transform.getOrigin()[2] + m_maxStepUpHeight))
        {
            if(!m_heightInfo.water || (m_heightInfo.floor_point[2] + m_height > m_heightInfo.transition_level)) // if (floor_hit == 0) then we went to MOVE_FREE_FALLING.
            {
                // continue walking
            }
            else
            {
                ss_anim->next_state = ss_anim->last_state == TR_STATE_LARA_WALK_LEFT
                                    ? TR_STATE_LARA_ONWATER_LEFT
                                    : TR_STATE_LARA_ONWATER_RIGHT;
                ss_anim->onFrame = &Character::toOnWater;
            }
        }
        else
        {
            m_dirFlag = ENT_STAY;
            setAnimation(TR_ANIMATION_LARA_STAY_SOLID, 0);
        }
    }
    else
    {
        ss_anim->next_state = TR_STATE_LARA_STOP;
    }
}

void Character::stateLaraSlideBack(SSAnimation* ss_anim)
{
    m_command.rot[0] = 0;
    lean(0.0);
    m_dirFlag = ENT_MOVE_BACKWARD;

    if(m_moveType == MOVE_FREE_FALLING)
    {
        if(m_command.action)
        {
            m_speed[0] = -m_transform.getBasis().getColumn(1)[0] * 128.0;
            m_speed[1] = -m_transform.getBasis().getColumn(1)[1] * 128.0;
        }

        setAnimation(TR_ANIMATION_LARA_FREE_FALL_FORWARD, 0);
    }
    else if(m_response.slide == SlideType::None)
    {
        ss_anim->next_state = TR_STATE_LARA_STOP;
    }
    else if(m_response.slide != SlideType::None && m_command.jump)
    {
        ss_anim->next_state = TR_STATE_LARA_JUMP_BACK;
    }
    else
    {
        return;
    }

    Audio_Kill(TR_AUDIO_SOUND_SLIDING, TR_AUDIO_EMITTER_ENTITY, id());
}

void Character::stateLaraSlideForward(SSAnimation* ss_anim)
{
    m_command.rot[0] = 0;
    lean(0.0);
    m_dirFlag = ENT_MOVE_FORWARD;

    if(m_moveType == MOVE_FREE_FALLING)
    {
        m_speed[0] *= 0.2;
        m_speed[1] *= 0.2;
        setAnimation(TR_ANIMATION_LARA_FREE_FALL_FORWARD, 0);
    }
    else if(m_response.slide == SlideType::None)
    {
        if((m_command.move[0] == 1) && (engine_world.version >= TR_III))
        {
             ss_anim->next_state = TR_STATE_LARA_RUN_FORWARD;
        }
        else
        {
             ss_anim->next_state = TR_STATE_LARA_STOP;                  // stop
        }
    }
    else if(m_response.slide != SlideType::None && m_command.jump)
    {
        ss_anim->next_state = TR_STATE_LARA_JUMP_FORWARD;               // jump
    }
    else
    {
        return;
    }

    Audio_Kill(TR_AUDIO_SOUND_SLIDING, TR_AUDIO_EMITTER_ENTITY, id());
}

void Character::stateLaraPushableGrab(SSAnimation* ss_anim)
{
    m_moveType = MOVE_ON_FLOOR;
    m_bt.no_fix_all = true;
    m_command.rot[0] = 0.0;

    if(m_command.action)//If Lara is grabbing the block
    {
        int tf = checkTraverse(*m_traversedObject);
        m_dirFlag = ENT_STAY;
        ss_anim->anim_flags = ANIM_LOOP_LAST_FRAME;                     //We hold it (loop last frame)

        if((m_command.move[0] == 1) && (tf & Character::TraverseForward))                          //If player press up push
        {
            m_dirFlag = ENT_MOVE_FORWARD;
            ss_anim->anim_flags = ANIM_NORMAL_CONTROL;
            ss_anim->next_state = TR_STATE_LARA_PUSHABLE_PUSH;
        }
        else if((m_command.move[0] == -1) && (tf & Character::TraverseBackward))                    //If player press down pull
        {
            m_dirFlag = ENT_MOVE_BACKWARD;
            ss_anim->anim_flags = ANIM_NORMAL_CONTROL;
            ss_anim->next_state = TR_STATE_LARA_PUSHABLE_PULL;
        }
    }
    else//Lara has let go of the block
    {
        m_dirFlag = ENT_STAY;
        ss_anim->anim_flags = ANIM_NORMAL_CONTROL;                      //We no longer loop last frame
        ss_anim->next_state = TR_STATE_LARA_STOP;                       //Switch to next Lara state
    }
}

void Character::stateLaraPushablePush(SSAnimation* ss_anim)
{
    m_bt.no_fix_all = true;
    ss_anim->onFrame = &Character::stopTraverse;
    m_command.rot[0] = 0.0;
    m_camFollowCenter = 64;
    int i = ss_anim->model->animations[ss_anim->current_animation].frames.size();

    if(!m_command.action || !(Character::TraverseForward & checkTraverse(*m_traversedObject)))   //For TOMB4/5 If Lara is pushing and action let go, don't push
    {
        ss_anim->next_state = TR_STATE_LARA_STOP;
    }

    if((m_traversedObject != NULL) && (ss_anim->current_frame > 16) && (ss_anim->current_frame < i - 16)) ///@FIXME: magick 16
    {
        bool was_traversed = false;

        if(m_transform.getBasis().getColumn(1)[0] > 0.9)
        {
            btScalar t = m_transform.getOrigin()[0] + (m_bf.bb_max[1] - m_traversedObject->m_bf.bb_min[0] - 32.0);
            if(t > m_traversedObject->m_transform.getOrigin()[0])
            {
                m_traversedObject->m_transform.getOrigin()[0] = t;
                was_traversed = true;
            }
        }
        else if(m_transform.getBasis().getColumn(1)[0] < -0.9)
        {
            btScalar t = m_transform.getOrigin()[0] - (m_bf.bb_max[1] + m_traversedObject->m_bf.bb_max[0] - 32.0);
            if(t < m_traversedObject->m_transform.getOrigin()[0])
            {
                m_traversedObject->m_transform.getOrigin()[0] = t;
                was_traversed = true;
            }
        }
        else if(m_transform.getBasis().getColumn(1)[1] > 0.9)
        {
            btScalar t = m_transform.getOrigin()[1] + (m_bf.bb_max[1] - m_traversedObject->m_bf.bb_min[1] - 32.0);
            if(t > m_traversedObject->m_transform.getOrigin()[1])
            {
                m_traversedObject->m_transform.getOrigin()[1] = t;
                was_traversed = true;
            }
        }
        else if(m_transform.getBasis().getColumn(1)[1] < -0.9)
        {
            btScalar t = m_transform.getOrigin()[1] - (m_bf.bb_max[1] + m_traversedObject->m_bf.bb_max[1] - 32.0);
            if(t < m_traversedObject->m_transform.getOrigin()[1])
            {
                m_traversedObject->m_transform.getOrigin()[1] = t;
                was_traversed = true;
            }
        }

        if(engine_world.version > TR_III)
        {
            if(was_traversed)
            {
                if(Audio_IsEffectPlaying(TR_AUDIO_SOUND_PUSHABLE,TR_AUDIO_EMITTER_ENTITY,id()) == -1)
                    Audio_Send(TR_AUDIO_SOUND_PUSHABLE, TR_AUDIO_EMITTER_ENTITY, id());
            }
            else
            {
                Audio_Kill(TR_AUDIO_SOUND_PUSHABLE, TR_AUDIO_EMITTER_ENTITY, id());
            }
        }
        else
        {
            if( (ss_anim->current_frame == 49)   ||
                (ss_anim->current_frame == 110)  ||
                (ss_anim->current_frame == 142)   )
            {
                if(Audio_IsEffectPlaying(TR_AUDIO_SOUND_PUSHABLE,TR_AUDIO_EMITTER_ENTITY,id()) == -1)
                    Audio_Send(TR_AUDIO_SOUND_PUSHABLE, TR_AUDIO_EMITTER_ENTITY, id());
            }
        }

        m_traversedObject->updateRigidBody(true);
    }
    else
    {
        if(engine_world.version > TR_III)
        {
            Audio_Kill(TR_AUDIO_SOUND_PUSHABLE, TR_AUDIO_EMITTER_ENTITY, id());
        }
    }
}

void Character::stateLaraPushablePull(SSAnimation* ss_anim)
{
    m_bt.no_fix_all = true;
    ss_anim->onFrame = &Character::stopTraverse;
    m_command.rot[0] = 0.0;
    m_camFollowCenter = 64;
    int i = ss_anim->model->animations[ss_anim->current_animation].frames.size();

    if(!m_command.action || !(Character::TraverseBackward & checkTraverse(*m_traversedObject)))   //For TOMB4/5 If Lara is pulling and action let go, don't pull
    {
        ss_anim->next_state = TR_STATE_LARA_STOP;
    }

    if((m_traversedObject != NULL) && (ss_anim->current_frame > 20) && (ss_anim->current_frame < i - 16)) ///@FIXME: magick 20
    {
        bool was_traversed = false;

        if(m_transform.getBasis().getColumn(1)[0] > 0.9)
        {
            btScalar t = m_transform.getOrigin()[0] + (m_bf.bb_max[1] - m_traversedObject->m_bf.bb_min[0] - 32.0);
            if(t < m_traversedObject->m_transform.getOrigin()[0])
            {
                m_traversedObject->m_transform.getOrigin()[0] = t;
                was_traversed = true;
            }
        }
        else if(m_transform.getBasis().getColumn(1)[0] < -0.9)
        {
            btScalar t = m_transform.getOrigin()[0] - (m_bf.bb_max[1] + m_traversedObject->m_bf.bb_max[0] - 32.0);
            if(t > m_traversedObject->m_transform.getOrigin()[0])
            {
                m_traversedObject->m_transform.getOrigin()[0] = t;
                was_traversed = true;
            }
        }
        else if(m_transform.getBasis().getColumn(1)[1] > 0.9)
        {
            btScalar t = m_transform.getOrigin()[1] + (m_bf.bb_max[1] - m_traversedObject->m_bf.bb_min[1] - 32.0);
            if(t < m_traversedObject->m_transform.getOrigin()[1])
            {
                m_traversedObject->m_transform.getOrigin()[1] = t;
                was_traversed = true;
            }
        }
        else if(m_transform.getBasis().getColumn(1)[1] < -0.9)
        {
            btScalar t = m_transform.getOrigin()[1] - (m_bf.bb_max[1] + m_traversedObject->m_bf.bb_max[1] - 32.0);
            if(t > m_traversedObject->m_transform.getOrigin()[1])
            {
                m_traversedObject->m_transform.getOrigin()[1] = t;
                was_traversed = true;
            }
        }

        if(engine_world.version > TR_III)
        {
            if(was_traversed)
            {
                if(Audio_IsEffectPlaying(TR_AUDIO_SOUND_PUSHABLE,TR_AUDIO_EMITTER_ENTITY,id()) == -1)

                    Audio_Send(TR_AUDIO_SOUND_PUSHABLE, TR_AUDIO_EMITTER_ENTITY, id());
            }
            else
            {
                Audio_Kill(TR_AUDIO_SOUND_PUSHABLE, TR_AUDIO_EMITTER_ENTITY, id());
            }
        }
        else
        {
            if( (ss_anim->current_frame == 40)  ||
                (ss_anim->current_frame == 92)  ||
                (ss_anim->current_frame == 124) ||
                (ss_anim->current_frame == 156)  )
            {
                if(Audio_IsEffectPlaying(TR_AUDIO_SOUND_PUSHABLE,TR_AUDIO_EMITTER_ENTITY,id()) == -1)
                    Audio_Send(TR_AUDIO_SOUND_PUSHABLE, TR_AUDIO_EMITTER_ENTITY, id());
            }
        }

        m_traversedObject->updateRigidBody(true);
    }
    else
    {
        if(engine_world.version > TR_III)
        {
            Audio_Kill(TR_AUDIO_SOUND_PUSHABLE, TR_AUDIO_EMITTER_ENTITY, id());
        }
    }
}

void Character::stateLaraRollBackward(bool low_vertical_space)
{
    if(m_moveType == MOVE_FREE_FALLING)
    {
        setAnimation(TR_ANIMATION_LARA_FREE_FALL_FORWARD, 0);
    }
    else if(low_vertical_space)
    {
        m_dirFlag = ENT_STAY;
    }
    else if(m_response.slide == SlideType::Front)
    {
        setAnimation(TR_ANIMATION_LARA_SLIDE_FORWARD, 0);
    }
    else if(m_response.slide == SlideType::Back)
    {
        setAnimation(TR_ANIMATION_LARA_START_SLIDE_BACKWARD, 0);
    }
}

void Character::stateLaraJumpUp(SSAnimation* ss_anim, HeightInfo& next_fc)
{
    m_command.rot[0] = 0.0;
    if(m_command.action && (m_moveType != MOVE_WALLS_CLIMB) && (m_moveType != MOVE_CLIMBING))
    {
        btScalar t = LARA_TRY_HANG_WALL_OFFSET + LARA_HANG_WALL_DISTANCE;
        btVector3 global_offset = m_transform.getBasis().getColumn(1) * t;
        global_offset[2] += m_bf.bb_max[2] + LARA_HANG_VERTICAL_EPSILON + engine_frame_time * m_speed[2];
        m_climb = checkClimbability(global_offset, &next_fc, 0.0);
        if(m_climb.edge_hit != ClimbType::NoClimb)
        {
            m_climb.point = m_climb.edge_point;
            m_angles[0] = m_climb.edge_z_ang;
            updateTransform();
            m_moveType = MOVE_CLIMBING;                             // hang on
            m_speed.setZero();

            m_transform.getOrigin()[0] = m_climb.point[0] - (LARA_HANG_WALL_DISTANCE) * m_transform.getBasis().getColumn(1)[0];
            m_transform.getOrigin()[1] = m_climb.point[1] - (LARA_HANG_WALL_DISTANCE) * m_transform.getBasis().getColumn(1)[1];
            m_transform.getOrigin()[2] = m_climb.point[2] - m_bf.bb_max[2] + LARA_HANG_VERTICAL_OFFSET;
        }
        else
        {
            m_climb = checkWallsClimbability();
            if((m_climb.wall_hit != ClimbType::NoClimb) &&
               (m_speed[2] < 0.0)) // Only hang if speed is lower than zero.
            {
                // Fix the position to the TR metering step.
                m_transform.getOrigin()[2] = std::floor(m_transform.getOrigin()[2] / TR_METERING_STEP) * TR_METERING_STEP;
                m_moveType = MOVE_WALLS_CLIMB;
                setAnimation(TR_ANIMATION_LARA_HANG_IDLE, -1);
                return;
            }
        }
    }

    if(m_command.move[0] == 1)
    {
        m_dirFlag = ENT_MOVE_FORWARD;
    }
    else if(m_command.move[0] == -1)
    {
        m_dirFlag = ENT_MOVE_BACKWARD;
    }
    else if(m_command.move[1] == 1)
    {
        m_dirFlag = ENT_MOVE_RIGHT;
    }
    else if(m_command.move[1] == -1)
    {
        m_dirFlag = ENT_MOVE_LEFT;
    }
    else
    {
        m_dirFlag = ENT_STAY;
    }

    if(m_moveType == MOVE_UNDERWATER)
    {
        m_angles[1] = -45.0;
        m_command.rot[1] = 0.0;
        updateTransform();
        setAnimation(TR_ANIMATION_LARA_FREE_FALL_TO_UNDERWATER, 0);
    }
    else if(m_command.action && m_heightInfo.ceiling_climb && (m_heightInfo.ceiling_hit) && (m_transform.getOrigin()[2] + m_bf.bb_max[2] > m_heightInfo.ceiling_point[2] - 64.0))
    {
        ss_anim->next_state = TR_STATE_LARA_MONKEYSWING_IDLE;
        ss_anim->onFrame = &Character::toMonkeySwing;
    }
    else if(m_command.action && (m_moveType == MOVE_CLIMBING))
    {
        ss_anim->next_state = TR_STATE_LARA_HANG;
        setAnimation(TR_ANIMATION_LARA_HANG_IDLE, -1);
    }
    else if(m_response.floor_collision || (m_moveType == MOVE_ON_FLOOR))
    {
        ss_anim->next_state = TR_STATE_LARA_STOP;                        // landing immediately
    }
    else
    {
        if(m_speed[2] < -FREE_FALL_SPEED_2)                 // next free fall stage
        {
            m_moveType = MOVE_FREE_FALLING;
            ss_anim->next_state = TR_STATE_LARA_FREEFALL;
        }
    }
}

void Character::stateLaraReach(SSAnimation* ss_anim, HeightInfo& next_fc)
{
    m_command.rot[0] = 0.0;
    if(m_moveType == MOVE_UNDERWATER)
    {
        m_angles[1] = -45.0;
        m_command.rot[1] = 0.0;
        updateTransform();
        setAnimation(TR_ANIMATION_LARA_FREE_FALL_TO_UNDERWATER, 0);
        return;
    }

    if(m_command.action && (m_moveType == MOVE_FREE_FALLING))
    {
        btScalar t = LARA_TRY_HANG_WALL_OFFSET + LARA_HANG_WALL_DISTANCE;
        btVector3 global_offset = m_transform.getBasis().getColumn(1) * t;
        global_offset[2] += m_bf.bb_max[2] + LARA_HANG_VERTICAL_EPSILON + engine_frame_time * m_speed[2];
        m_climb = checkClimbability(global_offset, &next_fc, 0.0);
        if(m_climb.edge_hit != ClimbType::NoClimb && m_climb.can_hang)
        {
            m_climb.point = m_climb.edge_point;
            m_angles[0] = m_climb.edge_z_ang;
            updateTransform();
            m_moveType = MOVE_CLIMBING;                             // hang on
            m_speed.setZero();
        }

        // If Lara is moving backwards off the ledge we want to move Lara slightly forwards
        // depending on the current angle.
        if((m_dirFlag == ENT_MOVE_BACKWARD) && (m_moveType == MOVE_CLIMBING))
        {
            m_transform.getOrigin()[0] = m_climb.point[0] - m_transform.getBasis().getColumn(1)[0] * (m_forwardSize + 16.0);
            m_transform.getOrigin()[1] = m_climb.point[1] - m_transform.getBasis().getColumn(1)[1] * (m_forwardSize + 16.0);
        }
    }

    if(((m_moveType != MOVE_ON_FLOOR)) && m_command.action && m_heightInfo.ceiling_climb && (m_heightInfo.ceiling_hit) && (m_transform.getOrigin()[2] + m_bf.bb_max[2] > m_heightInfo.ceiling_point[2] - 64.0))
    {
        ss_anim->next_state = TR_STATE_LARA_MONKEYSWING_IDLE;
        ss_anim->onFrame = &Character::toMonkeySwing;
        return;
    }
    if((m_response.floor_collision || (m_moveType == MOVE_ON_FLOOR)) && (!m_command.action || !m_climb.can_hang))
    {
        ss_anim->next_state = TR_STATE_LARA_STOP;                       // middle landing
        return;
    }

    if((m_speed[2] < -FREE_FALL_SPEED_2))
    {
        m_moveType = MOVE_FREE_FALLING;
        ss_anim->next_state = TR_STATE_LARA_FREEFALL;
        return;
    }

    if(m_moveType == MOVE_CLIMBING)
    {
        m_speed.setZero();
        ss_anim->next_state = TR_STATE_LARA_HANG;
        ss_anim->onFrame = &Character::toEdgeClimb;
#if OSCILLATE_HANG_USE
        move = ent->transform.getBasis().getColumn(1) * PENETRATION_TEST_OFFSET;
        if(Entity_CheckNextPenetration(ent, cmd, move) == 0)
        {
            ent->setAnimation(TR_ANIMATION_LARA_OSCILLATE_HANG_ON, 0);
            &Character::toEdgeClimb(ent);
        }
#endif
    }
}

void Character::stateLaraFixClimbEnd(SSAnimation* ss_anim)
{
    m_command.rot[0] = 0;
    m_bt.no_fix_all = true;
    ss_anim->onFrame = &Character::setOnFloorAfterClimb;
}

void Character::stateLaraHang(SSAnimation* ss_anim, HeightInfo& next_fc)
{
    m_command.rot[0] = 0.0;

    if(m_moveType == MOVE_WALLS_CLIMB)
    {
        if(m_command.action)
        {
            if((m_climb.wall_hit == ClimbType::FullClimb) && (m_command.move[0] == 0) && (m_command.move[1] == 0))
            {
                ss_anim->next_state = TR_STATE_LARA_LADDER_IDLE;
            }
            else if(m_command.move[0] == 1)             // UP
            {
                setAnimation(TR_ANIMATION_LARA_LADDER_UP_HANDS, 0);
            }
            else if(m_command.move[0] ==-1)             // DOWN
            {
                setAnimation(TR_ANIMATION_LARA_LADDER_DOWN_HANDS, 0);
            }
            else if(m_command.move[1] == 1)
            {
                m_dirFlag = ENT_MOVE_RIGHT;
                setAnimation(TR_ANIMATION_LARA_CLIMB_RIGHT, 0); // edge climb right
            }
            else if(m_command.move[1] ==-1)
            {
                m_dirFlag = ENT_MOVE_LEFT;
                setAnimation(TR_ANIMATION_LARA_CLIMB_LEFT, 0); // edge climb left
            }
            else if(m_climb.wall_hit == ClimbType::NoClimb)
            {
                m_moveType = MOVE_FREE_FALLING;
                setAnimation(TR_ANIMATION_LARA_STOP_HANG_VERTICAL, 0); // fall down
            }
            else
            {
                ss_anim->anim_flags = ANIM_LOOP_LAST_FRAME;             // disable shake
            }
        }
        else
        {
            m_moveType = MOVE_FREE_FALLING;
            setAnimation(TR_ANIMATION_LARA_TRY_HANG_VERTICAL, 0); // fall down
        }
        return;
    }

    if(!m_response.killed && m_command.action)                         // we have to update climb point every time so entity can move
    {
        btScalar t = LARA_TRY_HANG_WALL_OFFSET + LARA_HANG_WALL_DISTANCE;
        btVector3 global_offset = m_transform.getBasis().getColumn(1) * t;
        global_offset[2] += m_bf.bb_max[2] + LARA_HANG_VERTICAL_EPSILON;
        m_climb = checkClimbability(global_offset, &next_fc, 0.0);
        if(m_climb.can_hang)
        {
            m_climb.point = m_climb.edge_point;
            m_angles[0] = m_climb.edge_z_ang;
            updateTransform();
            m_moveType = MOVE_CLIMBING;                             // hang on
        }
    }
    else
    {
        m_moveType = MOVE_FREE_FALLING;
        setAnimation(TR_ANIMATION_LARA_TRY_HANG_VERTICAL, 0); // fall down
        return;
    }

    if(m_moveType == MOVE_CLIMBING)
    {
        if(m_command.move[0] == 1)
        {
            if(m_climb.edge_hit != ClimbType::NoClimb && (m_climb.next_z_space >= 512.0) && ((m_climb.next_z_space < m_height - LARA_HANG_VERTICAL_EPSILON) || m_command.crouch))
            {
                m_climb.point = m_climb.edge_point;
                ss_anim->next_state = TR_STATE_LARA_CLIMB_TO_CRAWL;     // crawlspace climb
            }
            else if(m_climb.edge_hit != ClimbType::NoClimb && (m_climb.next_z_space >= m_height - LARA_HANG_VERTICAL_EPSILON))
            {
                Sys_DebugLog(LOG_FILENAME, "Zspace = %f", m_climb.next_z_space);
                m_climb.point = m_climb.edge_point;
                ss_anim->next_state = (m_command.shift)?(TR_STATE_LARA_HANDSTAND):(TR_STATE_LARA_CLIMBING);               // climb up
            }
            else
            {
                m_transform.getOrigin()[0] = m_climb.point[0] - (LARA_HANG_WALL_DISTANCE) * m_transform.getBasis().getColumn(1)[0];
                m_transform.getOrigin()[1] = m_climb.point[1] - (LARA_HANG_WALL_DISTANCE) * m_transform.getBasis().getColumn(1)[1];
                m_transform.getOrigin()[2] = m_climb.point[2] - m_bf.bb_max[2] + LARA_HANG_VERTICAL_OFFSET;
                m_speed.setZero();
                ss_anim->anim_flags = ANIM_LOOP_LAST_FRAME;             // disable shake
            }
        }
        else if(m_command.move[0] ==-1)                                      // check walls climbing
        {
            m_climb = checkWallsClimbability();
            if(m_climb.wall_hit != ClimbType::NoClimb)
            {
                m_moveType = MOVE_WALLS_CLIMB;
            }
            ss_anim->anim_flags = ANIM_LOOP_LAST_FRAME;                 // disable shake
        }
        else if(m_command.move[1] ==-1)
        {
            btVector3 move = m_transform.getBasis().getColumn(0) * -PENETRATION_TEST_OFFSET;
            if((checkNextPenetration(move) == 0) || !m_response.horizontal_collision) //we only want lara to shimmy when last frame is reached!
            {
                m_moveType = ENT_MOVE_LEFT;
                setAnimation(TR_ANIMATION_LARA_CLIMB_LEFT, 0);
            }
            else
            {
                ss_anim->anim_flags = ANIM_LOOP_LAST_FRAME;             // disable shake
            }
        }
        else if(m_command.move[1] == 1)
        {
            btVector3 move = m_transform.getBasis().getColumn(0) * PENETRATION_TEST_OFFSET;
            if((checkNextPenetration(move) == 0) || !m_response.horizontal_collision) //we only want lara to shimmy when last frame is reached!
            {
                m_moveType = ENT_MOVE_RIGHT;
                setAnimation(TR_ANIMATION_LARA_CLIMB_RIGHT, 0);
            }
            else
            {
                ss_anim->anim_flags = ANIM_LOOP_LAST_FRAME;             // disable shake
            }
        }
        else
        {
            ss_anim->anim_flags = ANIM_LOOP_LAST_FRAME;                 // disable shake
            m_transform.getOrigin()[0] = m_climb.point[0] - (LARA_HANG_WALL_DISTANCE) * m_transform.getBasis().getColumn(1)[0];
            m_transform.getOrigin()[1] = m_climb.point[1] - (LARA_HANG_WALL_DISTANCE) * m_transform.getBasis().getColumn(1)[1];
            m_transform.getOrigin()[2] = m_climb.point[2] - m_bf.bb_max[2] + LARA_HANG_VERTICAL_OFFSET;
            m_speed.setZero();
        }
    }
    else if(m_command.action && m_heightInfo.ceiling_climb && (m_heightInfo.ceiling_hit) && (m_transform.getOrigin()[2] + m_bf.bb_max[2] > m_heightInfo.ceiling_point[2] - 64.0))
    {
        ss_anim->next_state = TR_STATE_LARA_MONKEYSWING_IDLE;
        ss_anim->onFrame = &Character::toMonkeySwing;
    }
    else
    {
        m_moveType = MOVE_FREE_FALLING;
        setAnimation(TR_ANIMATION_LARA_TRY_HANG_VERTICAL, 0); // fall down
    }
}

void Character::stateLaraLadderIdle(SSAnimation* ss_anim, HeightInfo& next_fc)
{
    m_command.rot[0] = 0;
    m_moveType = MOVE_WALLS_CLIMB;
    m_dirFlag = ENT_STAY;
    m_camFollowCenter = 64;
    if(m_moveType == MOVE_CLIMBING)
    {
        ss_anim->next_state = TR_STATE_LARA_CLIMBING;
        return;
    }
    if(!m_command.action)
    {
        m_moveType = MOVE_FREE_FALLING;
        setAnimation(TR_ANIMATION_LARA_STOP_HANG_VERTICAL, 0); // fall down
    }
    else if(m_command.jump)
    {
        ss_anim->next_state = TR_STATE_LARA_JUMP_BACK;
        m_dirFlag = ENT_MOVE_BACKWARD;
    }
    else if(m_command.move[0] == 1)
    {
        btScalar t = LARA_TRY_HANG_WALL_OFFSET + LARA_HANG_WALL_DISTANCE;
        btVector3 global_offset = m_transform.getBasis().getColumn(1) * t;
        global_offset[2] += m_bf.bb_max[2] + LARA_HANG_VERTICAL_EPSILON;
        m_climb = checkClimbability(global_offset, &next_fc, 0.0);
        if(m_climb.edge_hit != ClimbType::NoClimb && (m_climb.next_z_space >= 512.0))
        {
            m_moveType = MOVE_CLIMBING;
            ss_anim->next_state = TR_STATE_LARA_CLIMBING;
        }
        else if((!m_heightInfo.ceiling_hit) || (m_transform.getOrigin()[2] + m_bf.bb_max[2] < m_heightInfo.ceiling_point[2]))
        {
            ss_anim->next_state = TR_STATE_LARA_LADDER_UP;
        }
    }
    else if(m_command.move[0] == -1)
    {
        ss_anim->next_state = TR_STATE_LARA_LADDER_DOWN;
    }
    else if(m_command.move[1] == 1)
    {
        ss_anim->next_state = TR_STATE_LARA_LADDER_RIGHT;
    }
    else if(m_command.move[1] == -1)
    {
        ss_anim->next_state = TR_STATE_LARA_LADDER_LEFT;
    }
}

void Character::stateLaraLadderLeftRight(SSAnimation* ss_anim)
{
    m_dirFlag = ss_anim->last_state == TR_STATE_LARA_LADDER_LEFT
              ? ENT_MOVE_LEFT
              : ENT_MOVE_RIGHT;
    if(!m_command.action || (m_climb.wall_hit == ClimbType::NoClimb))
    {
        ss_anim->next_state = TR_STATE_LARA_HANG;
    }
    else
    {
        ss_anim->next_state = TR_STATE_LARA_LADDER_IDLE;
    }
}

void Character::stateLaraLadderUp(SSAnimation* ss_anim, HeightInfo& next_fc)
{
    m_camFollowCenter = 64;
    if(m_moveType == MOVE_CLIMBING)
    {
        ss_anim->next_state = TR_STATE_LARA_LADDER_IDLE;
        return;
    }

    if(m_command.action && m_climb.wall_hit != ClimbType::NoClimb)
    {
        btScalar t = LARA_TRY_HANG_WALL_OFFSET + LARA_HANG_WALL_DISTANCE;
        btVector3 global_offset = m_transform.getBasis().getColumn(1) * t;
        global_offset[2] += m_bf.bb_max[2] + LARA_HANG_VERTICAL_EPSILON;
        m_climb = checkClimbability(global_offset, &next_fc, 0.0);
        if(m_climb.edge_hit != ClimbType::NoClimb && (m_climb.next_z_space >= 512.0))
        {
            m_moveType = MOVE_CLIMBING;
            ss_anim->next_state = TR_STATE_LARA_LADDER_IDLE;
        }
        else if((m_command.move[0] <= 0) && (m_heightInfo.ceiling_hit || (m_transform.getOrigin()[2] + m_bf.bb_max[2] >= m_heightInfo.ceiling_point[2])))
        {
            ss_anim->next_state = TR_STATE_LARA_LADDER_IDLE;
        }

        if(m_heightInfo.ceiling_hit && (m_transform.getOrigin()[2] + m_bf.bb_max[2] > m_heightInfo.ceiling_point[2]))
        {
            m_transform.getOrigin()[2] = m_heightInfo.ceiling_point[2] - m_bf.bb_max[2];
        }
    }
    else
    {
        // Free fall after stop
        ss_anim->next_state = TR_STATE_LARA_LADDER_IDLE;
    }
}

void Character::stateLaraLadderDown(SSAnimation* ss_anim)
{
    m_camFollowCenter = 64;
    if(m_command.action && m_climb.wall_hit != ClimbType::NoClimb && (m_command.move[1] < 0))
    {
        if(m_climb.wall_hit != ClimbType::FullClimb)
        {
            ss_anim->next_state = TR_STATE_LARA_LADDER_IDLE;
        }
    }
    else
    {
        ss_anim->next_state = TR_STATE_LARA_LADDER_IDLE;
    }
}

void Character::stateLaraShimmyLeftRight(SSAnimation* ss_anim, HeightInfo& next_fc)
{
    m_bt.no_fix_body_parts = BODY_PART_LEGS_1 | BODY_PART_LEGS_2 | BODY_PART_LEGS_3;

    m_command.rot[0] = 0.0;
    m_dirFlag = ss_anim->last_state == TR_STATE_LARA_SHIMMY_LEFT
              ? ENT_MOVE_LEFT
              : ENT_MOVE_RIGHT;
    if(!m_command.action)
    {
        m_speed.setZero();
        m_moveType = MOVE_FREE_FALLING;
        setAnimation(TR_ANIMATION_LARA_TRY_HANG_VERTICAL, 0); // fall down
        return;
    }

    if(m_moveType == MOVE_WALLS_CLIMB)
    {
        if(m_climb.wall_hit == ClimbType::NoClimb)
        {
            m_moveType = MOVE_FREE_FALLING;
            setAnimation(TR_ANIMATION_LARA_STOP_HANG_VERTICAL, 0); // fall down
        }
    }
    else
    {
        btScalar t = LARA_TRY_HANG_WALL_OFFSET + LARA_HANG_WALL_DISTANCE;
        btVector3 global_offset = m_transform.getBasis().getColumn(1) * t;
        global_offset[2] += LARA_HANG_SENSOR_Z + LARA_HANG_VERTICAL_EPSILON;
        m_climb = checkClimbability(global_offset, &next_fc, 0.0);
        if(m_climb.edge_hit != ClimbType::NoClimb)
        {
            m_climb.point = m_climb.edge_point;
            m_angles[0] = m_climb.edge_z_ang;
            updateTransform();
            m_moveType = MOVE_CLIMBING;                             // hang on
            m_transform.getOrigin()[0] = m_climb.point[0] - (LARA_HANG_WALL_DISTANCE) * m_transform.getBasis().getColumn(1)[0];
            m_transform.getOrigin()[1] = m_climb.point[1] - (LARA_HANG_WALL_DISTANCE) * m_transform.getBasis().getColumn(1)[1];
            m_transform.getOrigin()[2] = m_climb.point[2] - m_bf.bb_max[2] + LARA_HANG_VERTICAL_OFFSET;
            m_speed.setZero();
        }
        else
        {
            m_moveType = MOVE_FREE_FALLING;
            setAnimation(TR_ANIMATION_LARA_STOP_HANG_VERTICAL, 0); // fall down
            return;
        }
    }

    int moveCommand = ss_anim->last_state == TR_STATE_LARA_SHIMMY_LEFT
                    ? -1
                    : 1;
    if(m_command.move[1] == moveCommand)
    {
        btVector3 move = moveCommand * m_transform.getBasis().getColumn(0) * PENETRATION_TEST_OFFSET;
        if((checkNextPenetration(move) > 0) && m_response.horizontal_collision)
        {
            ss_anim->next_state = TR_STATE_LARA_HANG;
        }
    }
    else
    {
        ss_anim->next_state = TR_STATE_LARA_HANG;
    }
}

void Character::stateLaraOnWaterExit(SSAnimation* ss_anim)
{
    m_command.rot[0] *= 0.0;
    m_bt.no_fix_all = true;
    ss_anim->onFrame = &Character::setOnFloorAfterClimb;
}

void Character::stateLaraJumpForwardFallBackward(SSAnimation* ss_anim)
{
    m_bt.no_fix_body_parts = BODY_PART_LEGS_1 | BODY_PART_LEGS_2 | BODY_PART_LEGS_3;
    lean(4.0);

    if(m_response.floor_collision || (m_moveType == MOVE_ON_FLOOR))
    {
        if(m_self->room->flags & TR_ROOM_FLAG_QUICKSAND)
        {
            setAnimation(TR_ANIMATION_LARA_STAY_IDLE, 0);
        }
        else if(!m_command.action && (m_command.move[0] == 1) && !m_command.crouch)
        {
            m_moveType = MOVE_ON_FLOOR;
            ss_anim->next_state = TR_STATE_LARA_RUN_FORWARD;
        }
        else
        {
            ss_anim->next_state = TR_STATE_LARA_STOP;
        }
    }
    else if(m_moveType == MOVE_UNDERWATER)
    {
        m_angles[1] = -45.0;
        m_command.rot[1] = 0.0;
        updateTransform();
        setAnimation(TR_ANIMATION_LARA_FREE_FALL_TO_UNDERWATER, 0);
    }
    else if(m_response.horizontal_collision)
    {
        setAnimation(TR_ANIMATION_LARA_SMASH_JUMP, 0);
        m_dirFlag = ENT_MOVE_BACKWARD;
        updateCurrentSpeed(true);
    }
    else if(m_speed[2] <= -FREE_FALL_SPEED_2)
    {
        ss_anim->next_state = TR_STATE_LARA_FREEFALL;                    // free falling
    }
    else if(m_command.action)
    {
        ss_anim->next_state = TR_STATE_LARA_REACH;
    }
    else if(m_command.shift)
    {
        ss_anim->next_state = TR_STATE_LARA_SWANDIVE_BEGIN;              // fly like fish
    }
    else if(m_speed[2] <= -FREE_FALL_SPEED_2)
    {
        ss_anim->next_state = TR_STATE_LARA_FREEFALL;                    // free falling
    }
    else if(m_command.roll)
    {
        ss_anim->next_state = TR_STATE_LARA_JUMP_ROLL;
    }
}

void Character::stateLaraUnderwaterDiving(SSAnimation* ss_anim)
{
    m_angles[1] = -45.0;
    m_command.rot[1] = 0.0;
    updateTransform();
    ss_anim->onFrame = &Character::correctDivingAngle;
}

void Character::stateLaraFreefall(SSAnimation* ss_anim)
{
    lean(1.0);

    if( (int(m_speed[2]) <=  -FREE_FALL_SPEED_CRITICAL) &&
        (int(m_speed[2]) >= (-FREE_FALL_SPEED_CRITICAL-100)) )
    {
        m_speed[2] = -FREE_FALL_SPEED_CRITICAL-101;
        Audio_Send(TR_AUDIO_SOUND_LARASCREAM, TR_AUDIO_EMITTER_ENTITY, id());       // Scream
    }
    else if(m_speed[2] <= -FREE_FALL_SPEED_MAXSAFE)
    {
        //Reset these to zero so Lara is only falling downwards
        m_speed[0] = 0.0;
        m_speed[1] = 0.0;
    }

    if(m_moveType == MOVE_UNDERWATER)
    {
        m_angles[1] = -45.0;
        m_command.rot[1] = 0.0;
        updateTransform();                                     // needed here to fix underwater in wall collision bug
        setAnimation(TR_ANIMATION_LARA_FREE_FALL_TO_UNDERWATER, 0);
        Audio_Kill(TR_AUDIO_SOUND_LARASCREAM, TR_AUDIO_EMITTER_ENTITY, id());       // Stop scream

        // Splash sound is hardcoded, beginning with TR3.
        if(engine_world.version > TR_II)
        {
            Audio_Send(TR_AUDIO_SOUND_SPLASH, TR_AUDIO_EMITTER_ENTITY, id());
        }
    }
    else if(m_response.floor_collision || (m_moveType == MOVE_ON_FLOOR))
    {
        if(m_self->room->flags & TR_ROOM_FLAG_QUICKSAND)
        {
            setAnimation(TR_ANIMATION_LARA_STAY_IDLE, 0);
            Audio_Kill(TR_AUDIO_SOUND_LARASCREAM, TR_AUDIO_EMITTER_ENTITY, id());
        }
        else if(m_speed[2] <= -FREE_FALL_SPEED_MAXSAFE)
        {
            if(!changeParam(PARAM_HEALTH, (m_speed[2] + FREE_FALL_SPEED_MAXSAFE) / 2))
            {
                m_response.killed = true;
                setAnimation(TR_ANIMATION_LARA_LANDING_DEATH, 0);
                Controls_JoyRumble(200.0, 500);
            }
            else
            {
                setAnimation(TR_ANIMATION_LARA_LANDING_HARD, 0);
            }
        }
        else if(m_speed[2] <= -FREE_FALL_SPEED_2)
        {
            setAnimation(TR_ANIMATION_LARA_LANDING_HARD, 0);
        }
        else
        {
            setAnimation(TR_ANIMATION_LARA_LANDING_MIDDLE, 0);
        }

        if(m_response.killed)
        {
            ss_anim->next_state = TR_STATE_LARA_DEATH;
            Audio_Kill(TR_AUDIO_SOUND_LARASCREAM, TR_AUDIO_EMITTER_ENTITY, id());
        }
    }
    else if(m_command.action)
    {
        m_dirFlag = ENT_MOVE_FORWARD;
        ss_anim->next_state = TR_STATE_LARA_REACH;
    }
}

void Character::stateLaraSwandiveBegin(SSAnimation* ss_anim)
{
    m_command.rot[0] *= 0.4;
    if(m_response.floor_collision || m_moveType == MOVE_ON_FLOOR)
    {
        ss_anim->next_state = TR_STATE_LARA_STOP;                        // landing - roll
    }
    else if(m_moveType == MOVE_UNDERWATER)
    {
        ss_anim->next_state = TR_STATE_LARA_UNDERWATER_DIVING;
    }
    else
    {
        ss_anim->next_state = TR_STATE_LARA_SWANDIVE_END;                // next stage
    }
}

void Character::stateLaraSwandiveEnd(SSAnimation* ss_anim)
{
    m_command.rot[0] = 0.0;

    //Reset these to zero so Lara is only falling downwards
    m_speed[0] = 0.0;
    m_speed[1] = 0.0;

    if(m_response.floor_collision || (m_moveType == MOVE_ON_FLOOR))
    {
        if(m_heightInfo.quicksand)
        {
            m_response.killed = true;
            setParam(PARAM_HEALTH, 0.0);
            setParam(PARAM_AIR, 0.0);
            setAnimation(TR_ANIMATION_LARA_LANDING_DEATH, -1);
        }
        else
        {
            setParam(PARAM_HEALTH, 0.0);
            ss_anim->next_state = TR_STATE_LARA_DEATH;
        }
    }
    else if(m_moveType == MOVE_UNDERWATER)
    {
        ss_anim->next_state = TR_STATE_LARA_UNDERWATER_DIVING;
    }
    else if(m_command.jump)
    {
        ss_anim->next_state = TR_STATE_LARA_JUMP_ROLL;
    }
}

void Character::stateLaraUnderwaterStop(SSAnimation* ss_anim)
{
    if(m_moveType != MOVE_UNDERWATER && m_moveType != MOVE_ON_WATER)
    {
        setAnimation(0, 0);
    }
    else if(m_response.killed)
    {
        ss_anim->next_state = TR_STATE_LARA_WATER_DEATH;
    }
    else if(m_command.roll)
    {
        setAnimation(TR_ANIMATION_LARA_UNDERWATER_ROLL_BEGIN, 0);
    }
    else if(m_command.jump)
    {
        ss_anim->next_state = TR_STATE_LARA_UNDERWATER_FORWARD;
    }
}

void Character::stateLaraUnderwaterDeath()
{
    if(m_moveType != MOVE_ON_WATER)
    {
        m_transform.getOrigin()[2] += (TR_METERING_SECTORSIZE / 4) * engine_frame_time;     // go to the air
    }
}

void Character::stateLaraUnderwaterForward(SSAnimation* ss_anim)
{
    if(m_moveType != MOVE_UNDERWATER && m_moveType != MOVE_ON_WATER)
    {
        setAnimation(0, 0);
    }
    else if(m_response.killed)
    {
        ss_anim->next_state = TR_STATE_LARA_WATER_DEATH;
    }
    else if(m_heightInfo.floor_hit && m_heightInfo.water && (m_heightInfo.transition_level - m_heightInfo.floor_point[2] <= m_maxStepUpHeight))
    {
        setAnimation(TR_ANIMATION_LARA_UNDERWATER_TO_WADE, 0); // go to the air
        ss_anim->next_state = TR_STATE_LARA_STOP;
        m_climb.point = m_heightInfo.floor_point;  ///@FIXME: without it Lara are pulled high up, but this string was not been here.
        m_moveType = MOVE_ON_FLOOR;
    }
    else if(m_command.roll)
    {
        setAnimation(TR_ANIMATION_LARA_UNDERWATER_ROLL_BEGIN, 0);
    }
    else if(m_command.jump)
    {
        if(m_moveType == MOVE_ON_WATER)
        {
            m_inertiaLinear = 0.0;
            ss_anim->next_state = TR_STATE_LARA_ONWATER_STOP;
            setAnimation(TR_ANIMATION_LARA_UNDERWATER_TO_ONWATER, 0); // go to the air
        }
    }
    else
    {
        ss_anim->next_state = TR_STATE_LARA_UNDERWATER_INERTIA;
    }
}

void Character::stateLaraUnderwaterInertia(SSAnimation* ss_anim)
{
    if(m_moveType == MOVE_ON_WATER)
    {
        m_inertiaLinear = 0.0;
        setAnimation(TR_ANIMATION_LARA_UNDERWATER_TO_ONWATER, 0); // go to the air
    }
    else if(m_response.killed)
    {
        ss_anim->next_state = TR_STATE_LARA_WATER_DEATH;
    }
    else if(m_command.roll)
    {
        setAnimation(TR_ANIMATION_LARA_UNDERWATER_ROLL_BEGIN, 0);
    }
    else if(m_command.jump)
    {
        ss_anim->next_state = TR_STATE_LARA_UNDERWATER_FORWARD;
    }
    else
    {
        ss_anim->next_state = TR_STATE_LARA_UNDERWATER_STOP;
    }
}

void Character::stateLaraOnwaterStop(SSAnimation* ss_anim, HeightInfo& next_fc, bool low_vertical_space)
{
    if(m_command.action && (m_command.move[0] == 1) && (m_moveType != MOVE_CLIMBING))
    {
        btScalar t = LARA_TRY_HANG_WALL_OFFSET + LARA_HANG_WALL_DISTANCE;
        btVector3 global_offset = m_transform.getBasis().getColumn(1) * t;
        global_offset[2] += LARA_HANG_VERTICAL_EPSILON;                        // inc for water_surf.z
        m_climb = checkClimbability(global_offset, &next_fc, 0.0);
        if(m_climb.edge_hit != ClimbType::NoClimb)
        {
            low_vertical_space = true;
        }
        else
        {
            low_vertical_space = false;
            global_offset[2] += m_maxStepUpHeight + LARA_HANG_VERTICAL_EPSILON;
            m_climb = checkClimbability(global_offset, &next_fc, 0.0);
        }

        if(m_climb.edge_hit != ClimbType::NoClimb && (m_climb.next_z_space >= m_height - LARA_HANG_VERTICAL_EPSILON))// && (m_climb.edge_point[2] - m_transform.getOrigin()[2] < ent->max_step_up_height))   // max_step_up_height is not correct value here
        {
            m_dirFlag = ENT_STAY;
            m_moveType = MOVE_CLIMBING;
            m_bt.no_fix_all = true;
            m_angles[0] = m_climb.edge_z_ang;
            updateTransform();
            m_climb.point = m_climb.edge_point;
        }
    }

    if(m_moveType == MOVE_CLIMBING)
    {
        m_speed.setZero();
        m_command.rot[0] = 0.0;
        m_bt.no_fix_all = true;
        if(low_vertical_space)
        {
            setAnimation(TR_ANIMATION_LARA_ONWATER_TO_LAND_LOW, 0);
            climbOutOfWater(ss_anim, ENTITY_ANIM_NEWANIM);
        }
        else
        {
            ss_anim->next_state = TR_STATE_LARA_STOP;
            ss_anim->onFrame = &Character::climbOutOfWater;
        }
    }
    else if(m_response.killed)
    {
        ss_anim->next_state = TR_STATE_LARA_WATER_DEATH;
    }
    else if((m_command.move[0] == 1) || m_command.jump)                    // dive works correct only after TR_STATE_LARA_ONWATER_FORWARD
    {
        m_dirFlag = ENT_MOVE_FORWARD;
        ss_anim->next_state = TR_STATE_LARA_ONWATER_FORWARD;
    }
    else if(m_command.move[0] ==-1)
    {
        m_dirFlag = ENT_MOVE_BACKWARD;
        ss_anim->next_state = TR_STATE_LARA_ONWATER_BACK;
    }
    else if(m_command.move[1] ==-1)
    {
        if(m_command.shift)
        {
            m_dirFlag = ENT_MOVE_LEFT;
            m_command.rot[0] = 0.0;
            ss_anim->next_state = TR_STATE_LARA_ONWATER_LEFT;
        }
        else
        {
            // rotate on water
        }
    }
    else if(m_command.move[1] == 1)
    {
        if(m_command.shift)
        {
            m_dirFlag = ENT_MOVE_RIGHT;
            m_command.rot[0] = 0.0;
            ss_anim->next_state = TR_STATE_LARA_ONWATER_RIGHT;
        }
        else
        {
            // rotate on water
        }
    }
    else if(m_moveType == MOVE_UNDERWATER)
    {
        m_moveType = MOVE_ON_WATER;
    }
}

void Character::stateLaraOnwaterForward(SSAnimation* ss_anim, HeightInfo& next_fc)
{
    m_moveType = MOVE_ON_WATER;
    if(m_response.killed)
    {
        ss_anim->next_state = TR_STATE_LARA_WATER_DEATH;
    }
    else if(m_command.jump)
    {
        btScalar t = m_transform.getOrigin()[2];
        Character::getHeightInfo(m_transform.getOrigin(), &next_fc);
        m_transform.getOrigin()[2] = t;
        ss_anim->next_state = TR_STATE_LARA_UNDERWATER_FORWARD;
        ss_anim->onFrame = &Character::setUnderwater;                          // dive
    }
    else if((m_command.move[0] == 1) && !m_command.action)
    {
        if(!m_heightInfo.floor_hit || (m_transform.getOrigin()[2] - m_height > m_heightInfo.floor_point[2]- m_swimDepth))
        {
            //ent->last_state = ent->last_state;                          // swim forward
        }
        else
        {
            ss_anim->next_state = TR_STATE_LARA_WADE_FORWARD;
            ss_anim->onFrame = &Character::setOnFloor;                        // to wade
        }
    }
    else
    {
        ss_anim->next_state = TR_STATE_LARA_ONWATER_STOP;
    }
}

void Character::stateLaraOnwaterBack(SSAnimation* ss_anim)
{
    if(m_command.move[0] == -1 && !m_command.jump)
    {
        if(!m_heightInfo.floor_hit || (m_heightInfo.floor_point[2] + m_height < m_heightInfo.transition_level))
        {
            //ent->current_state = TR_STATE_CURRENT;                      // continue swimming
        }
        else
        {
            ss_anim->next_state = TR_STATE_LARA_ONWATER_STOP;
        }
    }
    else
    {
        ss_anim->next_state = TR_STATE_LARA_ONWATER_STOP;
    }
}

void Character::stateLaraOnwaterLeftRight(SSAnimation* ss_anim)
{
    m_command.rot[0] = 0.0;
    if(!m_command.jump)
    {
        int moveCommand = ss_anim->last_state == TR_STATE_LARA_ONWATER_LEFT
                        ? -1
                        : 1;
        if(m_command.move[1] == moveCommand && m_command.shift)
        {
            if(!m_heightInfo.floor_hit || (m_transform.getOrigin()[2] - m_height > m_heightInfo.floor_point[2]))
            {
                // walk left
                ss_anim->next_state = ss_anim->last_state == TR_STATE_LARA_ONWATER_LEFT
                                    ? TR_STATE_LARA_ONWATER_LEFT
                                    : TR_STATE_LARA_ONWATER_RIGHT;
            }
            else
            {
                // walk left
                ss_anim->next_state = ss_anim->last_state == TR_STATE_LARA_ONWATER_LEFT
                                    ? TR_STATE_LARA_WALK_LEFT
                                    : TR_STATE_LARA_WALK_RIGHT;
                ss_anim->onFrame = &Character::setOnFloor;
            }
        }
        else
        {
            ss_anim->next_state = TR_STATE_LARA_ONWATER_STOP;
        }
    }
    else
    {
        ss_anim->next_state = TR_STATE_LARA_UNDERWATER_DIVING;
    }
}

void Character::stateLaraCrouchIdle(SSAnimation* ss_anim, HeightInfo& next_fc, bool low_vertical_space)
{
    m_dirFlag = ENT_MOVE_FORWARD;
    m_bt.no_fix_body_parts = BODY_PART_HANDS_2 | BODY_PART_HANDS_3 | BODY_PART_LEGS_3;
    btVector3 move = m_transform.getOrigin();
    move[2] += 0.5 * (m_bf.bb_max[2] - m_bf.bb_min[2]);
    Character::getHeightInfo(move, &next_fc);

    lean(0.0);

    if(!m_command.crouch && !low_vertical_space)
    {
        ss_anim->next_state = TR_STATE_LARA_STOP;                        // Back to stand
    }
    else if((m_command.move[0] != 0) || m_response.killed)
    {
        ss_anim->next_state = TR_STATE_LARA_CRAWL_IDLE;                  // Both forward & back provoke crawl stage
    }
    else if(m_command.jump)
    {
        ss_anim->next_state = TR_STATE_LARA_CROUCH_ROLL;                 // Crouch roll
    }
    else
    {
        if(engine_world.version > TR_III)
        {
            if(m_command.move[1] == 1)
            {
                m_dirFlag = ENT_MOVE_FORWARD;
                ss_anim->next_state = TR_STATE_LARA_CROUCH_TURN_RIGHT;
            }
            else if(m_command.move[1] == -1)
            {
                m_dirFlag = ENT_MOVE_FORWARD;
                ss_anim->next_state = TR_STATE_LARA_CROUCH_TURN_LEFT;
            }
        }
        else
        {
            m_command.rot[0] = 0.0;
        }
    }
}

void Character::stateLaraSprintCrouchRoll(SSAnimation* ss_anim)
{
    m_command.rot[0] = 0.0;
    lean(0.0);
    if(m_moveType == MOVE_FREE_FALLING)
    {
        m_speed[0] *= 0.5;
        m_speed[1] *= 0.5;
        setAnimation(TR_ANIMATION_LARA_FREE_FALL_FORWARD, 0);
    }

    btVector3 move = m_transform.getBasis().getColumn(1) * PENETRATION_TEST_OFFSET;
    if((checkNextPenetration(move) > 0) && m_response.horizontal_collision)  // Smash into wall
    {
        ss_anim->next_state = TR_STATE_LARA_STOP;
    }
}

void Character::stateLaraCrawlIdle(SSAnimation* ss_anim, HeightInfo& next_fc)
{
    m_dirFlag = ENT_MOVE_FORWARD;
    m_bt.no_fix_body_parts = BODY_PART_HANDS_2 | BODY_PART_HANDS_3 | BODY_PART_LEGS_3;
    if(m_response.killed)
    {
        m_dirFlag = ENT_STAY;
        ss_anim->next_state = TR_STATE_LARA_DEATH;
    }
    else if(m_command.move[1] == -1)
    {
        m_dirFlag = ENT_MOVE_FORWARD;
        setAnimation(TR_ANIMATION_LARA_CRAWL_TURN_LEFT, 0);
    }
    else if(m_command.move[1] == 1)
    {
        m_dirFlag = ENT_MOVE_FORWARD;
        setAnimation(TR_ANIMATION_LARA_CRAWL_TURN_RIGHT, 0);
    }
    else if(m_command.move[0] == 1)
    {
        btVector3 move = m_transform.getBasis().getColumn(1) * PENETRATION_TEST_OFFSET;
        if((checkNextPenetration(move) == 0) || !m_response.horizontal_collision)
        {
            btVector3 global_offset = m_transform.getBasis().getColumn(1) * CRAWL_FORWARD_OFFSET;
            global_offset[2] += 0.5 * (m_bf.bb_max[2] + m_bf.bb_min[2]);
            global_offset += m_transform.getOrigin();
            Character::getHeightInfo(global_offset, &next_fc);
            if((next_fc.floor_point[2] < m_transform.getOrigin()[2] + m_minStepUpHeight) &&
               (next_fc.floor_point[2] > m_transform.getOrigin()[2] - m_minStepUpHeight))
            {
                ss_anim->next_state = TR_STATE_LARA_CRAWL_FORWARD;           // In TR4+, first state is crawlspace jump
            }
        }
    }
    else if(m_command.move[0] == -1)
    {
        btVector3 move = m_transform.getBasis().getColumn(1) * -PENETRATION_TEST_OFFSET;
        if((checkNextPenetration(move) == 0) || !m_response.horizontal_collision)
        {
            btVector3 global_offset = m_transform.getBasis().getColumn(1) * -CRAWL_FORWARD_OFFSET;
            global_offset[2] += 0.5 * (m_bf.bb_max[2] + m_bf.bb_min[2]);
            global_offset += m_transform.getOrigin();
            Character::getHeightInfo(global_offset, &next_fc);
            if((next_fc.floor_point[2] < m_transform.getOrigin()[2] + m_minStepUpHeight) &&
               (next_fc.floor_point[2] > m_transform.getOrigin()[2] - m_minStepUpHeight))
            {
                m_dirFlag = ENT_MOVE_BACKWARD;
                ss_anim->next_state = TR_STATE_LARA_CRAWL_BACK;
            }
            else if(m_command.action && !m_response.horizontal_collision &&
               (next_fc.floor_point[2] < m_transform.getOrigin()[2] - m_height))
            {
                const btVector3 temp = m_transform.getOrigin();                                       // save entity position
                m_transform.getOrigin()[0] = next_fc.floor_point[0];
                m_transform.getOrigin()[1] = next_fc.floor_point[1];
                global_offset = m_transform.getBasis().getColumn(1) * 0.5 * CRAWL_FORWARD_OFFSET;
                global_offset[2] += 128.0;
                m_heightInfo.floor_hit = next_fc.floor_hit;
                m_heightInfo.floor_point = next_fc.floor_point;
                m_heightInfo.floor_normale = next_fc.floor_normale;
                m_heightInfo.floor_obj = next_fc.floor_obj;
                m_heightInfo.ceiling_hit = next_fc.ceiling_hit;
                m_heightInfo.ceiling_point = next_fc.ceiling_point;
                m_heightInfo.ceiling_normale = next_fc.ceiling_normale;
                m_heightInfo.ceiling_obj = next_fc.ceiling_obj;

                m_climb = checkClimbability(global_offset, &next_fc, 1.5 * m_bf.bb_max[2]);
                m_transform.getOrigin() = temp;                                       // restore entity position
                if(m_climb.can_hang)
                {
                    m_angles[0] = m_climb.edge_z_ang;
                    m_dirFlag = ENT_MOVE_BACKWARD;
                    m_moveType = MOVE_CLIMBING;
                    m_climb.point = m_climb.edge_point;
                    ss_anim->next_state = TR_STATE_LARA_CRAWL_TO_CLIMB;
                }
            }
        }
    }
    else if(!m_command.crouch)
    {
        ss_anim->next_state = TR_STATE_LARA_CROUCH_IDLE;                // Back to crouch.
    }
}

void Character::stateLaraCrawlToClimb(SSAnimation* ss_anim)
{
    m_bt.no_fix_all = true;
    ss_anim->onFrame = &Character::crawlToClimb;
}

void Character::stateLaraCrawlForward(SSAnimation* ss_anim, HeightInfo& next_fc)
{
    m_dirFlag = ENT_MOVE_FORWARD;
    m_bt.no_fix_body_parts = BODY_PART_HANDS_2 | BODY_PART_HANDS_3 | BODY_PART_LEGS_3;
    m_command.rot[0] = m_command.rot[0] * 0.5;
    btVector3 move = m_transform.getBasis().getColumn(1) * PENETRATION_TEST_OFFSET;
    if((checkNextPenetration(move) > 0) && m_response.horizontal_collision)
    {
        m_dirFlag = ENT_STAY;
        setAnimation(TR_ANIMATION_LARA_CRAWL_IDLE, 0);
        return;
    }
    btVector3 global_offset = m_transform.getBasis().getColumn(1) * CRAWL_FORWARD_OFFSET;
    global_offset[2] += 0.5 * (m_bf.bb_max[2] + m_bf.bb_min[2]);
    global_offset += m_transform.getOrigin();
    Character::getHeightInfo(global_offset, &next_fc);

    if((m_command.move[0] != 1) || m_response.killed)
    {
        ss_anim->next_state = TR_STATE_LARA_CRAWL_IDLE; // Stop
    }
    else if( (next_fc.floor_point[2] >= m_transform.getOrigin()[2] + m_minStepUpHeight) ||
             (next_fc.floor_point[2] <= m_transform.getOrigin()[2] - m_minStepUpHeight)  )
    {
        m_dirFlag = ENT_STAY;
        setAnimation(TR_ANIMATION_LARA_CRAWL_IDLE, 0);
    }
}

void Character::stateLaraCrawlBack(SSAnimation* ss_anim, HeightInfo& next_fc)
{
    m_dirFlag = ENT_MOVE_FORWARD;   // Absurd? No, Core Design.
    m_bt.no_fix_body_parts = BODY_PART_HANDS_2 | BODY_PART_HANDS_3 | BODY_PART_LEGS_3;
    m_command.rot[0] = m_command.rot[0] * 0.5;
    btVector3 move = m_transform.getBasis().getColumn(1) * -PENETRATION_TEST_OFFSET;
    if((checkNextPenetration(move) > 0) && m_response.horizontal_collision)
    {
        m_dirFlag = ENT_STAY;
        setAnimation(TR_ANIMATION_LARA_CRAWL_IDLE, 0);
        return;
    }
    btVector3 global_offset = m_transform.getBasis().getColumn(1) * -CRAWL_FORWARD_OFFSET;
    global_offset[2] += 0.5 * (m_bf.bb_max[2] + m_bf.bb_min[2]);
    global_offset += m_transform.getOrigin();
    Character::getHeightInfo(global_offset, &next_fc);
    if((m_command.move[0] != -1) || m_response.killed)
    {
        ss_anim->next_state = TR_STATE_LARA_CRAWL_IDLE; // Stop
    }
    else if( (next_fc.floor_point[2] >= m_transform.getOrigin()[2] + m_minStepUpHeight)   ||
             (next_fc.floor_point[2] <= m_transform.getOrigin()[2] - m_minStepUpHeight)    )
    {
        m_dirFlag = ENT_STAY;
        setAnimation(TR_ANIMATION_LARA_CRAWL_IDLE, 0);
    }
}

void Character::stateLaraCrawlTurnLeftRight(SSAnimation* ss_anim)
{
    m_dirFlag = ENT_MOVE_FORWARD;
    m_bt.no_fix_body_parts = BODY_PART_HANDS_2 | BODY_PART_HANDS_3 | BODY_PART_LEGS_3;
    m_command.rot[0] *= ((ss_anim->current_frame > 3) && (ss_anim->current_frame < 14))?(1.0):(0.0);

    int moveCommand = ss_anim->last_state == TR_STATE_LARA_CRAWL_TURN_LEFT
                    ? -1
                    : 1;

    if((m_command.move[1] != moveCommand) || m_response.killed)
    {
        ss_anim->next_state = TR_STATE_LARA_CRAWL_IDLE; // stop
    }
}

void Character::stateLaraCrouchTurnLeftRight(SSAnimation* ss_anim)
{
    m_bt.no_fix_body_parts = BODY_PART_HANDS_2 | BODY_PART_HANDS_3 | BODY_PART_LEGS_3;
    m_command.rot[0] *= ((ss_anim->current_frame > 3) && (ss_anim->current_frame < 23))?(0.6):(0.0);

    if((m_command.move[1] == 0) || m_response.killed)
    {
        ss_anim->next_state = TR_STATE_LARA_CROUCH_IDLE;
    }
}

void Character::stateLaraMonkeyswingIdle(SSAnimation* ss_anim)
{
    m_command.rot[0] = 0.0;
    m_dirFlag = ENT_STAY;
    ///@FIXME: stick for TR_III+ monkey swing fix... something wrong with anim 150
    if(m_command.action && (m_moveType != MOVE_MONKEYSWING) && m_heightInfo.ceiling_climb && (m_heightInfo.ceiling_hit) && (m_transform.getOrigin()[2] + m_bf.bb_max[2] > m_heightInfo.ceiling_point[2] - 96.0))
    {
        m_moveType = MOVE_MONKEYSWING;
        setAnimation(TR_ANIMATION_LARA_MONKEY_IDLE, 0);
        ss_anim->next_state = TR_STATE_LARA_MONKEYSWING_IDLE;
        m_transform.getOrigin()[2] = m_heightInfo.ceiling_point[2] - m_bf.bb_max[2];
    }

    if((m_moveType != MOVE_MONKEYSWING) || !m_command.action)
    {
        setAnimation(TR_ANIMATION_LARA_TRY_HANG_VERTICAL, 0);
        m_dirFlag = ENT_STAY;
        m_moveType = MOVE_FREE_FALLING;
    }
    else if(m_command.shift && (m_command.move[1] ==-1))
    {
        ss_anim->next_state = TR_STATE_LARA_MONKEYSWING_LEFT;
    }
    else if(m_command.shift && (m_command.move[1] == 1))
    {
        ss_anim->next_state = TR_STATE_LARA_MONKEYSWING_RIGHT;
    }
    else if(m_command.move[0] == 1)
    {
        m_dirFlag = ENT_MOVE_FORWARD;
        ss_anim->next_state = TR_STATE_LARA_MONKEYSWING_FORWARD;
    }
    else if(m_command.move[1] ==-1)
    {
        ss_anim->next_state = TR_STATE_LARA_MONKEYSWING_TURN_LEFT;
    }
    else if(m_command.move[1] == 1)
    {
        ss_anim->next_state = TR_STATE_LARA_MONKEYSWING_TURN_RIGHT;
    }
}

void Character::stateLaraMonkeyswingTurnLeftRight(SSAnimation* ss_anim)
{
    m_command.rot[0] *= 0.5;

    int moveCommand = ss_anim->last_state == TR_STATE_LARA_MONKEYSWING_TURN_LEFT
                    ? -1
                    : 1;

    if((m_moveType != MOVE_MONKEYSWING) || !m_command.action)
    {
        setAnimation(TR_ANIMATION_LARA_TRY_HANG_VERTICAL, 0);
        m_dirFlag = ENT_STAY;
        m_moveType = MOVE_FREE_FALLING;
    }
    else if(m_command.move[1] != moveCommand)
    {
        ss_anim->next_state = TR_STATE_LARA_MONKEYSWING_IDLE;
    }
}

void Character::stateLaraMonkeyswingForward(SSAnimation* ss_anim)
{
    m_command.rot[0] *= 0.45;
    m_dirFlag = ENT_MOVE_FORWARD;

    if((m_moveType != MOVE_MONKEYSWING) || !m_command.action)
    {
        setAnimation(TR_ANIMATION_LARA_TRY_HANG_VERTICAL, 0);
        m_moveType = MOVE_FREE_FALLING;
    }
    else if(m_command.move[0] != 1)
    {
        ss_anim->next_state = TR_STATE_LARA_MONKEYSWING_IDLE;
    }
}

void Character::stateLaraMonkeyswingLeftRight(SSAnimation* ss_anim)
{
    m_command.rot[0] = 0.0;
    m_dirFlag = ENT_MOVE_LEFT;

    int moveCommand = ss_anim->last_state == TR_STATE_LARA_MONKEYSWING_LEFT
                    ? -1
                    : 1;

    if((m_moveType != MOVE_MONKEYSWING) || !m_command.action)
    {
        setAnimation(TR_ANIMATION_LARA_TRY_HANG_VERTICAL, 0);
        m_moveType = MOVE_FREE_FALLING;
    }
    else if(m_command.move[0] != moveCommand)
    {
        ss_anim->next_state = TR_STATE_LARA_MONKEYSWING_IDLE;
    }
}

/**
 * Current animation != current state - use original TR state concept!
 */
int Character::stateControlLara(SSAnimation* ss_anim)
{
    HeightInfo next_fc;
    next_fc.sp = m_heightInfo.sp;
    next_fc.cb = m_rayCb;
    next_fc.cb->m_closestHitFraction = 1.0;
    next_fc.cb->m_collisionObject = nullptr;
    next_fc.ccb = m_convexCb;
    next_fc.ccb->m_closestHitFraction = 1.0;
    next_fc.ccb->m_hitCollisionObject = nullptr;
    m_bt.no_fix_body_parts = 0x00000000;

    ss_anim->anim_flags = ANIM_NORMAL_CONTROL;
    updateCurrentHeight();

    bool low_vertical_space = (m_heightInfo.floor_hit && m_heightInfo.ceiling_hit && (m_heightInfo.ceiling_point[2] - m_heightInfo.floor_point[2] < m_height - LARA_HANG_VERTICAL_EPSILON));
    const bool last_frame = static_cast<int>( ss_anim->model->animations[ss_anim->current_animation].frames.size() ) <= ss_anim->current_frame + 1;

    if(m_response.killed)   // Stop any music, if Lara is dead.
    {
        Audio_EndStreams(TR_AUDIO_STREAM_TYPE_ONESHOT);
        Audio_EndStreams(TR_AUDIO_STREAM_TYPE_CHAT);
    }

 /*
 * - On floor animations
 * - Climbing animations
 * - Landing animations
 * - Free fall animations
 * - Water animations
 */
    switch(ss_anim->last_state)
    {
        /*
         * Base onfloor animations
         */
        case TR_STATE_LARA_STOP:
            stateLaraStop(ss_anim, next_fc, low_vertical_space);
            break;

        case TR_STATE_LARA_JUMP_PREPARE:
            stateLaraJumpPrepare(ss_anim);
            break;

        case TR_STATE_LARA_JUMP_BACK:
            stateLaraJumpBack(ss_anim);
            break;

        case TR_STATE_LARA_JUMP_LEFT:
        case TR_STATE_LARA_JUMP_RIGHT:
            stateLaraJumpLeftRight(ss_anim);
            break;

        case TR_STATE_LARA_RUN_BACK:
            stateLaraRunBack();
            break;


        case TR_STATE_LARA_TURN_LEFT_SLOW:
        case TR_STATE_LARA_TURN_RIGHT_SLOW:
            stateLaraTurnSlow(ss_anim, last_frame);
            break;

        case TR_STATE_LARA_TURN_FAST:
            stateLaraTurnFast(ss_anim);
            break;

            /*
             * RUN AND WALK animations section
             */
        case TR_STATE_LARA_RUN_FORWARD:
            stateLaraRunForward(ss_anim, next_fc, low_vertical_space);
            break;

        case TR_STATE_LARA_SPRINT:
            stateLaraSprint(ss_anim, next_fc, low_vertical_space);
            break;

        case TR_STATE_LARA_WALK_FORWARD:
            stateLaraWalkForward(ss_anim, next_fc, low_vertical_space);
            break;

        case TR_STATE_LARA_WADE_FORWARD:
            stateLaraWadeForward(ss_anim);
            break;

        case TR_STATE_LARA_WALK_BACK:
            stateLaraWalkBack(ss_anim, next_fc);
            break;

        case TR_STATE_LARA_WALK_LEFT:
        case TR_STATE_LARA_WALK_RIGHT:
            stateLaraWalkLeftRight(ss_anim, next_fc);
            break;

            /*
             * Slide animations section
             */
        case TR_STATE_LARA_SLIDE_BACK:
            stateLaraSlideBack(ss_anim);
            break;

        case TR_STATE_LARA_SLIDE_FORWARD:
            stateLaraSlideForward(ss_anim);
            break;

            /*
             * Misk animations
             */
        case TR_STATE_LARA_PUSHABLE_GRAB:
            stateLaraPushableGrab(ss_anim);
            break;

        case TR_STATE_LARA_PUSHABLE_PUSH:
            stateLaraPushablePush(ss_anim);
            break;

        case TR_STATE_LARA_PUSHABLE_PULL:
            stateLaraPushablePull(ss_anim);
            break;

        case TR_STATE_LARA_ROLL_FORWARD:
            break;

        case TR_STATE_LARA_ROLL_BACKWARD:
            stateLaraRollBackward(low_vertical_space);
            break;

        /*
         * Climbing section
         */
        case TR_STATE_LARA_JUMP_UP:
            stateLaraJumpUp(ss_anim, next_fc);
            break;

        case TR_STATE_LARA_REACH:
            stateLaraReach(ss_anim, next_fc);
            break;

        /*other code here prevents to UGLY Lara's move in end of "climb on", do not loose &Character::setOnFloorAfterClimb callback here!*/
        case TR_STATE_LARA_HANDSTAND:
        case TR_STATE_LARA_CLIMBING:
        case TR_STATE_LARA_CLIMB_TO_CRAWL:
            stateLaraFixClimbEnd(ss_anim);
            break;

        case TR_STATE_LARA_HANG:
            stateLaraHang(ss_anim, next_fc);
            break;

        case TR_STATE_LARA_LADDER_IDLE:
            stateLaraLadderIdle(ss_anim, next_fc);
            break;

        case TR_STATE_LARA_LADDER_LEFT:
        case TR_STATE_LARA_LADDER_RIGHT:
            stateLaraLadderLeftRight(ss_anim);
            break;

        case TR_STATE_LARA_LADDER_UP:
            stateLaraLadderUp(ss_anim, next_fc);
            break;

        case TR_STATE_LARA_LADDER_DOWN:
            stateLaraLadderDown(ss_anim);
            break;

        case TR_STATE_LARA_SHIMMY_LEFT:
        case TR_STATE_LARA_SHIMMY_RIGHT:
            stateLaraShimmyLeftRight(ss_anim, next_fc);
            break;

        case TR_STATE_LARA_ONWATER_EXIT:
            stateLaraOnWaterExit(ss_anim);
            break;

        case TR_STATE_LARA_JUMP_FORWARD:
        case TR_STATE_LARA_FALL_BACKWARD:
            stateLaraJumpForwardFallBackward(ss_anim);
            break;

            /*
             * FREE FALL TO UNDERWATER CASES
             */
        case TR_STATE_LARA_UNDERWATER_DIVING:
            stateLaraUnderwaterDiving(ss_anim);
            break;

        case TR_STATE_LARA_FREEFALL:
            stateLaraFreefall(ss_anim);
            break;

        case TR_STATE_LARA_SWANDIVE_BEGIN:
            stateLaraSwandiveBegin(ss_anim);
            break;

        case TR_STATE_LARA_SWANDIVE_END:
            stateLaraSwandiveEnd(ss_anim);
            break;

            /*
             * WATER ANIMATIONS
             */
        case TR_STATE_LARA_UNDERWATER_STOP:
            stateLaraUnderwaterStop(ss_anim);
            break;

        case TR_STATE_LARA_WATER_DEATH:
            stateLaraUnderwaterDeath();
            break;

        case TR_STATE_LARA_UNDERWATER_FORWARD:
            stateLaraUnderwaterForward(ss_anim);
            break;

        case TR_STATE_LARA_UNDERWATER_INERTIA:
            stateLaraUnderwaterInertia(ss_anim);
            break;

        case TR_STATE_LARA_ONWATER_STOP:
            stateLaraOnwaterStop(ss_anim, next_fc, low_vertical_space);
            break;

        case TR_STATE_LARA_ONWATER_FORWARD:
            stateLaraOnwaterForward(ss_anim, next_fc);
            break;

        case TR_STATE_LARA_ONWATER_BACK:
            stateLaraOnwaterBack(ss_anim);
            break;

        case TR_STATE_LARA_ONWATER_LEFT:
        case TR_STATE_LARA_ONWATER_RIGHT:
            stateLaraOnwaterLeftRight(ss_anim);
            break;

            /*
             * CROUCH SECTION
             */
        case TR_STATE_LARA_CROUCH_IDLE:
            stateLaraCrouchIdle(ss_anim, next_fc, low_vertical_space);
            break;

        case TR_STATE_LARA_CROUCH_ROLL:
        case TR_STATE_LARA_SPRINT_ROLL:
            stateLaraSprintCrouchRoll(ss_anim);
            break;

        case TR_STATE_LARA_CRAWL_IDLE:
            stateLaraCrawlIdle(ss_anim, next_fc);
            break;

        case TR_STATE_LARA_CRAWL_TO_CLIMB:
            stateLaraCrawlToClimb(ss_anim);
            break;

        case TR_STATE_LARA_CRAWL_FORWARD:
            stateLaraCrawlForward(ss_anim, next_fc);
            break;

        case TR_STATE_LARA_CRAWL_BACK:
            stateLaraCrawlBack(ss_anim, next_fc);
            break;

        case TR_STATE_LARA_CRAWL_TURN_LEFT:
        case TR_STATE_LARA_CRAWL_TURN_RIGHT:
            stateLaraCrawlTurnLeftRight(ss_anim);
            break;

        case TR_STATE_LARA_CROUCH_TURN_LEFT:
        case TR_STATE_LARA_CROUCH_TURN_RIGHT:
            stateLaraCrouchTurnLeftRight(ss_anim);
            break;

            /*
             * CLIMB MONKEY
             */
        case TR_STATE_LARA_MONKEYSWING_IDLE:
            stateLaraMonkeyswingIdle(ss_anim);
            break;

        case TR_STATE_LARA_MONKEYSWING_TURN_LEFT:
        case TR_STATE_LARA_MONKEYSWING_TURN_RIGHT:
            stateLaraMonkeyswingTurnLeftRight(ss_anim);
            break;

        case TR_STATE_LARA_MONKEYSWING_FORWARD:
            stateLaraMonkeyswingForward(ss_anim);
            break;

        case TR_STATE_LARA_MONKEYSWING_LEFT:
        case TR_STATE_LARA_MONKEYSWING_RIGHT:
            stateLaraMonkeyswingLeftRight(ss_anim);
            break;

            /*
             * intermediate animations are processed automatically.
             */
        default:
        {
            m_command.rot[0] = 0.0;
            if((m_moveType == MOVE_MONKEYSWING) || (m_moveType == MOVE_WALLS_CLIMB))
            {
                if(!m_command.action)
                {
                    setAnimation(TR_ANIMATION_LARA_START_FREE_FALL, 0);
                    m_dirFlag = ENT_STAY;
                    m_moveType = MOVE_FREE_FALLING;
                }
            }
            break;
        }
    };

    /*
     * additional animations control
     */
    switch(ss_anim->current_animation)
    {
        case TR_ANIMATION_LARA_STAY_JUMP_SIDES:
            m_bt.no_fix_body_parts |= BODY_PART_HEAD;
            break;

        case TR_ANIMATION_LARA_TRY_HANG_SOLID:
        case TR_ANIMATION_LARA_FLY_FORWARD_TRY_HANG:
            if((m_moveType == MOVE_FREE_FALLING) && m_command.action &&
               (m_speed[0] * m_transform.getBasis().getColumn(1)[0] + m_speed[1] * m_transform.getBasis().getColumn(1)[1] < 0.0))
            {
                m_speed[0] = -m_speed[0];
                m_speed[1] = -m_speed[1];
            }
            break;
    };

    return 0;
}

void Character::stopTraverse(SSAnimation* ss_anim, int state)
{
    if(state == ENTITY_ANIM_NEWANIM)
    {
        btVector3& pos = m_traversedObject->m_transform.getOrigin();
        int i = pos[0] / TR_METERING_SECTORSIZE;
        pos[0] = i * TR_METERING_SECTORSIZE + 512.0;
        i = pos[1] / TR_METERING_SECTORSIZE;
        pos[1] = i * TR_METERING_SECTORSIZE + 512.0;
        m_traversedObject->updateRigidBody(true);
        m_traversedObject = nullptr;
        ss_anim->onFrame = nullptr;
    }
}

void Character::setOnFloor(SSAnimation* ss_anim, int state)
{
    if(state == ENTITY_ANIM_NEWANIM)
    {
        m_moveType = MOVE_ON_FLOOR;
        m_transform.getOrigin()[2] = m_heightInfo.floor_point[2];
        ghostUpdate();
        ss_anim->onFrame = nullptr;
    }
}

void Character::turnFast(SSAnimation* ss_anim, int state)
{
    if(state == ENTITY_ANIM_NEWANIM)
    {
        m_bf.animations.next_state = TR_STATE_LARA_TURN_FAST;
        ss_anim->onFrame = nullptr;
    }
}

void Character::setOnFloorAfterClimb(SSAnimation* ss_anim, int state)
{
    if(state == ENTITY_ANIM_NEWANIM)
    {
        AnimationFrame* af = &ss_anim->model->animations[ ss_anim->current_animation ];
        if(ss_anim->current_frame >= static_cast<int>(af->frames.size() - 1))
        {
            auto move = m_transform * m_bf.bone_tags[0].full_transform.getOrigin();
            setAnimation(af->next_anim->id, af->next_frame);
            auto p = m_transform * m_bf.bone_tags[0].full_transform.getOrigin();
            move -= p;
            m_transform.getOrigin() += move;
            m_transform.getOrigin()[2] = m_climb.point[2];
            Entity::updateCurrentBoneFrame(&m_bf, &m_transform);
            updateRigidBody(false);
            ghostUpdate();
            m_moveType = MOVE_ON_FLOOR;
            ss_anim->onFrame = nullptr;
        }
    }
}

void Character::setUnderwater(SSAnimation* ss_anim, int state)
{
    if(state == ENTITY_ANIM_NEWANIM)
    {
        m_moveType = MOVE_UNDERWATER;
        ss_anim->onFrame = nullptr;
    }
}

void Character::setFreeFalling(SSAnimation* ss_anim, int state)
{
    if(state == ENTITY_ANIM_NEWANIM)
    {
        m_moveType = MOVE_FREE_FALLING;
        ss_anim->onFrame = nullptr;
    }
}

void Character::setCmdSlide(SSAnimation* ss_anim, int state)
{
    if(state == ENTITY_ANIM_NEWANIM)
    {
        m_response.slide = SlideType::Back;
        ss_anim->onFrame = nullptr;
    }
}

void Character::correctDivingAngle(SSAnimation* ss_anim, int state)
{
    if(state == ENTITY_ANIM_NEWANIM)
    {
        m_angles[1] = -45.0;
        updateTransform();
        ss_anim->onFrame = nullptr;
    }
}

void Character::toOnWater(SSAnimation* ss_anim, int state)
{
    if(state == ENTITY_ANIM_NEWANIM)
    {
        m_transform.getOrigin()[2] = m_heightInfo.transition_level;
        ghostUpdate();
        m_moveType = MOVE_ON_WATER;
        ss_anim->onFrame = nullptr;
    }
}

void Character::climbOutOfWater(SSAnimation* ss_anim, int state)
{
    if(state == ENTITY_ANIM_NEWANIM)
    {
        const btVector3& v = m_climb.point;

        m_transform.getOrigin() = v + m_transform.getBasis().getColumn(1) * 48.0;             // temporary stick
        m_transform.getOrigin()[2] = v[2];
        ghostUpdate();
        ss_anim->onFrame = nullptr;
    }
}

void Character::toEdgeClimb(SSAnimation* ss_anim, int state)
{
    if(state == ENTITY_ANIM_NEWANIM)
    {
        m_transform.getOrigin()[0] = m_climb.point[0] - m_transform.getBasis().getColumn(1)[0] * m_bf.bb_max[1];
        m_transform.getOrigin()[1] = m_climb.point[1] - m_transform.getBasis().getColumn(1)[1] * m_bf.bb_max[1];
        m_transform.getOrigin()[2] = m_climb.point[2] - m_bf.bb_max[2];
        ghostUpdate();
        ss_anim->onFrame = nullptr;
    }
}

void Character::toMonkeySwing(SSAnimation* ss_anim, int state)
{
    if(state == ENTITY_ANIM_NEWANIM)
    {
        m_moveType = MOVE_MONKEYSWING;
        m_transform.getOrigin()[2] = m_heightInfo.ceiling_point[2] - m_bf.bb_max[2];
        ghostUpdate();
        ss_anim->onFrame = nullptr;
    }
}

void Character::crawlToClimb(SSAnimation* ss_anim, int state)
{
    if(state == ENTITY_ANIM_NEWANIM)
    {
        if(!m_command.action)
        {
            setAnimation(TR_ANIMATION_LARA_START_FREE_FALL, 0);
            m_moveType = MOVE_FREE_FALLING;
            m_dirFlag = ENT_MOVE_BACKWARD;
        }
        else
        {
            setAnimation(TR_ANIMATION_LARA_HANG_IDLE, -1);
        }

        m_bt.no_fix_all = false;
        toEdgeClimb(ss_anim, state);
        ss_anim->onFrame = nullptr;
    }
}
