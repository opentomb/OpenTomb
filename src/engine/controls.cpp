#include "controls.h"

#include "common.h"
#include "engine/bullet.h"
#include "engine/engine.h"
#include "engine/game.h"
#include "gui/console.h"
#include "inventory.h"
#include "script/script.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_haptic.h>

#include <glm/gtc/type_ptr.hpp>

#include <boost/range/adaptors.hpp>

#include <cstdlib>
#include <stdexcept>

extern bool done;

namespace engine
{

extern world::Object* last_object;

using gui::Console;

ControlSettings ControlSettings::instance{};

void ControlSettings::key(int32_t button, bool state)
{
    // Fill script-driven debug keyboard input.

    engine_lua.addKey(button, state);

    // Compare ALL mapped buttons.

    for(const auto& i : action_map)
    {
        if(button != i.second.primary && button != i.second.secondary)  // If button = mapped action...
            continue;

        switch(i.first)                                           // ...Choose corresponding action.
        {
            case Action::Up:
                Engine::instance.m_controlState.m_moveForward = state;
                break;

            case Action::Down:
                Engine::instance.m_controlState.m_moveBackward = state;
                break;

            case Action::Left:
                Engine::instance.m_controlState.m_moveLeft = state;
                break;

            case Action::Right:
                Engine::instance.m_controlState.m_moveRight = state;
                break;

            case Action::DrawWeapon:
                Engine::instance.m_controlState.m_doDrawWeapon = state;
                break;

            case Action::Action:
                Engine::instance.m_controlState.m_stateAction = state;
                break;

            case Action::Jump:
                Engine::instance.m_controlState.m_moveUp = state;
                Engine::instance.m_controlState.m_doJump = state;
                break;

            case Action::Roll:
                Engine::instance.m_controlState.m_doRoll = state;
                break;

            case Action::Walk:
                Engine::instance.m_controlState.m_stateWalk = state;
                break;

            case Action::Sprint:
                Engine::instance.m_controlState.m_stateSprint = state;
                break;

            case Action::Crouch:
                Engine::instance.m_controlState.m_moveDown = state;
                Engine::instance.m_controlState.m_stateCrouch = state;
                break;

            case Action::LookUp:
                Engine::instance.m_controlState.m_lookUp = state;
                break;

            case Action::LookDown:
                Engine::instance.m_controlState.m_lookDown = state;
                break;

            case Action::LookLeft:
                Engine::instance.m_controlState.m_lookLeft = state;
                break;

            case Action::LookRight:
                Engine::instance.m_controlState.m_lookRight = state;
                break;

            case Action::BigMedi:
                if(!action_map[i.first].already_pressed)
                {
                    Engine::instance.m_controlState.m_useBigMedi = state;
                }
                break;

            case Action::SmallMedi:
                if(!action_map[i.first].already_pressed)
                {
                    Engine::instance.m_controlState.m_useSmallMedi = state;
                }
                break;

            case Action::Console:
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

            case Action::Screenshot:
                if(!state)
                {
                    Com_TakeScreenShot();
                }
                break;

            case Action::Inventory:
                Engine::instance.m_controlState.m_guiInventory = state;
                break;

            case Action::SaveGame:
                if(!state)
                {
                    Game_Save("qsave.lua");
                }
                break;

            case Action::LoadGame:
                if(!state)
                {
                    Game_Load("qsave.lua");
                }
                break;

            default:
                // control_states.move_forward = state;
                return;
        }

        action_map[i.first].state = state;
    }
}

void ControlSettings::joyAxis(int axis, Sint16 axisValue)
{
    auto axisFilter = [axis](const std::pair<Axis,int>& entry){
        return entry.second == axis;
    };
    for(Axis i : joy_axis_map | boost::adaptors::filtered(axisFilter) | boost::adaptors::map_keys)            // Compare with ALL mapped axes.
    {
        switch(i)                                   // ...Choose corresponding action.
        {
            case Axis::LookX:
                if(axisValue < -joy_look_deadzone || axisValue > joy_look_deadzone)
                {
                    if(joy_look_invert_x)
                    {
                        joy_look_x = -(axisValue / (32767 / joy_look_sensitivity)); // 32767 is the max./min. axis value.
                    }
                    else
                    {
                        joy_look_x = axisValue / (32767 / joy_look_sensitivity);
                    }
                }
                else
                {
                    joy_look_x = 0;
                }
                return;

            case Axis::LookY:
                if(axisValue < -joy_look_deadzone || axisValue > joy_look_deadzone)
                {
                    if(joy_look_invert_y)
                    {
                        joy_look_y = -(axisValue / (32767 / joy_look_sensitivity));
                    }
                    else
                    {
                        joy_look_y = axisValue / (32767 / joy_look_sensitivity);
                    }
                }
                else
                {
                    joy_look_y = 0;
                }
                return;

            case Axis::MoveX:
                if(axisValue < -joy_move_deadzone || axisValue > joy_move_deadzone)
                {
                    if(joy_move_invert_x)
                    {
                        joy_move_x = -(axisValue / (32767 / joy_move_sensitivity));

                        if(axisValue > joy_move_deadzone)
                        {
                            Engine::instance.m_controlState.m_moveLeft = true;
                            Engine::instance.m_controlState.m_moveRight = false;
                        }
                        else
                        {
                            Engine::instance.m_controlState.m_moveLeft = false;
                            Engine::instance.m_controlState.m_moveRight = true;
                        }
                    }
                    else
                    {
                        joy_move_x = axisValue / (32767 / joy_move_sensitivity);
                        if(axisValue > joy_move_deadzone)
                        {
                            Engine::instance.m_controlState.m_moveLeft = false;
                            Engine::instance.m_controlState.m_moveRight = true;
                        }
                        else
                        {
                            Engine::instance.m_controlState.m_moveLeft = true;
                            Engine::instance.m_controlState.m_moveRight = false;
                        }
                    }
                }
                else
                {
                    Engine::instance.m_controlState.m_moveLeft = false;
                    Engine::instance.m_controlState.m_moveRight = false;
                    joy_move_x = 0;
                }
                return;

            case Axis::MoveY:
                if(axisValue < -joy_move_deadzone || axisValue > joy_move_deadzone)
                {
                    if(joy_move_invert_y)
                    {
                        joy_move_y = -(axisValue / (32767 / joy_move_sensitivity));
                        if(axisValue > joy_move_deadzone)
                        {
                            Engine::instance.m_controlState.m_moveForward = true;
                            Engine::instance.m_controlState.m_moveBackward = false;
                        }
                        else
                        {
                            Engine::instance.m_controlState.m_moveForward = false;
                            Engine::instance.m_controlState.m_moveBackward = true;
                        }
                    }
                    else
                    {
                        joy_move_y = axisValue / (32767 / joy_move_sensitivity);
                        if(axisValue > joy_move_deadzone)
                        {
                            Engine::instance.m_controlState.m_moveForward = false;
                            Engine::instance.m_controlState.m_moveBackward = true;
                        }
                        else
                        {
                            Engine::instance.m_controlState.m_moveForward = true;
                            Engine::instance.m_controlState.m_moveBackward = false;
                        }
                    }
                }
                else
                {
                    Engine::instance.m_controlState.m_moveForward = false;
                    Engine::instance.m_controlState.m_moveBackward = false;
                    joy_move_y = 0;
                }
                return;

            default:
                BOOST_THROW_EXCEPTION(std::runtime_error("Invalid axis"));
        } // end switch(i)
    } // end for(int i = 0; i < AXIS_LASTINDEX; i++)
}

void ControlSettings::joyHat(int value)
{
    // NOTE: Hat movements emulate keypresses
    // with HAT direction + JOY_HAT_MASK (1100) index.

    key(JOY_HAT_MASK + SDL_HAT_UP, false);     // Reset all directions.
    key(JOY_HAT_MASK + SDL_HAT_DOWN, false);
    key(JOY_HAT_MASK + SDL_HAT_LEFT, false);
    key(JOY_HAT_MASK + SDL_HAT_RIGHT, false);

    if(value & SDL_HAT_UP)
        key(JOY_HAT_MASK + SDL_HAT_UP, true);
    if(value & SDL_HAT_DOWN)
        key(JOY_HAT_MASK + SDL_HAT_DOWN, true);
    if(value & SDL_HAT_LEFT)
        key(JOY_HAT_MASK + SDL_HAT_LEFT, true);
    if(value & SDL_HAT_RIGHT)
        key(JOY_HAT_MASK + SDL_HAT_RIGHT, true);
}

void ControlSettings::wrapGameControllerKey(int button, bool state)
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
            key(JOY_HAT_MASK + SDL_HAT_UP, state);
            break;
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
            key(JOY_HAT_MASK + SDL_HAT_DOWN, state);
            break;
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
            key(JOY_HAT_MASK + SDL_HAT_LEFT, state);
            break;
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
            key(JOY_HAT_MASK + SDL_HAT_RIGHT, state);
            break;
        default:
            key(JOY_BUTTON_MASK + button, state);
            break;
    }
}

void ControlSettings::wrapGameControllerAxis(int axis, Sint16 value)
{
    // Since left/right triggers on X360-like controllers are actually axes,
    // and we still need them as buttons, we remap these axes to button events.
    // Button event is invoked only if trigger is pressed more than 1/3 of its range.
    // Triggers are coded as native SDL2 enum number + JOY_TRIGGER_MASK (1200).

    if(axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT ||
       axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT)
    {
        if(value >= JOY_TRIGGER_DEADZONE)
        {
            key(axis + JOY_TRIGGER_MASK, true);
        }
        else
        {
            key(axis + JOY_TRIGGER_MASK, false);
        }
    }
    else
    {
        joyAxis(axis, value);
    }
}

void ControlSettings::joyRumble(float power, int time)
{
    // JoyRumble is a simple wrapper for SDL's haptic rumble play.

    if(Engine::instance.m_haptic)
        SDL_HapticRumblePlay(Engine::instance.m_haptic, power, time);
}

void ControlSettings::refreshStates()
{
    for(ControlAction& action : action_map | boost::adaptors::map_values)
    {
        if(action.state)
        {
            action.already_pressed = true;
        }
        else
        {
            action.already_pressed = false;
        }
    }
}

void ControlSettings::initGlobals()
{
    mouse_sensitivity = 25.0;
    use_joy = false;

    joy_number = 0;              ///@FIXME: Replace with joystick scanner default value when done.
    joy_rumble = false;              ///@FIXME: Make it according to GetCaps of default joystick.

    joy_axis_map[Axis::MoveX] = 0;
    joy_axis_map[Axis::MoveY] = 1;
    joy_axis_map[Axis::LookX] = 2;
    joy_axis_map[Axis::LookY] = 3;

    joy_look_invert_x = false;
    joy_look_invert_y = false;
    joy_move_invert_x = false;
    joy_move_invert_y = false;

    joy_look_deadzone = 1500;
    joy_move_deadzone = 1500;

    joy_look_sensitivity = 1.5;
    joy_move_sensitivity = 1.5;

    action_map[Action::Jump].primary = SDLK_SPACE;
    action_map[Action::Action].primary = SDLK_LCTRL;
    action_map[Action::Roll].primary = SDLK_x;
    action_map[Action::Sprint].primary = SDLK_CAPSLOCK;
    action_map[Action::Crouch].primary = SDLK_c;
    action_map[Action::Walk].primary = SDLK_LSHIFT;

    action_map[Action::Up].primary = SDLK_w;
    action_map[Action::Down].primary = SDLK_s;
    action_map[Action::Left].primary = SDLK_a;
    action_map[Action::Right].primary = SDLK_d;

    action_map[Action::StepLeft].primary = SDLK_h;
    action_map[Action::StepRight].primary = SDLK_j;

    action_map[Action::LookUp].primary = SDLK_UP;
    action_map[Action::LookDown].primary = SDLK_DOWN;
    action_map[Action::LookLeft].primary = SDLK_LEFT;
    action_map[Action::LookRight].primary = SDLK_RIGHT;

    action_map[Action::Screenshot].primary = SDLK_PRINTSCREEN;
    action_map[Action::Console].primary = SDLK_F12;
    action_map[Action::SaveGame].primary = SDLK_F5;
    action_map[Action::LoadGame].primary = SDLK_F6;
}

void ControlSettings::pollSDLInput()
{
    SDL_Event event;

    while(SDL_PollEvent(&event))
    {
        switch(event.type)
        {
            case SDL_MOUSEMOTION:
                if(!Console::instance().isVisible() && Engine::instance.m_controlState.m_mouseLook)
                {
                        Engine::instance.m_controlState.m_lookAxisX = event.motion.xrel * mouse_sensitivity * mouse_scale_x;
                        Engine::instance.m_controlState.m_lookAxisY = event.motion.yrel * mouse_sensitivity * mouse_scale_y;
                }
                break;

            case SDL_MOUSEBUTTONDOWN:
                if(event.button.button == 1) //LM = 1, MM = 2, RM = 3
                {
                    primaryMouseDown();
                }
                else if(event.button.button == 3)
                {
                    secondaryMouseDown();
                }
                break;

                // Controller events are only invoked when joystick is initialized as
                // game controller, otherwise, generic joystick event will be used.
            case SDL_CONTROLLERAXISMOTION:
                wrapGameControllerAxis(event.caxis.axis, event.caxis.value);
                break;

            case SDL_CONTROLLERBUTTONDOWN:
            case SDL_CONTROLLERBUTTONUP:
                wrapGameControllerKey(event.cbutton.button, event.cbutton.state == SDL_PRESSED);
                break;

                // Joystick events are still invoked, even if joystick is initialized as game
                // controller - that's why we need sdl_joystick checking - to filter out
                // duplicate event calls.

            case SDL_JOYAXISMOTION:
                if(Engine::instance.m_joystick)
                    joyAxis(event.jaxis.axis, event.jaxis.value);
                break;

            case SDL_JOYHATMOTION:
                if(Engine::instance.m_joystick)
                    joyHat(event.jhat.value);
                break;

            case SDL_JOYBUTTONDOWN:
            case SDL_JOYBUTTONUP:
                // NOTE: Joystick button numbers are passed with added JOY_BUTTON_MASK (1000).
                if(Engine::instance.m_joystick)
                    key(event.jbutton.button + JOY_BUTTON_MASK, event.jbutton.state == SDL_PRESSED);
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
                if(event.key.keysym.sym == SDLK_F4 &&
                   event.key.state == SDL_PRESSED &&
                   event.key.keysym.mod & KMOD_ALT)
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
                    key(event.key.keysym.sym, event.key.state == SDL_PRESSED);
                    // DEBUG KEYBOARD COMMANDS
                    debugKeys(event.key.keysym.sym, event.key.state);
                }
                break;

            case SDL_QUIT:
                done = true;
                break;

            case SDL_WINDOWEVENT:
                if(event.window.event == SDL_WINDOWEVENT_RESIZED)
                {
                    Engine::instance.resize(event.window.data1, event.window.data2, event.window.data1, event.window.data2);
                }
                break;

            default:
                break;
        }
    }
}

///@FIXME: Move to debug.lua script!!!

void ControlSettings::debugKeys(int button, int state)
{
    if(state)
    {
        switch(button)
        {
            case SDLK_RETURN:
                if(gui::Gui::instance)
                {
                    gui::Gui::instance->inventory.send(InventoryManager::InventoryState::Activate);
                }
                break;

            case SDLK_UP:
                if(gui::Gui::instance)
                {
                    gui::Gui::instance->inventory.send(InventoryManager::InventoryState::Up);
                }
                break;

            case SDLK_DOWN:
                if(gui::Gui::instance)
                {
                    gui::Gui::instance->inventory.send(InventoryManager::InventoryState::Down);
                }
                break;

            case SDLK_LEFT:
                if(gui::Gui::instance)
                {
                    gui::Gui::instance->inventory.send(InventoryManager::InventoryState::RLeft);
                }
                break;

            case SDLK_RIGHT:
                if(gui::Gui::instance)
                {
                    gui::Gui::instance->inventory.send(InventoryManager::InventoryState::RRight);
                }
                break;

            default:
                //Con_Printf("key = %d", button);
                break;
        };
    }
}

void ControlSettings::primaryMouseDown()
{
    glm::float_t dbgR = 128.0;
    glm::vec3 v = Engine::instance.m_camera.getPosition();
    glm::vec3 dir = Engine::instance.m_camera.getViewDir();
    btVector3 localInertia(0, 0, 0);

    btCollisionShape* cshape = new btSphereShape(dbgR);
    cshape->setMargin(COLLISION_MARGIN_DEFAULT);
    //cshape = new btCapsuleShapeZ(50.0, 100.0);
    btTransform startTransform;
    startTransform.setIdentity();
    glm::vec3 new_pos = v;
    startTransform.setOrigin(util::convert(new_pos));
    cshape->calculateLocalInertia(12.0, localInertia);
    btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);
    btRigidBody* body = new btRigidBody(12.0, motionState, cshape, localInertia);
    BulletEngine::instance->dynamicsWorld->addRigidBody(body);
    body->setLinearVelocity(util::convert(dir) * 6000);
    world::BulletObject* object = new world::BulletObject(Room_FindPosCogerrence(new_pos, Engine::instance.m_camera.getCurrentRoom()));
    body->setUserPointer(object);
    body->setCcdMotionThreshold(dbgR);                          // disable tunneling effect
    body->setCcdSweptSphereRadius(dbgR);
}

void ControlSettings::secondaryMouseDown()
{
    glm::vec3 from = Engine::instance.m_camera.getPosition();
    glm::vec3 to = from + Engine::instance.m_camera.getViewDir() * 32768.0f;

    world::BulletObject* cam_cont = new world::BulletObject(Engine::instance.m_camera.getCurrentRoom());

    BtEngineClosestRayResultCallback cbc(cam_cont);
    //cbc.m_collisionFilterMask = btBroadphaseProxy::StaticFilter | btBroadphaseProxy::KinematicFilter;
    BulletEngine::instance->dynamicsWorld->rayTest(util::convert(from), util::convert(to), cbc);
    if(cbc.hasHit())
    {
        glm::vec3 place = glm::mix(from, to, cbc.m_closestHitFraction);
        Engine::instance.m_castRay[0] = place;
        Engine::instance.m_castRay[1] = Engine::instance.m_castRay[0] + 100.0f * util::convert(cbc.m_hitNormalWorld);

        if(world::Object* c0 = static_cast<world::Object*>(cbc.m_collisionObject->getUserPointer()))
        {
            if(dynamic_cast<world::BulletObject*>(c0))
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
                delete c0;

                BulletEngine::instance->dynamicsWorld->removeCollisionObject(obj);
                delete obj;
            }
            else
            {
                last_object = c0;
            }
        }
    }
}

} // namespace engine
