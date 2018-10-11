#include <stdlib.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_events.h>

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include "core/system.h"
#include "core/console.h"
#include "core/vmath.h"

#include "script/script.h"
#include "render/camera.h"
#include "physics/physics.h"
#include "gui/gui_obj.h"
#include "gui/gui_inventory.h"
#include "audio/audio.h"
#include "engine.h"
#include "controls.h"
#include "game.h"
#include "gui/gui.h"


struct engine_control_state_s           control_states = {0};
struct control_settings_s               control_settings = {0};


void Controls_Key(int32_t button, int state)
{
    int action = Controls_MapKey(button);
    int is_con_show = Gui_ConIsShown();
    
    if(is_con_show && (action != ACT_CONSOLE))
    {
        return;
    }
    
    // Fill script-driven debug keyboard input.
    Script_AddKey(engine_lua, button, state);

    if((control_states.last_key == 0) && state)
    {
        control_states.last_key = button;
    }
    
    switch(action)
    {
        case ACT_CONSOLE:
            if(!state)
            {
                Gui_ConShow(!is_con_show);
                if(!is_con_show)
                {
                    Audio_PauseStreams();
                    //Audio_Send(lua_GetGlobalSound(engine_lua, TR_AUDIO_SOUND_GLOBALID_MENUOPEN));
                    SDL_ShowCursor(1);
                    SDL_SetRelativeMouseMode(SDL_FALSE);
                    SDL_StartTextInput();
                }
                else
                {
                    Audio_ResumeStreams();
                    //Audio_Send(lua_GetGlobalSound(engine_lua, TR_AUDIO_SOUND_GLOBALID_MENUCLOSE));
                    SDL_ShowCursor(0);
                    SDL_SetRelativeMouseMode(SDL_TRUE);
                    SDL_StopTextInput();
                }
            }
            break;

        case ACT_SCREENSHOT:
            if(!state)
            {
                Engine_TakeScreenShot();
            }
            break;

        case ACT_SAVEGAME:
            if(!state)
            {
                Game_Save("qsave.lua");
            }
            break;

        case ACT_LOADGAME:
            if(!state)
            {
                Game_Load("qsave.lua");
            }
            break;

        case ACT_LOOK:
            control_states.look = state;
            break;
    }
    if(action < ACTIONS::ACT_LASTINDEX)
    {
        if(control_states.actions[action].state && state)
        {
            control_states.actions[action].prev_state = 0x00;
        }
        control_states.actions[action].state = state;
    }
}

void Controls_JoyAxis(int axis, Sint16 axisValue)
{
    for(int i = 0; i < AXIS_LASTINDEX; i++)            // Compare with ALL mapped axes.
    {
        if(axis == control_settings.joy_axis_map[i])      // If mapped = current...
        {
            switch(i)                                   // ...Choose corresponding action.
            {
                case AXIS_LOOK_X:
                    if((axisValue < -control_settings.joy_look_deadzone) || (axisValue > control_settings.joy_look_deadzone))
                    {
                        if(control_settings.joy_look_invert_x)
                        {
                            control_settings.joy_look_x = -(axisValue / (32767 / control_settings.joy_look_sensitivity)); // 32767 is the max./min. axis value.
                        }
                        else
                        {
                            control_settings.joy_look_x = (axisValue / (32767 / control_settings.joy_look_sensitivity));
                        }
                    }
                    else
                    {
                        control_settings.joy_look_x = 0;
                    }
                    return;

                case AXIS_LOOK_Y:
                    if( (axisValue < -control_settings.joy_look_deadzone) || (axisValue > control_settings.joy_look_deadzone) )
                    {
                        if(control_settings.joy_look_invert_y)
                        {
                            control_settings.joy_look_y = -(axisValue / (32767 / control_settings.joy_look_sensitivity));
                        }
                        else
                        {
                            control_settings.joy_look_y = (axisValue / (32767 / control_settings.joy_look_sensitivity));
                        }
                    }
                    else
                    {
                        control_settings.joy_look_y = 0;
                    }
                    return;

                case AXIS_MOVE_X:
                    if( (axisValue < -control_settings.joy_move_deadzone) || (axisValue > control_settings.joy_move_deadzone) )
                    {
                        if(control_settings.joy_move_invert_x)
                        {
                            control_settings.joy_move_x = -(axisValue / (32767 / control_settings.joy_move_sensitivity));

                            if(axisValue > control_settings.joy_move_deadzone)
                            {
                                control_states.actions[ACT_LEFT].state = SDL_PRESSED;
                                control_states.actions[ACT_RIGHT].state = SDL_RELEASED;
                            }
                            else
                            {
                                control_states.actions[ACT_LEFT].state = SDL_RELEASED;
                                control_states.actions[ACT_RIGHT].state = SDL_PRESSED;
                            }
                        }
                        else
                        {
                            control_settings.joy_move_x = (axisValue / (32767 / control_settings.joy_move_sensitivity));
                            if(axisValue > control_settings.joy_move_deadzone)
                            {
                                control_states.actions[ACT_LEFT].state = SDL_RELEASED;
                                control_states.actions[ACT_RIGHT].state = SDL_PRESSED;
                            }
                            else
                            {
                                control_states.actions[ACT_LEFT].state = SDL_PRESSED;
                                control_states.actions[ACT_RIGHT].state = SDL_RELEASED;
                            }
                        }
                    }
                    else
                    {
                        control_states.actions[ACT_LEFT].state = SDL_RELEASED;
                        control_states.actions[ACT_RIGHT].state = SDL_RELEASED;
                        control_settings.joy_move_x = 0;
                    }
                    return;

                case AXIS_MOVE_Y:
                    if((axisValue < -control_settings.joy_move_deadzone) || (axisValue > control_settings.joy_move_deadzone))
                    {

                        if(control_settings.joy_move_invert_y)
                        {
                            control_settings.joy_move_y = -(axisValue / (32767 / control_settings.joy_move_sensitivity));
                            if(axisValue > control_settings.joy_move_deadzone)
                            {
                                control_states.actions[ACT_UP].state = SDL_PRESSED;
                                control_states.actions[ACT_DOWN].state = SDL_RELEASED;
                            }
                            else
                            {
                                control_states.actions[ACT_UP].state = SDL_RELEASED;
                                control_states.actions[ACT_DOWN].state = SDL_PRESSED;
                            }
                        }
                        else
                        {
                            control_settings.joy_move_y = (axisValue / (32767 / control_settings.joy_move_sensitivity));
                            if(axisValue > control_settings.joy_move_deadzone)
                            {
                                control_states.actions[ACT_UP].state = SDL_RELEASED;
                                control_states.actions[ACT_DOWN].state = SDL_PRESSED;
                            }
                            else
                            {
                                control_states.actions[ACT_UP].state = SDL_PRESSED;
                                control_states.actions[ACT_DOWN].state = SDL_RELEASED;
                            }
                        }
                    }
                    else
                    {
                        control_states.actions[ACT_UP].state = SDL_RELEASED;
                        control_states.actions[ACT_DOWN].state = SDL_RELEASED;
                        control_settings.joy_move_y = 0;
                    }
                    return;

                default:
                    return;

            } // end switch(i)
        } // end if(axis == control_mapper.joy_axis_map[i])
    } // end for(int i = 0; i < AXIS_LASTINDEX; i++)
}

void Controls_JoyHat(int value)
{
    // NOTE: Hat movements emulate keypresses
    // with HAT direction + JOY_HAT_MASK (1100) index.

    Controls_Key(JOY_HAT_MASK + SDL_HAT_UP,    SDL_RELEASED);     // Reset all directions.
    Controls_Key(JOY_HAT_MASK + SDL_HAT_DOWN,  SDL_RELEASED);
    Controls_Key(JOY_HAT_MASK + SDL_HAT_LEFT,  SDL_RELEASED);
    Controls_Key(JOY_HAT_MASK + SDL_HAT_RIGHT, SDL_RELEASED);

    if(value & SDL_HAT_UP)
        Controls_Key(JOY_HAT_MASK + SDL_HAT_UP,    SDL_PRESSED);
    if(value & SDL_HAT_DOWN)
        Controls_Key(JOY_HAT_MASK + SDL_HAT_DOWN,  SDL_PRESSED);
    if(value & SDL_HAT_LEFT)
        Controls_Key(JOY_HAT_MASK + SDL_HAT_LEFT,  SDL_PRESSED);
    if(value & SDL_HAT_RIGHT)
        Controls_Key(JOY_HAT_MASK + SDL_HAT_RIGHT, SDL_PRESSED);
}

void Controls_WrapGameControllerKey(int button, int state)
{
    // SDL2 Game Controller interface doesn't operate with HAT directions,
    // instead it treats them as button pushes. So, HAT doesn't return
    // hat motion event on any HAT direction release - instead, each HAT
    // direction generates its own press and release event. That's why
    // game controller's HAT (DPAD) events are directly translated to
    // Controls_Key function.

    switch(button)
    {
        case SDL_CONTROLLER_BUTTON_DPAD_UP:
            Controls_Key(JOY_HAT_MASK + SDL_HAT_UP, state);
            break;
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
            Controls_Key(JOY_HAT_MASK + SDL_HAT_DOWN, state);
            break;
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
            Controls_Key(JOY_HAT_MASK + SDL_HAT_LEFT, state);
            break;
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
            Controls_Key(JOY_HAT_MASK + SDL_HAT_RIGHT, state);
            break;
        default:
            Controls_Key((JOY_BUTTON_MASK + button), state);
            break;
    }
}

void Controls_WrapGameControllerAxis(int axis, Sint16 value)
{
    // Since left/right triggers on X360-like controllers are actually axes,
    // and we still need them as buttons, we remap these axes to button events.
    // Button event is invoked only if trigger is pressed more than 1/3 of its range.
    // Triggers are coded as native SDL2 enum number + JOY_TRIGGER_MASK (1200).

    if((axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT) ||
       (axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT))
    {
        if(value >= JOY_TRIGGER_DEADZONE)
        {
            Controls_Key((axis + JOY_TRIGGER_MASK), SDL_PRESSED);
        }
        else
        {
            Controls_Key((axis + JOY_TRIGGER_MASK), SDL_RELEASED);
        }
    }
    else
    {
        Controls_JoyAxis(axis, value);
    }
}

void Controls_InitGlobals()
{
    control_settings.mouse_sensitivity_x = 0.25f;
    control_settings.mouse_sensitivity_y = 0.25f;
    control_settings.use_joy = 0;

    control_settings.joy_number = 0;              ///@FIXME: Replace with joystick scanner default value when done.
    control_settings.joy_rumble = 0;              ///@FIXME: Make it according to GetCaps of default joystick.

    control_settings.joy_axis_map[AXIS_MOVE_X] = 0;
    control_settings.joy_axis_map[AXIS_MOVE_Y] = 1;
    control_settings.joy_axis_map[AXIS_LOOK_X] = 2;
    control_settings.joy_axis_map[AXIS_LOOK_Y] = 3;

    control_settings.joy_look_invert_x = 0;
    control_settings.joy_look_invert_y = 0;
    control_settings.joy_move_invert_x = 0;
    control_settings.joy_move_invert_y = 0;

    control_settings.joy_look_deadzone = 1500;
    control_settings.joy_move_deadzone = 1500;

    control_settings.joy_look_sensitivity = 1.5f;
    control_settings.joy_move_sensitivity = 1.5f;

    control_states.actions[ACT_JUMP].primary       = SDL_SCANCODE_SPACE;
    control_states.actions[ACT_ACTION].primary     = SDL_SCANCODE_LCTRL;
    control_states.actions[ACT_ROLL].primary       = SDL_SCANCODE_X;
    control_states.actions[ACT_SPRINT].primary     = SDL_SCANCODE_CAPSLOCK;
    control_states.actions[ACT_CROUCH].primary     = SDL_SCANCODE_C;
    control_states.actions[ACT_WALK].primary       = SDL_SCANCODE_LSHIFT;

    control_states.actions[ACT_UP].primary         = SDL_SCANCODE_W;
    control_states.actions[ACT_DOWN].primary       = SDL_SCANCODE_S;
    control_states.actions[ACT_LEFT].primary       = SDL_SCANCODE_A;
    control_states.actions[ACT_RIGHT].primary      = SDL_SCANCODE_D;

    control_states.actions[ACT_STEPLEFT].primary   = SDL_SCANCODE_H;
    control_states.actions[ACT_STEPRIGHT].primary  = SDL_SCANCODE_J;

    control_states.actions[ACT_LOOK].primary       = SDL_SCANCODE_O;
    control_states.actions[ACT_LOOKUP].primary     = SDL_SCANCODE_UP;
    control_states.actions[ACT_LOOKDOWN].primary   = SDL_SCANCODE_DOWN;
    control_states.actions[ACT_LOOKLEFT].primary   = SDL_SCANCODE_LEFT;
    control_states.actions[ACT_LOOKRIGHT].primary  = SDL_SCANCODE_RIGHT;

    control_states.actions[ACT_SCREENSHOT].primary = SDL_SCANCODE_PRINTSCREEN;
    control_states.actions[ACT_CONSOLE].primary    = SDL_SCANCODE_GRAVE;
    control_states.actions[ACT_SAVEGAME].primary   = SDL_SCANCODE_F5;
    control_states.actions[ACT_LOADGAME].primary   = SDL_SCANCODE_F6;
}

void Controls_DebugKeys(int button, int state)
{
    if(state)
    {
        extern float time_scale;
        switch(button)
        {
            case SDL_SCANCODE_Y:
                screen_info.debug_view_state++;
                break;

            case SDL_SCANCODE_G:
                if(time_scale == 1.0f)
                {
                    time_scale = 0.033f;
                }
                else
                {
                    time_scale = 1.0f;
                }
                break;

            case SDL_SCANCODE_L:
                control_states.free_look = !control_states.free_look;
                break;

            case SDL_SCANCODE_N:
                control_states.noclip = !control_states.noclip;
                break;

            default:
                //Con_Printf("key = %d", button);
                break;
        };
    }
}

void Controls_PrimaryMouseDown(float from[3], float to[3])
{
    float test_to[3];
    collision_result_t cb;

    vec3_add_mul(test_to, engine_camera.transform.M4x4 + 12, engine_camera.transform.M4x4 + 8, 32768.0f);
    if(Physics_RayTestFiltered(&cb, engine_camera.transform.M4x4 + 12, test_to, NULL, COLLISION_MASK_ALL))
    {
        vec3_copy(from, cb.point);
        vec3_add_mul(to, cb.point, cb.normale, 256.0f);
    }
}

enum ACTIONS Controls_MapKey(uint32_t key)
{
    for(int i = 0; i < ACT_LASTINDEX; i++)
    {
        if((key == control_states.actions[i].primary) ||
           (key == control_states.actions[i].secondary))
        {
            return (enum ACTIONS)i;
        }
    }
    return ACTIONS::ACT_LASTINDEX;
}

void Controls_ActionToStr(char buff[128], enum ACTIONS act)
{
    switch(act)
    {
        case ACT_UP:
            strncpy(buff, "ACT_UP", 128);
            break;
        case ACT_DOWN:
            strncpy(buff, "ACT_DOWN", 128);
            break;
        case ACT_LEFT:
            strncpy(buff, "ACT_LEFT", 128);
            break;
        case ACT_RIGHT:
            strncpy(buff, "ACT_RIGHT", 128);
            break;
        case ACT_ACTION:
            strncpy(buff, "ACT_ACTION", 128);
            break;
        case ACT_JUMP:
            strncpy(buff, "ACT_JUMP", 128);
            break;
        case ACT_ROLL:
            strncpy(buff, "ACT_ROLL", 128);
            break;
        case ACT_DRAWWEAPON:
            strncpy(buff, "ACT_DRAWWEAPON", 128);
            break;
        case ACT_LOOK:
            strncpy(buff, "ACT_LOOK", 128);
            break;
        case ACT_WALK:
            strncpy(buff, "ACT_WALK", 128);
            break;
        case ACT_SPRINT:
            strncpy(buff, "ACT_SPRINT", 128);
            break;
        case ACT_CROUCH:
            strncpy(buff, "ACT_CROUCH", 128);
            break;
        case ACT_STEPLEFT:
            strncpy(buff, "ACT_STEPLEFT", 128);
            break;
        case ACT_STEPRIGHT:
            strncpy(buff, "ACT_STEPRIGHT", 128);
            break;
        case ACT_LOOKUP:
            strncpy(buff, "ACT_LOOKUP", 128);
            break;
        case ACT_LOOKDOWN:
            strncpy(buff, "ACT_LOOKDOWN", 128);
            break;
        case ACT_LOOKLEFT:
            strncpy(buff, "ACT_LOOKLEFT", 128);
            break;
        case ACT_LOOKRIGHT:
            strncpy(buff, "ACT_LOOKRIGHT", 128);
            break;
        case ACT_NEXTWEAPON:
            strncpy(buff, "ACT_NEXTWEAPON", 128);
            break;
        case ACT_PREVWEAPON:
            strncpy(buff, "ACT_PREVWEAPON", 128);
            break;
        case ACT_FLARE:
            strncpy(buff, "ACT_FLARE", 128);
            break;
        case ACT_BIGMEDI:
            strncpy(buff, "ACT_BIGMEDI", 128);
            break;
        case ACT_SMALLMEDI:
            strncpy(buff, "ACT_SMALLMEDI", 128);
            break;
        case ACT_WEAPON1:
            strncpy(buff, "ACT_WEAPON1", 128);
            break;
        case ACT_WEAPON2:
            strncpy(buff, "ACT_WEAPON2", 128);
            break;
        case ACT_WEAPON3:
            strncpy(buff, "ACT_WEAPON3", 128);
            break;
        case ACT_WEAPON4:
            strncpy(buff, "ACT_WEAPON4", 128);
            break;
        case ACT_WEAPON5:
            strncpy(buff, "ACT_WEAPON5", 128);
            break;
        case ACT_WEAPON6:
            strncpy(buff, "ACT_WEAPON6", 128);
            break;
        case ACT_WEAPON7:
            strncpy(buff, "ACT_WEAPON7", 128);
            break;
        case ACT_WEAPON8:
            strncpy(buff, "ACT_WEAPON8", 128);
            break;
        case ACT_WEAPON9:
            strncpy(buff, "ACT_WEAPON9", 128);
            break;
        case ACT_WEAPON10:
            strncpy(buff, "ACT_WEAPON10", 128);
            break;
        case ACT_BINOCULARS:
            strncpy(buff, "ACT_BINOCULARS", 128);
            break;
        case ACT_PLS:
            strncpy(buff, "ACT_PLS", 128);
            break;
        case ACT_PAUSE:
            strncpy(buff, "ACT_PAUSE", 128);
            break;
        case ACT_INVENTORY:
            strncpy(buff, "ACT_INVENTORY", 128);
            break;
        case ACT_DIARY:
            strncpy(buff, "ACT_DIARY", 128);
            break;
        case ACT_MAP:
            strncpy(buff, "ACT_MAP", 128);
            break;
        case ACT_LOADGAME:
            strncpy(buff, "ACT_LOADGAME", 128);
            break;
        case ACT_SAVEGAME:
            strncpy(buff, "ACT_SAVEGAME", 128);
            break;
        case ACT_CONSOLE:
            strncpy(buff, "ACT_CONSOLE", 128);
            break;
        case ACT_SCREENSHOT:
            strncpy(buff, "ACT_SCREENSHOT", 128);
            break;
        default:
            buff[0] = 0;
            break;
    }
}

void Controls_KeyToStr(char buff[128], int key)
{
    switch(key)
    {
        case 40:
            strncpy(buff, "KEY_RETURN", 128);
            break;
        case 41:
            strncpy(buff, "KEY_ESCAPE", 128);
            break;
        case 42:
            strncpy(buff, "KEY_BACKSPACE", 128);
            break;
        case 43:
            strncpy(buff, "KEY_TAB", 128);
            break;
        case 44:
            strncpy(buff, "KEY_SPACE", 128);
            break;
        case 45:
            strncpy(buff, "KEY_MINUS", 128);
            break;
        case 46:
            strncpy(buff, "KEY_EQUALS", 128);
            break;
        case 47:
            strncpy(buff, "KEY_LEFTBRACKET", 128);
            break;
        case 48:
            strncpy(buff, "KEY_RIGHTBRACKET", 128);
            break;
        case 49:
            strncpy(buff, "KEY_BACKSLASH", 128);
            break;
        case 50:
            strncpy(buff, "KEY_NONUSHASH", 128);
            break;
        case 51:
            strncpy(buff, "KEY_SEMICOLON", 128);
            break;
        case 52:
            strncpy(buff, "KEY_APOSTROPHE", 128);
            break;
        case 53:
            strncpy(buff, "KEY_BACKQUOTE", 128);
            break;
        case 54:
            strncpy(buff, "KEY_COMMA", 128);
            break;
        case 55:
            strncpy(buff, "KEY_PERIOD", 128);
            break;
        case 56:
            strncpy(buff, "KEY_SLASH", 128);
            break;
        case 57:
            strncpy(buff, "KEY_CAPSLOCK", 128);
            break;
        case 39:
            strncpy(buff, "KEY_0", 128);
            break;
        case 30:
            strncpy(buff, "KEY_1", 128);
            break;
        case 31:
            strncpy(buff, "KEY_2", 128);
            break;
        case 32:
            strncpy(buff, "KEY_3", 128);
            break;
        case 33:
            strncpy(buff, "KEY_4", 128);
            break;
        case 34:
            strncpy(buff, "KEY_5", 128);
            break;
        case 35:
            strncpy(buff, "KEY_6", 128);
            break;
        case 36:
            strncpy(buff, "KEY_7", 128);
            break;
        case 37:
            strncpy(buff, "KEY_8", 128);
            break;
        case 38:
            strncpy(buff, "KEY_9", 128);
            break;
        case 4:
            strncpy(buff, "KEY_A", 128);
            break;
        case 5:
            strncpy(buff, "KEY_B", 128);
            break;
        case 6:
            strncpy(buff, "KEY_C", 128);
            break;
        case 7:
            strncpy(buff, "KEY_D", 128);
            break;
        case 8:
            strncpy(buff, "KEY_E", 128);
            break;
        case 9:
            strncpy(buff, "KEY_F", 128);
            break;
        case 10:
            strncpy(buff, "KEY_G", 128);
            break;
        case 11:
            strncpy(buff, "KEY_H", 128);
            break;
        case 12:
            strncpy(buff, "KEY_I", 128);
            break;
        case 13:
            strncpy(buff, "KEY_J", 128);
            break;
        case 14:
            strncpy(buff, "KEY_K", 128);
            break;
        case 15:
            strncpy(buff, "KEY_L", 128);
            break;
        case 16:
            strncpy(buff, "KEY_M", 128);
            break;
        case 17:
            strncpy(buff, "KEY_N", 128);
            break;
        case 18:
            strncpy(buff, "KEY_O", 128);
            break;
        case 19:
            strncpy(buff, "KEY_P", 128);
            break;
        case 20:
            strncpy(buff, "KEY_Q", 128);
            break;
        case 21:
            strncpy(buff, "KEY_R", 128);
            break;
        case 22:
            strncpy(buff, "KEY_S", 128);
            break;
        case 23:
            strncpy(buff, "KEY_T", 128);
            break;
        case 24:
            strncpy(buff, "KEY_U", 128);
            break;
        case 25:
            strncpy(buff, "KEY_V", 128);
            break;
        case 26:
            strncpy(buff, "KEY_W", 128);
            break;
        case 27:
            strncpy(buff, "KEY_X", 128);
            break;
        case 28:
            strncpy(buff, "KEY_Y", 128);
            break;
        case 29:
            strncpy(buff, "KEY_Z", 128);
            break;
        case 58:
            strncpy(buff, "KEY_F1", 128);
            break;
        case 59:
            strncpy(buff, "KEY_F2", 128);
            break;
        case 60:
            strncpy(buff, "KEY_F3", 128);
            break;
        case 61:
            strncpy(buff, "KEY_F4", 128);
            break;
        case 62:
            strncpy(buff, "KEY_F5", 128);
            break;
        case 63:
            strncpy(buff, "KEY_F6", 128);
            break;
        case 64:
            strncpy(buff, "KEY_F7", 128);
            break;
        case 65:
            strncpy(buff, "KEY_F8", 128);
            break;
        case 66:
            strncpy(buff, "KEY_F9", 128);
            break;
        case 67:
            strncpy(buff, "KEY_F10", 128);
            break;
        case 68:
            strncpy(buff, "KEY_F11", 128);
            break;
        case 69:
            strncpy(buff, "KEY_F12", 128);
            break;
        case 70:
            strncpy(buff, "KEY_PRINTSCREEN", 128);
            break;
        case 71:
            strncpy(buff, "KEY_SCROLLLOCK", 128);
            break;
        case 72:
            strncpy(buff, "KEY_PAUSE", 128);
            break;
        case 73:
            strncpy(buff, "KEY_INSERT", 128);
            break;
        case 74:
            strncpy(buff, "KEY_HOME", 128);
            break;
        case 75:
            strncpy(buff, "KEY_PAGEUP", 128);
            break;
        case 76:
            strncpy(buff, "KEY_DELETE", 128);
            break;
        case 77:
            strncpy(buff, "KEY_END", 128);
            break;
        case 78:
            strncpy(buff, "KEY_PAGEDOWN", 128);
            break;
        case 79:
            strncpy(buff, "KEY_RIGHT", 128);
            break;
        case 80:
            strncpy(buff, "KEY_LEFT", 128);
            break;
        case 81:
            strncpy(buff, "KEY_DOWN", 128);
            break;
        case 82:
            strncpy(buff, "KEY_UP", 128);
            break;
        case 83:
            strncpy(buff, "KEY_NUMLOCKCLEAR", 128);
            break;
        case 84:
            strncpy(buff, "KEY_KP_DIVIDE", 128);
            break;
        case 85:
            strncpy(buff, "KEY_KP_MULTIPLY", 128);
            break;
        case 86:
            strncpy(buff, "KEY_KP_MINUS", 128);
            break;
        case 87:
            strncpy(buff, "KEY_KP_PLUS", 128);
            break;
        case 88:
            strncpy(buff, "KEY_KP_ENTER", 128);
            break;
        case 89:
            strncpy(buff, "KEY_KP_1", 128);
            break;
        case 90:
            strncpy(buff, "KEY_KP_2", 128);
            break;
        case 91:
            strncpy(buff, "KEY_KP_3", 128);
            break;
        case 92:
            strncpy(buff, "KEY_KP_4", 128);
            break;
        case 93:
            strncpy(buff, "KEY_KP_5", 128);
            break;
        case 94:
            strncpy(buff, "KEY_KP_6", 128);
            break;
        case 95:
            strncpy(buff, "KEY_KP_7", 128);
            break;
        case 96:
            strncpy(buff, "KEY_KP_8", 128);
            break;
        case 97:
            strncpy(buff, "KEY_KP_9", 128);
            break;
        case 98:
            strncpy(buff, "KEY_KP_0", 128);
            break;
        case 99:
            strncpy(buff, "KEY_KP_PERIOD", 128);
            break;
        case 101:
            strncpy(buff, "KEY_APPLICATION", 128);
            break;
        case 102:
            strncpy(buff, "KEY_POWER", 128);
            break;
        case 103:
            strncpy(buff, "KEY_KP_EQUALS", 128);
            break;
        case 104:
            strncpy(buff, "KEY_F13", 128);
            break;
        case 105:
            strncpy(buff, "KEY_F14", 128);
            break;
        case 106:
            strncpy(buff, "KEY_F15", 128);
            break;
        case 107:
            strncpy(buff, "KEY_F16", 128);
            break;
        case 108:
            strncpy(buff, "KEY_F17", 128);
            break;
        case 109:
            strncpy(buff, "KEY_F18", 128);
            break;
        case 110:
            strncpy(buff, "KEY_F19", 128);
            break;
        case 111:
            strncpy(buff, "KEY_F20", 128);
            break;
        case 112:
            strncpy(buff, "KEY_F21", 128);
            break;
        case 113:
            strncpy(buff, "KEY_F22", 128);
            break;
        case 114:
            strncpy(buff, "KEY_F23", 128);
            break;
        case 115:
            strncpy(buff, "KEY_F24", 128);
            break;
        case 116:
            strncpy(buff, "KEY_EXECUTE", 128);
            break;
        case 117:
            strncpy(buff, "KEY_HELP", 128);
            break;
        case 118:
            strncpy(buff, "KEY_MENU", 128);
            break;
        case 119:
            strncpy(buff, "KEY_SELECT", 128);
            break;
        case 120:
            strncpy(buff, "KEY_STOP", 128);
            break;
        case 121:
            strncpy(buff, "KEY_AGAIN", 128);
            break;
        case 122:
            strncpy(buff, "KEY_UNDO", 128);
            break;
        case 123:
            strncpy(buff, "KEY_CUT", 128);
            break;
        case 124:
            strncpy(buff, "KEY_COPY", 128);
            break;
        case 125:
            strncpy(buff, "KEY_PASTE", 128);
            break;
        case 126:
            strncpy(buff, "KEY_FIND", 128);
            break;
        case 127:
            strncpy(buff, "KEY_MUTE", 128);
            break;
        case 128:
            strncpy(buff, "KEY_VOLUMEUP", 128);
            break;
        case 129:
            strncpy(buff, "KEY_VOLUMEDOWN", 128);
            break;
        case 133:
            strncpy(buff, "KEY_KP_COMMA", 128);
            break;
        case 134:
            strncpy(buff, "KEY_KP_EQUALSAS400", 128);
            break;
        case 153:
            strncpy(buff, "KEY_ALTERASE", 128);
            break;
        case 154:
            strncpy(buff, "KEY_SYSREQ", 128);
            break;
        case 155:
            strncpy(buff, "KEY_CANCEL", 128);
            break;
        case 156:
            strncpy(buff, "KEY_CLEAR", 128);
            break;
        case 157:
            strncpy(buff, "KEY_PRIOR", 128);
            break;
        case 158:
            strncpy(buff, "KEY_RETURN2", 128);
            break;
        case 159:
            strncpy(buff, "KEY_SEPARATOR", 128);
            break;
        case 160:
            strncpy(buff, "KEY_OUT", 128);
            break;
        case 161:
            strncpy(buff, "KEY_OPER", 128);
            break;
        case 162:
            strncpy(buff, "KEY_CLEARAGAIN", 128);
            break;
        case 163:
            strncpy(buff, "KEY_CRSEL", 128);
            break;
        case 164:
            strncpy(buff, "KEY_EXSEL", 128);
            break;
        case 176:
            strncpy(buff, "KEY_KP_00", 128);
            break;
        case 177:
            strncpy(buff, "KEY_KP_000", 128);
            break;
        case 178:
            strncpy(buff, "KEY_THOUSANDSSEPARATOR", 128);
            break;
        case 179:
            strncpy(buff, "KEY_DECIMALSEPARATOR", 128);
            break;
        case 180:
            strncpy(buff, "KEY_CURRENCYUNIT", 128);
            break;
        case 181:
            strncpy(buff, "KEY_CURRENCYSUBUNIT", 128);
            break;
        case 182:
            strncpy(buff, "KEY_KP_LEFTPAREN", 128);
            break;
        case 183:
            strncpy(buff, "KEY_KP_RIGHTPAREN", 128);
            break;
        case 184:
            strncpy(buff, "KEY_KP_LEFTBRACE", 128);
            break;
        case 185:
            strncpy(buff, "KEY_KP_RIGHTBRACE", 128);
            break;
        case 186:
            strncpy(buff, "KEY_KP_TAB", 128);
            break;
        case 187:
            strncpy(buff, "KEY_KP_BACKSPACE", 128);
            break;
        case 188:
            strncpy(buff, "KEY_KP_A", 128);
            break;
        case 189:
            strncpy(buff, "KEY_KP_B", 128);
            break;
        case 190:
            strncpy(buff, "KEY_KP_C", 128);
            break;
        case 191:
            strncpy(buff, "KEY_KP_D", 128);
            break;
        case 192:
            strncpy(buff, "KEY_KP_E", 128);
            break;
        case 193:
            strncpy(buff, "KEY_KP_F", 128);
            break;
        case 194:
            strncpy(buff, "KEY_KP_XOR", 128);
            break;
        case 195:
            strncpy(buff, "KEY_KP_POWER", 128);
            break;
        case 196:
            strncpy(buff, "KEY_KP_PERCENT", 128);
            break;
        case 197:
            strncpy(buff, "KEY_KP_LESS", 128);
            break;
        case 198:
            strncpy(buff, "KEY_KP_GREATER", 128);
            break;
        case 199:
            strncpy(buff, "KEY_KP_AMPERSAND", 128);
            break;
        case 200:
            strncpy(buff, "KEY_KP_DBLAMPERSAND", 128);
            break;
        case 201:
            strncpy(buff, "KEY_KP_VERTICALBAR", 128);
            break;
        case 202:
            strncpy(buff, "KEY_KP_DBLVERTICALBAR", 128);
            break;
        case 203:
            strncpy(buff, "KEY_KP_COLON", 128);
            break;
        case 204:
            strncpy(buff, "KEY_KP_HASH", 128);
            break;
        case 205:
            strncpy(buff, "KEY_KP_SPACE", 128);
            break;
        case 206:
            strncpy(buff, "KEY_KP_AT", 128);
            break;
        case 207:
            strncpy(buff, "KEY_KP_EXCLAM", 128);
            break;
        case 208:
            strncpy(buff, "KEY_KP_MEMSTORE", 128);
            break;
        case 209:
            strncpy(buff, "KEY_KP_MEMRECALL", 128);
            break;
        case 210:
            strncpy(buff, "KEY_KP_MEMCLEAR", 128);
            break;
        case 211:
            strncpy(buff, "KEY_KP_MEMADD", 128);
            break;
        case 212:
            strncpy(buff, "KEY_KP_MEMSUBTRACT", 128);
            break;
        case 213:
            strncpy(buff, "KEY_KP_MEMMULTIPLY", 128);
            break;
        case 214:
            strncpy(buff, "KEY_KP_MEMDIVIDE", 128);
            break;
        case 215:
            strncpy(buff, "KEY_KP_PLUSMINUS", 128);
            break;
        case 216:
            strncpy(buff, "KEY_KP_CLEAR", 128);
            break;
        case 217:
            strncpy(buff, "KEY_KP_CLEARENTRY", 128);
            break;
        case 218:
            strncpy(buff, "KEY_KP_BINARY", 128);
            break;
        case 219:
            strncpy(buff, "KEY_KP_OCTAL", 128);
            break;
        case 220:
            strncpy(buff, "KEY_KP_DECIMAL", 128);
            break;
        case 221:
            strncpy(buff, "KEY_KP_HEXADECIMAL", 128);
            break;
        case 224:
            strncpy(buff, "KEY_LCTRL", 128);
            break;
        case 225:
            strncpy(buff, "KEY_LSHIFT", 128);
            break;
        case 226:
            strncpy(buff, "KEY_LALT", 128);
            break;
        case 227:
            strncpy(buff, "KEY_LGUI", 128);
            break;
        case 228:
            strncpy(buff, "KEY_RCTRL", 128);
            break;
        case 229:
            strncpy(buff, "KEY_RSHIFT", 128);
            break;
        case 230:
            strncpy(buff, "KEY_RALT", 128);
            break;
        case 231:
            strncpy(buff, "KEY_RGUI", 128);
            break;
        case 257:
            strncpy(buff, "KEY_MODE", 128);
            break;
        case 258:
            strncpy(buff, "KEY_AUDIONEXT", 128);
            break;
        case 259:
            strncpy(buff, "KEY_AUDIOPREV", 128);
            break;
        case 260:
            strncpy(buff, "KEY_AUDIOSTOP", 128);
            break;
        case 261:
            strncpy(buff, "KEY_AUDIOPLAY", 128);
            break;
        case 262:
            strncpy(buff, "KEY_AUDIOMUTE", 128);
            break;
        case 263:
            strncpy(buff, "KEY_MEDIASELECT", 128);
            break;
        case 264:
            strncpy(buff, "KEY_WWW", 128);
            break;
        case 265:
            strncpy(buff, "KEY_MAIL", 128);
            break;
        case 266:
            strncpy(buff, "KEY_CALCULATOR", 128);
            break;
        case 267:
            strncpy(buff, "KEY_COMPUTER", 128);
            break;
        case 268:
            strncpy(buff, "KEY_AC_SEARCH", 128);
            break;
        case 269:
            strncpy(buff, "KEY_AC_HOME", 128);
            break;
        case 270:
            strncpy(buff, "KEY_AC_BACK", 128);
            break;
        case 271:
            strncpy(buff, "KEY_AC_FORWARD", 128);
            break;
        case 272:
            strncpy(buff, "KEY_AC_STOP", 128);
            break;
        case 273:
            strncpy(buff, "KEY_AC_REFRESH", 128);
            break;
        case 274:
            strncpy(buff, "KEY_AC_BOOKMARKS", 128);
            break;
        case 275:
            strncpy(buff, "KEY_BRIGHTNESSDOWN", 128);
            break;
        case 276:
            strncpy(buff, "KEY_BRIGHTNESSUP", 128);
            break;
        case 277:
            strncpy(buff, "KEY_DISPLAYSWITCH", 128);
            break;
        case 278:
            strncpy(buff, "KEY_KBDILLUMTOGGLE", 128);
            break;
        case 279:
            strncpy(buff, "KEY_KBDILLUMDOWN", 128);
            break;
        case 280:
            strncpy(buff, "KEY_KBDILLUMUP", 128);
            break;
        case 281:
            strncpy(buff, "KEY_EJECT", 128);
            break;
        case 282:
            strncpy(buff, "KEY_SLEEP", 128);
            break;
        case 1000:
            strncpy(buff, "JOY_1", 128);
            break;
        case 1001:
            strncpy(buff, "JOY_2", 128);
            break;
        case 1002:
            strncpy(buff, "JOY_3", 128);
            break;
        case 1003:
            strncpy(buff, "JOY_4", 128);
            break;
        case 1004:
            strncpy(buff, "JOY_5", 128);
            break;
        case 1005:
            strncpy(buff, "JOY_6", 128);
            break;
        case 1006:
            strncpy(buff, "JOY_7", 128);
            break;
        case 1007:
            strncpy(buff, "JOY_8", 128);
            break;
        case 1008:
            strncpy(buff, "JOY_9", 128);
            break;
        case 1009:
            strncpy(buff, "JOY_10", 128);
            break;
        case 1010:
            strncpy(buff, "JOY_11", 128);
            break;
        case 1011:
            strncpy(buff, "JOY_12", 128);
            break;
        case 1012:
            strncpy(buff, "JOY_13", 128);
            break;
        case 1013:
            strncpy(buff, "JOY_14", 128);
            break;
        case 1014:
            strncpy(buff, "JOY_15", 128);
            break;
        case 1015:
            strncpy(buff, "JOY_16", 128);
            break;
        case 1016:
            strncpy(buff, "JOY_17", 128);
            break;
        case 1017:
            strncpy(buff, "JOY_18", 128);
            break;
        case 1018:
            strncpy(buff, "JOY_19", 128);
            break;
        case 1019:
            strncpy(buff, "JOY_20", 128);
            break;
        case 1020:
            strncpy(buff, "JOY_21", 128);
            break;
        case 1021:
            strncpy(buff, "JOY_22", 128);
            break;
        case 1022:
            strncpy(buff, "JOY_23", 128);
            break;
        case 1023:
            strncpy(buff, "JOY_24", 128);
            break;
        case 1024:
            strncpy(buff, "JOY_25", 128);
            break;
        case 1025:
            strncpy(buff, "JOY_26", 128);
            break;
        case 1026:
            strncpy(buff, "JOY_27", 128);
            break;
        case 1027:
            strncpy(buff, "JOY_28", 128);
            break;
        case 1028:
            strncpy(buff, "JOY_29", 128);
            break;
        case 1029:
            strncpy(buff, "JOY_30", 128);
            break;
        case 1030:
            strncpy(buff, "JOY_31", 128);
            break;
        case 1031:
            strncpy(buff, "JOY_32", 128);
            break;
        case 1101:
            strncpy(buff, "JOY_POVUP", 128);
            break;
        case 1104:
            strncpy(buff, "JOY_POVDOWN", 128);
            break;
        case 1108:
            strncpy(buff, "JOY_POVLEFT", 128);
            break;
        case 1102:
            strncpy(buff, "JOY_POVRIGHT", 128);
            break;
        case 1204:
            strncpy(buff, "JOY_TRIGGERLEFT", 128);
            break;
        case 1205:
            strncpy(buff, "JOY_TRIGGERRIGHT", 128);
            break;

        default:
            snprintf(buff, 128, "%d", key);
            break;
    }
}
