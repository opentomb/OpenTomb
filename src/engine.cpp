
#include <SDL2/SDL.h>
#include <SDL2/SDL_platform.h>
#if !defined(__MACOSX__)
#include <SDL2/SDL_image.h>
#endif
#include <SDL2/SDL_opengl.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>

#include "bullet/btBulletCollisionCommon.h"
#include "bullet/btBulletDynamicsCommon.h"
#include "bullet/BulletCollision/CollisionDispatch/btGhostObject.h"

#include <lua.hpp>

#include <AL/al.h>
#include <AL/alc.h>

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
#include "gl_font.h"
#include "string.h"
#include "hair.h"
#include "ragdoll.h"

#include "luahelper.h"

extern SDL_Window             *sdl_window;
extern SDL_GLContext           sdl_gl_context;
extern SDL_GameController     *sdl_controller;
extern SDL_Joystick           *sdl_joystick;
extern SDL_Haptic             *sdl_haptic;
extern ALCdevice              *al_device;
extern ALCcontext             *al_context;

EngineControlState           control_states{};
ControlSettings               control_mapper{};
AudioSettings                 audio_settings{};
btScalar                                engine_frame_time = 0.0;

Camera                         engine_camera;
World                          engine_world;

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

RenderDebugDrawer                       debugDrawer;

/**
 * overlapping room collision filter
 */
void Engine_RoomNearCallback(btBroadphasePair& collisionPair, btCollisionDispatcher& dispatcher, const btDispatcherInfo& dispatchInfo)
{
    EngineContainer* c0, *c1;

    c0 = (EngineContainer*)((btCollisionObject*)collisionPair.m_pProxy0->m_clientObject)->getUserPointer();
    Room* r0 = c0 ? c0->room : nullptr;
    c1 = (EngineContainer*)((btCollisionObject*)collisionPair.m_pProxy1->m_clientObject)->getUserPointer();
    Room* r1 = c1 ? c1->room : nullptr;

    if(c1 && c1 == c0)
    {
        if(((btCollisionObject*)collisionPair.m_pProxy0->m_clientObject)->isStaticOrKinematicObject() ||
                ((btCollisionObject*)collisionPair.m_pProxy1->m_clientObject)->isStaticOrKinematicObject())
        {
            return;                                                             // No self interaction
        }
        dispatcher.defaultNearCallback(collisionPair, dispatcher, dispatchInfo);
        return;
    }

    if(!r0 && !r1)
    {
        dispatcher.defaultNearCallback(collisionPair, dispatcher, dispatchInfo);// Both are out of rooms
        return;
    }

    if(r0 && r1)
    {
        if(r0->isInNearRoomsList(*r1))
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
void Engine_InternalTickCallback(btDynamicsWorld *world, btScalar /*timeStep*/)
{
    for(int i=world->getNumCollisionObjects()-1;i>=0;i--)
    {
        btCollisionObject* obj = bt_engine_dynamicsWorld->getCollisionObjectArray()[i];
        btRigidBody* body = btRigidBody::upcast(obj);
        if (body && !body->isStaticObject() && body->getMotionState())
        {
            btTransform trans;
            body->getMotionState()->getWorldTransform(trans);
            EngineContainer* cont = (EngineContainer*)body->getUserPointer();
            if(cont && (cont->object_type == OBJECT_BULLET_MISC))
            {
                cont->room = Room_FindPosCogerrence(trans.getOrigin(), cont->room);
            }
        }
    }
}

void Engine_InitDefaultGlobals()
{
    ConsoleInfo::instance().initGlobals();
    Controls_InitGlobals();
    Game_InitGlobals();
    renderer.initGlobals();
    Audio_InitGlobals();
}

// First stage of initialization.

void Engine_Init_Pre()
{
    /* Console must be initialized previously! some functions uses ConsoleInfo::instance().addLine before GL initialization!
     * Rendering activation may be done later. */

    Gui_InitFontManager();
    ConsoleInfo::instance().init();
    Engine_LuaInit();

    lua_CallVoidFunc(engine_lua, "loadscript_pre", true);

    Gameflow_Init();

    frame_vertex_buffer = (btScalar*)malloc(sizeof(btScalar) * INIT_FRAME_VERTEX_BUFFER_SIZE);
    frame_vertex_buffer_size = INIT_FRAME_VERTEX_BUFFER_SIZE;
    frame_vertex_buffer_size_left = frame_vertex_buffer_size;

    Com_Init();
    renderer.init();
    engine_camera = Camera();
    renderer.setCamera( &engine_camera );

    Engine_BTInit();
}

// Second stage of initialization.

void Engine_Init_Post()
{
    lua_CallVoidFunc(engine_lua, "loadscript_post", true);

    ConsoleInfo::instance().initFonts();

    Gui_Init();
    Sys_Init();

    ConsoleInfo::instance().addLine("Engine inited!", FONTSTYLE_CONSOLE_EVENT);
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

    debugDrawer.setDebugMode(btIDebugDraw::DBG_DrawWireframe | btIDebugDraw::DBG_DrawConstraints);
    bt_engine_dynamicsWorld->setDebugDrawer(&debugDrawer);
    //bt_engine_dynamicsWorld->getPairCache()->setInternalGhostPairCallback(bt_engine_filterCallback);
}

/*
 * debug functions
 */

void lua_CheckStack()
{
    ConsoleInfo::instance().printf("Current Lua stack index: %d", lua_gettop(lua::state()));
}

int lua_print(lua_State* lua)
{
     const int top = lua_gettop(lua);

     if(top == 0)
     {
        ConsoleInfo::instance().addLine("nil", FONTSTYLE_CONSOLE_EVENT);
        return 0;
     }

     for(int i=1; i<=top; i++)
     {
         const char* str = lua_tostring(lua, i);
         ConsoleInfo::instance().addLine(str ? str : std::string(), FONTSTYLE_CONSOLE_EVENT);
     }
     return 0;
}

void lua_DumpModel(int id)
{

    SkeletalModel* sm = engine_world.getModelByID(id);
    if(sm == NULL)
    {
        ConsoleInfo::instance().printf("wrong model id = %d", id);
        return;
    }

    for(int i=0;i<sm->mesh_count;i++)
    {
        ConsoleInfo::instance().printf("mesh[%d] = %d", i, sm->mesh_tree[i].mesh_base->m_id);
    }
}

void dumpRoom(Room* r)
{
    if(r != nullptr)
    {
        Sys_DebugLog("room_dump.txt", "ROOM = %d, (%d x %d), bottom = %g, top = %g, pos(%g, %g)", r->id, r->sectors_x, r->sectors_y, r->bb_min[2], r->bb_max[2], r->transform.getOrigin()[0], r->transform.getOrigin()[1]);
        Sys_DebugLog("room_dump.txt", "flag = 0x%X, alt_room = %d, base_room = %d", r->flags, (r->alternate_room != NULL)?(r->alternate_room->id):(-1), (r->base_room != NULL)?(r->base_room->id):(-1));
        for(const RoomSector& rs : r->sectors)
        {
            Sys_DebugLog("room_dump.txt", "(%d,%d)\tfloor = %d, ceiling = %d, portal = %d", rs.index_x, rs.index_y, rs.floor, rs.ceiling, rs.portal_to_room);
        }
        for(auto sm : r->static_mesh)
        {
            Sys_DebugLog("room_dump.txt", "static_mesh = %d", sm->object_id);
        }
        for(const std::shared_ptr<EngineContainer>& cont : r->containers)
        {
            if(cont->object_type == OBJECT_ENTITY)
            {
                Entity* ent = static_cast<Entity*>(cont->object);
                Sys_DebugLog("room_dump.txt", "entity: id = %d, model = %d", ent->m_id, ent->m_bf.animations.model->id);
            }
        }
    }
}

void lua_dumpRoom1()
{
    dumpRoom(engine_camera.m_currentRoom);
}

void lua_dumpRoom2(uint32_t id)
{
    dumpRoom(engine_camera.m_currentRoom);
    if(id >= engine_world.rooms.size())
    {
        ConsoleInfo::instance().warning(SYSWARN_WRONG_ROOM, id);
        return;
    }
    dumpRoom(engine_world.rooms[id].get());
}

void lua_SetRoomEnabled(int id, bool value)
{
    if(id < 0 || id >= static_cast<int>(engine_world.rooms.size())) {
        ConsoleInfo::instance().warning(SYSWARN_WRONG_ROOM, id);
    }

    if(!value) {
        engine_world.rooms[id]->disable();
    }
    else {
        engine_world.rooms[id]->enable();
    }
}

/*
 * Base engine functions
 */

void lua_SetModelCollisionMapSize(int id, int size)
{
    SkeletalModel* model = engine_world.getModelByID(id);
    if(model == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_MODELID_OVERFLOW, id);
        return;
    }

    if(size >= 0 && size < model->mesh_count)
    {
        model->collision_map.resize(size);
    }
}

void lua_SetModelCollisionMap(int id, int arg, int val)
{
    /// engine_world.skeletal_models[id] != engine_world.getModelByID(lua_tointeger(lua, 1));
    SkeletalModel* model = engine_world.getModelByID(id);
    if(model == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_MODELID_OVERFLOW, id);
    }

    if((arg >= 0) && (arg < model->mesh_count) &&
            (val >= 0) && (val < model->mesh_count))
    {
        model->collision_map[arg] = val;
    }
}

void lua_EnableEntity(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);
    if(ent)
        ent->enable();
}

void lua_DisableEntity(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);
    if(ent)
        ent->disable();
}

void lua_SetEntityCollision(int id, bool val)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);
    if(ent) {
        if(val)
            ent->enableCollision();
        else
            ent->disableCollision();
    }
}

void lua_SetEntityCollisionFlags3(int id, lua::Int ctype, lua::Int cshape)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);
    if(ent != NULL)
    {
        if(ctype)
        {
            ent->m_self->collision_type = *ctype;
        }
        if(cshape)
        {
            ent->m_self->collision_shape = *cshape;
        }

        if(ent->m_self->collision_type & 0x0001)
        {
            ent->enableCollision();
        }
        else
        {
            ent->disableCollision();
        }
    }
}

void lua_SetEntityCollisionFlags2(int id, lua::Int ctype)
{
    lua_SetEntityCollisionFlags3(id, ctype, lua::None);
}

void lua_SetEntityCollisionFlags1(int id)
{
    lua_SetEntityCollisionFlags3(id, lua::None, lua::None);
}

lua::UInt32 lua_GetEntitySectorFlags(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if((ent != NULL) && (ent->m_currentSector))
    {
        return ent->m_currentSector->flags;
    }
    return lua::None;
}

lua::UInt32 lua_GetEntitySectorIndex(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if((ent != NULL) && (ent->m_currentSector))
    {
        return ent->m_currentSector->trig_index;
    }
    return lua::None;
}

lua::UInt32 lua_GetEntitySectorMaterial(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if((ent != NULL) && (ent->m_currentSector))
    {
        return ent->m_currentSector->material;
    }
    return lua::None;
}

lua::Bool lua_SameRoom(int id1, int id2)
{
    std::shared_ptr<Entity> ent1 = engine_world.getEntityByID(id1);
    std::shared_ptr<Entity> ent2 = engine_world.getEntityByID(id2);

    if(ent1 && ent2 )
    {
        return ent1->m_self->room == ent2->m_self->room;
    }

    return lua::None;
}

lua::Bool lua_NewSector(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent != NULL)
    {
        return ent->m_currentSector == ent->m_lastSector;
    }
    return lua::None;
}

lua::Tuple<float,float,float> lua_GetGravity()
{
    btVector3 g = bt_engine_dynamicsWorld->getGravity();
    return lua::Tuple<float,float,float>{
        g[0],
        g[1],
        g[2]
    };
}

void lua_SetGravity3(float x, float y, float z)                                             // function to be exported to Lua
{
    btVector3 g = {x,y,z};
    bt_engine_dynamicsWorld->setGravity(g);
    ConsoleInfo::instance().printf("gravity = (%.3f, %.3f, %.3f)", g[0], g[1], g[2]);
}

void lua_SetGravity2(float x, float y)                                             // function to be exported to Lua
{
    btVector3 g = {x,y,0};
    bt_engine_dynamicsWorld->setGravity(g);
    ConsoleInfo::instance().printf("gravity = (%.3f, %.3f, %.3f)", g[0], g[1], g[2]);
}

void lua_SetGravity1(float x)                                             // function to be exported to Lua
{
    btVector3 g = {x,0,0};
    bt_engine_dynamicsWorld->setGravity(g);
    ConsoleInfo::instance().printf("gravity = (%.3f, %.3f, %.3f)", g[0], g[1], g[2]);
}

lua::Bool lua_DropEntity2(int id, float time, lua::Bool optOnlyRoom)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);
    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return lua::None;
    }

    btVector3 g = bt_engine_dynamicsWorld->getGravity();
    btVector3 move = ent->m_speed * time;;
    move += g * 0.5 * time * time;
    ent->m_speed += g * time;

    BtEngineClosestRayResultCallback cb(ent->m_self);
    btVector3 from, to;
    from = ent->m_transform * ent->m_bf.centre;
    from[2] = ent->m_transform.getOrigin()[2];
    to = from + move;
    to[2] -= (ent->m_bf.bb_max[2] - ent->m_bf.bb_min[2]);
    bt_engine_dynamicsWorld->rayTest(from, to, cb);

    if(cb.hasHit())
    {
        bool only_room = optOnlyRoom && *optOnlyRoom;
        EngineContainer* cont = (EngineContainer*)cb.m_collisionObject->getUserPointer();

        if((!only_room) || ((only_room) && (cont->object_type == OBJECT_ROOM_BASE)))
        {
            move.setInterpolate3(from ,to, cb.m_closestHitFraction);
            ent->m_transform.getOrigin()[2] = move[2];

            ent->updateRigidBody(true);
            return true;
        }
        else
        {
            ent->updateRigidBody(true);
            return false;
        }
    }
    else
    {
        ent->m_transform.getOrigin() += move;
        ent->updateRigidBody(true);
        return false;
    }
}

lua::Bool lua_DropEntity1(int id, float time)
{
    return lua_DropEntity2(id, time, lua::None);
}

lua::Int lua_GetEntityModelID(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);
    if(ent == NULL) return lua::None;

    if(ent->m_bf.animations.model)
    {
        return ent->m_bf.animations.model->id;
    }
    return lua::None;
}

lua::OptionalTuple<float,float,float,float> lua_GetEntityActivationOffset(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);
    if(ent == NULL) return lua::None;

    return lua::Tuple<float,float,float,float>(
                ent->m_activationOffset[0],
                ent->m_activationOffset[1],
                ent->m_activationOffset[2],
                ent->m_activationRadius);
}

void lua_SetEntityActivationOffset2(int id, lua::Float x, lua::Float y, lua::Float z, lua::Float r)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);
    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return;
    }

    if(x && y && z)
    {
        ent->m_activationOffset = {*x,*y,*z};
    }
    if(r)
    {
        ent->m_activationRadius = *r;
    }
}

void lua_SetEntityActivationOffset1(int id, lua::Float x, lua::Float y, lua::Float z)
{
    lua_SetEntityActivationOffset2(id, x, y, z, lua::None);
}

lua::Int lua_GetCharacterParam(int id, int parameter)
{
    std::shared_ptr<Character> ent = engine_world.getCharacterByID(id);

    if(parameter >= PARAM_SENTINEL)
    {
        ConsoleInfo::instance().warning(SYSWARN_WRONG_OPTION_INDEX, PARAM_SENTINEL);
        return lua::None;
    }

    if(ent)
    {
        return ent->getParam(parameter);
    }
    else
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_CHARACTER, id);
        return lua::None;
    }
}

void lua_SetCharacterParam2(int id, int parameter, float value, lua::Float max_value)
{
    std::shared_ptr<Character> ent = engine_world.getCharacterByID(id);

    if(parameter >= PARAM_SENTINEL)
    {
        ConsoleInfo::instance().warning(SYSWARN_WRONG_OPTION_INDEX, PARAM_SENTINEL);
        return;
    }

    if(!ent)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_CHARACTER, id);
        return;
    }
    else if(!max_value)
    {
        ent->setParam(parameter, value);
    }
    else
    {
        ent->m_parameters.param[parameter] = value;
        ent->m_parameters.maximum[parameter] = *max_value;
    }
}

void lua_SetCharacterParam1(int id, int parameter, float value)
{
    lua_SetCharacterParam2(id, parameter, value, lua::None);
}

lua::Int lua_GetCharacterCombatMode(int id)
{
    std::shared_ptr<Character> ent = engine_world.getCharacterByID(id);

    if(ent)
    {
        return static_cast<int>(ent->m_weaponCurrentState);
    }

    return lua::None;
}

void lua_ChangeCharacterParam(int id, int parameter, float value)
{
    std::shared_ptr<Character> ent = engine_world.getCharacterByID(id);

    if(parameter >= PARAM_SENTINEL)
    {
        ConsoleInfo::instance().warning(SYSWARN_WRONG_OPTION_INDEX, PARAM_SENTINEL);
        return;
    }

    if(ent)
    {
        ent->changeParam(parameter, value);
    }
    else
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_CHARACTER, id);
    }
}

void lua_AddCharacterHair(int ent_id, int setup_index)
{
    std::shared_ptr<Character> ent = engine_world.getCharacterByID(ent_id);

    if(ent)
    {
        HairSetup hair_setup;

        if(!hair_setup.getSetup(setup_index))
        {
            ConsoleInfo::instance().warning(SYSWARN_NO_HAIR_SETUP, setup_index);
        }
        else
        {
            ent->m_hairs.emplace_back(std::make_shared<Hair>());

            if(!ent->m_hairs.back()->create(&hair_setup, ent))
            {
                ConsoleInfo::instance().warning(SYSWARN_CANT_CREATE_HAIR, ent_id);
                ent->m_hairs.pop_back();
            }
        }
    }
    else
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_CHARACTER, ent_id);
    }
}

void lua_ResetCharacterHair(int ent_id)
{
    std::shared_ptr<Character> ent = engine_world.getCharacterByID(ent_id);

    if(ent)
    {
        if(!ent->m_hairs.empty())
        {
            ent->m_hairs.clear();
        }
        else
        {
            ConsoleInfo::instance().warning(SYSWARN_CANT_RESET_HAIR, ent_id);
        }
    }
    else
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_CHARACTER, ent_id);
    }
}

void lua_AddEntityRagdoll(int ent_id, int setup_index)
{
    std::shared_ptr<Entity> ent   = engine_world.getEntityByID(ent_id);

    if(ent)
    {
        RDSetup ragdoll_setup;

        if(!ragdoll_setup.getSetup(setup_index))
        {
            ConsoleInfo::instance().warning(SYSWARN_NO_RAGDOLL_SETUP, setup_index);
        }
        else
        {
            if(!ent->createRagdoll(&ragdoll_setup))
            {
                ConsoleInfo::instance().warning(SYSWARN_CANT_CREATE_RAGDOLL, ent_id);
            }
        }
    }
    else
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, ent_id);
    }
}

void lua_RemoveEntityRagdoll(int ent_id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(ent_id);

    if(ent)
    {
        if(!ent->m_bt.bt_joints.empty())
        {
            ent->deleteRagdoll();
        }
        else
        {
            ConsoleInfo::instance().warning(SYSWARN_CANT_REMOVE_RAGDOLL, ent_id);
        }
    }
    else
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, ent_id);
    }
}

lua::Optional<char> lua_GetSecretStatus(int secret_number)
{
    if((secret_number > TR_GAMEFLOW_MAX_SECRETS) || (secret_number < 0)) return lua::None;   // No such secret - return

    return gameflow_manager.SecretsTriggerMap[secret_number];
}

void lua_SetSecretStatus(int secret_number, char status)
{
    if((secret_number > TR_GAMEFLOW_MAX_SECRETS) || (secret_number < 0)) return;   // No such secret - return

    gameflow_manager.SecretsTriggerMap[secret_number] = status;
}

lua::Bool lua_GetActionState(int act)
{
    if(act < 0 || act >= ACT_LASTINDEX)
    {
        ConsoleInfo::instance().warning(SYSWARN_WRONG_ACTION_NUMBER);
        return lua::None;
    }
    else
    {
        return control_mapper.action_map[act].state;
    }
}

lua::Bool lua_GetActionChange(int act)
{
    if(act < 0 || act >= ACT_LASTINDEX)
    {
        ConsoleInfo::instance().warning(SYSWARN_WRONG_ACTION_NUMBER);
        return lua::None;
    }
    else
    {
        return control_mapper.action_map[act].already_pressed;
    }

}

int lua_GetLevelVersion()
{
    return engine_world.version;
}

void lua_BindKey2(int act, int primary, lua::Int secondary)
{
    if(act < 0 || act >= ACT_LASTINDEX)
    {
        ConsoleInfo::instance().warning(SYSWARN_WRONG_ACTION_NUMBER);
    }
    control_mapper.action_map[act].primary = primary;
    if(secondary)
        control_mapper.action_map[act].secondary = *secondary;
}

void lua_BindKey1(int act, int primary)
{
    lua_BindKey2(act, primary, lua::None);
}

void lua_AddFont(int index, const std::string& path, uint32_t size)
{
    if(!FontManager->AddFont((font_Type)index, size, path.c_str()))
    {
        ConsoleInfo::instance().warning(SYSWARN_CANT_CREATE_FONT, FontManager->GetFontCount(), GUI_MAX_FONTS);

    }
}

void lua_AddFontStyle(int style_index,
                      float color_R, float color_G, float color_B, float color_A,
                      bool shadowed, bool fading, bool rect, float rect_border,
                      float rect_R, float rect_G, float rect_B, float rect_A,
                      bool hide)
{
    if(!FontManager->AddFontStyle((font_Style)style_index,
                                  color_R, color_G, color_B, color_A,
                                  shadowed, fading,
                                  rect, rect_border, rect_R, rect_G, rect_B, rect_A,
                                  hide))
    {
        ConsoleInfo::instance().warning(SYSWARN_CANT_CREATE_STYLE, FontManager->GetFontStyleCount(), GUI_MAX_FONTSTYLES);
    }
}

void lua_DeleteFont(int fontindex)
{
    if(!FontManager->RemoveFont((font_Type)fontindex))
    {
        ConsoleInfo::instance().warning(SYSWARN_CANT_REMOVE_FONT);
    }
}

void lua_DeleteFontStyle(int styleindex)
{
    if(!FontManager->RemoveFontStyle((font_Style)styleindex))
    {
        ConsoleInfo::instance().warning(SYSWARN_CANT_REMOVE_STYLE);
    }
}

lua::Int lua_AddItem3(int entity_id, int item_id, int count)
{
    std::shared_ptr<Character> ent = engine_world.getCharacterByID(entity_id);

    if(ent)
    {
        return ent->addItem(item_id, count);
    }
    else
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, entity_id);
        return lua::None;
    }
}

lua::Int lua_AddItem2(int entity_id, int item_id)
{
    return lua_AddItem3(entity_id, item_id, -1);
}

lua::Int lua_RemoveItem(int entity_id, int item_id, int count)
{
    std::shared_ptr<Character> ent = engine_world.getCharacterByID(entity_id);

    if(ent)
    {
        return ent->removeItem(item_id, count);
    }
    else
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, entity_id);
        return lua::None;
    }
}

void lua_RemoveAllItems(int entity_id)
{
    std::shared_ptr<Character> ent = engine_world.getCharacterByID(entity_id);

    if(ent)
    {
        ent->removeAllItems();
    }
    else
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, entity_id);
    }
}

lua::Int lua_GetItemsCount(int entity_id, int item_id)
{
    std::shared_ptr<Character> ent = engine_world.getCharacterByID(entity_id);

    if(ent)
    {
        return ent->getItemsCount(item_id);
    }
    else
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, entity_id);
        return lua::None;
    }

}

void lua_CreateBaseItem2(int item_id, int model_id, int world_model_id, int type, int count, lua::String name)
{
    engine_world.createItem(item_id, model_id, world_model_id, type, count, name ? name->c_str() : nullptr);
}

void lua_CreateBaseItem1(int item_id, int model_id, int world_model_id, int type, int count)
{
    lua_CreateBaseItem2(item_id, model_id, world_model_id, type, count, lua::None);
}

void lua_DeleteBaseItem(int id)
{
    engine_world.deleteItem(id);
}

void lua_PrintItems(int entity_id)
{
    std::shared_ptr<Character> ent = engine_world.getCharacterByID(entity_id);
    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, entity_id);
        return;
    }

    for(const InventoryNode& i : ent->m_inventory)
    {
        ConsoleInfo::instance().printf("item[id = %d]: count = %d, type = %d", i.id, i.count);
    }
}

void lua_SetStateChangeRange2(int id, int anim, int state, int dispatch, int frame_low, int frame_high, lua::Int next_anim, lua::Int next_frame)
{
    SkeletalModel* model = engine_world.getModelByID(id);

    if(model == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_SKELETAL_MODEL, id);
        return;
    }

    if((anim < 0) || (anim + 1 > static_cast<int>(model->animations.size())))
    {
        ConsoleInfo::instance().warning(SYSWARN_WRONG_ANIM_NUMBER, anim);
        return;
    }

    AnimationFrame* af = &model->animations[anim];
    for(uint16_t i=0;i<af->state_change.size();i++)
    {
        if(af->state_change[i].id == (uint32_t)state)
        {
            if(dispatch >= 0 && dispatch < static_cast<int>(af->state_change[i].anim_dispatch.size()))
            {
                af->state_change[i].anim_dispatch[dispatch].frame_low = frame_low;
                af->state_change[i].anim_dispatch[dispatch].frame_high = frame_high;
                if(next_anim && next_frame)
                {
                    af->state_change[i].anim_dispatch[dispatch].next_anim = *next_anim;
                    af->state_change[i].anim_dispatch[dispatch].next_frame = *next_frame;
                }
            }
            else
            {
                ConsoleInfo::instance().warning(SYSWARN_WRONG_DISPATCH_NUMBER, dispatch);
            }
            break;
        }
    }
}

void lua_SetStateChangeRange1(int id, int anim, int state, int dispatch, int frame_low, int frame_high)
{
    lua_SetStateChangeRange2(id, anim, state, dispatch, frame_low, frame_high, lua::None, lua::None);
}

lua::OptionalTuple<int,float,float,float> lua_GetAnimCommandTransform(int id, int anim, int frame)
{
    SkeletalModel* model = engine_world.getModelByID(id);
    if(model == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_SKELETAL_MODEL, id);
        return lua::None;
    }

    if(anim < 0 || anim + 1 > static_cast<int>(model->animations.size()))
    {
        ConsoleInfo::instance().warning(SYSWARN_WRONG_ANIM_NUMBER, anim);
        return lua::None;
    }

    if(frame < 0)                                                               // it is convenient to use -1 as a last frame number
    {
        frame = (int)model->animations[anim].frames.size() + frame;
    }

    if(frame < 0 || frame + 1 > static_cast<int>(model->animations[anim].frames.size()))
    {
        ConsoleInfo::instance().warning(SYSWARN_WRONG_FRAME_NUMBER, frame);
        return lua::None;
    }

    return lua::Tuple<int,float,float,float>
    {
        model->animations[anim].frames[frame].command,
             model->animations[anim].frames[frame].move[0],
             model->animations[anim].frames[frame].move[1],
             model->animations[anim].frames[frame].move[2]
    };
}

void lua_SetAnimCommandTransform1(int id, int anim, int frame, uint8_t flag, float dx, float dy, float dz)
{
    SkeletalModel* model = engine_world.getModelByID(id);
    if(model == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_SKELETAL_MODEL, id);
        return;
    }

    if(anim < 0 || anim + 1 > static_cast<int>(model->animations.size()))
    {
        ConsoleInfo::instance().warning(SYSWARN_WRONG_ANIM_NUMBER, anim);
        return;
    }

    if(frame < 0)                                                               // it is convenient to use -1 as a last frame number
    {
        frame = (int)model->animations[anim].frames.size() + frame;
    }

    if(frame < 0 || frame + 1 > static_cast<int>(model->animations[anim].frames.size()))
    {
        ConsoleInfo::instance().warning(SYSWARN_WRONG_FRAME_NUMBER, frame);
        return;
    }

    model->animations[anim].frames[frame].command = flag;

    model->animations[anim].frames[frame].move = {dx,dy,dz};
}

void lua_SetAnimCommandTransform2(int id, int anim, int frame, uint8_t flag)
{
    SkeletalModel* model = engine_world.getModelByID(id);
    if(model == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_SKELETAL_MODEL, id);
        return;
    }

    if(anim < 0 || anim + 1 > static_cast<int>(model->animations.size()))
    {
        ConsoleInfo::instance().warning(SYSWARN_WRONG_ANIM_NUMBER, anim);
        return;
    }

    if(frame < 0)                                                               // it is convenient to use -1 as a last frame number
    {
        frame = (int)model->animations[anim].frames.size() + frame;
    }

    if(frame < 0 || frame + 1 > static_cast<int>(model->animations[anim].frames.size()))
    {
        ConsoleInfo::instance().warning(SYSWARN_WRONG_FRAME_NUMBER, frame);
        return;
    }

    model->animations[anim].frames[frame].command = flag;
}

lua::UInt32 lua_SpawnEntity2(int model_id, int room_id, float x, float y, float z, float ax, float ay, float az, lua::Int optOvId)
{
    btVector3 pos{x,y,z}, ang{ax,ay,az};
    int32_t ov_id = optOvId ? *optOvId : -1;

    uint32_t id = engine_world.spawnEntity(model_id, room_id, &pos, &ang, ov_id);
    if(id == 0xFFFFFFFF)
    {
        return lua::None;
    }
    else
    {
        return id;
    }
}

lua::UInt32 lua_SpawnEntity1(int model_id, int room_id, float x, float y, float z, float ax, float ay, float az)
{
    return lua_SpawnEntity2(model_id, room_id, x,y,z, ax,ay,az, lua::None);
}

/*
 * Moveables script control section
 */
lua::OptionalTuple<float,float,float> lua_GetEntityVector(int id1, int id2)
{
    std::shared_ptr<Entity> e1 = engine_world.getEntityByID(id1);
    if(e1 == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id1);
        return lua::None;
    }
    std::shared_ptr<Entity> e2 = engine_world.getEntityByID(id2);
    if(e2 == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id2);
        return lua::None;
    }

    return lua::Tuple<float,float,float>
    {
        e2->m_transform.getOrigin()[0] - e1->m_transform.getOrigin()[0],
        e2->m_transform.getOrigin()[1] - e1->m_transform.getOrigin()[1],
        e2->m_transform.getOrigin()[2] - e1->m_transform.getOrigin()[2]
    };
}

lua::Float lua_GetEntityDistance(int id1, int id2)
{
    std::shared_ptr<Entity> e1 = engine_world.getEntityByID(id1);
    if(e1 == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id1);
        return lua::None;
    }
    std::shared_ptr<Entity> e2 = engine_world.getEntityByID(id2);
    if(e2 == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id2);
        return lua::None;
    }

    return e1->findDistance(*e2);
}

lua::Float lua_GetEntityDirDot(int id1, int id2)
{
    std::shared_ptr<Entity> e1 = engine_world.getEntityByID(id1);
    if(e1 == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id2);
        return lua::None;
    }
    std::shared_ptr<Entity> e2 = engine_world.getEntityByID(id2);
    if(e2 == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id2);
    }

    return e1->m_transform.getBasis()[1].dot(e2->m_transform.getBasis()[1]);
}

lua::OptionalTuple<float,float,float,float,float,float> lua_GetEntityPosition(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);
    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return lua::None;
    }

    return lua::Tuple<float,float,float,float,float,float>
    {
        ent->m_transform.getOrigin()[0],
        ent->m_transform.getOrigin()[1],
        ent->m_transform.getOrigin()[2],
        ent->m_angles[0],
        ent->m_angles[1],
        ent->m_angles[2]
    };
}

lua::OptionalTuple<float,float,float> lua_GetEntityAngles(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return lua::None;
    }

    return lua::Tuple<float,float,float>
    {
        ent->m_angles[0],
        ent->m_angles[1],
        ent->m_angles[2]
    };
}

lua::OptionalTuple<float,float,float> lua_GetEntityScaling(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(!ent) {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return lua::None;
    }

    return lua::Tuple<float,float,float>
    {
        ent->m_scaling[0],
        ent->m_scaling[1],
        ent->m_scaling[2]
    };
}

lua::Bool lua_SimilarSector2(int id, float dx, float dy, float dz, bool ignore_doors, lua::Bool ceiling)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return lua::None;
    }

    auto next_pos = ent->m_transform.getOrigin() + (dx * ent->m_transform.getBasis()[0] + dy * ent->m_transform.getBasis()[1] + dz * ent->m_transform.getBasis()[2]);

    RoomSector* curr_sector = ent->m_self->room->getSectorRaw(ent->m_transform.getOrigin());
    RoomSector* next_sector = ent->m_self->room->getSectorRaw(next_pos);

    curr_sector = curr_sector->checkPortalPointer();
    next_sector = next_sector->checkPortalPointer();

    if(ceiling && *ceiling)
    {
        return curr_sector->similarCeiling(next_sector, ignore_doors);
    }
    else
    {
        return curr_sector->similarFloor(next_sector, ignore_doors);
    }
}

lua::Bool lua_SimilarSector1(int id, float dx, float dy, float dz, bool ignore_doors)
{
    return lua_SimilarSector2(id, dx,dy,dz, ignore_doors, lua::None);
}

lua::Float lua_GetSectorHeight3(int id, lua::Bool ceiling, lua::Float dx, lua::Float dy , lua::Float dz)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return lua::None;
    }

    auto pos = ent->m_transform.getOrigin();

    if(dx && dy && dz)
    {
        pos += *dx * ent->m_transform.getBasis()[0] + *dy * ent->m_transform.getBasis()[1] + *dz * ent->m_transform.getBasis()[2];
    }

    RoomSector* curr_sector = ent->m_self->room->getSectorRaw(pos);
    curr_sector = curr_sector->checkPortalPointer();
    btVector3 point = (ceiling && *ceiling) ? curr_sector->getCeilingPoint() : curr_sector->getFloorPoint();

    return point[2];
}

lua::Float lua_GetSectorHeight2(int id, lua::Bool ceiling)
{
    return lua_GetSectorHeight3(id, ceiling, lua::None,lua::None,lua::None);
}

lua::Float lua_GetSectorHeight1(int id)
{
    return lua_GetSectorHeight3(id, lua::None, lua::None,lua::None,lua::None);
}

void lua_SetEntityPosition2(int id, float x, float y, float z, lua::Float ax, lua::Float ay, lua::Float az)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);
    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return;
    }
    ent->m_transform.getOrigin() = {x,y,z};
    if(ax && ay && az) {
        ent->m_angles = {*ax,*ay,*az};
    }
    ent->updateTransform();
    ent->updatePlatformPreStep();
}

void lua_SetEntityPosition1(int id, float x, float y, float z)
{
    lua_SetEntityPosition2(id, x, y, z, lua::None, lua::None, lua::None);
}

void lua_SetEntityAngles2(int id, float x, lua::Float y, lua::Float z)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
    }
    else
    {
        ent->m_angles[0] = x;

        if(y && z)
        {
            ent->m_angles[1] = *y;
            ent->m_angles[2] = *z;
        }

        ent->updateTransform();
    }
}

void lua_SetEntityAngles1(int id, float x)
{
    lua_SetEntityAngles2(id, x, lua::None, lua::None);
}

void lua_SetEntityScaling(int id, float x, float y, float z)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(!ent) {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
    }
    else {
        ent->m_scaling = {x,y,z};

        if(!ent->m_bf.bone_tags.empty() && !ent->m_bt.bt_body.empty()) {
            for(size_t i=0; i<ent->m_bf.bone_tags.size(); i++) {
                if(ent->m_bt.bt_body[i]) {
                    bt_engine_dynamicsWorld->removeRigidBody(ent->m_bt.bt_body[i].get());
                    ent->m_bt.bt_body[i]->getCollisionShape()->setLocalScaling(ent->m_scaling);
                    bt_engine_dynamicsWorld->addRigidBody(ent->m_bt.bt_body[i].get());

                    ent->m_bt.bt_body[i]->activate();
                }
            }
        }

        ent->updateRigidBody(true);
    }
}

void lua_MoveEntityGlobal(int id, float x, float y, float z)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);
    if(ent == NULL)
    {
        ConsoleInfo::instance().printf("can not find entity with id = %d", id);
        return;
    }
    ent->m_transform.getOrigin()[0] += x;
    ent->m_transform.getOrigin()[1] += y;
    ent->m_transform.getOrigin()[2] += z;
    ent->updateRigidBody(true);
}

void lua_MoveEntityLocal(int id, float dx, float dy, float dz)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return;
    }

    ent->m_transform.getOrigin()[0] += dx * ent->m_transform.getBasis()[0][0] + dy * ent->m_transform.getBasis()[1][0] + dz * ent->m_transform.getBasis()[2][0];
    ent->m_transform.getOrigin()[1] += dx * ent->m_transform.getBasis()[0][1] + dy * ent->m_transform.getBasis()[1][1] + dz * ent->m_transform.getBasis()[2][1];
    ent->m_transform.getOrigin()[2] += dx * ent->m_transform.getBasis()[0][2] + dy * ent->m_transform.getBasis()[1][2] + dz * ent->m_transform.getBasis()[2][2];

    ent->updateRigidBody(true);
}

void lua_MoveEntityToSink(int id, int sink_index)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(sink_index < 0 || sink_index > static_cast<int>(engine_world.cameras_sinks.size()))
        return;
    StatCameraSink* sink = &engine_world.cameras_sinks[sink_index];

    btVector3 ent_pos;  ent_pos[0] = ent->m_transform.getOrigin()[0];
    ent_pos[1] = ent->m_transform.getOrigin()[1];
    ent_pos[2] = ent->m_transform.getOrigin()[2];

    btVector3 sink_pos; sink_pos[0] = sink->x;
    sink_pos[1] = sink->y;
    sink_pos[2] = sink->z + 256.0;

    RoomSector* ls = ent->m_currentSector->getLowestSector();
    RoomSector* hs = ent->m_currentSector->getHighestSector();
    if((sink_pos[2] > hs->ceiling) ||
       (sink_pos[2] < ls->floor) )
    {
        sink_pos[2] = ent_pos[2];
    }

    btScalar dist = btDistance(ent_pos, sink_pos);
    if(dist == 0.0) dist = 1.0; // Prevents division by zero.

    btVector3 speed = ((sink_pos - ent_pos) / dist) * ((btScalar)(sink->room_or_strength) * 1.5);

    ent->m_transform.getOrigin()[0] += speed[0];
    ent->m_transform.getOrigin()[1] += speed[1];
    ent->m_transform.getOrigin()[2] += speed[2] * 16.0;

    ent->updateRigidBody(true);
}

void lua_MoveEntityToEntity2(int id1, int id2, float speed_mult, lua::Bool ignore_z)
{
    std::shared_ptr<Entity> ent1 = engine_world.getEntityByID(id1);
    std::shared_ptr<Entity> ent2 = engine_world.getEntityByID(id2);

    btVector3 ent1_pos; ent1_pos[0] = ent1->m_transform.getOrigin()[0];
    ent1_pos[1] = ent1->m_transform.getOrigin()[1];
    ent1_pos[2] = ent1->m_transform.getOrigin()[2];

    btVector3 ent2_pos; ent2_pos[0] = ent2->m_transform.getOrigin()[0];
    ent2_pos[1] = ent2->m_transform.getOrigin()[1];
    ent2_pos[2] = ent2->m_transform.getOrigin()[2];

    btScalar dist = btDistance(ent1_pos, ent2_pos);
    if(dist == 0.0) dist = 1.0; // Prevents division by zero.

    btVector3 speed = ((ent2_pos - ent1_pos) / dist) * speed_mult; // FIXME!

    ent1->m_transform.getOrigin()[0] += speed[0];
    ent1->m_transform.getOrigin()[1] += speed[1];
    if(!ignore_z && !*ignore_z)
        ent1->m_transform.getOrigin()[2] += speed[2];
    ent1->updatePlatformPreStep();
    ent1->updateRigidBody(true);
}

void lua_MoveEntityToEntity1(int id1, int id2, float speed_mult)
{
    lua_MoveEntityToEntity2(id1, id2, speed_mult, lua::None);
}

void lua_RotateEntity2(int id, float rx, lua::Float ry, lua::Float rz)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
    }
    else
    {
        ent->m_angles[0] += rx;

        if(ry && rz)
        {
             ent->m_angles[1] += *ry;
             ent->m_angles[2] += *rz;
        }

        ent->updateTransform();
        ent->updateRigidBody(true);
    }
}

void lua_RotateEntity1(int id, float rx)
{
    lua_RotateEntity2(id, rx, lua::None, lua::None);
}

lua::OptionalTuple<float,float,float> lua_GetEntitySpeed(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return lua::None;
    }

    return lua::Tuple<float,float,float>
    {
        ent->m_speed[0],
        ent->m_speed[1],
        ent->m_speed[2]
    };
}

lua::Float lua_GetEntitySpeedLinear(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return lua::None;
    }

    return ent->m_speed.length();
}

void lua_SetEntitySpeed2(int id, float vx, lua::Float vy, lua::Float vz)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL) {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
    }
    else {
        ent->m_speed[0] = vx;
        if(vy && vz) {
            ent->m_speed[1] = *vy;
            ent->m_speed[2] = *vz;
        }
        ent->updateCurrentSpeed();
    }
}

void lua_SetEntitySpeed1(int id, float vx)
{
    lua_SetEntitySpeed2(id, vx, lua::None, lua::None);
}

void lua_SetEntityAnim3(int id, int anim, lua::Int frame, lua::Int otherModel)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return;
    }

    if(frame && otherModel)
        ent->setAnimation(anim, *frame, *otherModel);
    else if(frame)
        ent->setAnimation(anim, *frame);
    else
        ent->setAnimation(anim);
}

void lua_SetEntityAnim2(int id, int anim, lua::Int frame)
{
    lua_SetEntityAnim3(id, anim, frame, lua::None);
}

void lua_SetEntityAnim1(int id, int anim)
{
    lua_SetEntityAnim3(id, anim, lua::None, lua::None);
}

void lua_SetEntityAnimFlag(int id, uint16_t anim_flag)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return;
    }

    ent->m_bf.animations.anim_flags = anim_flag;
}

void lua_SetEntityBodyPartFlag(int id, int bone_id, int body_part_flag)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return;
    }

    if(bone_id < 0 || bone_id >= static_cast<int>(ent->m_bf.bone_tags.size()))
    {
        ConsoleInfo::instance().warning(SYSWARN_WRONG_OPTION_INDEX, bone_id);
        return;
    }

    ent->m_bf.bone_tags[bone_id].body_part = body_part_flag;
}

void lua_SetModelBodyPartFlag(int id, int bone_id, int body_part_flag)
{
    SkeletalModel* model = engine_world.getModelByID(id);

    if(model == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_SKELETAL_MODEL, id);
        return;
    }

    if((bone_id < 0) || (bone_id >= model->mesh_count))
    {
        ConsoleInfo::instance().warning(SYSWARN_WRONG_OPTION_INDEX, bone_id);
        return;
    }

    model->mesh_tree[bone_id].body_part = body_part_flag;
}

lua::OptionalTuple<int16_t,int16_t,uint32_t> lua_GetEntityAnim(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return lua::None;
    }

    return lua::Tuple<int16_t,int16_t,uint32_t>
    {
        ent->m_bf.animations.current_animation,
        ent->m_bf.animations.current_frame,
        static_cast<uint32_t>(ent->m_bf.animations.model->animations[ent->m_bf.animations.current_animation].frames.size())
    };
}

bool lua_CanTriggerEntity3(int id1, int id2, lua::Float rOpt, lua::Float ofsX, lua::Float ofsY, lua::Float ofsZ)
{
    std::shared_ptr<Character> e1 = engine_world.getCharacterByID(id1);
    if(!e1 || !e1->m_command.action)
    {
        return false;
    }

    std::shared_ptr<Entity> e2 = engine_world.getEntityByID(id2);
    if((e2 == NULL) || (e1 == e2))
    {
        return false;
    }

    float r = rOpt ? *rOpt : e2->m_activationRadius;
    r *= r;
    auto offset = e2->m_activationOffset;
    if(ofsX && ofsY && ofsZ)
    {
        offset[0] = *ofsX;
        offset[1] = *ofsY;
        offset[2] = *ofsZ;
    }

    auto pos = e2->m_transform * offset;
    if((e1->m_transform.getBasis()[1].dot(e2->m_transform.getBasis()[1]) > 0.75) &&
            ((e1->m_transform.getOrigin() - pos).length2() < r))
    {
        return true;
    }

    return false;
}

bool lua_CanTriggerEntity2(int id1, int id2, lua::Float rOpt)
{
    return lua_CanTriggerEntity3(id1, id2, rOpt, lua::None, lua::None, lua::None);
}

bool lua_CanTriggerEntity1(int id1, int id2)
{
    return lua_CanTriggerEntity3(id1, id2, lua::None, lua::None, lua::None, lua::None);
}

lua::Bool lua_GetEntityVisibility(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return lua::None;
    }

    return ent->m_visible;
}

void lua_SetEntityVisibility(int id, bool value)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return;
    }

    ent->m_visible = value;
}

lua::Bool lua_GetEntityEnability(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return lua::None;
    }

    return ent->m_enabled;
}

lua::Bool lua_GetEntityActivity(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return lua::None;
    }

    return ent->m_active;
}

void lua_SetEntityActivity(int id, bool value)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return;
    }

    ent->m_active = value;
}

lua::OptionalTuple<int,bool,bool> lua_GetEntityTriggerLayout(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);
    if(ent == NULL)
        return lua::None;   // No entity found - return.

    return lua::Tuple<int,bool,bool>
    {
        ent->m_triggerLayout & ENTITY_TLAYOUT_MASK,          // mask
        (ent->m_triggerLayout & ENTITY_TLAYOUT_EVENT),    // event
        (ent->m_triggerLayout & ENTITY_TLAYOUT_LOCK)     // lock
    };
}

void lua_SetEntityTriggerLayout1(int id, uint8_t layout)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return;
    }

    ent->m_triggerLayout = layout;
}

void lua_SetEntityTriggerLayout2(int id, uint8_t mask, bool event, bool once)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return;
    }

    uint8_t trigger_layout = ent->m_triggerLayout;
    trigger_layout &= ~(uint8_t)(ENTITY_TLAYOUT_MASK);  trigger_layout ^= mask;          // mask  - 00011111
    trigger_layout &= ~(uint8_t)(ENTITY_TLAYOUT_EVENT); trigger_layout ^= event << 5;   // event - 00100000
    trigger_layout &= ~(uint8_t)(ENTITY_TLAYOUT_LOCK);  trigger_layout ^= once << 6;   // lock  - 01000000
    ent->m_triggerLayout = trigger_layout;
}

void lua_SetEntityLock(int id, bool lock)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);
    if(ent != NULL)
    {
        uint8_t trigger_layout = ent->m_triggerLayout;
        trigger_layout &= ~(uint8_t)(ENTITY_TLAYOUT_LOCK);
        trigger_layout ^= lock << 6;   // lock  - 01000000
        ent->m_triggerLayout = trigger_layout;
    }
}

lua::Bool lua_GetEntityLock(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);
    if(ent != NULL)
    {
        return (ent->m_triggerLayout & ENTITY_TLAYOUT_LOCK) >> 6;      // lock
    }
    return lua::None;
}

void lua_SetEntityEvent(int id, uint8_t event)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);
    if(ent != NULL)
    {
        uint8_t trigger_layout = ent->m_triggerLayout;
        trigger_layout &= ~(uint8_t)(ENTITY_TLAYOUT_EVENT); trigger_layout ^= event << 5;   // event - 00100000
        ent->m_triggerLayout = trigger_layout;
    }
}

lua::UInt8 lua_GetEntityEvent(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);
    if(ent != NULL)
    {
        return (ent->m_triggerLayout & ENTITY_TLAYOUT_EVENT) >> 5;    // event
    }
    return lua::None;
}

lua::UInt8 lua_GetEntityMask(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);
    if(ent != NULL)
    {
        return ent->m_triggerLayout & ENTITY_TLAYOUT_MASK;          // mask
    }
    return lua::None;
}

void lua_SetEntityMask(int id, uint8_t mask)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);
    if(ent != NULL)
    {
        uint8_t trigger_layout = ent->m_triggerLayout;
        trigger_layout &= ~(uint8_t)(ENTITY_TLAYOUT_MASK);
        trigger_layout ^= mask;   // mask  - 00011111
        ent->m_triggerLayout = trigger_layout;
    }
}

lua::Int lua_GetEntitySectorStatus(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);
    if(ent != NULL)
    {
        return (ent->m_triggerLayout & ENTITY_TLAYOUT_SSTATUS) >> 7;
    }
    return lua::None;
}

void lua_SetEntitySectorStatus(int id, uint8_t status)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);
    if(ent != NULL)
    {
        uint8_t trigger_layout = ent->m_triggerLayout;
        trigger_layout &= ~(uint8_t)(ENTITY_TLAYOUT_SSTATUS);
        trigger_layout ^=  status << 7;   // sector_status  - 10000000
        ent->m_triggerLayout = trigger_layout;
    }
}

lua::Int lua_GetEntityOCB(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);
    if(ent == NULL)
        return lua::None;   // No entity found - return.

    return ent->m_OCB;
}

void lua_SetEntityOCB(int id, int ocb)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);
    if(ent == NULL)
        return;   // No entity found - return.

    ent->m_OCB = ocb;
}

lua::OptionalTuple<bool,bool,bool,uint16_t,uint32_t> lua_GetEntityFlags(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return lua::None;
    }

    return lua::Tuple<bool,bool,bool,uint16_t,uint32_t>
    {
        ent->m_active,
        ent->m_enabled,
        ent->m_visible,
        ent->m_typeFlags,
        ent->m_callbackFlags
    };
}

void lua_SetEntityFlags2(int id, lua::Bool active, lua::Bool enabled, lua::Bool visible, lua::UInt16 typeFlags, lua::UInt32 cbFlags)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return;
    }

    if(active)
        ent->m_active = *active;
    if(enabled)
        ent->m_enabled = *enabled;
    if(visible)
        ent->m_visible = *visible;
    if(typeFlags)
        ent->m_typeFlags = *typeFlags;
    if(cbFlags)
        ent->m_callbackFlags = *cbFlags;
}

void lua_SetEntityFlags1(int id, lua::Bool active, lua::Bool enabled, lua::Bool visible, lua::UInt16 typeFlags)
{
    lua_SetEntityFlags2(id, active, enabled, visible, typeFlags, lua::None);
}

lua::UInt32 lua_GetEntityTypeFlag2(int id, lua::UInt16 flag)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return lua::None;
    }

    if(!flag)
    {
        return ent->m_typeFlags;
    }
    else
    {
        return ent->m_typeFlags & *flag;
    }
}

lua::UInt32 lua_GetEntityTypeFlag1(int id)
{
    return lua_GetEntityTypeFlag2(id, lua::None);
}

void lua_SetEntityTypeFlag2(int id, uint16_t type_flag, lua::Bool value)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return;
    }

    if(!value)
    {
        ent->m_typeFlags ^= type_flag;
    }
    else
    {
        if(*value)
        {
            ent->m_typeFlags |=  type_flag;
        }
        else
        {
            ent->m_typeFlags &= ~*value;
        }
    }
}

void lua_SetEntityTypeFlag1(int id, uint16_t type_flag)
{
    lua_SetEntityTypeFlag2(id, type_flag, lua::None);
}

lua::Bool lua_GetEntityStateFlag(int id, const std::string& which)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return lua::None;
    }

    if(which == "active")
        return ent->m_active;
    else if(which == "enabled")
        return ent->m_enabled;
    else if(which == "visible")
        return ent->m_visible;
    else
        return lua::None;
}

void lua_SetEntityStateFlag2(int id, const std::string& which, lua::Bool value)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return;
    }

    bool* flag = nullptr;
    if(which == "active") flag = &ent->m_active;
    else if(which == "enabled") flag = &ent->m_enabled;
    else if(which == "visible") flag = &ent->m_visible;
    else return;

    if(!value)
    {
        *flag = !*flag;
    }
    else
    {
        *flag = *value;
    }
}

void lua_SetEntityStateFlag1(int id, const std::string& which)
{
    lua_SetEntityStateFlag2(id, which, lua::None);
}

lua::UInt32 lua_GetEntityCallbackFlag2(int id, lua::UInt32 flag)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return lua::None;
    }

    if(!flag)
    {
        return ent->m_callbackFlags;
    }
    else
    {
        return (ent->m_callbackFlags & *flag);
    }
}

lua::UInt32 lua_GetEntityCallbackFlag1(int id)
{
    return lua_GetEntityCallbackFlag2(id, lua::None);
}

void lua_SetEntityCallbackFlag2(int id, uint32_t flag, lua::Bool value)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return;
    }

    if(!value)
    {
        ent->m_callbackFlags ^= flag;
    }
    else
    {
        if(*value)
        {
            ent->m_callbackFlags |= flag;
        }
        else
        {
            ent->m_callbackFlags &= ~flag;
        }
    }
}

void lua_SetEntityCallbackFlag1(int id, uint32_t flag)
{
    lua_SetEntityCallbackFlag2(id, flag, lua::None);
}

lua::Float lua_GetEntityTimer(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);
    if(ent == NULL)
        return lua::None;   // No entity found - return.

    return ent->m_timer;
}

void lua_SetEntityTimer(int id, float timer)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);
    if(ent == NULL)
        return;   // No entity found - return.

    ent->m_timer = timer;
}

lua::UInt16 lua_GetEntityMoveType(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return lua::None;
    }

    return ent->m_moveType;
}

void lua_SetEntityMoveType(int id, uint16_t type)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
        return;
    ent->m_moveType = type;
}

lua::Int8 lua_GetEntityResponse(int id, int response)
{
    std::shared_ptr<Character> ent = engine_world.getCharacterByID(id);

    if(ent)
    {
        switch(response)
        {
        case 0: return ent->m_response.kill;
        case 1: return ent->m_response.vertical_collide;
        case 2: return ent->m_response.horizontal_collide;
        case 3: return ent->m_response.slide;
        default: return lua::Int8(0);
        }
    }
    else
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return lua::None;
    }
}

void lua_SetEntityResponse(int id, int response, int8_t value)
{
    std::shared_ptr<Character> ent = engine_world.getCharacterByID(id);

    if(ent)
    {
        switch(response)
        {
        case 0:
            ent->m_response.kill = value;
            break;
        case 1:
            ent->m_response.vertical_collide = value;
            break;
        case 2:
            ent->m_response.horizontal_collide = value;
            break;
        case 3:
            ent->m_response.slide = value;
            break;
        default:
            break;
        }
    }
    else
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
    }
}

lua::Int16 lua_GetEntityState(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return lua::None;
    }

    return ent->m_bf.animations.last_state;
}

lua::UInt32 lua_GetEntityModel(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return lua::None;
    }

    return ent->m_bf.animations.model->id;
}

void lua_SetEntityState2(int id, int16_t value, lua::Int16 next)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return;
    }

    ent->m_bf.animations.next_state = value;
    if(next)
    {
        ent->m_bf.animations.last_state = *next;
    }
}

void lua_SetEntityState1(int id, int16_t value)
{
    lua_SetEntityState2(id, value, lua::None);
}

void lua_SetEntityRoomMove(int id, lua::UInt room, lua::UInt16 moveType, lua::UInt8 dirFlag)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);
    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return;
    }

    if(room && (*room < engine_world.rooms.size()))
    {
        std::shared_ptr<Room> r = engine_world.rooms[*room];
        if(ent == engine_world.character)
        {
            ent->m_self->room = r.get();
        }
        else if(ent->m_self->room != r.get())
        {
            if(ent->m_self->room != NULL)
            {
                ent->m_self->room->removeEntity(ent.get());
            }
            r->addEntity(ent.get());
        }
    }
    ent->updateRoomPos();

    if(moveType)
    {
        ent->m_moveType = *moveType;
    }
    if(dirFlag)
    {
        ent->m_dirFlag = *dirFlag;
    }
}

lua::UInt32 lua_GetEntityMeshCount(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return lua::None;
    }

    return ent->m_bf.bone_tags.size();
}

void lua_SetEntityMeshswap(int id_dest, int id_src)
{
    std::shared_ptr<Entity>         ent_dest;
    SkeletalModel* model_src;

    ent_dest   = engine_world.getEntityByID(id_dest);
    model_src  = engine_world.getModelByID(id_src);

    int meshes_to_copy = (ent_dest->m_bf.bone_tags.size() > model_src->mesh_count)?(model_src->mesh_count):(ent_dest->m_bf.bone_tags.size());

    for(int i = 0; i < meshes_to_copy; i++)
    {
        ent_dest->m_bf.bone_tags[i].mesh_base = model_src->mesh_tree[i].mesh_base;
        ent_dest->m_bf.bone_tags[i].mesh_skin = model_src->mesh_tree[i].mesh_skin;
    }
}

void lua_SetModelMeshReplaceFlag(int id, int bone, uint8_t flag)
{
    SkeletalModel* sm = engine_world.getModelByID(id);
    if(sm != NULL)
    {
        if((bone >= 0) && (bone < sm->mesh_count))
        {
            sm->mesh_tree[bone].replace_mesh = flag;
        }
        else
        {
            ConsoleInfo::instance().printf("wrong bone number = %d", bone);
        }
    }
    else
    {
        ConsoleInfo::instance().printf("can not find model with id = %d", id);
    }
}

void lua_SetModelAnimReplaceFlag(int id, int bone, uint8_t flag)
{
    SkeletalModel* sm = engine_world.getModelByID(id);
    if(sm != NULL)
    {
        if((bone >= 0) && (bone < sm->mesh_count))
        {
            sm->mesh_tree[bone].replace_anim = flag;
        }
        else
        {
            ConsoleInfo::instance().printf("wrong bone number = %d", bone);
        }
    }
    else
    {
        ConsoleInfo::instance().printf("can not find model with id = %d", id);
    }
}

void lua_CopyMeshFromModelToModel(int id1, int id2, int bone1, int bone2)
{
    SkeletalModel* sm1 = engine_world.getModelByID(id1);
    if(sm1 == NULL)
    {
        ConsoleInfo::instance().printf("can not find model with id = %d", id1);
        return;
    }

    SkeletalModel* sm2 = engine_world.getModelByID(id2);
    if(sm2 == NULL)
    {
        ConsoleInfo::instance().printf("can not find model with id = %d", id2);
        return;
    }

    if((bone1 >= 0) && (bone1 < sm1->mesh_count) && (bone2 >= 0) && (bone2 < sm2->mesh_count))
    {
        sm1->mesh_tree[bone1].mesh_base = sm2->mesh_tree[bone2].mesh_base;
    }
    else
    {
        ConsoleInfo::instance().addLine("wrong bone number = %d", FONTSTYLE_CONSOLE_WARNING);
    }
}

void lua_PushEntityBody(int id, uint32_t body_number, float h_force, float v_force, bool resetFlag)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(!ent && (body_number < ent->m_bf.bone_tags.size()) && (ent->m_bt.bt_body[body_number] != NULL) && (ent->m_typeFlags & ENTITY_TYPE_DYNAMIC))
    {
        btScalar t = ent->m_angles[0] * M_PI / 180.0;

        btScalar ang1 = sinf(t);
        btScalar ang2 = cosf(t);

        btVector3 angle (-ang1 * h_force, ang2 * h_force, v_force);

        if(resetFlag)
            ent->m_bt.bt_body[body_number]->clearForces();

        ent->m_bt.bt_body[body_number]->setLinearVelocity(angle);
        ent->m_bt.bt_body[body_number]->setAngularVelocity(angle / 1024.0);
    }
    else
    {
        ConsoleInfo::instance().printf("Can't apply force to entity %d - no entity, body, or entity is not kinematic!", id);
    }
}

int lua_SetEntityBodyMass(lua_State *lua)
{
    int top = lua_gettop(lua);

    if(lua_gettop(lua) < 3)
    {
        ConsoleInfo::instance().printf("Wrong arguments count. Must be [entity_id, body_number, (mass / each body mass)]");
        return 0;
    }

    const auto id = lua_tounsigned(lua, 1);
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    int body_number = lua_tounsigned(lua, 2);
    body_number = (body_number < 1)?(1):(body_number);

    uint16_t argn  = 3;
    bool dynamic = false;

    btScalar mass;

    if(ent && (static_cast<int>(ent->m_bf.bone_tags.size()) >= body_number))
    {
        for(int i=0; i<body_number; i++)
        {
            btVector3 inertia (0.0, 0.0, 0.0);

            if(top >= argn) mass = lua_tonumber(lua, argn);
            argn++;

            if(ent->m_bt.bt_body[i])
            {
                bt_engine_dynamicsWorld->removeRigidBody(ent->m_bt.bt_body[i].get());

                ent->m_bt.bt_body[i]->getCollisionShape()->calculateLocalInertia(mass, inertia);

                ent->m_bt.bt_body[i]->setMassProps(mass, inertia);

                ent->m_bt.bt_body[i]->updateInertiaTensor();
                ent->m_bt.bt_body[i]->clearForces();

                ent->m_bt.bt_body[i]->getCollisionShape()->setLocalScaling(ent->m_scaling);

                btVector3 factor = (mass > 0.0)?(btVector3(1.0, 1.0, 1.0)):(btVector3(0.0, 0.0, 0.0));
                ent->m_bt.bt_body[i]->setLinearFactor (factor);
                ent->m_bt.bt_body[i]->setAngularFactor(factor);

                //ent->bt_body[i]->forceActivationState(DISABLE_DEACTIVATION);

                //ent->bt_body[i]->setCcdMotionThreshold(32.0);   // disable tunneling effect
                //ent->bt_body[i]->setCcdSweptSphereRadius(32.0);

                bt_engine_dynamicsWorld->addRigidBody(ent->m_bt.bt_body[i].get());

                ent->m_bt.bt_body[i]->activate();

                //ent->bt_body[i]->getBroadphaseHandle()->m_collisionFilterGroup = 0xFFFF;
                //ent->bt_body[i]->getBroadphaseHandle()->m_collisionFilterMask  = 0xFFFF;

                //ent->self->object_type = OBJECT_ENTITY;
                //ent->bt_body[i]->setUserPointer(ent->self);

                if(mass > 0.0) dynamic = true;
            }

        }

        ent->updateRigidBody(true);

        if(dynamic)
        {
            ent->m_typeFlags |=  ENTITY_TYPE_DYNAMIC;
        }
        else
        {
            ent->m_typeFlags &= ~ENTITY_TYPE_DYNAMIC;
        }
    }
    else
    {
        ConsoleInfo::instance().printf("Can't find entity %d or body number is more than %d", id, body_number);
    }

    return 0;
}

void lua_LockEntityBodyLinearFactor2(int id, uint32_t body_number, lua::Float vfactor)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent && (body_number < ent->m_bf.bone_tags.size()) && (ent->m_bt.bt_body[body_number] != NULL) && (ent->m_typeFlags & ENTITY_TYPE_DYNAMIC))
    {
        btScalar t    = ent->m_angles[0] * M_PI / 180.0;
        btScalar ang1 = sinf(t);
        btScalar ang2 = cosf(t);
        btScalar ang3 = 1.0;

        if(vfactor)
        {
            ang3 = abs(*vfactor);
            ang3 = (ang3 > 1.0)?(1.0):(ang3);
        }

        ent->m_bt.bt_body[body_number]->setLinearFactor(btVector3(abs(ang1), abs(ang2), ang3));
    }
    else
    {
        ConsoleInfo::instance().printf("Can't apply force to entity %d - no entity, body, or entity is not dynamic!", id);
    }
}

void lua_LockEntityBodyLinearFactor1(int id, uint32_t body_number)
{
    lua_LockEntityBodyLinearFactor2(id, body_number, lua::None);
}

void lua_SetCharacterWeaponModel(int id, int weaponmodel, int state)
{
    std::shared_ptr<Character> ent = engine_world.getCharacterByID(id);

    if(ent)
    {
        ent->setWeaponModel(weaponmodel, state);
    }
    else
    {
        ConsoleInfo::instance().printf("can not find entity with id = %d", id);
    }
}

lua::Int lua_GetCharacterCurrentWeapon(int id)
{
    std::shared_ptr<Character> ent = engine_world.getCharacterByID(id);

    if(ent)
    {
        return ent->m_currentWeapon;
    }
    else
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return lua::None;
    }
}

void lua_SetCharacterCurrentWeapon(int id, int weapon)
{
    std::shared_ptr<Character> ent = engine_world.getCharacterByID(id);

    if(ent)
    {
        ent->m_currentWeapon = weapon;
    }
    else
    {
        ConsoleInfo::instance().printf("can not find entity with id = %d", id);
    }
}

/*
 * Camera functions
 */

void lua_CamShake2(float power, float time, lua::Int id)
{
    if(id)
    {
        std::shared_ptr<Entity> ent = engine_world.getEntityByID(*id);

        btVector3 cam_pos = renderer.camera()->m_pos;

        btScalar dist = ent->m_transform.getOrigin().distance(cam_pos);
        dist = (dist > TR_CAM_MAX_SHAKE_DISTANCE)?(0):(1.0 - (dist / TR_CAM_MAX_SHAKE_DISTANCE));

        power *= dist;
    }

    if(power > 0.0)
        renderer.camera()->shake(power, time);
}

void lua_CamShake1(float power, float time)
{
    lua_CamShake2(power, time, lua::None);
}

void lua_FlashSetup(uint8_t alpha, uint8_t R, uint8_t G, uint8_t B, uint16_t fadeinSpeed, uint16_t fadeoutSpeed)
{
    Gui_FadeSetup(FADER_EFFECT,
                  alpha,
                  R, G, B,
                  BM_MULTIPLY,
                  fadeinSpeed, fadeoutSpeed);
}

void lua_FlashStart()
{
    Gui_FadeStart(FADER_EFFECT, GUI_FADER_DIR_TIMED);
}

void lua_FadeOut()
{
    Gui_FadeStart(FADER_BLACK, GUI_FADER_DIR_OUT);
}

void lua_FadeIn()
{
    Gui_FadeStart(FADER_BLACK, GUI_FADER_DIR_IN);
}

bool lua_FadeCheck()
{
    return Gui_FadeCheck(FADER_BLACK);
}

/*
 * General gameplay functions
 */

void lua_PlayStream2(int id, uint8_t mask)
{
    if(id < 0)
    {
        ConsoleInfo::instance().warning(SYSWARN_WRONG_STREAM_ID);
        return;
    }

    if(mask)
    {
        Audio_StreamPlay(id, mask);
    }
    else
    {
        Audio_StreamPlay(id);
    }
}

void lua_PlayStream1(int id)
{
    lua_PlayStream2(id, 0);
}

void lua_PlaySound2(uint32_t id, int ent_id)
{
    if(id >= engine_world.audio_map.size())
    {
        ConsoleInfo::instance().warning(SYSWARN_WRONG_SOUND_ID, engine_world.audio_map.size());
        return;
    }

    if(ent_id<0 || !engine_world.getEntityByID(ent_id))
        ent_id = -1;

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
            ConsoleInfo::instance().warning(SYSWARN_AS_NOCHANNEL);
            break;

        case TR_AUDIO_SEND_NOSAMPLE:
            ConsoleInfo::instance().warning(SYSWARN_AS_NOSAMPLE);
            break;
        }
    }
}

void lua_PlaySound1(uint32_t id)
{
    lua_PlaySound2(id, -1);
}

void lua_StopSound2(uint32_t id, int ent_id)
{
    if(id >= engine_world.audio_map.size())
    {
        ConsoleInfo::instance().warning(SYSWARN_WRONG_SOUND_ID, engine_world.audio_map.size());
        return;
    }

    if(ent_id<0 || engine_world.getEntityByID(ent_id) == NULL)
        ent_id = -1;

    int result;

    if(ent_id == -1)
    {
        result = Audio_Kill(id, TR_AUDIO_EMITTER_GLOBAL);
    }
    else
    {
        result = Audio_Kill(id, TR_AUDIO_EMITTER_ENTITY, ent_id);
    }

    if(result < 0)
        ConsoleInfo::instance().warning(SYSWARN_AK_NOTPLAYED, id);
}

void lua_StopSound1(uint32_t id)
{
    lua_StopSound2(id, -1);
}

int lua_GetLevel()
{
    return gameflow_manager.CurrentLevelID;
}

void lua_SetLevel(int id)
{
    ConsoleInfo::instance().notify(SYSNOTE_CHANGING_LEVEL, id);

    Game_LevelTransition(id);
    Gameflow_Send(TR_GAMEFLOW_OP_LEVELCOMPLETE, id);    // Next level
}

void lua_SetGame2(int gameId, lua::Int levelId)
{
    gameflow_manager.CurrentGameID = gameId;
    if(levelId)
        gameflow_manager.CurrentLevelID = *levelId;

    auto state = lua::state();
    const auto oldTop = lua_gettop(state);
    lua_getglobal(state, "getTitleScreen");
    if(lua_isfunction(state, -1))
    {
        lua_pushnumber(state, gameflow_manager.CurrentGameID);
        if (lua_CallAndLog(state, 1, 1, 0))
        {
            Gui_FadeAssignPic(FADER_LOADSCREEN, lua_tostring(state, -1));
            lua_pop(state, 1);
            Gui_FadeStart(FADER_LOADSCREEN, GUI_FADER_DIR_OUT);
        }
    }
    lua_settop(state, oldTop);

    ConsoleInfo::instance().notify(SYSNOTE_CHANGING_GAME, gameflow_manager.CurrentGameID);
    Game_LevelTransition(gameflow_manager.CurrentLevelID);
    Gameflow_Send(TR_GAMEFLOW_OP_LEVELCOMPLETE, gameflow_manager.CurrentLevelID);
}

void lua_SetGame1(int gameId)
{
    lua_SetGame2(gameId, lua::None);
}

void lua_LoadMap3(const std::string& mapName, lua::Int gameId, lua::Int mapId)
{
    ConsoleInfo::instance().printf("Loading map %s", mapName.c_str());
    if(!mapName.empty() && mapName != gameflow_manager.CurrentLevelPath)
    {
        if(gameId)
        {
            gameflow_manager.CurrentGameID = *gameId;
        }
        if(mapId)
        {
            gameflow_manager.CurrentLevelID = *mapId;
        }
        char file_path[MAX_ENGINE_PATH];
        lua_GetLoadingScreen(lua::state(), gameflow_manager.CurrentLevelID, file_path);
        Gui_FadeAssignPic(FADER_LOADSCREEN, file_path);
        Gui_FadeStart(FADER_LOADSCREEN, GUI_FADER_DIR_IN);
        Engine_LoadMap(mapName.c_str());
    }
}

void lua_LoadMap2(const std::string& mapName, lua::Int gameId)
{
    lua_LoadMap3(mapName, gameId, lua::None);
}

void lua_LoadMap1(const std::string& mapName)
{
    lua_LoadMap3(mapName, lua::None, lua::None);
}

/*
 * Flipped (alternate) room functions
 */

void lua_SetFlipState(uint32_t group, bool state)
{
    if(group >= engine_world.flip_data.size())
    {
        ConsoleInfo::instance().warning(SYSWARN_WRONG_FLIPMAP_INDEX);
        return;
    }

    if(engine_world.flip_data[group].map == 0x1F)         // Check flipmap state.
    {
        std::vector< std::shared_ptr<Room> >::iterator current_room = engine_world.rooms.begin();

        if(engine_world.version > TR_III)
        {
            for(; current_room!=engine_world.rooms.end(); ++current_room)
            {
                if((*current_room)->alternate_group == group)    // Check if group is valid.
                {
                    if(state)
                        (*current_room)->swapToAlternate();
                    else
                        (*current_room)->swapToBase();
                }
            }

            engine_world.flip_data[group].state = state;
        }
        else
        {
            for(; current_room!=engine_world.rooms.end(); ++current_room)
            {
                if(state)
                    (*current_room)->swapToAlternate();
                else
                    (*current_room)->swapToBase();
            }

            engine_world.flip_data[0].state = state;    // In TR1-3, state is always global.
        }
    }
}

void lua_SetFlipMap(uint32_t group, uint8_t mask, uint8_t op)
{
    op = (mask > AMASK_OP_XOR)?(AMASK_OP_XOR):(AMASK_OP_OR);

    if(group >= engine_world.flip_data.size())
    {
        ConsoleInfo::instance().warning(SYSWARN_WRONG_FLIPMAP_INDEX);
        return;
    }

    if(op == AMASK_OP_XOR)
    {
        engine_world.flip_data[group].map ^= mask;
    }
    else
    {
        engine_world.flip_data[group].map |= mask;
    }
}

lua::UInt8 lua_GetFlipMap(uint32_t group)
{
    if(group >= engine_world.flip_data.size())
    {
        ConsoleInfo::instance().warning(SYSWARN_WRONG_FLIPMAP_INDEX);
        return lua::None;
    }

    return engine_world.flip_data[group].map;
}

lua::UInt8 lua_GetFlipState(uint32_t group)
{
    if(group >= engine_world.flip_data.size())
    {
        ConsoleInfo::instance().warning(SYSWARN_WRONG_FLIPMAP_INDEX);
        return lua::None;
    }

    return engine_world.flip_data[group].state;
}

/*
 * Generate UV rotate animations
 */

void lua_genUVRotateAnimation(int id)
{
    SkeletalModel* model = engine_world.getModelByID(id);

    if(!model)
        return;

    if(model->mesh_tree.front().mesh_base->m_transparencyPolygons.empty())
        return;
    const struct Polygon& firstPolygon = model->mesh_tree.front().mesh_base->m_transparencyPolygons.front();
    if(firstPolygon.anim_id != 0)
        return;

    engine_world.anim_sequences.emplace_back();
    AnimSeq* seq = &engine_world.anim_sequences.back();

    // Fill up new sequence with frame list.
    seq->anim_type         = TR_ANIMTEXTURE_FORWARD;
    seq->frame_lock        = false; // by default anim is playing
    seq->uvrotate          = true;
    seq->reverse_direction = false; // Needed for proper reverse-type start-up.
    seq->frame_rate        = 0.025;  // Should be passed as 1 / FPS.
    seq->frame_time        = 0.0;   // Reset frame time to initial state.
    seq->current_frame     = 0;     // Reset current frame to zero.
    seq->frames.resize(16);
    seq->frame_list.resize(16);
    seq->frame_list[0] = 0;

    btScalar v_min, v_max;
    v_min = v_max = firstPolygon.vertices[0].tex_coord[1];
    for(size_t j=1; j<firstPolygon.vertices.size(); j++)
    {
        if(firstPolygon.vertices[j].tex_coord[1] > v_max)
        {
            v_max = firstPolygon.vertices[j].tex_coord[1];
        }
        if(firstPolygon.vertices[j].tex_coord[1] < v_min)
        {
            v_min = firstPolygon.vertices[j].tex_coord[1];
        }
    }

    seq->uvrotate_max = 0.5 * (v_max - v_min);
    seq->uvrotate_speed = seq->uvrotate_max / (btScalar)seq->frames.size();
    for(uint16_t j=0;j<seq->frames.size();j++)
    {
        seq->frames[j].tex_ind = firstPolygon.tex_index;
        seq->frames[j].mat[0] = 1.0;
        seq->frames[j].mat[1] = 0.0;
        seq->frames[j].mat[2] = 0.0;
        seq->frames[j].mat[3] = 1.0;
        seq->frames[j].move[0] = 0.0;
        seq->frames[j].move[1] = -((btScalar)j * seq->uvrotate_speed);
    }

    for(struct Polygon& p : model->mesh_tree.front().mesh_base->m_transparencyPolygons) {
        p.anim_id = engine_world.anim_sequences.size();
        for(Vertex& v : p.vertices) {
            v.tex_coord[1] = v_min + 0.5 * (v.tex_coord[1] - v_min) + seq->uvrotate_max;
        }
    }

    return;
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

        // Load script loading order (sic!)

        luaL_dofile(engine_lua, "scripts/loadscript.lua");

        return true;
    }
    else
    {
        return false;
    }
}

void Engine_LuaClearTasks()
{
    lua_CallVoidFunc(engine_lua, "clearTasks");
}

void lua_registerc(lua_State *lua, const char* func_name, int(*func)(lua_State*))
{
    char uc[64] = {0}; char lc[64] = {0};
    for(size_t i=0; i < strlen(func_name); i++)
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
    lua_registerc(lua, "checkStack", WRAP_FOR_LUA(lua_CheckStack));
    lua_registerc(lua, "dumpModel", WRAP_FOR_LUA(lua_DumpModel));
    lua_registerc(lua, "dumpRoom", WRAP_FOR_LUA(lua_dumpRoom1, lua_dumpRoom2));
    lua_registerc(lua, "setRoomEnabled", WRAP_FOR_LUA(lua_SetRoomEnabled));

    lua_registerc(lua, "playSound", WRAP_FOR_LUA(lua_PlaySound1, lua_PlaySound2));
    lua_registerc(lua, "stopSound", WRAP_FOR_LUA(lua_StopSound1, lua_StopSound2));

    lua_registerc(lua, "playStream", WRAP_FOR_LUA(lua_PlayStream1, lua_PlayStream2));

    lua_registerc(lua, "setLevel", WRAP_FOR_LUA(lua_SetLevel));
    lua_registerc(lua, "getLevel", WRAP_FOR_LUA(lua_GetLevel));

    lua_registerc(lua, "setGame", WRAP_FOR_LUA(lua_SetGame1, lua_SetGame2));
    lua_registerc(lua, "loadMap", WRAP_FOR_LUA(lua_LoadMap1, lua_LoadMap2, lua_LoadMap3));

    lua_register(lua, "camShake", WRAP_FOR_LUA(lua_CamShake2, lua_CamShake1));

    lua_register(lua, "fadeOut", WRAP_FOR_LUA(lua_FadeOut));
    lua_register(lua, "fadeIn", WRAP_FOR_LUA(lua_FadeIn));
    lua_register(lua, "fadeCheck", WRAP_FOR_LUA(lua_FadeCheck));

    lua_register(lua, "flashSetup", WRAP_FOR_LUA(lua_FlashSetup));
    lua_register(lua, "flashStart", WRAP_FOR_LUA(lua_FlashStart));

    lua_register(lua, "getLevelVersion", WRAP_FOR_LUA(lua_GetLevelVersion));

    lua_register(lua, "setFlipMap", WRAP_FOR_LUA(lua_SetFlipMap));
    lua_register(lua, "getFlipMap", WRAP_FOR_LUA(lua_GetFlipMap));
    lua_register(lua, "setFlipState", WRAP_FOR_LUA(lua_SetFlipState));
    lua_register(lua, "getFlipState", WRAP_FOR_LUA(lua_GetFlipState));

    lua_register(lua, "setModelCollisionMapSize", WRAP_FOR_LUA(lua_SetModelCollisionMapSize));
    lua_register(lua, "setModelCollisionMap", WRAP_FOR_LUA(lua_SetModelCollisionMap));
    lua_register(lua, "getAnimCommandTransform", WRAP_FOR_LUA(lua_GetAnimCommandTransform));
    lua_register(lua, "setAnimCommandTransform", WRAP_FOR_LUA(lua_SetAnimCommandTransform1, lua_SetAnimCommandTransform2));
    lua_register(lua, "setStateChangeRange", WRAP_FOR_LUA(lua_SetStateChangeRange2, lua_SetStateChangeRange1));

    lua_register(lua, "addItem", WRAP_FOR_LUA(lua_AddItem3, lua_AddItem2));
    lua_register(lua, "removeItem", WRAP_FOR_LUA(lua_RemoveItem));
    lua_register(lua, "removeAllItems", WRAP_FOR_LUA(lua_RemoveAllItems));
    lua_register(lua, "getItemsCount", WRAP_FOR_LUA(lua_GetItemsCount));
    lua_register(lua, "createBaseItem", WRAP_FOR_LUA(lua_CreateBaseItem2, lua_CreateBaseItem1));
    lua_register(lua, "deleteBaseItem", WRAP_FOR_LUA(lua_DeleteBaseItem));
    lua_register(lua, "printItems", WRAP_FOR_LUA(lua_PrintItems));

    lua_register(lua, "canTriggerEntity", WRAP_FOR_LUA(lua_CanTriggerEntity3,lua_CanTriggerEntity2,lua_CanTriggerEntity1));
    lua_register(lua, "spawnEntity", WRAP_FOR_LUA(lua_SpawnEntity2,lua_SpawnEntity1));
    lua_register(lua, "enableEntity", WRAP_FOR_LUA(lua_EnableEntity));
    lua_register(lua, "disableEntity", WRAP_FOR_LUA(lua_DisableEntity));

    lua_register(lua, "sameRoom", WRAP_FOR_LUA(lua_SameRoom));
    lua_register(lua, "newSector", WRAP_FOR_LUA(lua_NewSector));
    lua_register(lua, "similarSector", WRAP_FOR_LUA(lua_SimilarSector2,lua_SimilarSector1));
    lua_register(lua, "getSectorHeight", WRAP_FOR_LUA(lua_GetSectorHeight3,lua_GetSectorHeight2,lua_GetSectorHeight1));

    lua_register(lua, "moveEntityGlobal", WRAP_FOR_LUA(lua_MoveEntityGlobal));
    lua_register(lua, "moveEntityLocal", WRAP_FOR_LUA(lua_MoveEntityLocal));
    lua_register(lua, "moveEntityToSink", WRAP_FOR_LUA(lua_MoveEntityToSink));
    lua_register(lua, "moveEntityToEntity", WRAP_FOR_LUA(lua_MoveEntityToEntity2,lua_MoveEntityToEntity1));
    lua_register(lua, "rotateEntity", WRAP_FOR_LUA(lua_RotateEntity2, lua_RotateEntity1));

    lua_register(lua, "getEntityModelID", WRAP_FOR_LUA(lua_GetEntityModelID));

    lua_register(lua, "getEntityVector", WRAP_FOR_LUA(lua_GetEntityVector));
    lua_register(lua, "getEntityDirDot", WRAP_FOR_LUA(lua_GetEntityDirDot));
    lua_register(lua, "getEntityDistance", WRAP_FOR_LUA(lua_GetEntityDistance));
    lua_register(lua, "getEntityPos", WRAP_FOR_LUA(lua_GetEntityPosition));
    lua_register(lua, "setEntityPos", WRAP_FOR_LUA(lua_SetEntityPosition2,lua_SetEntityPosition1));
    lua_register(lua, "getEntityAngles", WRAP_FOR_LUA(lua_GetEntityAngles));
    lua_register(lua, "setEntityAngles", WRAP_FOR_LUA(lua_SetEntityAngles2,lua_SetEntityAngles1));
    lua_register(lua, "getEntityScaling", WRAP_FOR_LUA(lua_GetEntityScaling));
    lua_register(lua, "setEntityScaling", WRAP_FOR_LUA(lua_SetEntityScaling));
    lua_register(lua, "getEntitySpeed", WRAP_FOR_LUA(lua_GetEntitySpeed));
    lua_register(lua, "setEntitySpeed", WRAP_FOR_LUA(lua_SetEntitySpeed2,lua_SetEntitySpeed1));
    lua_register(lua, "getEntitySpeedLinear", WRAP_FOR_LUA(lua_GetEntitySpeedLinear));
    lua_register(lua, "setEntityCollision", WRAP_FOR_LUA(lua_SetEntityCollision));
    lua_register(lua, "setEntityCollisionFlags", WRAP_FOR_LUA(lua_SetEntityCollisionFlags3,lua_SetEntityCollisionFlags2,lua_SetEntityCollisionFlags1));
    lua_register(lua, "getEntityAnim", WRAP_FOR_LUA(lua_GetEntityAnim));
    lua_register(lua, "setEntityAnim", WRAP_FOR_LUA(lua_SetEntityAnim3,lua_SetEntityAnim2,lua_SetEntityAnim1));
    lua_register(lua, "setEntityAnimFlag", WRAP_FOR_LUA(lua_SetEntityAnimFlag));
    lua_register(lua, "setEntityBodyPartFlag", WRAP_FOR_LUA(lua_SetEntityBodyPartFlag));
    lua_register(lua, "setModelBodyPartFlag", WRAP_FOR_LUA(lua_SetModelBodyPartFlag));
    lua_register(lua, "getEntityModel", WRAP_FOR_LUA(lua_GetEntityModel));
    lua_register(lua, "getEntityVisibility", WRAP_FOR_LUA(lua_GetEntityVisibility));
    lua_register(lua, "setEntityVisibility", WRAP_FOR_LUA(lua_SetEntityVisibility));
    lua_register(lua, "getEntityActivity", WRAP_FOR_LUA(lua_GetEntityActivity));
    lua_register(lua, "setEntityActivity", WRAP_FOR_LUA(lua_SetEntityActivity));
    lua_register(lua, "getEntityEnability", WRAP_FOR_LUA(lua_GetEntityEnability));
    lua_register(lua, "getEntityOCB", WRAP_FOR_LUA(lua_GetEntityOCB));
    lua_register(lua, "setEntityOCB", WRAP_FOR_LUA(lua_SetEntityOCB));
    lua_register(lua, "getEntityTimer", WRAP_FOR_LUA(lua_GetEntityTimer));
    lua_register(lua, "setEntityTimer", WRAP_FOR_LUA(lua_SetEntityTimer));
    lua_register(lua, "getEntityFlags", WRAP_FOR_LUA(lua_GetEntityFlags));
    lua_register(lua, "setEntityFlags", WRAP_FOR_LUA(lua_SetEntityFlags2,lua_SetEntityFlags1));
    lua_register(lua, "getEntityTypeFlag", WRAP_FOR_LUA(lua_GetEntityTypeFlag2,lua_GetEntityTypeFlag1));
    lua_register(lua, "setEntityTypeFlag", WRAP_FOR_LUA(lua_SetEntityTypeFlag2,lua_SetEntityTypeFlag1));
    lua_register(lua, "getEntityStateFlag", WRAP_FOR_LUA(lua_GetEntityStateFlag));
    lua_register(lua, "setEntityStateFlag", WRAP_FOR_LUA(lua_SetEntityStateFlag2,lua_SetEntityStateFlag1));
    lua_register(lua, "getEntityCallbackFlag", WRAP_FOR_LUA(lua_GetEntityCallbackFlag2,lua_GetEntityCallbackFlag1));
    lua_register(lua, "setEntityCallbackFlag", WRAP_FOR_LUA(lua_SetEntityCallbackFlag2,lua_SetEntityCallbackFlag1));
    lua_register(lua, "getEntityState", WRAP_FOR_LUA(lua_GetEntityState));
    lua_register(lua, "setEntityState", WRAP_FOR_LUA(lua_SetEntityState2,lua_SetEntityState1));
    lua_register(lua, "setEntityRoomMove", WRAP_FOR_LUA(lua_SetEntityRoomMove));
    lua_register(lua, "getEntityMoveType", WRAP_FOR_LUA(lua_GetEntityMoveType));
    lua_register(lua, "setEntityMoveType", WRAP_FOR_LUA(lua_SetEntityMoveType));
    lua_register(lua, "getEntityResponse", WRAP_FOR_LUA(lua_GetEntityResponse));
    lua_register(lua, "setEntityResponse", WRAP_FOR_LUA(lua_SetEntityResponse));
    lua_register(lua, "getEntityMeshCount", WRAP_FOR_LUA(lua_GetEntityMeshCount));
    lua_register(lua, "setEntityMeshswap", WRAP_FOR_LUA(lua_SetEntityMeshswap));
    lua_register(lua, "setModelMeshReplaceFlag", WRAP_FOR_LUA(lua_SetModelMeshReplaceFlag));
    lua_register(lua, "setModelAnimReplaceFlag", WRAP_FOR_LUA(lua_SetModelAnimReplaceFlag));
    lua_register(lua, "copyMeshFromModelToModel", WRAP_FOR_LUA(lua_CopyMeshFromModelToModel));

    lua_register(lua, "setEntityBodyMass", lua_SetEntityBodyMass);
    lua_register(lua, "pushEntityBody", WRAP_FOR_LUA(lua_PushEntityBody));
    lua_register(lua, "lockEntityBodyLinearFactor", WRAP_FOR_LUA(lua_LockEntityBodyLinearFactor2,lua_LockEntityBodyLinearFactor1));

    lua_register(lua, "getEntityTriggerLayout", WRAP_FOR_LUA(lua_GetEntityTriggerLayout));
    lua_register(lua, "setEntityTriggerLayout", WRAP_FOR_LUA(lua_SetEntityTriggerLayout2,lua_SetEntityTriggerLayout1));
    lua_register(lua, "getEntityMask", WRAP_FOR_LUA(lua_GetEntityMask));
    lua_register(lua, "setEntityMask", WRAP_FOR_LUA(lua_SetEntityMask));
    lua_register(lua, "getEntityEvent", WRAP_FOR_LUA(lua_GetEntityEvent));
    lua_register(lua, "setEntityEvent", WRAP_FOR_LUA(lua_SetEntityEvent));
    lua_register(lua, "getEntityLock", WRAP_FOR_LUA(lua_GetEntityLock));
    lua_register(lua, "setEntityLock", WRAP_FOR_LUA(lua_SetEntityLock));
    lua_register(lua, "getEntitySectorStatus", WRAP_FOR_LUA(lua_GetEntitySectorStatus));
    lua_register(lua, "setEntitySectorStatus", WRAP_FOR_LUA(lua_SetEntitySectorStatus));

    lua_register(lua, "getEntityActivationOffset", WRAP_FOR_LUA(lua_GetEntityActivationOffset));
    lua_register(lua, "setEntityActivationOffset", WRAP_FOR_LUA(lua_SetEntityActivationOffset2,lua_SetEntityActivationOffset1));
    lua_register(lua, "getEntitySectorIndex", WRAP_FOR_LUA(lua_GetEntitySectorIndex));
    lua_register(lua, "getEntitySectorFlags", WRAP_FOR_LUA(lua_GetEntitySectorFlags));
    lua_register(lua, "getEntitySectorMaterial", WRAP_FOR_LUA(lua_GetEntitySectorMaterial));

    lua_register(lua, "addEntityRagdoll", WRAP_FOR_LUA(lua_AddEntityRagdoll));
    lua_register(lua, "removeEntityRagdoll", WRAP_FOR_LUA(lua_RemoveEntityRagdoll));

    lua_register(lua, "getCharacterParam", WRAP_FOR_LUA(lua_GetCharacterParam));
    lua_register(lua, "setCharacterParam", WRAP_FOR_LUA(lua_SetCharacterParam2,lua_SetCharacterParam1));
    lua_register(lua, "changeCharacterParam", WRAP_FOR_LUA(lua_ChangeCharacterParam));
    lua_register(lua, "getCharacterCurrentWeapon", WRAP_FOR_LUA(lua_GetCharacterCurrentWeapon));
    lua_register(lua, "setCharacterCurrentWeapon", WRAP_FOR_LUA(lua_SetCharacterCurrentWeapon));
    lua_register(lua, "setCharacterWeaponModel", WRAP_FOR_LUA(lua_SetCharacterWeaponModel));
    lua_register(lua, "getCharacterCombatMode", WRAP_FOR_LUA(lua_GetCharacterCombatMode));

    lua_register(lua, "addCharacterHair", WRAP_FOR_LUA(lua_AddCharacterHair));
    lua_register(lua, "resetCharacterHair", WRAP_FOR_LUA(lua_ResetCharacterHair));

    lua_register(lua, "getSecretStatus", WRAP_FOR_LUA(lua_GetSecretStatus));
    lua_register(lua, "setSecretStatus", WRAP_FOR_LUA(lua_SetSecretStatus));

    lua_register(lua, "getActionState", WRAP_FOR_LUA(lua_GetActionState));
    lua_register(lua, "getActionChange", WRAP_FOR_LUA(lua_GetActionChange));

    lua_register(lua, "genUVRotateAnimation", WRAP_FOR_LUA(lua_genUVRotateAnimation));

    lua_register(lua, "getGravity", WRAP_FOR_LUA(lua_GetGravity));
    lua_register(lua, "setGravity", WRAP_FOR_LUA(lua_SetGravity3,lua_SetGravity2,lua_SetGravity1));
    lua_register(lua, "dropEntity", WRAP_FOR_LUA(lua_DropEntity2, lua_DropEntity1));
    lua_register(lua, "bind", WRAP_FOR_LUA(lua_BindKey2,lua_BindKey1));

    lua_register(lua, "addFont", WRAP_FOR_LUA(lua_AddFont));
    lua_register(lua, "deleteFont", WRAP_FOR_LUA(lua_DeleteFont));
    lua_register(lua, "addFontStyle", WRAP_FOR_LUA(lua_AddFontStyle));
    lua_register(lua, "deleteFontStyle", WRAP_FOR_LUA(lua_DeleteFontStyle));
}


void Engine_Destroy()
{
    renderer.empty();
    //ConsoleInfo::instance().destroy();
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
    renderer.empty();
    engine_world.empty();
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


int engine_lua_fputs(const char *str, FILE* /*f*/)
{
    ConsoleInfo::instance().addText(str, FONTSTYLE_CONSOLE_NOTIFY);
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
    ConsoleInfo::instance().addText(buf, FONTSTYLE_CONSOLE_NOTIFY);

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

    ConsoleInfo::instance().addText(buf, FONTSTYLE_CONSOLE_NOTIFY);

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

int Engine_GetLevelFormat(const char* /*name*/)
{
    // PLACEHOLDER: Currently, only PC levels are supported.

    return LEVEL_FORMAT_PC;
}


int Engine_GetPCLevelVersion(const char *name)
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

    for(char *ch=level_name;*ch!=0;ch++)
    {
        *ch = toupper(*ch);
    }

    strcat(name, level_name);
    if(postfix) strcat(name, postfix);
    strcat(name, ".lua");
}

bool Engine_LoadPCLevel(const char *name)
{
    VT_Level *tr_level = new VT_Level();

    int trv = Engine_GetPCLevelVersion(name);
    if(trv == TR_UNKNOWN) return false;

    tr_level->read_level(name, trv);
    tr_level->prepare_level();
    //tr_level->dump_textures();

    TR_GenWorld(&engine_world, tr_level);

    char buf[LEVEL_NAME_MAX_LEN] = {0x00};
    Engine_GetLevelName(buf, name);

    ConsoleInfo::instance().notify(SYSNOTE_LOADED_PC_LEVEL);
    ConsoleInfo::instance().notify(SYSNOTE_ENGINE_VERSION, trv, buf);
    ConsoleInfo::instance().notify(SYSNOTE_NUM_ROOMS, engine_world.rooms.size());

    delete tr_level;

    return true;
}

int Engine_LoadMap(const char *name)
{
    if(!Engine_FileFound(name))
    {
        ConsoleInfo::instance().warning(SYSWARN_FILE_NOT_FOUND, name);
        return 0;
    }

    Gui_DrawLoadScreen(0);

    renderer.hideSkyBox();
    renderer.resetRListActiveCount();
    renderer.resetWorld();

    strncpy(gameflow_manager.CurrentLevelPath, name, MAX_ENGINE_PATH);          // it is needed for "not in the game" levels or correct saves loading.

    Gui_DrawLoadScreen(50);

    engine_world.empty();
    engine_world.prepare();

    lua_Clean(engine_lua);

    Audio_Init();

    Gui_DrawLoadScreen(100);


    // Here we can place different platform-specific level loading routines.

    switch(Engine_GetLevelFormat(name))
    {
    case LEVEL_FORMAT_PC:
        if(Engine_LoadPCLevel(name) == false) return 0;
        break;

    case LEVEL_FORMAT_PSX:
        break;

    case LEVEL_FORMAT_DC:
        break;

    case LEVEL_FORMAT_OPENTOMB:
        break;

    default:
        break;
    }

    engine_world.id   = 0;
    engine_world.name = 0;
    engine_world.type = 0;

    Game_Prepare();

    renderer.setWorld(&engine_world);

    Gui_DrawLoadScreen(1000);

    Gui_FadeStart(FADER_LOADSCREEN, GUI_FADER_DIR_IN);
    Gui_NotifierStop();

    return 1;
}

int Engine_ExecCmd(const char *ch)
{
    char token[ConsoleInfo::instance().lineSize()];
    char buf[ConsoleInfo::instance().lineSize() + 32];
    const char *pch;
    RoomSector* sect;
    FILE *f;

    while(ch!=NULL)
    {
        pch = ch;
        ch = parse_token(ch, token);
        if(!strcmp(token, "help"))
        {
            ConsoleInfo::instance().addLine("Available commands:", FONTSTYLE_CONSOLE_WARNING);
            ConsoleInfo::instance().addLine("help - show help info", FONTSTYLE_CONSOLE_NOTIFY);
            ConsoleInfo::instance().addLine("loadMap(\"file_name\") - load level \"file_name\"", FONTSTYLE_CONSOLE_NOTIFY);
            ConsoleInfo::instance().addLine("save, load - save and load game state in \"file_name\"", FONTSTYLE_CONSOLE_NOTIFY);
            ConsoleInfo::instance().addLine("exit - close program", FONTSTYLE_CONSOLE_NOTIFY);
            ConsoleInfo::instance().addLine("cls - clean console", FONTSTYLE_CONSOLE_NOTIFY);
            ConsoleInfo::instance().addLine("show_fps - switch show fps flag", FONTSTYLE_CONSOLE_NOTIFY);
            ConsoleInfo::instance().addLine("spacing - read and write spacing", FONTSTYLE_CONSOLE_NOTIFY);
            ConsoleInfo::instance().addLine("showing_lines - read and write number of showing lines", FONTSTYLE_CONSOLE_NOTIFY);
            ConsoleInfo::instance().addLine("cvars - lua's table of cvar's, to see them type: show_table(cvars)", FONTSTYLE_CONSOLE_NOTIFY);
            ConsoleInfo::instance().addLine("free_look - switch camera mode", FONTSTYLE_CONSOLE_NOTIFY);
            ConsoleInfo::instance().addLine("cam_distance - camera distance to actor", FONTSTYLE_CONSOLE_NOTIFY);
            ConsoleInfo::instance().addLine("r_wireframe, r_portals, r_frustums, r_room_boxes, r_boxes, r_normals, r_skip_room - render modes\0", FONTSTYLE_CONSOLE_NOTIFY);
            ConsoleInfo::instance().addLine("playsound(id) - play specified sound", FONTSTYLE_CONSOLE_NOTIFY);
            ConsoleInfo::instance().addLine("stopsound(id) - stop specified sound", FONTSTYLE_CONSOLE_NOTIFY);
            ConsoleInfo::instance().addLine("Watch out for case sensitive commands!", FONTSTYLE_CONSOLE_WARNING);
        }
        else if(!strcmp(token, "goto"))
        {
            control_states.free_look = 1;
            renderer.camera()->m_pos[0] = SC_ParseFloat(&ch);
            renderer.camera()->m_pos[1] = SC_ParseFloat(&ch);
            renderer.camera()->m_pos[2] = SC_ParseFloat(&ch);
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
            ConsoleInfo::instance().clean();
            return 1;
        }
        else if(!strcmp(token, "spacing"))
        {
            ch = parse_token(ch, token);
            if(NULL == ch)
            {
                ConsoleInfo::instance().notify(SYSNOTE_CONSOLE_SPACING, ConsoleInfo::instance().spacing());
                return 1;
            }
            ConsoleInfo::instance().setLineInterval(atof(token));
            return 1;
        }
        else if(!strcmp(token, "showing_lines"))
        {
            ch = parse_token(ch, token);
            if(NULL == ch)
            {
                ConsoleInfo::instance().notify(SYSNOTE_CONSOLE_LINECOUNT, ConsoleInfo::instance().visibleLines());
                return 1;
            }
            else
            {
                const auto val = atoi(token);
                if((val >=2 ) && (val <= screen_info.h/ConsoleInfo::instance().lineHeight()))
                {
                    ConsoleInfo::instance().setVisibleLines( val );
                    ConsoleInfo::instance().setCursorY( screen_info.h - ConsoleInfo::instance().lineHeight() * ConsoleInfo::instance().visibleLines() );
                }
                else
                {
                    ConsoleInfo::instance().warning(SYSWARN_INVALID_LINECOUNT);
                }
            }
            return 1;
        }
        else if(!strcmp(token, "r_wireframe"))
        {
            renderer.toggleWireframe();
            return 1;
        }
        else if(!strcmp(token, "r_points"))
        {
            renderer.toggleDrawPoints();
            return 1;
        }
        else if(!strcmp(token, "r_coll"))
        {
            renderer.toggleDrawColl();
            return 1;
        }
        else if(!strcmp(token, "r_normals"))
        {
            renderer.toggleDrawNormals();
            return 1;
        }
        else if(!strcmp(token, "r_portals"))
        {
            renderer.toggleDrawPortals();
            return 1;
        }
        else if(!strcmp(token, "r_frustums"))
        {
            renderer.toggleDrawFrustums();
            return 1;
        }
        else if(!strcmp(token, "r_room_boxes"))
        {
            renderer.toggleDrawRoomBoxes();
            return 1;
        }
        else if(!strcmp(token, "r_boxes"))
        {
            renderer.toggleDrawBoxes();
            return 1;
        }
        else if(!strcmp(token, "r_axis"))
        {
            renderer.toggleDrawAxis();
            return 1;
        }
        else if(!strcmp(token, "r_nullmeshes"))
        {
            renderer.toggleDrawNullMeshes();
            return 1;
        }
        else if(!strcmp(token, "r_dummy_statics"))
        {
            renderer.toggleDrawDummyStatics();
            return 1;
        }
        else if(!strcmp(token, "r_skip_room"))
        {
            renderer.toggleSkipRoom();
            return 1;
        }
        else if(!strcmp(token, "room_info"))
        {
            if(Room* r = renderer.camera()->m_currentRoom)
            {
                sect = r->getSectorXYZ(renderer.camera()->m_pos);
                ConsoleInfo::instance().printf("ID = %d, x_sect = %d, y_sect = %d", r->id, r->sectors_x, r->sectors_y);
                if(sect)
                {
                    ConsoleInfo::instance().printf("sect(%d, %d), inpenitrable = %d, r_up = %d, r_down = %d", sect->index_x, sect->index_y,
                                                   (int)(sect->ceiling == TR_METERING_WALLHEIGHT || sect->floor == TR_METERING_WALLHEIGHT), (int)(sect->sector_above != NULL), (int)(sect->sector_below != NULL));
                    for(uint32_t i=0;i<sect->owner_room->static_mesh.size();i++)
                    {
                        ConsoleInfo::instance().printf("static[%d].object_id = %d", i, sect->owner_room->static_mesh[i]->object_id);
                    }
                    for(const std::shared_ptr<EngineContainer>& cont : sect->owner_room->containers)
                    {
                        if(cont->object_type == OBJECT_ENTITY)
                        {
                            Entity* e = static_cast<Entity*>(cont->object);
                            ConsoleInfo::instance().printf("cont[entity](%d, %d, %d).object_id = %d", (int)e->m_transform.getOrigin()[0], (int)e->m_transform.getOrigin()[1], (int)e->m_transform.getOrigin()[2], e->m_id);
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
                ConsoleInfo::instance().clean();
                ConsoleInfo::instance().addText(buf, FONTSTYLE_CONSOLE_INFO);
                free(buf);
            }
            else
            {
                ConsoleInfo::instance().addText("Not avaliable =(", FONTSTYLE_CONSOLE_WARNING);
            }
            return 1;
        }
        else if(token[0])
        {
            if(engine_lua)
            {
                ConsoleInfo::instance().addLine(pch, FONTSTYLE_CONSOLE_EVENT);
                if (luaL_dostring(engine_lua, pch) != LUA_OK)
                {
                    ConsoleInfo::instance().addLine(lua_tostring(engine_lua, -1), FONTSTYLE_CONSOLE_WARNING);
                    lua_pop(engine_lua, 1);
                }
            }
            else
            {
                snprintf(buf, ConsoleInfo::instance().lineSize() + 32, "Command \"%s\" not found", token);
                ConsoleInfo::instance().addLine(buf, FONTSTYLE_CONSOLE_WARNING);
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
            lua_register(lua, "bind", WRAP_FOR_LUA(lua_BindKey2,lua_BindKey1));                             // get and set key bindings
            luaL_dofile(lua, filename);

            lua_ParseScreen(lua, &screen_info);
            lua_ParseRender(lua, &renderer.settings());
            lua_ParseAudio(lua, &audio_settings);
            lua_ParseConsole(lua, &ConsoleInfo::instance());
            lua_ParseControls(lua, &control_mapper);
            lua_close(lua);
        }
        else
        {
            Sys_Warn("Could not find \"%s\"", filename);
        }
    }
}
