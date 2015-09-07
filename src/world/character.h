#pragma once

#include "character_controller.h"
#include "entity.h"
#include "hair.h"

#include <list>

namespace engine
{
class BtEngineClosestRayResultCallback;
class BtEngineClosestConvexResultCallback;
}

struct InventoryNode;

namespace world
{
namespace
{
// Lara's character behavior constants
constexpr int   DEFAULT_MAX_MOVE_ITERATIONS             = 3;                              //!< @fixme magic
constexpr float DEFAULT_MIN_STEP_UP_HEIGHT              = 128.0f;                         //!< @fixme check original
constexpr float DEFAULT_MAX_STEP_UP_HEIGHT              = 256.0f + 32.0f;                 //!< @fixme check original
constexpr float DEFAULT_FALL_DOWN_HEIGHT                = 320.0f;                         //!< @fixme check original
constexpr float DEFAULT_CLIMB_UP_HEIGHT                 = 1920.0f;                        //!< @fixme check original
constexpr float DEFAULT_CRITICAL_SLANT_Z_COMPONENT      = 0.810f;                         //!< @fixme cos(alpha = 30 deg)
constexpr float DEFAULT_CRITICAL_WALL_COMPONENT         = -0.707f;                        //!< @fixme cos(alpha = 45 deg)
constexpr float DEFAULT_CHARACTER_SLIDE_SPEED_MULT      = 75.0f;                          //!< @fixme magic - not like in original
constexpr float DEFAULT_CHARACTER_CLIMB_R               = 32.0f;
constexpr float DEFAULT_CHARACTER_WADE_DEPTH            = 256.0f;

constexpr float CHARACTER_BOX_HALF_SIZE = 128.0f;
constexpr float CHARACTER_BASE_RADIUS = 128.0f;
constexpr float CHARACTER_BASE_HEIGHT = 512.0f;

//! If less than this much of Lara is looking out of the water, she goes from wading to swimming.
//! @fixme Guess
constexpr float DEFAULT_CHARACTER_SWIM_DEPTH = 100.0f;
} // anonymous namespace

struct CharacterCommand
{
    btVector3 rot = { 0,0,0 };
    std::array<int8_t, 3> move{ { 0,0,0 } };

    bool        roll = false;
    bool        jump = false;
    bool        crouch = false;
    bool        shift = false;
    bool        action = false;
    bool        ready_weapon = false;
    bool        sprint = false;
};

enum class SlideType
{
    None,
    Back,
    Front
};

enum class LeanType
{
    None,
    Left,
    Right
};

struct CharacterResponse
{
    bool        killed = false;
    int8_t      vertical_collide = 0;
    int8_t      horizontal_collide = 0;
    SlideType   slide = SlideType::None;
    LeanType    lean = LeanType::None;
};

enum class QuicksandPosition
{
    None,
    Sinking,
    Drowning
};

struct HeightInfo
{
    HeightInfo();

    std::shared_ptr<engine::BtEngineClosestRayResultCallback> cb;
    std::shared_ptr<engine::BtEngineClosestConvexResultCallback> ccb;
    std::shared_ptr<btConvexShape> sp = std::make_shared<btSphereShape>(16.0);

    bool                                        ceiling_climb = false;
    bool                                        walls_climb = false;
    int8_t                                      walls_climb_dir = 0;

    btVector3                                   floor_normale = { 0,0,1 };
    btVector3                                   floor_point = { 0,0,0 };
    bool                                        floor_hit = false;
    const btCollisionObject                    *floor_obj = nullptr;

    btVector3                                   ceiling_normale = { 0,0,-1 };
    btVector3                                   ceiling_point = { 0,0,0 };
    bool                                        ceiling_hit = false;
    const btCollisionObject                    *ceiling_obj = nullptr;

    btScalar                                    transition_level;
    bool                                        water = false;
    QuicksandPosition                           quicksand = QuicksandPosition::None;
};

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

struct CharacterParam
{
    std::array<float, PARAM_SENTINEL> param{ {} };
    std::array<float, PARAM_SENTINEL> maximum{ {} };

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

/**
 * Next step height information
 */
enum class StepType
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

inline constexpr bool isLittleStep(StepType type)
{
    return type >= StepType::DownLittle && type <= StepType::UpLittle;
}

//! Check if the step type doesn't require a drop or a climb
inline constexpr bool isWakableStep(StepType type)
{
    return type >= StepType::DownBig && type <= StepType::UpBig;
}

enum class ClimbType
{
    None,
    HandsOnly,
    FullBody
};

struct ClimbInfo
{
    StepType                       height_info = StepType::Horizontal;
    bool                           can_hang = false;

    btVector3 point;
    btVector3 n;
    btVector3 right;
    btVector3 up;
    btScalar                       floor_limit;
    btScalar                       ceiling_limit;
    btScalar                       next_z_space = 0;

    ClimbType                      wall_hit = ClimbType::None;
    bool                           edge_hit = false;
    btVector3                      edge_point;
    btVector3                      edge_normale;
    btVector3                      edge_tan_xy;
    btScalar                       edge_z_ang;
    btCollisionObject             *edge_obj = nullptr;
};

enum class WeaponState
{
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

    void state_func();

    int8_t                       m_camFollowCenter = 0;
    btScalar                     m_minStepUpHeight = DEFAULT_MIN_STEP_UP_HEIGHT;
    btScalar                     m_maxStepUpHeight = DEFAULT_MAX_STEP_UP_HEIGHT;
    btScalar                     m_maxClimbHeight = DEFAULT_CLIMB_UP_HEIGHT;
    btScalar                     m_fallDownHeight = DEFAULT_FALL_DOWN_HEIGHT;
    btScalar                     m_criticalSlantZComponent = DEFAULT_CRITICAL_SLANT_Z_COMPONENT;
    btScalar                     m_criticalWallComponent = DEFAULT_CRITICAL_WALL_COMPONENT;

    btScalar                     m_climbR = DEFAULT_CHARACTER_CLIMB_R;                // climbing sensor radius
    btScalar                     m_forwardSize = 48;           // offset for climbing calculation
    btScalar                     m_height = CHARACTER_BASE_HEIGHT;                 // base character height
    btScalar                     m_wadeDepth = DEFAULT_CHARACTER_WADE_DEPTH;             // water depth that enable wade walk
    btScalar                     m_swimDepth = DEFAULT_CHARACTER_SWIM_DEPTH;             // depth offset for starting to swim

    std::unique_ptr<btSphereShape> m_sphere{ new btSphereShape(CHARACTER_BASE_RADIUS) };                 // needs to height calculation
    std::unique_ptr<btSphereShape> m_climbSensor;

    HeightInfo         m_heightInfo{};
    ClimbInfo          m_climb{};

    Entity* m_traversedObject = nullptr;

    std::shared_ptr<engine::BtEngineClosestRayResultCallback> m_rayCb;
    std::shared_ptr<engine::BtEngineClosestConvexResultCallback> m_convexCb;

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
    void transferToRoom(Room* /*room*/) override
    {
    }

    void updateHair() override;
    void frame(btScalar time) override;

    void processSectorImpl() override;
    void jump(btScalar vert, btScalar v_horizontal) override;
    void kill() override
    {
        m_response.killed = true;
    }
    virtual Substance getSubstanceState() const override;
    void updateTransform() override
    {
        ghostUpdate();
        Entity::updateTransform();
    }
    void updateGhostRigidBody() override;
    virtual std::shared_ptr<engine::BtEngineClosestConvexResultCallback> callbackForCamera() const override
    {
        return m_convexCb;
    }
    btVector3 camPosForFollowing(btScalar dz) override;

    int32_t addItem(uint32_t item_id, int32_t count);       // returns items count after in the function's end
    int32_t removeItem(uint32_t item_id, int32_t count);    // returns items count after in the function's end
    int32_t removeAllItems();
    int32_t getItemsCount(uint32_t item_id);                // returns items count

    static void getHeightInfo(const btVector3& pos, HeightInfo *fc, btScalar v_offset = 0.0);
    StepType checkNextStep(const btVector3 &offset, HeightInfo *nfc) const;
    bool hasStopSlant(const HeightInfo &next_fc);
    ClimbInfo checkClimbability(const btVector3& offset, HeightInfo *nfc, btScalar test_height);
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
};

} // namespace world
