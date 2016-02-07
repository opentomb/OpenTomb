#pragma once

#include "entity.h"
#include "hair.h"
#include "inventory.h"
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
constexpr int DEFAULT_MAX_MOVE_ITERATIONS = 3;               //!< @fixme magic
constexpr float DEFAULT_MIN_STEP_UP_HEIGHT = 128.0f;         //!< @fixme check original
constexpr float DEFAULT_MAX_STEP_UP_HEIGHT = 256.0f + 32.0f; //!< @fixme check original
constexpr float DEFAULT_FALL_DOWN_HEIGHT = 320.0f;           //!< @fixme check original
constexpr float DEFAULT_CLIMB_UP_HEIGHT = 1920.0f;           //!< @fixme check original
constexpr float DEFAULT_CRITICAL_SLANT_Z_COMPONENT = 0.810f; //!< @fixme cos(alpha = 30 deg)
constexpr float DEFAULT_CRITICAL_WALL_COMPONENT = -0.707f;   //!< @fixme cos(alpha = 45 deg)
constexpr float DEFAULT_CHARACTER_SLIDE_SPEED_MULT = 75.0f;  //!< @fixme magic - not like in original
constexpr float DEFAULT_CHARACTER_CLIMB_R = 32.0f;
constexpr float DEFAULT_CHARACTER_WADE_DEPTH = 256.0f;

constexpr float CHARACTER_BOX_HALF_SIZE = 128.0f;
constexpr float CHARACTER_BASE_RADIUS = 128.0f;
constexpr float CHARACTER_BASE_HEIGHT = 512.0f;

// Speed limits
constexpr float FREE_FALL_SPEED_1 = 2000.0f;
constexpr float FREE_FALL_SPEED_2 = 4500.0f;
constexpr float FREE_FALL_SPEED_MAXSAFE = 5500.0f;
constexpr float FREE_FALL_SPEED_CRITICAL = 7500.0f;
constexpr float FREE_FALL_SPEED_MAXIMUM = 7800.0f;

constexpr float MAX_SPEED_UNDERWATER = 64.0f;
constexpr float MAX_SPEED_ONWATER = 24.0f;
constexpr float MAX_SPEED_QUICKSAND = 5.0f;

constexpr float ROT_SPEED_UNDERWATER = 2.0f;
constexpr float ROT_SPEED_ONWATER = 3.0f;
constexpr float ROT_SPEED_LAND = 4.5f;
constexpr float ROT_SPEED_FREEFALL = 0.5f;
constexpr float ROT_SPEED_MONKEYSWING = 3.5f;

constexpr float INERTIA_SPEED_UNDERWATER = 1.0f;
constexpr float INERTIA_SPEED_ONWATER = 1.5f;

//! If less than this much of Lara is looking out of the water, she goes from wading to swimming.
//! @fixme Guess
constexpr float DEFAULT_CHARACTER_SWIM_DEPTH = 100.0f;

enum class MovementStrafe
{
    Left,
    None,
    Right
};

enum class MovementVertical
{
    Up,
    None,
    Down
};

enum class MovementWalk
{
    Forward,
    None,
    Backward
};

struct Movement final
{
    MovementStrafe x = MovementStrafe::None;
    MovementVertical y = MovementVertical::None;
    MovementWalk z = MovementWalk::None;

    void setX(bool left, bool right) noexcept
    {
        if(left && !right)
            x = MovementStrafe::Left;
        else if(right && !left)
            x = MovementStrafe::Right;
        else
            x = MovementStrafe::None;
    }

    void setY(bool up, bool down) noexcept
    {
        if(up && !down)
            y = MovementVertical::Up;
        else if(down && !up)
            y = MovementVertical::Down;
        else
            y = MovementVertical::None;
    }

    void setZ(bool forward, bool back) noexcept
    {
        if(forward && !back)
            z = MovementWalk::Forward;
        else if(back && !forward)
            z = MovementWalk::Backward;
        else
            z = MovementWalk::None;
    }

    glm::float_t getDistanceX(glm::float_t dist) const noexcept
    {
        switch(x)
        {
            case MovementStrafe::Left:
                return -dist;
            case MovementStrafe::Right:
                return dist;
            default:
                return 0;
        }
    }

    glm::float_t getDistanceY(glm::float_t dist) const noexcept
    {
        switch(y)
        {
            case MovementVertical::Up:
                return dist;
            case MovementVertical::Down:
                return -dist;
            default:
                return 0;
        }
    }

    glm::float_t getDistanceZ(glm::float_t dist) const noexcept
    {
        switch(z)
        {
            case MovementWalk::Forward:
                return dist;
            case MovementWalk::Backward:
                return -dist;
            default:
                return 0;
        }
    }

    glm::vec3 getDistance(glm::float_t dist) const noexcept
    {
        return{
            getDistanceX(dist),
            getDistanceY(dist),
            getDistanceZ(dist)
        };
    }
};

struct CharacterCommand
{
    glm::vec3 rot = { 0, 0, 0 };
    Movement move;

    bool roll = false;
    bool jump = false;
    bool crouch = false;
    bool shift = false;
    bool action = false;
    bool ready_weapon = false;
    bool sprint = false;
};

struct CharacterResponse
{
    bool killed = false;
    int8_t vertical_collide = 0;
    int8_t horizontal_collide = 0;
    MovementWalk slide = MovementWalk::None;
    MovementStrafe lean = MovementStrafe::None;
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
    std::shared_ptr<btConvexShape> sp = std::make_shared<btSphereShape>(btScalar(16));

    bool ceiling_climb = false;
    bool walls_climb = false;
    int8_t walls_climb_dir = 0;

    struct HitObject final
    {
        bool hasHit = false;
        glm::vec3 hitNormal;
        glm::vec3 hitPoint = { 0, 0, 0 };
        const btCollisionObject* collisionObject = nullptr;

        explicit HitObject(glm::float_t normalZComponent) noexcept
            : hitNormal{ 0,0,normalZComponent }
        {
        }

        void assign(const btCollisionWorld::ClosestRayResultCallback& cb, const glm::vec3& from, const glm::vec3& to)
        {
            hasHit = cb.hasHit();
            if(hasHit)
            {
                hitNormal = util::convert(cb.m_hitNormalWorld);
                hitPoint = glm::mix(from, to, cb.m_closestHitFraction);
                collisionObject = cb.m_collisionObject;
            }
        }
    };

    HitObject floor{ 1 };
    HitObject ceiling{ -1 };

    glm::float_t transition_level = 0;
    bool water = false;
    QuicksandPosition quicksand = QuicksandPosition::None;
};

enum class CharParameterId
{
    PARAM_HEALTH,
    PARAM_AIR,
    PARAM_STAMINA,
    PARAM_WARMTH,
    PARAM_POISON,
    PARAM_EXTRA1,
    PARAM_EXTRA2,
    PARAM_EXTRA3,
    PARAM_EXTRA4
};

ENUM_TO_OSTREAM(CharParameterId)

struct CharParameter
{
    float value = 0;
    float maximum = std::numeric_limits<float>::max();
};

using CharacterParam = std::map<CharParameterId, CharParameter>;

struct CharacterStats
{
    float distance = 0;
    uint32_t secrets_level = 0; // Level amount of secrets.
    uint32_t secrets_game = 0;  // Overall amount of secrets.
    uint32_t ammo_used = 0;
    uint32_t hits = 0;
    uint32_t kills = 0;
    uint32_t medipacks_used = 0;
    uint32_t saves_used = 0;
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
    StepType height_info = StepType::Horizontal;
    bool can_hang = false;

    glm::vec3 point;
    glm::vec3 n;
    glm::vec3 right;
    glm::vec3 up;
    glm::float_t floor_limit;
    glm::float_t ceiling_limit;
    glm::float_t next_z_space = 0;

    ClimbType wall_hit = ClimbType::None;
    bool edge_hit = false;
    glm::vec3 edge_point;
    glm::vec3 edge_normale;
    glm::vec3 edge_tan_xy;
    glm::float_t edge_z_ang;
    btCollisionObject* edge_obj = nullptr;
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

class Character : public Entity
{
    friend struct StateController;
private:
    CharacterCommand m_command;   // character control commands
    CharacterResponse m_response; // character response info (collides, slide, next steps, drops, e.t.c.)

    InventoryManager m_inventory;
    CharacterParam m_parameters{};
    CharacterStats m_statistics;

    std::vector<std::shared_ptr<Hair>> m_hairs{};

    std::shared_ptr<animation::SkeletalModel> m_currentWeapon = nullptr;
    WeaponState m_currentWeaponState = WeaponState::Hide;

    int8_t m_camFollowCenter = 0;
    glm::float_t m_minStepUpHeight = DEFAULT_MIN_STEP_UP_HEIGHT;
    glm::float_t m_maxStepUpHeight = DEFAULT_MAX_STEP_UP_HEIGHT;
    glm::float_t m_maxClimbHeight = DEFAULT_CLIMB_UP_HEIGHT;
    glm::float_t m_fallDownHeight = DEFAULT_FALL_DOWN_HEIGHT;
    glm::float_t m_criticalSlantZComponent = DEFAULT_CRITICAL_SLANT_Z_COMPONENT;
    glm::float_t m_criticalWallComponent = DEFAULT_CRITICAL_WALL_COMPONENT;

    glm::float_t m_climbR = DEFAULT_CHARACTER_CLIMB_R;       // climbing sensor radius
    glm::float_t m_forwardSize = 48;                         // offset for climbing calculation
    glm::float_t m_height = CHARACTER_BASE_HEIGHT;           // base character height
    glm::float_t m_wadeDepth = DEFAULT_CHARACTER_WADE_DEPTH; // water depth that enable wade walk
    glm::float_t m_swimDepth = DEFAULT_CHARACTER_SWIM_DEPTH; // depth offset for starting to swim

    std::unique_ptr<btSphereShape> m_sphere{ new btSphereShape(CHARACTER_BASE_RADIUS) }; // needs to height calculation
    std::unique_ptr<btSphereShape> m_climbSensor;

    HeightInfo m_heightInfo{};
    ClimbInfo m_climb{};

    Entity* m_traversedObject = nullptr;

    std::shared_ptr<engine::BtEngineClosestRayResultCallback> m_rayCb;
    std::shared_ptr<engine::BtEngineClosestConvexResultCallback> m_convexCb;

public:
    explicit Character(ObjectId id, world::World* world);
    ~Character();

    InventoryManager& inventory() noexcept
    {
        return m_inventory;
    }

    WeaponState getCurrentWeaponState() const noexcept
    {
        return m_currentWeaponState;
    }

    const std::vector<std::shared_ptr<Hair>>& getHairs() const noexcept
    {
        return m_hairs;
    }

    bool removeHairs()
    {
        bool wasEmpty = m_hairs.empty();
        m_hairs.clear();
        return wasEmpty;
    }

    void resetStatistics()
    {
        m_statistics = CharacterStats();
    }

    const CharacterCommand& getCommand() const noexcept
    {
        return m_command;
    }

    const CharacterResponse& getResponse() const noexcept
    {
        return m_response;
    }

    CharacterResponse& response() noexcept
    {
        return m_response;
    }

    std::shared_ptr<animation::SkeletalModel> getCurrentWeapon() const noexcept
    {
        return m_currentWeapon;
    }

    void setCurrentWeapon(const std::shared_ptr<animation::SkeletalModel>& w) noexcept
    {
        m_currentWeapon = w;
    }

    bool addHair(HairSetup* setup)
    {
        m_hairs.emplace_back(std::make_shared<world::Hair>(getWorld()));

        if(!m_hairs.back()->create(setup, *this))
        {
            m_hairs.pop_back();
            return false;
        }

        return true;
    }

    void saveGame(std::ostream& f) const;

    void setHeight(glm::float_t height) noexcept
    {
        m_height = height;
    }

    void applyControls(engine::EngineControlState& controlState, const Movement& moveLogic);
    void applyJoystickMove(float dx, float dy)
    {
        m_command.rot[0] += glm::degrees(-2 * dx);
        m_command.rot[1] += glm::degrees(-2 * dy);
    }

    int checkNextPenetration(const glm::vec3& move);

    void doWeaponFrame(util::Duration time);

    void fixPenetrations(const glm::vec3* move) override;
    glm::vec3 getRoomPos() const override;
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

    size_t addItem(ObjectId item_id, size_t count);    // returns items count after in the function's end
    size_t removeItem(ObjectId item_id, size_t count); // returns items count after in the function's end
    void removeAllItems();
    size_t getItemsCount(ObjectId item_id); // returns items count

    void getHeightInfo(const glm::vec3& pos, HeightInfo* fc, glm::float_t v_offset = 0.0) const;
    StepType checkNextStep(const glm::vec3& offset, HeightInfo* nfc) const;
    bool hasStopSlant(const HeightInfo& next_fc);
    ClimbInfo checkClimbability(const glm::vec3& offset, HeightInfo* nfc, glm::float_t test_height);
    ClimbInfo checkWallsClimbability();

    void updateCurrentHeight();
    void updatePlatformPreStep() override;
    void updatePlatformPostStep();

    void lean(glm::float_t max_lean);
    glm::float_t inertiaLinear(glm::float_t max_speed, glm::float_t accel, bool command);
    glm::float_t inertiaAngular(glm::float_t max_angle, glm::float_t accel, uint8_t axis);

    void moveOnFloor();
    int freeFalling();
    int monkeyClimbing();
    int wallsClimbing();
    int climbing();
    int moveUnderWater();
    int moveOnWater();

    int findTraverse();
    int checkTraverse(const Entity& obj);

    static constexpr const int TraverseNone = 0x00;
    static constexpr const int TraverseForward = 0x01;
    static constexpr const int TraverseBackward = 0x02;

    void applyCommands();
    void updateParams();

    float getParam(CharParameterId parameter);
    bool setParam(CharParameterId parameter, float value);
    bool changeParam(CharParameterId parameter, float value);
    void setParamMaximum(CharParameterId parameter, float max_value);

    bool setWeaponModel(const std::shared_ptr<animation::SkeletalModel>& model, bool armed);

private:
    StateController m_stateController;
};
} // namespace world
