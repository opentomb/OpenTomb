#pragma once

#include <map>

// ====== LARA'S STATES ======

namespace world
{
enum class LaraState
{
    WALK_FORWARD = 0,
    RUN_FORWARD = 1,
    STOP = 2,
    JUMP_FORWARD = 3,
    POSE = 4,                // Derived from leaked TOMB.MAP
    RUN_BACK = 5,
    TURN_RIGHT_SLOW = 6,
    TURN_LEFT_SLOW = 7,
    DEATH = 8,
    FREEFALL = 9,
    HANG = 10,
    REACH = 11,
//    UNUSED2 12
    UNDERWATER_STOP = 13,
    GRAB_TO_FALL = 14,
    JUMP_PREPARE = 15,
    WALK_BACK = 16,
    UNDERWATER_FORWARD = 17,
    UNDERWATER_INERTIA = 18,
    CLIMBING = 19,
    TURN_FAST = 20,
    WALK_RIGHT = 21,
    WALK_LEFT = 22,
    ROLL_BACKWARD = 23,
    SLIDE_FORWARD = 24,
    JUMP_BACK = 25,
    JUMP_LEFT = 26,
    JUMP_RIGHT = 27,
    JUMP_UP = 28,
    FALL_BACKWARD = 29,
    SHIMMY_LEFT = 30,
    SHIMMY_RIGHT = 31,
    SLIDE_BACK = 32,
    ONWATER_STOP = 33,
    ONWATER_FORWARD = 34,
    UNDERWATER_DIVING = 35,
    PUSHABLE_PUSH = 36,
    PUSHABLE_PULL = 37,
    PUSHABLE_GRAB = 38,
    PICKUP = 39,
    SWITCH_DOWN = 40,
    SWITCH_UP = 41,
    INSERT_KEY = 42,
    INSERT_PUZZLE = 43,
    WATER_DEATH = 44,
    ROLL_FORWARD = 45,
    BOULDER_DEATH = 46,
    ONWATER_BACK = 47,
    ONWATER_LEFT = 48,
    ONWATER_RIGHT = 49,
    USE_MIDAS = 50,          //  Derived from leaked TOMB.MAP
    DIE_MIDAS = 51,          //  Derived from leaked TOMB.MAP
    SWANDIVE_BEGIN = 52,
    SWANDIVE_END = 53,
    HANDSTAND = 54,
    ONWATER_EXIT = 55,
    LADDER_IDLE = 56,
    LADDER_UP = 57,
    LADDER_LEFT = 58,
//    UNUSED5 59
    LADDER_RIGHT = 60,
    LADDER_DOWN = 61,
//    UNUSED6 62
//    UNUSED7 63
//    UNUSED8 64
    WADE_FORWARD = 65,
    UNDERWATER_TURNAROUND = 66,
    FLARE_PICKUP = 67,
    JUMP_ROLL = 68,
//    UNUSED10 69
    ZIPLINE_RIDE = 70,
    CROUCH_IDLE = 71,
    CROUCH_ROLL = 72,
    SPRINT = 73,
    SPRINT_ROLL = 74,
    MONKEYSWING_IDLE = 75,
    MONKEYSWING_FORWARD = 76,
    MONKEYSWING_LEFT = 77,
    MONKEYSWING_RIGHT = 78,
    MONKEYSWING_TURNAROUND = 79,
    CRAWL_IDLE = 80,
    CRAWL_FORWARD = 81,
    MONKEYSWING_TURN_LEFT = 82,
    MONKEYSWING_TURN_RIGHT = 83,
    CRAWL_TURN_LEFT = 84,
    CRAWL_TURN_RIGHT = 85,
    CRAWL_BACK = 86,
    CLIMB_TO_CRAWL = 87,
    CRAWL_TO_CLIMB = 88,
    MISC_CONTROL = 89,
    ROPE_TURN_LEFT = 90,
    ROPE_TURN_RIGHT = 91,
    GIANT_BUTTON_PUSH = 92,
    TRAPDOOR_FLOOR_OPEN = 93,
//    UNUSED11 94
    ROUND_HANDLE = 95,
    COGWHEEL = 96,
    LEVERSWITCH_PUSH = 97,
    HOLE = 98,
    POLE_IDLE = 99,
    POLE_UP = 100,
    POLE_DOWN = 101,
    POLE_TURN_LEFT = 102,
    POLE_TURN_RIGHT = 103,
    PULLEY = 104,
    CROUCH_TURN_LEFT = 105,
    CROUCH_TURN_RIGHT = 106,
    CLIMB_CORNER_LEFT_OUTER = 107,
    CLIMB_CORNER_RIGHT_OUTER = 108,
    CLIMB_CORNER_LEFT_INNER = 109,
    CLIMB_CORNER_RIGHT_INNER = 110,
    ROPE_IDLE = 111,
    ROPE_CLIMB_UP = 112,
    ROPE_CLIMB_DOWN = 113,
    ROPE_SWING = 114,
    LADDER_TO_HANDS = 115,
    POSITION_CORRECTOR = 116,
    DOUBLEDOORS_PUSH = 117,
    DOZY = 118,
    TIGHTROPE_IDLE = 119,
    TIGHTROPE_TURNAROUND = 120,
    TIGHTROPE_FORWARD = 121,
    TIGHTROPE_BALANCING_LEFT = 122,
    TIGHTROPE_BALANCING_RIGHT = 123,
    TIGHTROPE_ENTER = 124,
    TIGHTROPE_EXIT = 125,
    DOVESWITCH = 126,
    TIGHTROPE_RESTORE_BALANCE = 127,
    BARS_SWING = 128,
    BARS_JUMP = 129,
//    UNUSED12 130
    RADIO_LISTENING = 131,
    RADIO_OFF = 132,
//    UNUSED13 133
//    UNUSED14 134
//    UNUSED15 135
//    UNUSED16 136
    PICKUP_FROM_CHEST = 137
};

class Character;
struct HeightInfo;

constexpr float LaraHangVerticalEpsilon = 64.0f;

// State control is based on original TR system, where several animations may
// belong to the same state.
struct StateController
{
public:
    explicit StateController(Character* c);

    void handle(LaraState state);

private:
    using Handler = void (StateController::*)();

    void on(LaraState state, Handler handler);

    void stop();

    void jumpPrepare();

    void jumpBack();

    void jumpLeft();

    void jumpRight();

    void runBack();

    void turnSlow();

    void turnFast();

    void runForward();

    void sprint();

    void walkForward();

    void wadeForward();

    void walkBack();

    void walkLeft();

    void walkRight();

    void slideBack();

    void slideForward();

    void pushableGrab();

    void pushablePush();

    void pushablePull();

    void rollForward();

    void rollBackward();

    void jumpUp();

    void reach();

    void hang();

    void ladderIdle();

    void ladderLeft();

    void ladderRight();

    void ladderUp();

    void ladderDown();

    void shimmyLeft();

    void shimmyRight();

    void onwaterExit();

    void jumpForwardFallBackward();

    void underwaterDiving();

    void freefall();

    void swandiveBegin();

    void swandiveEnd();

    void underwaterStop();

    void waterDeath();

    void underwaterForward();

    void underwaterInertia();

    void onwaterStop();

    void onwaterForward();

    void onwaterBack();

    void onwaterLeft();

    void onwaterRight();

    void crouchIdle();

    void roll();

    void crawlIdle();

    void crawlToClimb();

    void crawlForward();

    void crawlBack();

    void crawlTurnLeft();

    void crawlTurnRight();

    void crouchTurnLeftRight();

    void monkeyswingIdle();

    void monkeyswingTurnLeft();

    void monkeyswingTurnRight();

    void monkeyswingForward();

    void monkeyswingLeft();

    void monkeyswingRight();

    void tightropeEnter();

    void tightropeExit();

    void tightropeIdle();

    void tightropeForward();

    void tightropeBalancingLeft();

    void tightropeBalancingRight();

    void fixEndOfClimbOn();

    // ---------------------------------------

    HeightInfo initHeightInfo() const;

    bool isLowVerticalSpace() const;

    bool isLastFrame() const;

    void setNextState(LaraState state);

    Character* const m_character;
    std::map<LaraState, Handler> m_handlers;
};
} // namespace world
