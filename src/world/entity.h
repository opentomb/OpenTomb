#pragma once

#include <cstdint>
#include <memory>

#include <BulletCollision/BroadphaseCollision/btCollisionAlgorithm.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <BulletCollision/CollisionShapes/btCollisionShape.h>
#include <BulletDynamics/ConstraintSolver/btTypedConstraint.h>

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
struct EngineContainer;
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

struct EntityCollisionNode
{
    std::vector<btCollisionObject*> obj;
};

struct BtEntityData
{
    bool no_fix_all;
    uint32_t no_fix_body_parts;
    std::vector<std::unique_ptr<btPairCachingGhostObject>> ghostObjects;           // like Bullet character controller for penetration resolving.
    std::unique_ptr<btManifoldArray> manifoldArray;          // keep track of the contact manifolds

    std::vector<std::unique_ptr<btCollisionShape>> shapes;
    std::vector< std::shared_ptr<btRigidBody> > bt_body;
    std::vector<std::shared_ptr<btTypedConstraint>> bt_joints;              // Ragdoll joints

    std::vector<EntityCollisionNode> last_collisions;
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
private:
    const uint32_t m_id;                 // Unique entity ID
public:
    uint32_t id() const noexcept
    {
        return m_id;
    }

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

    bool m_wasRendered;       // render once per frame trigger
    bool m_wasRenderedLines; // same for debug lines

    btScalar                            m_currentSpeed;      // current linear speed from animation info
    btVector3                           m_speed;              // speed of the entity XYZ
    btScalar                            m_vspeed_override;

    btScalar                            m_inertiaLinear;     // linear inertia
    btScalar                            m_inertiaAngular[2]; // angular inertia - X and Y axes

    animation::SSBoneFrame m_bf;                 // current boneframe with full frame information
    BtEntityData m_bt;
    btVector3 m_angles;
    btTransform m_transform; // GL transformation matrix
    btVector3 m_scaling = { 1,1,1 };

    btTransform m_lerp_last_transform; // interp
    btTransform m_lerp_curr_transform; // interp
    bool        m_lerp_valid = false;
    bool        m_lerp_skip = false;
    btScalar    m_lerp = 0;

    core::OrientedBoundingBox m_obb;

    RoomSector* m_currentSector = nullptr;
    RoomSector* m_lastSector;

    std::shared_ptr<engine::EngineContainerImpl<Entity>> m_self;

    btVector3 m_activationOffset = { 0,256,0 };   // where we can activate object (dx, dy, dz)
    btScalar m_activationRadius = 128;

    Entity(uint32_t id);
    ~Entity();

    void createGhosts();
    void enable();
    void disable();
    void enableCollision();
    void disableCollision();
    void genRigidBody();

    void ghostUpdate();
    void updateCurrentCollisions();
    int getPenetrationFixVector(btVector3 *reaction, bool hasMove);
    void checkCollisionCallbacks();
    bool wasCollisionBodyParts(uint32_t parts_flags);
    void cleanCollisionAllBodyParts();
    void cleanCollisionBodyParts(uint32_t parts_flags);
    btCollisionObject* getRemoveCollisionBodyParts(uint32_t parts_flags, uint32_t *curr_flag);
    void updateRoomPos();
    void updateRigidBody(bool force);
    void rebuildBV();

    int  getAnimDispatchCase(uint32_t id);

    animation::AnimUpdate stepAnimation(btScalar time);
    virtual void frame(btScalar time);  // entity frame step

    bool isPlayer()
    {
        // FIXME: isPlayer()
        return reinterpret_cast<Entity*>(engine::engine_world.character.get()) == this;
    }

    void updateInterpolation(btScalar time);

    virtual void updateTransform();
    void updateCurrentSpeed(bool zeroVz = 0);
    void addOverrideAnim(int model_id);
    void checkActivators();

    virtual Substance getSubstanceState() const
    {
        return Substance::None;
    }

    void doAnimCommand(const animation::AnimCommand& command);
    void processSector();
    void setAnimation(int animation, int frame = 0, int another_model = -1);
    void moveForward(btScalar dist);
    void moveStrafe(btScalar dist);
    void moveVertical(btScalar dist);

    btScalar findDistance(const Entity& entity_2);

    // Constantly updates some specific parameters to keep hair aligned to entity.
    virtual void updateHair()
    {
    }

    bool createRagdoll(RDSetup* setup);
    bool deleteRagdoll();

    virtual void fixPenetrations(const btVector3* move);
    virtual btVector3 getRoomPos() const
    {
        return m_transform * m_bf.boundingBox.getCenter();
    }
    virtual void transferToRoom(Room *room);

    virtual void processSectorImpl()
    {
    }
    virtual void jump(btScalar /*vert*/, btScalar /*hor*/)
    {
    }
    virtual void kill()
    {
    }
    virtual void updateGhostRigidBody()
    {
    }
    virtual std::shared_ptr<engine::BtEngineClosestConvexResultCallback> callbackForCamera() const;

    virtual btVector3 camPosForFollowing(btScalar dz)
    {
        auto cam_pos = m_transform * m_bf.bone_tags.front().full_transform.getOrigin();
        cam_pos[2] += dz;
        return cam_pos;
    }

    virtual void updatePlatformPreStep()
    {
    }

    btVector3 applyGravity(btScalar time);

private:
//    void doAnimMove(int16_t *anim, int16_t *frame);
    void slerpBones(btScalar lerp);
    void lerpTransform(btScalar lerp);

    static btScalar getInnerBBRadius(const btVector3& bb_min, const btVector3& bb_max)
    {
        btVector3 d = bb_max - bb_min;
        return btMin(d[0], btMin(d[1], d[2]));
    }
};

int Ghost_GetPenetrationFixVector(btPairCachingGhostObject *ghost, btManifoldArray *manifoldArray, btVector3 *correction);

} // namespace world
