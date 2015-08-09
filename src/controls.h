
#ifndef CONTROLS_H
#define CONTROLS_H

#include <SDL2/SDL.h>
#include <stdint.h>

#define JOY_BUTTON_MASK  1000
#define JOY_HAT_MASK     1100
#define JOY_TRIGGER_MASK 1200

#define JOY_TRIGGER_DEADZONE 10000

// Action mapper index constants
enum ACTIONS {
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

enum AXES {
    AXIS_LOOK_X,        // Look axes
    AXIS_LOOK_Y,
    AXIS_MOVE_X,        // Move axes
    AXIS_MOVE_Y,
    // Last axis index. This should ALWAYS remain last entry!
    AXIS_LASTINDEX
};

typedef struct control_action_s
{
    int      primary;
    int      secondary;
    bool     state;
    bool     already_pressed;
}control_action_t, *control_action_p;

typedef struct control_settings_s
{
    float    mouse_sensitivity;

    // Global joystick settings.
    int8_t   use_joy;
    int8_t   joy_number;
    int8_t   joy_rumble;

    // Look axis settings.
    btScalar joy_look_x;                        // Raw look axis data!
    btScalar joy_look_y;                        // Raw look axis data!
    int8_t   joy_look_invert_x;
    int8_t   joy_look_invert_y;
    btScalar joy_look_sensitivity;
    int16_t  joy_look_deadzone;

    // Move axis settings.
    btScalar joy_move_x;                        // Raw move axis data!
    btScalar joy_move_y;                        // Raw move axis data!
    int8_t   joy_move_invert_x;
    int8_t   joy_move_invert_y;
    btScalar joy_move_sensitivity;
    int16_t  joy_move_deadzone;

    int8_t   joy_axis_map[AXIS_LASTINDEX];      // Axis array for action mapper.

    control_action_s  action_map[ACT_LASTINDEX];         // Actions array for action mapper.
}control_settings_t, *control_settings_p;


extern struct engine_control_state_s            control_states;
extern struct control_settings_s                control_mapper;

void Controls_PollSDLInput();
void Controls_DebugKeys(int button, int state);
void Controls_PrimaryMouseDown();
void Controls_SecondaryMouseDown();

void Controls_Key(int32_t button, int state);
void Controls_WrapGameControllerKey(int button, int state);
void Controls_WrapGameControllerAxis(int axis, Sint16 value);
void Controls_JoyAxis(int axis, Sint16 value);
void Controls_JoyHat(int value);
void Controls_JoyRumble(float power, int time);
void Controls_RefreshStates();
void Controls_InitGlobals();

#endif /* CONTROLS_H */
