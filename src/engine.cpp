
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

#include <lua.hpp>
#include "LuaState.h"

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

static btScalar                        *frame_vertex_buffer = nullptr;
static size_t                           frame_vertex_buffer_size = 0;
static size_t                           frame_vertex_buffer_size_left = 0;

lua::State engine_lua;

btDefaultCollisionConfiguration         *bt_engine_collisionConfiguration = nullptr;
btCollisionDispatcher                   *bt_engine_dispatcher = nullptr;
btGhostPairCallback                     *bt_engine_ghostPairCallback = nullptr;
btBroadphaseInterface                   *bt_engine_overlappingPairCache = nullptr;
btSequentialImpulseConstraintSolver     *bt_engine_solver = nullptr;
btDiscreteDynamicsWorld                 *bt_engine_dynamicsWorld = nullptr;
btOverlapFilterCallback                 *bt_engine_filterCallback = nullptr;

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
        assert( i>=0 && i<bt_engine_dynamicsWorld->getCollisionObjectArray().size() );
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

    engine_lua["loadscript_pre"]();

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
    engine_lua["loadscript_post"]();

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
    ConsoleInfo::instance().printf("Current Lua stack index: %d", lua_gettop(engine_lua.getState()));
}

int lua_print(lua_State* state)
{
     const int top = lua_gettop(state);

     if(top == 0)
     {
        ConsoleInfo::instance().addLine("nil", FONTSTYLE_CONSOLE_EVENT);
        return 0;
     }

     for(int i=1; i<=top; i++)
     {
         const char* str = lua_tostring(state, i);
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
                Sys_DebugLog("room_dump.txt", "entity: id = %d, model = %d", ent->id(), ent->m_bf.animations.model->id);
            }
        }
    }
}

void lua_dumpRoom(lua::Value id)
{
    if(id.is<lua::Nil>()) {
        dumpRoom(engine_camera.m_currentRoom);
        return;
    }
    if(id.is<lua::Integer>() && static_cast<uint32_t>(id) >= engine_world.rooms.size())
    {
        ConsoleInfo::instance().warning(SYSWARN_WRONG_ROOM, static_cast<int>(id));
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

void lua_SetEntityCollisionFlags(int id, lua::Value ctype, lua::Value cshape)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);
    if(ent != NULL)
    {
        if(ctype.is<lua::Integer>())
        {
            ent->m_self->collision_type = ctype;
        }
        if(cshape.is<lua::Integer>())
        {
            ent->m_self->collision_shape = cshape;
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

uint32_t lua_GetEntitySectorFlags(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if((ent != NULL) && (ent->m_currentSector))
    {
        return ent->m_currentSector->flags;
    }
    return 0;
}

uint32_t lua_GetEntitySectorIndex(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if((ent != NULL) && (ent->m_currentSector))
    {
        return ent->m_currentSector->trig_index;
    }
    return 0;
}

uint32_t lua_GetEntitySectorMaterial(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if((ent != NULL) && (ent->m_currentSector))
    {
        return ent->m_currentSector->material;
    }
    return 0;
}

bool lua_SameRoom(int id1, int id2)
{
    std::shared_ptr<Entity> ent1 = engine_world.getEntityByID(id1);
    std::shared_ptr<Entity> ent2 = engine_world.getEntityByID(id2);

    if(ent1 && ent2)
    {
        return ent1->m_self->room == ent2->m_self->room;
    }

    return false;
}

bool lua_NewSector(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent != NULL)
    {
        return ent->m_currentSector == ent->m_lastSector;
    }
    return false;
}

std::tuple<float,float,float> lua_GetGravity()
{
    btVector3 g = bt_engine_dynamicsWorld->getGravity();
    return std::tuple<float,float,float>{
        g[0],
        g[1],
        g[2]
    };
}

void lua_SetGravity(float x, lua::Value y, lua::Value z)                                             // function to be exported to Lua
{
    btVector3 g( x, y.is<lua::Number>() ? y : 0, z.is<lua::Number>() ? z : 0 );
    bt_engine_dynamicsWorld->setGravity(g);
    ConsoleInfo::instance().printf("gravity = (%.3f, %.3f, %.3f)", g[0], g[1], g[2]);
}

bool lua_DropEntity(int id, float time, lua::Value only_room)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);
    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return false;
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
        EngineContainer* cont = (EngineContainer*)cb.m_collisionObject->getUserPointer();

        if(!only_room.is<lua::Boolean>() || !static_cast<bool>(only_room) || (static_cast<bool>(only_room) && (cont->object_type == OBJECT_ROOM_BASE)))
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

int lua_GetEntityModelID(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);
    if(ent == NULL)
        return -1;

    if(ent->m_bf.animations.model)
    {
        return ent->m_bf.animations.model->id;
    }
    return -1;
}

std::tuple<float,float,float,float> lua_GetEntityActivationOffset(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);
    if(ent == NULL)
        return {};

    return std::tuple<float,float,float,float>(
                ent->m_activationOffset[0],
                ent->m_activationOffset[1],
                ent->m_activationOffset[2],
                ent->m_activationRadius);
}

void lua_SetEntityActivationOffset(int id, float x, float y, float z, lua::Value r)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);
    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return;
    }

    ent->m_activationOffset = {x,y,z};
    if(r.is<lua::Number>())
        ent->m_activationRadius = r;
}

int lua_GetCharacterParam(int id, int parameter)
{
    std::shared_ptr<Character> ent = engine_world.getCharacterByID(id);

    if(parameter >= PARAM_SENTINEL)
    {
        ConsoleInfo::instance().warning(SYSWARN_WRONG_OPTION_INDEX, PARAM_SENTINEL);
        return -1;
    }

    if(ent)
    {
        return ent->getParam(parameter);
    }
    else
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_CHARACTER, id);
        return -1;
    }
}

void lua_SetCharacterParam(int id, int parameter, float value, lua::Value max_value)
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
    else if(!max_value.is<lua::Number>())
    {
        ent->setParam(parameter, value);
    }
    else
    {
        ent->m_parameters.param[parameter] = value;
        ent->m_parameters.maximum[parameter] = max_value;
    }
}

int lua_GetCharacterCombatMode(int id)
{
    std::shared_ptr<Character> ent = engine_world.getCharacterByID(id);

    if(ent)
    {
        return static_cast<int>(ent->m_weaponCurrentState);
    }

    return -1;
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

        hair_setup.getSetup(setup_index);
        ent->m_hairs.emplace_back(std::make_shared<Hair>());

        if(!ent->m_hairs.back()->create(&hair_setup, ent))
        {
            ConsoleInfo::instance().warning(SYSWARN_CANT_CREATE_HAIR, ent_id);
            ent->m_hairs.pop_back();
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

bool lua_GetSecretStatus(int secret_number)
{
    if((secret_number > TR_GAMEFLOW_MAX_SECRETS) || (secret_number < 0))
        return false;   // No such secret - return

    return gameflow_manager.SecretsTriggerMap[secret_number];
}

void lua_SetSecretStatus(int secret_number, bool status)
{
    if((secret_number > TR_GAMEFLOW_MAX_SECRETS) || (secret_number < 0)) return;   // No such secret - return

    gameflow_manager.SecretsTriggerMap[secret_number] = status;
}

bool lua_GetActionState(int act)
{
    if(act < 0 || act >= ACT_LASTINDEX)
    {
        ConsoleInfo::instance().warning(SYSWARN_WRONG_ACTION_NUMBER);
        return false;
    }
    else
    {
        return control_mapper.action_map[act].state;
    }
}

bool lua_GetActionChange(int act)
{
    if(act < 0 || act >= ACT_LASTINDEX)
    {
        ConsoleInfo::instance().warning(SYSWARN_WRONG_ACTION_NUMBER);
        return false;
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

void lua_BindKey(int act, int primary, lua::Value secondary)
{
    if(act < 0 || act >= ACT_LASTINDEX)
    {
        ConsoleInfo::instance().warning(SYSWARN_WRONG_ACTION_NUMBER);
    }
    control_mapper.action_map[act].primary = primary;
    if(secondary.is<lua::Integer>())
        control_mapper.action_map[act].secondary = secondary;
}

void lua_AddFont(int index, const char* path, uint32_t size)
{
    if(!FontManager->AddFont((font_Type)index, size, path))
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

int lua_AddItem(int entity_id, int item_id, lua::Value count)
{
    std::shared_ptr<Character> ent = engine_world.getCharacterByID(entity_id);

    if(ent)
    {
        return ent->addItem(item_id, count.is<lua::Integer>() ? count : -1);
    }
    else
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, entity_id);
        return -1;
    }
}

int lua_RemoveItem(int entity_id, int item_id, int count)
{
    std::shared_ptr<Character> ent = engine_world.getCharacterByID(entity_id);

    if(ent)
    {
        return ent->removeItem(item_id, count);
    }
    else
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, entity_id);
        return -1;
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

int lua_GetItemsCount(int entity_id, int item_id)
{
    std::shared_ptr<Character> ent = engine_world.getCharacterByID(entity_id);

    if(ent)
    {
        return ent->getItemsCount(item_id);
    }
    else
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, entity_id);
        return -1;
    }

}

void lua_CreateBaseItem(int item_id, int model_id, int world_model_id, int type, int count, const char* name)
{

    engine_world.createItem(item_id, model_id, world_model_id, type, count, name ? name : std::string());
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

void lua_SetStateChangeRange(int id, int anim, int state, int dispatch, int frame_low, int frame_high, lua::Value next_anim, lua::Value next_frame)
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
                if(!next_anim.is<lua::Nil>() && !next_frame.is<lua::Nil>()) {
                    af->state_change[i].anim_dispatch[dispatch].next_anim = next_anim;
                    af->state_change[i].anim_dispatch[dispatch].next_frame = next_frame;
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

std::tuple<int,float,float,float> lua_GetAnimCommandTransform(int id, int anim, int frame)
{
    SkeletalModel* model = engine_world.getModelByID(id);
    if(model == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_SKELETAL_MODEL, id);
        return {};
    }

    if(anim < 0 || anim + 1 > static_cast<int>(model->animations.size()))
    {
        ConsoleInfo::instance().warning(SYSWARN_WRONG_ANIM_NUMBER, anim);
        return {};
    }

    if(frame < 0)                                                               // it is convenient to use -1 as a last frame number
    {
        frame = (int)model->animations[anim].frames.size() + frame;
    }

    if(frame < 0 || frame + 1 > static_cast<int>(model->animations[anim].frames.size()))
    {
        ConsoleInfo::instance().warning(SYSWARN_WRONG_FRAME_NUMBER, frame);
        return {};
    }

    return std::tuple<int,float,float,float>
    {
        model->animations[anim].frames[frame].command,
             model->animations[anim].frames[frame].move[0],
             model->animations[anim].frames[frame].move[1],
             model->animations[anim].frames[frame].move[2]
    };
}

void lua_SetAnimCommandTransform(int id, int anim, int frame, int flag, lua::Value dx, lua::Value dy, lua::Value dz)
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

    if(dx.is<lua::Number>() && dy.is<lua::Number>() && dz.is<lua::Number>())
        model->animations[anim].frames[frame].move = {dx,dy,dz};
}

uint32_t lua_SpawnEntity(int model_id, int room_id, float x, float y, float z, float ax, float ay, float az, lua::Value ov_id)
{
    btVector3 pos{x,y,z}, ang{ax,ay,az};

    uint32_t id = engine_world.spawnEntity(model_id, room_id, &pos, &ang, ov_id.is<lua::Integer>() ? ov_id : -1);
    if(id == 0xFFFFFFFF)
    {
        return -1;
    }
    else
    {
        return id;
    }
}

/*
 * Moveables script control section
 */
std::tuple<float,float,float> lua_GetEntityVector(int id1, int id2)
{
    std::shared_ptr<Entity> e1 = engine_world.getEntityByID(id1);
    if(e1 == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id1);
        return {};
    }
    std::shared_ptr<Entity> e2 = engine_world.getEntityByID(id2);
    if(e2 == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id2);
        return {};
    }

    return std::tuple<float,float,float>
    {
        e2->m_transform.getOrigin()[0] - e1->m_transform.getOrigin()[0],
        e2->m_transform.getOrigin()[1] - e1->m_transform.getOrigin()[1],
        e2->m_transform.getOrigin()[2] - e1->m_transform.getOrigin()[2]
    };
}

float lua_GetEntityDistance(int id1, int id2)
{
    std::shared_ptr<Entity> e1 = engine_world.getEntityByID(id1);
    if(!e1)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id1);
        return std::numeric_limits<float>::max();
    }
    std::shared_ptr<Entity> e2 = engine_world.getEntityByID(id2);
    if(!e2)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id2);
        return std::numeric_limits<float>::max();
    }

    return e1->findDistance(*e2);
}

float lua_GetEntityDirDot(int id1, int id2)
{
    std::shared_ptr<Entity> e1 = engine_world.getEntityByID(id1);
    if(!e1)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id2);
        return std::numeric_limits<float>::max();
    }
    std::shared_ptr<Entity> e2 = engine_world.getEntityByID(id2);
    if(!e2)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id2);
        return std::numeric_limits<float>::max();
    }

    return e1->m_transform.getBasis()[1].dot(e2->m_transform.getBasis()[1]);
}

std::tuple<float,float,float,float,float,float> lua_GetEntityPosition(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);
    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return {};
    }

    return std::tuple<float,float,float,float,float,float>
    {
        ent->m_transform.getOrigin()[0],
        ent->m_transform.getOrigin()[1],
        ent->m_transform.getOrigin()[2],
        ent->m_angles[0],
        ent->m_angles[1],
        ent->m_angles[2]
    };
}

std::tuple<float,float,float> lua_GetEntityAngles(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return {};
    }

    return std::tuple<float,float,float>
    {
        ent->m_angles[0],
        ent->m_angles[1],
        ent->m_angles[2]
    };
}

std::tuple<float,float,float> lua_GetEntityScaling(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(!ent) {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return {};
    }

    return std::tuple<float,float,float>
    {
        ent->m_scaling[0],
        ent->m_scaling[1],
        ent->m_scaling[2]
    };
}

bool lua_SimilarSector(int id, float dx, float dy, float dz, bool ignore_doors, lua::Value ceiling)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(!ent)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return {};
    }

    auto next_pos = ent->m_transform.getOrigin() + (dx * ent->m_transform.getBasis()[0] + dy * ent->m_transform.getBasis()[1] + dz * ent->m_transform.getBasis()[2]);

    RoomSector* curr_sector = ent->m_self->room->getSectorRaw(ent->m_transform.getOrigin());
    RoomSector* next_sector = ent->m_self->room->getSectorRaw(next_pos);

    curr_sector = curr_sector->checkPortalPointer();
    next_sector = next_sector->checkPortalPointer();

    if(ceiling.is<lua::Boolean>() && static_cast<bool>(ceiling))
    {
        return curr_sector->similarCeiling(next_sector, ignore_doors);
    }
    else
    {
        return curr_sector->similarFloor(next_sector, ignore_doors);
    }
}

float lua_GetSectorHeight(int id, lua::Value ceiling, lua::Value dx, lua::Value dy, lua::Value dz)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(!ent)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return 0;
    }

    auto pos = ent->m_transform.getOrigin();

    if(dx.is<lua::Number>() && dy.is<lua::Number>() && dz.is<lua::Number>())
        pos += static_cast<btScalar>(dx) * ent->m_transform.getBasis()[0] + static_cast<btScalar>(dy) * ent->m_transform.getBasis()[1] + static_cast<btScalar>(dz) * ent->m_transform.getBasis()[2];

    RoomSector* curr_sector = ent->m_self->room->getSectorRaw(pos);
    curr_sector = curr_sector->checkPortalPointer();
    btVector3 point = (ceiling.is<lua::Boolean>() && static_cast<bool>(ceiling))
                    ? curr_sector->getCeilingPoint()
                    : curr_sector->getFloorPoint();

    return point[2];
}

void lua_SetEntityPosition(int id, float x, float y, float z, lua::Value ax, lua::Value ay, lua::Value az)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);
    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return;
    }
    ent->m_transform.getOrigin() = {x,y,z};
    if(ax.is<lua::Number>() && ay.is<lua::Number>() && az.is<lua::Number>())
        ent->m_angles = {ax,ay,az};
    ent->updateTransform();
    ent->updatePlatformPreStep();
}

void lua_SetEntityAngles(int id, float x, lua::Value y, lua::Value z)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
    }
    else
    {
        if(!y.is<lua::Number>() || !z.is<lua::Number>())
            ent->m_angles[0] = x;
        else
            ent->m_angles = {x,y,z};
        ent->updateTransform();
    }
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

void lua_MoveEntityToEntity(int id1, int id2, float speed_mult, lua::Value ignore_z)
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
    if(!ignore_z.is<lua::Boolean>() || !static_cast<bool>(ignore_z))
        ent1->m_transform.getOrigin()[2] += speed[2];
    ent1->updatePlatformPreStep();
    ent1->updateRigidBody(true);
}

void lua_RotateEntity(int id, float rx, lua::Value ry, lua::Value rz)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
    }
    else
    {
        if(!ry.is<lua::Number>() || !rz.is<lua::Number>())
            ent->m_angles += {rx,0,0};
        else
            ent->m_angles += {rx,ry,rz};
        ent->updateTransform();
        ent->updateRigidBody(true);
    }
}

std::tuple<float,float,float> lua_GetEntitySpeed(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return {};
    }

    return std::tuple<float,float,float>
    {
        ent->m_speed[0],
        ent->m_speed[1],
        ent->m_speed[2]
    };
}

float lua_GetEntitySpeedLinear(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return 0;
    }

    return ent->m_speed.length();
}

void lua_SetEntitySpeed(int id, float vx, lua::Value vy, lua::Value vz)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL) {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
    }
    else {
        if(!vy.is<lua::Number>() || !vz.is<lua::Number>())
            ent->m_speed[0] = vx;
        else
            ent->m_speed = {vx,vy,vz};
        ent->updateCurrentSpeed();
    }
}

void lua_SetEntityAnim(int id, int anim, lua::Value frame, lua::Value otherModel)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return;
    }

    if(frame.is<lua::Number>() && otherModel.is<lua::Number>())
        ent->setAnimation(anim, frame, otherModel);
    else if(frame.is<lua::Number>())
        ent->setAnimation(anim, frame);
    else
        ent->setAnimation(anim);
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

std::tuple<int16_t,int16_t,uint32_t> lua_GetEntityAnim(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return {};
    }

    return std::tuple<int16_t,int16_t,uint32_t>
    {
        ent->m_bf.animations.current_animation,
        ent->m_bf.animations.current_frame,
        static_cast<uint32_t>(ent->m_bf.animations.model->animations[ent->m_bf.animations.current_animation].frames.size())
    };
}

bool lua_CanTriggerEntity(int id1, int id2, lua::Value rv, lua::Value ofsX, lua::Value ofsY, lua::Value ofsZ)
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

    float r;
    if(!rv.is<lua::Number>() || rv<0)
        r = e2->m_activationRadius;
    else
        r = rv;
    r *= r;

    btVector3 offset;
    if(ofsX.is<lua::Number>() && ofsY.is<lua::Number>() && ofsZ.is<lua::Number>())
        offset = {ofsX,ofsY,ofsZ};
    else
        offset = e2->m_activationOffset;

    auto pos = e2->m_transform * offset;
    if((e1->m_transform.getBasis()[1].dot(e2->m_transform.getBasis()[1]) > 0.75) &&
            ((e1->m_transform.getOrigin() - pos).length2() < r))
    {
        return true;
    }

    return false;
}

bool lua_GetEntityVisibility(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return false;
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

bool lua_GetEntityEnability(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return false;
    }

    return ent->m_enabled;
}

bool lua_GetEntityActivity(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return false;
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

std::tuple<int,bool,bool> lua_GetEntityTriggerLayout(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);
    if(ent == NULL)
        return {};   // No entity found - return.

    return std::tuple<int,bool,bool>
    {
        ent->m_triggerLayout & ENTITY_TLAYOUT_MASK,          // mask
        (ent->m_triggerLayout & ENTITY_TLAYOUT_EVENT),    // event
        (ent->m_triggerLayout & ENTITY_TLAYOUT_LOCK)     // lock
    };
}

void lua_SetEntityTriggerLayout1(int id, int layout)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return;
    }

    ent->m_triggerLayout = layout;
}

void lua_SetEntityTriggerLayout(int id, int mask, lua::Value eventOrLayout, lua::Value once)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return;
    }

    if(once.is<lua::Boolean>()) {
        uint8_t trigger_layout = ent->m_triggerLayout;
        trigger_layout &= ~(uint8_t)(ENTITY_TLAYOUT_MASK);  trigger_layout ^= mask;          // mask  - 00011111
        trigger_layout &= ~(uint8_t)(ENTITY_TLAYOUT_EVENT); trigger_layout ^= static_cast<bool>(eventOrLayout) << 5;   // event - 00100000
        trigger_layout &= ~(uint8_t)(ENTITY_TLAYOUT_LOCK);  trigger_layout ^= static_cast<bool>(once) << 6;   // lock  - 01000000
        ent->m_triggerLayout = trigger_layout;
    }
    else {
        ent->m_triggerLayout = static_cast<int>(eventOrLayout);
    }
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

bool lua_GetEntityLock(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);
    if(ent != NULL)
    {
        return (ent->m_triggerLayout & ENTITY_TLAYOUT_LOCK) >> 6;      // lock
    }
    return false;
}

void lua_SetEntityEvent(int id, bool event)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);
    if(ent != NULL)
    {
        uint8_t trigger_layout = ent->m_triggerLayout;
        trigger_layout &= ~(uint8_t)(ENTITY_TLAYOUT_EVENT); trigger_layout ^= event << 5;   // event - 00100000
        ent->m_triggerLayout = trigger_layout;
    }
}

bool lua_GetEntityEvent(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);
    if(ent != NULL)
    {
        return (ent->m_triggerLayout & ENTITY_TLAYOUT_EVENT) >> 5;    // event
    }
    return false;
}

int lua_GetEntityMask(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);
    if(ent != NULL)
    {
        return ent->m_triggerLayout & ENTITY_TLAYOUT_MASK;          // mask
    }
    return -1;
}

void lua_SetEntityMask(int id, int mask)
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

bool lua_GetEntitySectorStatus(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);
    if(ent != NULL)
    {
        return (ent->m_triggerLayout & ENTITY_TLAYOUT_SSTATUS) >> 7;
    }
    return -1;
}

void lua_SetEntitySectorStatus(int id, bool status)
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

int lua_GetEntityOCB(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);
    if(ent == NULL)
        return -1;   // No entity found - return.

    return ent->m_OCB;
}

void lua_SetEntityOCB(int id, int ocb)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);
    if(ent == NULL)
        return;   // No entity found - return.

    ent->m_OCB = ocb;
}

std::tuple<bool,bool,bool,uint16_t,uint32_t> lua_GetEntityFlags(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return {};
    }

    return std::tuple<bool,bool,bool,uint16_t,uint32_t>
    {
        ent->m_active,
        ent->m_enabled,
        ent->m_visible,
        ent->m_typeFlags,
        ent->m_callbackFlags
    };
}

void lua_SetEntityFlags(int id, bool active, bool enabled, bool visible, uint16_t typeFlags, lua::Value cbFlags)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(!ent)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return;
    }

    ent->m_active = active;
    ent->m_enabled = enabled;
    ent->m_visible = visible;
    ent->m_typeFlags = typeFlags;
    if(cbFlags.is<lua::Integer>())
        ent->m_callbackFlags = cbFlags;
}

uint32_t lua_GetEntityTypeFlag(int id, lua::Value flag)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return -1;
    }

    if(flag.is<lua::Integer>())
        return ent->m_typeFlags & static_cast<uint16_t>(flag);
    else
        return ent->m_typeFlags;
}

void lua_SetEntityTypeFlag(int id, uint16_t type_flag, lua::Value value)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return;
    }
    if(!value.is<lua::Boolean>()) {
        ent->m_typeFlags ^= type_flag;
        return;
    }

    if(static_cast<bool>(value))
    {
        ent->m_typeFlags |=  type_flag;
    }
    else
    {
        ent->m_typeFlags &= ~type_flag;
    }
}

bool lua_GetEntityStateFlag(int id, const char* whichCstr)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return false;
    }

    std::string which = whichCstr;
    if(which == "active")
        return ent->m_active;
    else if(which == "enabled")
        return ent->m_enabled;
    else if(which == "visible")
        return ent->m_visible;
    else
        return false;
}

void lua_SetEntityStateFlag(int id, const char* whichCstr, lua::Value value)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return;
    }

    bool* flag = nullptr;
    std::string which = whichCstr;
    if(which == "active") flag = &ent->m_active;
    else if(which == "enabled") flag = &ent->m_enabled;
    else if(which == "visible") flag = &ent->m_visible;
    else return;

    if(value.is<lua::Boolean>())
        *flag = value;
    else
        *flag = !*flag;
}

uint32_t lua_GetEntityCallbackFlag(int id, lua::Value flag)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return 0;
    }

    if(!flag.is<lua::Integer>())
        return ent->m_callbackFlags;
    else
        return ent->m_callbackFlags & static_cast<uint16_t>(flag);
}

void lua_SetEntityCallbackFlag(int id, uint32_t flag, lua::Value value)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return;
    }

    if(!value.is<lua::Boolean>()) {
        ent->m_callbackFlags ^= flag;
        return;
    }

    if(static_cast<bool>(value))
    {
        ent->m_callbackFlags |= flag;
    }
    else
    {
        ent->m_callbackFlags &= ~flag;
    }
}

float lua_GetEntityTimer(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);
    if(ent == NULL)
        return std::numeric_limits<float>::max();   // No entity found - return.

    return ent->m_timer;
}

void lua_SetEntityTimer(int id, float timer)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);
    if(ent == NULL)
        return;   // No entity found - return.

    ent->m_timer = timer;
}

uint16_t lua_GetEntityMoveType(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return -1;
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

int lua_GetEntityResponse(int id, int response)
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
        default: return 0;
        }
    }
    else
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return 0;
    }
}

void lua_SetEntityResponse(int id, int response, int value)
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

int16_t lua_GetEntityState(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return -1;
    }

    return ent->m_bf.animations.last_state;
}

uint32_t lua_GetEntityModel(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return -1;
    }

    return ent->m_bf.animations.model->id;
}

void lua_SetEntityState(int id, int16_t value, lua::Value next)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return;
    }

    ent->m_bf.animations.next_state = value;
    if(next.is<lua::Integer>())
        ent->m_bf.animations.last_state = next;
}

void lua_SetEntityRoomMove(int id, uint32_t room, uint16_t moveType, int dirFlag)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);
    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return;
    }

    if(room < engine_world.rooms.size())
    {
        std::shared_ptr<Room> r = engine_world.rooms[room];
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

    ent->m_moveType = moveType;
    ent->m_dirFlag = dirFlag;
}

uint32_t lua_GetEntityMeshCount(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return 0;
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

void lua_SetModelMeshReplaceFlag(int id, int bone, int flag)
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

void lua_SetModelAnimReplaceFlag(int id, int bone, int flag)
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

void lua_LockEntityBodyLinearFactor(int id, uint32_t body_number, lua::Value vfactor)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent && (body_number < ent->m_bf.bone_tags.size()) && (ent->m_bt.bt_body[body_number] != NULL) && (ent->m_typeFlags & ENTITY_TYPE_DYNAMIC))
    {
        btScalar t    = ent->m_angles[0] * M_PI / 180.0;
        btScalar ang1 = sinf(t);
        btScalar ang2 = cosf(t);
        btScalar ang3 = 1.0;

        if(vfactor.is<lua::Number>())
        {
            ang3 = std::abs(vfactor.to<float>());
            ang3 = (ang3 > 1.0)?(1.0):(ang3);
        }

        ent->m_bt.bt_body[body_number]->setLinearFactor(btVector3(std::abs(ang1), std::abs(ang2), ang3));
    }
    else
    {
        ConsoleInfo::instance().printf("Can't apply force to entity %d - no entity, body, or entity is not dynamic!", id);
    }
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

int lua_GetCharacterCurrentWeapon(int id)
{
    std::shared_ptr<Character> ent = engine_world.getCharacterByID(id);

    if(ent)
    {
        return ent->m_currentWeapon;
    }
    else
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return -1;
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

void lua_CamShake(float power, float time, lua::Value id)
{
    if(!id.is<lua::Nil>() && id>=0)
    {
        std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

        btVector3 cam_pos = renderer.camera()->m_pos;

        btScalar dist = ent->m_transform.getOrigin().distance(cam_pos);
        dist = (dist > TR_CAM_MAX_SHAKE_DISTANCE)?(0):(1.0 - (dist / TR_CAM_MAX_SHAKE_DISTANCE));

        power *= dist;
    }

    if(power > 0.0)
        renderer.camera()->shake(power, time);
}

void lua_FlashSetup(int alpha, int R, int G, int B, uint16_t fadeinSpeed, uint16_t fadeoutSpeed)
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

void lua_PlayStream(int id, lua::Value mask)
{
    if(id < 0)
    {
        ConsoleInfo::instance().warning(SYSWARN_WRONG_STREAM_ID);
        return;
    }

    if(!mask.is<lua::Nil>())
    {
        Audio_StreamPlay(id, static_cast<int>(mask));
    }
    else
    {
        Audio_StreamPlay(id);
    }
}

void lua_PlaySound(uint32_t id, lua::Value ent_id)
{
    if(id >= engine_world.audio_map.size())
    {
        ConsoleInfo::instance().warning(SYSWARN_WRONG_SOUND_ID, engine_world.audio_map.size());
        return;
    }

    int eid = -1;
    if(!ent_id.is<lua::Nil>())
        eid = static_cast<int>(ent_id);
    if(eid<0 || !engine_world.getEntityByID(eid))
        eid = -1;

    int result;

    if(eid >= 0)
    {
        result = Audio_Send(id, TR_AUDIO_EMITTER_ENTITY, eid);
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

void lua_StopSound(uint32_t id, lua::Value ent_id)
{
    if(id >= engine_world.audio_map.size())
    {
        ConsoleInfo::instance().warning(SYSWARN_WRONG_SOUND_ID, engine_world.audio_map.size());
        return;
    }

    int eid = -1;
    if(!ent_id.is<lua::Nil>())
        eid = static_cast<int>(ent_id);
    if(eid<0 || engine_world.getEntityByID(eid) == NULL)
        eid = -1;

    int result;

    if(eid == -1)
    {
        result = Audio_Kill(id, TR_AUDIO_EMITTER_GLOBAL);
    }
    else
    {
        result = Audio_Kill(id, TR_AUDIO_EMITTER_ENTITY, eid);
    }

    if(result < 0)
        ConsoleInfo::instance().warning(SYSWARN_AK_NOTPLAYED, id);
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

void lua_SetGame(int gameId, lua::Value levelId)
{
    gameflow_manager.CurrentGameID = gameId;
    if(!levelId.is<lua::Nil>() && levelId>=0)
        gameflow_manager.CurrentLevelID = levelId;

    const char* str = engine_lua["getTitleScreen"](int(gameflow_manager.CurrentGameID));
    Gui_FadeAssignPic(FADER_LOADSCREEN, str);
    Gui_FadeStart(FADER_LOADSCREEN, GUI_FADER_DIR_OUT);

    ConsoleInfo::instance().notify(SYSNOTE_CHANGING_GAME, gameflow_manager.CurrentGameID);
    Game_LevelTransition(gameflow_manager.CurrentLevelID);
    Gameflow_Send(TR_GAMEFLOW_OP_LEVELCOMPLETE, gameflow_manager.CurrentLevelID);
}

void lua_LoadMap(const char* mapName, lua::Value gameId, lua::Value mapId)
{
    ConsoleInfo::instance().printf("Loading map %s", mapName);
    if(mapName && mapName != gameflow_manager.CurrentLevelPath)
    {
        if(gameId.is<lua::Integer>() && gameId>=0)
        {
            gameflow_manager.CurrentGameID = static_cast<int>(gameId);
        }
        if(mapId.is<lua::Integer>() && mapId>=0)
        {
            gameflow_manager.CurrentLevelID = mapId;
        }
        char file_path[MAX_ENGINE_PATH];
        lua_GetLoadingScreen(engine_lua, gameflow_manager.CurrentLevelID, file_path);
        Gui_FadeAssignPic(FADER_LOADSCREEN, file_path);
        Gui_FadeStart(FADER_LOADSCREEN, GUI_FADER_DIR_IN);
        Engine_LoadMap(mapName);
    }
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

void lua_SetFlipMap(uint32_t group, int mask, int op)
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

int lua_GetFlipMap(uint32_t group)
{
    if(group >= engine_world.flip_data.size())
    {
        ConsoleInfo::instance().warning(SYSWARN_WRONG_FLIPMAP_INDEX);
        return 0;
    }

    return engine_world.flip_data[group].map;
}

int lua_GetFlipState(uint32_t group)
{
    if(group >= engine_world.flip_data.size())
    {
        ConsoleInfo::instance().warning(SYSWARN_WRONG_FLIPMAP_INDEX);
        return 0;
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

void Engine_LuaInit()
{
    Engine_LuaRegisterFuncs(engine_lua);
    lua_atpanic(engine_lua.getState(), engine_LuaPanic);

    // Load script loading order (sic!)

    luaL_dofile(engine_lua.getState(), "scripts/loadscript.lua");
}

void Engine_LuaClearTasks()
{
    engine_lua["clearTasks"]();
}


void Engine_LuaRegisterFuncs(lua::State& state)
{
    /*
     * register globals
     */
    char cvar_init[64]; cvar_init[0] = 0;
    strcat(cvar_init, CVAR_LUA_TABLE_NAME); strcat(cvar_init, " = {};");
    state.doString(cvar_init);

    Game_RegisterLuaFunctions(state);

    // Register script functions

    lua_registerc(state, "print", lua_print);

    lua_registerc(state, "checkStack", lua_CheckStack);
    lua_registerc(state, "dumpModel", lua_DumpModel);
    lua_registerc(state, "dumpRoom", lua_dumpRoom);
    lua_registerc(state, "setRoomEnabled", lua_SetRoomEnabled);

    lua_registerc(state, "playSound", lua_PlaySound);
    lua_registerc(state, "stopSound", lua_StopSound);

    lua_registerc(state, "playStream", lua_PlayStream);

    lua_registerc(state, "setLevel", lua_SetLevel);
    lua_registerc(state, "getLevel", lua_GetLevel);

    lua_registerc(state, "setGame", lua_SetGame);
    lua_registerc(state, "loadMap", lua_LoadMap);

    lua_registerc(state, "camShake", lua_CamShake);

    lua_registerc(state, "fadeOut", lua_FadeOut);
    lua_registerc(state, "fadeIn", lua_FadeIn);
    lua_registerc(state, "fadeCheck", lua_FadeCheck);

    lua_registerc(state, "flashSetup", lua_FlashSetup);
    lua_registerc(state, "flashStart", lua_FlashStart);

    lua_registerc(state, "getLevelVersion", lua_GetLevelVersion);

    lua_registerc(state, "setFlipMap", lua_SetFlipMap);
    lua_registerc(state, "getFlipMap", lua_GetFlipMap);
    lua_registerc(state, "setFlipState", lua_SetFlipState);
    lua_registerc(state, "getFlipState", lua_GetFlipState);

    lua_registerc(state, "setModelCollisionMapSize", lua_SetModelCollisionMapSize);
    lua_registerc(state, "setModelCollisionMap", lua_SetModelCollisionMap);
    lua_registerc(state, "getAnimCommandTransform", lua_GetAnimCommandTransform);
    lua_registerc(state, "setAnimCommandTransform", lua_SetAnimCommandTransform);
    lua_registerc(state, "setStateChangeRange", lua_SetStateChangeRange);

    lua_registerc(state, "addItem", lua_AddItem);
    lua_registerc(state, "removeItem", lua_RemoveItem);
    lua_registerc(state, "removeAllItems", lua_RemoveAllItems);
    lua_registerc(state, "getItemsCount", lua_GetItemsCount);
    lua_registerc(state, "createBaseItem", lua_CreateBaseItem);
    lua_registerc(state, "deleteBaseItem", lua_DeleteBaseItem);
    lua_registerc(state, "printItems", lua_PrintItems);

    lua_registerc(state, "canTriggerEntity", lua_CanTriggerEntity);
    lua_registerc(state, "spawnEntity", lua_SpawnEntity);
    lua_registerc(state, "enableEntity", lua_EnableEntity);
    lua_registerc(state, "disableEntity", lua_DisableEntity);

    lua_registerc(state, "sameRoom", lua_SameRoom);
    lua_registerc(state, "newSector", lua_NewSector);
    lua_registerc(state, "similarSector", lua_SimilarSector);
    lua_registerc(state, "getSectorHeight", lua_GetSectorHeight);

    lua_registerc(state, "moveEntityGlobal", lua_MoveEntityGlobal);
    lua_registerc(state, "moveEntityLocal", lua_MoveEntityLocal);
    lua_registerc(state, "moveEntityToSink", lua_MoveEntityToSink);
    lua_registerc(state, "moveEntityToEntity", lua_MoveEntityToEntity);
    lua_registerc(state, "rotateEntity", lua_RotateEntity);

    lua_registerc(state, "getEntityModelID", lua_GetEntityModelID);

    lua_registerc(state, "getEntityVector", lua_GetEntityVector);
    lua_registerc(state, "getEntityDirDot", lua_GetEntityDirDot);
    lua_registerc(state, "getEntityDistance", lua_GetEntityDistance);
    lua_registerc(state, "getEntityPos", lua_GetEntityPosition);
    lua_registerc(state, "setEntityPos", lua_SetEntityPosition);
    lua_registerc(state, "getEntityAngles", lua_GetEntityAngles);
    lua_registerc(state, "setEntityAngles", lua_SetEntityAngles);
    lua_registerc(state, "getEntityScaling", lua_GetEntityScaling);
    lua_registerc(state, "setEntityScaling", lua_SetEntityScaling);
    lua_registerc(state, "getEntitySpeed", lua_GetEntitySpeed);
    lua_registerc(state, "setEntitySpeed", lua_SetEntitySpeed);
    lua_registerc(state, "getEntitySpeedLinear", lua_GetEntitySpeedLinear);
    lua_registerc(state, "setEntityCollision", lua_SetEntityCollision);
    lua_registerc(state, "setEntityCollisionFlags", lua_SetEntityCollisionFlags);
    lua_registerc(state, "getEntityAnim", lua_GetEntityAnim);
    lua_registerc(state, "setEntityAnim", lua_SetEntityAnim);
    lua_registerc(state, "setEntityAnimFlag", lua_SetEntityAnimFlag);
    lua_registerc(state, "setEntityBodyPartFlag", lua_SetEntityBodyPartFlag);
    lua_registerc(state, "setModelBodyPartFlag", lua_SetModelBodyPartFlag);
    lua_registerc(state, "getEntityModel", lua_GetEntityModel);
    lua_registerc(state, "getEntityVisibility", lua_GetEntityVisibility);
    lua_registerc(state, "setEntityVisibility", lua_SetEntityVisibility);
    lua_registerc(state, "getEntityActivity", lua_GetEntityActivity);
    lua_registerc(state, "setEntityActivity", lua_SetEntityActivity);
    lua_registerc(state, "getEntityEnability", lua_GetEntityEnability);
    lua_registerc(state, "getEntityOCB", lua_GetEntityOCB);
    lua_registerc(state, "setEntityOCB", lua_SetEntityOCB);
    lua_registerc(state, "getEntityTimer", lua_GetEntityTimer);
    lua_registerc(state, "setEntityTimer", lua_SetEntityTimer);
    lua_registerc(state, "getEntityFlags", lua_GetEntityFlags);
    lua_registerc(state, "setEntityFlags", lua_SetEntityFlags);
    lua_registerc(state, "getEntityTypeFlag", lua_GetEntityTypeFlag);
    lua_registerc(state, "setEntityTypeFlag", lua_SetEntityTypeFlag);
    lua_registerc(state, "getEntityStateFlag", lua_GetEntityStateFlag);
    lua_registerc(state, "setEntityStateFlag", lua_SetEntityStateFlag);
    lua_registerc(state, "getEntityCallbackFlag", lua_GetEntityCallbackFlag);
    lua_registerc(state, "setEntityCallbackFlag", lua_SetEntityCallbackFlag);
    lua_registerc(state, "getEntityState", lua_GetEntityState);
    lua_registerc(state, "setEntityState", lua_SetEntityState);
    lua_registerc(state, "setEntityRoomMove", lua_SetEntityRoomMove);
    lua_registerc(state, "getEntityMoveType", lua_GetEntityMoveType);
    lua_registerc(state, "setEntityMoveType", lua_SetEntityMoveType);
    lua_registerc(state, "getEntityResponse", lua_GetEntityResponse);
    lua_registerc(state, "setEntityResponse", lua_SetEntityResponse);
    lua_registerc(state, "getEntityMeshCount", lua_GetEntityMeshCount);
    lua_registerc(state, "setEntityMeshswap", lua_SetEntityMeshswap);
    lua_registerc(state, "setModelMeshReplaceFlag", lua_SetModelMeshReplaceFlag);
    lua_registerc(state, "setModelAnimReplaceFlag", lua_SetModelAnimReplaceFlag);
    lua_registerc(state, "copyMeshFromModelToModel", lua_CopyMeshFromModelToModel);

    lua_registerc(state, "setEntityBodyMass", lua_SetEntityBodyMass);
    lua_registerc(state, "pushEntityBody", lua_PushEntityBody);
    lua_registerc(state, "lockEntityBodyLinearFactor", lua_LockEntityBodyLinearFactor);

    lua_registerc(state, "getEntityTriggerLayout", lua_GetEntityTriggerLayout);
    lua_registerc(state, "setEntityTriggerLayout", lua_SetEntityTriggerLayout);
    lua_registerc(state, "getEntityMask", lua_GetEntityMask);
    lua_registerc(state, "setEntityMask", lua_SetEntityMask);
    lua_registerc(state, "getEntityEvent", lua_GetEntityEvent);
    lua_registerc(state, "setEntityEvent", lua_SetEntityEvent);
    lua_registerc(state, "getEntityLock", lua_GetEntityLock);
    lua_registerc(state, "setEntityLock", lua_SetEntityLock);
    lua_registerc(state, "getEntitySectorStatus", lua_GetEntitySectorStatus);
    lua_registerc(state, "setEntitySectorStatus", lua_SetEntitySectorStatus);

    lua_registerc(state, "getEntityActivationOffset", lua_GetEntityActivationOffset);
    lua_registerc(state, "setEntityActivationOffset", lua_SetEntityActivationOffset);
    lua_registerc(state, "getEntitySectorIndex", lua_GetEntitySectorIndex);
    lua_registerc(state, "getEntitySectorFlags", lua_GetEntitySectorFlags);
    lua_registerc(state, "getEntitySectorMaterial", lua_GetEntitySectorMaterial);

    lua_registerc(state, "addEntityRagdoll", lua_AddEntityRagdoll);
    lua_registerc(state, "removeEntityRagdoll", lua_RemoveEntityRagdoll);

    lua_registerc(state, "getCharacterParam", lua_GetCharacterParam);
    lua_registerc(state, "setCharacterParam", lua_SetCharacterParam);
    lua_registerc(state, "changeCharacterParam", lua_ChangeCharacterParam);
    lua_registerc(state, "getCharacterCurrentWeapon", lua_GetCharacterCurrentWeapon);
    lua_registerc(state, "setCharacterCurrentWeapon", lua_SetCharacterCurrentWeapon);
    lua_registerc(state, "setCharacterWeaponModel", lua_SetCharacterWeaponModel);
    lua_registerc(state, "getCharacterCombatMode", lua_GetCharacterCombatMode);

    lua_registerc(state, "addCharacterHair", lua_AddCharacterHair);
    lua_registerc(state, "resetCharacterHair", lua_ResetCharacterHair);

    lua_registerc(state, "getSecretStatus", lua_GetSecretStatus);
    lua_registerc(state, "setSecretStatus", lua_SetSecretStatus);

    lua_registerc(state, "getActionState", lua_GetActionState);
    lua_registerc(state, "getActionChange", lua_GetActionChange);

    lua_registerc(state, "genUVRotateAnimation", lua_genUVRotateAnimation);

    lua_registerc(state, "getGravity", lua_GetGravity);
    lua_registerc(state, "setGravity", lua_SetGravity);
    lua_registerc(state, "dropEntity", lua_DropEntity);
    lua_registerc(state, "bind", lua_BindKey);

    lua_registerc(state, "addFont", lua_AddFont);
    lua_registerc(state, "deleteFont", lua_DeleteFont);
    lua_registerc(state, "addFontStyle", lua_AddFontStyle);
    lua_registerc(state, "deleteFontStyle", lua_DeleteFontStyle);
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


bool Engine_FileFound(const std::string& name, bool Write)
{
    FILE *ff;

    if(Write)
    {
        ff = fopen(name.c_str(), "ab");
    }
    else
    {
        ff = fopen(name.c_str(), "rb");
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

int Engine_GetLevelFormat(const std::string& /*name*/)
{
    // PLACEHOLDER: Currently, only PC levels are supported.

    return LEVEL_FORMAT_PC;
}


int Engine_GetPCLevelVersion(const std::string& name)
{
    int ret = TR_UNKNOWN;
    FILE *ff;

    if(name.length() < 5)
    {
        return ret;                                                             // Wrong (too short) filename
    }

    ff = fopen(name.c_str(), "rb");
    if(ff)
    {
        char ext[5];
        uint8_t check[4];

        ext[0] = name[name.length()-4];                                                   // .
        ext[1] = toupper(name[name.length()-3]);                                          // P
        ext[2] = toupper(name[name.length()-2]);                                          // H
        ext[3] = toupper(name[name.length()-1]);                                          // D
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


std::string Engine_GetLevelName(const std::string& path)
{
    if(path.empty())
    {
        return {};
    }


    size_t ext = path.find_last_of(".");
    assert(ext != std::string::npos);

    size_t start = path.find_last_of("\\/");
    if(start == std::string::npos)
        start = 0;
    else
        ++start;

    return path.substr(start, ext-start);
}

std::string Engine_GetLevelScriptName(int game_version, const std::string& postfix)
{
    std::string level_name = Engine_GetLevelName(gameflow_manager.CurrentLevelPath);

    std::string name = "scripts/level/";

    if(game_version < TR_II)
    {
        name += "tr1/";
    }
    else if(game_version < TR_III)
    {
        name += "tr2/";
    }
    else if(game_version < TR_IV)
    {
        name += "tr3/";
    }
    else if(game_version < TR_V)
    {
        name += "tr4/";
    }
    else
    {
        name += "tr5/";
    }

    for(char& c : level_name)
    {
        c = std::toupper(c);
    }

    name += level_name;
    name += postfix;
    name += ".lua";
    return name;
}

bool Engine_LoadPCLevel(const std::string& name)
{
    VT_Level *tr_level = new VT_Level();

    int trv = Engine_GetPCLevelVersion(name);
    if(trv == TR_UNKNOWN) return false;

    tr_level->read_level(name, trv);
    tr_level->prepare_level();
    //tr_level->dump_textures();

    TR_GenWorld(&engine_world, tr_level);

    std::string buf = Engine_GetLevelName(name);

    ConsoleInfo::instance().notify(SYSNOTE_LOADED_PC_LEVEL);
    ConsoleInfo::instance().notify(SYSNOTE_ENGINE_VERSION, trv, buf.c_str());
    ConsoleInfo::instance().notify(SYSNOTE_NUM_ROOMS, engine_world.rooms.size());

    delete tr_level;

    return true;
}

int Engine_LoadMap(const std::string& name)
{
    if(!Engine_FileFound(name))
    {
        ConsoleInfo::instance().warning(SYSWARN_FILE_NOT_FOUND, name.c_str());
        return 0;
    }

    Gui_DrawLoadScreen(0);

    renderer.hideSkyBox();
    renderer.resetRListActiveCount();
    renderer.resetWorld();

    gameflow_manager.CurrentLevelPath = name;          // it is needed for "not in the game" levels or correct saves loading.

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
            control_states.free_look = true;
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
                            ConsoleInfo::instance().printf("cont[entity](%d, %d, %d).object_id = %d", (int)e->m_transform.getOrigin()[0], (int)e->m_transform.getOrigin()[1], (int)e->m_transform.getOrigin()[2], e->id());
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
            ConsoleInfo::instance().addLine(pch, FONTSTYLE_CONSOLE_EVENT);
            try {
                engine_lua.doString(pch);
            }
            catch(lua::RuntimeError& error) {
                ConsoleInfo::instance().addLine(error.what(), FONTSTYLE_CONSOLE_WARNING);
            }
            return 0;
        }
    }

    return 0;
}


void Engine_InitConfig(const char *filename)
{

    Engine_InitDefaultGlobals();

    if((filename != NULL) && Engine_FileFound(filename))
    {
        lua::State state;
        lua_registerc(state, "bind", lua_BindKey);                             // get and set key bindings
        try {
            state.doFile(filename);
        }
        catch(lua::RuntimeError& error) {
            Sys_DebugLog("lua_out.txt", "%s", error.what());
            return;
        }
        catch(lua::LoadError& error) {
            Sys_DebugLog("lua_out.txt", "%s", error.what());
            return;
        }

        lua_ParseScreen(state, &screen_info);
        lua_ParseRender(state, &renderer.settings());
        lua_ParseAudio(state, &audio_settings);
        lua_ParseConsole(state, &ConsoleInfo::instance());
        lua_ParseControls(state, &control_mapper);
    }
    else
    {
        Sys_Warn("Could not find \"%s\"", filename);
    }
}
