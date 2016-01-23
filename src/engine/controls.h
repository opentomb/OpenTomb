#pragma once

#include <SDL2/SDL.h>

#include <map>

namespace engine
{

#define JOY_BUTTON_MASK  1000
#define JOY_HAT_MASK     1100
#define JOY_TRIGGER_MASK 1200

#define JOY_TRIGGER_DEADZONE 10000

// Action mapper index constants
enum class Action
{
    // Movement directions
    Up,                     // 0
    Down,                   // 1
    Left,                   // 2
    Right,                  // 3
    // Functional keys
    Action,                 // 4
    Jump,                   // 5
    Roll,                   // 6
    DrawWeapon,             // 7
    Look,                   // 8
    Walk,                   // 9
    Sprint,                 // 10
    Crouch,                 // 11
    StepLeft,               // 12
    StepRight,              // 13
    // Free look keys
    LookUp,                 // 14
    LookDown,               // 15
    LookLeft,               // 16
    LookRight,              // 17
    // Weapon scroller
    NextWeapon,             // 18
    PrevWeapon,             // 19
    // Item hotkeys
    Flare,                  // 20
    BigMedi,                // 21
    SmallMedi,              // 22
    Weapon1,                // 23
    Weapon2,                // 24
    Weapon3,                // 25
    Weapon4,                // 26
    Weapon5,                // 27
    Weapon6,                // 28
    Weapon7,                // 29
    Weapon8,                // 30
    Weapon9,                // 31
    Weapon10,               // 32
    Binoculars,             // 33
    Pls,                    // 34 Not in original, reserved for future
    // Interface keys
    Pause,                  // 35
    Inventory,              // 36
    Diary,                  // 37 Not in original, reserved for future
    Map,                    // 38 Not in original, reserved for future
    LoadGame,               // 39
    SaveGame,               // 40
    // Service keys
    Console,                // 41
    Screenshot,             // 42
    Sentinel
};

enum class Axis
{
    LookX,        // Look axes
    LookY,
    MoveX,        // Move axes
    MoveY
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
    static ControlSettings instance;

    float    mouse_sensitivity = 0;
    float    mouse_scale_x = 0.01f;
    float    mouse_scale_y = 0.01f;

    // Global joystick settings.
    bool   use_joy = false;
    int    joy_number = 0;
    bool   joy_rumble = false;

    // Look axis settings.
    float    joy_look_x = 0;                        // Raw look axis data!
    float    joy_look_y = 0;                        // Raw look axis data!
    bool     joy_look_invert_x = false;
    bool     joy_look_invert_y = false;
    float    joy_look_sensitivity = 0;
    int16_t  joy_look_deadzone = 0;

    // Move axis settings.
    float    joy_move_x = 0;                        // Raw move axis data!
    float    joy_move_y = 0;                        // Raw move axis data!
    bool     joy_move_invert_x = false;
    bool     joy_move_invert_y = false;
    float    joy_move_sensitivity = 0;
    int16_t  joy_move_deadzone = 0;

    std::map<Axis, int>     joy_axis_map;      // Axis array for action mapper.

    std::map<Action, ControlAction> action_map{};         // Actions array for action mapper.

    void joyAxis(int axis, Sint16 value);

    void pollSDLInput();
    void debugKeys(int button, int state);
    void primaryMouseDown();
    void secondaryMouseDown();

    void key(int32_t button, bool state);
    void wrapGameControllerKey(int button, bool state);
    void wrapGameControllerAxis(int axis, Sint16 value);
    void joyHat(int value);
    void joyRumble(float power, int time);
    void refreshStates();
    void initGlobals();
};

} // namespace engine
