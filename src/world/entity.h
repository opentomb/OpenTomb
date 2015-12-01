#pragma once

#include <cstdint>
#include <memory>

#include <BulletCollision/BroadphaseCollision/btCollisionAlgorithm.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>

#include "animation/animation.h"
#include "core/orientedboundingbox.h"
#include "engine/engine.h"
#include "engine/game.h"
#include "object.h"

class btCollisionShape;
class btRigidBody;

namespace engine
{
class BtEngineClosestConvexResultCallback;
} // namespace engine

namespace world
{
struct Room;
struct RoomSector;
struct RDSetup;
struct Character;

namespace core
{
struct OrientedBoundingBox;
} // namespace core

#define ENTITY_TYPE_GENERIC                         (0x0000)    // Just an animating.
#define ENTITY_TYPE_INTERACTIVE                     (0x0001)    // Can respond to other entity's commands.
#define ENTITY_TYPE_TRIGGER_ACTIVATOR               (0x0002)    // Can activate triggers.
#define ENTITY_TYPE_HEAVYTRIGGER_ACTIVATOR          (0x0004)    // Can activate heavy triggers.
#define ENTITY_TYPE_PICKABLE                        (0x0008)    // Can be picked up.
#define ENTITY_TYPE_TRAVERSE                        (0x0010)    // Can be pushed/pulled.
#define ENTITY_TYPE_TRAVERSE_FLOOR                  (0x0020)    // Can be walked upon.
#define ENTITY_TYPE_DYNAMIC                         (0x0040)    // Acts as a physical dynamic object.
#define ENTITY_TYPE_ACTOR                           (0x0080)    // Is actor.
#define ENTITY_TYPE_COLLCHECK                       (0x0100)    // Does collision checks for itself.

#define ENTITY_TYPE_SPAWNED                         (0x8000)    // Was spawned.

#define ENTITY_CALLBACK_NONE                        (0x00000000)
#define ENTITY_CALLBACK_ACTIVATE                    (0x00000001)
#define ENTITY_CALLBACK_DEACTIVATE                  (0x00000002)
#define ENTITY_CALLBACK_COLLISION                   (0x00000004)
#define ENTITY_CALLBACK_STAND                       (0x00000008)
#define ENTITY_CALLBACK_HIT                         (0x00000010)
#define ENTITY_CALLBACK_ROOMCOLLISION               (0x00000020)

enum class Substance
{
    None,
    WaterShallow,
    WaterWade,
    WaterSwim,
    QuicksandShallow,
    QuicksandConsumed
};

#define ENTITY_TLAYOUT_MASK     0x1F    // Activation mask
#define ENTITY_TLAYOUT_EVENT    0x20    // Last trigger event
#define ENTITY_TLAYOUT_LOCK     0x40    // Activity lock
#define ENTITY_TLAYOUT_SSTATUS  0x80    // Sector status

// Specific in-game entity structure.

struct BtEntityData
{
};


/*
 * ENTITY MOVEMENT TYPES
 */
enum class MoveType
{
   StaticPos,
   Kinematic,
   OnFloor,
   Wade,
   Quicksand,
   OnWater,
   Underwater,
   FreeFalling,
   Climbing,
   Monkeyswing,
   WallsClimb,
   Dozy
};

// Surface movement directions
enum class MoveDirection
{
    Stay,
    Forward,
    Backward,
    Left,
    Right,
    Jump,
    Crouch
};

struct Entity : public Object
{
public:
    int32_t                             m_OCB = 0;                // Object code bit (since TR4)
    uint8_t                             m_triggerLayout = 0;     // Mask + once + event + sector status flags
    float                               m_timer = 0;              // Set by "timer" trigger field

    uint32_t                            m_callbackFlags = 0;     // information about scripts callbacks
    uint16_t                            m_typeFlags = ENTITY_TYPE_GENERIC;
    bool m_enabled = true;
    bool m_active = true;
    bool m_visible = true;

    MoveDirection                       m_moveDir = MoveDirection::Stay;           // (move direction)
    MoveType                            m_moveType = MoveType::OnFloor;          // on floor / free fall / swim ....

    bool m_wasRendered = false;       // render once per frame trigger
    bool m_wasRenderedLines = false; // same for debug lines

    glm::float_t                        m_currentSpeed = 0;      // current linear speed from animation info
    glm::vec3                           m_speed = {0,0,0};              // speed of the entity XYZ
    glm::float_t                        m_vspeed_override = 0;

    btScalar                            m_inertiaLinear = 0;     // linear inertia
    btScalar                            m_inertiaAngular[2] = {0,0}; // angular inertia - X and Y axes

    animation::Skeleton m_skeleton;

    glm::vec3 m_angles = { 0,0,0 };
    glm::mat4 m_transform{ 1.0f }; // GL transformation matrix
    glm::vec3 m_scaling = { 1,1,1 };

#if 0
    bool m_lerp_skip = false;
#endif

    core::OrientedBoundingBox m_obb;

    RoomSector* m_currentSector = nullptr;
    RoomSector* m_lastSector = nullptr;

    glm::vec3 m_activationOffset = { 0,256,0 };   // where we can activate object (dx, dy, dz)
    glm::float_t m_activationRadius = 128;

    Entity(uint32_t id);
    ~Entity();

    void enable();
    void disable();

    void ghostUpdate();
    int getPenetrationFixVector(glm::vec3 *reaction, bool hasMove);
    void checkCollisionCallbacks();
    bool wasCollisionBodyParts(uint32_t parts_flags) const;
    void updateRoomPos();
    void updateRigidBody(bool force);
    void rebuildBoundingBox();

    int  getAnimDispatchCase(LaraState id) const;

    animation::AnimUpdate stepAnimation(util::Duration time);
    virtual void frame(util::Duration time);  // entity frame step

    bool isPlayer()
    {
        // FIXME: isPlayer()
        return reinterpret_cast<Entity*>(engine::engine_world.character.get()) == this;
    }

    void updateInterpolation();

    virtual void updateTransform();
    void updateCurrentSpeed(bool zeroVz = 0);
    void addOverrideAnim(ModelId model_id);
    void checkActivators();

    virtual Substance getSubstanceState() const
    {
        return Substance::None;
    }

    void doAnimCommand(const animation::AnimCommand& command);
    void processSector();
    void setAnimation(int animation, int frame = 0);
    void moveForward(glm::float_t dist);
    void moveStrafe(glm::float_t dist);
    void moveVertical(glm::float_t dist);

    glm::float_t findDistance(const Entity& entity_2);

    // Constantly updates some specific parameters to keep hair aligned to entity.
    virtual void updateHair()
    {
    }

    bool createRagdoll(RDSetup* setup);
    bool deleteRagdoll();

    virtual void fixPenetrations(const glm::vec3* move);
    virtual glm::vec3 getRoomPos() const
    {
        return glm::vec3(m_transform * glm::vec4(m_skeleton.getBoundingBox().getCenter(), 1.0f));
    }
    virtual void transferToRoom(Room *room);

    virtual void processSectorImpl()
    {
    }
    virtual void jump(glm::float_t /*vert*/, glm::float_t /*hor*/)
    {
    }
    virtual void kill()
    {
    }
    virtual void updateGhostRigidBody()
    {
    }
    virtual std::shared_ptr<engine::BtEngineClosestConvexResultCallback> callbackForCamera() const;

    virtual glm::vec3 camPosForFollowing(glm::float_t dz)
    {
        glm::vec4 cam_pos = m_transform * m_skeleton.getRootTransform()[3];
        cam_pos[2] += dz;
        return glm::vec3(cam_pos);
    }

    virtual void updatePlatformPreStep()
    {
    }

    glm::vec3 applyGravity(util::Duration time);

private:
//    void doAnimMove(int16_t *anim, int16_t *frame);

    static glm::float_t getInnerBBRadius(const core::BoundingBox& bb)
    {
        auto d = bb.max - bb.min;
        return glm::min(d[0], glm::min(d[1], d[2]));
    }
};

int Ghost_GetPenetrationFixVector(btPairCachingGhostObject *ghost, btManifoldArray *manifoldArray, glm::vec3 *correction);

} // namespace world
