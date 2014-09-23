
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_opengl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "bullet/btBulletCollisionCommon.h"
#include "bullet/btBulletDynamicsCommon.h"
#include "bullet/BulletCollision/CollisionDispatch/btGhostObject.h"

extern "C" {
#include "lua/lua.h"
#include "lua/lualib.h"
#include "lua/lauxlib.h"
#include "lua/lstate.h"
#include "al/AL/al.h"
#include "al/AL/alc.h"
}

#include "vt/vt_level.h"
#include "engine.h"
#include "vmath.h"
#include "controls.h"
#include "console.h"
#include "system.h"
#include "common.h"
#include "script.h"
#include "frustum.h"
#include "portal.h"
#include "render.h"
#include "game.h"
#include "world.h"
#include "camera.h"
#include "mesh.h"
#include "entity.h"
#include "resource.h"
#include "anim_state_control.h"
#include "gui.h"
#include "audio.h"
#include "character_controller.h"
#include "gameflow.h"

#define INIT_FRAME_VERTEX_BUF_SIZE              (1024 * 1024)

extern SDL_Window             *sdl_window;
extern SDL_GLContext           sdl_gl_context;
extern SDL_GameController     *sdl_controller;
extern SDL_Joystick           *sdl_joystick;
extern SDL_Haptic             *sdl_haptic;
extern ALCdevice              *al_device;

struct engine_control_state_s           control_states = {0};
struct control_settings_s               control_mapper = {0};
struct audio_settings_s                 audio_settings = {0};
btScalar                                engine_frame_time = 0.0;

struct camera_s                         engine_camera;
struct world_s                          engine_world;

static btScalar                        *frame_vertex_buffer = NULL;
static size_t                           frame_vertex_buffer_size = 0;
static size_t                           frame_vertex_buffer_size_left = 0;

lua_State                               *engine_lua = NULL;

btDefaultCollisionConfiguration         *bt_engine_collisionConfiguration;
btCollisionDispatcher                   *bt_engine_dispatcher;
btGhostPairCallback                     *bt_engine_ghostPairCallback;
btBroadphaseInterface                   *bt_engine_overlappingPairCache;
btSequentialImpulseConstraintSolver     *bt_engine_solver ;
btDiscreteDynamicsWorld                 *bt_engine_dynamicsWorld;
btOverlapFilterCallback                 *bt_engine_filterCallback;

void RoomNearCallback(btBroadphasePair& collisionPair, btCollisionDispatcher& dispatcher, const btDispatcherInfo& dispatchInfo);
void Engine_InternalTickCallback(btDynamicsWorld *world, btScalar timeStep);

/**
 * overlapping room collision filter
 */
void RoomNearCallback(btBroadphasePair& collisionPair, btCollisionDispatcher& dispatcher, const btDispatcherInfo& dispatchInfo)
{
    engine_container_p c0, c1;
    room_p r0 = NULL, r1 = NULL;

    c0 = (engine_container_p)((btCollisionObject*)collisionPair.m_pProxy0->m_clientObject)->getUserPointer();
    r0 = (c0)?(c0->room):(NULL);
    c1 = (engine_container_p)((btCollisionObject*)collisionPair.m_pProxy1->m_clientObject)->getUserPointer();
    r1 = (c1)?(c1->room):(NULL);

    if(c1 && c1 == c0)
    {
        return;                                                                 // No self interaction
    }

    if(!r0 && !r1)
    {
        dispatcher.defaultNearCallback(collisionPair, dispatcher, dispatchInfo);// Both are out of rooms
        return;
    }

    if(r0 && r1)
    {
        if(Room_IsInNearRoomsList(r0, r1))
        {
            dispatcher.defaultNearCallback(collisionPair, dispatcher, dispatchInfo);
            return;
        }
        else
        {
            return;
        }
    }
}

/**
 * update current room of object
 */
void Engine_InternalTickCallback(btDynamicsWorld *world, btScalar timeStep)
{
    int i;

    for(i=world->getNumCollisionObjects()-1;i>=0;i--)
    {
        btCollisionObject* obj = bt_engine_dynamicsWorld->getCollisionObjectArray()[i];
        btRigidBody* body = btRigidBody::upcast(obj);
        if (body && !body->isStaticObject() && body->getMotionState())
        {
            btTransform trans;
            body->getMotionState()->getWorldTransform(trans);
            engine_container_p cont = (engine_container_p)body->getUserPointer();
            if(cont /*&& cont->object_type == OBJECT_BULLET_MISC*/)
            {
                cont->room = Room_FindPosCogerrence(&engine_world, trans.getOrigin().m_floats, cont->room);
            }
        }
    }
}


btScalar *GetTempbtScalar(size_t size)
{
    btScalar *ret = NULL;

    if(frame_vertex_buffer_size_left >= size)
    {
        ret = frame_vertex_buffer + frame_vertex_buffer_size - frame_vertex_buffer_size_left;
        frame_vertex_buffer_size_left -= size;
    }
    else
    {
        frame_vertex_buffer_size_left = frame_vertex_buffer_size;       // glitch generator, but not crash
        ret = frame_vertex_buffer;
    }

    return ret;
}


void ReturnTempbtScalar(size_t size)
{
    if(frame_vertex_buffer_size_left + size <= frame_vertex_buffer_size)
    {
        frame_vertex_buffer_size_left += size;
    }
}


void ResetTempbtScalar()
{
    frame_vertex_buffer_size_left = frame_vertex_buffer_size;
}


void Engine_InitGlobals()
{
    Controls_InitGlobals();
    Game_InitGlobals();
    Render_InitGlobals();
    Audio_InitGlobals();
}


void Engine_Init()
{
    Con_Init();
    Gui_Init();

    frame_vertex_buffer = (btScalar*)malloc(sizeof(btScalar) * INIT_FRAME_VERTEX_BUF_SIZE);
    frame_vertex_buffer_size = INIT_FRAME_VERTEX_BUF_SIZE;
    frame_vertex_buffer_size_left = frame_vertex_buffer_size;

    Sys_Init();
    Com_Init();
    Render_Init();

    Cam_Init(&engine_camera);
    engine_camera.dist_near = 10.0;
    engine_camera.dist_far = 65536.0;
    engine_camera.pos[0] = 300.0;
    engine_camera.pos[1] = 850.0;
    engine_camera.pos[2] = 150.0;

    renderer.cam = &engine_camera;
    engine_camera.frustum->next = NULL;
    engine_camera.current_room = NULL;

    ///collision configuration contains default setup for memory, collision setup. Advanced users can create their own configuration.
    bt_engine_collisionConfiguration = new btDefaultCollisionConfiguration();

    ///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
    bt_engine_dispatcher = new btCollisionDispatcher(bt_engine_collisionConfiguration);
    bt_engine_dispatcher->setNearCallback(RoomNearCallback);
    ///btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
    bt_engine_overlappingPairCache = new btDbvtBroadphase();
    bt_engine_ghostPairCallback = new btGhostPairCallback();
    bt_engine_overlappingPairCache->getOverlappingPairCache()->setInternalGhostPairCallback(bt_engine_ghostPairCallback);
    ///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
    bt_engine_solver = new btSequentialImpulseConstraintSolver;

    bt_engine_dynamicsWorld = new btDiscreteDynamicsWorld(bt_engine_dispatcher, bt_engine_overlappingPairCache, bt_engine_solver, bt_engine_collisionConfiguration);
    bt_engine_dynamicsWorld->setInternalTickCallback(Engine_InternalTickCallback);
    bt_engine_dynamicsWorld->setGravity(btVector3(0, 0, -4500.0));
    //bt_engine_dynamicsWorld->getPairCache()->setInternalGhostPairCallback(bt_engine_filterCallback);

    engine_lua = luaL_newstate();
    if(engine_lua != NULL)
    {
        ///@FIXME: CRITICAL: ALL LUA's ACTIONS PREVENTS TO CRASH!!!
        luaL_openlibs(engine_lua);
        Engine_LuaRegisterFuncs(engine_lua);
    }

    CVAR_Register("game_level", "empty"); // Empty level name - jump right to the game sequence!
    CVAR_Register("engine_version", "1");
    CVAR_Register("time_scale", "1.0");

    Con_AddLine("Engine inited");
    luaL_dofile(engine_lua, "scripts/gameflow/gameflow.lua");
    luaL_dofile(engine_lua, "scripts/system/sys_scripts.lua");
}


int lua_SetModelCollisionMapSize(lua_State * lua)
{
    int size, id, top;
    top = lua_gettop(lua);                      ///@FIXME: top are not used!
    id = lua_tointeger(lua, 1);

    if(id < 0 || id > engine_world.skeletal_model_count - 1)
    {
        Con_Printf("there are not models with id = %d", id);
        return 0;
    }

    size = lua_tointeger(lua, 2);
    if(size >= 0 && size < engine_world.skeletal_models[id].mesh_count)
    {
        engine_world.skeletal_models[id].collision_map_size = size;
    }

    return 0;
}


int lua_SetModelCollisionMap(lua_State * lua)
{
    int arg, val, id, top;
    top = lua_gettop(lua);                      ///@FIXME: top are not used!
    id = lua_tointeger(lua, 1);

    if(id < 0 || id > engine_world.skeletal_model_count - 1)
    {
        Con_Printf("there are not models with id = %d", id);
        return 0;
    }

    arg = lua_tointeger(lua, 2);
    val = lua_tointeger(lua, 3);
    if(arg >= 0 && arg < engine_world.skeletal_models[id].mesh_count)
    {
        engine_world.skeletal_models[id].collision_map[arg] = val;
    }

    return 0;
}


int lua_EnableEntity(lua_State * lua)
{
    int num = lua_gettop(lua);                                                  // get # of arguments
    entity_p ent;

    if(num < 1)
    {
        Con_AddLine("You must to enter entity ID");
        return 0;
    }

    ent = World_GetEntityByID(&engine_world, lua_tonumber(lua, 1));
    if(ent)
    {
        Entity_Enable(ent);
    }

    return 0;
}


int lua_DisableEntity(lua_State * lua)
{
    int num = lua_gettop(lua);                                                  // get # of arguments
    entity_p ent;

    if(num < 1)
    {
        Con_AddLine("You must to enter entity ID");
        return 0;
    }

    ent = World_GetEntityByID(&engine_world, lua_tonumber(lua, 1));
    if(ent)
    {
        Entity_Disable(ent);
    }

    return 0;
}


int lua_SetGravity(lua_State * lua)                                             // function to be exported to Lua
{
    int num = lua_gettop(lua);                                                  // get # of arguments
    btVector3 g;

    switch(num)
    {
        case 0:                                                                 // get value
            g = bt_engine_dynamicsWorld->getGravity();
            Con_Printf("gravity = (%.3f, %.3f, %.3f)", g.m_floats[0], g.m_floats[1], g.m_floats[2]);
            break;

        case 1:                                                                 // set z only value
            g.m_floats[0] = 0.0;
            g.m_floats[1] = 0.0;
            g.m_floats[2] = lua_tonumber(lua, 1);
            bt_engine_dynamicsWorld->setGravity(g);
            Con_Printf("gravity = (%.3f, %.3f, %.3f)", g.m_floats[0], g.m_floats[1], g.m_floats[2]);
            break;

        case 3:                                                                 // set xyz value
            g.m_floats[0] = lua_tonumber(lua, 1);
            g.m_floats[1] = lua_tonumber(lua, 2);
            g.m_floats[2] = lua_tonumber(lua, 3);
            bt_engine_dynamicsWorld->setGravity(g);
            Con_Printf("gravity = (%.3f, %.3f, %.3f)", g.m_floats[0], g.m_floats[1], g.m_floats[2]);
            break;

        default:
            Con_Printf("wrong arguments number, must be 0 or 1 or 3");
            break;
    };

    return 0;                                                                   // we returned two vaues
}


int lua_GetModelID(lua_State * lua)
{
    int id, top;
    entity_p ent;
    top = lua_gettop(lua);              ///@FIXME: top are not used!
    id = lua_tointeger(lua, 1);

    ent = World_GetEntityByID(&engine_world, id);
    if(ent == NULL)
    {
        Con_Printf("can not find entity with id = %d", id);
        return 0;
    }

    if(ent->model)
    {
        lua_pushinteger(lua, ent->model->ID);
        return 1;
    }
    return 0;
}


int lua_GetActivationOffset(lua_State * lua)
{
    int id, top;
    entity_p ent;
    top = lua_gettop(lua);                  ///@FIXME: top are not used!
    id = lua_tointeger(lua, 1);

    ent = World_GetEntityByID(&engine_world, id);
    if(ent == NULL)
    {
        Con_Printf("can not find entity with id = %d", id);
        return 0;
    }

    lua_pushnumber(lua, ent->activation_offset[0]);
    lua_pushnumber(lua, ent->activation_offset[1]);
    lua_pushnumber(lua, ent->activation_offset[2]);
    lua_pushnumber(lua, ent->activation_offset[3]);

    return 4;
}


int lua_SetActivationOffset(lua_State * lua)
{
    int id, top;
    entity_p ent;
    top = lua_gettop(lua);
    id = lua_tointeger(lua, 1);

    ent = World_GetEntityByID(&engine_world, id);
    if(ent == NULL)
    {
        Con_Printf("can not find entity with id = %d", id);
        return 0;
    }

    if(top >= 4)
    {
        ent->activation_offset[0] = lua_tonumber(lua, 2);
        ent->activation_offset[1] = lua_tonumber(lua, 3);
        ent->activation_offset[2] = lua_tonumber(lua, 4);
    }
    if(top >= 5)
    {
        ent->activation_offset[3] = lua_tonumber(lua, 5);
    }

    return 0;
}


int lua_BindKey(lua_State * lua)
{
    int act, top;

    top = lua_gettop(lua);
    act = lua_tointeger(lua, 1);
    if(top < 1 || act < 0 || act >= ACT_LASTINDEX)
    {
        Con_Printf("wrong action number");
        return 0;
    }

    else if(top == 2)
    {
        control_mapper.action_map[act].primary = lua_tointeger(lua, 2);
        return 0;
    }
    else if(top == 3)
    {
        control_mapper.action_map[act].primary   = lua_tointeger(lua, 2);
        control_mapper.action_map[act].secondary = lua_tointeger(lua, 3);
        return 0;
    }

    Con_Printf("wrong arguments number, must be 2 or 3");
    return 0;
}


int lua_AddItem(lua_State * lua)
{
    int top, entity_id, item_id, count;
    top = lua_gettop(lua);

    if(top < 3)
    {
        Con_Printf("Wrong arguments count. Must be (entity_id, item_id, items_count)");
        return 0;
    }
    entity_id = lua_tointeger(lua, 1);
    item_id = lua_tointeger(lua, 2);
    count = lua_tointeger(lua, 3);

    entity_p ent = World_GetEntityByID(&engine_world, entity_id);
    if(ent == NULL)
    {
        Con_Printf("can not find entity with id = %d", entity_id);
        return 0;
    }

    lua_pushinteger(lua, Character_AddItem(ent, item_id, count));
    return 1;
}


int lua_RemoveItem(lua_State * lua)
{
    int top, entity_id, item_id, count;
    top = lua_gettop(lua);

    if(top < 3)
    {
        Con_Printf("Wrong arguments count. Must be (entity_id, item_id, items_count)");
        return 0;
    }
    entity_id = lua_tointeger(lua, 1);
    item_id = lua_tointeger(lua, 2);
    count = lua_tointeger(lua, 3);

    entity_p ent = World_GetEntityByID(&engine_world, entity_id);
    if(ent == NULL)
    {
        Con_Printf("can not find entity with id = %d", entity_id);
        return 0;
    }

    lua_pushinteger(lua, Character_RemoveItem(ent, item_id, count));
    return 1;
}


int lua_PrintItems(lua_State * lua)
{
    int top, entity_id;
    top = lua_gettop(lua);

    if(top < 1)
    {
        Con_Printf("Wrong arguments count. Must be (entity_id)");
        return 0;
    }
    entity_id = lua_tointeger(lua, 1);

    entity_p ent = World_GetEntityByID(&engine_world, entity_id);
    if(ent == NULL)
    {
        Con_Printf("can not find entity with id = %d", entity_id);
        return 0;
    }

    if(ent->character)
    {
        inventory_node_p i = ent->character->inventory;
        for(;i;i=i->next)
        {
            Con_Printf("item[id = %d]: count = %d, type = %d", i->id, i->count, i->type);
        }
    }
    return 0;
}


int lua_GetItemsCount(lua_State * lua)
{
    int top, entity_id, item_id;
    top = lua_gettop(lua);

    if(top < 2)
    {
        Con_Printf("Wrong arguments count. Must be (entity_id, item_id)");
        return 0;
    }
    entity_id = lua_tointeger(lua, 1);
    item_id = lua_tointeger(lua, 2);

    entity_p ent = World_GetEntityByID(&engine_world, entity_id);
    if(ent == NULL)
    {
        Con_Printf("can not find entity with id = %d", entity_id);
        return 0;
    }

    lua_pushinteger(lua, Character_GetItemsCount(ent, item_id));
    return 1;
}


int lua_SetStateChangeRange(lua_State * lua)
{
    int top, id, anim, state, dispath, frame_low, frame_high;
    top = lua_gettop(lua);

    if(top < 6)
    {
        Con_Printf("Wrong arguments count. Must be (model_id, anim_num, state_id, dispath_num, start_frame, end_frame (, next_anim, next_frame)");
        return 0;
    }

    id = lua_tointeger(lua, 1);
    anim = lua_tointeger(lua, 2);
    state = lua_tointeger(lua, 3);
    dispath = lua_tointeger(lua, 4);
    frame_low = lua_tointeger(lua, 5);
    frame_high = lua_tointeger(lua, 6);

    skeletal_model_p model = World_FindModelByID(&engine_world, id);
    if(model == NULL)
    {
        Con_Printf("can not find skeletal model with id = %d", id);
        return 0;
    }

    if((anim < 0) || (anim + 1 > model->animation_count))
    {
        Con_Printf("wrong anim number");
        return 0;
    }

    animation_frame_p af = model->animations + anim;
    for(int i=0;i<af->state_change_count;i++)
    {
        if(af->state_change[i].ID == state)
        {
            if((dispath >= 0) && (dispath < af->state_change[i].anim_dispath_count))
            {
                af->state_change[i].anim_dispath[dispath].frame_low = frame_low;
                af->state_change[i].anim_dispath[dispath].frame_high = frame_high;
                if(top >= 8)
                {
                    af->state_change[i].anim_dispath[dispath].next_anim = lua_tointeger(lua, 7);
                    af->state_change[i].anim_dispath[dispath].next_frame = lua_tointeger(lua, 8);
                }
            }
            else
            {
                Con_Printf("wrong dispath number");
            }
            break;
        }
    }

    return 0;
}


int lua_GetAnimCommandTransform(lua_State * lua)
{
    int top, id, anim, frame;
    top = lua_gettop(lua);

    if(top < 3)
    {
        Con_Printf("Wrong arguments count. Must be (model_id, anim_num, frame_num");
        return 0;
    }
    id = lua_tointeger(lua, 1);
    anim = lua_tointeger(lua, 2);
    frame = lua_tointeger(lua, 3);

    skeletal_model_p model = World_FindModelByID(&engine_world, id);
    if(model == NULL)
    {
        Con_Printf("can not find skeletal model with id = %d", id);
        return 0;
    }

    if((anim < 0) || (anim + 1 > model->animation_count))
    {
        Con_Printf("wrong anim number");
        return 0;
    }

    if(frame < 0)                                                               // it is convenient to use -1 as a last frame number
    {
        frame = (int)model->animations[anim].frames_count + frame;
    }

    if((frame < 0) || (frame + 1 > model->animations[anim].frames_count))
    {
        Con_Printf("wrong frame number");
        return 0;
    }

    lua_pushinteger(lua, model->animations[anim].frames[frame].command);
    lua_pushnumber(lua, model->animations[anim].frames[frame].move[0]);
    lua_pushnumber(lua, model->animations[anim].frames[frame].move[1]);
    lua_pushnumber(lua, model->animations[anim].frames[frame].move[2]);

    return 4;
}


int lua_SetAnimCommandTransform(lua_State * lua)
{
    int top, id, anim, frame;
    top = lua_gettop(lua);

    if(top < 4)
    {
        Con_Printf("Wrong arguments count. Must be (model_id, anim_num, frame_num, flag, (option: dx, dy, dz))");
        return 0;
    }
    id = lua_tointeger(lua, 1);
    anim = lua_tointeger(lua, 2);
    frame = lua_tointeger(lua, 3);

    skeletal_model_p model = World_FindModelByID(&engine_world, id);
    if(model == NULL)
    {
        Con_Printf("can not find skeletal model with id = %d", id);
        return 0;
    }

    if((anim < 0) || (anim + 1 > model->animation_count))
    {
        Con_Printf("wrong anim number");
        return 0;
    }

    if(frame < 0)                                                               // it is convenient to use -1 as a last frame number
    {
        frame = (int)model->animations[anim].frames_count + frame;
    }

    if((frame < 0) || (frame + 1 > model->animations[anim].frames_count))
    {
        Con_Printf("wrong frame number");
        return 0;
    }

    model->animations[anim].frames[frame].command = 0x00ff & lua_tointeger(lua, 4);
    if(top >= 7)
    {
        model->animations[anim].frames[frame].move[0] = lua_tonumber(lua, 5);
        model->animations[anim].frames[frame].move[1] = lua_tonumber(lua, 6);
        model->animations[anim].frames[frame].move[2] = lua_tonumber(lua, 7);
    }

    return 0;
}

/*
 * Moveables script control section
 */
int lua_GetEntityVector(lua_State * lua)
{
    int id, top;
    entity_p e1, e2;

    top = lua_gettop(lua);
    if(top < 2)
    {
        Con_Printf("Wrong arguments count. Must be (id1, id2)");
        return 0;
    }
    id = lua_tointeger(lua, 1);
    e1 = World_GetEntityByID(&engine_world, id);
    if(e1 == NULL)
    {
        Con_Printf("can not find entity with id = %d", id);
        return 0;
    }
    id = lua_tointeger(lua, 2);
    e2 = World_GetEntityByID(&engine_world, id);
    if(e2 == NULL)
    {
        Con_Printf("can not find entity with id = %d", id);
        return 0;
    }

    lua_pushnumber(lua, e2->transform[12+0] - e1->transform[12+0]);
    lua_pushnumber(lua, e2->transform[12+1] - e1->transform[12+1]);
    lua_pushnumber(lua, e2->transform[12+2] - e1->transform[12+2]);
    return 3;
}


int lua_GetEntityPosition(lua_State * lua)
{
    int id, top;
    entity_p ent;
    top = lua_gettop(lua);
    id = lua_tointeger(lua, 1);
    if(top != 1)
    {
        Con_Printf("Wrong arguments count. Must be (id)");
        return 0;
    }
    ent = World_GetEntityByID(&engine_world, id);

    if(ent == NULL)
    {
        Con_Printf("can not find entity with id = %d", id);
        return 0;
    }

    lua_pushnumber(lua, ent->transform[12+0]);
    lua_pushnumber(lua, ent->transform[12+1]);
    lua_pushnumber(lua, ent->transform[12+2]);
    lua_pushnumber(lua, ent->angles[0]);
    lua_pushnumber(lua, ent->angles[1]);
    lua_pushnumber(lua, ent->angles[2]);
    return 6;
}


int lua_SetEntityPosition(lua_State * lua)
{
    int id, top;
    entity_p ent;
    top = lua_gettop(lua);
    id = lua_tointeger(lua, 1);

    ent = World_GetEntityByID(&engine_world, id);
    if(ent == NULL)
    {
        Con_Printf("can not find entity with id = %d", id);
        return 0;
    }

    switch(top)
    {
        case 4:
            ent->transform[12+0] = lua_tonumber(lua, 2);
            ent->transform[12+1] = lua_tonumber(lua, 3);
            ent->transform[12+2] = lua_tonumber(lua, 4);
            if(ent->character)
            {
                Character_UpdatePlatformPreStep(ent);
            }
            return 0;

        case 7:
            ent->transform[12+0] = lua_tonumber(lua, 2);
            ent->transform[12+1] = lua_tonumber(lua, 3);
            ent->transform[12+2] = lua_tonumber(lua, 4);
            ent->angles[0] = lua_tonumber(lua, 5);
            ent->angles[1] = lua_tonumber(lua, 6);
            ent->angles[2] = lua_tonumber(lua, 7);
            Entity_UpdateRotation(ent);
            if(ent->character)
            {
                Character_UpdatePlatformPreStep(ent);
            }
            return 0;

        default:
            Con_Printf("Wrong arguments count. Must be (id, x, y, z) or (id, x, y, z, fi_x, fi_y, fi_z)");
            return 0;
    }

    return 0;
}


int lua_MoveEntityGlobal(lua_State * lua)
{
    int id, top;
    entity_p ent;
    top = lua_gettop(lua);
    id = lua_tointeger(lua, 1);

    ent = World_GetEntityByID(&engine_world, id);
    if(ent == NULL)
    {
        Con_Printf("can not find entity with id = %d", id);
        return 0;
    }

    switch(top)
    {
        case 4:
            ent->transform[12+0] += lua_tonumber(lua, 2);
            ent->transform[12+1] += lua_tonumber(lua, 3);
            ent->transform[12+2] += lua_tonumber(lua, 4);
            return 0;

        default:
            Con_Printf("Wrong arguments count. Must be (id, x, y, z)");
            return 0;
    }

    return 0;
}


int lua_MoveEntityLocal(lua_State * lua)
{
    int id, top;
    entity_p ent;
    btScalar dx, dy, dz;
    top = lua_gettop(lua);                      ///@FIXME: top are not used!
    id = lua_tointeger(lua, 1);

    ent = World_GetEntityByID(&engine_world, id);
    if(ent == NULL)
    {
        Con_Printf("can not find entity with id = %d", id);
        return 0;
    }

    dx = lua_tonumber(lua, 2);
    dy = lua_tonumber(lua, 3);
    dz = lua_tonumber(lua, 4);

    ent->transform[12+0] += dx * ent->transform[0+0] + dy * ent->transform[4+0] + dz * ent->transform[8+0];
    ent->transform[12+1] += dx * ent->transform[0+1] + dy * ent->transform[4+1] + dz * ent->transform[8+1];
    ent->transform[12+2] += dx * ent->transform[0+2] + dy * ent->transform[4+2] + dz * ent->transform[8+2];

    return 0;
}


int lua_GetEntitySpeed(lua_State * lua)
{
    int id, top;
    entity_p ent;
    top = lua_gettop(lua);
    id = lua_tointeger(lua, 1);
    if(top != 1)
    {
        Con_Printf("Wrong arguments count. Must be (id)");
        return 0;
    }
    ent = World_GetEntityByID(&engine_world, id);

    if(ent == NULL)
    {
        Con_Printf("can not find entity with id = %d", id);
        return 0;
    }

    lua_pushnumber(lua, ent->speed[0]);
    lua_pushnumber(lua, ent->speed[1]);
    lua_pushnumber(lua, ent->speed[2]);
    return 3;
}


int lua_SetEntitySpeed(lua_State * lua)
{
    int id, top;
    entity_p ent;
    top = lua_gettop(lua);
    id = lua_tointeger(lua, 1);

    ent = World_GetEntityByID(&engine_world, id);
    if(ent == NULL)
    {
        Con_Printf("can not find entity with id = %d", id);
        return 0;
    }

    switch(top)
    {
        case 4:
            ent->speed[0] = lua_tonumber(lua, 2);
            ent->speed[1] = lua_tonumber(lua, 3);
            ent->speed[2] = lua_tonumber(lua, 4);
            return 0;

        default:
            Con_Printf("Wrong arguments count. Must be (id, Vx, Vy, Vz)");
            return 0;
    }

    return 0;
}


int lua_SetEntityAnim(lua_State * lua)
{
    int id, top;
    entity_p ent;
    top = lua_gettop(lua);                          ///@FIXME: top are not used!
    id = lua_tointeger(lua, 1);

    ent = World_GetEntityByID(&engine_world, id);
    if(ent == NULL)
    {
        Con_Printf("can not find entity with id = %d", id);
        return 0;
    }

    Entity_SetAnimation(ent, lua_tointeger(lua, 2), lua_tointeger(lua, 3));

    return 0;
}


int lua_GetEntityAnim(lua_State * lua)
{
    int id, top;
    entity_p ent;
    top = lua_gettop(lua);                          ///@FIXME: top are not used!
    id = lua_tointeger(lua, 1);

    ent = World_GetEntityByID(&engine_world, id);
    if(ent == NULL)
    {
        Con_Printf("can not find entity with id = %d", id);
        return 0;
    }

    lua_pushinteger(lua, ent->current_animation);
    lua_pushinteger(lua, ent->current_frame);
    lua_pushinteger(lua, ent->model->animations[ent->current_animation].frames_count);

    return 3;
}


int lua_CanTriggerEntity(lua_State * lua)
{
    int id, top;
    entity_p e1, e2;
    btScalar pos[3], offset[3], r;
    top = lua_gettop(lua);

    if(top < 2)
    {
        lua_pushinteger(lua, 0);
        return 1;
    }

    id = lua_tointeger(lua, 1);
    e1 = World_GetEntityByID(&engine_world, id);
    if(e1 == NULL || !e1->character || !e1->character->cmd.action)
    {
        lua_pushinteger(lua, 0);
        return 1;
    }
    id = lua_tointeger(lua, 2);
    e2 = World_GetEntityByID(&engine_world, id);
    if((e2 == NULL) || (e1 == e2))
    {
        lua_pushinteger(lua, 0);
        return 1;
    }

    r = e2->activation_offset[3];
    if(top >= 3)
    {
        r = lua_tonumber(lua, 3);
    }
    r *= r;
    vec3_copy(offset, e2->activation_offset);
    if(top >= 4)
    {
        offset[0] = lua_tonumber(lua, 4);
        offset[1] = lua_tonumber(lua, 5);
        offset[2] = lua_tonumber(lua, 6);
    }

    Mat4_vec3_mul_macro(pos, e2->transform, offset);
    if((vec3_dot(e1->transform+4, e2->transform+4) > 0.75) &&
       (vec3_dist_sq(e1->transform+12, pos) < r))
    {
        lua_pushinteger(lua, 1);
        return 1;
    }

    lua_pushinteger(lua, 0);
    return 1;
}


int lua_SetEntityVisibility(lua_State * lua)
{
    int id, top;
    entity_p ent;
    top = lua_gettop(lua);                              ///@FIXME: top are not used!
    id = lua_tointeger(lua, 1);

    ent = World_GetEntityByID(&engine_world, id);
    if(ent == NULL)
    {
        Con_Printf("can not find entity with id = %d", id);
        return 0;
    }

    ent->hide = !lua_tointeger(lua, 2);

    return 0;
}


int lua_GetEntityActivity(lua_State * lua)
{
    int id, top;
    entity_p ent;
    top = lua_gettop(lua);                              ///@FIXME: top are not used!
    id = lua_tointeger(lua, 1);

    ent = World_GetEntityByID(&engine_world, id);
    if(ent == NULL)
    {
        Con_Printf("can not find entity with id = %d", id);
        return 0;
    }

    lua_pushinteger(lua, ent->active);

    return 1;
}


int lua_SetEntityActivity(lua_State * lua)
{
    int id, top;
    entity_p ent;
    top = lua_gettop(lua);              ///@FIXME: top are not used!
    id = lua_tointeger(lua, 1);

    ent = World_GetEntityByID(&engine_world, id);
    if(ent == NULL)
    {
        Con_Printf("can not find entity with id = %d", id);
        return 0;
    }

    ent->active = lua_tointeger(lua, 2);

    return 0;
}


int lua_GetEntityOCB(lua_State * lua)
{
    int id, top;
    entity_p ent;
    top = lua_gettop(lua);                      ///@FIXME: top are not used!
    id = lua_tointeger(lua, 1);

    ent = World_GetEntityByID(&engine_world, id);
    if(ent == NULL)
    {
        Con_Printf("can not find entity with id = %d", id);
        return 0;
    }

    lua_pushinteger(lua, ent->OCB);

    return 1;
}


int lua_GetEntityFlag(lua_State * lua)
{
    int id, top;
    entity_p ent;
    top = lua_gettop(lua);                              ///@FIXME: top are not used!
    id = lua_tointeger(lua, 1);

    ent = World_GetEntityByID(&engine_world, id);
    if(ent == NULL)
    {
        Con_Printf("can not find entity with id = %d", id);
        return 0;
    }

    lua_pushinteger(lua, ent->flags);

    return 1;
}


int lua_SetEntityFlag(lua_State * lua)
{
    int id, top;
    entity_p ent;
    top = lua_gettop(lua);                  ///@FIXME: top are not used!
    id = lua_tointeger(lua, 1);

    ent = World_GetEntityByID(&engine_world, id);
    if(ent == NULL)
    {
        Con_Printf("can not find entity with id = %d", id);
        return 0;
    }

    ent->flags = lua_tointeger(lua, 2);

    return 0;
}

int lua_GetEntityActivationMask(lua_State * lua)
{
    int id, top;
    entity_p ent;
    top = lua_gettop(lua);                  ///@FIXME: top are not used!
    id = lua_tointeger(lua, 1);

    ent = World_GetEntityByID(&engine_world, id);
    if(ent == NULL)
    {
        Con_Printf("can not find entity with id = %d", id);
        return 0;
    }

    lua_pushinteger(lua, ent->activation_mask);

    return 1;
}


int lua_SetEntityActivationMask(lua_State * lua)
{
    int id, top;
    entity_p ent;
    top = lua_gettop(lua);                      ///@FIXME: top are not used!
    id = lua_tointeger(lua, 1);

    ent = World_GetEntityByID(&engine_world, id);
    if(ent == NULL)
    {
        Con_Printf("can not find entity with id = %d", id);
        return 0;
    }

    ent->activation_mask = lua_tointeger(lua, 2);

    return 0;
}

int lua_GetEntityState(lua_State * lua)
{
    int id, top;
    entity_p ent;
    top = lua_gettop(lua);                          ///@FIXME: top are not used!
    id = lua_tointeger(lua, 1);

    ent = World_GetEntityByID(&engine_world, id);
    if(ent == NULL)
    {
        Con_Printf("can not find entity with id = %d", id);
        return 0;
    }

    lua_pushinteger(lua, ent->next_state);

    return 1;
}

int lua_SetEntityMeshswap(lua_State * lua)
{
    int top     = lua_gettop(lua);                      ///@FIXME: top are not used!
    int id_src  = lua_tointeger(lua, 2);
    int id_dest = lua_tointeger(lua, 1);

    entity_p         ent_dest;
    skeletal_model_p model_src;

    ent_dest   = World_GetEntityByID(&engine_world, id_dest);
    model_src  = World_FindModelByID(&engine_world, id_src );

    int meshes_to_copy = (ent_dest->bf.bone_tag_count > model_src->mesh_count)?(model_src->mesh_count):(ent_dest->bf.bone_tag_count);

    for(int i = 0; i < meshes_to_copy; i++)
    {
        ent_dest->bf.bone_tags[i].mesh  = model_src->mesh_tree[i].mesh;
        ent_dest->bf.bone_tags[i].mesh2 = model_src->mesh_tree[i].mesh2;
    }
    return 0;
}

int lua_SetEntityState(lua_State * lua)
{
    int id, top;
    entity_p ent;                       ///@FIXME: top are not used!
    top = lua_gettop(lua);
    id = lua_tointeger(lua, 1);

    ent = World_GetEntityByID(&engine_world, id);
    if(ent == NULL)
    {
        Con_Printf("can not find entity with id = %d", id);
        return 0;
    }

    ent->next_state = lua_tointeger(lua, 2);

    return 0;
}

/*
 * General gameplay functions
 */

int lua_PlayStream(lua_State *lua)
{
        int id, top;

    top = lua_gettop(lua);
    id  = lua_tointeger(lua, 1);

    if(top != 1)
    {
        Con_Printf("Wrong arguments count. Must be (id).");
        return 0;
    }

    if(id < 0)
    {
        Con_Printf("Wrong stream ID. Must be more or equal to 0.");
        return 0;
    }

    Audio_StreamPlay(id);

    return 1;
}

int lua_PlaySound(lua_State *lua)
{
    int id, top;

    top = lua_gettop(lua);
    id  = lua_tointeger(lua, 1);

    if(top != 1)
    {
        Con_Printf("Wrong arguments count. Must be (id).");
        return 0;
    }

    if((id < 0) || (id >= engine_world.audio_map_count))
    {
        Con_Printf("Wrong sound ID. Must be in interval 0..%d.", engine_world.audio_map_count);
        return 0;
    }

    switch(Audio_Send(id, TR_AUDIO_EMITTER_GLOBAL))
    {
        case TR_AUDIO_SEND_NOCHANNEL:
            Con_Printf("Audio_Send error: no free channel.", id);
            break;

        case TR_AUDIO_SEND_NOSAMPLE:
            Con_Printf("Audio_Send error: no such sample.", id);
            break;

        case TR_AUDIO_SEND_IGNORED:
            Con_Printf("Audio_Send: sample skipped - please retry!", id);
            break;
    }

    return 0;
}


int lua_StopSound(lua_State *lua)
{
    int id, top;

    top = lua_gettop(lua);
    id  = lua_tointeger(lua, 1);

    if(top != 1)
    {
        Con_Printf("Wrong arguments count. Must be (id).");
        return 0;
    }

    if((id < 0) || (id >= engine_world.audio_map_count))
    {
        Con_Printf("Wrong sound ID. Must be in interval 0..%d.", engine_world.audio_map_count);
        return 0;
    }

    if(Audio_Kill(id, TR_AUDIO_EMITTER_GLOBAL) == 0)
    {
        Con_Printf("Audio_Kill: sample %d isn't playing.", id);
    }

    return 0;
}

int lua_SetLevel(lua_State *lua)
{
    int id, top;

    top = lua_gettop(lua);
    id  = lua_tointeger(lua, 1);

    if(top != 1)
    {
        Con_Printf("Wrong arguments count. Must be (id).");
        return 0;
    }

    Con_Printf("Changing gameflow_manager.CurrentLevelID to %d", id);

    Game_LevelTransition(id);
    Gameflow_Send(TR_GAMEFLOW_OP_LEVELCOMPLETE, id);    // Next level

    return 0;
}

int lua_SetGame(lua_State *lua)
{
    int   top = lua_gettop(lua);
    float id  = lua_tonumber(lua, 1);

    if(top != 1)
    {
        Con_Printf("Wrong arguments count. Must be (gameversion).");
        return 0;
    }

    lua_getglobal(lua, "GetTitleScreen");

    if(lua_isfunction(lua, -1))                                        // If function exists...
    {
        lua_pushnumber(lua, id);                                       // add to stack first argument
        lua_pcall(lua, 1, 1, 0);                                       // call that function
        Gui_FadeAssignPic(FADER_LOADSCREEN, lua_tostring(lua, -1));
        Gui_FadeStart(FADER_LOADSCREEN, TR_FADER_DIR_OUT);
    }

    lua_getglobal(lua, "GetGameflowScriptPath");

    if(lua_isfunction(lua, -1))                                        // If function exists...
    {
        lua_pushnumber(lua, id);                                       // add to stack first argument
        lua_pcall(lua, 1, 0, 0);                                       // call that function
        lua_settop(lua, top);                                          // restore LUA stack

        Con_Printf("Changing game to Tomb Raider %.1f", id);

        Gameflow_Send(TR_GAMEFLOW_OP_LEVELCOMPLETE, 1);

        return 1;
    }

    lua_settop(lua, top);   // restore LUA stack
    return 0;
}

void Engine_LuaClearTasks()
{
    int top = lua_gettop(engine_lua);
    lua_getglobal(engine_lua, "clearTasks");
    lua_pcall(engine_lua, 0, 0, 0);
    lua_settop(engine_lua, top);
}


void Engine_LuaRegisterFuncs(lua_State *lua)
{
    /*
     * register globals
     */
    luaL_dostring(lua, CVAR_LUA_TABLE_NAME" = {};");

    /*
     * register functions
     */
    lua_register(lua, "playsound", lua_PlaySound);
    lua_register(lua, "stopsound", lua_StopSound);

    lua_register(lua, "playstream", lua_PlayStream);

    lua_register(lua, "setlevel", lua_SetLevel);
    lua_register(lua, "setgame", lua_SetGame);

    lua_register(lua, "setModelCollisionMapSize", lua_SetModelCollisionMapSize);
    lua_register(lua, "setModelCollisionMap", lua_SetModelCollisionMap);
    lua_register(lua, "getAnimCommandTransform", lua_GetAnimCommandTransform);
    lua_register(lua, "setStateChangeRange", lua_SetStateChangeRange);
    lua_register(lua, "setAnimCommandTransform", lua_SetAnimCommandTransform);

    lua_register(lua, "addItem", lua_AddItem);
    lua_register(lua, "removeItem", lua_RemoveItem);
    lua_register(lua, "printItems", lua_PrintItems);
    lua_register(lua, "getItemsCount", lua_GetItemsCount);
    lua_register(lua, "getEntityVector", lua_GetEntityVector);
    lua_register(lua, "getEntityPos", lua_GetEntityPosition);
    lua_register(lua, "setEntityPos", lua_SetEntityPosition);
    lua_register(lua, "moveEntityGlobal", lua_MoveEntityGlobal);
    lua_register(lua, "moveEntityLocal", lua_MoveEntityLocal);
    lua_register(lua, "getEntitySpeed", lua_GetEntitySpeed);
    lua_register(lua, "setEntitySpeed", lua_SetEntitySpeed);
    lua_register(lua, "enableEntity", lua_EnableEntity);
    lua_register(lua, "disableEntity", lua_DisableEntity);
    lua_register(lua, "getEntityAnim", lua_GetEntityAnim);
    lua_register(lua, "setEntityAnim", lua_SetEntityAnim);
    lua_register(lua, "getModelID", lua_GetModelID);
    lua_register(lua, "canTriggerEntity", lua_CanTriggerEntity);
    lua_register(lua, "setEntityVisibility", lua_SetEntityVisibility);
    lua_register(lua, "getEntityActivity", lua_GetEntityActivity);
    lua_register(lua, "setEntityActivity", lua_SetEntityActivity);
    lua_register(lua, "getEntityOCB", lua_GetEntityOCB);
    lua_register(lua, "getEntityFlag", lua_GetEntityFlag);
    lua_register(lua, "setEntityFlag", lua_SetEntityFlag);
    lua_register(lua, "getEntityActivationMask", lua_GetEntityActivationMask);
    lua_register(lua, "setEntityActivationMask", lua_SetEntityActivationMask);
    lua_register(lua, "getEntityState", lua_GetEntityState);
    lua_register(lua, "setEntityState", lua_SetEntityState);
    lua_register(lua, "setEntityMeshswap", lua_SetEntityMeshswap);
    lua_register(lua, "getEntityActivationOffset", lua_GetActivationOffset);
    lua_register(lua, "setEntityActivationOffset", lua_SetActivationOffset);

    lua_register(lua, "gravity", lua_SetGravity);                               // get and set gravity function
    lua_register(lua, "bind", lua_BindKey);                                     // get and set key bindings
}


void Engine_Destroy()
{
    Render_Empty(&renderer);
    Con_Destroy();
    Com_Destroy();
    Sys_Destroy();

    //delete dynamics world
    delete bt_engine_dynamicsWorld;

    //delete solver
    delete bt_engine_solver;

    //delete broadphase
    delete bt_engine_overlappingPairCache;

    //delete dispatcher
    delete bt_engine_dispatcher;

    delete bt_engine_collisionConfiguration;

    delete bt_engine_ghostPairCallback;

    ///-----cleanup_end-----
    if(engine_lua)
    {
        lua_close(engine_lua);
        engine_lua = NULL;
    }

    Gui_Destroy();
}


void Engine_Shutdown(int val)
{
    Engine_LuaClearTasks();
    Render_Empty(&renderer);
    World_Empty(&engine_world);
    Engine_Destroy();

    if(frame_vertex_buffer)
    {
        free(frame_vertex_buffer);
    }
    frame_vertex_buffer = NULL;
    frame_vertex_buffer_size = 0;
    frame_vertex_buffer_size_left = 0;

    SDL_GL_DeleteContext(sdl_gl_context);
    SDL_DestroyWindow(sdl_window);

    if(sdl_joystick)
    {
        SDL_JoystickClose(sdl_joystick);
    }

    if(sdl_controller)
    {
        SDL_GameControllerClose(sdl_controller);
    }

    if(sdl_haptic)
    {
        SDL_HapticClose(sdl_haptic);
    }

    if(al_device)
    {
        alcCloseDevice(al_device);
    }

    IMG_Quit();
    SDL_Quit();

    //printf("\nSDL_Quit...");
    exit(val);
}


int engine_lua_fputs(const char *str, FILE *f)
{
    Con_AddText(str);
    return strlen(str);
}


int engine_lua_fprintf(FILE *f, const char *fmt, ...)
{
    va_list argptr;
    char buf[4096];
    int ret;

    // Create string
    va_start(argptr, fmt);
    ret = vsnprintf(buf, 4096, fmt, argptr);
    va_end(argptr);

    // Write it to target file
    fwrite(buf, 1, ret, f);

    // Write it to console, too (if it helps) und
    Con_AddText(buf);

    return ret;
}


int engine_lua_printf(const char *fmt, ...)
{
    va_list argptr;
    char buf[4096];
    int ret;

    va_start(argptr, fmt);
    ret = vsnprintf(buf, 4096, fmt, argptr);
    va_end(argptr);

    Con_AddText(buf);

    return ret;
}


engine_container_p Container_Create()
{
    engine_container_p ret;

    ret = (engine_container_p)malloc(sizeof(engine_container_t));
    ret->next = NULL;
    ret->object = NULL;
    ret->object_type = 0;
    return ret;
}


bool Engine_FileFound(const char *name, bool Write)
{
    FILE *ff;

    if(Write)
    {
        ff = fopen(name, "ab");
    }
    else
    {
        ff = fopen(name, "rb");
    }

    if(!ff)
    {
        return false;
    }
    else
    {
        fclose(ff);
        return true;
    }
}


int Engine_GetLevelVersion(const char *name)
{
    int ret = TR_UNKNOWN;
    int len = strlen(name);
    FILE *ff;

    if(len < 5)
    {
        return ret;                                                             // Wrong (too short) filename
    }

    ff = fopen(name, "rb");
    if(ff)
    {
        char ext[5];
        uint8_t check[4];

        ext[0] = name[len-4];                                                   // .
        ext[1] = toupper(name[len-3]);                                          // P
        ext[2] = toupper(name[len-2]);                                          // H
        ext[3] = toupper(name[len-1]);                                          // D
        ext[4] = 0;
        fread(check, 4, 1, ff);

        if(!strncmp(ext, ".PHD", 4))                                            //
        {
            if(check[0] == 0x20 &&
               check[1] == 0x00 &&
               check[2] == 0x00 &&
               check[3] == 0x00)
            {
                ret = TR_I;                                                     // TR_I ? OR TR_I_DEMO
            }
            else
            {
                ret = TR_UNKNOWN;
            }
        }
        else if(!strncmp(ext, ".TUB", 4))
        {
            if(check[0] == 0x20 &&
               check[1] == 0x00 &&
               check[2] == 0x00 &&
               check[3] == 0x00)
            {
                ret = TR_I_UB;                                                  // TR_I_UB
            }
            else
            {
                ret = TR_UNKNOWN;
            }
        }
        else if(!strncmp(ext, ".TR2", 4))
        {
            if(check[0] == 0x2D &&
               check[1] == 0x00 &&
               check[2] == 0x00 &&
               check[3] == 0x00)
            {
                ret = TR_II;                                                    // TR_II
            }
            else if((check[0] == 0x38 || check[0] == 0x34) &&
                    (check[1] == 0x00) &&
                    (check[2] == 0x18 || check[2] == 0x08) &&
                    (check[3] == 0xFF))
            {
                ret = TR_III;                                                   // TR_III
            }
            else
            {
                ret = TR_UNKNOWN;
            }
        }
        else if(!strncmp(ext, ".TR4", 4))
        {
            if(check[0] == 0x54 &&                                              // T
               check[1] == 0x52 &&                                              // R
               check[2] == 0x34 &&                                              // 4
               check[3] == 0x00)
            {
                ret = TR_IV;                                                    // OR TR TR_IV_DEMO
            }
            else if(check[0] == 0x54 &&                                         // T
                    check[1] == 0x52 &&                                         // R
                    check[2] == 0x34 &&                                         // 4
                    check[3] == 0x63)                                           //
            {
                ret = TR_IV;                                                    // TRLE
            }
            else if(check[0] == 0xF0 &&                                         // T
                    check[1] == 0xFF &&                                         // R
                    check[2] == 0xFF &&                                         // 4
                    check[3] == 0xFF)
            {
                ret = TR_IV;                                                    // BOGUS (OpenRaider =))
            }
            else
            {
                ret = TR_UNKNOWN;
            }
        }
        else if(!strncmp(ext, ".TRC", 4))
        {
            if(check[0] == 0x54 &&                                              // T
               check[1] == 0x52 &&                                              // R
               check[2] == 0x34 &&                                              // 4
               check[3] == 0x00)
            {
                ret = TR_V;                                                     // TR_V
            }
            else
            {
                ret = TR_UNKNOWN;
            }
        }
        else                                                                    // unknown ext.
        {
            ret = TR_UNKNOWN;
        }

        fclose(ff);
    }

    return ret;
}


void Engine_GetLevelName(char *name, const char *path)
{
    int len, start, ext, i;

    if(!path || (path[0] == 0x00))
    {
        name[0] = 0x00;
        return;
    }

    ext = len = strlen(path);
    start = 0;

    for(i=len;i>=0;i--)
    {
        if(path[i] == '.')
        {
            ext = i;
        }
        if(path[i] == '\\' || path[i] == '/')
        {
            start = i + 1;
            break;
        }
    }

    for(i=start;i<ext && i-start<LEVEL_NAME_MAX_LEN-1;i++)
    {
        name[i-start] = path[i];
    }
    name[i-start] = 0;
}


int Engine_LoadMap(const char *name)
{
    int trv;
    VT_Level tr_level;
    char buf[LEVEL_NAME_MAX_LEN], sbuf[LEVEL_NAME_MAX_LEN] = {0x00};

    Gui_DrawLoadScreen(0);

    if(!Engine_FileFound(name))
    {
        Con_Printf("File not found - \"%s\"", name);
        return 0;
    }

    trv = Engine_GetLevelVersion(name);

    if(trv == TR_UNKNOWN)
    {
        return 0;
    }

    renderer.world = NULL;

    strncpy(gameflow_manager.CurrentLevelPath, name, MAX_ENGINE_PATCH);         // it is needed for "not in the game" levels or correct saves loading.
    CVAR_set_val_s("game_level", name);
    CVAR_set_val_d("engine_version", (btScalar)trv);

    Engine_GetLevelName(buf, name);
    strcat(sbuf, "scripts/level/");
    if(trv < TR_II)
    {
        strcat(sbuf, "tr1/");
    }
    else if(trv < TR_III)
    {
        strcat(sbuf, "tr2/");
    }
    else if(trv < TR_IV)
    {
        strcat(sbuf, "tr3/");
    }
    else if(trv < TR_V)
    {
        strcat(sbuf, "tr4/");
    }
    else
    {
        strcat(sbuf, "tr5/");
    }
    strcat(sbuf, buf);
    strcat(sbuf, ".lua");
    CVAR_set_val_s("game_script", sbuf);

    tr_level.read_level(name, trv);
    tr_level.prepare_level();

    Gui_DrawLoadScreen(50);
    //tr_level.dump_textures();

    Gui_DrawLoadScreen(100);

    World_Empty(&engine_world);
    World_Prepare(&engine_world);

    Gui_DrawLoadScreen(150);

    TR_GenWorld(&engine_world, &tr_level);

    engine_world.ID   = 0;
    engine_world.name = 0;
    engine_world.type = 0;

    Con_Printf("Tomb engine version = %d, map = \"%s\"", trv, buf);
    Con_Printf("Rooms = %d", tr_level.rooms_count);
    Con_Printf("Num textures = %d", tr_level.textile32_count);
    luaL_dofile(engine_lua, "scripts/autoexec.lua");

    Game_Prepare();

    Render_SetWorld(&engine_world);

    return 1;
}


int Engine_ExecCmd(char *ch)
{
    char token[con_base.line_size];
    char buf[con_base.line_size + 32];
    char *pch;
    int val;
    room_p r;
    room_sector_p sect;
    FILE *f;

    while(ch!=NULL)
    {
        pch = ch;
        ch = parse_token(ch, token);
        if(!strcmp(token, "help"))
        {
            Con_AddLine("help - show help info\0");
            Con_AddLine("map - load level \"file_name\"\0");
            Con_AddLine("save, load - save and load game state in \"file_name\"\0");
            Con_AddLine("exit - close program\0");
            Con_AddLine("cls - clean console\0");
            Con_AddLine("show_fps - switch show fps flag\0");
            Con_AddLine("font_size - get and set font size\0");
            Con_AddLine("spacing - read and write spacing\0");
            Con_AddLine("showing_lines - read and write number of showing lines\0");
            Con_AddLine("cvars - lua's table of cvar's, to see them type: show_table(cvars)\0");
            Con_AddLine("free_look - switch camera mode\0");
            Con_AddLine("cam_distance - camera distance to actor\0");
            Con_AddLine("r_wireframe, r_portals, r_frustums, r_room_boxes, r_boxes, r_normals, r_skip_room - render modes\0");
            Con_AddLine("playsound(id) - play specified sound\0");
            Con_AddLine("stopsound(id) - stop specified sound\0");
        }
        else if(!strcmp(token, "map"))
        {
            ch = parse_token(ch, token);
            if(NULL != ch)
            {
                Engine_LoadMap(token);
            }
            return 1;
        }
        else if(!strcmp(token, "goto"))
        {
            control_states.free_look = 1;
            renderer.cam->pos[0] = SC_ParseFloat(&ch);
            renderer.cam->pos[1] = SC_ParseFloat(&ch);
            renderer.cam->pos[2] = SC_ParseFloat(&ch);
            return 1;
        }
        else if(!strcmp(token, "save"))
        {
            ch = parse_token(ch, token);
            if(NULL != ch)
            {
                Game_Save(token);
            }
            return 1;
        }
        else if(!strcmp(token, "load"))
        {
            ch = parse_token(ch, token);
            if(NULL != ch)
            {
                Game_Load(token);
            }
            return 1;
        }
        else if(!strcmp(token, "exit"))
        {
            Engine_Shutdown(0);
            return 1;
        }
        else if(!strcmp(token, "cls"))
        {
            Con_Clean();
            return 1;
        }
        else if(!strcmp(token, "font_size"))
        {
            ch = parse_token(ch, token);
            if(NULL == ch)
            {
                Con_Printf("font size = %d", con_base.font_size);
                return 1;
            }
            Con_SetFontSize(atoi(token));
            return 1;
        }
        else if(!strcmp(token, "spacing"))
        {
            ch = parse_token(ch, token);
            if(NULL == ch)
            {
                Con_Printf("spacing = %f", con_base.spacing);
                return 1;
            }
            Con_SetLineInterval(atof(token));
            return 1;
        }
        else if(!strcmp(token, "showing_lines"))
        {
            ch = parse_token(ch, token);
            if(NULL == ch)
            {
                snprintf(buf, con_base.line_size + 32, "showing_lines = %d\0", con_base.showing_lines);
                Con_AddLine(buf);
                return 1;
            }
            else
            {
                val = atoi(token);
                if((val >=2 ) && (val <= con_base.shown_lines_count))
                {
                    con_base.showing_lines = val;
                    con_base.cursor_y = screen_info.h - con_base.line_height * con_base.showing_lines;
                }
                else
                {
                    Con_AddLine("Invalid showing_lines values\0");
                }
            }
            return 1;
        }
        else if(!strcmp(token, "r_wireframe"))
        {
            renderer.style ^= R_DRAW_WIRE;
            return 1;
        }
        else if(!strcmp(token, "r_normals"))
        {
            renderer.style ^= R_DRAW_NORMALS;
            return 1;
        }
        else if(!strcmp(token, "r_portals"))
        {
            renderer.style ^= R_DRAW_PORTALS;
            return 1;
        }
        else if(!strcmp(token, "r_frustums"))
        {
            renderer.style ^= R_DRAW_FRUSTUMS;
            return 1;
        }
        else if(!strcmp(token, "r_room_boxes"))
        {
            renderer.style ^= R_DRAW_ROOMBOXES;
            return 1;
        }
        else if(!strcmp(token, "r_boxes"))
        {
            renderer.style ^= R_DRAW_BOXES;
            return 1;
        }
        else if(!strcmp(token, "r_axis"))
        {
            renderer.style ^= R_DRAW_AXIS;
            return 1;
        }
        else if(!strcmp(token, "r_nullmeshes"))
        {
            renderer.style ^= R_DRAW_NULLMESHES;
            return 1;
        }
        else if(!strcmp(token, "r_dummy_statics"))
        {
            renderer.style ^= R_DRAW_DUMMY_STATICS;
            return 1;
        }
        else if(!strcmp(token, "r_skip_room"))
        {
            renderer.style ^= R_SKIP_ROOM;
            return 1;
        }
        else if(!strcmp(token, "room_info"))
        {
            r = renderer.cam->current_room;
            if(r)
            {
                sect = Room_GetSectorXYZ(r, renderer.cam->pos);
                Con_Printf("ID = %d, x_sect = %d, y_sect = %d", r->ID, r->sectors_x, r->sectors_y);
                if(sect)
                {
                    Con_Printf("sect(%d, %d), inpenitrable = %d, r_up = %d, r_down = %d", sect->index_x, sect->index_y,
                               (int)(sect->ceiling == 32512 || sect->floor == 32512), (int)(sect->sector_above != NULL), (int)(sect->sector_below != NULL));
                    for(int i=0;i<sect->owner_room->static_mesh_count;i++)
                    {
                        Con_Printf("static[%d].object_id = %d", i, sect->owner_room->static_mesh[i].object_id);
                    }
                    for(engine_container_p cont=sect->owner_room->containers;cont;cont=cont->next)
                    {
                        if(cont->object_type == OBJECT_ENTITY)
                        {
                            entity_p e = (entity_p)cont->object;
                            Con_Printf("cont[entity](%d, %d, %d).object_id = %d", (int)e->transform[12+0], (int)e->transform[12+1], (int)e->transform[12+2], e->ID);
                        }
                    }
                }
            }
            return 1;
        }
        else if(!strcmp(token, "xxx"))
        {
            f = fopen("ascII.txt", "r");
            if(f)
            {
                long int size;
                char *buf;
                fseek(f, 0, SEEK_END);
                size= ftell(f);
                buf = (char*) malloc((size+1)*sizeof(char));

                fseek(f, 0, SEEK_SET);
                fread(buf, size, sizeof(char), f);
                buf[size] = 0;
                fclose(f);
                Con_Clean();
                Con_AddText(buf);
                free(buf);
            }
            else
            {
                Con_AddText("Not avaliable =(");
            }
            return 1;
        }
        else if(token[0])
        {
            if(engine_lua)
            {
                Con_AddLine(pch);
                luaL_dostring(engine_lua, pch);
            }
            else
            {
                snprintf(buf, con_base.line_size + 32, "Command \"%s\" not found\0", token);
                Con_AddLine(buf);
            }
            return 0;
        }
    }

    return 0;
}


void Engine_LoadConfig()
{
    if(!engine_lua)
    {
        return;
    }

    if(Engine_FileFound("scripts/config/control_constants.lua"))
    {
        luaL_dofile(engine_lua, "scripts/config/control_constants.lua");
    }
    else
    {
        Sys_Warn("Could not find \"scripts/config/control_constants.lua\"");
    }

    if(Engine_FileFound("config.lua"))
    {
        luaL_dofile(engine_lua, "config.lua");
    }
    else
    {
        Sys_Warn("Could not find \"config.lua\"");
    }

    lua_ParseScreen(engine_lua, &screen_info);
    lua_ParseRender(engine_lua, &renderer.settings);
    lua_ParseAudio(engine_lua, &audio_settings);
    lua_ParseConsole(engine_lua, &con_base);
    lua_ParseControlSettings(engine_lua, &control_mapper);
}


void Engine_SaveConfig()
{

}

