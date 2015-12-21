#include "character.h"

#include "character_controller.h"
#include "gui/itemnotifier.h"
#include "inventory.h"
#include "resource.h"
#include "script/script.h"
#include "world/core/basemesh.h"
#include "world/room.h"
#include "world/skeletalmodel.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>

constexpr float CollisionTraverseTestRadius = 0.48f;

namespace engine
{
extern GLfloat cast_ray[6]; // pointer to the test line coordinates
} // namespace engine

namespace world
{

namespace
{
/**
 *
 * @param rs: room sector pointer
 * @param floor: floor height
 * @return 0x01: can traverse, 0x00 can not;
 */
bool allowTraverse(const RoomSector& rs, glm::float_t floor, const Object& object)
{
    glm::float_t f0 = rs.floor_corners[0][2];
    if(rs.floor_corners[0][2] != f0 || rs.floor_corners[1][2] != f0 || rs.floor_corners[2][2] != f0 || rs.floor_corners[3][2] != f0)
    {
        return false;
    }

    if(glm::abs(floor - f0) < 1.1 && rs.ceiling - rs.floor >= MeteringSectorSize)
    {
        return true;
    }

    engine::BtEngineClosestRayResultCallback cb(&object);
    glm::vec3 from{rs.position[0], rs.position[1], floor + MeteringSectorSize * 0.5f};
    glm::vec3 to = from - glm::vec3{0, 0, MeteringSectorSize};
    engine::bt_engine_dynamicsWorld->rayTest(util::convert(from), util::convert(to), cb);
    if(cb.hasHit())
    {
        glm::vec3 v = glm::mix(from, to, cb.m_closestHitFraction);
        if(glm::abs(v[2] - floor) < 1.1)
        {
            Entity* e = dynamic_cast<Entity*>(static_cast<Object*>(cb.m_collisionObject->getUserPointer()));
            if(e && (e->m_typeFlags & ENTITY_TYPE_TRAVERSE_FLOOR))
            {
                return true;
            }
        }
    }

    return false;
}
} // anonymous namespace

HeightInfo::HeightInfo()
{
    sp->setMargin(COLLISION_MARGIN_DEFAULT);
}

Character::Character(uint32_t id) : Entity(id), m_stateController(this)
{
    m_sphere->setMargin(COLLISION_MARGIN_DEFAULT);

    m_climbSensor.reset(new btSphereShape(m_climbR));
    m_climbSensor->setMargin(COLLISION_MARGIN_DEFAULT);

    m_rayCb = std::make_shared<engine::BtEngineClosestRayResultCallback>(this, true);
    m_rayCb->m_collisionFilterMask = btBroadphaseProxy::StaticFilter | btBroadphaseProxy::KinematicFilter;
    m_heightInfo.cb = m_rayCb;

    m_convexCb = std::make_shared<engine::BtEngineClosestConvexResultCallback>(this, true);
    m_convexCb->m_collisionFilterMask = btBroadphaseProxy::StaticFilter | btBroadphaseProxy::KinematicFilter;
    m_heightInfo.ccb = m_convexCb;
}

Character::~Character()
{
    if(getRoom() && this != engine::engine_world.character.get())
    {
        getRoom()->removeEntity(this);
    }
}

int32_t Character::addItem(uint32_t item_id, int32_t count) // returns items count after in the function's end
{
    gui::notifierStart(item_id);

    auto item = engine::engine_world.getBaseItemByID(item_id);
    if(!item)
        return 0;

    count = count < 0 ? item->count : count;

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
    return static_cast<int32_t>(ret);
}

int32_t Character::getItemsCount(uint32_t item_id) // returns items count
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
    glm::vec4 t{0, 0, m_skeleton.getRootTransform()[3][2], 1};
    auto pos = glm::vec3(m_transform * t);
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
            glm::float_t trpl[16];
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
            glm::float_t trpl[16];
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
void Character::getHeightInfo(const glm::vec3& pos, HeightInfo* fc, glm::float_t v_offset)
{
    auto cb = fc->cb;
    Room* r = cb->m_object ? cb->m_object->getRoom() : nullptr;

    fc->floor.hasHit = false;
    fc->ceiling.hasHit = false;
    fc->water = false;
    fc->quicksand = QuicksandPosition::None;
    fc->transition_level = 32512.0;

    r = Room_FindPosCogerrence(pos, r);
    if(r)
        r = r->checkFlip();
    if(r)
    {
        const RoomSector* rs = r->getSectorXYZ(pos); // if r != nullptr then rs can not been nullptr!!!
        if(r->m_flags & TR_ROOM_FLAG_WATER)          // in water - go up
        {
            while(rs->sector_above)
            {
                BOOST_ASSERT(rs->sector_above != nullptr);
                rs = rs->sector_above->checkFlip();
                BOOST_ASSERT(rs != nullptr && rs->owner_room != nullptr);
                if((rs->owner_room->m_flags & TR_ROOM_FLAG_WATER) == 0x00) // find air
                {
                    fc->transition_level = static_cast<glm::float_t>(rs->floor);
                    fc->water = true;
                    break;
                }
            }
        }
        else if(r->m_flags & TR_ROOM_FLAG_QUICKSAND)
        {
            while(rs->sector_above)
            {
                BOOST_ASSERT(rs->sector_above != nullptr);
                rs = rs->sector_above->checkFlip();
                BOOST_ASSERT(rs != nullptr && rs->owner_room != nullptr);
                if((rs->owner_room->m_flags & TR_ROOM_FLAG_QUICKSAND) == 0x00) // find air
                {
                    fc->transition_level = static_cast<glm::float_t>(rs->floor);
                    if(fc->transition_level - fc->floor.hitPoint[2] > v_offset)
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
        else // in air - go down
        {
            while(rs->sector_below)
            {
                BOOST_ASSERT(rs->sector_below != nullptr);
                rs = rs->sector_below->checkFlip();
                BOOST_ASSERT(rs != nullptr && rs->owner_room != nullptr);
                if((rs->owner_room->m_flags & TR_ROOM_FLAG_WATER) != 0x00) // find water
                {
                    fc->transition_level = static_cast<glm::float_t>(rs->ceiling);
                    fc->water = true;
                    break;
                }
                else if((rs->owner_room->m_flags & TR_ROOM_FLAG_QUICKSAND) != 0x00) // find water
                {
                    fc->transition_level = static_cast<glm::float_t>(rs->ceiling);
                    if(fc->transition_level - fc->floor.hitPoint[2] > v_offset)
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
    glm::vec3 from = pos;
    glm::vec3 to = from;
    to[2] -= 4096.0;
    cb->m_closestHitFraction = 1.0;
    cb->m_collisionObject = nullptr;
    engine::bt_engine_dynamicsWorld->rayTest(util::convert(from), util::convert(to), *cb);
    fc->floor.assign(*cb, from, to);

    to = from;
    to[2] += 4096.0;
    cb->m_closestHitFraction = 1.0;
    cb->m_collisionObject = nullptr;
    // cb->m_flags = btTriangleRaycastCallback::kF_FilterBackfaces;
    engine::bt_engine_dynamicsWorld->rayTest(util::convert(from), util::convert(to), *cb);
    fc->ceiling.assign(*cb, from, to);
}

/**
 * Calculates next floor info + phantom filter + returns step info.
 * Current height info must be calculated!
 */
StepType Character::checkNextStep(const glm::vec3& offset, struct HeightInfo* nfc) const
{
    /// penetration test?

    auto pos = glm::vec3(m_transform[3]) + offset;
    Character::getHeightInfo(pos, nfc);

    glm::vec3 from, to;
    StepType ret;
    if(m_heightInfo.floor.hasHit && nfc->floor.hasHit)
    {
        glm::float_t delta = nfc->floor.hitPoint[2] - m_heightInfo.floor.hitPoint[2];
        if(glm::abs(delta) < core::SplitEpsilon)
        {
            from[2] = m_heightInfo.floor.hitPoint[2];
            ret = StepType::Horizontal; // horizontal
        }
        else if(delta < 0.0) // down way
        {
            delta = -delta;
            from[2] = m_heightInfo.floor.hitPoint[2];
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
        else // up way
        {
            from[2] = nfc->floor.hitPoint[2];
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
    else if(!m_heightInfo.floor.hasHit && !nfc->floor.hasHit)
    {
        from[2] = pos[2];
        ret = StepType::Horizontal; // horizontal? yes no maybe...
    }
    else if(!m_heightInfo.floor.hasHit && nfc->floor.hasHit) // strange case
    {
        from[2] = nfc->floor.hitPoint[2];
        ret = StepType::Horizontal;
    }
    else // if(m_heightInfo.floor_hit && !nfc->floor_hit)                                 // bottomless
    {
        from[2] = m_heightInfo.floor.hitPoint[2];
        ret = StepType::DownCanHang;
    }

    /*
     * check walls! If test is positive, than CHARACTER_STEP_UP_IMPOSSIBLE - can not go next!
     */
    from[2] += m_climbR;
    to[2] = from[2];
    from[0] = m_transform[3][0];
    from[1] = m_transform[3][1];
    to[0] = pos[0];
    to[1] = pos[1];
    m_heightInfo.cb->m_closestHitFraction = 1.0;
    m_heightInfo.cb->m_collisionObject = nullptr;
    engine::bt_engine_dynamicsWorld->rayTest(util::convert(from), util::convert(to), *m_heightInfo.cb);
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
    const glm::vec4& pos = m_transform[3];
    const glm::vec4 forward = m_transform[1];
    const glm::vec3& floor = next_fc.floor.hitNormal;

    return next_fc.floor.hitPoint[2] > pos[2] && next_fc.floor.hitNormal[2] < m_criticalSlantZComponent && forward[0] * floor[0] + forward[1] * floor[1] < 0.0;
}

/**
 * @FIXME: MAGICK CONST!
 * @param ent - entity
 * @param offset - offset, when we check height
 * @param nfc - height info (floor / ceiling)
 */
ClimbInfo Character::checkClimbability(const glm::vec3& offset, struct HeightInfo* nfc, glm::float_t test_height)
{
    const glm::vec4& pos = m_transform[3];
    /*
     * init callbacks functions
     */
    nfc->cb = m_rayCb;
    nfc->ccb = m_convexCb;
    auto tmp = glm::vec3(pos) + offset; // tmp = native offset point

    ClimbInfo ret;
    ret.height_info = checkNextStep(offset + glm::vec3{0, 0, 128}, nfc); ///@FIXME: stick for big slant
    ret.can_hang = false;
    ret.edge_hit = false;
    ret.edge_obj = nullptr;
    ret.floor_limit = m_heightInfo.floor.hasHit ? m_heightInfo.floor.hitPoint[2] : std::numeric_limits<glm::float_t>::min();
    ret.ceiling_limit = m_heightInfo.ceiling.hasHit ? m_heightInfo.ceiling.hitPoint[2] : std::numeric_limits<glm::float_t>::max();
    if(nfc->ceiling.hasHit && nfc->ceiling.hitPoint[2] < ret.ceiling_limit)
    {
        ret.ceiling_limit = nfc->ceiling.hitPoint[2];
    }
    ret.point = m_climb.point;
    /*
     * check max height
     */
    if(m_heightInfo.ceiling.hasHit && tmp[2] > m_heightInfo.ceiling.hitPoint[2] - m_climbR - 1.0f)
    {
        tmp[2] = m_heightInfo.ceiling.hitPoint[2] - m_climbR - 1.0f;
    }

    /*
    * Let us calculate EDGE
    */
    glm::vec3 from;
    from[0] = pos[0] - m_transform[1][0] * m_climbR * 2.0f;
    from[1] = pos[1] - m_transform[1][1] * m_climbR * 2.0f;
    from[2] = tmp[2];
    glm::vec3 to = tmp;

    // vec3_copy(cast_ray, from);
    // vec3_copy(cast_ray+3, to);

    btTransform t1 = btTransform::getIdentity();
    btTransform t2 = btTransform::getIdentity();
    uint8_t up_founded = 0;
    test_height = test_height >= m_maxStepUpHeight ? test_height : m_maxStepUpHeight;
    glm::float_t d = pos[2] + m_skeleton.getBoundingBox().max[2] - test_height;
    std::copy_n(glm::value_ptr(to), 3, &engine::cast_ray[0]);
    std::copy_n(glm::value_ptr(from), 3, &engine::cast_ray[3]);
    engine::cast_ray[5] -= d;
    glm::vec3 n0{0, 0, 0}, n1{0, 0, 0};
    glm::float_t n0d{0}, n1d{0};
    do
    {
        t1.setOrigin(util::convert(from));
        t2.setOrigin(util::convert(to));
        nfc->ccb->m_closestHitFraction = 1.0;
        nfc->ccb->m_hitCollisionObject = nullptr;
        engine::bt_engine_dynamicsWorld->convexSweepTest(m_climbSensor.get(), t1, t2, *nfc->ccb);
        if(nfc->ccb->hasHit())
        {
            if(nfc->ccb->m_hitNormalWorld[2] >= 0.1)
            {
                up_founded = 1;
                n0 = util::convert(nfc->ccb->m_hitNormalWorld);
                n0d = -glm::dot(n0, util::convert(nfc->ccb->m_hitPointWorld));
            }
            if(up_founded && nfc->ccb->m_hitNormalWorld[2] < 0.001)
            {
                n1 = util::convert(nfc->ccb->m_hitNormalWorld);
                n1d = -glm::dot(n1, util::convert(nfc->ccb->m_hitPointWorld));
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
            t1.setOrigin(util::convert(to));
            t2.setOrigin(util::convert(tmp));
            // vec3_copy(cast_ray, to);
            // vec3_copy(cast_ray+3, tmp);
            nfc->ccb->m_closestHitFraction = 1.0;
            nfc->ccb->m_hitCollisionObject = nullptr;
            engine::bt_engine_dynamicsWorld->convexSweepTest(m_climbSensor.get(), t1, t2, *nfc->ccb);
            if(nfc->ccb->hasHit())
            {
                up_founded = 1;
                n0 = util::convert(nfc->ccb->m_hitNormalWorld);
                n0d = -glm::dot(n0, util::convert(nfc->ccb->m_hitPointWorld));
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
    } while(to[2] >= d); // we can't climb under floor!

    if(up_founded != 2)
    {
        return ret;
    }

    // get the character plane equation
    glm::vec3 n2 = glm::vec3(m_transform[0]);
    glm::float_t n2d = -glm::dot(n2, glm::vec3(pos));

    /*
     * Solve system of the linear equations by Kramer method!
     * I know - It may be slow, but it has a good precision!
     * The root is point of 3 planes intersection.
     */
    d = -n0[0] * (n1[1] * n2[2] - n1[2] * n2[1]) + n1[0] * (n0[1] * n2[2] - n0[2] * n2[1]) - n2[0] * (n0[1] * n1[2] - n0[2] * n1[1]);

    if(glm::abs(d) < 0.005)
    {
        return ret;
    }

    ret.edge_point[0] = n0d * (n1[1] * n2[2] - n1[2] * n2[1]) - n1d * (n0[1] * n2[2] - n0[2] * n2[1]) + n2d * (n0[1] * n1[2] - n0[2] * n1[1]);
    ret.edge_point[0] /= d;

    ret.edge_point[1] = n0[0] * (n1d * n2[2] - n1[2] * n2d) - n1[0] * (n0d * n2[2] - n0[2] * n2d) + n2[0] * (n0d * n1[2] - n0[2] * n1d);
    ret.edge_point[1] /= d;

    ret.edge_point[2] = n0[0] * (n1[1] * n2d - n1d * n2[1]) - n1[0] * (n0[1] * n2d - n0d * n2[1]) + n2[0] * (n0[1] * n1d - n0d * n1[1]);
    ret.edge_point[2] /= d;
    ret.point = ret.edge_point;
    std::copy_n(glm::value_ptr(ret.point), 3, engine::cast_ray + 3);
    /*
     * unclimbable edge slant %)
     */
    n2 = glm::cross(n0, n1);
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
    if(n2[0] * m_transform[1][0] + n2[1] * m_transform[1][1] > 0) // direction fixing
    {
        n2[0] = -n2[0];
        n2[1] = -n2[1];
    }

    ret.n = n2;
    ret.up[0] = 0.0;
    ret.up[1] = 0.0;
    ret.up[2] = 1.0;
    ret.edge_z_ang = glm::degrees(glm::atan(n2[0], -n2[1]));
    ret.edge_tan_xy[0] = -n2[1];
    ret.edge_tan_xy[1] = n2[0];
    ret.edge_tan_xy[2] = 0.0;
    ret.edge_tan_xy /= btSqrt(n2[0] * n2[0] + n2[1] * n2[1]);
    ret.right = ret.edge_tan_xy;

    if(!m_heightInfo.floor.hasHit || ret.edge_point[2] - m_heightInfo.floor.hitPoint[2] >= m_height)
    {
        ret.can_hang = true;
    }

    ret.next_z_space = 2.0f * m_height;
    if(nfc->floor.hasHit && nfc->ceiling.hasHit)
    {
        ret.next_z_space = nfc->ceiling.hitPoint[2] - nfc->floor.hitPoint[2];
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
    ret.floor_limit = m_heightInfo.floor.hasHit ? m_heightInfo.floor.hitPoint[2] : std::numeric_limits<glm::float_t>::min();
    ret.ceiling_limit = m_heightInfo.ceiling.hasHit ? m_heightInfo.ceiling.hitPoint[2] : std::numeric_limits<glm::float_t>::max();
    ret.point = m_climb.point;

    if(!m_heightInfo.walls_climb)
    {
        return ret;
    }

    ret.up = {0, 0, 1};

    const glm::vec4& pos = m_transform[3];
    glm::vec3 from = glm::vec3(pos + m_transform[2] * m_skeleton.getBoundingBox().max[2] - m_transform[1] * m_climbR);
    glm::vec3 to = from;
    glm::float_t t = m_forwardSize + m_skeleton.getBoundingBox().max[1];
    to += glm::vec3(m_transform[1] * t);

    auto ccb = m_convexCb;
    ccb->m_closestHitFraction = 1.0;
    ccb->m_hitCollisionObject = nullptr;

    btTransform tr1 = btTransform::getIdentity();
    tr1.setOrigin(util::convert(from));

    btTransform tr2 = btTransform::getIdentity();
    tr2.setOrigin(util::convert(to));

    engine::bt_engine_dynamicsWorld->convexSweepTest(m_climbSensor.get(), tr1, tr2, *ccb);
    if(!ccb->hasHit())
    {
        return ret;
    }

    ret.point = util::convert(ccb->m_hitPointWorld);
    ret.n = util::convert(ccb->m_hitNormalWorld);
    glm::float_t wn2[2] = {ret.n[0], ret.n[1]};
    t = sqrt(wn2[0] * wn2[0] + wn2[1] * wn2[1]);
    wn2[0] /= t;
    wn2[1] /= t;

    ret.right[0] = -wn2[1];
    ret.right[1] = wn2[0];
    ret.right[2] = 0.0;
    // now we have wall normale in XOY plane. Let us check all flags

    if((m_heightInfo.walls_climb_dir & SECTOR_FLAG_CLIMB_NORTH) && wn2[1] < -0.7)
    {
        ret.wall_hit = ClimbType::HandsOnly; // nW = (0, -1, 0);
    }
    if((m_heightInfo.walls_climb_dir & SECTOR_FLAG_CLIMB_EAST) && wn2[0] < -0.7)
    {
        ret.wall_hit = ClimbType::HandsOnly; // nW = (-1, 0, 0);
    }
    if((m_heightInfo.walls_climb_dir & SECTOR_FLAG_CLIMB_SOUTH) && wn2[1] > 0.7)
    {
        ret.wall_hit = ClimbType::HandsOnly; // nW = (0, 1, 0);
    }
    if((m_heightInfo.walls_climb_dir & SECTOR_FLAG_CLIMB_WEST) && wn2[0] > 0.7)
    {
        ret.wall_hit = ClimbType::HandsOnly; // nW = (1, 0, 0);
    }

    if(ret.wall_hit != ClimbType::None)
    {
        t = 0.67f * m_height;
        from -= glm::vec3(m_transform[2] * t);
        to = from;
        t = m_forwardSize + m_skeleton.getBoundingBox().max[1];
        to += glm::vec3(m_transform[1] * t);

        ccb->m_closestHitFraction = 1.0;
        ccb->m_hitCollisionObject = nullptr;
        tr1.setIdentity();
        tr1.setOrigin(util::convert(from));
        tr2.setIdentity();
        tr2.setOrigin(util::convert(to));
        engine::bt_engine_dynamicsWorld->convexSweepTest(m_climbSensor.get(), tr1, tr2, *ccb);
        if(ccb->hasHit())
        {
            ret.wall_hit = ClimbType::FullBody;
        }
    }

    return ret;
}

void Character::lean(glm::float_t max_lean)
{
    glm::float_t neg_lean = 360.0f - max_lean;
    glm::float_t lean_coeff = util::fuzzyZero(max_lean) ? 48.0f : max_lean * 3;

    // Continously lean character, according to current left/right direction.

    if(m_command.move.x == MovementStrafe::None || util::fuzzyZero(max_lean)) // No direction - restore straight vertical position!
    {
        if(m_angles[2] != 0.0)
        {
            if(m_angles[2] < 180.0)
            {
                m_angles[2] -= (glm::abs(m_angles[2]) + lean_coeff) / 2 * util::toSeconds(engine::engine_frame_time);
                if(m_angles[2] < 0.0)
                    m_angles[2] = 0.0;
            }
            else
            {
                m_angles[2] += (360 - glm::abs(m_angles[2]) + lean_coeff) / 2 * util::toSeconds(engine::engine_frame_time);
                if(m_angles[2] < 180.0)
                    m_angles[2] = 0.0;
            }
        }
    }
    else if(m_command.move.x == MovementStrafe::Right) // Right direction
    {
        if(m_angles[2] != max_lean)
        {
            if(m_angles[2] < max_lean) // Approaching from center
            {
                m_angles[2] += (glm::abs(m_angles[2]) + lean_coeff) / 2 * util::toSeconds(engine::engine_frame_time);
                if(m_angles[2] > max_lean)
                    m_angles[2] = max_lean;
            }
            else if(m_angles[2] > 180.0) // Approaching from left
            {
                m_angles[2] += (360.0f - glm::abs(m_angles[2]) + (lean_coeff * 2) / 2) * util::toSeconds(engine::engine_frame_time);
                if(m_angles[2] < 180.0)
                    m_angles[2] = 0.0;
            }
            else // Reduce previous lean
            {
                m_angles[2] -= (glm::abs(m_angles[2]) + lean_coeff) / 2 * util::toSeconds(engine::engine_frame_time);
                if(m_angles[2] < 0.0)
                    m_angles[2] = 0.0;
            }
        }
    }
    else if(m_command.move.x == MovementStrafe::Left) // Left direction
    {
        if(m_angles[2] != neg_lean)
        {
            if(m_angles[2] > neg_lean) // Reduce previous lean
            {
                m_angles[2] -= (360.0f - glm::abs(m_angles[2]) + lean_coeff) / 2 * util::toSeconds(engine::engine_frame_time);
                if(m_angles[2] < neg_lean)
                    m_angles[2] = neg_lean;
            }
            else if(m_angles[2] < 180.0) // Approaching from right
            {
                m_angles[2] -= (glm::abs(m_angles[2]) + (lean_coeff * 2)) / 2 * util::toSeconds(engine::engine_frame_time);
                if(m_angles[2] < 0.0)
                    m_angles[2] += 360.0;
            }
            else // Approaching from center
            {
                m_angles[2] += (360.0f - glm::abs(m_angles[2]) + lean_coeff) / 2 * util::toSeconds(engine::engine_frame_time);
                if(m_angles[2] > 360.0)
                    m_angles[2] -= 360.0f;
            }
        }
    }
}

/*
 * Linear inertia is absolutely needed for in-water states, and also it gives
 * more organic feel to land animations.
 */
glm::float_t Character::inertiaLinear(glm::float_t max_speed, glm::float_t accel, bool command)
{
    if(util::fuzzyZero(accel) || accel >= max_speed)
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
                m_inertiaLinear += max_speed * accel * util::toSeconds(engine::engine_frame_time);
                if(m_inertiaLinear > max_speed)
                    m_inertiaLinear = max_speed;
            }
        }
        else
        {
            if(m_inertiaLinear > 0.0)
            {
                m_inertiaLinear -= max_speed * accel * util::toSeconds(engine::engine_frame_time);
                if(m_inertiaLinear < 0.0)
                    m_inertiaLinear = 0.0;
            }
        }
    }

    return m_inertiaLinear * animation::AnimationFrameRate;
}

/*
 * Angular inertia is used on keyboard-driven (non-analog) rotational controls.
 */
glm::float_t Character::inertiaAngular(glm::float_t max_angle, glm::float_t accel, uint8_t axis)
{
    if(axis > 1)
        return 0.0;

    uint8_t curr_rot_dir = 0;
    if(m_command.rot[axis] < 0.0)
    {
        curr_rot_dir = 1;
    }
    else if(m_command.rot[axis] > 0.0)
    {
        curr_rot_dir = 2;
    }

    if(!curr_rot_dir || util::fuzzyZero(max_angle) || util::fuzzyZero(accel))
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
                    m_inertiaAngular[axis] += max_angle * accel * util::toSeconds(engine::engine_frame_time);
                    if(m_inertiaAngular[axis] > max_angle)
                        m_inertiaAngular[axis] = max_angle;
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
                    m_inertiaAngular[axis] -= max_angle * accel * util::toSeconds(engine::engine_frame_time);
                    if(m_inertiaAngular[axis] < -max_angle)
                        m_inertiaAngular[axis] = -max_angle;
                }
            }
        }
    }

    return glm::abs(m_inertiaAngular[axis]) * m_command.rot[axis];
}

/*
 * MOVE IN DIFFERENT CONDITIONS
 */
void Character::moveOnFloor()
{
    /*
    * init height info structure
    */
    m_response.horizontal_collide = 0x00;
    m_response.vertical_collide = 0x00;
    // First of all - get information about floor and ceiling!!!
    updateCurrentHeight();
    if(m_heightInfo.floor.hasHit && m_heightInfo.floor.hitPoint[2] + 1.0 >= m_transform[3][2] + m_skeleton.getBoundingBox().min[2])
    {
        Object* object = static_cast<Object*>(m_heightInfo.floor.collisionObject->getUserPointer());
        if(Entity* e = dynamic_cast<Entity*>(object))
        {
            if(e->m_callbackFlags & ENTITY_CALLBACK_STAND)
            {
                engine_lua.execEntity(ENTITY_CALLBACK_STAND, e->getId(), getId());
            }
        }
    }

    glm::vec3 speed(0.0, 0.0, 0.0);

    /*
    * check move type
    */
    if(m_heightInfo.floor.hasHit || (m_response.vertical_collide & 0x01))
    {
        if(m_heightInfo.floor.hitPoint[2] + m_fallDownHeight < m_transform[3][2])
        {
            m_moveType = MoveType::FreeFalling;
            m_speed[2] = 0.0;
            return; // nothing to do here
        }
        else
        {
            m_response.vertical_collide |= 0x01;
        }

        glm::vec3 floorNormal = m_heightInfo.floor.hitNormal;
        if(floorNormal[2] > 0.02 && floorNormal[2] < m_criticalSlantZComponent)
        {
            floorNormal[2] = -floorNormal[2];
            speed = floorNormal * animation::AnimationFrameRate * DEFAULT_CHARACTER_SLIDE_SPEED_MULT;      // slide down direction
            const glm::float_t zAngle = glm::degrees(glm::atan(floorNormal[0], -floorNormal[1])); // from -180 deg to +180 deg
            // ang = (ang < 0.0)?(ang + 360.0):(ang);
            glm::float_t t = floorNormal[0] * m_transform[1][0] + floorNormal[1] * m_transform[1][1];
            if(t >= 0.0)
            {
                // front forward slide down
                m_response.slide = MovementWalk::Forward;
                m_angles[0] = zAngle + 180;
            }
            else
            {
                // back forward slide down
                m_response.slide = MovementWalk::Backward;
                m_angles[0] = zAngle;
            }
            updateTransform();
            m_response.vertical_collide |= 0x01;
        }
        else // no slide - free to walk
        {
            const glm::float_t fullSpeed = m_currentSpeed * animation::AnimationFrameRate;
            m_response.vertical_collide |= 0x01;

            m_angles[0] += inertiaAngular(1.0, ROT_SPEED_LAND, 0);

            updateTransform(); // apply rotations

            if(m_moveDir == MoveDirection::Forward)
            {
                speed = glm::vec3(m_transform[1] * fullSpeed);
            }
            else if(m_moveDir == MoveDirection::Backward)
            {
                speed = glm::vec3(m_transform[1] * -fullSpeed);
            }
            else if(m_moveDir == MoveDirection::Left)
            {
                speed = glm::vec3(m_transform[0] * -fullSpeed);
            }
            else if(m_moveDir == MoveDirection::Right)
            {
                speed = glm::vec3(m_transform[0] * fullSpeed);
            }
            else
            {
                // dir_flag = DirFlag::Forward;
            }
            m_response.slide = MovementWalk::None;
        }
    }
    else // no hit to the floor
    {
        m_response.slide = MovementWalk::None;
        m_response.vertical_collide = 0x00;
        m_moveType = MoveType::FreeFalling;
        m_speed[2] = 0.0;
        return; // nothing to do here
    }

    /*
    * now move normally
    */
    m_speed = speed;
    glm::vec3 positionDelta = speed * util::toSeconds(engine::engine_frame_time);
    const glm::float_t distance = glm::length(positionDelta);

    glm::vec3 norm_move_xy(positionDelta[0], positionDelta[1], 0.0);
    glm::float_t norm_move_xy_len = glm::length(norm_move_xy);
    if(norm_move_xy_len > 0.2 * distance)
    {
        norm_move_xy /= norm_move_xy_len;
    }
    else
    {
        norm_move_xy = {0, 0, 0};
    }

    ghostUpdate();
    m_transform[3] += glm::vec4(positionDelta, 0);
    fixPenetrations(&positionDelta);
    if(m_heightInfo.floor.hasHit)
    {
        if(m_heightInfo.floor.hitPoint[2] + m_fallDownHeight > m_transform[3][2])
        {
            glm::float_t dz_to_land = util::toSeconds(engine::engine_frame_time) * 2400.0f; ///@FIXME: magick
            if(m_transform[3][2] > m_heightInfo.floor.hitPoint[2] + dz_to_land)
            {
                m_transform[3][2] -= dz_to_land;
                fixPenetrations(nullptr);
            }
            else if(m_transform[3][2] > m_heightInfo.floor.hitPoint[2])
            {
                m_transform[3][2] = m_heightInfo.floor.hitPoint[2];
                fixPenetrations(nullptr);
            }
        }
        else
        {
            m_moveType = MoveType::FreeFalling;
            m_speed[2] = 0.0;
            updateRoomPos();
            return;
        }
        if(m_transform[3][2] < m_heightInfo.floor.hitPoint[2] && !m_skeleton.getModel()->no_fix_all)
        {
            m_transform[3][2] = m_heightInfo.floor.hitPoint[2];
            fixPenetrations(nullptr);
            m_response.vertical_collide |= 0x01;
        }
    }
    else if(!(m_response.vertical_collide & 0x01))
    {
        m_moveType = MoveType::FreeFalling;
        m_speed[2] = 0.0;
        updateRoomPos();
        return;
    }

    updateRoomPos();
}

int Character::freeFalling()
{
    /*
    * init height info structure
    */

    m_response.slide = MovementWalk::None;
    m_response.horizontal_collide = 0x00;
    m_response.vertical_collide = 0x00;

    glm::float_t rot = inertiaAngular(1.0, ROT_SPEED_FREEFALL, 0);
    m_angles[0] += rot;
    m_angles[1] = 0.0;

    updateTransform(); // apply rotations

    glm::vec3 move = applyGravity(engine::engine_frame_time);
    m_speed[2] = m_speed[2] < -FREE_FALL_SPEED_MAXIMUM ? -FREE_FALL_SPEED_MAXIMUM : m_speed[2];
    m_speed = glm::rotate(m_speed, glm::radians(rot), {0, 0, 1});

    updateCurrentHeight();

    if(getRoom() && (getRoom()->m_flags & TR_ROOM_FLAG_WATER))
    {
        if(m_speed[2] < 0.0)
        {
            m_currentSpeed = 0.0;
            m_speed[0] = 0.0;
            m_speed[1] = 0.0;
        }

        if(engine::engine_world.engineVersion < loader::Engine::TR2) // Lara cannot wade in < TRII so when floor < transition level she has to swim
        {
            if(!m_heightInfo.water || m_currentSector->floor <= m_heightInfo.transition_level)
            {
                m_moveType = MoveType::Underwater;
                return 2;
            }
        }
        else
        {
            if(!m_heightInfo.water || m_currentSector->floor + m_height <= m_heightInfo.transition_level)
            {
                m_moveType = MoveType::Underwater;
                return 2;
            }
        }
    }

    ghostUpdate();

    if(m_heightInfo.ceiling.hasHit && m_speed[2] > 0.0)
    {
        if(m_heightInfo.ceiling.hitPoint[2] < m_skeleton.getBoundingBox().max[2] + m_transform[3][2])
        {
            m_transform[3][2] = m_heightInfo.ceiling.hitPoint[2] - m_skeleton.getBoundingBox().max[2];
            m_speed[2] = 1.0; // As in original.
            m_response.vertical_collide |= 0x02;
            fixPenetrations(nullptr);
            updateRoomPos();
        }
    }
    if(m_heightInfo.floor.hasHit && m_speed[2] < 0.0) // move down
    {
        if(m_heightInfo.floor.hitPoint[2] >= m_transform[3][2] + m_skeleton.getBoundingBox().min[2] + move[2])
        {
            m_transform[3][2] = m_heightInfo.floor.hitPoint[2];
            // speed[2] = 0.0;
            m_moveType = MoveType::OnFloor;
            m_response.vertical_collide |= 0x01;
            fixPenetrations(nullptr);
            updateRoomPos();
            return 2;
        }
    }

    m_transform[3] += glm::vec4(move, 0);
    fixPenetrations(&move); // get horizontal collide

    if(m_heightInfo.ceiling.hasHit && m_speed[2] > 0.0)
    {
        if(m_heightInfo.ceiling.hitPoint[2] < m_skeleton.getBoundingBox().max[2] + m_transform[3][2])
        {
            m_transform[3][2] = m_heightInfo.ceiling.hitPoint[2] - m_skeleton.getBoundingBox().max[2];
            m_speed[2] = 1.0; // As in original.
            m_response.vertical_collide |= 0x02;
        }
    }
    if(m_heightInfo.floor.hasHit && m_speed[2] < 0.0) // move down
    {
        if(m_heightInfo.floor.hitPoint[2] >= m_transform[3][2] + m_skeleton.getBoundingBox().min[2] + move[2])
        {
            m_transform[3][2] = m_heightInfo.floor.hitPoint[2];
            // speed[2] = 0.0;
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
    glm::vec3 move;
    glm::float_t t;

    m_speed[2] = 0.0;

    m_response.slide = MovementWalk::None;
    m_response.lean = MovementStrafe::None;

    m_response.horizontal_collide = 0x00;
    m_response.vertical_collide = 0x00;

    t = m_currentSpeed * animation::AnimationFrameRate;
    m_response.vertical_collide |= 0x01;

    m_angles[0] += inertiaAngular(1.0, ROT_SPEED_MONKEYSWING, 0);
    m_angles[1] = 0.0;
    m_angles[2] = 0.0;
    updateTransform(); // apply rotations

    if(m_moveDir == MoveDirection::Forward)
    {
        m_speed = glm::vec3(m_transform[1] * t);
    }
    else if(m_moveDir == MoveDirection::Backward)
    {
        m_speed = glm::vec3(m_transform[1] * -t);
    }
    else if(m_moveDir == MoveDirection::Left)
    {
        m_speed = glm::vec3(m_transform[0] * -t);
    }
    else if(m_moveDir == MoveDirection::Right)
    {
        m_speed = glm::vec3(m_transform[0] * t);
    }
    else
    {
        m_speed = {0, 0, 0};
        // dir_flag = DirFlag::Forward;
    }

    move = m_speed * util::toSeconds(engine::engine_frame_time);
    move[2] = 0.0;

    ghostUpdate();
    updateCurrentHeight();
    m_transform[3] += glm::vec4(move, 0);
    fixPenetrations(&move); // get horizontal collide
                            ///@FIXME: rewrite conditions! or add fixer to update_entity_rigid_body func
    if(m_heightInfo.ceiling.hasHit && m_transform[3][2] + m_skeleton.getBoundingBox().max[2] - m_heightInfo.ceiling.hitPoint[2] > -0.33 * m_minStepUpHeight)
    {
        m_transform[3][2] = m_heightInfo.ceiling.hitPoint[2] - m_skeleton.getBoundingBox().max[2];
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
    glm::vec3 spd, move;
    glm::float_t t;

    m_response.slide = MovementWalk::None;
    m_response.horizontal_collide = 0x00;
    m_response.vertical_collide = 0x00;

    spd = {0, 0, 0};
    *climb = checkWallsClimbability();
    m_climb = *climb;
    if(climb->wall_hit == ClimbType::None)
    {
        m_heightInfo.walls_climb = false;
        return 2;
    }

    m_angles[0] = glm::degrees(glm::atan(climb->n[0], -climb->n[1]));
    updateTransform();
    m_transform[3][0] = climb->point[0] - m_transform[1][0] * m_skeleton.getBoundingBox().max[1];
    m_transform[3][1] = climb->point[1] - m_transform[1][1] * m_skeleton.getBoundingBox().max[1];

    if(m_moveDir == MoveDirection::Forward)
    {
        spd += climb->up;
    }
    else if(m_moveDir == MoveDirection::Backward)
    {
        spd -= climb->up;
    }
    else if(m_moveDir == MoveDirection::Right)
    {
        spd += climb->right;
    }
    else if(m_moveDir == MoveDirection::Left)
    {
        spd -= climb->right;
    }
    t = glm::length(spd);
    if(t > 0.01)
    {
        spd /= t;
    }
    m_speed = spd * m_currentSpeed * animation::AnimationFrameRate;
    move = m_speed * util::toSeconds(engine::engine_frame_time);

    ghostUpdate();
    updateCurrentHeight();
    m_transform[3] += glm::vec4(move, 0);
    fixPenetrations(&move); // get horizontal collide
    updateRoomPos();

    *climb = checkWallsClimbability();
    if(m_transform[3][2] + m_skeleton.getBoundingBox().max[2] > climb->ceiling_limit)
    {
        m_transform[3][2] = climb->ceiling_limit - m_skeleton.getBoundingBox().max[2];
    }

    return 1;
}

/*
 * CLIMBING - MOVE NO Z LANDING
 */
int Character::climbing()
{
    glm::float_t z = m_transform[3][2];

    m_response.slide = MovementWalk::None;
    m_response.horizontal_collide = 0x00;
    m_response.vertical_collide = 0x00;

    glm::float_t t = m_currentSpeed * animation::AnimationFrameRate;
    m_response.vertical_collide |= 0x01;
    m_angles[0] += m_command.rot[0];
    m_angles[1] = 0.0;
    m_angles[2] = 0.0;
    updateTransform(); // apply rotations

    if(m_moveDir == MoveDirection::Forward)
    {
        m_speed = glm::vec3(m_transform[1] * t);
    }
    else if(m_moveDir == MoveDirection::Backward)
    {
        m_speed = glm::vec3(m_transform[1] * -t);
    }
    else if(m_moveDir == MoveDirection::Left)
    {
        m_speed = glm::vec3(m_transform[0] * -t);
    }
    else if(m_moveDir == MoveDirection::Right)
    {
        m_speed = glm::vec3(m_transform[0] * t);
    }
    else
    {
        m_response.slide = MovementWalk::None;
        ghostUpdate();
        fixPenetrations(nullptr);
        return 1;
    }

    m_response.slide = MovementWalk::None;

    glm::vec3 move = m_speed * util::toSeconds(engine::engine_frame_time);

    ghostUpdate();
    m_transform[3] += glm::vec4(move, 0);
    fixPenetrations(&move); // get horizontal collide
    updateRoomPos();
    m_transform[3][2] = z;

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
    glm::vec3 move;

    // Check current place.

    if(getRoom() && !(getRoom()->m_flags & TR_ROOM_FLAG_WATER))
    {
        m_moveType = MoveType::FreeFalling;
        return 2;
    }

    m_response.slide = MovementWalk::None;
    m_response.lean = MovementStrafe::None;

    m_response.horizontal_collide = 0x00;
    m_response.vertical_collide = 0x00;

    // Calculate current speed.

    glm::float_t t = inertiaLinear(MAX_SPEED_UNDERWATER, INERTIA_SPEED_UNDERWATER, m_command.jump);

    if(!m_response.killed) // Block controls if Lara is dead.
    {
        m_angles[0] += inertiaAngular(1.0, ROT_SPEED_UNDERWATER, 0);
        m_angles[1] -= inertiaAngular(1.0, ROT_SPEED_UNDERWATER, 1);
        m_angles[2] = 0.0;

        if(m_angles[1] > 70.0 && m_angles[1] < 180.0) // Underwater angle limiter.
        {
            m_angles[1] = 70.0;
        }
        else if(m_angles[1] > 180.0 && m_angles[1] < 270.0)
        {
            m_angles[1] = 270.0;
        }

        updateTransform(); // apply rotations

        m_speed = glm::vec3(m_transform[1] * t); // OY move only!
        move = m_speed * util::toSeconds(engine::engine_frame_time);
    }
    else
    {
        move = {0, 0, 0};
    }

    ghostUpdate();
    m_transform[3] += glm::vec4(move, 0);
    fixPenetrations(&move); // get horizontal collide

    updateRoomPos();
    if(m_heightInfo.water && m_transform[3][2] + m_skeleton.getBoundingBox().max[2] >= m_heightInfo.transition_level)
    {
        if(/*(spd[2] > 0.0)*/ m_transform[1][2] > 0.67) ///@FIXME: magick!
        {
            m_moveType = MoveType::OnWater;
            // pos[2] = fc.transition_level;
            return 2;
        }
        if(!m_heightInfo.floor.hasHit || m_heightInfo.transition_level - m_heightInfo.floor.hitPoint[2] >= m_height)
        {
            m_transform[3][2] = m_heightInfo.transition_level - m_skeleton.getBoundingBox().max[2];
        }
    }

    return 1;
}

int Character::moveOnWater()
{
    m_response.slide = MovementWalk::None;
    m_response.lean = MovementStrafe::None;

    m_response.horizontal_collide = 0x00;
    m_response.vertical_collide = 0x00;

    m_angles[0] += inertiaAngular(1.0, ROT_SPEED_ONWATER, 0);
    m_angles[1] = 0.0;
    m_angles[2] = 0.0;
    updateTransform(); // apply rotations

    // Calculate current speed.

    glm::float_t t = inertiaLinear(MAX_SPEED_ONWATER, INERTIA_SPEED_ONWATER, m_command.move.z != MovementWalk::None || m_command.move.x != MovementStrafe::None);

    if(m_moveDir == MoveDirection::Forward && m_command.move.z == MovementWalk::Forward)
    {
        m_speed = glm::vec3(m_transform[1] * t);
    }
    else if(m_moveDir == MoveDirection::Backward && m_command.move.z == MovementWalk::Backward)
    {
        m_speed = glm::vec3(m_transform[1] * -t);
    }
    else if(m_moveDir == MoveDirection::Left && m_command.move.x == MovementStrafe::Left)
    {
        m_speed = glm::vec3(m_transform[0] * -t);
    }
    else if(m_moveDir == MoveDirection::Right && m_command.move.x == MovementStrafe::Right)
    {
        m_speed = glm::vec3(m_transform[0] * t);
    }
    else
    {
        ghostUpdate();
        fixPenetrations(nullptr);
        updateRoomPos();
        if(m_heightInfo.water)
        {
            m_transform[3][2] = m_heightInfo.transition_level;
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
    glm::vec3 move = m_speed * util::toSeconds(engine::engine_frame_time);
    ghostUpdate();
    m_transform[3] += glm::vec4(move, 0);
    fixPenetrations(&move); // get horizontal collide

    updateRoomPos();
    if(m_heightInfo.water)
    {
        m_transform[3][2] = m_heightInfo.transition_level;
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
    const RoomSector* ch_s = getRoom()->getSectorRaw(glm::vec3(m_transform[3]));

    if(ch_s == nullptr)
    {
        return 0;
    }

    m_traversedObject = nullptr;

    // OX move case
    RoomSector* obj_s = nullptr;
    if(m_transform[1][0] > 0.9)
    {
        obj_s = ch_s->owner_room->getSectorRaw({ch_s->position[0] + MeteringSectorSize, ch_s->position[1], glm::float_t(0.0)});
    }
    else if(m_transform[1][0] < -0.9)
    {
        obj_s = ch_s->owner_room->getSectorRaw({ch_s->position[0] - MeteringSectorSize, ch_s->position[1], glm::float_t(0.0)});
    }
    // OY move case
    else if(m_transform[1][1] > 0.9)
    {
        obj_s = ch_s->owner_room->getSectorRaw({ch_s->position[0], ch_s->position[1] + MeteringSectorSize, glm::float_t(0.0)});
    }
    else if(m_transform[1][1] < -0.9)
    {
        obj_s = ch_s->owner_room->getSectorRaw({ch_s->position[0], ch_s->position[1] - MeteringSectorSize, glm::float_t(0.0)});
    }

    if(obj_s != nullptr)
    {
        obj_s = obj_s->checkPortalPointer();
        for(Object* object : obj_s->owner_room->m_objects)
        {
            if(Entity* e = dynamic_cast<Entity*>(object))
            {
                if((e->m_typeFlags & ENTITY_TYPE_TRAVERSE) && core::testOverlap(*e, *this) && glm::abs(e->m_transform[3][2] - m_transform[3][2]) < 1.1)
                {
                    m_angles[0] = std::lround(m_angles[0] / 90.0f) * 90.0f;
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
 * @param obj Traversed object pointer
 * @see TraverseNone TraverseForward TraverseBackward
 */
int Character::checkTraverse(const Entity& obj)
{
    RoomSector* ch_s = getRoom()->getSectorRaw(glm::vec3(m_transform[3]));
    RoomSector* obj_s = obj.getRoom()->getSectorRaw(glm::vec3(obj.m_transform[3]));

    if(obj_s == ch_s)
    {
        if(m_transform[1][0] > 0.8)
        {
            ch_s = obj_s->owner_room->getSectorRaw({obj_s->position[0] - MeteringSectorSize, obj_s->position[1], 0});
        }
        else if(m_transform[1][0] < -0.8)
        {
            ch_s = obj_s->owner_room->getSectorRaw({obj_s->position[0] + MeteringSectorSize, obj_s->position[1], 0});
        }
        // OY move case
        else if(m_transform[1][1] > 0.8)
        {
            ch_s = obj_s->owner_room->getSectorRaw({obj_s->position[0], obj_s->position[1] - MeteringSectorSize, 0});
        }
        else if(m_transform[1][1] < -0.8)
        {
            ch_s = obj_s->owner_room->getSectorRaw({obj_s->position[0], obj_s->position[1] + MeteringSectorSize, 0});
        }
        ch_s = ch_s->checkPortalPointer();
    }

    if(ch_s == nullptr || obj_s == nullptr)
    {
        return TraverseNone;
    }

    glm::float_t floor = m_transform[3][2];
    if(ch_s->floor != obj_s->floor || !allowTraverse(*ch_s, floor, *this) || !allowTraverse(*obj_s, floor, obj))
    {
        return TraverseNone;
    }

    engine::BtEngineClosestRayResultCallback cb(&obj);
    btVector3 v0, v1;
    v1[0] = v0[0] = obj_s->position[0];
    v1[1] = v0[1] = obj_s->position[1];
    v0[2] = floor + MeteringSectorSize * 0.5f;
    v1[2] = floor + MeteringSectorSize * 2.5f;
    engine::bt_engine_dynamicsWorld->rayTest(v0, v1, cb);
    if(cb.hasHit())
    {
        Object* object = static_cast<Object*>(cb.m_collisionObject->getUserPointer());
        Entity* e = dynamic_cast<Entity*>(object);
        if(e && (e->m_typeFlags & ENTITY_TYPE_TRAVERSE))
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
    if(m_transform[1][0] > 0.8)
    {
        next_s = obj_s->owner_room->getSectorRaw({static_cast<glm::float_t>(obj_s->position[0] + MeteringSectorSize),
                                                  static_cast<glm::float_t>(obj_s->position[1]), static_cast<glm::float_t>(0.0)});
    }
    else if(m_transform[1][0] < -0.8)
    {
        next_s = obj_s->owner_room->getSectorRaw({static_cast<glm::float_t>(obj_s->position[0] - MeteringSectorSize),
                                                  static_cast<glm::float_t>(obj_s->position[1]), static_cast<glm::float_t>(0.0)});
    }
    // OY move case
    else if(m_transform[1][1] > 0.8)
    {
        next_s = obj_s->owner_room->getSectorRaw({static_cast<glm::float_t>(obj_s->position[0]),
                                                  static_cast<glm::float_t>(obj_s->position[1] + MeteringSectorSize), static_cast<glm::float_t>(0.0)});
    }
    else if(m_transform[1][1] < -0.8)
    {
        next_s = obj_s->owner_room->getSectorRaw({static_cast<glm::float_t>(obj_s->position[0]),
                                                  static_cast<glm::float_t>(obj_s->position[1] - MeteringSectorSize), static_cast<glm::float_t>(0.0)});
    }

    if(next_s)
        next_s = next_s->checkPortalPointer();

    if(next_s != nullptr && allowTraverse(*next_s, floor, *this))
    {
        btTransform from;
        from.setIdentity();
        from.setOrigin(btVector3(obj_s->position[0], obj_s->position[1], floor + 0.5f * MeteringSectorSize));

        btTransform to;
        to.setIdentity();
        to.setOrigin(btVector3(next_s->position[0], next_s->position[1], floor + 0.5f * MeteringSectorSize));

        btSphereShape sp(CollisionTraverseTestRadius * MeteringSectorSize);
        sp.setMargin(COLLISION_MARGIN_DEFAULT);
        engine::BtEngineClosestConvexResultCallback ccb(&obj);
        engine::bt_engine_dynamicsWorld->convexSweepTest(&sp, from, to, ccb);

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
    if(m_transform[1][0] > 0.8)
    {
        next_s = ch_s->owner_room->getSectorRaw({ch_s->position[0] - MeteringSectorSize, ch_s->position[1], 0});
    }
    else if(m_transform[1][0] < -0.8)
    {
        next_s = ch_s->owner_room->getSectorRaw({ch_s->position[0] + MeteringSectorSize, ch_s->position[1], 0});
    }
    // OY move case
    else if(m_transform[1][1] > 0.8)
    {
        next_s = ch_s->owner_room->getSectorRaw({ch_s->position[0], ch_s->position[1] - MeteringSectorSize, 0});
    }
    else if(m_transform[1][1] < -0.8)
    {
        next_s = ch_s->owner_room->getSectorRaw({ch_s->position[0], ch_s->position[1] + MeteringSectorSize, 0});
    }

    if(next_s)
        next_s = next_s->checkPortalPointer();

    if(next_s != nullptr && allowTraverse(*next_s, floor, *this))
    {
        btTransform from;
        from.setIdentity();
        from.setOrigin(btVector3(ch_s->position[0], ch_s->position[1], floor + 0.5f * MeteringSectorSize));

        btTransform to;
        to.setIdentity();
        to.setOrigin(btVector3(next_s->position[0], next_s->position[1], floor + 0.5f * MeteringSectorSize));

        btSphereShape sp(CollisionTraverseTestRadius * MeteringSectorSize);
        sp.setMargin(COLLISION_MARGIN_DEFAULT);
        engine::BtEngineClosestConvexResultCallback ccb(this);
        engine::bt_engine_dynamicsWorld->convexSweepTest(&sp, from, to, ccb);

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

    m_stateController.handle(m_skeleton.getPreviousState());

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

    m_command.rot = {0, 0, 0};

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

            if(m_heightInfo.quicksand == QuicksandPosition::Drowning && m_moveType == MoveType::OnFloor)
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

            if(m_skeleton.getPreviousState() == LaraState::Sprint || m_skeleton.getPreviousState() == LaraState::SprintRoll)
            {
                changeParam(PARAM_STAMINA, -0.5);
            }
            else
            {
                changeParam(PARAM_STAMINA, 0.5);
            }
            break;

        case MoveType::OnWater:
            changeParam(PARAM_AIR, 3.0);
            ;
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
            break; // Add quicksand later...
    }
}

bool Character::setParamMaximum(int parameter, float max_value)
{
    if(parameter >= PARAM_SENTINEL)
        return false;

    max_value = max_value < 0 ? 0 : max_value; // Clamp max. to at least zero
    m_parameters.maximum[parameter] = max_value;
    return true;
}

bool Character::setParam(int parameter, float value)
{
    if(parameter >= PARAM_SENTINEL)
        return false;

    if(value == m_parameters.param[parameter])
        return false;

    float maximum = m_parameters.maximum[parameter];

    m_parameters.param[parameter] = glm::clamp(value, 0.0f, maximum);
    return true;
}

float Character::getParam(int parameter)
{
    if(parameter >= PARAM_SENTINEL)
        return 0;

    return m_parameters.param[parameter];
}

bool Character::changeParam(int parameter, float value)
{
    if(parameter >= PARAM_SENTINEL)
        return false;

    float maximum = m_parameters.maximum[parameter];
    float current = m_parameters.param[parameter];

    if(current == maximum && value > 0)
        return false;

    current += value;

    if(current < 0)
    {
        m_parameters.param[parameter] = 0;
        return false;
    }
    else if(current > maximum)
    {
        m_parameters.param[parameter] = m_parameters.maximum[parameter];
    }
    else
    {
        m_parameters.param[parameter] = current;
    }

    return true;
}

// overrided == 0x00: no overriding;
// overrided == 0x01: overriding mesh in armed state;
// overrided == 0x02: add mesh to slot in armed state;
// overrided == 0x03: overriding mesh in disarmed state;
// overrided == 0x04: add mesh to slot in disarmed state;
///@TODO: separate mesh replacing control and animation disabling / enabling
bool Character::setWeaponModel(ModelId weapon_model, bool armed)
{
    SkeletalModel* model = engine::engine_world.getModelByID(weapon_model);

    if(model != nullptr && m_skeleton.getBoneCount() == model->meshes.size() && model->animations.size() >= 4)
    {
        addOverrideAnim(weapon_model);

        for(size_t i = 0; i < m_skeleton.getModel()->meshes.size(); i++)
        {
            m_skeleton.bone(i).mesh = m_skeleton.getModel()->meshes[i].mesh_base;
            m_skeleton.bone(i).mesh_slot = nullptr;
        }

        if(armed)
        {
            for(size_t i = 0; i < m_skeleton.getModel()->meshes.size(); i++)
            {
                if(model->meshes[i].replace_mesh == 0x01)
                {
                    m_skeleton.bone(i).mesh = model->meshes[i].mesh_base;
                }
                else if(model->meshes[i].replace_mesh == 0x02)
                {
                    m_skeleton.bone(i).mesh_slot = model->meshes[i].mesh_base;
                }
            }
        }
        else
        {
            for(size_t i = 0; i < m_skeleton.getModel()->meshes.size(); i++)
            {
                if(model->meshes[i].replace_mesh == 0x03)
                {
                    m_skeleton.bone(i).mesh = model->meshes[i].mesh_base;
                }
                else if(model->meshes[i].replace_mesh == 0x04)
                {
                    m_skeleton.bone(i).mesh_slot = model->meshes[i].mesh_base;
                }
            }
        }

        return true;
    }
    else
    {
        // do unarmed default model
        const SkeletalModel* bm = m_skeleton.getModel();
        for(size_t i = 0; i < bm->meshes.size(); i++)
        {
            m_skeleton.bone(i).mesh = bm->meshes[i].mesh_base;
            m_skeleton.bone(i).mesh_slot = nullptr;
        }
    }

    return false;
}

void Character::fixPenetrations(const glm::vec3* move)
{
    if(!m_skeleton.hasGhosts())
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

    if(m_skeleton.getModel()->no_fix_all)
    {
        ghostUpdate();
        return;
    }

    glm::vec3 reaction;
    int numPenetrationLoops = getPenetrationFixVector(reaction, move != nullptr);
    m_transform[3] += glm::vec4(reaction, 0);

    updateCurrentHeight();
    if(move != nullptr && numPenetrationLoops > 0)
    {
        glm::float_t t1 = reaction[0] * reaction[0] + reaction[1] * reaction[1];
        glm::float_t t2 = move->x * move->x + move->y * move->y;
        if(reaction[2] * reaction[2] < t1 && move->z * move->z < t2) // we have horizontal move and horizontal correction
        {
            t2 *= t1;
            t1 = (reaction[0] * move->x + reaction[1] * move->y) / sqrtf(t2);
            if(t1 < m_criticalWallComponent)
            {
                m_response.horizontal_collide |= 0x01;
            }
        }
        else if(reaction[2] * reaction[2] > t1 && move->z * move->z > t2)
        {
            if(reaction[2] > 0.0 && move->z < 0.0)
            {
                m_response.vertical_collide |= 0x01;
            }
            else if(reaction[2] < 0.0 && move->z > 0.0)
            {
                m_response.vertical_collide |= 0x02;
            }
        }
    }

    if(m_heightInfo.ceiling.hasHit && reaction[2] < -0.1)
    {
        m_response.vertical_collide |= 0x02;
    }

    if(m_heightInfo.floor.hasHit && reaction[2] > 0.1)
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
int Character::checkNextPenetration(const glm::vec3& move)
{
    if(!m_skeleton.hasGhosts())
        return 0;

    ghostUpdate();
    m_transform[3] += glm::vec4(move, 0);
    // resp->horizontal_collide = 0x00;
    glm::vec3 reaction;
    int ret = getPenetrationFixVector(reaction, true);
    if(ret > 0)
    {
        glm::float_t t1 = reaction[0] * reaction[0] + reaction[1] * reaction[1];
        glm::float_t t2 = move[0] * move[0] + move[1] * move[1];
        if(reaction[2] * reaction[2] < t1 && move[2] * move[2] < t2)
        {
            t2 *= t1;
            t1 = (reaction[0] * move[0] + reaction[1] * move[1]) / sqrtf(t2);
            if(t1 < m_criticalWallComponent)
            {
                m_response.horizontal_collide |= 0x01;
            }
        }
    }
    m_transform[3] -= glm::vec4(move, 0);
    ghostUpdate();
    m_skeleton.cleanCollisionAllBodyParts();

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

        if(auto ownerChar = hair->m_ownerChar.lock())
        {
            hair->setRoom(ownerChar->getRoom());
        }
    }
}

void Character::frame(util::Duration time)
{
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
    if(isPlayer() && (engine::control_states.noclip || engine::control_states.free_look))
    {
        m_skeleton.updatePose();
        updateRigidBody(false); // bbox update, room update, m_transform from btBody...
        return;
    }

    fixPenetrations(nullptr);
    processSector();

    if(isPlayer()) // Player:
    {
        updateParams();
        checkCollisionCallbacks(); // physics collision checks, lua callbacks
    }
    else // Other Character entities:
    {
        engine_lua.loopEntity(getId());
        if(m_typeFlags & ENTITY_TYPE_COLLCHECK)
            checkCollisionCallbacks();
    }

    animation::AnimUpdate animStepResult = stepAnimation(time);
    if(m_skeleton.onFrame != nullptr)
    {
        m_skeleton.onFrame(*this, animStepResult);
    }

    applyCommands(); // state_func()

    if(m_command.action && (m_typeFlags & ENTITY_TYPE_TRIGGER_ACTIVATOR))
    {
        checkActivators(); // bbox check f. interact/pickup, lua callbacks
    }
    if(getParam(PARAM_HEALTH) <= 0.0)
    {
        m_response.killed = true; // Kill, if no HP.
    }
    updateHair();

    doWeaponFrame(time);

    // Update acceleration/speed, it is calculated per anim frame index
    const animation::Animation* af = &m_skeleton.getModel()->animations[m_skeleton.getCurrentAnimation()];
    m_currentSpeed = (af->speed_x + m_skeleton.getCurrentFrame() * af->accel_x) / float(1 << 16); // Decompiled from TOMB5.EXE

    // TODO: check rigidbody update requirements.
    // if(animStepResult != ENTITY_ANIM_NONE)
    //{ }
    m_skeleton.updatePose();
    updateRigidBody(false); // bbox update, room update, m_transform from btBody...
}

void Character::processSectorImpl()
{
    BOOST_ASSERT(m_currentSector != nullptr);
    RoomSector* highest_sector = m_currentSector->getHighestSector();
    BOOST_ASSERT(highest_sector != nullptr);
    RoomSector* lowest_sector = m_currentSector->getLowestSector();
    BOOST_ASSERT(lowest_sector != nullptr);

    m_heightInfo.walls_climb_dir = 0;
    m_heightInfo.walls_climb_dir |=
        lowest_sector->flags & (SECTOR_FLAG_CLIMB_WEST | SECTOR_FLAG_CLIMB_EAST | SECTOR_FLAG_CLIMB_NORTH | SECTOR_FLAG_CLIMB_SOUTH);

    m_heightInfo.walls_climb = m_heightInfo.walls_climb_dir > 0;
    m_heightInfo.ceiling_climb = false;

    if((highest_sector->flags & SECTOR_FLAG_CLIMB_CEILING) || (lowest_sector->flags & SECTOR_FLAG_CLIMB_CEILING))
    {
        m_heightInfo.ceiling_climb = true;
    }

    if(lowest_sector->flags & SECTOR_FLAG_DEATH)
    {
        if(m_moveType == MoveType::OnFloor || m_moveType == MoveType::Wade || m_moveType == MoveType::Quicksand)
        {
            if(m_heightInfo.floor.hasHit)
            {
                Object* object = static_cast<Object*>(m_heightInfo.floor.collisionObject->getUserPointer());

                if(dynamic_cast<Room*>(object))
                {
                    setParam(PARAM_HEALTH, 0.0);
                    m_response.killed = true;
                }
            }
        }
        else if(m_moveType == MoveType::Underwater || m_moveType == MoveType::OnWater)
        {
            setParam(PARAM_HEALTH, 0.0);
            m_response.killed = true;
        }
    }
}

void Character::jump(glm::float_t v_vertical, glm::float_t v_horizontal)
{
    glm::float_t t;

    // Jump length is a speed value multiplied by global speed coefficient.
    t = v_horizontal * animation::AnimationFrameRate;

    // Calculate the direction of jump by vector multiplication.
    if(m_moveDir == MoveDirection::Forward)
    {
        m_speed = glm::vec3(m_transform[1] * t);
    }
    else if(m_moveDir == MoveDirection::Backward)
    {
        m_speed = glm::vec3(m_transform[1] * -t);
    }
    else if(m_moveDir == MoveDirection::Left)
    {
        m_speed = glm::vec3(m_transform[0] * -t);
    }
    else if(m_moveDir == MoveDirection::Right)
    {
        m_speed = glm::vec3(m_transform[0] * t);
    }
    else
    {
        m_moveDir = MoveDirection::Forward;
        m_speed = {0, 0, 0};
    }

    m_response.vertical_collide = 0x00;

    m_response.slide = MovementWalk::None;
    m_response.lean = MovementStrafe::None;

    // Jump speed should NOT be added to current speed, as native engine
    // fully replaces current speed with jump speed by anim command.

    // Apply vertical speed.
    m_speed[2] = v_vertical * animation::AnimationFrameRate;
    m_moveType = MoveType::FreeFalling;
}

Substance Character::getSubstanceState() const
{
    if(getRoom()->m_flags & TR_ROOM_FLAG_QUICKSAND)
    {
        if(m_heightInfo.transition_level > m_transform[3][2] + m_height)
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
    else if(m_heightInfo.water && m_heightInfo.transition_level > m_transform[3][2] && m_heightInfo.transition_level < m_transform[3][2] + m_wadeDepth)
    {
        return Substance::WaterShallow;
    }
    else if(m_heightInfo.water && m_heightInfo.transition_level > m_transform[3][2] + m_wadeDepth)
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
    if(!m_skeleton.hasGhosts())
        return;

    for(const world::animation::Bone& bone : m_skeleton.getBones())
    {
        auto tr = bone.bt_body->getWorldTransform();
        tr.setOrigin(tr * util::convert(bone.mesh->m_center));
        bone.ghostObject->getWorldTransform() = tr;
    }
}

glm::vec3 Character::camPosForFollowing(glm::float_t dz)
{
    if(m_camFollowCenter > 0)
    {
        m_camFollowCenter--;
        return m_obb.center;
    }
    return Entity::camPosForFollowing(dz);
}

/* There are stick code for multianimation (weapon mode) testing
 * Model replacing will be upgraded too, I have to add override
 * flags to model manually in the script*/
void Character::doWeaponFrame(util::Duration time)
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
    if(m_command.ready_weapon && m_currentWeapon > 0 && m_currentWeaponState == WeaponState::Hide)
    {
        setWeaponModel(m_currentWeapon, true);
    }

    if(m_skeleton.getModel() != nullptr && m_skeleton.getModel()->animations.size() > 4)
    {
        // fixme: set weapon combat flag depending on specific weapon versions (pistols, uzi, revolver)
        m_skeleton.setAnimationMode(animation::AnimationMode::NormalControl);
        switch(m_currentWeaponState)
        {
            case WeaponState::Hide:
                if(m_command.ready_weapon) // ready weapon
                {
                    m_skeleton.setAnimation(1); // draw from holster
                                                // fixme: reset lerp:
                    m_skeleton.setPreviousAnimation(m_skeleton.getCurrentAnimation());
                    m_skeleton.setPreviousFrame(m_skeleton.getCurrentFrame());
                    m_currentWeaponState = WeaponState::HideToReady;
                }
                break;

            case WeaponState::HideToReady:
                if(m_skeleton.stepAnimation(time, this) == animation::AnimUpdate::NewAnim)
                {
                    m_skeleton.setAnimation(0); // hold drawn weapon to aim at target transition
                    m_currentWeaponState = WeaponState::Idle;
                }
                break;

            case WeaponState::Idle:
                if(m_command.ready_weapon)
                {
                    m_skeleton.setAnimation(3); // holster weapon
                    m_currentWeaponState = WeaponState::IdleToHide;
                }
                else if(m_command.action)
                {
                    m_currentWeaponState = WeaponState::IdleToFire;
                }
                else
                {
                    // stay at frame 0
                    m_skeleton.setAnimation(0); // hold drawn weapon to aim at target transition
                }
                break;

            case WeaponState::FireToIdle:
                // reverse stepping:
                // (there is a separate animation (4) for this, hence the original shotgun/bow don't reverse mid-anim)
                if(m_skeleton.stepAnimation(-time, this) == animation::AnimUpdate::NewAnim)
                {
                    m_skeleton.setAnimation(0); // hold drawn weapon to aim at target transition
                    m_currentWeaponState = WeaponState::Idle;
                }
                break;

            case WeaponState::IdleToFire:
                if(m_command.action)
                {
                    if(m_skeleton.stepAnimation(time, this) == animation::AnimUpdate::NewAnim)
                    {
                        m_skeleton.setAnimation(2); // shooting cycle
                        m_currentWeaponState = WeaponState::Fire;
                    }
                }
                else
                {
                    m_currentWeaponState = WeaponState::FireToIdle;
                }
                break;

            case WeaponState::Fire:
                if(m_command.action)
                {
                    if(m_skeleton.stepAnimation(time, this) == animation::AnimUpdate::NewAnim)
                    {
                        m_skeleton.setAnimation(2); // shooting cycle
                                                    // bang
                    }
                }
                else
                {
                    m_skeleton.setAnimation(0, -1); // hold drawn weapon to aim at target transition
                    m_currentWeaponState = WeaponState::FireToIdle;
                }
                break;

            case WeaponState::IdleToHide:
                if(m_skeleton.stepAnimation(time, this) == animation::AnimUpdate::NewAnim)
                {
                    m_currentWeaponState = WeaponState::Hide;
                    setWeaponModel(m_currentWeapon, false);
                }
                break;
        };
    }
    else if(m_skeleton.getModel() != nullptr && m_skeleton.getModel()->animations.size() == 4)
    {
        // fixme: set weapon combat flag depending on specific weapon versions (pistols, uzi, revolver)
        m_skeleton.setAnimationMode(animation::AnimationMode::WeaponCompat);
        switch(m_currentWeaponState)
        {
            case WeaponState::Hide:
                if(m_command.ready_weapon) // ready weapon
                {
                    m_skeleton.setAnimation(2); // draw from holster
                                                // fixme: reset lerp:
                    m_skeleton.setPreviousAnimation(m_skeleton.getCurrentAnimation());
                    m_skeleton.setPreviousFrame(m_skeleton.getCurrentFrame());
                    m_currentWeaponState = WeaponState::HideToReady;
                }
                break;

            case WeaponState::HideToReady:
                if(m_skeleton.stepAnimation(time, this) == animation::AnimUpdate::NewAnim)
                {
                    m_skeleton.setAnimation(0); // hold drawn weapon to aim at target transition
                    m_currentWeaponState = WeaponState::Idle;
                }
                break;

            case WeaponState::Idle:
                if(m_command.ready_weapon)
                {
                    m_skeleton.setAnimation(2, -1); // draw weapon, end for reverse
                    m_currentWeaponState = WeaponState::IdleToHide;
                }
                else if(m_command.action)
                {
                    m_currentWeaponState = WeaponState::IdleToFire;
                }
                else
                {
                    // stay
                    m_skeleton.setAnimation(0); // hold drawn weapon to aim at target transition
                }
                break;

            case WeaponState::FireToIdle:
                // reverse stepping:
                if(m_skeleton.stepAnimation(-time, this) == animation::AnimUpdate::NewAnim)
                {
                    m_skeleton.setAnimation(0); // hold drawn weapon to aim at target transition
                    m_currentWeaponState = WeaponState::Idle;
                }
                break;

            case WeaponState::IdleToFire:
                if(m_command.action)
                {
                    if(m_skeleton.stepAnimation(time, this) == animation::AnimUpdate::NewAnim)
                    {
                        m_skeleton.setAnimation(3); // shooting cycle
                        m_currentWeaponState = WeaponState::Fire;
                    }
                }
                else
                {
                    m_currentWeaponState = WeaponState::FireToIdle;
                }
                break;

            case WeaponState::Fire:
                if(m_command.action)
                {
                    if(m_skeleton.stepAnimation(time, this) == animation::AnimUpdate::NewAnim)
                    {
                        m_skeleton.setAnimation(3); // shooting cycle
                                                    // bang
                    }
                }
                else
                {
                    m_skeleton.setAnimation(0, -1); // hold drawn weapon to aim at target transition
                    m_currentWeaponState = WeaponState::FireToIdle;
                }
                break;

            case WeaponState::IdleToHide:
                // reverse stepping:
                if(m_skeleton.stepAnimation(-time, this) == animation::AnimUpdate::NewAnim)
                {
                    m_currentWeaponState = WeaponState::Hide;
                    setWeaponModel(m_currentWeapon, false);
                }
                break;
        };
    }
}

} // namespace world
