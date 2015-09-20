#include "controls.h"

#include <cstdlib>

#include <SDL2/SDL.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_haptic.h>

#include "common.h"
#include "engine/engine.h"
#include "engine/game.h"
#include "gui/console.h"
#include "inventory.h"
#include "script/script.h"

extern bool done;

namespace engine
{

extern SDL_Joystick         *sdl_joystick;
extern SDL_GameController   *sdl_controller;
extern SDL_Haptic           *sdl_haptic;
extern SDL_Window           *sdl_window;

extern EngineContainer* last_cont;

using gui::Console;

void Controls_Key(int32_t button, bool state)
{
    // Fill script-driven debug keyboard input.

    engine_lua.addKey(button, state);

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
                        Console::instance().toggleVisibility();

                        if(Console::instance().isVisible())
                        {
                            //Audio_Send(lua_GetGlobalSound(engine_lua, TR_AUDIO_SOUND_GLOBALID_MENUOPEN));
                            SDL_SetRelativeMouseMode(SDL_FALSE);
                            SDL_StartTextInput();
                        }
                        else
                        {
                            //Audio_Send(lua_GetGlobalSound(engine_lua, TR_AUDIO_SOUND_GLOBALID_MENUCLOSE));
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
                    if((axisValue < -control_mapper.joy_look_deadzone) || (axisValue > control_mapper.joy_look_deadzone))
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
                    if((axisValue < -control_mapper.joy_move_deadzone) || (axisValue > control_mapper.joy_move_deadzone))
                    {
                        if(control_mapper.joy_move_invert_x)
                        {
                            control_mapper.joy_move_x = -(axisValue / (32767 / control_mapper.joy_move_sensitivity));

                            if(axisValue > control_mapper.joy_move_deadzone)
                            {
                                control_states.move_left = true;
                                control_states.move_right = false;
                            }
                            else
                            {
                                control_states.move_left = false;
                                control_states.move_right = true;
                            }
                        }
                        else
                        {
                            control_mapper.joy_move_x = (axisValue / (32767 / control_mapper.joy_move_sensitivity));
                            if(axisValue > control_mapper.joy_move_deadzone)
                            {
                                control_states.move_left = false;
                                control_states.move_right = true;
                            }
                            else
                            {
                                control_states.move_left = true;
                                control_states.move_right = false;
                            }
                        }
                    }
                    else
                    {
                        control_states.move_left = false;
                        control_states.move_right = false;
                        control_mapper.joy_move_x = 0;
                    }
                    return;

                case AXIS_MOVE_Y:
                    if((axisValue < -control_mapper.joy_move_deadzone) || (axisValue > control_mapper.joy_move_deadzone))
                    {
                        if(control_mapper.joy_move_invert_y)
                        {
                            control_mapper.joy_move_y = -(axisValue / (32767 / control_mapper.joy_move_sensitivity));
                            if(axisValue > control_mapper.joy_move_deadzone)
                            {
                                control_states.move_forward = true;
                                control_states.move_backward = false;
                            }
                            else
                            {
                                control_states.move_forward = false;
                                control_states.move_backward = true;
                            }
                        }
                        else
                        {
                            control_mapper.joy_move_y = (axisValue / (32767 / control_mapper.joy_move_sensitivity));
                            if(axisValue > control_mapper.joy_move_deadzone)
                            {
                                control_states.move_forward = false;
                                control_states.move_backward = true;
                            }
                            else
                            {
                                control_states.move_forward = true;
                                control_states.move_backward = false;
                            }
                        }
                    }
                    else
                    {
                        control_states.move_forward = false;
                        control_states.move_backward = false;
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

    Controls_Key(JOY_HAT_MASK + SDL_HAT_UP, false);     // Reset all directions.
    Controls_Key(JOY_HAT_MASK + SDL_HAT_DOWN, false);
    Controls_Key(JOY_HAT_MASK + SDL_HAT_LEFT, false);
    Controls_Key(JOY_HAT_MASK + SDL_HAT_RIGHT, false);

    if(value & SDL_HAT_UP)
        Controls_Key(JOY_HAT_MASK + SDL_HAT_UP, true);
    if(value & SDL_HAT_DOWN)
        Controls_Key(JOY_HAT_MASK + SDL_HAT_DOWN, true);
    if(value & SDL_HAT_LEFT)
        Controls_Key(JOY_HAT_MASK + SDL_HAT_LEFT, true);
    if(value & SDL_HAT_RIGHT)
        Controls_Key(JOY_HAT_MASK + SDL_HAT_RIGHT, true);
}

void Controls_WrapGameControllerKey(int button, bool state)
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
            Controls_Key((axis + JOY_TRIGGER_MASK), true);
        }
        else
        {
            Controls_Key((axis + JOY_TRIGGER_MASK), false);
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
    control_mapper.use_joy = false;

    control_mapper.joy_number = 0;              ///@FIXME: Replace with joystick scanner default value when done.
    control_mapper.joy_rumble = false;              ///@FIXME: Make it according to GetCaps of default joystick.

    control_mapper.joy_axis_map[AXIS_MOVE_X] = 0;
    control_mapper.joy_axis_map[AXIS_MOVE_Y] = 1;
    control_mapper.joy_axis_map[AXIS_LOOK_X] = 2;
    control_mapper.joy_axis_map[AXIS_LOOK_Y] = 3;

    control_mapper.joy_look_invert_x = false;
    control_mapper.joy_look_invert_y = false;
    control_mapper.joy_move_invert_x = false;
    control_mapper.joy_move_invert_y = false;

    control_mapper.joy_look_deadzone = 1500;
    control_mapper.joy_move_deadzone = 1500;

    control_mapper.joy_look_sensitivity = 1.5;
    control_mapper.joy_move_sensitivity = 1.5;

    control_mapper.action_map[ACT_JUMP].primary = SDLK_SPACE;
    control_mapper.action_map[ACT_ACTION].primary = SDLK_LCTRL;
    control_mapper.action_map[ACT_ROLL].primary = SDLK_x;
    control_mapper.action_map[ACT_SPRINT].primary = SDLK_CAPSLOCK;
    control_mapper.action_map[ACT_CROUCH].primary = SDLK_c;
    control_mapper.action_map[ACT_WALK].primary = SDLK_LSHIFT;

    control_mapper.action_map[ACT_UP].primary = SDLK_w;
    control_mapper.action_map[ACT_DOWN].primary = SDLK_s;
    control_mapper.action_map[ACT_LEFT].primary = SDLK_a;
    control_mapper.action_map[ACT_RIGHT].primary = SDLK_d;

    control_mapper.action_map[ACT_STEPLEFT].primary = SDLK_h;
    control_mapper.action_map[ACT_STEPRIGHT].primary = SDLK_j;

    control_mapper.action_map[ACT_LOOKUP].primary = SDLK_UP;
    control_mapper.action_map[ACT_LOOKDOWN].primary = SDLK_DOWN;
    control_mapper.action_map[ACT_LOOKLEFT].primary = SDLK_LEFT;
    control_mapper.action_map[ACT_LOOKRIGHT].primary = SDLK_RIGHT;

    control_mapper.action_map[ACT_SCREENSHOT].primary = SDLK_PRINTSCREEN;
    control_mapper.action_map[ACT_CONSOLE].primary = SDLK_F12;
    control_mapper.action_map[ACT_SAVEGAME].primary = SDLK_F5;
    control_mapper.action_map[ACT_LOADGAME].primary = SDLK_F6;
}

void Controls_PollSDLInput()
{
    SDL_Event event;

    while(SDL_PollEvent(&event))
    {
        switch(event.type)
        {
            case SDL_MOUSEMOTION:
                if(!Console::instance().isVisible() && control_states.mouse_look)
                {
                        control_states.look_axis_x = event.motion.xrel * control_mapper.mouse_sensitivity * control_mapper.mouse_scale_x;
                        control_states.look_axis_y = event.motion.yrel * control_mapper.mouse_sensitivity * control_mapper.mouse_scale_y;
                }
                break;

            case SDL_MOUSEBUTTONDOWN:
                if(event.button.button == 1) //LM = 1, MM = 2, RM = 3
                {
                    Controls_PrimaryMouseDown();
                }
                else if(event.button.button == 3)
                {
                    Controls_SecondaryMouseDown();
                }
                break;

                // Controller events are only invoked when joystick is initialized as
                // game controller, otherwise, generic joystick event will be used.
            case SDL_CONTROLLERAXISMOTION:
                Controls_WrapGameControllerAxis(event.caxis.axis, event.caxis.value);
                break;

            case SDL_CONTROLLERBUTTONDOWN:
            case SDL_CONTROLLERBUTTONUP:
                Controls_WrapGameControllerKey(event.cbutton.button, event.cbutton.state == SDL_PRESSED);
                break;

                // Joystick events are still invoked, even if joystick is initialized as game
                // controller - that's why we need sdl_joystick checking - to filter out
                // duplicate event calls.

            case SDL_JOYAXISMOTION:
                if(sdl_joystick)
                    Controls_JoyAxis(event.jaxis.axis, event.jaxis.value);
                break;

            case SDL_JOYHATMOTION:
                if(sdl_joystick)
                    Controls_JoyHat(event.jhat.value);
                break;

            case SDL_JOYBUTTONDOWN:
            case SDL_JOYBUTTONUP:
                // NOTE: Joystick button numbers are passed with added JOY_BUTTON_MASK (1000).
                if(sdl_joystick)
                    Controls_Key((event.jbutton.button + JOY_BUTTON_MASK), event.jbutton.state == SDL_PRESSED);
                break;

            case SDL_TEXTINPUT:
            case SDL_TEXTEDITING:
                if(Console::instance().isVisible() && event.key.state)
                {
                    Console::instance().filter(event.text.text);
                    return;
                }
                break;

            case SDL_KEYUP:
            case SDL_KEYDOWN:
                if((event.key.keysym.sym == SDLK_F4) &&
                   (event.key.state == SDL_PRESSED) &&
                   (event.key.keysym.mod & KMOD_ALT))
                {
                    done = true;
                    break;
                }

                if(Console::instance().isVisible() && event.key.state)
                {
                    switch(event.key.keysym.sym)
                    {
                        case SDLK_RETURN:
                        case SDLK_UP:
                        case SDLK_DOWN:
                        case SDLK_LEFT:
                        case SDLK_RIGHT:
                        case SDLK_HOME:
                        case SDLK_END:
                        case SDLK_BACKSPACE:
                        case SDLK_DELETE:
                        case SDLK_TAB:
                        case SDLK_v: // for Ctrl+V
                            Console::instance().edit(event.key.keysym.sym, event.key.keysym.mod);
                            break;
                        default:
                            break;
                    }
                    return;
                }
                else
                {
                    Controls_Key(event.key.keysym.sym, event.key.state == SDL_PRESSED);
                    // DEBUG KEYBOARD COMMANDS
                    Controls_DebugKeys(event.key.keysym.sym, event.key.state);
                }
                break;

            case SDL_QUIT:
                done = true;
                break;

            case SDL_WINDOWEVENT:
                if(event.window.event == SDL_WINDOWEVENT_RESIZED)
                {
                    resize(event.window.data1, event.window.data2, event.window.data1, event.window.data2);
                }
                break;

            default:
                break;
        }
    }
}

///@FIXME: Move to debug.lua script!!!

void Controls_DebugKeys(int button, int state)
{
    if(state)
    {
        switch(button)
        {
            case SDLK_RETURN:
                if(main_inventory_manager)
                {
                    main_inventory_manager->send(InventoryManager::InventoryState::Activate);
                }
                break;

            case SDLK_UP:
                if(main_inventory_manager)
                {
                    main_inventory_manager->send(InventoryManager::InventoryState::Up);
                }
                break;

            case SDLK_DOWN:
                if(main_inventory_manager)
                {
                    main_inventory_manager->send(InventoryManager::InventoryState::Down);
                }
                break;

            case SDLK_LEFT:
                if(main_inventory_manager)
                {
                    main_inventory_manager->send(InventoryManager::InventoryState::RLeft);
                }
                break;

            case SDLK_RIGHT:
                if(main_inventory_manager)
                {
                    main_inventory_manager->send(InventoryManager::InventoryState::RRight);
                }
                break;

            default:
                //Con_Printf("key = %d", button);
                break;
        };
    }
}


void Controls_PrimaryMouseDown()
{
    EngineContainer* cont = new EngineContainer();
    btScalar dbgR = 128.0;
    btVector3 v = engine_camera.getPosition();
    btVector3 dir = engine_camera.getViewDir();
    btVector3 localInertia(0, 0, 0);

    btCollisionShape* cshape = new btSphereShape(dbgR);
    cshape->setMargin(COLLISION_MARGIN_DEFAULT);
    //cshape = new btCapsuleShapeZ(50.0, 100.0);
    btTransform startTransform;
    startTransform.setIdentity();
    btVector3 new_pos = v;
    startTransform.setOrigin(btVector3(new_pos[0], new_pos[1], new_pos[2]));
    cshape->calculateLocalInertia(12.0, localInertia);
    btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);
    btRigidBody* body = new btRigidBody(12.0, motionState, cshape, localInertia);
    bt_engine_dynamicsWorld->addRigidBody(body);
    body->setLinearVelocity(btVector3(dir[0], dir[1], dir[2]) * 6000);
    cont->room = Room_FindPosCogerrence(new_pos, engine_camera.m_currentRoom);
    cont->object_type = engine::ObjectType::BulletMisc;                     // bullet have to destroy this user pointer
    body->setUserPointer(cont);
    body->setCcdMotionThreshold(dbgR);                          // disable tunneling effect
    body->setCcdSweptSphereRadius(dbgR);
}

void Controls_SecondaryMouseDown()
{
    btVector3 from = engine_camera.getPosition();
    btVector3 to = from + engine_camera.getViewDir() * 32768.0;

    std::shared_ptr<EngineContainer> cam_cont = std::make_shared<EngineContainer>();
    cam_cont->room = engine_camera.m_currentRoom;

    BtEngineClosestRayResultCallback cbc(cam_cont);
    //cbc.m_collisionFilterMask = btBroadphaseProxy::StaticFilter | btBroadphaseProxy::KinematicFilter;
    bt_engine_dynamicsWorld->rayTest(from, to, cbc);
    if(cbc.hasHit())
    {
        extern GLfloat cast_ray[6];

        btVector3 place;
        place.setInterpolate3(from, to, cbc.m_closestHitFraction);
        std::copy(place+0, place+3, cast_ray);
        cast_ray[3] = cast_ray[0] + 100.0f * cbc.m_hitNormalWorld[0];
        cast_ray[4] = cast_ray[1] + 100.0f * cbc.m_hitNormalWorld[1];
        cast_ray[5] = cast_ray[2] + 100.0f * cbc.m_hitNormalWorld[2];

        if(EngineContainer* c0 = static_cast<EngineContainer*>(cbc.m_collisionObject->getUserPointer()))
        {
            if(c0->object_type == engine::ObjectType::BulletMisc)
            {
                btCollisionObject* obj = const_cast<btCollisionObject*>(cbc.m_collisionObject);
                btRigidBody* body = btRigidBody::upcast(obj);
                if(body && body->getMotionState())
                {
                    delete body->getMotionState();
                }
                if(body && body->getCollisionShape())
                {
                    delete body->getCollisionShape();
                }

                if(body)
                {
                    body->setUserPointer(nullptr);
                }
                c0->room = nullptr;
                delete c0;

                bt_engine_dynamicsWorld->removeCollisionObject(obj);
                delete obj;
            }
            else
            {
                last_cont = c0;
            }
        }
    }
}

} // namespace engine
