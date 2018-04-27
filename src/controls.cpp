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


struct engine_control_state_s           control_states = {0};
struct control_settings_s               control_mapper = {0};


void Controls_Key(int32_t button, int state)
{
    // Fill script-driven debug keyboard input.
    Script_AddKey(engine_lua, button, state);

    for(int i = 0; i < ACT_LASTINDEX; i++)
    {
        if((button == control_states.actions[i].primary) ||
           (button == control_states.actions[i].secondary))
        {
            switch(i)
            {
                case ACT_CONSOLE:
                    if(!state)
                    {
                        Con_SetShown(!Con_IsShown());

                        if(Con_IsShown())
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
            }
            if(control_states.actions[i].state && state)
            {
                control_states.actions[i].prev_state = 0x00;
            }
            control_states.actions[i].state = state;
            break;
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
                    if((axisValue < -control_mapper.joy_look_deadzone) || (axisValue > control_mapper.joy_look_deadzone))
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
                            control_mapper.joy_move_x = (axisValue / (32767 / control_mapper.joy_move_sensitivity));
                            if(axisValue > control_mapper.joy_move_deadzone)
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
                            control_mapper.joy_move_y = (axisValue / (32767 / control_mapper.joy_move_sensitivity));
                            if(axisValue > control_mapper.joy_move_deadzone)
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

void Controls_InitGlobals()
{
    control_mapper.mouse_sensitivity_x = 0.25f;
    control_mapper.mouse_sensitivity_y = 0.25f;
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

    control_mapper.joy_look_sensitivity = 1.5f;
    control_mapper.joy_move_sensitivity = 1.5f;

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


void Controls_SecondaryMouseDown(struct engine_container_s **cont, float dot[3])
{
    float from[3], to[3];
    engine_container_t cam_cont;
    collision_result_t cb;

    vec3_copy(from, engine_camera.transform.M4x4 + 12);
    vec3_add_mul(to, from, engine_camera.transform.M4x4 + 8, 32768.0f);

    cam_cont.next = NULL;
    cam_cont.object = NULL;
    cam_cont.object_type = 0;
    cam_cont.room = engine_camera.current_room;

    if(Physics_RayTest(&cb, from, to, &cam_cont, COLLISION_MASK_ALL))
    {
        if(cb.obj && cb.obj->object_type != OBJECT_BULLET_MISC)
        {
            *cont = cb.obj;
            vec3_copy(dot, cb.point);
        }
    }
}
