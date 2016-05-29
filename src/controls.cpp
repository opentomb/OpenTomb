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

#include "render/camera.h"
#include "anim_state_control.h"
#include "script.h"
#include "engine.h"
#include "physics.h"
#include "controls.h"
#include "gui.h"
#include "game.h"
#include "script.h"


void Controls_Key(int32_t button, int state)
{
    // Fill script-driven debug keyboard input.

    Script_AddKey(engine_lua, button, state);

    // Compare ALL mapped buttons.

    for(int i = 0; i < ACT_LASTINDEX; i++)
    {
        if((button == control_mapper.action_map[i].primary) ||
           (button == control_mapper.action_map[i].secondary))  // If button = mapped action...
        {
            switch(i)                                           // ...Choose corresponding action.
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

                case ACT_DRAWWEAPON:
                    control_states.do_draw_weapon = state;
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
                        Con_SetShown(!Con_IsShown());

                        if(Con_IsShown())
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
                        Sys_TakeScreenShot();
                    }
                    break;

                case ACT_INVENTORY:
                    control_states.gui_inventory = state;
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

void Controls_DebugKeys(int button, int state)
{
    if(state)
    {
        extern float time_scale;
        switch(button)
        {
            case SDLK_RETURN:
                if(main_inventory_manager)
                {
                    main_inventory_manager->send(gui_InventoryManager::INVENTORY_ACTIVATE);
                }
                break;

            case SDLK_UP:
                if(main_inventory_manager)
                {
                    main_inventory_manager->send(gui_InventoryManager::INVENTORY_UP);
                }
                break;

            case SDLK_DOWN:
                if(main_inventory_manager)
                {
                    main_inventory_manager->send(gui_InventoryManager::INVENTORY_DOWN);
                }
                break;

            case SDLK_LEFT:
                if(main_inventory_manager)
                {
                    main_inventory_manager->send(gui_InventoryManager::INVENTORY_R_LEFT);
                }
                break;

            case SDLK_RIGHT:
                if(main_inventory_manager)
                {
                    main_inventory_manager->send(gui_InventoryManager::INVENTORY_R_RIGHT);
                }
                break;

            case SDLK_g:
                if(time_scale == 1.0)
                {
                    time_scale = 0.033;
                }
                else
                {
                    time_scale = 1.0;
                }
                break;

            case SDLK_l:
                control_states.free_look = !control_states.free_look;
                break;

            case SDLK_n:
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

    vec3_add_mul(test_to, engine_camera.pos, engine_camera.view_dir, 32768.0f);
    if(Physics_RayTestFiltered(&cb, engine_camera.pos, test_to, NULL))
    {
        vec3_copy(from, cb.point);
        vec3_add_mul(to, cb.point, cb.normale, 256.0);
    }
}


void Controls_SecondaryMouseDown(struct engine_container_s **cont, float dot[3])
{
    float from[3], to[3];
    engine_container_t cam_cont;
    collision_result_t cb;

    vec3_copy(from, engine_camera.pos);
    vec3_add_mul(to, from, engine_camera.view_dir, 32768.0);

    cam_cont.next = NULL;
    cam_cont.object = NULL;
    cam_cont.object_type = 0;
    cam_cont.room = engine_camera.current_room;

    if(Physics_RayTest(&cb, from, to, &cam_cont))
    {
        if(cb.obj && cb.obj->object_type != OBJECT_BULLET_MISC)
        {
            *cont = cb.obj;
            vec3_copy(dot, cb.point);
        }
    }
}
