
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
        if((button == control_mapper.action_map[i]) ||
           (button == control_mapper.action_alt[i]))                            // If button = mapped action...
        {
            switch(i)                                                           // ...Choose corresponding action.
            {
                case ACT_UP:
                    control_states.move_forward = state;
                    return;

                case ACT_DOWN:
                    control_states.move_backward = state;
                    return;

                case ACT_LEFT:
                    control_states.move_left = state;
                    return;

                case ACT_RIGHT:
                    control_states.move_right = state;
                    return;

                case ACT_ACTION:
                    control_states.state_action = state;
                    return;

                case ACT_JUMP:
                    control_states.move_up = state;
                    control_states.do_jump = state;
                    return;

                case ACT_ROLL:
                    control_states.do_roll = state;
                    return;

                case ACT_WALK:
                    control_states.state_walk = state;
                    return;

                case ACT_SPRINT:
                    control_states.state_sprint = state;
                    return;

                case ACT_CROUCH:
                    control_states.move_down = state;
                    control_states.state_crouch = state;
                    return;

                case ACT_LOOKUP:
                    control_states.look_up = state;
                    return;

                case ACT_LOOKDOWN:
                    control_states.look_down = state;
                    return;

                case ACT_LOOKLEFT:
                    control_states.look_left = state;
                    return;

                case ACT_LOOKRIGHT:
                    control_states.look_right = state;
                    return;

                case ACT_CONSOLE:
                    if(!state)
                    {
                        con_base.show = !con_base.show;
                        if(con_base.show)
                        {
                            SDL_ShowCursor(1);
                        }
                        else
                        {
                            SDL_ShowCursor(0);
                        }
                    }
                    return;

                case ACT_SCREENSHOT:
                    if(!state)
                    {
                        Com_TakeScreenShot();
                    }
                    return;

                case ACT_SAVEGAME:
                    if(!state)
                    {
                        Game_Save("qsave.txt");
                    }
                    return;

                case ACT_LOADGAME:
                    if(!state)
                    {
                        Game_Load("qsave.txt");
                    }
                    return;

                default:
                    // control_states.move_forward = state;
                    return;
            }
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

int Controls_KeyConsoleFilter(int32_t key, int kmod_states)
{
    switch(key)
    {
        case SDLK_BACKSLASH:
            if( !(kmod_states & KMOD_SHIFT) )
                return key;
            else
                return '|';

        case SDLK_SLASH:
            if( !(kmod_states & KMOD_SHIFT) )
                return key;
            else
                return '?';
        case SDLK_QUOTE:
            if( !(kmod_states & KMOD_SHIFT) )
                return key;
            else
                return '\"';
        case SDLK_MINUS:
            if( !(kmod_states & KMOD_SHIFT) )
                return key;
            else
                return '_';

        case SDLK_EQUALS:
            if( !(kmod_states & KMOD_SHIFT) )
                return key;
            else
                return '+';

        case SDLK_COMMA:
            if( !(kmod_states & KMOD_SHIFT) )
                return key;
            else
                return '<';

        case SDLK_PERIOD:
            if( !(kmod_states & KMOD_SHIFT) )
                return key;
            else
                return '>';

        case SDLK_LEFTBRACKET:
            if( !(kmod_states & KMOD_SHIFT) )
                return key;
            else
                return '{';

        case SDLK_RIGHTBRACKET:
            if( !(kmod_states & KMOD_SHIFT) )
                return key;
            else
                return '}';

        case SDLK_KP_DIVIDE:
            return SDLK_COLON;

        case SDLK_KP_MULTIPLY:
            return SDLK_ASTERISK;

        case SDLK_KP_MINUS:
            return SDLK_MINUS;

        case SDLK_KP_PLUS:
            return SDLK_PLUS;

        case SDLK_KP_ENTER:
            return SDLK_RETURN;

        case SDLK_KP_EQUALS:
            return SDLK_EQUALS;

        case SDLK_BACKSPACE:
        case SDLK_DELETE:
        case SDLK_SPACE:
        case SDLK_RETURN:
        case SDLK_LEFT:
        case SDLK_RIGHT:
        case SDLK_UP:
        case SDLK_DOWN:
        case SDLK_HOME:
        case SDLK_END:
            return key;

        case SDLK_KP_PERIOD:
            return (kmod_states & KMOD_NUM)?(SDLK_PERIOD):(SDLK_DELETE);

        case SDLK_KP_0:
        case SDLK_KP_1:
        case SDLK_KP_2:
        case SDLK_KP_3:
        case SDLK_KP_4:
        case SDLK_KP_5:
        case SDLK_KP_6:
        case SDLK_KP_7:
        case SDLK_KP_8:
        case SDLK_KP_9:
            if(kmod_states & KMOD_NUM)
            {
                return key - 208;
            }
            else
            {
                switch(key)
                {
                case SDLK_KP_1:
                    return SDLK_END;
                case SDLK_KP_2:
                    return SDLK_DOWN;
                case SDLK_KP_4:
                    return SDLK_LEFT;
                case SDLK_KP_6:
                    return SDLK_RIGHT;
                case SDLK_KP_7:
                    return SDLK_HOME;
                case SDLK_KP_8:
                    return SDLK_UP;
                default:
                    return 0;
                }
            }

        default:
            if((key <= SDLK_9) && (key >= SDLK_0))
            {
                if( !(kmod_states & KMOD_SHIFT) )
                {
                    return key;
                }
                else switch(key)
                {
                    case SDLK_1:
                        return '!';

                    case SDLK_2:
                        return '@';

                    case SDLK_3:
                        return '#';

                    case SDLK_4:
                        return '$';

                    case SDLK_5:
                        return '%';

                    case SDLK_6:
                        return '^';

                    case SDLK_7:
                        return '&';

                    case SDLK_8:
                        return '*';

                    case SDLK_9:
                        return '(';

                    case SDLK_0:
                        return ')';
                }
            }
            else if((key <= SDLK_z) && (key >= SDLK_a))
            {
                if( (kmod_states & KMOD_SHIFT) || (kmod_states & KMOD_CAPS) )
                {
                    return key - SDLK_a + 'A';
                }
                else
                {
                    return key - SDLK_a + 'a';
                }
            }
            else
            {
                return 0;
            }
    }
	return 0;
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

    control_mapper.action_map[ACT_JUMP]       = SDLK_SPACE;
    control_mapper.action_map[ACT_ACTION]     = SDLK_LCTRL;
    control_mapper.action_map[ACT_ROLL]       = SDLK_x;
    control_mapper.action_map[ACT_SPRINT]     = SDLK_CAPSLOCK;
    control_mapper.action_map[ACT_CROUCH]     = SDLK_v;
    control_mapper.action_map[ACT_WALK]       = SDLK_LSHIFT;

    control_mapper.action_map[ACT_UP]         = SDLK_w;
    control_mapper.action_map[ACT_DOWN]       = SDLK_s;
    control_mapper.action_map[ACT_LEFT]       = SDLK_a;
    control_mapper.action_map[ACT_RIGHT]      = SDLK_d;

    control_mapper.action_map[ACT_STEPLEFT]   = SDLK_h;
    control_mapper.action_map[ACT_STEPRIGHT]  = SDLK_j;

    control_mapper.action_map[ACT_LOOKUP]     = SDLK_UP;
    control_mapper.action_map[ACT_LOOKDOWN]   = SDLK_DOWN;
    control_mapper.action_map[ACT_LOOKLEFT]   = SDLK_LEFT;
    control_mapper.action_map[ACT_LOOKRIGHT]  = SDLK_RIGHT;

    control_mapper.action_map[ACT_SCREENSHOT] = SDLK_PRINTSCREEN;
    control_mapper.action_map[ACT_CONSOLE]    = SDLK_BACKQUOTE;
    control_mapper.action_map[ACT_SAVEGAME]   = SDLK_F5;
    control_mapper.action_map[ACT_LOADGAME]   = SDLK_F6;
}
