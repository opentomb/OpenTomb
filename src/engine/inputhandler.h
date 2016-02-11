#pragma once

#include "util/helpers.h"

#include <glm/glm.hpp>

#include <boost/signals2/signal.hpp>

struct _SDL_Haptic;
struct _SDL_GameController;
struct _SDL_Joystick;

namespace engine
{
class Engine;

//! Action mapper index constants
//! @todo Expose to Lua and remove scripts/config/control_constants.lua
enum class InputAction
{
    // Movement directions
    Up,                     // 0
    Down,                   // 1
    Left,                   // 2
    Right,                  // 3
    // Functional keys
    Action,                 // 4
    Jump,                   // 5
    Roll,                   // 6
    DrawWeapon,             // 7
    Look,                   // 8
    Walk,                   // 9
    Sprint,                 // 10
    Crouch,                 // 11
    StepLeft,               // 12
    StepRight,              // 13
    // Free look keys
    LookUp,                 // 14
    LookDown,               // 15
    LookLeft,               // 16
    LookRight,              // 17
    // Weapon scroller
    NextWeapon,             // 18
    PrevWeapon,             // 19
    // Item hotkeys
    Flare,                  // 20
    BigMedi,                // 21
    SmallMedi,              // 22
    Weapon1,                // 23
    Weapon2,                // 24
    Weapon3,                // 25
    Weapon4,                // 26
    Weapon5,                // 27
    Weapon6,                // 28
    Weapon7,                // 29
    Weapon8,                // 30
    Weapon9,                // 31
    Weapon10,               // 32
    Binoculars,             // 33
    Pls,                    // 34 Not in original, reserved for future
    // Interface keys
    Pause,                  // 35
    Inventory,              // 36
    Diary,                  // 37 Not in original, reserved for future
    Map,                    // 38 Not in original, reserved for future
    LoadGame,               // 39
    SaveGame,               // 40
    // Service keys
    Console,                // 41
    Screenshot,             // 42
    Sentinel
};

constexpr int JOY_BUTTON_MASK = 1000;
constexpr int JOY_HAT_MASK = 1100;
constexpr int JOY_TRIGGER_MASK = 1200;

constexpr int JOY_TRIGGER_DEADZONE = 10000;

class InputHandler
{
    TRACK_LIFETIME();
public:
    explicit InputHandler(Engine* engine, boost::property_tree::ptree& config);

    ~InputHandler();

    void poll();

    using MovementHandler = void(float dx, float dy);
    void registerMouseMoveHandler(const std::function<MovementHandler>& handler)
    {
        m_onMouseMove.connect(handler);
    }
    void registerJoystickLookHandler(const std::function<MovementHandler>& handler)
    {
        m_joystickLookConfig.handler.connect(handler);
    }
    void registerJoystickMoveHandler(const std::function<MovementHandler>& handler)
    {
        m_joystickMoveConfig.handler.connect(handler);
    }

    using ActionHandler = void(bool pressed);
    void registerActionHandler(InputAction action, const std::function<ActionHandler>& handler)
    {
        m_actionHandlers[action].connect(handler);

        auto self = this;
        auto updateActionState = [self, action](bool pressed){
            self->m_actionStates[action].active = pressed;
        };
        m_actionHandlers[action].connect(updateActionState);
    }

    void bindKey(int key, InputAction action)
    {
        m_keyToAction[key] = action;
    }
    void unbindKey(int key)
    {
        m_keyToAction.erase(key);
    }
    void clearBindings()
    {
        m_keyToAction.clear();
    }
    void clearHandlers()
    {
        m_actionHandlers.clear();
    }

    struct ActionState
    {
        bool active = false;
        bool wasActive = false;
    };

    void refreshStates();

    const ActionState& getActionState(InputAction action)
    {
        return m_actionStates[action];
    }

    void configureControllers(int controllerId, bool useJoystick, bool useHaptic);

    void rumble(float power, util::Duration time);

private:
    struct JoystickAxisConfig
    {
        float sensitivity = 1.5f;
        int deadzone = 1500;
        bool invertX = false;
        bool invertY = false;
        int xAxis = 0;
        int yAxis = 0;
        glm::vec2 position = { 0, 0 };
        glm::vec2 prevPosition = { 0, 0 };

        boost::signals2::signal<MovementHandler> handler;

        bool handle(int axis, int axisValue)
        {
            bool handled = false;
            if(axis == xAxis)
            {
                handled |= handleImpl(position.x, axisValue, invertX);
            }
            else if(axis == yAxis)
            {
                handled |= handleImpl(position.y, axisValue, invertY);
            }
            return handled;
        }

        void dispatch()
        {
            if(position != prevPosition)
                handler(position.x, position.y);

            prevPosition = position;
        }

    private:
        bool handleImpl(glm::float_t& position, int axisValue, bool invert)
        {
            if(axisValue < -deadzone || axisValue > deadzone)
            {
                glm::float_t p = axisValue / (32767.0f / sensitivity); // 32767 is the max./min. axis value.
                if(invert)
                {
                    position = -p;
                }
                else
                {
                    position = p;
                }
                return true;
            }
            else
            {
                position = 0;
                return false;
            }
        }
    };

    void primaryMouseDown();
    void secondaryMouseDown();
    void dispatchActionHandler(int key, bool pressed);

    void dispatchJoystickHat(int value);
    void dispatchGameControllerKey(int button, bool pressed);
    void dispatchJoystickAxis(int axis, int axisValue);
    void dispatchGameControllerAxis(int axis, int value);

    void debugKeys(int button, int state);

    Engine* m_engine;

    float m_mouseSensitivity = 25.0f;
    glm::vec2 m_mouseScale = { 0.01f, 0.01f };

    JoystickAxisConfig m_joystickLookConfig;
    JoystickAxisConfig m_joystickMoveConfig;

    boost::signals2::signal<MovementHandler> m_onMouseMove;

    std::map<InputAction, boost::signals2::signal<ActionHandler>> m_actionHandlers;
    std::map<int, InputAction> m_keyToAction;
    std::map<InputAction, ActionState> m_actionStates;

    _SDL_Haptic* m_haptic = nullptr;
    _SDL_GameController* m_gameController = nullptr;
    _SDL_Joystick* m_joystick = nullptr;
    int m_joystickId = -1;
};
}
