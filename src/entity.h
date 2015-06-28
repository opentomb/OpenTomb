#pragma once

#include <cstdint>
#include <memory>

#include <bullet/LinearMath/btVector3.h>
#include <bullet/BulletCollision/CollisionShapes/btCollisionShape.h>
#include <bullet/BulletDynamics/ConstraintSolver/btTypedConstraint.h>
#include <bullet/BulletCollision/CollisionDispatch/btGhostObject.h>
#include <bullet/BulletCollision/BroadphaseCollision/btCollisionAlgorithm.h>
#include "object.h"
#include "mesh.h"


class btCollisionShape;
class btRigidBody;

struct Room;
struct RoomSector;
struct OBB;
struct Character;
struct SSAnimation;
struct SSBoneFrame;
struct RDSetup;

#define ENTITY_STATE_ENABLED                        (0x0001)    // Entity is enabled.
#define ENTITY_STATE_ACTIVE                         (0x0002)    // Entity is animated.
#define ENTITY_STATE_VISIBLE                        (0x0004)    // Entity is visible.

#define ENTITY_TYPE_GENERIC                         (0x0000)    // Just an animating.
#define ENTITY_TYPE_INTERACTIVE                     (0x0001)    // Can respond to other entity's commands.
#define ENTITY_TYPE_TRIGGER_ACTIVATOR               (0x0002)    // Can activate triggers.
#define ENTITY_TYPE_HEAVYTRIGGER_ACTIVATOR          (0x0004)    // Can activate heavy triggers.
#define ENTITY_TYPE_PICKABLE                        (0x0008)    // Can be picked up.
#define ENTITY_TYPE_TRAVERSE                        (0x0010)    // Can be pushed/pulled.
#define ENTITY_TYPE_TRAVERSE_FLOOR                  (0x0020)    // Can be walked upon.
#define ENTITY_TYPE_DYNAMIC                         (0x0040)    // Acts as a physical dynamic object.
#define ENTITY_TYPE_ACTOR                           (0x0080)    // Is actor.

#define ENTITY_TYPE_SPAWNED                         (0x8000)    // Was spawned.

#define ENTITY_CALLBACK_NONE                        (0x00000000)
#define ENTITY_CALLBACK_ACTIVATE                    (0x00000001)
#define ENTITY_CALLBACK_DEACTIVATE                  (0x00000002)
#define ENTITY_CALLBACK_COLLISION                   (0x00000004)
#define ENTITY_CALLBACK_STAND                       (0x00000008)
#define ENTITY_CALLBACK_HIT                         (0x00000010)

#define ENTITY_COLLISION_GHOST                    0     // no one collisions
#define ENTITY_COLLISION_DYNAMIC                  1     // hallo full physics interaction
#define ENTITY_COLLISION_KINEMATIC                2     // doors and other moveable statics
#define ENTITY_COLLISION_STATIC                   3     // static object - never moved
#define ENTITY_COLLISION_ACTOR                    4     // actor, enemies, NPC, animals
#define ENTITY_COLLISION_VEHICLE                  5     // car, moto, bike

#define ENTITY_SUBSTANCE_NONE                     0
#define ENTITY_SUBSTANCE_WATER_SHALLOW            1
#define ENTITY_SUBSTANCE_WATER_WADE               2
#define ENTITY_SUBSTANCE_WATER_SWIM               3
#define ENTITY_SUBSTANCE_QUICKSAND_SHALLOW        4
#define ENTITY_SUBSTANCE_QUICKSAND_CONSUMED       5

#define ENTITY_TLAYOUT_MASK     0x1F    // Activation mask
#define ENTITY_TLAYOUT_EVENT    0x20    // Last trigger event
#define ENTITY_TLAYOUT_LOCK     0x40    // Activity lock
#define ENTITY_TLAYOUT_SSTATUS  0x80    // Sector status

#define WEAPON_STATE_HIDE                       (0x00)
#define WEAPON_STATE_HIDE_TO_READY              (0x01)
#define WEAPON_STATE_IDLE                       (0x02)
#define WEAPON_STATE_IDLE_TO_FIRE               (0x03)
#define WEAPON_STATE_FIRE                       (0x04)
#define WEAPON_STATE_FIRE_TO_IDLE               (0x05)
#define WEAPON_STATE_IDLE_TO_HIDE               (0x06)

// Specific in-game entity structure.

#define MAX_OBJECTS_IN_COLLSION_NODE    (4)

struct EntityCollisionNode
{
    uint16_t                    obj_count;
    btCollisionObject          *obj[MAX_OBJECTS_IN_COLLSION_NODE];
};

struct BtEntityData
{
    int8_t                              no_fix_all;
    uint32_t                            no_fix_body_parts;
    btPairCachingGhostObject          **ghostObjects;           // like Bullet character controller for penetration resolving.
    btManifoldArray                    *manifoldArray;          // keep track of the contact manifolds
    
    btCollisionShape                  **shapes;
    std::vector< std::shared_ptr<btRigidBody> > bt_body;
    std::vector<std::shared_ptr<btTypedConstraint>> bt_joints;              // Ragdoll joints
    
    EntityCollisionNode     *last_collisions;
};

struct Entity : public Object
{
    uint32_t                            m_id;                 // Unique entity ID
    int32_t                             m_OCB;                // Object code bit (since TR4)
    uint8_t                             m_triggerLayout;     // Mask + once + event + sector status flags
    float                               m_timer;              // Set by "timer" trigger field

    uint32_t                            m_callbackFlags;     // information about scripts callbacks
    uint16_t                            m_typeFlags;
    uint16_t                            m_stateFlags;

    uint8_t                             m_dirFlag;           // (move direction)
    uint16_t                            m_moveType;          // on floor / free fall / swim ....
    
    bool m_wasRendered;       // render once per frame trigger
    bool m_wasRenderedLines; // same for debug lines

    btScalar                            m_currentSpeed;      // current linear speed from animation info
    btVector3                           m_speed;              // speed of the entity XYZ
    
    btScalar                            m_inertiaLinear;     // linear inertia
    btScalar                            m_inertiaAngular[2]; // angular inertia - X and Y axes
    
    SSBoneFrame m_bf;                 // current boneframe with full frame information
    BtEntityData m_bt;
    btVector3 m_angles;
    btTransform m_transform; // GL transformation matrix

    std::unique_ptr<OBB> m_obb;                // oriented bounding box

    RoomSector* m_currentSector;
    RoomSector* m_lastSector;

    std::shared_ptr<EngineContainer> m_self;

    btVector3 m_activationOffset;   // where we can activate object (dx, dy, dz)
    btScalar m_activationRadius = 128;
    
    std::shared_ptr<Character> m_character;

    Entity();
    ~Entity();

    void createGhosts();
    void enable();
    void disable();
    void enableCollision();
    void disableCollision();
    void genEntityRigidBody();

    void ghostUpdate();
    void updateCurrentCollisions();
    int getPenetrationFixVector(btVector3 *reaction, bool hasMove);
    void fixPenetrations(btVector3 *move);
    int checkNextPenetration(const btVector3& move);
    void checkCollisionCallbacks();
    bool wasCollisionBodyParts(uint32_t parts_flags);
    void cleanCollisionAllBodyParts();
    void cleanCollisionBodyParts(uint32_t parts_flags);
    btCollisionObject* getRemoveCollisionBodyParts(uint32_t parts_flags, uint32_t *curr_flag);
    void updateRoomPos();
    void updateRigidBody(bool force);
    void rebuildBV();

    int  getAnimDispatchCase(uint32_t id);
    static void getNextFrame(SSBoneFrame *bf, btScalar time, StateChange *stc, int16_t *frame, int16_t *anim, uint16_t anim_flags);
    int  frame(btScalar time);  // process frame + trying to change state

    void updateRotation();
    void updateCurrentSpeed(bool zeroVz = 0);
    void addOverrideAnim(int model_id);
    void checkActivators();

    int  getSubstanceState();

    static void updateCurrentBoneFrame(SSBoneFrame *bf, const btTransform *etr);
    void doAnimCommands(SSAnimation *ss_anim, int changing);
    void processSector();
    void setAnimation(int animation, int frame = 0, int another_model = -1);
    void moveForward(btScalar dist);
    void moveStrafe(btScalar dist);
    void moveVertical(btScalar dist);

    btScalar findDistance(const Entity& entity_2);
    RoomSector* getLowestSector(RoomSector* sector);
    RoomSector* getHighestSector(RoomSector* sector);

    // Constantly updates some specific parameters to keep hair aligned to entity.
    void updateHair();

    bool createRagdoll(RDSetup* setup);
    bool deleteRagdoll();

private:
    void doAnimMove(int16_t *anim, int16_t *frame);
    void doWeaponFrame(btScalar time);

    static btScalar getInnerBBRadius(const btVector3& bb_min, const btVector3& bb_max)
    {
        btVector3 d = bb_max-bb_min;
        return std::min(d[0], std::min(d[1], d[2]));
    }
};

int Ghost_GetPenetrationFixVector(btPairCachingGhostObject *ghost, btManifoldArray *manifoldArray, btVector3 *correction);

struct StateChange *Anim_FindStateChangeByAnim(struct AnimationFrame *anim, int state_change_anim);
struct StateChange *Anim_FindStateChangeByID(struct AnimationFrame *anim, uint32_t id);
