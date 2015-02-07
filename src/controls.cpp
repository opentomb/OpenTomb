
#include <SDL2/SDL.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_scancode.h>
#include <stdlib.h>

#include "engine.h"
#include "controls.h"
#include "console.h"
#include "common.h"
#include "game.h"
#include "main_SDL.h"

extern SDL_Haptic           *sdl_haptic;

void Controls_Key(int32_t button, int state)
{
    for(int i = 0; i < ACT_LASTINDEX; i++)                                      // Compare ALL mapped buttons.
    {
        if((button == control_mapper.action_map[i].primary) ||
           (button == control_mapper.action_map[i].secondary))                            // If button = mapped action...
        {
            switch(i)                                                           // ...Choose corresponding action.
            {
                case ACT_UP:
                    control_states.move_forward = state;
                    break;

                case ACT_DOWN:
                    control_states.move_backward = state;
                    break;

                case ACT_LEFT:
                    control_states.move_left = state;
                    break;

                case ACT_RIGHT:
                    control_states.move_right = state;
                    break;

                case ACT_ACTION:
                    control_states.state_action = state;
                    break;

                case ACT_JUMP:
                    control_states.move_up = state;
                    control_states.do_jump = state;
                    break;

                case ACT_ROLL:
                    control_states.do_roll = state;
                    break;

                case ACT_WALK:
                    control_states.state_walk = state;
                    break;

                case ACT_SPRINT:
                    control_states.state_sprint = state;
                    break;

                case ACT_CROUCH:
                    control_states.move_down = state;
                    control_states.state_crouch = state;
                    break;

                case ACT_LOOKUP:
                    control_states.look_up = state;
                    break;

                case ACT_LOOKDOWN:
                    control_states.look_down = state;
                    break;

                case ACT_LOOKLEFT:
                    control_states.look_left = state;
                    break;

                case ACT_LOOKRIGHT:
                    control_states.look_right = state;
                    break;

                case ACT_BIGMEDI:
                    if(!control_mapper.action_map[i].already_pressed)
                    {
                        control_states.use_big_medi = state;
                    }
                    break;

                case ACT_SMALLMEDI:
                    if(!control_mapper.action_map[i].already_pressed)
                    {
                        control_states.use_small_medi = state;
                    }
                    break;

                case ACT_CONSOLE:
                    if(!state)
                    {
                        con_base.show = !con_base.show;

                        if(con_base.show)
                        {
                            //Audio_Send(lua_GetGlobalSound(engine_lua, TR_AUDIO_SOUND_GLOBALID_MENUOPEN));
                            SDL_ShowCursor(1);
                            SDL_SetRelativeMouseMode(SDL_FALSE);
                            SDL_StartTextInput();
                        }
                        else
                        {
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
                        Com_TakeScreenShot();
                    }
                    break;

                case ACT_INVENTORY:
                    if(!state)
                    {
                        control_states.gui_inventory = !control_states.gui_inventory;
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

                default:
                    // control_states.move_forward = state;
                    return;
            }

            control_mapper.action_map[i].state = state;
        }
    }
}

void Controls_JoyAxis(int axis, Sint16 axisValue)
{
    for(int i = 0; i < AXIS_LASTINDEX; i++)            // Compare with ALL mapped axes.
    {
        if(axis == control_mapper.joy_axis_map[i])      // If mapped = current...
        {
            switch(i)                                   // ...Choose corresponding action.
            {
                case AXIS_LOOK_X:
                    if( (axisValue < -control_mapper.joy_look_deadzone) || (axisValue > control_mapper.joy_look_deadzone) )
                    {
                        if(control_mapper.joy_look_invert_x)
                        {
                            control_mapper.joy_look_x = -(axisValue / (32767 / control_mapper.joy_look_sensitivity)); // 32767 is the max./min. axis value.
                        }
                        else
                        {
                            control_mapper.joy_look_x = (axisValue / (32767 / control_mapper.joy_look_sensitivity));
                        }
                    }
                    else
                    {
                        control_mapper.joy_look_x = 0;
                    }
                    return;

                case AXIS_LOOK_Y:
                    if( (axisValue < -control_mapper.joy_look_deadzone) || (axisValue > control_mapper.joy_look_deadzone) )
                    {
                        if(control_mapper.joy_look_invert_y)
                        {
                            control_mapper.joy_look_y = -(axisValue / (32767 / control_mapper.joy_look_sensitivity));
                        }
                        else
                        {
                            control_mapper.joy_look_y = (axisValue / (32767 / control_mapper.joy_look_sensitivity));
                        }
                    }
                    else
                    {
                        control_mapper.joy_look_y = 0;
                    }
                    return;

                case AXIS_MOVE_X:
                    if( (axisValue < -control_mapper.joy_move_deadzone) || (axisValue > control_mapper.joy_move_deadzone) )
                    {
                        if(control_mapper.joy_move_invert_x)
                        {
                            control_mapper.joy_move_x = -(axisValue / (32767 / control_mapper.joy_move_sensitivity));

                            if(axisValue > control_mapper.joy_move_deadzone)
                            {
                                control_states.move_left  = SDL_PRESSED;
                                control_states.move_right = SDL_RELEASED;
                            }
                            else
                            {
                                control_states.move_left  = SDL_RELEASED;
                                control_states.move_right = SDL_PRESSED;
                            }
                        }
                        else
                        {
                            control_mapper.joy_move_x = (axisValue / (32767 / control_mapper.joy_move_sensitivity));
                            if(axisValue > control_mapper.joy_move_deadzone)
                            {
                                control_states.move_left  = SDL_RELEASED;
                                control_states.move_right = SDL_PRESSED;
                            }
                            else
                            {
                                control_states.move_left  = SDL_PRESSED;
                                control_states.move_right = SDL_RELEASED;
                            }
                        }
                    }
                    else
                    {
                        control_states.move_left  = SDL_RELEASED;
                        control_states.move_right = SDL_RELEASED;
                        control_mapper.joy_move_x = 0;
                    }
                    return;

                case AXIS_MOVE_Y:
                    if( (axisValue < -control_mapper.joy_move_deadzone) || (axisValue > control_mapper.joy_move_deadzone) )
                    {

                        if(control_mapper.joy_move_invert_y)
                        {
                            control_mapper.joy_move_y = -(axisValue / (32767 / control_mapper.joy_move_sensitivity));
                            if(axisValue > control_mapper.joy_move_deadzone)
                            {
                                control_states.move_forward  = SDL_PRESSED;
                                control_states.move_backward = SDL_RELEASED;
                            }
                            else
                            {
                                control_states.move_forward  = SDL_RELEASED;
                                control_states.move_backward = SDL_PRESSED;
                            }
                        }
                        else
                        {
                            control_mapper.joy_move_y = (axisValue / (32767 / control_mapper.joy_move_sensitivity));
                            if(axisValue > control_mapper.joy_move_deadzone)
                            {
                                control_states.move_forward  = SDL_RELEASED;
                                control_states.move_backward = SDL_PRESSED;
                            }
                            else
                            {
                                control_states.move_forward  = SDL_PRESSED;
                                control_states.move_backward = SDL_RELEASED;
                            }
                        }
                    }
                    else
                    {
                        control_states.move_forward  = SDL_RELEASED;
                        control_states.move_backward = SDL_RELEASED;
                        control_mapper.joy_move_y = 0;
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

    if( (axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT) ||
        (axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT) )
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

void Controls_JoyRumble(float power, int time)
{
    // JoyRumble is a simple wrapper for SDL's haptic rumble play.

    if(sdl_haptic)
        SDL_HapticRumblePlay(sdl_haptic, power, time);
}

void Controls_RefreshStates()
{
    for(int i = 0; i < ACT_LASTINDEX; i++)
    {
        if(control_mapper.action_map[i].state)
        {
            control_mapper.action_map[i].already_pressed = true;
        }
        else
        {
            control_mapper.action_map[i].already_pressed = false;
        }
    }
}

void Controls_InitGlobals()
{
    control_mapper.mouse_sensitivity = 25.0;
    control_mapper.use_joy = 0;

    control_mapper.joy_number = 0;              ///@FIXME: Replace with joystick scanner default value when done.
    control_mapper.joy_rumble = 0;              ///@FIXME: Make it according to GetCaps of default joystick.

    control_mapper.joy_axis_map[AXIS_MOVE_X] = 0;
    control_mapper.joy_axis_map[AXIS_MOVE_Y] = 1;
    control_mapper.joy_axis_map[AXIS_LOOK_X] = 2;
    control_mapper.joy_axis_map[AXIS_LOOK_Y] = 3;

    control_mapper.joy_look_invert_x = 0;
    control_mapper.joy_look_invert_y = 0;
    control_mapper.joy_move_invert_x = 0;
    control_mapper.joy_move_invert_y = 0;

    control_mapper.joy_look_deadzone = 1500;
    control_mapper.joy_move_deadzone = 1500;

    control_mapper.joy_look_sensitivity = 1.5;
    control_mapper.joy_move_sensitivity = 1.5;

    control_mapper.action_map[ACT_JUMP].primary       = SDLK_SPACE;
    control_mapper.action_map[ACT_ACTION].primary     = SDLK_LCTRL;
    control_mapper.action_map[ACT_ROLL].primary       = SDLK_x;
    control_mapper.action_map[ACT_SPRINT].primary     = SDLK_CAPSLOCK;
    control_mapper.action_map[ACT_CROUCH].primary     = SDLK_c;
    control_mapper.action_map[ACT_WALK].primary       = SDLK_LSHIFT;

    control_mapper.action_map[ACT_UP].primary         = SDLK_w;
    control_mapper.action_map[ACT_DOWN].primary       = SDLK_s;
    control_mapper.action_map[ACT_LEFT].primary       = SDLK_a;
    control_mapper.action_map[ACT_RIGHT].primary      = SDLK_d;

    control_mapper.action_map[ACT_STEPLEFT].primary   = SDLK_h;
    control_mapper.action_map[ACT_STEPRIGHT].primary  = SDLK_j;

    control_mapper.action_map[ACT_LOOKUP].primary     = SDLK_UP;
    control_mapper.action_map[ACT_LOOKDOWN].primary   = SDLK_DOWN;
    control_mapper.action_map[ACT_LOOKLEFT].primary   = SDLK_LEFT;
    control_mapper.action_map[ACT_LOOKRIGHT].primary  = SDLK_RIGHT;

    control_mapper.action_map[ACT_SCREENSHOT].primary = SDLK_PRINTSCREEN;
    control_mapper.action_map[ACT_CONSOLE].primary    = SDLK_BACKQUOTE;
    control_mapper.action_map[ACT_SAVEGAME].primary   = SDLK_F5;
    control_mapper.action_map[ACT_LOADGAME].primary   = SDLK_F6;
}
