
#include <SDL2/SDL.h>
#include <SDL2/SDL_platform.h>
#if !defined(__MACOSX__)
#include <SDL2/SDL_image.h>
#endif
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
#include "redblack.h"
#include "gl_font.h"
#include "string.h"

extern SDL_Window             *sdl_window;
extern SDL_GLContext           sdl_gl_context;
extern SDL_GameController     *sdl_controller;
extern SDL_Joystick           *sdl_joystick;
extern SDL_Haptic             *sdl_haptic;
extern ALCdevice              *al_device;
extern ALCcontext             *al_context;

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
btSequentialImpulseConstraintSolver     *bt_engine_solver;
btDiscreteDynamicsWorld                 *bt_engine_dynamicsWorld;
btOverlapFilterCallback                 *bt_engine_filterCallback;

render_DebugDrawer                       debugDrawer;

/**
 * overlapping room collision filter
 */
void Engine_RoomNearCallback(btBroadphasePair& collisionPair, btCollisionDispatcher& dispatcher, const btDispatcherInfo& dispatchInfo)
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
 * update current room of bullet object
 */
void Engine_InternalTickCallback(btDynamicsWorld *world, btScalar timeStep)
{
    for(int i=world->getNumCollisionObjects()-1;i>=0;i--)
    {
        btCollisionObject* obj = bt_engine_dynamicsWorld->getCollisionObjectArray()[i];
        btRigidBody* body = btRigidBody::upcast(obj);
        if (body && !body->isStaticObject() && body->getMotionState())
        {
            btTransform trans;
            body->getMotionState()->getWorldTransform(trans);
            engine_container_p cont = (engine_container_p)body->getUserPointer();
            if(cont && (cont->object_type == OBJECT_BULLET_MISC))
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


void Engine_InitDefaultGlobals()
{
    Con_InitGlobals();
    Controls_InitGlobals();
    Game_InitGlobals();
    Render_InitGlobals();
    Audio_InitGlobals();
}

// First stage of initialization.

void Engine_Init_Pre()
{
    /* Console must be initialized previously! some functions uses CON_AddLine before GL initialization!
     * Rendering activation may be done later. */
    Gui_InitFontManager();
    Con_Init();
    Engine_LuaInit();
    Gameflow_Init();

    frame_vertex_buffer = (btScalar*)malloc(sizeof(btScalar) * INIT_FRAME_VERTEX_BUFFER_SIZE);
    frame_vertex_buffer_size = INIT_FRAME_VERTEX_BUFFER_SIZE;
    frame_vertex_buffer_size_left = frame_vertex_buffer_size;

    Com_Init();
    Render_Init();
    Cam_Init(&engine_camera);
    renderer.cam = &engine_camera;

    Engine_BTInit();
}

// Second stage of initialization.

void Engine_Init_Post()
{
    luaL_dofile(engine_lua, "scripts/gui/fonts.lua");
    Con_InitFonts();

    Gui_Init();
    Sys_Init();

    Con_AddLine("Engine inited!", FONTSTYLE_CONSOLE_EVENT);
}

// Bullet Physics initialization.

void Engine_BTInit()
{
    ///collision configuration contains default setup for memory, collision setup. Advanced users can create their own configuration.
    bt_engine_collisionConfiguration = new btDefaultCollisionConfiguration();

    ///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
    bt_engine_dispatcher = new btCollisionDispatcher(bt_engine_collisionConfiguration);
    bt_engine_dispatcher->setNearCallback(Engine_RoomNearCallback);

    ///btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
    bt_engine_overlappingPairCache = new btDbvtBroadphase();
    bt_engine_ghostPairCallback = new btGhostPairCallback();
    bt_engine_overlappingPairCache->getOverlappingPairCache()->setInternalGhostPairCallback(bt_engine_ghostPairCallback);

    ///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
    bt_engine_solver = new btSequentialImpulseConstraintSolver;

    bt_engine_dynamicsWorld = new btDiscreteDynamicsWorld(bt_engine_dispatcher, bt_engine_overlappingPairCache, bt_engine_solver, bt_engine_collisionConfiguration);
    bt_engine_dynamicsWorld->setInternalTickCallback(Engine_InternalTickCallback);
    bt_engine_dynamicsWorld->setGravity(btVector3(0, 0, -4500.0));

    debugDrawer.setDebugMode(btIDebugDraw::DBG_DrawWireframe);
    bt_engine_dynamicsWorld->setDebugDrawer(&debugDrawer);
    //bt_engine_dynamicsWorld->getPairCache()->setInternalGhostPairCallback(bt_engine_filterCallback);
}

/*
 * debug functions
 */

 int lua_CheckStack(lua_State *lua)
 {
     Con_Printf("Current Lua stack index: %d", lua_gettop(lua));
     return 0;
 }

 int lua_print(lua_State * lua)
 {
     int top = lua_gettop(lua);

     if(top == 0)
     {
        Con_AddLine("nil");
     }

     for(int i=1;i<=top;i++)
     {
         Con_AddLine(lua_tostring(lua, i), FONTSTYLE_CONSOLE_EVENT);
     }

     return 0;
 }

 int lua_DumpModel(lua_State * lua)
 {
     int id = 0;
     if(lua_gettop(lua) > 0)
     {
         id = lua_tointeger(lua, 1);
     }

     skeletal_model_p sm = World_GetModelByID(&engine_world, id);
     if(sm == NULL)
     {
        Con_Printf("wrong model id = %d", id);
        return 0;
     }

     for(int i=0;i<sm->mesh_count;i++)
     {
         Con_Printf("mesh[%d] = %d", i, sm->mesh_tree[i].mesh_base->id);
     }

     return 0;
 }

int lua_DumpRoom(lua_State * lua)
{
    room_p r = NULL;

    if(lua_gettop(lua) == 0)
    {
        r = engine_camera.current_room;
    }
    else
    {
        uint32_t id = lua_tointeger(lua, 1);
        if(id >= engine_world.room_count)
        {
            Con_Warning(SYSWARN_WRONG_ROOM, id);
            return 0;
        }
        r = engine_world.rooms + id;
    }

    if(r != NULL)
    {
        room_sector_p rs = r->sectors;
        Sys_DebugLog("room_dump.txt", "ROOM = %d, (%d x %d), bottom = %g, top = %g, pos(%g, %g)", r->id, r->sectors_x, r->sectors_y, r->bb_min[2], r->bb_max[2], r->transform[12 + 0], r->transform[12 + 1]);
        Sys_DebugLog("room_dump.txt", "flag = 0x%X, alt_room = %d, base_room = %d", r->flags, (r->alternate_room != NULL)?(r->alternate_room->id):(-1), (r->base_room != NULL)?(r->base_room->id):(-1));
        for(uint32_t i=0;i<r->sectors_count;i++,rs++)
        {
            Sys_DebugLog("room_dump.txt", "(%d,%d)\tfloor = %d, ceiling = %d, portal = %d", rs->index_x, rs->index_y, rs->floor, rs->ceiling, rs->portal_to_room);
        }
    }

    return 0;
}


int lua_SetRoomEnabled(lua_State * lua)
{
    if(lua_gettop(lua) < 2)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[id], [value]");
        return 0;
    }

    uint32_t id = lua_tointeger(lua, 1);
    if(id >= engine_world.room_count)
    {
        Con_Warning(SYSWARN_WRONG_ROOM, id);
        return 0;
    }

    if(lua_tointeger(lua, 2) == 0)
    {
        Room_Disable(engine_world.rooms + id);
    }
    else
    {
        Room_Enable(engine_world.rooms + id);
    }

    return 0;
}

/*
 * Base engine functions
 */

int lua_SetModelCollisionMapSize(lua_State * lua)
{
    if(lua_gettop(lua) < 2)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[id], [value]");
        return 0;
    }

    uint32_t id = lua_tointeger(lua, 1);
    if(id > engine_world.skeletal_model_count - 1)
    {
        Con_Warning(SYSWARN_MODELID_OVERFLOW, id);
        return 0;
    }

    int size = lua_tointeger(lua, 2);
    if(size >= 0 && size < engine_world.skeletal_models[id].mesh_count)
    {
        engine_world.skeletal_models[id].collision_map_size = size;
    }

    return 0;
}


int lua_SetModelCollisionMap(lua_State * lua)
{
    if(lua_gettop(lua) < 3)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "(id, map_index, value)");
        return 0;
    }

    uint32_t id = lua_tointeger(lua, 1);
    if(id > engine_world.skeletal_model_count - 1)
    {
        Con_Warning(SYSWARN_MODELID_OVERFLOW, id);
        return 0;
    }

    int arg = lua_tointeger(lua, 2);
    int val = lua_tointeger(lua, 3);
    if((arg >= 0) && (arg < engine_world.skeletal_models[id].mesh_count) &&
       (val >= 0) && (val < engine_world.skeletal_models[id].mesh_count))
    {
        engine_world.skeletal_models[id].collision_map[arg] = val;
    }

    return 0;
}


int lua_EnableEntity(lua_State * lua)
{
    if(lua_gettop(lua) < 1)
    {
        Con_Warning(SYSWARN_ENTER_ENTITY_ID);
        return 0;
    }

    entity_p ent = World_GetEntityByID(&engine_world, lua_tonumber(lua, 1));
    if(ent != NULL) Entity_Enable(ent);

    return 0;
}


int lua_DisableEntity(lua_State * lua)
{
    if(lua_gettop(lua) < 1)
    {
        Con_Warning(SYSWARN_ENTER_ENTITY_ID);
        return 0;
    }

    entity_p ent = World_GetEntityByID(&engine_world, lua_tonumber(lua, 1));
    if(ent != NULL) Entity_Disable(ent);

    return 0;
}


int lua_SetEntityCollision(lua_State * lua)
{
    if(lua_gettop(lua) < 1)
    {
        Con_Warning(SYSWARN_ENTER_ENTITY_ID);
        return 0;
    }

    entity_p ent = World_GetEntityByID(&engine_world, lua_tonumber(lua, 1));
    if(ent != NULL)
    {
        if(lua_tointeger(lua, 2) != 0)
        {
            Entity_EnableCollision(ent);
        }
        else
        {
            Entity_DisableCollision(ent);
        }
    }

    return 0;
}


int lua_GetEntitySectorFlags(lua_State *lua)
{
    if(lua_gettop(lua) < 1) return 0;   // No entity specified - return.
    entity_p ent = World_GetEntityByID(&engine_world, lua_tonumber(lua, 1));

    if((ent != NULL) && (ent->current_sector))
    {
        lua_pushinteger(lua, ent->current_sector->flags);
        return 1;
    }
    return 0;
}

int lua_GetEntitySectorIndex(lua_State *lua)
{
    if(lua_gettop(lua) < 1) return 0;   // No entity specified - return.
    entity_p ent = World_GetEntityByID(&engine_world, lua_tonumber(lua, 1));

    if((ent != NULL) && (ent->current_sector))
    {
        lua_pushinteger(lua, ent->current_sector->trig_index);
        return 1;
    }
    return 0;
}

int lua_GetEntitySectorMaterial(lua_State *lua)
{
    if(lua_gettop(lua) < 1) return 0;   // No entity specified - return.
    entity_p ent = World_GetEntityByID(&engine_world, lua_tonumber(lua, 1));

    if((ent != NULL) && (ent->current_sector))
    {
        lua_pushinteger(lua, ent->current_sector->material);
        return 1;
    }
    return 0;
}

int lua_NewSector(lua_State *lua)
{
    if(lua_gettop(lua) < 1) return 0;   // No argument specified - return.
    entity_p ent = World_GetEntityByID(&engine_world, lua_tonumber(lua, 1));

    if(ent != NULL)
    {
        if(ent->current_sector == ent->last_sector)
        {
            lua_pushinteger(lua, 1);
        }
        else
        {
            lua_pushinteger(lua, 0);
        }
        return 1;
    }
    return 0;
}


int lua_GetGravity(lua_State * lua)
{
    btVector3 g = bt_engine_dynamicsWorld->getGravity();
    lua_pushnumber(lua, g.m_floats[0]);
    lua_pushnumber(lua, g.m_floats[1]);
    lua_pushnumber(lua, g.m_floats[2]);

    return 3;
}


int lua_SetGravity(lua_State * lua)                                             // function to be exported to Lua
{
    btVector3 g;

    switch(lua_gettop(lua))
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
            Con_Warning(SYSWARN_WRONG_ARGS_COUNT, "0, 1 or 3");
            break;
    };

    return 0;
}


int lua_DropEntity(lua_State * lua)
{
    if(lua_gettop(lua) < 2)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[entity_id], [time]");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(&engine_world, id);
    if(ent == NULL)
    {
        Con_Warning(SYSWARN_NO_ENTITY, id);
        return 0;
    }

    btScalar time = lua_tonumber(lua, 2);
    btVector3 g = bt_engine_dynamicsWorld->getGravity();
    btVector3 move = ent->speed * time;;
    move += g * 0.5 * time * time;
    ent->speed += g * time;

    bt_engine_ClosestRayResultCallback cb(ent->self);
    btVector3 from, to;
    Mat4_vec3_mul_macro(from.m_floats, ent->transform, ent->bf.centre);
    from.m_floats[2] = ent->transform[12 + 2];
    to = from + move;
    to.m_floats[2] -= (ent->bf.bb_max[2] - ent->bf.bb_min[2]);
    bt_engine_dynamicsWorld->rayTest(from, to, cb);
    if(cb.hasHit())
    {
        move.setInterpolate3(from ,to, cb.m_closestHitFraction);
        ent->transform[12+2] = move.m_floats[2];
        lua_pushboolean(lua, 1);
        return 1;
    }

    ent->transform[12+0] += move.m_floats[0];
    ent->transform[12+1] += move.m_floats[1];
    ent->transform[12+2] += move.m_floats[2];
    lua_pushboolean(lua, 0);
    return 1;
}


int lua_GetModelID(lua_State * lua)
{
    if(lua_gettop(lua) < 1) return 0;   // No argument - return.

    entity_p ent = World_GetEntityByID(&engine_world, lua_tointeger(lua, 1));
    if(ent == NULL) return 0;

    if(ent->bf.animations.model)
    {
        lua_pushinteger(lua, ent->bf.animations.model->id);
        return 1;
    }
    return 0;
}


int lua_GetActivationOffset(lua_State * lua)
{
    if(lua_gettop(lua) < 1) return 0;   // No argument - return.

    entity_p ent = World_GetEntityByID(&engine_world, lua_tointeger(lua, 1));
    if(ent == NULL) return 0;

    lua_pushnumber(lua, ent->activation_offset[0]);
    lua_pushnumber(lua, ent->activation_offset[1]);
    lua_pushnumber(lua, ent->activation_offset[2]);
    lua_pushnumber(lua, ent->activation_offset[3]);

    return 4;
}


int lua_SetActivationOffset(lua_State * lua)
{
    int top = lua_gettop(lua);

    if(top < 1)
    {
        Con_AddLine("not set entity id");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(&engine_world, id);
    if(ent == NULL)
    {
        Con_Warning(SYSWARN_NO_ENTITY, id);
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

int lua_GetCharacterParam(lua_State * lua)
{
    if(lua_gettop(lua) < 2)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[entity_id], [param]");
        return 0;
    }

    int id         = lua_tointeger(lua, 1);
    int parameter  = lua_tointeger(lua, 2);
    entity_p ent   = World_GetEntityByID(&engine_world, id);

    if(parameter >= PARAM_LASTINDEX)
    {
        Con_Warning(SYSWARN_WRONG_OPTION_INDEX, PARAM_LASTINDEX);
        return 0;
    }
    if(!IsCharacter(ent))
    {
        Con_Warning(SYSWARN_NO_CHARACTER, id);
        return 0;
    }
    else
    {
        lua_pushnumber(lua, Character_GetParam(ent, parameter));
        return 1;
    }
}


int lua_SetCharacterParam(lua_State * lua)
{
    int top = lua_gettop(lua);

    if(top < 3)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[entity_id], [param], [value], (max_value)");
        return 0;
    }

    int id           = lua_tointeger(lua, 1);
    int parameter    = lua_tointeger(lua, 2);
    entity_p ent     = World_GetEntityByID(&engine_world, id);

    if(parameter >= PARAM_LASTINDEX)
    {
        Con_Warning(SYSWARN_WRONG_OPTION_INDEX, PARAM_LASTINDEX);
        return 0;
    }
    if(!IsCharacter(ent))
    {
        Con_Warning(SYSWARN_NO_CHARACTER, id);
        return 0;
    }
    else if(top == 3)
    {
        Character_SetParam(ent, parameter, lua_tonumber(lua, 3));
    }
    else
    {
        ent->character->parameters.param[parameter] = lua_tonumber(lua, 3);
        ent->character->parameters.maximum[parameter] = lua_tonumber(lua, 4);
    }

    return 0;
}

int lua_GetCharacterCombatMode(lua_State * lua)
{
    if(lua_gettop(lua) < 1) return 0;
    entity_p ent = World_GetEntityByID(&engine_world, lua_tointeger(lua, 1));

    if(!IsCharacter(ent))
    {
        return 0;
    }
    else
    {
        lua_pushnumber(lua, ent->character->weapon_current_state);
        return 1;
    }
}

int lua_ChangeCharacterParam(lua_State * lua)
{
    if(lua_gettop(lua) < 3)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[entity_id], [param], [value]");
        return 0;
    }

    int id         = lua_tointeger(lua, 1);
    int parameter  = lua_tointeger(lua, 2);
    int value      = lua_tonumber(lua, 3);
    entity_p ent   = World_GetEntityByID(&engine_world, id);

    if(parameter >= PARAM_LASTINDEX)
    {
        Con_Warning(SYSWARN_WRONG_OPTION_INDEX, PARAM_LASTINDEX);
        return 0;
    }
    if(!IsCharacter(ent))
    {
        Con_Warning(SYSWARN_NO_CHARACTER, id);
        return 0;
    }
    Character_ChangeParam(ent, parameter, value);

    return 0;
}


int lua_GetSecretStatus(lua_State *lua)
{
    if(lua_gettop(lua) < 1) return 0;   // No parameter specified - return

    int secret_number = lua_tointeger(lua, 1);
    if((secret_number > TR_GAMEFLOW_MAX_SECRETS) || (secret_number < 0)) return 0;   // No such secret - return

    lua_pushinteger(lua, (int)gameflow_manager.SecretsTriggerMap[secret_number]);
    return 1;
}

int lua_SetSecretStatus(lua_State *lua)
{
    if(lua_gettop(lua) < 2) return 0;   // No parameter specified - return

    int secret_number = lua_tointeger(lua, 1);
    if((secret_number > TR_GAMEFLOW_MAX_SECRETS) || (secret_number < 0)) return 0;   // No such secret - return

    gameflow_manager.SecretsTriggerMap[secret_number] = lua_tointeger(lua, 2);
    return 0;
}


int lua_GetActionState(lua_State *lua)
{
    int top = lua_gettop(lua);
    int act = lua_tointeger(lua, 1);

    if(top < 1 || act < 0 || act >= ACT_LASTINDEX)
    {
        Con_Warning(SYSWARN_WRONG_ACTION_NUMBER);
        return 0;
    }
    else if(top == 1)
    {
        lua_pushinteger(lua, (int)(control_mapper.action_map[act].state));
        return 1;
    }

    Con_Warning(SYSWARN_WRONG_ARGS_COUNT, "1");
    return 0;
}


int lua_GetActionChange(lua_State *lua)
{
    int top = lua_gettop(lua);
    int act = lua_tointeger(lua, 1);

    if(top < 1 || act < 0 || act >= ACT_LASTINDEX)
    {
        Con_Warning(SYSWARN_WRONG_ACTION_NUMBER);
        return 0;
    }
    else if(top == 1)
    {
        lua_pushinteger(lua, (int)(control_mapper.action_map[act].already_pressed));
        return 1;
    }

    Con_Warning(SYSWARN_WRONG_ARGS_COUNT, "1");
    return 0;
}


int lua_GetLevelVersion(lua_State *lua)
{
    lua_pushinteger(lua, engine_world.version);
    return 1;
}


int lua_BindKey(lua_State *lua)
{
    int top = lua_gettop(lua);
    int act = lua_tointeger(lua, 1);

    if(top < 1 || act < 0 || act >= ACT_LASTINDEX)
    {
        Con_Warning(SYSWARN_WRONG_ACTION_NUMBER);
    }

    else if(top == 2)
    {
        control_mapper.action_map[act].primary = lua_tointeger(lua, 2);
    }
    else if(top == 3)
    {
        control_mapper.action_map[act].primary   = lua_tointeger(lua, 2);
        control_mapper.action_map[act].secondary = lua_tointeger(lua, 3);
    }
    else
    {
        Con_Warning(SYSWARN_WRONG_ARGS_COUNT, "2 or 3");
    }

    return 0;
}

int lua_AddFont(lua_State *lua)
{
    if(lua_gettop(lua) != 3)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[font index], [font path], [font size]");
        return 0;
    }

    if(!FontManager->AddFont((font_Type)lua_tointeger(lua, 1),
                            (uint32_t) lua_tointeger(lua, 3),
                                       lua_tostring(lua, 2)))
    {
        Con_Warning(SYSWARN_CANT_CREATE_FONT, FontManager->GetFontCount(), GUI_MAX_FONTS);

    }

    return 0;
}

int lua_AddFontStyle(lua_State *lua)
{
    if(lua_gettop(lua) != 14)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[index, R, G, B, A, shadow, fade, rect, border, bR, bG, bB, bA, hide]");
        return 0;
    }

    font_Style  style_index = (font_Style)lua_tointeger(lua, 1);
    GLfloat     color_R     = (GLfloat)lua_tonumber(lua, 2);
    GLfloat     color_G     = (GLfloat)lua_tonumber(lua, 3);
    GLfloat     color_B     = (GLfloat)lua_tonumber(lua, 4);
    GLfloat     color_A     = (GLfloat)lua_tonumber(lua, 5);
    bool        shadowed    = lua_toboolean(lua, 6);
    bool        fading      = lua_toboolean(lua, 7);
    bool        rect        = lua_toboolean(lua, 8);
    GLfloat     rect_border = (GLfloat)lua_tonumber(lua, 9);
    GLfloat     rect_R      = (GLfloat)lua_tonumber(lua, 10);
    GLfloat     rect_G      = (GLfloat)lua_tonumber(lua, 11);
    GLfloat     rect_B      = (GLfloat)lua_tonumber(lua, 12);
    GLfloat     rect_A      = (GLfloat)lua_tonumber(lua, 13);
    bool        hide        = lua_toboolean(lua, 14);


    if(!FontManager->AddFontStyle(style_index,
                                  color_R, color_G, color_B, color_A,
                                  shadowed, fading,
                                  rect, rect_border, rect_R, rect_G, rect_B, rect_A,
                                  hide))
    {

        Con_Warning(SYSWARN_CANT_CREATE_STYLE, FontManager->GetFontStyleCount(), GUI_MAX_FONTSTYLES);
    }

    return 0;
}

int lua_DeleteFont(lua_State *lua)
{
    if(lua_gettop(lua) != 1)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[font index]");
        return 0;
    }

    if(!FontManager->RemoveFont((font_Type)lua_tointeger(lua, 1)))
    {
        Con_Warning(SYSWARN_CANT_REMOVE_FONT);
    }

    return 0;
}

int lua_DeleteFontStyle(lua_State *lua)
{
    if(lua_gettop(lua) != 1)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[style index]");
        return 0;
    }

    if(!FontManager->RemoveFontStyle((font_Style)lua_tointeger(lua, 1)))
    {
        Con_Warning(SYSWARN_CANT_REMOVE_STYLE);
    }

    return 0;
}

int lua_AddItem(lua_State * lua)
{
    int top, count;
    top = lua_gettop(lua);

    if(top < 2)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[entity_id], [item_id], [items_count]");
        return 0;
    }

    if(top >= 3)
    {
        count = lua_tointeger(lua, 3);
    }
    else
    {
        count = -1;
    }

    int entity_id = lua_tointeger(lua, 1);
    int item_id = lua_tointeger(lua, 2);

    entity_p ent = World_GetEntityByID(&engine_world, entity_id);
    if(ent == NULL)
    {
        Con_Warning(SYSWARN_NO_ENTITY, entity_id);
        return 0;
    }

    lua_pushinteger(lua, Character_AddItem(ent, item_id, count));
    return 1;
}


int lua_RemoveItem(lua_State * lua)
{
    if(lua_gettop(lua) < 3)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[entity_id], [item_id], [items_count]");
        return 0;
    }

    int entity_id = lua_tointeger(lua, 1);
    int item_id = lua_tointeger(lua, 2);
    int count = lua_tointeger(lua, 3);

    entity_p ent = World_GetEntityByID(&engine_world, entity_id);
    if(ent == NULL)
    {
        Con_Warning(SYSWARN_NO_ENTITY, entity_id);
        return 0;
    }

    lua_pushinteger(lua, Character_RemoveItem(ent, item_id, count));
    return 1;
}


int lua_RemoveAllItems(lua_State * lua)
{
    if(lua_gettop(lua) < 1)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[entity_id]");
        return 0;
    }

    int entity_id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(&engine_world, entity_id);
    if(ent == NULL)
    {
        Con_Warning(SYSWARN_NO_ENTITY, entity_id);
        return 0;
    }
    Character_RemoveAllItems(ent);

    return 0;
}


int lua_GetItemsCount(lua_State * lua)
{
    if(lua_gettop(lua) < 2)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[entity_id], [item_id]");
        return 0;
    }
    int entity_id = lua_tointeger(lua, 1);
    int item_id = lua_tointeger(lua, 2);

    entity_p ent = World_GetEntityByID(&engine_world, entity_id);
    if(ent == NULL)
    {
        Con_Warning(SYSWARN_NO_ENTITY, entity_id);
        return 0;
    }

    lua_pushinteger(lua, Character_GetItemsCount(ent, item_id));
    return 1;
}


int lua_CreateBaseItem(lua_State * lua)
{
    if(lua_gettop(lua) < 5)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[item_id], [model_id], [world_model_id], [type], [count], (name))");
        return 0;
    }

    int item_id         = lua_tointeger(lua, 1);
    int model_id        = lua_tointeger(lua, 2);
    int world_model_id  = lua_tointeger(lua, 3);
    int type            = lua_tointeger(lua, 4);
    int count           = lua_tointeger(lua, 5);

    World_CreateItem(&engine_world, item_id, model_id, world_model_id, type, count, lua_tostring(lua, 6));

    return 0;
}


int lua_DeleteBaseItem(lua_State * lua)
{
    if(lua_gettop(lua) < 1)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[item_id]");
    }
    else
    {
        World_DeleteItem(&engine_world, lua_tointeger(lua, 1));
    }
    return 0;
}


int lua_PrintItems(lua_State * lua)
{
    if(lua_gettop(lua) < 1)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[entity_id]");
        return 0;
    }

    int entity_id = lua_tointeger(lua, 1);
    entity_p  ent = World_GetEntityByID(&engine_world, entity_id);
    if(ent == NULL)
    {
        Con_Warning(SYSWARN_NO_ENTITY, entity_id);
        return 0;
    }

    if(ent->character)
    {
        inventory_node_p i = ent->character->inventory;
        for(;i;i=i->next)
        {
            Con_Printf("item[id = %d]: count = %d, type = %d", i->id, i->count);
        }
    }
    return 0;
}


int lua_SetStateChangeRange(lua_State * lua)
{
    int top = lua_gettop(lua);

    if(top < 6)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[model_id], [anim_num], [state_id], [dispatch_num], [start_frame], [end_frame], (next_anim), (next_frame)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    skeletal_model_p model = World_GetModelByID(&engine_world, id);

    if(model == NULL)
    {
        Con_Warning(SYSWARN_NO_SKELETAL_MODEL, id);
        return 0;
    }

    int anim = lua_tointeger(lua, 2);
    int state = lua_tointeger(lua, 3);
    int dispatch = lua_tointeger(lua, 4);
    int frame_low = lua_tointeger(lua, 5);
    int frame_high = lua_tointeger(lua, 6);

    if((anim < 0) || (anim + 1 > model->animation_count))
    {
        Con_Warning(SYSWARN_WRONG_ANIM_NUMBER, anim);
        return 0;
    }

    animation_frame_p af = model->animations + anim;
    for(uint16_t i=0;i<af->state_change_count;i++)
    {
        if(af->state_change[i].id == (uint32_t)state)
        {
            if((dispatch >= 0) && (dispatch < af->state_change[i].anim_dispatch_count))
            {
                af->state_change[i].anim_dispatch[dispatch].frame_low = frame_low;
                af->state_change[i].anim_dispatch[dispatch].frame_high = frame_high;
                if(top >= 8)
                {
                    af->state_change[i].anim_dispatch[dispatch].next_anim = lua_tointeger(lua, 7);
                    af->state_change[i].anim_dispatch[dispatch].next_frame = lua_tointeger(lua, 8);
                }
            }
            else
            {
                Con_Warning(SYSWARN_WRONG_DISPATCH_NUMBER, dispatch);
            }
            break;
        }
    }

    return 0;
}


int lua_GetAnimCommandTransform(lua_State * lua)
{
    if(lua_gettop(lua) < 3)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[model_id], [anim_num], [frame_num]");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    int anim = lua_tointeger(lua, 2);
    int frame = lua_tointeger(lua, 3);
    skeletal_model_p model = World_GetModelByID(&engine_world, id);
    if(model == NULL)
    {
        Con_Warning(SYSWARN_NO_SKELETAL_MODEL, id);
        return 0;
    }

    if((anim < 0) || (anim + 1 > model->animation_count))
    {
        Con_Warning(SYSWARN_WRONG_ANIM_NUMBER, anim);
        return 0;
    }

    if(frame < 0)                                                               // it is convenient to use -1 as a last frame number
    {
        frame = (int)model->animations[anim].frames_count + frame;
    }

    if((frame < 0) || (frame + 1 > model->animations[anim].frames_count))
    {
        Con_Warning(SYSWARN_WRONG_FRAME_NUMBER, frame);
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
    int top = lua_gettop(lua);

    if(top < 4)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[model_id] [anim_num], [frame_num], [flag], (dx, dy, dz)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    int anim = lua_tointeger(lua, 2);
    int frame = lua_tointeger(lua, 3);
    skeletal_model_p model = World_GetModelByID(&engine_world, id);
    if(model == NULL)
    {
        Con_Warning(SYSWARN_NO_SKELETAL_MODEL, id);
        return 0;
    }

    if((anim < 0) || (anim + 1 > model->animation_count))
    {
        Con_Warning(SYSWARN_WRONG_ANIM_NUMBER, anim);
        return 0;
    }

    if(frame < 0)                                                               // it is convenient to use -1 as a last frame number
    {
        frame = (int)model->animations[anim].frames_count + frame;
    }

    if((frame < 0) || (frame + 1 > model->animations[anim].frames_count))
    {
        Con_Warning(SYSWARN_WRONG_FRAME_NUMBER, frame);
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


int lua_SpawnEntity(lua_State * lua)
{
    if(lua_gettop(lua) < 5)
    {
        ///uint32_t World_SpawnEntity(uint32_t model_id, uint32_t room_id, btScalar pos[3], btScalar ang[3])
        Con_Warning(SYSWARN_WRONG_ARGS, "[model_id1], [room_id], [x], [y], [z], (ax, ay, az))");
        return 0;
    }

    btScalar pos[3], ang[3];
    int model_id = lua_tointeger(lua, 1);
    int room_id = lua_tointeger(lua, 2);
    pos[0] = lua_tonumber(lua, 3);
    pos[1] = lua_tonumber(lua, 4);
    pos[2] = lua_tonumber(lua, 5);
    ang[0] = lua_tonumber(lua, 6);
    ang[1] = lua_tonumber(lua, 7);
    ang[2] = lua_tonumber(lua, 8);

    int32_t ov_id = -1;
    if(lua_isnumber(lua, 9))
    {
        ov_id = lua_tointeger(lua, 9);
    }

    uint32_t id = World_SpawnEntity(model_id, room_id, pos, ang, ov_id);
    if(id == 0xFFFFFFFF)
    {
        lua_pushnil(lua);
    }
    else
    {
        lua_pushinteger(lua, id);
    }

    return 1;
}


/*
 * Moveables script control section
 */
int lua_GetEntityVector(lua_State * lua)
{
    if(lua_gettop(lua) < 2)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[id1], [id2]");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p e1 = World_GetEntityByID(&engine_world, id);
    if(e1 == NULL)
    {
        Con_Warning(SYSWARN_NO_ENTITY, id);
        return 0;
    }
    id = lua_tointeger(lua, 2);
    entity_p e2 = World_GetEntityByID(&engine_world, id);
    if(e2 == NULL)
    {
        Con_Warning(SYSWARN_NO_ENTITY, id);
        return 0;
    }

    lua_pushnumber(lua, e2->transform[12+0] - e1->transform[12+0]);
    lua_pushnumber(lua, e2->transform[12+1] - e1->transform[12+1]);
    lua_pushnumber(lua, e2->transform[12+2] - e1->transform[12+2]);
    return 3;
}

int lua_GetEntityDistance(lua_State * lua)
{
    if(lua_gettop(lua) < 2)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[id1], [id2]");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p e1 = World_GetEntityByID(&engine_world, id);
    if(e1 == NULL)
    {
        Con_Warning(SYSWARN_NO_ENTITY, id);
        return 0;
    }
    id = lua_tointeger(lua, 2);
    entity_p e2 = World_GetEntityByID(&engine_world, id);
    if(e2 == NULL)
    {
        Con_Warning(SYSWARN_NO_ENTITY, id);
        return 0;
    }

    lua_pushnumber(lua, Entity_FindDistance(e1, e2));
    return 1;
}


int lua_GetEntityDirDot(lua_State * lua)
{
    if(lua_gettop(lua) < 2)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[id1], [id2]");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p e1 = World_GetEntityByID(&engine_world, id);
    if(e1 == NULL)
    {
        Con_Warning(SYSWARN_NO_ENTITY, id);
        return 0;
    }
    id = lua_tointeger(lua, 2);
    entity_p e2 = World_GetEntityByID(&engine_world, id);
    if(e2 == NULL)
    {
        Con_Warning(SYSWARN_NO_ENTITY, id);
        return 0;
    }

    lua_pushnumber(lua, vec3_dot(e1->transform + 4, e2->transform + 4));
    return 1;
}


int lua_GetEntityPosition(lua_State * lua)
{
    if(lua_gettop(lua) != 1)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[id]");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(&engine_world, id);
    if(ent == NULL)
    {
        Con_Warning(SYSWARN_NO_ENTITY, id);
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
    switch(lua_gettop(lua))
    {
        case 4:
            {
                int id = lua_tointeger(lua, 1);
                entity_p ent = World_GetEntityByID(&engine_world, id);
                if(ent == NULL)
                {
                    Con_Printf("can not find entity with id = %d", id);
                    return 0;
                }
                ent->transform[12+0] = lua_tonumber(lua, 2);
                ent->transform[12+1] = lua_tonumber(lua, 3);
                ent->transform[12+2] = lua_tonumber(lua, 4);
                if(ent->character)
                {
                    Character_UpdatePlatformPreStep(ent);
                }
            }
            return 0;

        case 7:
            {
                int id = lua_tointeger(lua, 1);
                entity_p ent = World_GetEntityByID(&engine_world, id);
                if(ent == NULL)
                {
                    Con_Printf("can not find entity with id = %d", id);
                    return 0;
                }
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
            }
            return 0;

        default:
            Con_Warning(SYSWARN_WRONG_ARGS, "[id, x, y, z] or [id, x, y, z, fi_x, fi_y, fi_z]");
            return 0;
    }

    return 0;
}


int lua_MoveEntityGlobal(lua_State * lua)
{
    switch(lua_gettop(lua))
    {
        case 4:
            {
                int id = lua_tointeger(lua, 1);
                entity_p ent = World_GetEntityByID(&engine_world, id);
                if(ent == NULL)
                {
                    Con_Printf("can not find entity with id = %d", id);
                    return 0;
                }
                ent->transform[12+0] += lua_tonumber(lua, 2);
                ent->transform[12+1] += lua_tonumber(lua, 3);
                ent->transform[12+2] += lua_tonumber(lua, 4);
                Entity_UpdateRigidBody(ent, 1);
            }
            return 0;

        default:
            Con_Warning(SYSWARN_WRONG_ARGS, "[id, x, y, z]");
            return 0;
    }

    return 0;
}


int lua_MoveEntityLocal(lua_State * lua)
{
    if(lua_gettop(lua) < 4)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[entity_id, dx, dy, dz]");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(&engine_world, id);

    if(ent == NULL)
    {
        Con_Warning(SYSWARN_NO_ENTITY, id);
        return 0;
    }

    btScalar dx = lua_tonumber(lua, 2);
    btScalar dy = lua_tonumber(lua, 3);
    btScalar dz = lua_tonumber(lua, 4);

    ent->transform[12+0] += dx * ent->transform[0+0] + dy * ent->transform[4+0] + dz * ent->transform[8+0];
    ent->transform[12+1] += dx * ent->transform[0+1] + dy * ent->transform[4+1] + dz * ent->transform[8+1];
    ent->transform[12+2] += dx * ent->transform[0+2] + dy * ent->transform[4+2] + dz * ent->transform[8+2];

    Entity_UpdateRigidBody(ent, 1);

    return 0;
}

int lua_MoveEntityToSink(lua_State * lua)
{
    if(lua_gettop(lua) < 2)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[entity_id, sink_id]");
        return 0;
    }

    entity_p ent = World_GetEntityByID(&engine_world, lua_tointeger(lua, 1));
    uint32_t sink_index = lua_tointeger(lua, 2);

    if(sink_index > engine_world.cameras_sinks_count) return 0;
    stat_camera_sink_p sink = &engine_world.cameras_sinks[sink_index];

    btVector3 ent_pos;  ent_pos.m_floats[0] = ent->transform[12+0];
                        ent_pos.m_floats[1] = ent->transform[12+1];
                        ent_pos.m_floats[2] = ent->transform[12+2];

    btVector3 sink_pos; sink_pos.m_floats[0] = sink->x;
                        sink_pos.m_floats[1] = sink->y;

                    if(engine_world.version < TR_II)
                    {
                        sink_pos.m_floats[2] = ent_pos.m_floats[2];
                    }
                    else
                    {
                        sink_pos.m_floats[2] = sink->z + 256.0; // Prevents digging into the floor.
                    }

    btScalar dist = btDistance(ent_pos, sink_pos);
    if(dist == 0.0) dist = 1.0; // Prevents division by zero.

    btVector3 speed = ((sink_pos - ent_pos) / dist) * ((btScalar)(sink->room_or_strength) * 1.5);

    ent->transform[12+0] += speed.m_floats[0];
    ent->transform[12+1] += speed.m_floats[1];
    ent->transform[12+2] += speed.m_floats[2] * 16.0;

    Entity_UpdateRigidBody(ent, 1);

    return 0;
}

int lua_MoveEntityToEntity(lua_State * lua)
{
    if(lua_gettop(lua) < 3)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[entity_to_move_id, entity_id, speed]");
        return 0;
    }

    entity_p ent1 = World_GetEntityByID(&engine_world, lua_tointeger(lua, 1));
    entity_p ent2 = World_GetEntityByID(&engine_world, lua_tointeger(lua, 2));
    btScalar speed_mult = lua_tonumber(lua, 3);

    btVector3 ent1_pos; ent1_pos.m_floats[0] = ent1->transform[12+0];
                        ent1_pos.m_floats[1] = ent1->transform[12+1];
                        ent1_pos.m_floats[2] = ent1->transform[12+2];

    btVector3 ent2_pos; ent2_pos.m_floats[0] = ent2->transform[12+0];
                        ent2_pos.m_floats[1] = ent2->transform[12+1];
                        ent2_pos.m_floats[2] = ent2->transform[12+2];

    btScalar dist = btDistance(ent1_pos, ent2_pos);
    if(dist == 0.0) dist = 1.0; // Prevents division by zero.

    btVector3 speed = ((ent2_pos - ent1_pos) / dist) * speed_mult; // FIXME!

    ent1->transform[12+0] += speed.m_floats[0];
    ent1->transform[12+1] += speed.m_floats[1];
    ent1->transform[12+2] += speed.m_floats[2];
    if(ent1->character) Character_UpdatePlatformPreStep(ent1);
    Entity_UpdateRigidBody(ent1, 1);

    return 0;
}

int lua_GetEntitySpeed(lua_State * lua)
{
    if(lua_gettop(lua) != 1)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[id]");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(&engine_world, id);

    if(ent == NULL)
    {
        Con_Warning(SYSWARN_NO_ENTITY, id);
        return 0;
    }

    lua_pushnumber(lua, ent->speed[0]);
    lua_pushnumber(lua, ent->speed[1]);
    lua_pushnumber(lua, ent->speed[2]);
    return 3;
}


int lua_SetEntitySpeed(lua_State * lua)
{
    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(&engine_world, id);

    if(ent == NULL)
    {
        Con_Warning(SYSWARN_NO_ENTITY, id);
        return 0;
    }

    switch(lua_gettop(lua))
    {
        case 4:
            ent->speed[0] = lua_tonumber(lua, 2);
            ent->speed[1] = lua_tonumber(lua, 3);
            ent->speed[2] = lua_tonumber(lua, 4);
            break;

        default:
            Con_Warning(SYSWARN_WRONG_ARGS, "[id, Vx, Vy, Vz]");
            break;
    }

    return 0;
}


int lua_SetEntityAnim(lua_State * lua)
{
    int top = lua_gettop(lua);

    if(top < 2)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[entity_id, anim_id, (frame_number, another_model)]");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(&engine_world, id);

    if(ent == NULL)
    {
        Con_Warning(SYSWARN_NO_ENTITY, id);
        return 0;
    }

    switch(top)
    {
        case 2:
        default:
            Entity_SetAnimation(ent, lua_tointeger(lua, 2));
            break;
        case 3:
            Entity_SetAnimation(ent, lua_tointeger(lua, 2), lua_tointeger(lua, 3));
            break;
        case 4:
            Entity_SetAnimation(ent, lua_tointeger(lua, 2), lua_tointeger(lua, 3), lua_tointeger(lua, 4));
            break;
    }

    return 0;
}


int lua_GetEntityAnim(lua_State * lua)
{
    if(lua_gettop(lua) < 1)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[entity_id]");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(&engine_world, id);

    if(ent == NULL)
    {
        Con_Warning(SYSWARN_NO_ENTITY, id);
        return 0;
    }

    lua_pushinteger(lua, ent->bf.animations.current_animation);
    lua_pushinteger(lua, ent->bf.animations.current_frame);
    lua_pushinteger(lua, ent->bf.animations.model->animations[ent->bf.animations.current_animation].frames_count);

    return 3;
}


int lua_CanTriggerEntity(lua_State * lua)
{
    int id;
    int top = lua_gettop(lua);
    btScalar pos[3], offset[3], r;

    if(top < 2)
    {
        lua_pushinteger(lua, 0);
        return 1;
    }

    id = lua_tointeger(lua, 1);
    entity_p e1 = World_GetEntityByID(&engine_world, id);
    if(e1 == NULL || !e1->character || !e1->character->cmd.action)
    {
        lua_pushinteger(lua, 0);
        return 1;
    }

    id = lua_tointeger(lua, 2);
    entity_p e2 = World_GetEntityByID(&engine_world, id);
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


int lua_GetEntityVisibility(lua_State * lua)
{
    if(lua_gettop(lua) < 1)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[entity_id]");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(&engine_world, id);

    if(ent == NULL)
    {
        Con_Warning(SYSWARN_NO_ENTITY, id);
        return 0;
    }

    lua_pushinteger(lua, (ent->state_flags & ENTITY_STATE_VISIBLE) != 0);

    return 1;
}

int lua_SetEntityVisibility(lua_State * lua)
{
    if(lua_gettop(lua) < 2)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[entity_id, value]");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(&engine_world, id);

    if(ent == NULL)
    {
        Con_Warning(SYSWARN_NO_ENTITY, id);
        return 0;
    }

    if(lua_tointeger(lua, 2) != 0)
    {
        ent->state_flags |= ENTITY_STATE_VISIBLE;
    }
    else
    {
        ent->state_flags &= ~ENTITY_STATE_VISIBLE;
    }

    return 0;
}


int lua_GetEntityEnability(lua_State * lua)
{
    if(lua_gettop(lua) < 1)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[entity_id]");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(&engine_world, id);

    if(ent == NULL)
    {
        Con_Warning(SYSWARN_NO_ENTITY, id);
        return 0;
    }

    lua_pushinteger(lua, (ent->state_flags & ENTITY_STATE_ENABLED) != 0);

    return 1;
}


int lua_GetEntityActivity(lua_State * lua)
{
    if(lua_gettop(lua) < 1)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[entity_id]");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(&engine_world, id);

    if(ent == NULL)
    {
        Con_Warning(SYSWARN_NO_ENTITY, id);
        return 0;
    }

    lua_pushinteger(lua, (ent->state_flags & ENTITY_STATE_ACTIVE) != 0);

    return 1;
}


int lua_SetEntityActivity(lua_State * lua)
{
    if(lua_gettop(lua) < 2)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[entity_id, value]");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(&engine_world, id);

    if(ent == NULL)
    {
        Con_Warning(SYSWARN_NO_ENTITY, id);
        return 0;
    }

    if(lua_tointeger(lua, 2) != 0)
    {
        ent->state_flags |= ENTITY_STATE_ACTIVE;
    }
    else
    {
        ent->state_flags &= ~ENTITY_STATE_ACTIVE;
    }

    return 0;
}


int lua_GetEntityTriggerLayout(lua_State *lua)
{
    if(lua_gettop(lua) < 1) return 0;

    entity_p ent = World_GetEntityByID(&engine_world, lua_tointeger(lua, 1));
    if(ent == NULL) return 0;   // No entity found - return.

    lua_pushinteger(lua, (ent->trigger_layout & ENTITY_TLAYOUT_MASK));          // mask
    lua_pushinteger(lua, (ent->trigger_layout & ENTITY_TLAYOUT_EVENT) >> 5);    // event
    lua_pushinteger(lua, (ent->trigger_layout & ENTITY_TLAYOUT_LOCK) >> 6);     // lock

    return 3;
}

int lua_SetEntityTriggerLayout(lua_State *lua)
{
    int top = lua_gettop(lua);

    if(top < 2)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[entity_id, layout] or [entity_id, mask, event, once] / %d");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(&engine_world, id);

    if(ent == NULL)
    {
        Con_Warning(SYSWARN_NO_ENTITY, id);
        return 0;
    }

    if(top == 2)
    {
        ent->trigger_layout = (uint8_t)lua_tointeger(lua, 2);
    }
    else if(top == 4)
    {
        uint8_t trigger_layout = ent->trigger_layout;
        trigger_layout &= ~(uint8_t)(ENTITY_TLAYOUT_MASK);  trigger_layout ^= (uint8_t)lua_tointeger(lua, 2);          // mask  - 00011111
        trigger_layout &= ~(uint8_t)(ENTITY_TLAYOUT_EVENT); trigger_layout ^= ((uint8_t)lua_tointeger(lua, 3)) << 5;   // event - 00100000
        trigger_layout &= ~(uint8_t)(ENTITY_TLAYOUT_LOCK);  trigger_layout ^= ((uint8_t)lua_tointeger(lua, 4)) << 6;   // lock  - 01000000
        ent->trigger_layout = trigger_layout;
    }

    return 0;
}

int lua_SetEntityLock(lua_State * lua)
{
    if(lua_gettop(lua) < 2) return 0;   // No arguments provided - return.

    entity_p ent = World_GetEntityByID(&engine_world, lua_tointeger(lua, 1));
    if(ent != NULL)
    {
        uint8_t trigger_layout = ent->trigger_layout;
        trigger_layout &= ~(uint8_t)(ENTITY_TLAYOUT_LOCK);  trigger_layout ^= ((uint8_t)lua_tointeger(lua, 2)) << 6;   // lock  - 01000000
        ent->trigger_layout = trigger_layout;
    }
    return 0;
}

int lua_GetEntityLock(lua_State * lua)
{
    if(lua_gettop(lua) < 1) return 0;   // No argument provided - return.

    entity_p ent = World_GetEntityByID(&engine_world, lua_tointeger(lua, 1));
    if(ent != NULL)
    {
        lua_pushinteger(lua, ((ent->trigger_layout & ENTITY_TLAYOUT_LOCK) >> 6));      // lock
        return 1;
    }
    return 0;
}

int lua_SetEntityEvent(lua_State * lua)
{
    if(lua_gettop(lua) < 2) return 0;   // No arguments provided - return.

    entity_p ent = World_GetEntityByID(&engine_world, lua_tointeger(lua, 1));
    if(ent != NULL)
    {
        uint8_t trigger_layout = ent->trigger_layout;
        trigger_layout &= ~(uint8_t)(ENTITY_TLAYOUT_EVENT); trigger_layout ^= ((uint8_t)lua_tointeger(lua, 2)) << 5;   // event - 00100000
        ent->trigger_layout = trigger_layout;
    }
    return 0;
}

int lua_GetEntityEvent(lua_State *lua)
{
    if(lua_gettop(lua) < 1) return 0;   // No argument provided - return.

    entity_p ent = World_GetEntityByID(&engine_world, lua_tointeger(lua, 1));
    if(ent != NULL)
    {
        lua_pushinteger(lua, ((ent->trigger_layout & ENTITY_TLAYOUT_EVENT) >> 5));    // event
        return 1;
    }
    return 0;
}

int lua_GetEntityMask(lua_State * lua)
{
    if(lua_gettop(lua) < 1) return 0;   // No argument provided - return.

    entity_p ent = World_GetEntityByID(&engine_world, lua_tointeger(lua, 1));
    if(ent != NULL)
    {
        lua_pushinteger(lua, (ent->trigger_layout & ENTITY_TLAYOUT_MASK));          // mask
        return 1;
    }
    return 0;
}

int lua_SetEntityMask(lua_State * lua)
{
    if(lua_gettop(lua) < 2) return 0;   // No arguments provided - return.

    entity_p ent = World_GetEntityByID(&engine_world, lua_tointeger(lua, 1));
    if(ent != NULL)
    {
        uint8_t trigger_layout = ent->trigger_layout;
        trigger_layout &= ~(uint8_t)(ENTITY_TLAYOUT_MASK);  trigger_layout ^= (uint8_t)lua_tointeger(lua, 2);   // mask  - 00011111
        ent->trigger_layout = trigger_layout;
    }
    return 0;
}

int lua_GetEntitySectorStatus(lua_State *lua)
{
    if(lua_gettop(lua) < 1) return 0;

    entity_p ent = World_GetEntityByID(&engine_world, lua_tonumber(lua, 1));
    if(ent != NULL)
    {
        lua_pushinteger(lua, ((ent->trigger_layout & ENTITY_TLAYOUT_SSTATUS) >> 7));
        return 1;
    }
    return 0;
}

int lua_SetEntitySectorStatus(lua_State *lua)
{
    if(lua_gettop(lua) < 2) return 0;   // No arguments specified - return.

    entity_p ent = World_GetEntityByID(&engine_world, lua_tonumber(lua, 1));
    if(ent != NULL)
    {
        uint8_t trigger_layout = ent->trigger_layout;
        trigger_layout &= ~(uint8_t)(ENTITY_TLAYOUT_SSTATUS);
        trigger_layout ^=  ((uint8_t)lua_tointeger(lua, 2)) << 7;   // sector_status  - 10000000
        ent->trigger_layout = trigger_layout;
    }
    return 0;
}

int lua_GetEntityOCB(lua_State * lua)
{
    if(lua_gettop(lua) < 1) return 0;   // No argument provided - return.

    entity_p ent = World_GetEntityByID(&engine_world, lua_tointeger(lua, 1));
    if(ent == NULL) return 0;   // No entity found - return.

    lua_pushinteger(lua, ent->OCB);
    return 1;
}


int lua_SetEntityOCB(lua_State * lua)
{
    if(lua_gettop(lua) < 2) return 0;   // No arguments provided - return.

    entity_p ent = World_GetEntityByID(&engine_world, lua_tointeger(lua, 1));
    if(ent == NULL) return 0;   // No entity found - return.

    ent->OCB = lua_tointeger(lua, 2);
    return 0;
}

int lua_GetEntityFlags(lua_State * lua)
{
    if(lua_gettop(lua) < 1)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[entity_id]");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(&engine_world, id);

    if(ent == NULL)
    {
        Con_Warning(SYSWARN_NO_ENTITY, id);
        return 0;
    }

    lua_pushinteger(lua, ent->state_flags);
    lua_pushinteger(lua, ent->type_flags);
    lua_pushinteger(lua, ent->callback_flags);

    return 3;
}

int lua_SetEntityTypeFlag(lua_State *lua)
{
    if(lua_gettop(lua) < 2)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[entity_id, type_flags]");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(&engine_world, id);

    if(ent == NULL)
    {
        Con_Warning(SYSWARN_NO_ENTITY, id);
        return 0;
    }

    ent->type_flags ^= (uint16_t)lua_tointeger(lua, 2);
    return 0;
}

int lua_SetEntityFlags(lua_State * lua)
{
    if(lua_gettop(lua) < 3)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[entity_id, state_flags, type_flags, (callback_flags)]");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(&engine_world, id);

    if(ent == NULL)
    {
        Con_Warning(SYSWARN_NO_ENTITY, id);
        return 0;
    }

    if(!lua_isnil(lua, 2))
    {
        ent->state_flags = lua_tointeger(lua, 2);
    }
    if(!lua_isnil(lua, 3))
    {
        ent->type_flags = lua_tointeger(lua, 3);
    }
    if(!lua_isnil(lua, 4))
    {
        ent->callback_flags = lua_tointeger(lua, 4);
    }

    return 0;
}

int lua_GetEntityTimer(lua_State * lua)
{
    if(lua_gettop(lua) < 1) return 0;   // No arguments provided - return.

    entity_p ent = World_GetEntityByID(&engine_world, lua_tointeger(lua, 1));
    if(ent == NULL) return 0;   // No entity found - return.

    lua_pushnumber(lua, ent->timer);
    return 1;
}

int lua_SetEntityTimer(lua_State * lua)
{
    if(lua_gettop(lua) < 2) return 0;   // No arguments provided - return.

    entity_p ent = World_GetEntityByID(&engine_world, lua_tointeger(lua, 1));
    if(ent == NULL) return 0;   // No entity found - return.

    ent->timer = lua_tonumber(lua, 2);
    return 0;
}

int lua_GetEntityMoveType(lua_State * lua)
{
    if(lua_gettop(lua) < 1)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[entity_id]");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(&engine_world, id);

    if(ent == NULL)
    {
        Con_Warning(SYSWARN_NO_ENTITY, id);
        return 0;
    }

    lua_pushinteger(lua, ent->move_type);

    return 1;
}

int lua_SetEntityMoveType(lua_State * lua)
{
    if(lua_gettop(lua) < 2)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[entity_id, move_type]");
        return 0;
    }

    entity_p ent = World_GetEntityByID(&engine_world, lua_tointeger(lua, 1));

    if(ent == NULL) return 0;
    ent->move_type = lua_tointeger(lua, 2);

    return 0;
}

int lua_GetEntityState(lua_State * lua)
{
    if(lua_gettop(lua) < 1)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[entity_id]");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(&engine_world, id);

    if(ent == NULL)
    {
        Con_Warning(SYSWARN_NO_ENTITY, id);
        return 0;
    }

    lua_pushinteger(lua, ent->bf.animations.last_state);

    return 1;
}

int lua_GetEntityModel(lua_State * lua)
{
    if(lua_gettop(lua) < 1)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[entity_id]");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(&engine_world, id);

    if(ent == NULL)
    {
        Con_Warning(SYSWARN_NO_ENTITY, id);
        return 0;
    }

    lua_pushinteger(lua, ent->bf.animations.model->id);

    return 1;
}

int lua_SetEntityState(lua_State * lua)
{
    if(lua_gettop(lua) < 2)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[entity_id, value]");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(&engine_world, id);

    if(ent == NULL)
    {
        Con_Warning(SYSWARN_NO_ENTITY, id);
        return 0;
    }

    ent->bf.animations.next_state = lua_tointeger(lua, 2);
    if(!lua_isnil(lua, 3))
    {
        ent->bf.animations.last_state = lua_tointeger(lua, 3);
    }

    return 0;
}

int lua_SetEntityRoomMove(lua_State * lua)
{
    if(lua_gettop(lua) < 4)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[id, room_id, move_type, dir_flag]");
        return 0;
    }

    uint32_t id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(&engine_world, id);
    if(ent == NULL)
    {
        Con_Warning(SYSWARN_NO_ENTITY, id);
        return 0;
    }

    uint32_t room = lua_tointeger(lua, 2);
    if(!lua_isnil(lua, 2) && (room < engine_world.room_count))
    {
        room_p r = engine_world.rooms + room;
        if(ent == engine_world.Character)
        {
            ent->self->room = r;
        }
        else if(ent->self->room != r)
        {
            if(ent->self->room != NULL)
            {
                Room_RemoveEntity(ent->self->room, ent);
            }
            Room_AddEntity(r, ent);
        }
    }
    Entity_UpdateRoomPos(ent);

    if(!lua_isnil(lua, 3))
    {
        ent->move_type = lua_tointeger(lua, 3);
    }
    if(!lua_isnil(lua, 4))
    {
        ent->dir_flag = lua_tointeger(lua, 4);
    }

    return 0;
}


int lua_SetEntityMeshswap(lua_State * lua)
{
    if(lua_gettop(lua) < 2)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[id_dest, id_src]");
        return 0;
    }

    int id_dest = lua_tointeger(lua, 1);
    int id_src = lua_tointeger(lua, 2);

    entity_p         ent_dest;
    skeletal_model_p model_src;

    ent_dest   = World_GetEntityByID(&engine_world, id_dest);
    model_src  = World_GetModelByID(&engine_world, id_src);

    int meshes_to_copy = (ent_dest->bf.bone_tag_count > model_src->mesh_count)?(model_src->mesh_count):(ent_dest->bf.bone_tag_count);

    for(int i = 0; i < meshes_to_copy; i++)
    {
        ent_dest->bf.bone_tags[i].mesh_base = model_src->mesh_tree[i].mesh_base;
        ent_dest->bf.bone_tags[i].mesh_skin = model_src->mesh_tree[i].mesh_skin;
    }

    return 0;
}

int lua_SetModelMeshReplaceFlag(lua_State *lua)
{
    if(lua_gettop(lua) < 3)
    {
        Con_Printf("Wrong arguments count. Must be (id_model, bone_num, flag)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    skeletal_model_p sm = World_GetModelByID(&engine_world, id);
    if(sm != NULL)
    {
        int bone = lua_tointeger(lua, 2);
        if((bone >= 0) && (bone < sm->mesh_count))
        {
            sm->mesh_tree[bone].replace_mesh = lua_tointeger(lua, 3);
        }
        else
        {
            Con_Printf("wrong bone number = %d", bone);
        }
    }
    else
    {
        Con_Printf("can not find model with id = %d", id);
    }

    return 0;
}

int lua_SetModelAnimReplaceFlag(lua_State *lua)
{
    if(lua_gettop(lua) < 3)
    {
        Con_Printf("Wrong arguments count. Must be (id_model, bone_num, flag)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    skeletal_model_p sm = World_GetModelByID(&engine_world, id);
    if(sm != NULL)
    {
        int bone = lua_tointeger(lua, 2);
        if((bone >= 0) && (bone < sm->mesh_count))
        {
            sm->mesh_tree[bone].replace_anim = lua_tointeger(lua, 3);
        }
        else
        {
            Con_Printf("wrong bone number = %d", bone);
        }
    }
    else
    {
        Con_Printf("can not find model with id = %d", id);
    }

    return 0;
}

int lua_CopyMeshFromModelToModel(lua_State *lua)
{
    if(lua_gettop(lua) < 4)
    {
        Con_Printf("Wrong arguments count. Must be (id_model1, id_model2, bone_num1, bone_num2)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    skeletal_model_p sm1 = World_GetModelByID(&engine_world, id);
    if(sm1 == NULL)
    {
        Con_Printf("can not find model with id = %d", id);
        return 0;
    }

    id = lua_tointeger(lua, 2);
    skeletal_model_p sm2 = World_GetModelByID(&engine_world, id);
    if(sm2 == NULL)
    {
        Con_Printf("can not find model with id = %d", id);
        return 0;
    }

    int bone1 = lua_tointeger(lua, 3);
    int bone2 = lua_tointeger(lua, 4);

    if((bone1 >= 0) && (bone1 < sm1->mesh_count) && (bone2 >= 0) && (bone2 < sm2->mesh_count))
    {
        sm1->mesh_tree[bone1].mesh_base = sm2->mesh_tree[bone2].mesh_base;
    }
    else
    {
        Con_AddLine("wrong bone number = %d");
    }

    return 0;
}

int lua_SetCharacterWeaponModel(lua_State *lua)
{
    if(lua_gettop(lua) < 3)
    {
        Con_Printf("Wrong arguments count. Must be (id_entity, id_weapon_model, armed_state)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(&engine_world, id);
    if(ent != NULL)
    {
        Character_SetWeaponModel(ent, lua_tointeger(lua, 2), lua_tointeger(lua, 3));
    }
    else
    {
        Con_Printf("can not find entity with id = %d", id);
    }

    return 0;
}

/*
 * Camera functions
 */

int lua_CamShake(lua_State *lua)
{
    if(lua_gettop(lua) != 2) return 0;

    float power = lua_tonumber(lua, 1);
    float time  = lua_tonumber(lua, 2);
    Cam_Shake(renderer.cam, power, time);

    return 0;
}

int lua_FlashSetup(lua_State *lua)
{
    if(lua_gettop(lua) != 6) return 0;

    Gui_FadeSetup(FADER_EFFECT,
                  (uint8_t)(lua_tointeger(lua, 1)),
                  (uint8_t)(lua_tointeger(lua, 2)), (uint8_t)(lua_tointeger(lua, 3)), (uint8_t)(lua_tointeger(lua, 4)),
                  BM_MULTIPLY,
                  (uint16_t)(lua_tointeger(lua, 5)), (uint16_t)(lua_tointeger(lua, 6)));
    return 0;
}

int lua_FlashStart(lua_State *lua)
{
    Gui_FadeStart(FADER_EFFECT, GUI_FADER_DIR_TIMED);
    return 0;
}

int lua_FadeOut(lua_State *lua)
{
    Gui_FadeStart(FADER_BLACK, GUI_FADER_DIR_OUT);
    return 0;
}

int lua_FadeIn(lua_State *lua)
{
    Gui_FadeStart(FADER_BLACK, GUI_FADER_DIR_IN);
    return 0;
}

int lua_FadeCheck(lua_State *lua)
{

    lua_pushinteger(lua, Gui_FadeCheck(FADER_BLACK));
    return 1;
}

/*
 * General gameplay functions
 */

int lua_PlayStream(lua_State *lua)
{
    int top = lua_gettop(lua);

    if(top < 1)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[id] or [id, mask].");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    uint8_t mask = 0;
    if(top >= 2) mask = lua_tointeger(lua, 2);

    if(id < 0)
    {
        Con_Warning(SYSWARN_WRONG_STREAM_ID);
        return 0;
    }

    if(mask)
    {
        Audio_StreamPlay(id, mask);
    }
    else
    {
        Audio_StreamPlay(id);
    }

    return 0;
}

int lua_PlaySound(lua_State *lua)
{
    int top = lua_gettop(lua);

    if(top < 1)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[sound_id], (entity_id)");
        return 0;
    }

    uint32_t id  = lua_tointeger(lua, 1);
    if(id >= engine_world.audio_map_count)
    {
        Con_Warning(SYSWARN_WRONG_SOUND_ID, engine_world.audio_map_count);
        return 0;
    }

    int ent_id = -1;

    if(top >= 2)
    {
        ent_id = lua_tointeger(lua, 2);
        if(World_GetEntityByID(&engine_world, ent_id) == NULL) ent_id = -1;
    }

    int result;

    if(ent_id >= 0)
    {
        result = Audio_Send(id, TR_AUDIO_EMITTER_ENTITY, ent_id);
    }
    else
    {
        result = Audio_Send(id, TR_AUDIO_EMITTER_GLOBAL);
    }

    if(result < 0)
    {
        switch(result)
        {
            case TR_AUDIO_SEND_NOCHANNEL:
                Con_Warning(SYSWARN_AS_NOCHANNEL);
                break;

            case TR_AUDIO_SEND_NOSAMPLE:
                Con_Warning(SYSWARN_AS_NOSAMPLE);
                break;
        }
    }

    return 0;
}


int lua_StopSound(lua_State *lua)
{
    int top = lua_gettop(lua);

    if(top < 1)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[sound_id], (entity_id)");
        return 0;
    }

    uint32_t id  = lua_tointeger(lua, 1);
    if(id >= engine_world.audio_map_count)
    {
        Con_Warning(SYSWARN_WRONG_SOUND_ID, engine_world.audio_map_count);
        return 0;
    }

    int ent_id = -1;

    if(top > 1)
    {
        ent_id = lua_tointeger(lua, 2);
        if(World_GetEntityByID(&engine_world, ent_id) == NULL) ent_id = -1;
    }

    int result;

    if(ent_id == -1)
    {
        result = Audio_Kill(id, TR_AUDIO_EMITTER_GLOBAL);
    }
    else
    {
        result = Audio_Kill(id, TR_AUDIO_EMITTER_ENTITY, ent_id);
    }

    if(result < 0) Con_Warning(SYSWARN_AK_NOTPLAYED, id);

    return 0;
}

int lua_GetLevel(lua_State *lua)
{
    lua_pushinteger(lua, gameflow_manager.CurrentLevelID);
    return 1;
}

int lua_SetLevel(lua_State *lua)
{
    if(lua_gettop(lua) != 1)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[level_id]");
        return 0;
    }

    int id  = lua_tointeger(lua, 1);
    Con_Notify(SYSNOTE_CHANGING_LEVEL, id);

    Game_LevelTransition(id);
    Gameflow_Send(TR_GAMEFLOW_OP_LEVELCOMPLETE, id);    // Next level

    return 0;
}

int lua_SetGame(lua_State *lua)
{
    int top = lua_gettop(lua);
    if(top < 1)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[gameversion], (level_id)");
        return 0;
    }

    gameflow_manager.CurrentGameID = lua_tointeger(lua, 1);
    if(!lua_isnil(lua, 2))
    {
        gameflow_manager.CurrentLevelID = lua_tointeger(lua, 2);
    }

    lua_getglobal(lua, "getTitleScreen");
    if(lua_isfunction(lua, -1))
    {
        lua_pushnumber(lua, gameflow_manager.CurrentGameID);
        if (lua_CallAndLog(lua, 1, 1, 0))
        {
            Gui_FadeAssignPic(FADER_LOADSCREEN, lua_tostring(lua, -1));
            lua_pop(lua, 1);
            Gui_FadeStart(FADER_LOADSCREEN, GUI_FADER_DIR_OUT);
        }
    }
    lua_settop(lua, top);

    Con_Notify(SYSNOTE_CHANGING_GAME, gameflow_manager.CurrentGameID);
    Game_LevelTransition(gameflow_manager.CurrentLevelID);
    Gameflow_Send(TR_GAMEFLOW_OP_LEVELCOMPLETE, gameflow_manager.CurrentLevelID);

    return 0;
}

int lua_LoadMap(lua_State *lua)
{
    if(lua_gettop(lua) < 1)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[map_name], (game_id, map_id)");
        return 0;
    }

    if(lua_isstring(lua, 1))
    {
        const char *s = lua_tostring(lua, 1);
        if((s != NULL) && (s[0] != 0) && (strcmp(s, gameflow_manager.CurrentLevelPath) != 0))
        {
            if(!lua_isnil(lua, 2))
            {
                gameflow_manager.CurrentGameID = lua_tointeger(lua, 2);
            }
            if(!lua_isnil(lua, 3))
            {
                gameflow_manager.CurrentLevelID = lua_tointeger(lua, 3);
            }
            char file_path[MAX_ENGINE_PATH];
            lua_GetLoadingScreen(lua, gameflow_manager.CurrentLevelID, file_path);
            Gui_FadeAssignPic(FADER_LOADSCREEN, file_path);
            Gui_FadeStart(FADER_LOADSCREEN, GUI_FADER_DIR_IN);
            Engine_LoadMap(s);
        }
    }

    return 0;
}


/*
 * Flipped (alternate) room functions
 */

int lua_SetFlipState(lua_State *lua)
{
    if(lua_gettop(lua) != 2)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[flip_index, flip_state]");
        return 0;
    }

    uint32_t group = (uint32_t)lua_tointeger(lua, 1);
    uint32_t state = (uint32_t)lua_tointeger(lua, 2);
             state = (state > 1)?(1):(state);       // State is always boolean.

    if(group >= engine_world.flip_count)
    {
        Con_Warning(SYSWARN_WRONG_FLIPMAP_INDEX);
        return 0;
    }

    if(engine_world.flip_map[group] == 0x1F)         // Check flipmap state.
    {
        room_p current_room = engine_world.rooms;

        if(engine_world.version > TR_III)
        {
            for(int i=0;i<engine_world.room_count;i++, current_room++)
            {
                if(current_room->alternate_group == group)    // Check if group is valid.
                {
                    if(state)
                    {
                        Room_SwapToAlternate(current_room);
                    }
                    else
                    {
                        Room_SwapToBase(current_room);
                    }
                }
            }

            engine_world.flip_state[group] = state;
        }
        else
        {
            for(int i=0;i<engine_world.room_count;i++,current_room++)
            {
                if(state)
                {
                    Room_SwapToAlternate(current_room);
                }
                else
                {
                    Room_SwapToBase(current_room);
                }
            }

            engine_world.flip_state[0] = state;    // In TR1-3, state is always global.
        }
    }

    return 0;
}

int lua_SetFlipMap(lua_State *lua)
{
    if(lua_gettop(lua) != 3)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[flip_index, flip_mask, flip_operation]");
        return 0;
    }

    uint32_t group = (uint32_t)lua_tointeger(lua, 1);
    uint8_t  mask  = (uint8_t)lua_tointeger(lua, 2);
    uint8_t  op    = (uint8_t)lua_tointeger(lua, 3);
             op    = (mask > AMASK_OP_XOR)?(AMASK_OP_XOR):(AMASK_OP_OR);

    if(group >= engine_world.flip_count)
    {
        Con_Warning(SYSWARN_WRONG_FLIPMAP_INDEX);
        return 0;
    }

    if(op == AMASK_OP_XOR)
    {
        engine_world.flip_map[group] ^= mask;
    }
    else
    {
        engine_world.flip_map[group] |= mask;
    }

    return 0;
}

int lua_GetFlipMap(lua_State *lua)
{
    if(lua_gettop(lua) == 1)
    {
        uint32_t group = (uint32_t)lua_tointeger(lua, 1);

        if(group >= engine_world.flip_count)
        {
            Con_Warning(SYSWARN_WRONG_FLIPMAP_INDEX);
            return 0;
        }

        lua_pushinteger(lua, engine_world.flip_map[group]);
        return 1;
    }
    else
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[flip_index]");
        return 0;
    }
}

int lua_GetFlipState(lua_State *lua)
{
    if(lua_gettop(lua) == 1)
    {
        uint32_t group = (uint32_t)lua_tointeger(lua, 1);

        if(group >= engine_world.flip_count)
        {
            Con_Warning(SYSWARN_WRONG_FLIPMAP_INDEX);
            return 0;
        }

        lua_pushinteger(lua, engine_world.flip_state[group]);
        return 1;
    }
    else
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[flip_index]");
        return 0;
    }
}

/*
 * Generate UV rotate animations
 */

int lua_genUVRotateAnimation(lua_State *lua)
{
    if(lua_gettop(lua) < 1)
    {
        Con_Warning(SYSWARN_WRONG_ARGS, "[model_id]", FONTSTYLE_CONSOLE_WARNING);
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    skeletal_model_p model = World_GetModelByID(&engine_world, id);

    if(model != NULL)
    {
        polygon_p p=model->mesh_tree->mesh_base->transparency_polygons;
        if((p != NULL) && (p->anim_id == 0))
        {
            engine_world.anim_sequences_count++;
            engine_world.anim_sequences = (anim_seq_p)realloc(engine_world.anim_sequences, engine_world.anim_sequences_count * sizeof(anim_seq_t));
            anim_seq_p seq = engine_world.anim_sequences + engine_world.anim_sequences_count - 1;

            // Fill up new sequence with frame list.
            seq->anim_type         = TR_ANIMTEXTURE_FORWARD;
            seq->frame_lock        = false; // by default anim is playing
            seq->uvrotate          = true;
            seq->reverse_direction = false; // Needed for proper reverse-type start-up.
            seq->frame_rate        = 0.05;  // Should be passed as 1 / FPS.
            seq->frame_time        = 0.0;   // Reset frame time to initial state.
            seq->current_frame     = 0;     // Reset current frame to zero.
            seq->frames_count      = 8;
            seq->frame_list        = (uint32_t*)calloc(seq->frames_count, sizeof(uint32_t));
            seq->frame_list[0]     = 0;
            seq->frames            = (tex_frame_p)calloc(seq->frames_count, sizeof(tex_frame_t));

            btScalar v_min, v_max;
            v_min = v_max = p->vertices->tex_coord[1];
            for(uint16_t j=1;j<p->vertex_count;j++)
            {
                if(p->vertices[j].tex_coord[1] > v_max)
                {
                    v_max = p->vertices[j].tex_coord[1];
                }
                if(p->vertices[j].tex_coord[1] < v_min)
                {
                    v_min = p->vertices[j].tex_coord[1];
                }
            }

            seq->uvrotate_max = 0.5 * (v_max - v_min);
            seq->uvrotate_speed = seq->uvrotate_max / (btScalar)seq->frames_count;
            for(uint16_t j=0;j<seq->frames_count;j++)
            {
                seq->frames[j].tex_ind = p->tex_index;
                seq->frames[j].mat[0] = 1.0;
                seq->frames[j].mat[1] = 0.0;
                seq->frames[j].mat[2] = 0.0;
                seq->frames[j].mat[3] = 1.0;
                seq->frames[j].move[0] = 0.0;
                seq->frames[j].move[1] = -((btScalar)j * seq->uvrotate_speed);
            }

            for(;p!=NULL;p=p->next)
            {
                p->anim_id = engine_world.anim_sequences_count;
                for(uint16_t j=0;j<p->vertex_count;j++)
                {
                    p->vertices[j].tex_coord[1] = v_min + 0.5 * (p->vertices[j].tex_coord[1] - v_min) + seq->uvrotate_max;
                }
            }
        }
    }

    return 0;
}

// Called when something goes absolutely horribly wrong in Lua, and tries
// to produce some debug output. Lua calls abort afterwards, so sending
// the output to the internal console is not an option.
static int engine_LuaPanic(lua_State *lua) {
    if (lua_gettop(lua) < 1) {
        fprintf(stderr, "Fatal lua error (no details provided).\n");
    } else {
        fprintf(stderr, "Fatal lua error: %s\n", lua_tostring(lua, 1));
    }
    fflush(stderr);
    return 0;
}

bool Engine_LuaInit()
{
    engine_lua = luaL_newstate();

    if(engine_lua != NULL)
    {
        luaL_openlibs(engine_lua);
        Engine_LuaRegisterFuncs(engine_lua);
        lua_atpanic(engine_lua, engine_LuaPanic);

        // Load and run global engine scripts, except font script, which
        // should be called AFTER OpenGL/SDL are initialized.

        luaL_dofile(engine_lua, "scripts/strings/getstring.lua");
        luaL_dofile(engine_lua, "scripts/system/sys_scripts.lua");
        luaL_dofile(engine_lua, "scripts/gameflow/gameflow.lua");
        luaL_dofile(engine_lua, "scripts/trigger/trigger_functions.lua");
        luaL_dofile(engine_lua, "scripts/trigger/helper_functions.lua");
        luaL_dofile(engine_lua, "scripts/entity/entity_functions.lua");
        luaL_dofile(engine_lua, "scripts/config/control_constants.lua");
        luaL_dofile(engine_lua, "scripts/audio/common_sounds.lua");
        luaL_dofile(engine_lua, "scripts/audio/soundtrack.lua");
        luaL_dofile(engine_lua, "scripts/audio/sample_override.lua");

        return true;
    }
    else
    {
        return false;
    }
}

void Engine_LuaClearTasks()
{
    int top = lua_gettop(engine_lua);
    lua_getglobal(engine_lua, "clearTasks");
    lua_CallAndLog(engine_lua, 0, 0, 0);
    lua_settop(engine_lua, top);
}

void lua_registerc(lua_State *lua, const char* func_name, int(*func)(lua_State*))
{
    char uc[64] = {0}; char lc[64] = {0};
    for(int i=0; i < strlen(func_name); i++)
    {
        lc[i]=tolower(func_name[i]);
        uc[i]=toupper(func_name[i]);
    }

    lua_register(lua, func_name, func);
    lua_register(lua, lc, func);
    lua_register(lua, uc, func);
}


void Engine_LuaRegisterFuncs(lua_State *lua)
{
    /*
     * register globals
     */
    char cvar_init[64]; cvar_init[0] = 0;
    strcat(cvar_init, CVAR_LUA_TABLE_NAME); strcat(cvar_init, " = {};");
    luaL_dostring(lua, cvar_init);

    Game_RegisterLuaFunctions(lua);

    // Register script functions

    lua_registerc(lua, "print", lua_print);
    lua_registerc(lua, "checkStack", lua_CheckStack);
    lua_registerc(lua, "dumpModel", lua_DumpModel);
    lua_registerc(lua, "dumpRoom", lua_DumpRoom);
    lua_registerc(lua, "setRoomEnabled", lua_SetRoomEnabled);

    lua_registerc(lua, "playSound", lua_PlaySound);
    lua_registerc(lua, "stopSound", lua_StopSound);

    lua_registerc(lua, "playStream", lua_PlayStream);

    lua_registerc(lua, "setLevel", lua_SetLevel);
    lua_registerc(lua, "getLevel", lua_GetLevel);

    lua_registerc(lua, "setGame", lua_SetGame);
    lua_registerc(lua, "loadMap", lua_LoadMap);

    lua_register(lua, "camShake", lua_CamShake);

    lua_register(lua, "fadeOut", lua_FadeOut);
    lua_register(lua, "fadeIn", lua_FadeIn);
    lua_register(lua, "fadeCheck", lua_FadeCheck);

    lua_register(lua, "flashSetup", lua_FlashSetup);
    lua_register(lua, "flashStart", lua_FlashStart);

    lua_register(lua, "getLevelVersion", lua_GetLevelVersion);

    lua_register(lua, "setFlipMap", lua_SetFlipMap);
    lua_register(lua, "getFlipMap", lua_GetFlipMap);
    lua_register(lua, "setFlipState", lua_SetFlipState);
    lua_register(lua, "getFlipState", lua_GetFlipState);

    lua_register(lua, "setModelCollisionMapSize", lua_SetModelCollisionMapSize);
    lua_register(lua, "setModelCollisionMap", lua_SetModelCollisionMap);
    lua_register(lua, "getAnimCommandTransform", lua_GetAnimCommandTransform);
    lua_register(lua, "setAnimCommandTransform", lua_SetAnimCommandTransform);
    lua_register(lua, "setStateChangeRange", lua_SetStateChangeRange);

    lua_register(lua, "addItem", lua_AddItem);
    lua_register(lua, "removeItem", lua_RemoveItem);
    lua_register(lua, "removeAllItems", lua_RemoveAllItems);
    lua_register(lua, "getItemsCount", lua_GetItemsCount);
    lua_register(lua, "createBaseItem", lua_CreateBaseItem);
    lua_register(lua, "deleteBaseItem", lua_DeleteBaseItem);
    lua_register(lua, "printItems", lua_PrintItems);

    lua_register(lua, "getModelID", lua_GetModelID);
    lua_register(lua, "canTriggerEntity", lua_CanTriggerEntity);
    lua_register(lua, "spawnEntity", lua_SpawnEntity);
    lua_register(lua, "enableEntity", lua_EnableEntity);
    lua_register(lua, "disableEntity", lua_DisableEntity);

    lua_register(lua, "newSector", lua_NewSector);

    lua_register(lua, "moveEntityGlobal", lua_MoveEntityGlobal);
    lua_register(lua, "moveEntityLocal", lua_MoveEntityLocal);
    lua_register(lua, "moveEntityToSink", lua_MoveEntityToSink);
    lua_register(lua, "moveEntityToEntity", lua_MoveEntityToEntity);

    lua_register(lua, "getEntityVector", lua_GetEntityVector);
    lua_register(lua, "getEntityDirDot", lua_GetEntityDirDot);
    lua_register(lua, "getEntityDistance", lua_GetEntityDistance);
    lua_register(lua, "getEntityPos", lua_GetEntityPosition);
    lua_register(lua, "setEntityPos", lua_SetEntityPosition);
    lua_register(lua, "getEntitySpeed", lua_GetEntitySpeed);
    lua_register(lua, "setEntitySpeed", lua_SetEntitySpeed);
    lua_register(lua, "setEntityCollision", lua_SetEntityCollision);
    lua_register(lua, "getEntityAnim", lua_GetEntityAnim);
    lua_register(lua, "setEntityAnim", lua_SetEntityAnim);
    lua_register(lua, "getEntityModel", lua_GetEntityModel);
    lua_register(lua, "getEntityVisibility", lua_GetEntityVisibility);
    lua_register(lua, "setEntityVisibility", lua_SetEntityVisibility);
    lua_register(lua, "getEntityActivity", lua_GetEntityActivity);
    lua_register(lua, "setEntityActivity", lua_SetEntityActivity);
    lua_register(lua, "getEntityEnability", lua_GetEntityEnability);
    lua_register(lua, "getEntityOCB", lua_GetEntityOCB);
    lua_register(lua, "setEntityOCB", lua_SetEntityOCB);
    lua_register(lua, "getEntityTimer", lua_GetEntityTimer);
    lua_register(lua, "setEntityTimer", lua_SetEntityTimer);
    lua_register(lua, "getEntityFlags", lua_GetEntityFlags);
    lua_register(lua, "setEntityFlags", lua_SetEntityFlags);
    lua_register(lua, "setEntityTypeFlag", lua_SetEntityTypeFlag);
    lua_register(lua, "getEntityState", lua_GetEntityState);
    lua_register(lua, "setEntityState", lua_SetEntityState);
    lua_register(lua, "setEntityRoomMove", lua_SetEntityRoomMove);
    lua_register(lua, "getEntityMoveType", lua_GetEntityMoveType);
    lua_register(lua, "setEntityMoveType", lua_SetEntityMoveType);
    lua_register(lua, "setEntityMeshswap", lua_SetEntityMeshswap);
    lua_register(lua, "setModelMeshReplaceFlag", lua_SetModelMeshReplaceFlag);
    lua_register(lua, "setModelAnimReplaceFlag", lua_SetModelAnimReplaceFlag);
    lua_register(lua, "copyMeshFromModelToModel", lua_CopyMeshFromModelToModel);

    lua_register(lua, "getEntityTriggerLayout", lua_GetEntityTriggerLayout);
    lua_register(lua, "setEntityTriggerLayout", lua_SetEntityTriggerLayout);
    lua_register(lua, "getEntityMask", lua_GetEntityMask);
    lua_register(lua, "setEntityMask", lua_SetEntityMask);
    lua_register(lua, "getEntityEvent", lua_GetEntityEvent);
    lua_register(lua, "setEntityEvent", lua_SetEntityEvent);
    lua_register(lua, "getEntityLock", lua_GetEntityLock);
    lua_register(lua, "setEntityLock", lua_SetEntityLock);
    lua_register(lua, "getEntitySectorStatus", lua_GetEntitySectorStatus);
    lua_register(lua, "setEntitySectorStatus", lua_SetEntitySectorStatus);

    lua_register(lua, "getEntityActivationOffset", lua_GetActivationOffset);
    lua_register(lua, "setEntityActivationOffset", lua_SetActivationOffset);
    lua_register(lua, "getEntitySectorIndex", lua_GetEntitySectorIndex);
    lua_register(lua, "getEntitySectorFlags", lua_GetEntitySectorFlags);
    lua_register(lua, "getEntitySectorMaterial", lua_GetEntitySectorMaterial);

    lua_register(lua, "getCharacterParam", lua_GetCharacterParam);
    lua_register(lua, "setCharacterParam", lua_SetCharacterParam);
    lua_register(lua, "changeCharacterParam", lua_ChangeCharacterParam);
    lua_register(lua, "setCharacterWeaponModel", lua_SetCharacterWeaponModel);
    lua_register(lua, "getCharacterCombatMode", lua_GetCharacterCombatMode);

    lua_register(lua, "getSecretStatus", lua_GetSecretStatus);
    lua_register(lua, "setSecretStatus", lua_SetSecretStatus);

    lua_register(lua, "getActionState", lua_GetActionState);
    lua_register(lua, "getActionChange", lua_GetActionChange);

    lua_register(lua, "genUVRotateAnimation", lua_genUVRotateAnimation);

    lua_register(lua, "getGravity", lua_GetGravity);
    lua_register(lua, "setGravity", lua_SetGravity);
    lua_register(lua, "dropEntity", lua_DropEntity);
    lua_register(lua, "bind", lua_BindKey);

    lua_register(lua, "addFont", lua_AddFont);
    lua_register(lua, "deleteFont", lua_DeleteFont);
    lua_register(lua, "addFontStyle", lua_AddFontStyle);
    lua_register(lua, "deleteFontStyle", lua_DeleteFontStyle);
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

    /* no more renderings */
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

    if(al_context)  // T4Larson <t4larson@gmail.com>: fixed
    {
        alcMakeContextCurrent(NULL);
        alcDestroyContext(al_context);
    }

    if(al_device)
    {
        alcCloseDevice(al_device);
    }

    /* free temporary memory */
    if(frame_vertex_buffer)
    {
        free(frame_vertex_buffer);
    }
    frame_vertex_buffer = NULL;
    frame_vertex_buffer_size = 0;
    frame_vertex_buffer_size_left = 0;

#if !defined(__MACOSX__)
    IMG_Quit();
#endif
    SDL_Quit();

    exit(val);
}


int engine_lua_fputs(const char *str, FILE *f)
{
    Con_AddText(str, FONTSTYLE_CONSOLE_NOTIFY);
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
    Con_AddText(buf, FONTSTYLE_CONSOLE_NOTIFY);

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

    Con_AddText(buf, FONTSTYLE_CONSOLE_NOTIFY);

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
    int i, len, start, ext;

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

void Engine_GetLevelScriptName(int game_version, char *name, const char *postfix)
{
    char level_name[LEVEL_NAME_MAX_LEN];
    Engine_GetLevelName(level_name, gameflow_manager.CurrentLevelPath);

    name[0] = 0;

    strcat(name, "scripts/level/");

    if(game_version < TR_II)
    {
        strcat(name, "tr1/");
    }
    else if(game_version < TR_III)
    {
        strcat(name, "tr2/");
    }
    else if(game_version < TR_IV)
    {
        strcat(name, "tr3/");
    }
    else if(game_version < TR_V)
    {
        strcat(name, "tr4/");
    }
    else
    {
        strcat(name, "tr5/");
    }

    strcat(name, level_name);
    if(postfix) strcat(name, postfix);
    strcat(name, ".lua");
}

int Engine_LoadMap(const char *name)
{
    int trv;
    VT_Level tr_level;
    char buf[LEVEL_NAME_MAX_LEN] = {0x00};
    extern gui_Fader Fader[];

    Gui_DrawLoadScreen(0);

    if(!Engine_FileFound(name))
    {
        Con_Warning(SYSWARN_FILE_NOT_FOUND, name);
        return 0;
    }

    trv = Engine_GetLevelVersion(name);

    if(trv == TR_UNKNOWN)
    {
        return 0;
    }

    renderer.style &= ~R_DRAW_SKYBOX;
    renderer.r_list_active_count = 0;
    renderer.world = NULL;

    strncpy(gameflow_manager.CurrentLevelPath, name, MAX_ENGINE_PATH);          // it is needed for "not in the game" levels or correct saves loading.

    tr_level.read_level(name, trv);
    tr_level.prepare_level();

    //tr_level.dump_textures();

    Gui_DrawLoadScreen(100);

    World_Empty(&engine_world);
    World_Prepare(&engine_world);

    lua_Clean(engine_lua);

    Gui_DrawLoadScreen(150);

    TR_GenWorld(&engine_world, &tr_level);

    engine_world.id   = 0;
    engine_world.name = 0;
    engine_world.type = 0;

    Engine_GetLevelName(buf, name);
    Con_Notify(SYSNOTE_ENGINE_VERSION, trv, buf);
    Con_Notify(SYSNOTE_NUM_ROOMS, tr_level.rooms_count);
    Con_Notify(SYSNOTE_NUM_TEXTURES, tr_level.textile32_count);

    Game_Prepare();

    Render_SetWorld(&engine_world);

    Gui_FadeStart(FADER_LOADSCREEN, GUI_FADER_DIR_IN);
    Gui_NotifierStop();

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
            Con_AddLine("Available commands:\0", FONTSTYLE_CONSOLE_WARNING);
            Con_AddLine("help - show help info\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("loadMap(\"file_name\") - load level \"file_name\"\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("save, load - save and load game state in \"file_name\"\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("exit - close program\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("cls - clean console\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("show_fps - switch show fps flag\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("spacing - read and write spacing\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("showing_lines - read and write number of showing lines\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("cvars - lua's table of cvar's, to see them type: show_table(cvars)\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("free_look - switch camera mode\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("cam_distance - camera distance to actor\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("r_wireframe, r_portals, r_frustums, r_room_boxes, r_boxes, r_normals, r_skip_room - render modes\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("playsound(id) - play specified sound\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("stopsound(id) - stop specified sound\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("Watch out for case sensitive commands!\0", FONTSTYLE_CONSOLE_WARNING);
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
        else if(!strcmp(token, "spacing"))
        {
            ch = parse_token(ch, token);
            if(NULL == ch)
            {
                Con_Notify(SYSNOTE_CONSOLE_SPACING, con_base.spacing);
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
                Con_Notify(SYSNOTE_CONSOLE_LINECOUNT, con_base.showing_lines);
                return 1;
            }
            else
            {
                val = atoi(token);
                if((val >=2 ) && (val <= con_base.line_count))
                {
                    con_base.showing_lines = val;
                    con_base.cursor_y = screen_info.h - con_base.line_height * con_base.showing_lines;
                }
                else
                {
                    Con_Warning(SYSWARN_INVALID_LINECOUNT);
                }
            }
            return 1;
        }
        else if(!strcmp(token, "r_wireframe"))
        {
            renderer.style ^= R_DRAW_WIRE;
            return 1;
        }
        else if(!strcmp(token, "r_points"))
        {
            renderer.style ^= R_DRAW_POINTS;
            return 1;
        }
        else if(!strcmp(token, "r_coll"))
        {
            renderer.style ^= R_DRAW_COLL;
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
                Con_Printf("ID = %d, x_sect = %d, y_sect = %d", r->id, r->sectors_x, r->sectors_y);
                if(sect)
                {
                    Con_Printf("sect(%d, %d), inpenitrable = %d, r_up = %d, r_down = %d", sect->index_x, sect->index_y,
                               (int)(sect->ceiling == TR_METERING_WALLHEIGHT || sect->floor == TR_METERING_WALLHEIGHT), (int)(sect->sector_above != NULL), (int)(sect->sector_below != NULL));
                    for(uint32_t i=0;i<sect->owner_room->static_mesh_count;i++)
                    {
                        Con_Printf("static[%d].object_id = %d", i, sect->owner_room->static_mesh[i].object_id);
                    }
                    for(engine_container_p cont=sect->owner_room->containers;cont;cont=cont->next)
                    {
                        if(cont->object_type == OBJECT_ENTITY)
                        {
                            entity_p e = (entity_p)cont->object;
                            Con_Printf("cont[entity](%d, %d, %d).object_id = %d", (int)e->transform[12+0], (int)e->transform[12+1], (int)e->transform[12+2], e->id);
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
                Con_AddText(buf, FONTSTYLE_CONSOLE_INFO);
                free(buf);
            }
            else
            {
                Con_AddText("Not avaliable =(", FONTSTYLE_CONSOLE_WARNING);
            }
            return 1;
        }
        else if(token[0])
        {
            if(engine_lua)
            {
                Con_AddLine(pch);
                if (luaL_dostring(engine_lua, pch) != LUA_OK)
                {
                    Con_AddLine(lua_tostring(engine_lua, -1), FONTSTYLE_CONSOLE_WARNING);
                    lua_pop(engine_lua, 1);
                }
            }
            else
            {
                snprintf(buf, con_base.line_size + 32, "Command \"%s\" not found", token);
                Con_AddLine(buf, FONTSTYLE_CONSOLE_WARNING);
            }
            return 0;
        }
    }

    return 0;
}


void Engine_InitConfig(const char *filename)
{
    lua_State *lua = luaL_newstate();

    Engine_InitDefaultGlobals();

    if(lua != NULL)
    {
        if((filename != NULL) && Engine_FileFound(filename))
        {
            luaL_openlibs(lua);
            lua_register(lua, "bind", lua_BindKey);                             // get and set key bindings
            luaL_dofile(lua, filename);

            lua_ParseScreen(lua, &screen_info);
            lua_ParseRender(lua, &renderer.settings);
            lua_ParseAudio(lua, &audio_settings);
            lua_ParseConsole(lua, &con_base);
            lua_ParseControls(lua, &control_mapper);
            lua_close(lua);
        }
        else
        {
            Sys_Warn("Could not find \"%s\"", filename);
        }
    }
}


void Engine_SaveConfig()
{

}
