
#ifndef CONTROLS_H
#define	CONTROLS_H

#include <SDL/SDL.h>
#include <stdint.h>


#define ACTIONS_NAMES_LUA "actions = {}; \
    actions.key_up = 0; \
    actions.key_down = 1; \
    actions.key_left = 2; \
    actions.key_right = 3; \
    actions.key_action = 4; \
    actions.key_jump = 5; \
    actions.key_roll = 6; \
    actions.key_drawweapon = 7; \
    actions.key_look = 8; \
    actions.key_walk = 9; \
    actions.key_sprint = 10; \
    actions.key_crouch = 11; \
    actions.key_stepleft = 12; \
    actions.key_stepright = 13; \
    actions.key_lookup = 14; \
    actions.key_lookdown = 15; \
    actions.key_lookleft = 16; \
    actions.key_lookright = 17; \
    actions.key_nextweapon = 18; \
    actions.key_prevweapon = 19; \
    actions.key_flare = 20; \
    actions.key_bigmedi = 21; \
    actions.key_smallmedi = 22; \
    actions.key_weapon1 = 23; \
    actions.key_weapon2 = 24; \
    actions.key_weapon3 = 25; \
    actions.key_weapon4 = 26; \
    actions.key_weapon5 = 27; \
    actions.key_weapon6 = 28; \
    actions.key_weapon7 = 29; \
    actions.key_weapon8 = 30; \
    actions.key_binoculars = 31; \
    actions.key_pls = 32; \
    actions.key_pause = 33; \
    actions.key_inventory = 34; \
    actions.key_diary = 35; \
    actions.key_map = 36; \
    actions.key_loadgame = 37; \
    actions.key_savegame = 38; \
    actions.key_console = 39; \
    actions.key_screenshot = 40;"

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
    ACT_BINOCULARS,             // 31
    ACT_PLS,                    // 32 Not in original, reserved for future
    // Interface keys
    ACT_PAUSE,                  // 33
    ACT_INVENTORY,              // 34
    ACT_DIARY,                  // 35 Not in original, reserved for future
    ACT_MAP,                    // 36 Not in original, reserved for future
    ACT_LOADGAME,               // 37
    ACT_SAVEGAME,               // 38
    // Service keys
    ACT_CONSOLE,                // 39
    ACT_SCREENSHOT,             // 40
    // Last action index. This should ALWAYS remain last entry!
    ACT_LASTINDEX               // 41
};

enum AXES {
    AXIS_LOOK_X,        // Look axes
    AXIS_LOOK_Y,
    AXIS_MOVE_X,        // Move axes
    AXIS_MOVE_Y,
    // Last axis index. This should ALWAYS remain last entry!
    AXIS_LASTINDEX
};


void Controls_Key(int button, int state);
void Controls_JoyAxis(int axis, Sint16 value);
void Controls_JoyHat(int value);
void Controls_InitGlobals();

int  Controls_KeyConsoleFilter(int key, int kmod_states);

#endif	/* CONTROLS_H */

