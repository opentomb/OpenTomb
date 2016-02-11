#pragma once

#include "gameflow.h"
#include "gui/gui.h"
#include "gui/textline.h"
#include "inputhandler.h"
#include "script/script.h"
#include "world/world.h"
#include "bullet.h"
#include "system.h"

#include <boost/property_tree/ptree.hpp>

namespace world
{
class Camera;
class Room;
} // namespace world

struct SDL_Window;

namespace engine
{
#define LEVEL_FORMAT_PC         0
#define LEVEL_FORMAT_PSX        1
#define LEVEL_FORMAT_DC         2
#define LEVEL_FORMAT_OPENTOMB   3   // Maybe some day...

#define COLLISION_NONE                          (0x0000)
#define COLLISION_MASK_ALL                      (0x7FFF)        // bullet uses signed short int for these flags!

#define COLLISION_GROUP_ALL                     (0x7FFF)
#define COLLISION_GROUP_STATIC                  (0x0001)        // room mesh, statics
#define COLLISION_GROUP_KINEMATIC               (0x0002)        // doors, blocks, static animated entityes
#define COLLISION_GROUP_CHARACTERS              (0x0004)        // Lara, enemies, friends, creatures
#define COLLISION_GROUP_BULLETS                 (0x0008)        // bullets, rockets, grenades, arrows...
#define COLLISION_GROUP_DYNAMICS                (0x0010)        // test balls, warious

#define COLLISION_MARGIN_DEFAULT                (2.0f)
#define COLLISION_MARGIN_RIGIDBODY              (0.5f)

struct EngineControlState
{
    bool     m_freeLook = false;
    float    m_freeLookSpeed = 0;

    bool     m_mouseLook = false;
    glm::float_t m_camDistance = 800;
    bool     m_noClip = false;

    float    m_lookAxisX = 0;                       // Unified look axis data.
    float    m_lookAxisY = 0;

    bool     m_moveForward = false;                      // Directional movement keys.
    bool     m_moveBackward = false;
    bool     m_moveLeft = false;
    bool     m_moveRight = false;
    bool     m_moveUp = false;                           // These are not typically used.
    bool     m_moveDown = false;

    bool     m_lookUp = false;                           // Look (camera) keys.
    bool     m_lookDown = false;
    bool     m_lookLeft = false;
    bool     m_lookRight = false;
    bool     m_lookRollLeft = false;
    bool     m_lookRollRight = false;

    bool     m_doJump = false;                          // Eventual actions.
    bool     m_doDrawWeapon = false;
    bool     m_doRoll = false;

    bool     m_stateAction = false;                         // Conditional actions.
    bool     m_stateWalk = false;
    bool     m_stateSprint = false;
    bool     m_stateCrouch = false;

    bool     m_useBigMedi = false;
    bool     m_useSmallMedi = false;

    bool     m_guiInventory = false;

    explicit EngineControlState(boost::property_tree::ptree& config)
    {
        m_freeLookSpeed = util::getSetting(config, "freeLookSpeed", 3000.0f);
        m_mouseLook = util::getSetting(config, "mouseLook", true);
        m_freeLook = util::getSetting(config, "freeLook", false);
        m_noClip = util::getSetting(config, "noClip", false);
        m_camDistance = util::getSetting(config, "camDistance", 800.0f);
    }
};

class Engine
{
    TRACK_LIFETIME();

    util::Duration m_frameTime = util::Duration(0);

public:
    explicit Engine(boost::property_tree::ptree& config);
    ~Engine();

    util::Duration getFrameTime() const noexcept
    {
        return m_frameTime;
    }

    util::FloatDuration getFrameTimeSecs() const noexcept
    {
        return util::toSeconds(m_frameTime);
    }

    void setFrameTime(util::Duration time)
    {
        BOOST_ASSERT(time.count() > 0);
        m_frameTime = time;
    }

    EngineControlState m_controlState;
    InputHandler m_inputHandler;

    render::Render renderer;
    render::RenderDebugDrawer debugDrawer;

    world::Camera m_camera;
    world::World m_world;
    gui::Gui m_gui;
    Gameflow m_gameflow;
    BulletEngine m_bullet;

    float m_timeScale = 1;
    bool m_done = false;
    ScreenInfo m_screenInfo;
    SystemSettings m_systemSettings;
    world::Object* m_lastObject = nullptr;

    audio::Engine m_audioEngine;

    script::MainEngine m_scriptEngine;

    // Initializers

    void initPre();     // Initial init
    void initPost();    // Finalizing init

    void registerInputHandlers();

    void initGL();
    void initSDLVideo();

    // Core system routines - display and tick.

    void display();
    void frame(util::Duration time);

    // Resize event.
    // Nominal values are used e.g. to set the size for the console.
    // pixel values are used for glViewport. Both will be the same on
    // normal displays, but on retina displays or similar, pixels will be twice nominal (or more).

    void resize(int nominalW, int nominalH, int pixelsW, int pixelsH);

    // Debug functions.

    void showDebugInfo();
    static void dumpRoom(const world::Room* r);

    // PC-specific level loader routines.

    bool loadPCLevel(const std::string &name);

    // General level loading routines.

    int  getLevelFormat(const std::string &name);
    bool loadMap(const std::string &name);

    // String getters.

    static std::string getLevelName(const std::string &path);
    std::string getAutoexecName(loader::Game game_version, const std::string &postfix = std::string());

    // Console command parser.

    int execCmd(const char *ch);

    SDL_Window* m_window = nullptr;
    void* m_glContext = nullptr;

    glm::vec3 m_lightPosition = { 255.0, 255.0, 8.0 };
    glm::vec3 m_castRay[2]{ {0,0,0}, {0,0,0} };

private:
    gui::TextLine system_fps;

    void sysInit();
    void sysDestroy();

    int fpsCycles = 0;
    util::Duration fpsTime = util::Duration(0);

    void fpsCycle(util::Duration time);
};
} // namespace engine
