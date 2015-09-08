#pragma once

#include <cstdio>

#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btCollisionWorld.h>

#include <SDL2/SDL.h>

#include "LuaState.h"

#include "controls.h"
#include "world/object.h"
#include "world/world.h"

class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;

namespace world
{
class Camera;
struct Room;
} // namespace world

namespace audio
{
extern Settings audio_settings;
} // namespace audio

namespace engine
{

#define MAX_ENGINE_PATH                         (1024)

#define LEVEL_FORMAT_PC         0
#define LEVEL_FORMAT_PSX        1
#define LEVEL_FORMAT_DC         2
#define LEVEL_FORMAT_OPENTOMB   3   // Maybe some day...

#define OBJECT_STATIC_MESH                      (0x0001)
#define OBJECT_ROOM_BASE                        (0x0002)
#define OBJECT_ENTITY                           (0x0003)
#define OBJECT_HAIR                             (0x0004)
#define OBJECT_BULLET_MISC                      (0x7FFF)

#define COLLISION_SHAPE_BOX                     (0x0001)
#define COLLISION_SHAPE_BOX_BASE                (0x0002)     // use single box collision
#define COLLISION_SHAPE_SPHERE                  (0x0003)
#define COLLISION_SHAPE_TRIMESH                 (0x0004)     // for static objects and room's!
#define COLLISION_SHAPE_TRIMESH_CONVEX          (0x0005)     // for dynamic objects

#define COLLISION_TYPE_NONE                     (0x0000)
#define COLLISION_TYPE_STATIC                   (0x0001)     // static object - never moved
#define COLLISION_TYPE_KINEMATIC                (0x0003)     // doors and other moveable statics
#define COLLISION_TYPE_DYNAMIC                  (0x0005)     // hellow full physics interaction
#define COLLISION_TYPE_ACTOR                    (0x0007)     // actor, enemies, NPC, animals
#define COLLISION_TYPE_VEHICLE                  (0x0009)     // car, moto, bike
#define COLLISION_TYPE_GHOST                    (0x000B)     // no fix character position, but works in collision callbacks and interacts with dynamic objects

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
#define COLLISION_GHOST_VOLUME_COEFFICIENT      (0.4f)
#define COLLISION_CAMERA_SPHERE_RADIUS          (16.0f)
#define COLLISION_TRAVERSE_TEST_RADIUS          (0.48f)

struct EngineContainer
{
    uint16_t object_type = 0;
    lua::Integer collision_type = COLLISION_TYPE_NONE;
    lua::Integer collision_shape = 0;
    world::Object* object = nullptr;
    world::Room* room = nullptr;
};

struct EngineControlState
{
    bool     free_look = false;
    btScalar free_look_speed = 0;

    bool     mouse_look = false;
    btScalar cam_distance = 800;
    bool     noclip = false;

    btScalar look_axis_x = 0;                       // Unified look axis data.
    btScalar look_axis_y = 0;

    bool     move_forward = false;                      // Directional movement keys.
    bool     move_backward = false;
    bool     move_left = false;
    bool     move_right = false;
    bool     move_up = false;                           // These are not typically used.
    bool     move_down = false;

    bool     look_up = false;                           // Look (camera) keys.
    bool     look_down = false;
    bool     look_left = false;
    bool     look_right = false;
    bool     look_roll_left = false;
    bool     look_roll_right = false;

    bool     do_jump = false;                          // Eventual actions.
    bool     do_draw_weapon = false;
    bool     do_roll = false;

    bool     state_action = false;                         // Conditional actions.
    bool     state_walk = false;
    bool     state_sprint = false;
    bool     state_crouch = false;

    bool     use_big_medi = false;
    bool     use_small_medi = false;

    bool     gui_inventory = false;
};

extern EngineControlState            control_states;
extern ControlSettings                control_mapper;


extern btScalar                                 engine_frame_time;
extern world::Camera                            engine_camera;
extern world::World                             engine_world;

extern btDefaultCollisionConfiguration         *bt_engine_collisionConfiguration;
extern btCollisionDispatcher                   *bt_engine_dispatcher;
extern btBroadphaseInterface                   *bt_engine_overlappingPairCache;
extern btSequentialImpulseConstraintSolver     *bt_engine_solver;
extern btDiscreteDynamicsWorld                 *bt_engine_dynamicsWorld;

class BtEngineClosestRayResultCallback : public btCollisionWorld::ClosestRayResultCallback
{
public:
    BtEngineClosestRayResultCallback(std::shared_ptr<EngineContainer> cont, bool skipGhost = false)
        : btCollisionWorld::ClosestRayResultCallback(btVector3(0.0, 0.0, 0.0), btVector3(0.0, 0.0, 0.0))
        , m_container(cont)
        , m_skip_ghost(skipGhost)
    {
    }

    virtual btScalar addSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace) override;

    const std::shared_ptr<const EngineContainer> m_container;
    const bool m_skip_ghost;
};

class BtEngineClosestConvexResultCallback : public btCollisionWorld::ClosestConvexResultCallback
{
public:
    BtEngineClosestConvexResultCallback(std::shared_ptr<EngineContainer> cont, bool skipGhost = false)
        : btCollisionWorld::ClosestConvexResultCallback(btVector3(0.0, 0.0, 0.0), btVector3(0.0, 0.0, 0.0))
        , m_container(cont)
        , m_skip_ghost(skipGhost)
    {
    }

    virtual btScalar addSingleResult(btCollisionWorld::LocalConvexResult& convexResult, bool normalInWorldSpace) override;

protected:
    const std::shared_ptr<const EngineContainer> m_container;
    const bool m_skip_ghost;
};

// ? Are they used at all ?

int engine_lua_fputs(const char *str, FILE *f);
int engine_lua_fprintf(FILE *f, const char *fmt, ...);
int engine_lua_printf(const char *fmt, ...);

// Starter and destructor.

void start();
void destroy();
#ifdef __GNUC__
void shutdown(int val) __attribute__((noreturn));
#else
void shutdown(int val);
#endif

// Initializers

void initPre();     // Initial init
void initPost();    // Finalizing init

void initDefaultGlobals();

void initGL();
void initSDLControls();
void initSDLVideo();
void initSDLImage();
void initAL();
void initBullet();

// Config parser

void initConfig(const char *filename);

// Core system routines - display and tick.

void display();
void frame(btScalar time);

// Resize event.
// Nominal values are used e.g. to set the size for the console.
// pixel values are used for glViewport. Both will be the same on
// normal displays, but on retina displays or similar, pixels will be twice nominal (or more).

void resize(int nominalW, int nominalH, int pixelsW, int pixelsH);

// Debug functions.

void showDebugInfo();
void dumpRoom(world::Room* r);

// PC-specific level loader routines.

bool loadPCLevel(const std::string &name);

// General level loading routines.

bool fileExists(const std::string &name, bool Write = false);
int  getLevelFormat(const std::string &name);
int  loadMap(const std::string &name);

// String getters.

std::string getLevelName(const std::string &path);
std::string getAutoexecName(loader::Game game_version, const std::string &postfix = std::string());

// Console command parser.

int  execCmd(const char *ch);

// Bullet global methods.

void roomNearCallback(btBroadphasePair& collisionPair, btCollisionDispatcher& dispatcher, const btDispatcherInfo& dispatchInfo);
void internalTickCallback(btDynamicsWorld *world, btScalar timeStep);
void internalPreTickCallback(btDynamicsWorld *world, btScalar timeStep);

} // namespace engine
