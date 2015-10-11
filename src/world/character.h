#pragma once

#include "entity.h"
#include "hair.h"
#include "statecontroller.h"

#include <list>

namespace engine
{
class BtEngineClosestRayResultCallback;
class BtEngineClosestConvexResultCallback;
}

struct InventoryNode;

namespace world
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

// Speed limits
constexpr float FREE_FALL_SPEED_1        = 2000.0f;
constexpr float FREE_FALL_SPEED_2        = 4500.0f;
constexpr float FREE_FALL_SPEED_MAXSAFE  = 5500.0f;
constexpr float FREE_FALL_SPEED_CRITICAL = 7500.0f;
constexpr float FREE_FALL_SPEED_MAXIMUM  = 7800.0f;

constexpr float MAX_SPEED_UNDERWATER     = 64.0f;
constexpr float MAX_SPEED_ONWATER        = 24.0f;
constexpr float MAX_SPEED_QUICKSAND      = 5.0f;

constexpr float ROT_SPEED_UNDERWATER     = 2.0f;
constexpr float ROT_SPEED_ONWATER        = 3.0f;
constexpr float ROT_SPEED_LAND           = 4.5f;
constexpr float ROT_SPEED_FREEFALL       = 0.5f;
constexpr float ROT_SPEED_MONKEYSWING    = 3.5f;

constexpr float INERTIA_SPEED_UNDERWATER = 1.0f;
constexpr float INERTIA_SPEED_ONWATER    = 1.5f;

//! If less than this much of Lara is looking out of the water, she goes from wading to swimming.
//! @fixme Guess
constexpr float DEFAULT_CHARACTER_SWIM_DEPTH = 100.0f;

struct CharacterCommand
{
    glm::vec3 rot = { 0,0,0 };
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

    glm::vec3                                   floor_normale = { 0,0,1 };
    glm::vec3                                   floor_point = { 0,0,0 };
    bool                                        floor_hit = false;
    const btCollisionObject                    *floor_obj = nullptr;

    glm::vec3                                   ceiling_normale = { 0,0,-1 };
    glm::vec3                                   ceiling_point = { 0,0,0 };
    bool                                        ceiling_hit = false;
    const btCollisionObject                    *ceiling_obj = nullptr;

    glm::float_t                                transition_level = 0;
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

    glm::vec3 point;
    glm::vec3 n;
    glm::vec3 right;
    glm::vec3 up;
    glm::float_t                       floor_limit;
    glm::float_t                       ceiling_limit;
    glm::float_t                       next_z_space = 0;

    ClimbType                      wall_hit = ClimbType::None;
    bool                           edge_hit = false;
    glm::vec3                      edge_point;
    glm::vec3                      edge_normale;
    glm::vec3                      edge_tan_xy;
    glm::float_t                       edge_z_ang;
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

    int8_t                       m_camFollowCenter = 0;
    glm::float_t                     m_minStepUpHeight = DEFAULT_MIN_STEP_UP_HEIGHT;
    glm::float_t                     m_maxStepUpHeight = DEFAULT_MAX_STEP_UP_HEIGHT;
    glm::float_t                     m_maxClimbHeight = DEFAULT_CLIMB_UP_HEIGHT;
    glm::float_t                     m_fallDownHeight = DEFAULT_FALL_DOWN_HEIGHT;
    glm::float_t                     m_criticalSlantZComponent = DEFAULT_CRITICAL_SLANT_Z_COMPONENT;
    glm::float_t                     m_criticalWallComponent = DEFAULT_CRITICAL_WALL_COMPONENT;

    glm::float_t                     m_climbR = DEFAULT_CHARACTER_CLIMB_R;                // climbing sensor radius
    glm::float_t                     m_forwardSize = 48;           // offset for climbing calculation
    glm::float_t                     m_height = CHARACTER_BASE_HEIGHT;                 // base character height
    glm::float_t                     m_wadeDepth = DEFAULT_CHARACTER_WADE_DEPTH;             // water depth that enable wade walk
    glm::float_t                     m_swimDepth = DEFAULT_CHARACTER_SWIM_DEPTH;             // depth offset for starting to swim

    std::unique_ptr<btSphereShape> m_sphere{ new btSphereShape(CHARACTER_BASE_RADIUS) };                 // needs to height calculation
    std::unique_ptr<btSphereShape> m_climbSensor;

    HeightInfo         m_heightInfo{};
    ClimbInfo          m_climb{};

    Entity* m_traversedObject = nullptr;

    std::shared_ptr<engine::BtEngineClosestRayResultCallback> m_rayCb;
    std::shared_ptr<engine::BtEngineClosestConvexResultCallback> m_convexCb;

    Character(uint32_t id);
    ~Character();

    int checkNextPenetration(const glm::vec3& move);

    void doWeaponFrame(util::Duration time);

    void fixPenetrations(const glm::vec3* move) override;
    glm::vec3 getRoomPos() const override
    {
        glm::vec4 pos = m_transform * m_bf.getRootTransform()[3];
        pos[0] = m_transform[3][0];
        pos[1] = m_transform[3][1];
        return glm::vec3(pos);
    }
    void transferToRoom(Room* /*room*/) override
    {
    }

    void updateHair() override;
    void frame(util::Duration time) override;

    void processSectorImpl() override;
    void jump(glm::float_t vert, glm::float_t v_horizontal) override;
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
    glm::vec3 camPosForFollowing(glm::float_t dz) override;

    int32_t addItem(uint32_t item_id, int32_t count);       // returns items count after in the function's end
    int32_t removeItem(uint32_t item_id, int32_t count);    // returns items count after in the function's end
    int32_t removeAllItems();
    int32_t getItemsCount(uint32_t item_id);                // returns items count

    static void getHeightInfo(const glm::vec3& pos, HeightInfo *fc, glm::float_t v_offset = 0.0);
    StepType checkNextStep(const glm::vec3 &offset, HeightInfo *nfc) const;
    bool hasStopSlant(const HeightInfo &next_fc);
    ClimbInfo checkClimbability(const glm::vec3& offset, HeightInfo *nfc, glm::float_t test_height);
    ClimbInfo checkWallsClimbability();

    void updateCurrentHeight();
    void updatePlatformPreStep() override;
    void updatePlatformPostStep();

    void lean(glm::float_t max_lean);
    glm::float_t inertiaLinear(glm::float_t max_speed, glm::float_t accel, bool command);
    glm::float_t inertiaAngular(glm::float_t max_angle, glm::float_t accel, uint8_t axis);

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

private:
    StateController m_stateController;
};

} // namespace world
