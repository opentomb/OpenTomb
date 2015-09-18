#pragma once

#include <map>

// ====== LARA'S STATES ======

#define TR_STATE_CURRENT (-1)
#define TR_STATE_LARA_WALK_FORWARD 0
#define TR_STATE_LARA_RUN_FORWARD 1
#define TR_STATE_LARA_STOP 2
#define TR_STATE_LARA_JUMP_FORWARD 3
#define TR_STATE_LARA_POSE 4                // Derived from leaked TOMB.MAP
#define TR_STATE_LARA_RUN_BACK 5
#define TR_STATE_LARA_TURN_RIGHT_SLOW 6
#define TR_STATE_LARA_TURN_LEFT_SLOW 7
#define TR_STATE_LARA_DEATH 8
#define TR_STATE_LARA_FREEFALL 9
#define TR_STATE_LARA_HANG 10
#define TR_STATE_LARA_REACH 11
//#define TR_STATE_LARA_UNUSED2 12
#define TR_STATE_LARA_UNDERWATER_STOP 13
#define TR_STATE_LARA_GRAB_TO_FALL 14
#define TR_STATE_LARA_JUMP_PREPARE 15
#define TR_STATE_LARA_WALK_BACK 16
#define TR_STATE_LARA_UNDERWATER_FORWARD 17
#define TR_STATE_LARA_UNDERWATER_INERTIA 18
#define TR_STATE_LARA_CLIMBING 19
#define TR_STATE_LARA_TURN_FAST 20
#define TR_STATE_LARA_WALK_RIGHT 21
#define TR_STATE_LARA_WALK_LEFT 22
#define TR_STATE_LARA_ROLL_BACKWARD 23
#define TR_STATE_LARA_SLIDE_FORWARD 24
#define TR_STATE_LARA_JUMP_BACK 25
#define TR_STATE_LARA_JUMP_LEFT 26
#define TR_STATE_LARA_JUMP_RIGHT 27
#define TR_STATE_LARA_JUMP_UP 28
#define TR_STATE_LARA_FALL_BACKWARD 29
#define TR_STATE_LARA_SHIMMY_LEFT 30
#define TR_STATE_LARA_SHIMMY_RIGHT 31
#define TR_STATE_LARA_SLIDE_BACK 32
#define TR_STATE_LARA_ONWATER_STOP 33
#define TR_STATE_LARA_ONWATER_FORWARD 34
#define TR_STATE_LARA_UNDERWATER_DIVING 35
#define TR_STATE_LARA_PUSHABLE_PUSH 36
#define TR_STATE_LARA_PUSHABLE_PULL 37
#define TR_STATE_LARA_PUSHABLE_GRAB 38
#define TR_STATE_LARA_PICKUP 39
#define TR_STATE_LARA_SWITCH_DOWN 40
#define TR_STATE_LARA_SWITCH_UP 41
#define TR_STATE_LARA_INSERT_KEY 42
#define TR_STATE_LARA_INSERT_PUZZLE 43
#define TR_STATE_LARA_WATER_DEATH 44
#define TR_STATE_LARA_ROLL_FORWARD 45
#define TR_STATE_LARA_BOULDER_DEATH 46
#define TR_STATE_LARA_ONWATER_BACK 47
#define TR_STATE_LARA_ONWATER_LEFT 48
#define TR_STATE_LARA_ONWATER_RIGHT 49
#define TR_STATE_LARA_USE_MIDAS 50          //  Derived from leaked TOMB.MAP
#define TR_STATE_LARA_DIE_MIDAS 51          //  Derived from leaked TOMB.MAP
#define TR_STATE_LARA_SWANDIVE_BEGIN 52
#define TR_STATE_LARA_SWANDIVE_END 53
#define TR_STATE_LARA_HANDSTAND 54
#define TR_STATE_LARA_ONWATER_EXIT 55
#define TR_STATE_LARA_LADDER_IDLE 56
#define TR_STATE_LARA_LADDER_UP 57
#define TR_STATE_LARA_LADDER_LEFT 58
//#define TR_STATE_LARA_UNUSED5 59
#define TR_STATE_LARA_LADDER_RIGHT 60
#define TR_STATE_LARA_LADDER_DOWN 61
//#define TR_STATE_LARA_UNUSED6 62
//#define TR_STATE_LARA_UNUSED7 63
//#define TR_STATE_LARA_UNUSED8 64
#define TR_STATE_LARA_WADE_FORWARD 65
#define TR_STATE_LARA_UNDERWATER_TURNAROUND 66
#define TR_STATE_LARA_FLARE_PICKUP 67
#define TR_STATE_LARA_JUMP_ROLL 68
//#define TR_STATE_LARA_UNUSED10 69
#define TR_STATE_LARA_ZIPLINE_RIDE 70
#define TR_STATE_LARA_CROUCH_IDLE 71
#define TR_STATE_LARA_CROUCH_ROLL 72
#define TR_STATE_LARA_SPRINT 73
#define TR_STATE_LARA_SPRINT_ROLL 74
#define TR_STATE_LARA_MONKEYSWING_IDLE 75
#define TR_STATE_LARA_MONKEYSWING_FORWARD 76
#define TR_STATE_LARA_MONKEYSWING_LEFT 77
#define TR_STATE_LARA_MONKEYSWING_RIGHT 78
#define TR_STATE_LARA_MONKEYSWING_TURNAROUND 79
#define TR_STATE_LARA_CRAWL_IDLE 80
#define TR_STATE_LARA_CRAWL_FORWARD 81
#define TR_STATE_LARA_MONKEYSWING_TURN_LEFT 82
#define TR_STATE_LARA_MONKEYSWING_TURN_RIGHT 83
#define TR_STATE_LARA_CRAWL_TURN_LEFT 84
#define TR_STATE_LARA_CRAWL_TURN_RIGHT 85
#define TR_STATE_LARA_CRAWL_BACK 86
#define TR_STATE_LARA_CLIMB_TO_CRAWL 87
#define TR_STATE_LARA_CRAWL_TO_CLIMB 88
#define TR_STATE_LARA_MISC_CONTROL 89
#define TR_STATE_LARA_ROPE_TURN_LEFT 90
#define TR_STATE_LARA_ROPE_TURN_RIGHT 91
#define TR_STATE_LARA_GIANT_BUTTON_PUSH 92
#define TR_STATE_LARA_TRAPDOOR_FLOOR_OPEN 93
//#define TR_STATE_LARA_UNUSED11 94
#define TR_STATE_LARA_ROUND_HANDLE 95
#define TR_STATE_LARA_COGWHEEL 96
#define TR_STATE_LARA_LEVERSWITCH_PUSH 97
#define TR_STATE_LARA_HOLE 98
#define TR_STATE_LARA_POLE_IDLE 99
#define TR_STATE_LARA_POLE_UP 100
#define TR_STATE_LARA_POLE_DOWN 101
#define TR_STATE_LARA_POLE_TURN_LEFT 102
#define TR_STATE_LARA_POLE_TURN_RIGHT 103
#define TR_STATE_LARA_PULLEY 104
#define TR_STATE_LARA_CROUCH_TURN_LEFT 105
#define TR_STATE_LARA_CROUCH_TURN_RIGHT 106
#define TR_STATE_LARA_CLIMB_CORNER_LEFT_OUTER 107
#define TR_STATE_LARA_CLIMB_CORNER_RIGHT_OUTER 108
#define TR_STATE_LARA_CLIMB_CORNER_LEFT_INNER 109
#define TR_STATE_LARA_CLIMB_CORNER_RIGHT_INNER 110
#define TR_STATE_LARA_ROPE_IDLE 111
#define TR_STATE_LARA_ROPE_CLIMB_UP 112
#define TR_STATE_LARA_ROPE_CLIMB_DOWN 113
#define TR_STATE_LARA_ROPE_SWING 114
#define TR_STATE_LARA_LADDER_TO_HANDS 115
#define TR_STATE_LARA_POSITION_CORRECTOR 116
#define TR_STATE_LARA_DOUBLEDOORS_PUSH 117
#define TR_STATE_LARA_DOZY 118
#define TR_STATE_LARA_TIGHTROPE_IDLE 119
#define TR_STATE_LARA_TIGHTROPE_TURNAROUND 120
#define TR_STATE_LARA_TIGHTROPE_FORWARD 121
#define TR_STATE_LARA_TIGHTROPE_BALANCING_LEFT 122
#define TR_STATE_LARA_TIGHTROPE_BALANCING_RIGHT 123
#define TR_STATE_LARA_TIGHTROPE_ENTER 124
#define TR_STATE_LARA_TIGHTROPE_EXIT 125
#define TR_STATE_LARA_DOVESWITCH 126
#define TR_STATE_LARA_TIGHTROPE_RESTORE_BALANCE 127
#define TR_STATE_LARA_BARS_SWING 128
#define TR_STATE_LARA_BARS_JUMP 129
//#define TR_STATE_LARA_UNUSED12 130
#define TR_STATE_LARA_RADIO_LISTENING 131
#define TR_STATE_LARA_RADIO_OFF 132
//#define TR_STATE_LARA_UNUSED13 133
//#define TR_STATE_LARA_UNUSED14 134
//#define TR_STATE_LARA_UNUSED15 135
//#define TR_STATE_LARA_UNUSED16 136
#define TR_STATE_LARA_PICKUP_FROM_CHEST 137

namespace world
{
class Character;
struct HeightInfo;

constexpr float LaraHangVerticalEpsilon = 64.0f;

// State control is based on original TR system, where several animations may
// belong to the same state.
struct StateController
{
public:
    explicit StateController(Character* c);

    void handle(int state);

private:
    using Handler = void (StateController::*)();

    void on(int state, Handler handler);

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

    void setNextState(int state);

    Character* const m_character;
    std::map<int, Handler> m_handlers;
};
} // namespace world
