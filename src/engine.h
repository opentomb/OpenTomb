
#ifndef ENGINE_H
#define ENGINE_H

#include <SDL2/SDL.h>
#include <cstdint>
#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/BulletCollision/CollisionDispatch/btCollisionWorld.h>
#include "world.h"
#include "script.h"
#include "controls.h"
#include "object.h"

#include <lua.hpp>
#include "LuaState.h"

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

class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;

struct Camera;
struct lua_State;

struct EngineContainer
{
    uint16_t object_type = 0;
    lua::Integer collision_type = COLLISION_TYPE_NONE;
    lua::Integer collision_shape = 0;
    Object* object = nullptr;
    Room* room = nullptr;
};

//! @todo Use bools where appropriate.
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

extern AudioSettings                  audio_settings;

extern btScalar                                 engine_frame_time;
extern Camera                          engine_camera;
extern World                           engine_world;


extern btDefaultCollisionConfiguration         *bt_engine_collisionConfiguration;
extern btCollisionDispatcher                   *bt_engine_dispatcher;
extern btBroadphaseInterface                   *bt_engine_overlappingPairCache;
extern btSequentialImpulseConstraintSolver     *bt_engine_solver ;
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

    virtual btScalar addSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace) override
    {
        const Room* r0 = m_container ? m_container->room : nullptr;
        const EngineContainer* c1 = (EngineContainer*)rayResult.m_collisionObject->getUserPointer();
        const Room* r1 = c1 ? c1->room : nullptr;

        if(c1 && ((c1 == m_container.get()) || (m_skip_ghost && (c1->collision_type == COLLISION_TYPE_GHOST))))
        {
            return 1.0;
        }

        if(!r0 || !r1)
        {
            return ClosestRayResultCallback::addSingleResult(rayResult, normalInWorldSpace);
        }

        if(r0 && r1)
        {
            if(r0->isInNearRoomsList(*r1))
            {
                return ClosestRayResultCallback::addSingleResult(rayResult, normalInWorldSpace);
            }
            else
            {
                return 1.0;
            }
        }

        return 1.0;
    }

    std::shared_ptr<EngineContainer> m_container;
    bool                             m_skip_ghost;
};


class BtEngineClosestConvexResultCallback : public btCollisionWorld::ClosestConvexResultCallback
{
public:
    BtEngineClosestConvexResultCallback(std::shared_ptr<EngineContainer> cont, bool skipGhost = false) : btCollisionWorld::ClosestConvexResultCallback(btVector3(0.0, 0.0, 0.0), btVector3(0.0, 0.0, 0.0))
    {
        m_container = cont;
        m_skip_ghost = skipGhost;
    }

    virtual btScalar addSingleResult(btCollisionWorld::LocalConvexResult& convexResult,bool normalInWorldSpace)
    {
        EngineContainer* c1;

        Room* r0 = m_container ? m_container->room : nullptr;
        c1 = (EngineContainer*)convexResult.m_hitCollisionObject->getUserPointer();
        Room* r1 = c1 ? c1->room : nullptr;

        if(c1 && ((c1 == m_container.get()) || (m_skip_ghost && (c1->collision_type == COLLISION_TYPE_GHOST))))
        {
            return 1.0;
        }

        if(!r0 || !r1)
        {
            return ClosestConvexResultCallback::addSingleResult(convexResult, normalInWorldSpace);
        }

        if(r0 && r1)
        {
            if(r0->isInNearRoomsList(*r1))
            {
                return ClosestConvexResultCallback::addSingleResult(convexResult, normalInWorldSpace);
            }
            else
            {
                return 1.0;
            }
        }

        return 1.0;
    }

protected:
    std::shared_ptr<EngineContainer> m_container;
    bool               m_skip_ghost;
};

int engine_lua_fputs(const char *str, FILE *f);
int engine_lua_fprintf(FILE *f, const char *fmt, ...);
int engine_lua_printf(const char *fmt, ...);

void Engine_Init_Pre();     // Initial init
void Engine_Init_Post();    // Finalizing init

void Engine_InitDefaultGlobals();

void Engine_Destroy();
void Engine_Shutdown(int val) __attribute__((noreturn));

void Engine_Frame(btScalar time);
void Engine_Display();

void Engine_BTInit();

int lua_print(lua_State *state);
void Engine_LuaInit();
void Engine_LuaClearTasks();
void Engine_LuaRegisterFuncs(lua::State &state);

// Simple override to register both upper- and lowercase versions of function name.

template<typename Function>
inline void lua_registerc(lua::State& state, const std::string& func_name, Function func)
{
    std::string uc, lc;
    for(char c : func_name)
    {
        lc += std::tolower(c);
        uc += std::toupper(c);
    }

    state.set(func_name.c_str(), func);
    state.set(lc.c_str(), func);
    state.set(uc.c_str(), func);
}

template<>
inline void lua_registerc(lua::State& state, const std::string& func_name, int (*func)(lua_State*))
{
    std::string uc, lc;
    for(char c : func_name)
    {
        lc += std::tolower(c);
        uc += std::toupper(c);
    }

    lua_register(state.getState(), func_name.c_str(), func);
    lua_register(state.getState(), lc.c_str(), func);
    lua_register(state.getState(), uc.c_str(), func);
}

// PC-specific level loader routines.

bool Engine_LoadPCLevel(const std::string &name);
int  Engine_GetPCLevelVersion(const std::string &name);

// General level loading routines.

bool Engine_FileFound(const std::string &name, bool Write = false);
int  Engine_GetLevelFormat(const std::string &name);
std::string Engine_GetLevelName(const std::string &path);
std::string Engine_GetLevelScriptName(int game_version, const std::string &postfix = NULL);
int  Engine_LoadMap(const std::string &name);

int  Engine_ExecCmd(const char *ch);

void Engine_InitConfig(const char *filename);
void Engine_SaveConfig();

void Engine_RoomNearCallback(btBroadphasePair& collisionPair, btCollisionDispatcher& dispatcher, const btDispatcherInfo& dispatchInfo);
void Engine_InternalTickCallback(btDynamicsWorld *world, btScalar timeStep);


#endif
