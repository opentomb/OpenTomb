
#ifndef ENGINE_H
#define ENGINE_H

#include <SDL2/SDL.h>
#include <stdint.h>
#include "bullet/btBulletDynamicsCommon.h"
#include "bullet/BulletCollision/CollisionDispatch/btCollisionWorld.h"
#include "world.h"
#include "script.h"
#include "controls.h"

#define LEVEL_NAME_MAX_LEN                      (64)
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

#define COLLISION_SHAPE_BOX                     0x0001
#define COLLISION_SHAPE_BOX_BASE                0x0002     // use single box collision
#define COLLISION_SHAPE_SPHERE                  0x0003
#define COLLISION_SHAPE_TRIMESH                 0x0004     // for static objects and room's!
#define COLLISION_SHAPE_TRIMESH_CONVEX          0x0005     // for dynamic objects

#define COLLISION_TYPE_NONE                     0x0000
#define COLLISION_TYPE_STATIC                   0x0001     // static object - never moved
#define COLLISION_TYPE_KINEMATIC                0x0003     // doors and other moveable statics
#define COLLISION_TYPE_DYNAMIC                  0x0005     // hellow full physics interaction
#define COLLISION_TYPE_ACTOR                    0x0007     // actor, enemies, NPC, animals
#define COLLISION_TYPE_VEHICLE                  0x0009     // car, moto, bike
#define COLLISION_TYPE_GHOST                    0x000B     // no fix character position, but works in collision callbacks and interacts with dynamic objects

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

struct camera_s;
struct lua_State;

typedef struct engine_container_s
{
    uint16_t                     object_type;
    uint16_t                     collision_type;
    uint16_t                     collision_shape;
    void                        *object;
    struct room_s               *room;
    struct engine_container_s   *next;
}engine_container_t, *engine_container_p;

typedef struct engine_control_state_s
{
    int8_t   free_look;
    btScalar free_look_speed;

    int8_t   mouse_look;
    btScalar cam_distance;
    int8_t   noclip;

    btScalar look_axis_x;                       // Unified look axis data.
    btScalar look_axis_y;

    int8_t   move_forward;                      // Directional movement keys.
    int8_t   move_backward;
    int8_t   move_left;
    int8_t   move_right;
    int8_t   move_up;                           // These are not typically used.
    int8_t   move_down;

    int8_t   look_up;                           // Look (camera) keys.
    int8_t   look_down;
    int8_t   look_left;
    int8_t   look_right;
    int8_t   look_roll_left;
    int8_t   look_roll_right;

    int8_t   do_jump;                              // Eventual actions.
    int8_t   do_draw_weapon;
    int8_t   do_roll;

    int8_t   state_action;                         // Conditional actions.
    int8_t   state_walk;
    int8_t   state_sprint;
    int8_t   state_crouch;
    int8_t   state_look;

    int8_t   use_flare;                            // Use item hotkeys.
    int8_t   use_big_medi;
    int8_t   use_small_medi;

    int8_t   use_prev_weapon;                      // Weapon hotkeys.
    int8_t   use_next_weapon;
    int8_t   use_weapon1;
    int8_t   use_weapon2;
    int8_t   use_weapon3;
    int8_t   use_weapon4;
    int8_t   use_weapon5;
    int8_t   use_weapon6;
    int8_t   use_weapon7;
    int8_t   use_weapon8;


    int8_t   gui_pause;                         // GUI keys - not sure if it must be here.
    int8_t   gui_inventory;

}engine_control_state_t, *engine_control_state_p;


extern struct engine_control_state_s            control_states;
extern struct control_settings_s                control_mapper;

extern struct audio_settings_s                  audio_settings;

extern btScalar                                 engine_frame_time;
extern struct camera_s                          engine_camera;
extern struct world_s                           engine_world;


extern btDefaultCollisionConfiguration         *bt_engine_collisionConfiguration;
extern btCollisionDispatcher                   *bt_engine_dispatcher;
extern btBroadphaseInterface                   *bt_engine_overlappingPairCache;
extern btSequentialImpulseConstraintSolver     *bt_engine_solver ;
extern btDiscreteDynamicsWorld                 *bt_engine_dynamicsWorld;



class bt_engine_ClosestRayResultCallback : public btCollisionWorld::ClosestRayResultCallback
{
public:
    bt_engine_ClosestRayResultCallback(engine_container_p cont, bool skip_ghost = false) : btCollisionWorld::ClosestRayResultCallback(btVector3(0.0, 0.0, 0.0), btVector3(0.0, 0.0, 0.0))
    {
        m_cont = cont;
        m_skip_ghost = skip_ghost;
    }

    virtual btScalar addSingleResult(btCollisionWorld::LocalRayResult& rayResult,bool normalInWorldSpace)
    {
        room_p r0 = NULL, r1 = NULL;
        engine_container_p c1;

        r0 = (m_cont)?(m_cont->room):(NULL);
        c1 = (engine_container_p)rayResult.m_collisionObject->getUserPointer();
        r1 = (c1)?(c1->room):(NULL);

        if(c1 && ((c1 == m_cont) || (m_skip_ghost && (c1->collision_type == COLLISION_TYPE_GHOST))))
        {
            return 1.0;
        }

        if(!r0 || !r1)
        {
            return ClosestRayResultCallback::addSingleResult(rayResult, normalInWorldSpace);
        }

        if(r0 && r1)
        {
            if(Room_IsInNearRoomsList(r0, r1))
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

    bool               m_skip_ghost;
    engine_container_p m_cont;
};


class bt_engine_ClosestConvexResultCallback : public btCollisionWorld::ClosestConvexResultCallback
{
public:
    bt_engine_ClosestConvexResultCallback(engine_container_p cont, bool skip_ghost = false) : btCollisionWorld::ClosestConvexResultCallback(btVector3(0.0, 0.0, 0.0), btVector3(0.0, 0.0, 0.0))
    {
        m_cont = cont;
        m_skip_ghost = skip_ghost;
    }

    virtual btScalar addSingleResult(btCollisionWorld::LocalConvexResult& convexResult,bool normalInWorldSpace)
    {
        room_p r0 = NULL, r1 = NULL;
        engine_container_p c1;

        r0 = (m_cont)?(m_cont->room):(NULL);
        c1 = (engine_container_p)convexResult.m_hitCollisionObject->getUserPointer();
        r1 = (c1)?(c1->room):(NULL);

        if(c1 && ((c1 == m_cont) || (m_skip_ghost && (c1->collision_type == COLLISION_TYPE_GHOST))))
        {
            return 1.0;
        }

        if(!r0 || !r1)
        {
            return ClosestConvexResultCallback::addSingleResult(convexResult, normalInWorldSpace);
        }

        if(r0 && r1)
        {
            if(Room_IsInNearRoomsList(r0, r1))
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
    bool               m_skip_ghost;
    engine_container_p m_cont;
};

extern "C" {
int engine_lua_fputs(const char *str, FILE *f);
int engine_lua_fprintf(FILE *f, const char *fmt, ...);
int engine_lua_printf(const char *fmt, ...);
}

engine_container_p Container_Create();

void Engine_Init_Pre();     // Initial init
void Engine_Init_Post();    // Finalizing init

void Engine_InitDefaultGlobals();

void Engine_Destroy();
void Engine_Shutdown(int val) __attribute__((noreturn));

void Engine_Frame(btScalar time);
void Engine_Display();

void Engine_BTInit();

int lua_print(lua_State * lua);

bool Engine_LuaInit();
void Engine_LuaClearTasks();
void Engine_LuaRegisterFuncs(lua_State *lua);

// Simple override to register both upper- and lowercase versions of function name.

void lua_registerc(lua_State *lua, const char* func_name, int(*func)(lua_State*));

// PC-specific level loader routines.

bool Engine_LoadPCLevel(const char *name);
int  Engine_GetPCLevelVersion(const char *name);

// General level loading routines.

bool Engine_FileFound(const char *name, bool Write = false);
int  Engine_GetLevelFormat(const char *name);
void Engine_GetLevelName(char *name, const char *path);
void Engine_GetLevelScriptName(int game_version, char *name, const char *postfix = NULL);
int  Engine_LoadMap(const char *name);

int  Engine_ExecCmd(char *ch);

void Engine_InitConfig(const char *filename);
void Engine_SaveConfig();

void Engine_RoomNearCallback(btBroadphasePair& collisionPair, btCollisionDispatcher& dispatcher, const btDispatcherInfo& dispatchInfo);
void Engine_InternalTickCallback(btDynamicsWorld *world, btScalar timeStep);


#endif
