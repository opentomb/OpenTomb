#include "character_controller.h"

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

#include "engine/anim_state_control.h"
#include "engine/engine.h"
#include "world/entity.h"
#include "gui/gui.h"
#include "world/hair.h"
#include "world/core/orientedboundingbox.h"
#include "world/core/polygon.h"
#include "world/resource.h"
#include "script/script.h"
#include "util/vmath.h"
#include "world/room.h"
#include "world/world.h"

/**
 *
 * @param rs: room sector pointer
 * @param floor: floor height
 * @return 0x01: can traverse, 0x00 can not;
 */
int Sector_AllowTraverse(world::RoomSector *rs, btScalar floor, const std::shared_ptr<engine::EngineContainer>& container)
{
    btScalar f0 = rs->floor_corners[0][2];
    if((rs->floor_corners[0][2] != f0) || (rs->floor_corners[1][2] != f0) ||
       (rs->floor_corners[2][2] != f0) || (rs->floor_corners[3][2] != f0))
    {
        return 0x00;
    }

    if((std::abs(floor - f0) < 1.1) && (rs->ceiling - rs->floor >= world::MeteringSectorSize))
    {
        return 0x01;
    }

    engine::BtEngineClosestRayResultCallback cb(container);
    btVector3 from, to;
    to[0] = from[0] = rs->position[0];
    to[1] = from[1] = rs->position[1];
    from[2] = floor + world::MeteringSectorSize * 0.5f;
    to[2] = floor - world::MeteringSectorSize * 0.5f;
    engine::bt_engine_dynamicsWorld->rayTest(from, to, cb);
    if(cb.hasHit())
    {
        btVector3 v;
        v.setInterpolate3(from, to, cb.m_closestHitFraction);
        if(std::abs(v[2] - floor) < 1.1)
        {
            engine::EngineContainer* cont = static_cast<engine::EngineContainer*>(cb.m_collisionObject->getUserPointer());
            if((cont != nullptr) && (cont->object_type == OBJECT_ENTITY) && ((static_cast<world::Entity*>(cont->object))->m_typeFlags & ENTITY_TYPE_TRAVERSE_FLOOR))
            {
                return 0x01;
            }
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
}

/*
 * Linear inertia is absolutely needed for in-water states, and also it gives
 * more organic feel to land animations.
 */
btScalar Character::inertiaLinear(btScalar max_speed, btScalar accel, bool command)
{
}

/*
 * Angular inertia is used on keyboard-driven (non-analog) rotational controls.
 */
btScalar Character::inertiaAngular(btScalar max_angle, btScalar accel, uint8_t axis)
{
}

/*
 * MOVE IN DIFFERENT CONDITIONS
 */
int Character::moveOnFloor()
{
}

int Character::freeFalling()
{
}

/*
 * Monkey CLIMBING - MOVE NO Z LANDING
 */
int Character::monkeyClimbing()
{
}

/*
 * WALLS CLIMBING - MOVE IN ZT plane
 */
int Character::wallsClimbing()
{
}

/*
 * CLIMBING - MOVE NO Z LANDING
 */
int Character::climbing()
{
}

/*
 * underwater and onwater swimming has a big trouble:
 * the speed and acceleration information is absent...
 * I add some sticks to make it work for testing.
 * I thought to make export anim information to LUA script...
 */
int Character::moveUnderWater()
{
}

int Character::moveOnWater()
{
}

int Character::findTraverse()
{
}

/**
 *
 * @param obj Traversed object pointer
 * @see TraverseNone TraverseForward TraverseBackward
 */
int Character::checkTraverse(const Entity& obj)
{
}

/**
 * Main character frame function
 */
void Character::applyCommands()
{
}

void Character::updateParams()
{
}

bool IsCharacter(std::shared_ptr<Entity> ent)
{
    return std::dynamic_pointer_cast<Character>(ent) != nullptr;
}

int Character::setParamMaximum(int parameter, float max_value)
{
}

int Character::setParam(int parameter, float value)
{
}

float Character::getParam(int parameter)
{
}

int Character::changeParam(int parameter, float value)
{
}

// overrided == 0x00: no overriding;
// overrided == 0x01: overriding mesh in armed state;
// overrided == 0x02: add mesh to slot in armed state;
// overrided == 0x03: overriding mesh in disarmed state;
// overrided == 0x04: add mesh to slot in disarmed state;
///@TODO: separate mesh replacing control and animation disabling / enabling
int Character::setWeaponModel(int weapon_model, int armed)
{
}

void Character::fixPenetrations(const btVector3* move)
{
}

/**
 * we check walls and other collision objects reaction. if reaction more than critical
 * then cmd->horizontal_collide |= 0x01;
 * @param move absolute 3d move vector
 */
int Character::checkNextPenetration(const btVector3& move)
{
}

void Character::updateHair()
{
}

/**
 * Character framestep actions
 * @param time      frame time
 */
void Character::frame(btScalar time)
{
}

void Character::processSectorImpl()
{
}

void Character::jump(btScalar v_vertical, btScalar v_horizontal)
{
}

Substance Character::getSubstanceState() const
{
}

void Character::updateGhostRigidBody()
{
}

btVector3 Character::camPosForFollowing(btScalar dz)
{
}

/* There are stick code for multianimation (weapon mode) testing
 * Model replacing will be upgraded too, I have to add override
 * flags to model manually in the script*/
void Character::doWeaponFrame(btScalar time)
{
}
