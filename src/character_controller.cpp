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

    return 0x00;
}
