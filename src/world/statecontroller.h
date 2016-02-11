#pragma once

#include "util/helpers.h"

#include <iostream>
#include <map>

namespace world
{
namespace animation
{
enum class AnimUpdate;
}

enum class LaraState
{
    WalkForward = 0,
    RunForward = 1,
    Stop = 2,
    JumpForward = 3,
    Pose = 4,                // Derived from leaked TOMB.MAP
    RunBack = 5,
    TurnRightSlow = 6,
    TurnLeftSlow = 7,
    Death = 8,
    FreeFall = 9,
    Hang = 10,
    Reach = 11,
    //    UNUSED2 12
    UnderwaterStop = 13,
    GrabToFall = 14,
    JumpPrepare = 15,
    WalkBackward = 16,
    UnderwaterForward = 17,
    UnderwaterInertia = 18,
    Climbing = 19,
    TurnFast = 20,
    StepRight = 21,
    StepLeft = 22,
    RollBackward = 23,
    SlideForward = 24,
    JumpBack = 25,
    JumpLeft = 26,
    JumpRight = 27,
    JumpUp = 28,
    FallBackward = 29,
    ShimmyLeft = 30,
    ShimmyRight = 31,
    SlideBackward = 32,
    OnWaterStop = 33,
    OnWaterForward = 34,
    UnderwaterDiving = 35,
    PushablePush = 36,
    PushablePull = 37,
    PushableGrab = 38,
    PickUp = 39,
    SwitchDown = 40,
    SwitchUp = 41,
    InsertKey = 42,
    InsertPuzzle = 43,
    WaterDeath = 44,
    RollForward = 45,
    BoulderDeath = 46,
    OnWaterBackward = 47,
    OnWaterLeft = 48,
    OnWaterRight = 49,
    UseMidas = 50,          //  Derived from leaked TOMB.MAP
    MidasDeath = 51,          //  Derived from leaked TOMB.MAP
    SwandiveBegin = 52,
    SwandiveEnd = 53,
    Handstand = 54,
    OnWaterExit = 55,
    LadderIdle = 56,
    LadderUp = 57,
    LadderLeft = 58,
    //    UNUSED5 59
    LadderRight = 60,
    LadderDown = 61,
    //    UNUSED6 62
    //    UNUSED7 63
    //    UNUSED8 64
    WadeForward = 65,
    UnderwaterTurnAround = 66,
    FlarePickUp = 67,
    JumpRoll = 68,
    //    UNUSED10 69
    ZiplineRide = 70,
    CrouchIdle = 71,
    CrouchRoll = 72,
    Sprint = 73,
    SprintRoll = 74,
    MonkeyswingIdle = 75,
    MonkeyswingForward = 76,
    MonkeyswingLeft = 77,
    MonkeyswingRight = 78,
    MonkeyswingTurnAround = 79,
    CrawlIdle = 80,
    CrawlForward = 81,
    MonkeyswingTurnLeft = 82,
    MonkeyswingTurnRight = 83,
    CrawlTurnLeft = 84,
    CrawlTurnRight = 85,
    CrawlBackward = 86,
    ClimbToCrawl = 87,
    CrawlToClimb = 88,
    MiscControl = 89,
    RopeTurnLeft = 90,
    RopeTurnRight = 91,
    GiantButtonPush = 92,
    TrapdoorFloorOpen = 93,
    //    UNUSED11 94
    RoundHandle = 95,
    CogWheel = 96,
    LeverSwitchPush = 97,
    Hole = 98,
    PoleIdle = 99,
    PoleUp = 100,
    PoleDown = 101,
    PoleTurnLeft = 102,
    PoleTurnRight = 103,
    Pulley = 104,
    CrouchTurnLeft = 105,
    CrouchTurnRight = 106,
    ClimbOuterCornerLeft = 107,
    ClimbOuterCornerRight = 108,
    ClimbInnerCornerLeft = 109,
    ClimbInnerCornerRight = 110,
    RopeIdle = 111,
    RopeClimbUp = 112,
    RopeClimbDown = 113,
    RopeSwing = 114,
    LadderToHands = 115,
    PositionCorrector = 116,
    DoubledoorsPush = 117,
    Dozy = 118,
    TightropeIdle = 119,
    TightropeTurnAround = 120,
    TightropeForward = 121,
    TightropeBalancingLeft = 122,
    TightropeBalancingRight = 123,
    TightropeEnter = 124,
    TightropeExit = 125,
    DoveSwitch = 126,
    TightropeRestoreBalance = 127,
    BarsSwing = 128,
    BarsJump = 129,
    //    UNUSED12 130
    RadioListening = 131,
    RadioOff = 132,
    //    UNUSED13 133
    //    UNUSED14 134
    //    UNUSED15 135
    //    UNUSED16 136
    PickUpFromChest = 137
};

ENUM_TO_OSTREAM(LaraState)

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

    static void onFrameStopTraverse(Character& ent, animation::AnimUpdate state);
    static void onFrameSetOnFloor(Character& ent, animation::AnimUpdate state);
    static void onFrameSetOnFloorAfterClimb(Character& ent, animation::AnimUpdate state);
    static void onFrameSetUnderwater(Character& ent, animation::AnimUpdate state);
    static void onFrameSetFreeFalling(Character& ent, animation::AnimUpdate state);
    static void onFrameSetCmdSlide(Character& ent, animation::AnimUpdate state);
    static void onFrameCorrectDivingAngle(Character& ent, animation::AnimUpdate state);
    static void onFrameToOnWater(Character& ent, animation::AnimUpdate state);
    static void onFrameClimbOutOfWater(Character& ent, animation::AnimUpdate state);
    static void onFrameToEdgeClimb(Character& ent, animation::AnimUpdate state);
    static void onFrameToMonkeyswing(Character& ent, animation::AnimUpdate state);
    static void onFrameToTightrope(Character& ent, animation::AnimUpdate state);
    static void onFrameFromTightrope(Character& ent, animation::AnimUpdate state);
    static void onFrameCrawlToClimb(Character& ent, animation::AnimUpdate state);

    // ---------------------------------------

    HeightInfo initHeightInfo() const;

    bool isLowVerticalSpace() const;

    bool isLastFrame() const;

    void setNextState(LaraState state);

    Character* const m_character;
    std::map<LaraState, Handler> m_handlers;
};
} // namespace world
