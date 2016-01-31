#include "inputhandler.h"

#include "engine/bullet.h"
#include "engine/engine.h"
#include "gui/console.h"
#include "script/script.h"
#include "world/character.h"

#include <glm/glm.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/log/trivial.hpp>

#include <SDL2/SDL.h>

namespace engine
{

using gui::Console;

InputHandler::InputHandler(Engine* engine)
    : m_engine(engine)
{
}

void InputHandler::primaryMouseDown()
{
    glm::float_t dbgR = 128.0;
    glm::vec3 v = m_engine->m_camera.getPosition();
    glm::vec3 dir = m_engine->m_camera.getViewDir();
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
    m_engine->m_bullet.dynamicsWorld->addRigidBody(body);
    body->setLinearVelocity(util::convert(dir) * 6000);
    world::BulletObject* object = new world::BulletObject(&m_engine->m_world, m_engine->m_world.Room_FindPosCogerrence(new_pos, m_engine->m_camera.getCurrentRoom()));
    body->setUserPointer(object);
    body->setCcdMotionThreshold(dbgR);                          // disable tunneling effect
    body->setCcdSweptSphereRadius(dbgR);
}

void InputHandler::secondaryMouseDown()
{
    glm::vec3 from = m_engine->m_camera.getPosition();
    glm::vec3 to = from + m_engine->m_camera.getViewDir() * 32768.0f;

    world::BulletObject* cam_cont = new world::BulletObject(&m_engine->m_world, m_engine->m_camera.getCurrentRoom());

    BtEngineClosestRayResultCallback cbc(cam_cont);
    //cbc.m_collisionFilterMask = btBroadphaseProxy::StaticFilter | btBroadphaseProxy::KinematicFilter;
    m_engine->m_bullet.dynamicsWorld->rayTest(util::convert(from), util::convert(to), cbc);
    if(!cbc.hasHit())
        return;

    glm::vec3 place = glm::mix(from, to, cbc.m_closestHitFraction);
    m_engine->m_castRay[0] = place;
    m_engine->m_castRay[1] = m_engine->m_castRay[0] + 100.0f * util::convert(cbc.m_hitNormalWorld);

    world::Object* c0 = static_cast<world::Object*>(cbc.m_collisionObject->getUserPointer());
    if(c0 == nullptr)
        return;

    if(dynamic_cast<world::BulletObject*>(c0) == nullptr)
    {
        m_engine->m_lastObject = c0;
        return;
    }

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

    m_engine->m_bullet.dynamicsWorld->removeCollisionObject(obj);
    delete obj;
}

InputHandler::~InputHandler()
{
    if(m_joystick)
    {
        SDL_JoystickClose(m_joystick);
    }

    if(m_gameController)
    {
        SDL_GameControllerClose(m_gameController);
    }

    if(m_haptic)
    {
        SDL_HapticClose(m_haptic);
    }
}

void InputHandler::configureControllers(int controllerId, bool useJoystick, bool useHaptic)
{
    SDL_InitSubSystem(SDL_INIT_EVENTS);
    if(!useJoystick)
        return;

    SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);

    auto numJoysticks = SDL_NumJoysticks();

    if(numJoysticks < 1 || numJoysticks - 1 < controllerId)
    {
        BOOST_LOG_TRIVIAL(error) << "There is no joystick #" << controllerId << " present";
        return;
    }

    m_joystickId = controllerId;

    if(useHaptic)
        SDL_InitSubSystem(SDL_INIT_HAPTIC);

    if(SDL_IsGameController(m_joystickId))                     // If joystick has mapping (e.g. X360 controller)
    {
        SDL_GameControllerEventState(SDL_ENABLE);                           // Use GameController API
        m_gameController = SDL_GameControllerOpen(m_joystickId);

        if(!m_gameController)
        {
            BOOST_LOG_TRIVIAL(error) << "Can't open game controller #" << controllerId;
            SDL_GameControllerEventState(SDL_DISABLE);                      // If controller init failed, close state.
            m_joystickId = -1;
            m_joystick = nullptr;
        }
        else if(useHaptic)                                  // Create force feedback interface.
        {
            m_haptic = SDL_HapticOpenFromJoystick(SDL_GameControllerGetJoystick(m_gameController));
            if(!m_haptic)
            {
                BOOST_LOG_TRIVIAL(error) << "Can't initialize haptic from game controller #" << controllerId;
            }
        }
    }
    else
    {
        SDL_JoystickEventState(SDL_ENABLE);                                 // If joystick isn't mapped, use generic API.
        m_joystick = SDL_JoystickOpen(m_joystickId);

        if(!m_joystick)
        {
            BOOST_LOG_TRIVIAL(error) << "Can't open joystick #" << controllerId;
            SDL_JoystickEventState(SDL_DISABLE);                            // If joystick init failed, close state.
            m_joystickId = -1;
        }
        else if(useHaptic)                                  // Create force feedback interface.
        {
            m_haptic = SDL_HapticOpenFromJoystick(m_joystick);
            if(!m_haptic)
            {
                BOOST_LOG_TRIVIAL(error) << "Can't initialize haptic from joystick #" << controllerId;
            }
        }
    }

    if(m_haptic)                                                          // To check if force feedback is working or not.
    {
        SDL_HapticRumbleInit(m_haptic);
        SDL_HapticRumblePlay(m_haptic, 1.0, 300);
    }
}

void InputHandler::poll()
{
    SDL_Event event;

    while(SDL_PollEvent(&event))
    {
        switch(event.type)
        {
            case SDL_MOUSEMOTION:
                m_onMouseMove(event.motion.xrel * m_mouseSensitivity * m_mouseScale.x, event.motion.yrel * m_mouseSensitivity * m_mouseScale.y);
                break;

            case SDL_MOUSEBUTTONDOWN:
                if(event.button.button == SDL_BUTTON_LEFT) //LM = 1, MM = 2, RM = 3
                {
                    primaryMouseDown();
                }
                else if(event.button.button == SDL_BUTTON_RIGHT)
                {
                    secondaryMouseDown();
                }
                break;

                // Controller events are only invoked when joystick is initialized as
                // game controller, otherwise, generic joystick event will be used.
            case SDL_CONTROLLERAXISMOTION:
                dispatchGameControllerAxis(event.caxis.axis, event.caxis.value);
                break;

            case SDL_CONTROLLERBUTTONDOWN:
            case SDL_CONTROLLERBUTTONUP:
                dispatchGameControllerKey(event.cbutton.button, event.cbutton.state == SDL_PRESSED);
                break;

                // Joystick events are still invoked, even if joystick is initialized as game
                // controller - that's why we need sdl_joystick checking - to filter out
                // duplicate event calls.

            case SDL_JOYAXISMOTION:
                dispatchJoystickAxis(event.jaxis.axis, event.jaxis.value);
                break;

            case SDL_JOYHATMOTION:
                dispatchJoystickHat(event.jhat.value);
                break;

            case SDL_JOYBUTTONDOWN:
            case SDL_JOYBUTTONUP:
                // NOTE: Joystick button numbers are passed with added JOY_BUTTON_MASK (1000).
                dispatchActionHandler(event.jbutton.button + JOY_BUTTON_MASK, event.jbutton.state == SDL_PRESSED);
                break;

            case SDL_TEXTINPUT:
            case SDL_TEXTEDITING:
                if(m_engine->m_gui.getConsole().isVisible() && event.key.state)
                {
                    m_engine->m_gui.getConsole().filter(event.text.text);
                    return;
                }
                break;

            case SDL_KEYUP:
            case SDL_KEYDOWN:
                if(event.key.keysym.sym == SDLK_F4 &&
                   event.key.state == SDL_PRESSED &&
                   event.key.keysym.mod & KMOD_ALT)
                {
                    m_engine->m_done = true;
                    break;
                }

                if(m_engine->m_gui.getConsole().isVisible() && event.key.state)
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
                            m_engine->m_gui.getConsole().edit(event.key.keysym.sym, event.key.keysym.mod);
                            break;
                        default:
                            break;
                    }
                    return;
                }
                else
                {
                    dispatchActionHandler(event.key.keysym.sym, event.key.state == SDL_PRESSED);
                    // DEBUG KEYBOARD COMMANDS
                    debugKeys(event.key.keysym.sym, event.key.state);
                }
                break;

            case SDL_QUIT:
                m_engine->m_done = true;
                break;

            case SDL_WINDOWEVENT:
                if(event.window.event == SDL_WINDOWEVENT_RESIZED)
                {
                    m_engine->resize(event.window.data1, event.window.data2, event.window.data1, event.window.data2);
                }
                break;

            default:
                break;
        }
    }

    m_joystickLookConfig.dispatch();
    m_joystickMoveConfig.dispatch();
}

void InputHandler::refreshStates()
{
    for(ActionState& action : m_actionStates | boost::adaptors::map_values)
    {
        action.wasActive = action.active;
    }
}

void InputHandler::rumble(float power, util::Duration time)
{
    if(m_haptic != nullptr)
        SDL_HapticRumblePlay(m_haptic, power, static_cast<int>(util::toSeconds(time) * 1000));
}

void InputHandler::dispatchActionHandler(int key, bool pressed)
{
    // Fill script-driven debug keyboard input.
    m_engine->m_scriptEngine.addKey(key, pressed);

    auto it1 = m_keyToAction.find(key);
    if(it1 == m_keyToAction.end())
        return; // key is not mapped

    auto it2 = m_actionHandlers.find(it1->second);
    if(it2 == m_actionHandlers.end())
        return; // no action handlers registered

    it2->second(pressed);
}

void InputHandler::dispatchJoystickHat(int value)
{
    // NOTE: Hat movements emulate keypresses
    // with HAT direction + JOY_HAT_MASK (1100) index.

    dispatchActionHandler(JOY_HAT_MASK + SDL_HAT_UP, false);     // Reset all directions.
    dispatchActionHandler(JOY_HAT_MASK + SDL_HAT_DOWN, false);
    dispatchActionHandler(JOY_HAT_MASK + SDL_HAT_LEFT, false);
    dispatchActionHandler(JOY_HAT_MASK + SDL_HAT_RIGHT, false);

    if(value & SDL_HAT_UP)
        dispatchActionHandler(JOY_HAT_MASK + SDL_HAT_UP, true);
    if(value & SDL_HAT_DOWN)
        dispatchActionHandler(JOY_HAT_MASK + SDL_HAT_DOWN, true);
    if(value & SDL_HAT_LEFT)
        dispatchActionHandler(JOY_HAT_MASK + SDL_HAT_LEFT, true);
    if(value & SDL_HAT_RIGHT)
        dispatchActionHandler(JOY_HAT_MASK + SDL_HAT_RIGHT, true);
}

void InputHandler::dispatchGameControllerKey(int button, bool pressed)
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
            dispatchActionHandler(JOY_HAT_MASK + SDL_HAT_UP, pressed);
            break;
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
            dispatchActionHandler(JOY_HAT_MASK + SDL_HAT_DOWN, pressed);
            break;
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
            dispatchActionHandler(JOY_HAT_MASK + SDL_HAT_LEFT, pressed);
            break;
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
            dispatchActionHandler(JOY_HAT_MASK + SDL_HAT_RIGHT, pressed);
            break;
        default:
            dispatchActionHandler(JOY_BUTTON_MASK + button, pressed);
            break;
    }
}

void InputHandler::dispatchJoystickAxis(int axis, int axisValue)
{
    m_joystickLookConfig.handle(axis, axisValue);
    m_joystickMoveConfig.handle(axis, axisValue);
}

void InputHandler::dispatchGameControllerAxis(int axis, int value)
{
    // Since left/right triggers on X360-like controllers are actually axes,
    // and we still need them as buttons, we remap these axes to button events.
    // Button event is invoked only if trigger is pressed more than 1/3 of its range.
    // Triggers are coded as native SDL2 enum number + JOY_TRIGGER_MASK (1200).

    if(axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT || axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT)
    {
        dispatchActionHandler(axis + JOY_TRIGGER_MASK, value >= JOY_TRIGGER_DEADZONE);
    }
    else
    {
        dispatchJoystickAxis(axis, value);
    }
}

void InputHandler::debugKeys(int button, int state)
{
    if(state == 0)
        return;

    switch(button)
    {
        case SDLK_RETURN:
            m_engine->m_world.m_character->inventory().send(InventoryManager::InventoryState::Activate);
            break;

        case SDLK_UP:
            m_engine->m_world.m_character->inventory().send(InventoryManager::InventoryState::Up);
            break;

        case SDLK_DOWN:
            m_engine->m_world.m_character->inventory().send(InventoryManager::InventoryState::Down);
            break;

        case SDLK_LEFT:
            m_engine->m_world.m_character->inventory().send(InventoryManager::InventoryState::RLeft);
            break;

        case SDLK_RIGHT:
            m_engine->m_world.m_character->inventory().send(InventoryManager::InventoryState::RRight);
            break;

        default:
            //Con_Printf("key = %d", button);
            break;
    };
}
}
