#pragma once

#include <cstdint>
#include <vector>
#include <list>

#include <LinearMath/btScalar.h>
#include <LinearMath/btVector3.h>
#include <BulletCollision/CollisionShapes/btCapsuleShape.h>
#include <BulletCollision/CollisionShapes/btSphereShape.h>
#include <BulletCollision/CollisionDispatch/btCollisionObject.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <BulletCollision/CollisionShapes/btBoxShape.h>
#include <BulletCollision/BroadphaseCollision/btCollisionAlgorithm.h>
#include <BulletCollision/CollisionShapes/btMultiSphereShape.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>

#include "engine.h"
#include "entity.h"

/*------ Lara's model-------
             .=.
            | 14|
             \ / \
         / |     | \
    11  / |   7   | \  8
       /   |     |   \
       |    =====    |
    12 |    =====    | 9
       |   /  0  \   |
    13 0  /_______\  0 10
          |  | |  |
          |1 | |4 |
          |  | |  |
          |__| |__|
          |  | |  |
          |2 | |5 |
          |  | |  |
          |__| |__|
       3  |__| |__|  6
--------------------------*/

#define BODY_PART_BODY_LOW      (0x00000001)            // 0
#define BODY_PART_BODY_UPPER    (0x00000002)            // 7
#define BODY_PART_HEAD          (0x00000004)            // 14

#define BODY_PART_LEFT_HAND_1   (0x00000008)            // 11
#define BODY_PART_LEFT_HAND_2   (0x00000010)            // 12
#define BODY_PART_LEFT_HAND_3   (0x00000020)            // 13
#define BODY_PART_RIGHT_HAND_1  (0x00000040)            // 8
#define BODY_PART_RIGHT_HAND_2  (0x00000080)            // 9
#define BODY_PART_RIGHT_HAND_3  (0x00000100)            // 10

#define BODY_PART_LEFT_LEG_1    (0x00000200)            // 1
#define BODY_PART_LEFT_LEG_2    (0x00000400)            // 2
#define BODY_PART_LEFT_LEG_3    (0x00000800)            // 3
#define BODY_PART_RIGHT_LEG_1   (0x00001000)            // 4
#define BODY_PART_RIGHT_LEG_2   (0x00002000)            // 5
#define BODY_PART_RIGHT_LEG_3   (0x00004000)            // 6

#define BODY_PART_LEGS_1        (BODY_PART_LEFT_LEG_1 | BODY_PART_RIGHT_LEG_1)
#define BODY_PART_LEGS_2        (BODY_PART_LEFT_LEG_2 | BODY_PART_RIGHT_LEG_2)
#define BODY_PART_LEGS_3        (BODY_PART_LEFT_LEG_3 | BODY_PART_RIGHT_LEG_3)

#define BODY_PART_HANDS_1        (BODY_PART_LEFT_HAND_1 | BODY_PART_RIGHT_HAND_1)
#define BODY_PART_HANDS_2        (BODY_PART_LEFT_HAND_2 | BODY_PART_RIGHT_HAND_2)
#define BODY_PART_HANDS_3        (BODY_PART_LEFT_HAND_3 | BODY_PART_RIGHT_HAND_3)

#define BODY_PART_HANDS          (BODY_PART_HANDS_1 | BODY_PART_HANDS_2 | BODY_PART_HANDS_3)
#define BODY_PART_LEGS           (BODY_PART_LEGS_1 | BODY_PART_LEGS_2 | BODY_PART_LEGS_3)

#define CHARACTER_BOX_HALF_SIZE (128.0)
#define CHARACTER_BASE_RADIUS   (128.0)
#define CHARACTER_BASE_HEIGHT   (512.0)


/*
 * default legs offsets
 */
#define LEFT_LEG                    (3)
#define RIGHT_LEG                   (6)
#define LEFT_HAND                   (13)
#define RIGHT_HAND                  (10)

/*
 * ENTITY MOVEMENT TYPES
 */

#define MOVE_STATIC_POS         (0)
#define MOVE_KINEMATIC          (1)
#define MOVE_ON_FLOOR           (2)
#define MOVE_WADE               (3)
#define MOVE_QUICKSAND          (4)
#define MOVE_ON_WATER           (5)
#define MOVE_UNDERWATER         (6)
#define MOVE_FREE_FALLING       (7)
#define MOVE_CLIMBING           (8)
#define MOVE_MONKEYSWING        (9)
#define MOVE_WALLS_CLIMB        (10)
#define MOVE_DOZY               (11)

#define CHARACTER_USE_COMPLEX_COLLISION         (1)

// Lara's character behavior constants
#define DEFAULT_MAX_MOVE_ITERATIONS             (3)                             ///@FIXME: magic
#define DEFAULT_MIN_STEP_UP_HEIGHT              (128.0)                         ///@FIXME: check original
#define DEFAULT_MAX_STEP_UP_HEIGHT              (256.0 + 32.0)                  ///@FIXME: check original
#define DEFAULT_FALL_DOWN_HEIGHT                (320.0)                         ///@FIXME: check original
#define DEFAULT_CLIMB_UP_HEIGHT                 (1920.0)                        ///@FIXME: check original
#define DEFAULT_CRITICAL_SLANT_Z_COMPONENT      (0.810)                         ///@FIXME: cos(alpha = 30 deg)
#define DEFAULT_CRITICAL_WALL_COMPONENT         (-0.707)                        ///@FIXME: cos(alpha = 45 deg)
#define DEFAULT_CHARACTER_SLIDE_SPEED_MULT      (75.0)                          ///@FIXME: magic - not like in original
#define DEFAULT_CHARACTER_CLIMB_R               (32.0)
#define DEFAULT_CHARACTER_WADE_DEPTH            (256.0)
// If less than this much of Lara is looking out of the water, she goes from wading to swimming.
#define DEFAULT_CHARACTER_SWIM_DEPTH            (100.0) ///@FIXME: Guess

// Speed limits

#define FREE_FALL_SPEED_1        (2000.0)
#define FREE_FALL_SPEED_2        (4500.0)
#define FREE_FALL_SPEED_MAXSAFE  (5500.0)
#define FREE_FALL_SPEED_CRITICAL (7500.0)
#define FREE_FALL_SPEED_MAXIMUM  (7800.0)

#define MAX_SPEED_UNDERWATER     (64.0)
#define MAX_SPEED_ONWATER        (24.0)
#define MAX_SPEED_QUICKSAND      (5.0 )

#define ROT_SPEED_UNDERWATER     (2.0)
#define ROT_SPEED_ONWATER        (3.0)
#define ROT_SPEED_LAND           (4.5)
#define ROT_SPEED_FREEFALL       (0.5)
#define ROT_SPEED_MONKEYSWING    (3.5)

#define INERTIA_SPEED_UNDERWATER (1.0)
#define INERTIA_SPEED_ONWATER    (1.5)

// flags constants
enum class SlideType
{
    None,
    Back,
    Front
};

/*
 * Next step height information
 */
enum class NextStepInfo
{
    DownCanHang, //!< enough height to hang here
    DownDrop,    //!< big height, cannot walk next, drop only
    DownBig,     //!< enough height change, step down is needed
    DownLittle,  //!< too little height change, step down is not needed
    Horizontal,  //!< horizontal plane
    UpLittle,    //!< too little height change, step up is not needed
    UpBig,       //!< enough height change, step up is needed
    UpClimb,     //!< big height, cannot walk next, climb only
    UpImpossible //!< too big height, no one ways here, or phantom case
};

// CHARACTER PARAMETERS TYPES

enum CharParameters
{
    PARAM_HEALTH,
    PARAM_AIR,
    PARAM_STAMINA,
    PARAM_WARMTH,
    PARAM_POISON,
    PARAM_EXTRA1,
    PARAM_EXTRA2,
    PARAM_EXTRA3,
    PARAM_EXTRA4,
    PARAM_SENTINEL
};

// CHARACTER PARAMETERS DEFAULTS

#define PARAM_ABSOLUTE_MAX                (-1)

#define LARA_PARAM_HEALTH_MAX             (1000.0)      // 1000 HP
#define LARA_PARAM_AIR_MAX                (3600.0)      // 60 secs of air
#define LARA_PARAM_STAMINA_MAX            (120.0)       // 4  secs of sprint
#define LARA_PARAM_WARMTH_MAX             (240.0)       // 8  secs of freeze
#define LARA_PARAM_POISON_MAX             (5.0)

struct EngineContainer;
struct Entity;
class BtEngineClosestConvexResultCallback;
class BtEngineClosestRayResultCallback;
class btCollisionObject;
class btConvexShape;

enum class ClimbType
{
    NoClimb,   //!< No climability at all
    HandsOnly, //!< Hands only climbability
    FullClimb  //!< Full body climbability (hands and feet)
};

struct ClimbInfo
{
    NextStepInfo                   height_info = NextStepInfo::Horizontal;
    bool                           can_hang = false;

    btVector3 point;
    btVector3 n;
    btVector3                      right_dir = {0,0,0}; //!< Where to climb to when climbing "right"
    btVector3                      up_dir = {0,0,0};    //!< Where to climb to when climbing "up"
    btScalar                       floor_limit   = -9e10;
    btScalar                       ceiling_limit = +9e10;
    btScalar                       next_z_space  = 0;

    ClimbType                      wall_hit = ClimbType::NoClimb;
    ClimbType                      edge_hit = ClimbType::NoClimb;
    btVector3                      edge_point;  //!< The point we're grabbing the edge
    btVector3                      edge_tan_xy; //!< The edge's normalized direction, excluding vertical component
    btScalar                       edge_z_ang;  //!< Z Angle of the edge we're hanging on
    btCollisionObject             *edge_obj = nullptr;
};

struct HeightInfo
{
    HeightInfo()
    {
        sp->setMargin(COLLISION_MARGIN_DEFAULT);
    }

    std::shared_ptr<BtEngineClosestRayResultCallback> cb;
    std::shared_ptr<BtEngineClosestConvexResultCallback> ccb;
    std::shared_ptr<btConvexShape> sp = std::make_shared<btSphereShape>(16.0);

    bool                                        ceiling_climb = false;
    bool                                        walls_climb = false;
    int8_t                                      walls_climb_dir = 0;

    btVector3                                   floor_normale = {0,0,1};
    btVector3                                   floor_point = {0,0,0};
    bool                                        floor_hit = false;
    const btCollisionObject                    *floor_obj = nullptr;

    btVector3                                   ceiling_normale = {0,0,-1};
    btVector3                                   ceiling_point = {0,0,0};
    bool                                        ceiling_hit = false;
    const btCollisionObject                    *ceiling_obj = nullptr;

    btScalar                                    transition_level;
    bool                                        water = false;
    int                                         quicksand = 0;
};

struct CharacterCommand
{
    btVector3 rot = {0,0,0};
    std::array<int8_t,3> move{{0,0,0}};

    bool        roll = false;
    bool        jump = false;
    bool        crouch = false;
    bool        shift = false;
    bool        action = false;
    bool        ready_weapon = false;
    bool        sprint = false;
};

struct CharacterResponse
{
    bool killed = false;

    union
    {
        int8_t      vertical_collide_raw = 0;
        struct
        {
            bool ceiling_collision : 1;
            bool floor_collision : 1;
        };
    };

    bool horizontal_collision = false;

    SlideType   slide = SlideType::None;
};

struct CharacterParam
{
    std::array<float,PARAM_SENTINEL> param{{}};
    std::array<float,PARAM_SENTINEL> maximum{{}};

    CharacterParam()
    {
        param.fill(0);
        maximum.fill(0);
    }
};

struct CharacterStats
{
    float       distance;
    uint32_t    secrets_level;         // Level amount of secrets.
    uint32_t    secrets_game;          // Overall amount of secrets.
    uint32_t    ammo_used;
    uint32_t    hits;
    uint32_t    kills;
    uint32_t    medipacks_used;
    uint32_t    saves_used;
};

struct InventoryNode
{
    uint32_t                    id;
    int32_t                     count;
    uint32_t                    max_count;
};


struct Hair;
struct SSAnimation;

enum class WeaponState {
    Hide,
    HideToReady,
    Idle,
    IdleToFire,
    Fire,
    FireToIdle,
    IdleToHide
};

struct Character : public Entity
{
    CharacterCommand   m_command;                    // character control commands
    CharacterResponse  m_response;                   // character response info (collides, slide, next steps, drops, e.t.c.)

    std::list<InventoryNode> m_inventory;
    CharacterParam     m_parameters{};
    CharacterStats     m_statistics;

    std::vector<std::shared_ptr<Hair>> m_hairs{};

    int                          m_currentWeapon = 0;
    WeaponState m_weaponCurrentState = WeaponState::Hide;

    int (Character::*state_func)(SSAnimation *ssAnim) = nullptr;

    int8_t                       m_camFollowCenter = 0;
    btScalar                     m_minStepUpHeight = DEFAULT_MIN_STEP_UP_HEIGHT;
    btScalar                     m_maxStepUpHeight = DEFAULT_MAX_STEP_UP_HEIGHT;
    btScalar                     m_maxClimbHeight = DEFAULT_CLIMB_UP_HEIGHT;
    btScalar                     m_fallDownHeight = DEFAULT_FALL_DOWN_HEIGHT;
    btScalar                     m_criticalSlantZComponent = DEFAULT_CRITICAL_SLANT_Z_COMPONENT;
    btScalar                     m_criticalWallComponent = DEFAULT_CRITICAL_WALL_COMPONENT;

    btScalar                     m_forwardSize = 48;           // offset for climbing calculation
    btScalar                     m_height = CHARACTER_BASE_HEIGHT;                 // base character height
    btScalar                     m_wadeDepth = DEFAULT_CHARACTER_WADE_DEPTH;             // water depth that enable wade walk
    btScalar                     m_swimDepth = DEFAULT_CHARACTER_SWIM_DEPTH;             // depth offset for starting to swim

    std::unique_ptr<btSphereShape> m_sphere{ new btSphereShape(CHARACTER_BASE_RADIUS) };                 // needs to height calculation
    btScalar      m_climbR = DEFAULT_CHARACTER_CLIMB_R;                // climbing sensor radius
    btSphereShape m_climbSensor{DEFAULT_CHARACTER_CLIMB_R};

    HeightInfo         m_heightInfo{};
    ClimbInfo          m_climb{};

    Entity* m_traversedObject = nullptr;

    std::shared_ptr<BtEngineClosestRayResultCallback> m_rayCb;
    std::shared_ptr<BtEngineClosestConvexResultCallback> m_convexCb;

    Character(uint32_t id);
    ~Character();

    int checkNextPenetration(const btVector3& move);

    void doWeaponFrame(btScalar time);

    void fixPenetrations(const btVector3* move) override;
    btVector3 getRoomPos() const override
    {
        btVector3 pos = m_transform * m_bf.bone_tags.front().full_transform.getOrigin();
        pos[0] = m_transform.getOrigin()[0];
        pos[1] = m_transform.getOrigin()[1];
        return pos;
    }
    void transferToRoom(Room* /*room*/) override {
    }
    void updateHair() override;
    void frameImpl(btScalar time, int16_t frame, int state) override;
    void processSectorImpl() override;
    void jump(btScalar vert, btScalar v_horizontal) override;
    void kill() override {
        m_response.killed = true;
    }
    virtual Substance getSubstanceState() const override;
    void updateTransform() override {
        ghostUpdate();
        Entity::updateTransform();
    }
    void updateGhostRigidBody() override;
    virtual std::shared_ptr<BtEngineClosestConvexResultCallback> callbackForCamera() const override {
        return m_convexCb;
    }
    btVector3 camPosForFollowing(btScalar dz) override;

    int32_t addItem(uint32_t item_id, int32_t count);       // returns items count after in the function's end
    int32_t removeItem(uint32_t item_id, int32_t count);    // returns items count after in the function's end
    int32_t removeAllItems();
    int32_t getItemsCount(uint32_t item_id);                // returns items count

    static void getHeightInfo(const btVector3& pos, HeightInfo *fc, btScalar v_offset = 0.0);
    NextStepInfo checkNextStep(const btVector3 &offset, HeightInfo *nfc);
    bool hasStopSlant(const HeightInfo &next_fc);
    ClimbInfo checkClimbability(const btVector3 &offset, HeightInfo *nfc, btScalar test_height);
    ClimbInfo checkWallsClimbability();

    void updateCurrentHeight();
    void updatePlatformPreStep() override;
    void updatePlatformPostStep();

    void lean(btScalar max_lean);
    btScalar inertiaLinear(btScalar max_speed, btScalar accel, bool command);
    btScalar inertiaAngular(btScalar max_angle, btScalar accel, uint8_t axis);

    int moveOnFloor();
    int freeFalling();
    int monkeyClimbing();
    int wallsClimbing();
    int climbing();
    int moveUnderWater();
    int moveOnWater();

    int findTraverse();
    int checkTraverse(const Entity &obj);

    static constexpr const int TraverseNone = 0x00;
    static constexpr const int TraverseForward = 0x01;
    static constexpr const int TraverseBackward = 0x02;

    void applyCommands();
    void updateParams();

    float getParam(int parameter);
    int   setParam(int parameter, float value);
    int   changeParam(int parameter, float value);
    int   setParamMaximum(int parameter, float max_value);

    int   setWeaponModel(int weapon_model, int armed);

    int stateControlLara(SSAnimation* ss_anim);

private:
    // Anim frame callbacks
    void stopTraverse(SSAnimation* ss_anim, int state);
    void setOnFloor(SSAnimation* ss_anim, int state);
    void turnFast(SSAnimation* ss_anim, int state);
    void setOnFloorAfterClimb(SSAnimation* ss_anim, int state);
    void setUnderwater(SSAnimation* ss_anim, int state);
    void setFreeFalling(SSAnimation* ss_anim, int state);
    void setCmdSlide(SSAnimation* ss_anim, int state);
    void correctDivingAngle(SSAnimation* ss_anim, int state);
    void toOnWater(SSAnimation* ss_anim, int state);
    void climbOutOfWater(SSAnimation* ss_anim, int state);
    void toEdgeClimb(SSAnimation* ss_anim, int state);
    void toMonkeySwing(SSAnimation* ss_anim, int state);
    void crawlToClimb(SSAnimation* ss_anim, int state);

    // State control subfunctions
    void stateLaraStop(SSAnimation* ss_anim, HeightInfo& next_fc, bool low_vertical_space);
    void stateLaraJumpPrepare(SSAnimation* ss_anim);
    void stateLaraJumpBack(SSAnimation* ss_anim);
    void stateLaraJumpLeftRight(SSAnimation* ss_anim);
    void stateLaraRunBack();
    void stateLaraTurnSlow(SSAnimation* ss_anim, bool last_frame);
    void stateLaraTurnFast(SSAnimation* ss_anim);
    void stateLaraRunForward(SSAnimation* ss_anim, HeightInfo& next_fc, bool low_vertical_space);
    void stateLaraSprint(SSAnimation* ss_anim, HeightInfo& next_fc, bool low_vertical_space);
    void stateLaraWalkForward(SSAnimation* ss_anim, HeightInfo& next_fc, bool low_vertical_space);
    void stateLaraWadeForward(SSAnimation* ss_anim);
    void stateLaraWalkBack(SSAnimation* ss_anim, HeightInfo& next_fc);
    void stateLaraWalkLeftRight(SSAnimation* ss_anim, HeightInfo& next_fc);
    void stateLaraSlideBack(SSAnimation* ss_anim);
    void stateLaraSlideForward(SSAnimation* ss_anim);
    void stateLaraPushableGrab(SSAnimation* ss_anim);
    void stateLaraPushablePush(SSAnimation* ss_anim);
    void stateLaraPushablePull(SSAnimation* ss_anim);
    void stateLaraRollBackward(bool low_vertical_space);
    void stateLaraJumpUp(SSAnimation* ss_anim, HeightInfo &next_fc);
    void stateLaraReach(SSAnimation* ss_anim, HeightInfo &next_fc);
    void stateLaraFixClimbEnd(SSAnimation* ss_anim);
    void stateLaraHang(SSAnimation* ss_anim, HeightInfo &next_fc);
    void stateLaraLadderIdle(SSAnimation* ss_anim, HeightInfo &next_fc);
    void stateLaraLadderLeftRight(SSAnimation* ss_anim);
    void stateLaraLadderUp(SSAnimation* ss_anim, HeightInfo &next_fc);
    void stateLaraLadderDown(SSAnimation* ss_anim);
    void stateLaraShimmyLeftRight(SSAnimation* ss_anim, HeightInfo &next_fc);
    void stateLaraOnWaterExit(SSAnimation* ss_anim);
    void stateLaraJumpForwardFallBackward(SSAnimation* ss_anim);
    void stateLaraUnderwaterDiving(SSAnimation* ss_anim);
    void stateLaraFreefall(SSAnimation* ss_anim);
    void stateLaraSwandiveBegin(SSAnimation* ss_anim);
    void stateLaraSwandiveEnd(SSAnimation* ss_anim);
    void stateLaraUnderwaterStop(SSAnimation* ss_anim);
    void stateLaraUnderwaterDeath();
    void stateLaraUnderwaterForward(SSAnimation* ss_anim);
    void stateLaraOnwaterStop(SSAnimation* ss_anim);
    void stateLaraOnwaterBack(SSAnimation* ss_anim);
    void stateLaraOnwaterForward(SSAnimation* ss_anim, HeightInfo &next_fc, bool low_vertical_space);
    void stateLaraOnwaterLeftRight(SSAnimation* ss_anim);
    void stateLaraUnderwaterInertia(SSAnimation* ss_anim);
    void stateLaraCrouchIdle(SSAnimation* ss_anim, HeightInfo &next_fc, bool low_vertical_space);
    void stateLaraSprintCrouchRoll(SSAnimation* ss_anim);
    void stateLaraCrawlIdle(SSAnimation* ss_anim, HeightInfo &next_fc);
    void stateLaraCrawlToClimb(SSAnimation* ss_anim);
    void stateLaraCrawlForward(SSAnimation* ss_anim, HeightInfo &next_fc);
    void stateLaraCrawlBack(SSAnimation* ss_anim, HeightInfo &next_fc);
    void stateLaraCrawlTurnLeftRight(SSAnimation* ss_anim);
    void stateLaraCrouchTurnLeftRight(SSAnimation* ss_anim);
    void stateLaraMonkeyswingIdle(SSAnimation* ss_anim);
    void stateLaraMonkeyswingTurnLeftRight(SSAnimation* ss_anim);
    void stateLaraMonkeyswingForward(SSAnimation* ss_anim);
    void stateLaraMonkeyswingLeftRight(SSAnimation* ss_anim);
};

bool IsCharacter(std::shared_ptr<Entity> ent);
int Sector_AllowTraverse(RoomSector *rs, btScalar floor, const std::shared_ptr<EngineContainer> &cont);

