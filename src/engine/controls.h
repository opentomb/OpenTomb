#pragma once

#include <LinearMath/btScalar.h>

#include <SDL2/SDL.h>

namespace engine
{

#define JOY_BUTTON_MASK  1000
#define JOY_HAT_MASK     1100
#define JOY_TRIGGER_MASK 1200

#define JOY_TRIGGER_DEADZONE 10000

// Action mapper index constants
enum ACTIONS
{
    // Movement directions
    ACT_UP,                     // 0
    ACT_DOWN,                   // 1
    ACT_LEFT,                   // 2
    ACT_RIGHT,                  // 3
    // Functional keys
    ACT_ACTION,                 // 4
    ACT_JUMP,                   // 5
    ACT_ROLL,                   // 6
    ACT_DRAWWEAPON,             // 7
    ACT_LOOK,                   // 8
    ACT_WALK,                   // 9
    ACT_SPRINT,                 // 10
    ACT_CROUCH,                 // 11
    ACT_STEPLEFT,               // 12
    ACT_STEPRIGHT,              // 13
    // Free look keys
    ACT_LOOKUP,                 // 14
    ACT_LOOKDOWN,               // 15
    ACT_LOOKLEFT,               // 16
    ACT_LOOKRIGHT,              // 17
    // Weapon scroller
    ACT_NEXTWEAPON,             // 18
    ACT_PREVWEAPON,             // 19
    // Item hotkeys
    ACT_FLARE,                  // 20
    ACT_BIGMEDI,                // 21
    ACT_SMALLMEDI,              // 22
    ACT_WEAPON1,                // 23
    ACT_WEAPON2,                // 24
    ACT_WEAPON3,                // 25
    ACT_WEAPON4,                // 26
    ACT_WEAPON5,                // 27
    ACT_WEAPON6,                // 28
    ACT_WEAPON7,                // 29
    ACT_WEAPON8,                // 30
    ACT_WEAPON9,                // 31
    ACT_WEAPON10,               // 32
    ACT_BINOCULARS,             // 33
    ACT_PLS,                    // 34 Not in original, reserved for future
    // Interface keys
    ACT_PAUSE,                  // 35
    ACT_INVENTORY,              // 36
    ACT_DIARY,                  // 37 Not in original, reserved for future
    ACT_MAP,                    // 38 Not in original, reserved for future
    ACT_LOADGAME,               // 39
    ACT_SAVEGAME,               // 40
    // Service keys
    ACT_CONSOLE,                // 41
    ACT_SCREENSHOT,             // 42
    // Last action index. This should ALWAYS remain last entry!
    ACT_LASTINDEX               // 43
};

enum AXES
{
    AXIS_LOOK_X,        // Look axes
    AXIS_LOOK_Y,
    AXIS_MOVE_X,        // Move axes
    AXIS_MOVE_Y,
    // Last axis index. This should ALWAYS remain last entry!
    AXIS_LASTINDEX
};

struct ControlAction
{
    int      primary = 0;
    int      secondary = 0;
    bool     state = false;
    bool     already_pressed = false;
};

//! @todo Use bool where appropriate.
struct ControlSettings
{
    float    mouse_sensitivity = 0;
    float    mouse_scale_x = 0.01f;
    float    mouse_scale_y = 0.01f;

    // Global joystick settings.
    bool   use_joy = false;
    int    joy_number = 0;
    bool   joy_rumble = false;

    // Look axis settings.
    btScalar joy_look_x = 0;                        // Raw look axis data!
    btScalar joy_look_y = 0;                        // Raw look axis data!
    bool     joy_look_invert_x = false;
    bool     joy_look_invert_y = false;
    btScalar joy_look_sensitivity = 0;
    int16_t  joy_look_deadzone = 0;

    // Move axis settings.
    btScalar joy_move_x = 0;                        // Raw move axis data!
    btScalar joy_move_y = 0;                        // Raw move axis data!
    bool     joy_move_invert_x = false;
    bool     joy_move_invert_y = false;
    btScalar joy_move_sensitivity = 0;
    int16_t  joy_move_deadzone = 0;

    int      joy_axis_map[AXIS_LASTINDEX+1] = {0};      // Axis array for action mapper.

    ControlAction  action_map[ACT_LASTINDEX+1]{};         // Actions array for action mapper.
};

void Controls_PollSDLInput();
void Controls_DebugKeys(int button, int state);
void Controls_PrimaryMouseDown();
void Controls_SecondaryMouseDown();

void Controls_Key(int32_t button, bool state);
void Controls_WrapGameControllerKey(int button, bool state);
void Controls_WrapGameControllerAxis(int axis, Sint16 value);
void Controls_JoyAxis(int axis, Sint16 value);
void Controls_JoyHat(int value);
void Controls_JoyRumble(float power, int time);
void Controls_RefreshStates();
void Controls_InitGlobals();

} // namespace engine
