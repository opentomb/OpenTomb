#include "script.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include <GL/glew.h>

#include "LuaState.h"

#include "engine/anim_state_control.h"
#include "audio/audio.h"
#include "character_controller.h"
#include "gui/console.h"
#include "engine/controls.h"
#include "engine/engine.h"
#include "world/entity.h"
#include "engine/game.h"
#include "engine/gameflow.h"
#include "world/hair.h"
#include "util/helpers.h"
#include "world/ragdoll.h"
#include "render/render.h"
#include "strings.h"
#include "engine/system.h"
#include "util/vmath.h"
#include "world/world.h"

// Debug functions

using gui::Console;

script::MainEngine engine_lua;

void script::ScriptEngine::checkStack()
{
    Console::instance().notify(SYSNOTE_LUA_STACK_INDEX, lua_gettop(m_state.getState()));
}

void lua_DumpModel(int id)
{
    world::core::SkeletalModel* sm = engine::engine_world.getModelByID(id);
    if(sm == nullptr)
    {
        Console::instance().warning(SYSWARN_WRONG_MODEL_ID, id);
        return;
    }

    for(int i = 0; i < sm->mesh_count; i++)
    {
        Console::instance().printf("mesh[%d] = %d", i, sm->mesh_tree[i].mesh_base->m_id);
    }
}

void lua_DumpRoom(lua::Value id)
{
    if(id.is<lua::Nil>())
    {
        engine::dumpRoom(engine::engine_camera.m_currentRoom);
        return;
    }
    if(id.is<lua::Integer>() && static_cast<uint32_t>(id) >= engine::engine_world.rooms.size())
    {
        Console::instance().warning(SYSWARN_WRONG_ROOM, static_cast<int>(id));
        return;
    }
    engine::dumpRoom(engine::engine_world.rooms[id].get());
}

void lua_SetRoomEnabled(int id, bool value)
{
    if(id < 0 || id >= static_cast<int>(engine::engine_world.rooms.size()))
    {
        Console::instance().warning(SYSWARN_WRONG_ROOM, id);
    }

    if(!value)
    {
        engine::engine_world.rooms[id]->disable();
    }
    else
    {
        engine::engine_world.rooms[id]->enable();
    }
}

// Base engine functions

void lua_SetModelCollisionMapSize(int id, int size)
{
    world::core::SkeletalModel* model = engine::engine_world.getModelByID(id);
    if(model == nullptr)
    {
        Console::instance().warning(SYSWARN_MODELID_OVERFLOW, id);
        return;
    }

    if(size >= 0 && size < model->mesh_count)
    {
        model->collision_map.resize(size);
    }
}

void lua_SetModelCollisionMap(int id, int arg, int val)
{
    /// engine_world.skeletal_models[id] != engine::engine_world.getModelByID(lua_tointeger(lua, 1));
    world::core::SkeletalModel* model = engine::engine_world.getModelByID(id);
    if(model == nullptr)
    {
        Console::instance().warning(SYSWARN_MODELID_OVERFLOW, id);
		return;
    }

    if((arg >= 0) && (static_cast<size_t>(arg) < model->collision_map.size()) &&
       (val >= 0) && (val < model->mesh_count))
    {
        model->collision_map[arg] = val;
    }
}

void lua_EnableEntity(int id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);
    if(ent)
        ent->enable();
}

void lua_DisableEntity(int id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);
    if(ent)
        ent->disable();
}

void lua_SetEntityCollision(int id, bool val)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);
    if(ent)
    {
        if(val)
            ent->enableCollision();
        else
            ent->disableCollision();
    }
}

void lua_SetEntityCollisionFlags(int id, lua::Value ctype, lua::Value cshape)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);
    if(ent != nullptr)
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
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if((ent != nullptr) && (ent->m_currentSector))
    {
        return ent->m_currentSector->flags;
    }
    return 0;
}

uint32_t lua_GetEntitySectorIndex(int id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if((ent != nullptr) && (ent->m_currentSector))
    {
        return ent->m_currentSector->trig_index;
    }
    return 0;
}

uint32_t lua_GetEntitySectorMaterial(int id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if((ent != nullptr) && (ent->m_currentSector))
    {
        return ent->m_currentSector->material;
    }
    return 0;
}

uint32_t lua_GetEntitySubstanceState(int id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent != nullptr)
    {
        return static_cast<uint32_t>(ent->getSubstanceState());
    }
    return 0;
}

bool lua_SameRoom(int id1, int id2)
{
    std::shared_ptr<world::Entity> ent1 = engine::engine_world.getEntityByID(id1);
    std::shared_ptr<world::Entity> ent2 = engine::engine_world.getEntityByID(id2);

    if(ent1 && ent2)
    {
        return ent1->m_self->room == ent2->m_self->room;
    }

    return false;
}

bool lua_SameSector(int id1, int id2)
{
    std::shared_ptr<world::Entity> ent1 = engine::engine_world.getEntityByID(id1);
    std::shared_ptr<world::Entity> ent2 = engine::engine_world.getEntityByID(id2);

    if(ent1 && ent2 && ent1->m_currentSector && ent2->m_currentSector)
    {
        return ent1->m_currentSector->trig_index == ent2->m_currentSector->trig_index;
    }

    return false;
}

bool lua_NewSector(int id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent != nullptr)
    {
        return ent->m_currentSector == ent->m_lastSector;
    }
    return false;
}

std::tuple<float, float, float> lua_GetGravity()
{
    btVector3 g = engine::bt_engine_dynamicsWorld->getGravity();
    return std::tuple<float, float, float>{
        g[0],
            g[1],
            g[2]
    };
}

void lua_SetGravity(float x, lua::Value y, lua::Value z)                                             // function to be exported to Lua
{
    btVector3 g(x, y.is<lua::Number>() ? y : 0, z.is<lua::Number>() ? z : 0);
    engine::bt_engine_dynamicsWorld->setGravity(g);
    Console::instance().printf("gravity = (%.3f, %.3f, %.3f)", g[0], g[1], g[2]);
}

bool lua_DropEntity(int id, float time, lua::Value only_room)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);
    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return false;
    }

    btVector3 move = ent->applyGravity(time);

    engine::BtEngineClosestRayResultCallback cb(ent->m_self);
    btVector3 from, to;
    from = ent->m_transform * ent->m_bf.centre;
    from[2] = ent->m_transform.getOrigin()[2];
    to = from + move;
    //to[2] -= (ent->m_bf.bb_max[2] - ent->m_bf.bb_min[2]);
    engine::bt_engine_dynamicsWorld->rayTest(from, to, cb);

    if(cb.hasHit())
    {
        engine::EngineContainer* cont = static_cast<engine::EngineContainer*>(cb.m_collisionObject->getUserPointer());

        if(!only_room.is<lua::Boolean>() || !only_room.to<bool>() || (only_room.to<bool>() && (cont->object_type == OBJECT_ROOM_BASE)))
        {
            move.setInterpolate3(from, to, cb.m_closestHitFraction);
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
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);
    if(ent == nullptr)
        return -1;

    if(ent->m_bf.animations.model)
    {
        return ent->m_bf.animations.model->id;
    }
    return -1;
}

std::tuple<float, float, float, float> lua_GetEntityActivationOffset(int id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);
    if(ent == nullptr)
        return{};

    return std::tuple<float, float, float, float>(
        ent->m_activationOffset[0],
        ent->m_activationOffset[1],
        ent->m_activationOffset[2],
        ent->m_activationRadius);
}

void lua_SetEntityActivationOffset(int id, float x, float y, float z, lua::Value r)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);
    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return;
    }

    ent->m_activationOffset = { x,y,z };
    if(r.is<lua::Number>())
        ent->m_activationRadius = r;
}

int lua_GetCharacterParam(int id, int parameter)
{
    std::shared_ptr<Character> ent = engine::engine_world.getCharacterByID(id);

    if(parameter >= PARAM_SENTINEL)
    {
        Console::instance().warning(SYSWARN_WRONG_OPTION_INDEX, PARAM_SENTINEL);
        return -1;
    }

    if(ent)
    {
        return ent->getParam(parameter);
    }
    else
    {
        Console::instance().warning(SYSWARN_NO_CHARACTER, id);
        return -1;
    }
}

void lua_SetCharacterParam(int id, int parameter, float value, lua::Value max_value)
{
    std::shared_ptr<Character> ent = engine::engine_world.getCharacterByID(id);

    if(parameter >= PARAM_SENTINEL)
    {
        Console::instance().warning(SYSWARN_WRONG_OPTION_INDEX, PARAM_SENTINEL);
        return;
    }

    if(!ent)
    {
        Console::instance().warning(SYSWARN_NO_CHARACTER, id);
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
    std::shared_ptr<Character> ent = engine::engine_world.getCharacterByID(id);

    if(ent)
    {
        return static_cast<int>(ent->m_weaponCurrentState);
    }

    return -1;
}

void lua_ChangeCharacterParam(int id, int parameter, lua::Value value)
{
    std::shared_ptr<Character> ent = engine::engine_world.getCharacterByID(id);

    if(parameter >= PARAM_SENTINEL)
    {
        Console::instance().warning(SYSWARN_WRONG_OPTION_INDEX, PARAM_SENTINEL);
        return;
    }

    if(ent && (value.is<lua::Number>() || value.is<lua::Integer>()))
    {
        if(value.is<lua::Number>())
            ent->changeParam(parameter, value.to<lua::Number>());
    }
    else
    {
        Console::instance().warning(SYSWARN_NO_CHARACTER, id);
    }
}

void lua_AddCharacterHair(int ent_id, int setup_index)
{
    std::shared_ptr<Character> ent = engine::engine_world.getCharacterByID(ent_id);

    if(ent)
    {
        world::HairSetup hair_setup;

        hair_setup.getSetup(setup_index);
        ent->m_hairs.emplace_back(std::make_shared<world::Hair>());

        if(!ent->m_hairs.back()->create(&hair_setup, ent))
        {
            Console::instance().warning(SYSWARN_CANT_CREATE_HAIR, ent_id);
            ent->m_hairs.pop_back();
        }
    }
    else
    {
        Console::instance().warning(SYSWARN_NO_CHARACTER, ent_id);
    }
}

void lua_ResetCharacterHair(int ent_id)
{
    std::shared_ptr<Character> ent = engine::engine_world.getCharacterByID(ent_id);

    if(ent)
    {
        if(!ent->m_hairs.empty())
        {
            ent->m_hairs.clear();
        }
        else
        {
            Console::instance().warning(SYSWARN_CANT_RESET_HAIR, ent_id);
        }
    }
    else
    {
        Console::instance().warning(SYSWARN_NO_CHARACTER, ent_id);
    }
}

void lua_AddEntityRagdoll(int ent_id, int setup_index)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(ent_id);

    if(ent)
    {
        world::RDSetup ragdoll_setup;

        if(!ragdoll_setup.getSetup(setup_index))
        {
            Console::instance().warning(SYSWARN_NO_RAGDOLL_SETUP, setup_index);
        }
        else
        {
            if(!ent->createRagdoll(&ragdoll_setup))
            {
                Console::instance().warning(SYSWARN_CANT_CREATE_RAGDOLL, ent_id);
            }
        }
    }
    else
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, ent_id);
    }
}

void lua_RemoveEntityRagdoll(int ent_id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(ent_id);

    if(ent)
    {
        if(!ent->m_bt.bt_joints.empty())
        {
            ent->deleteRagdoll();
        }
        else
        {
            Console::instance().warning(SYSWARN_CANT_REMOVE_RAGDOLL, ent_id);
        }
    }
    else
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, ent_id);
    }
}

bool lua_GetSecretStatus(int secret_number)
{
    return engine::Gameflow_Manager.getSecretStatus(secret_number);
}

void lua_SetSecretStatus(int secret_number, bool status)
{
    engine::Gameflow_Manager.setSecretStatus(secret_number, status);
}

bool lua_GetActionState(int act)
{
    if(act < 0 || act >= engine::ACT_LASTINDEX)
    {
        Console::instance().warning(SYSWARN_WRONG_ACTION_NUMBER);
        return false;
    }
    else
    {
        return engine::control_mapper.action_map[act].state;
    }
}

bool lua_GetActionChange(int act)
{
    if(act < 0 || act >= engine::ACT_LASTINDEX)
    {
        Console::instance().warning(SYSWARN_WRONG_ACTION_NUMBER);
        return false;
    }
    else
    {
        return engine::control_mapper.action_map[act].already_pressed;
    }
}

int lua_GetEngineVersion()
{
    return static_cast<int>(engine::engine_world.engineVersion);
}

void script::MainEngine::bindKey(int act, int primary, lua::Value secondary)
{
    if(act < 0 || act >= engine::ACT_LASTINDEX)
    {
        Console::instance().warning(SYSWARN_WRONG_ACTION_NUMBER);
    }
    engine::control_mapper.action_map[act].primary = primary;
    if(secondary.is<lua::Integer>())
        engine::control_mapper.action_map[act].secondary = secondary;
}

void lua_AddFont(int index, const char* path, uint32_t size)
{
    if(!gui::fontManager->AddFont(static_cast<gui::FontType>(index), size, path))
    {
        Console::instance().warning(SYSWARN_CANT_CREATE_FONT, gui::fontManager->GetFontCount(), gui::MaxFonts);
    }
}

void lua_AddFontStyle(int style_index,
                      float color_R, float color_G, float color_B, float color_A,
                      bool shadowed, bool fading, bool rect, float rect_border,
                      float rect_R, float rect_G, float rect_B, float rect_A,
                      bool hide)
{
    if(!gui::fontManager->AddFontStyle(static_cast<gui::FontStyle>(style_index),
                                       color_R, color_G, color_B, color_A,
                                       shadowed, fading,
                                       rect, rect_border, rect_R, rect_G, rect_B, rect_A,
                                       hide))
    {
        Console::instance().warning(SYSWARN_CANT_CREATE_STYLE, gui::fontManager->GetFontStyleCount(), gui::FontStyle::Sentinel);
    }
}

void lua_DeleteFont(int fontindex)
{
    if(!gui::fontManager->RemoveFont(static_cast<gui::FontType>(fontindex)))
    {
        Console::instance().warning(SYSWARN_CANT_REMOVE_FONT);
    }
}

void lua_DeleteFontStyle(int styleindex)
{
    if(!gui::fontManager->RemoveFontStyle(static_cast<gui::FontStyle>(styleindex)))
    {
        Console::instance().warning(SYSWARN_CANT_REMOVE_STYLE);
    }
}

int lua_AddItem(int entity_id, int item_id, lua::Value count)
{
    std::shared_ptr<Character> ent = engine::engine_world.getCharacterByID(entity_id);

    if(ent)
    {
        return ent->addItem(item_id, count.is<lua::Integer>() ? count : -1);
    }
    else
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, entity_id);
        return -1;
    }
}

int lua_RemoveItem(int entity_id, int item_id, int count)
{
    std::shared_ptr<Character> ent = engine::engine_world.getCharacterByID(entity_id);

    if(ent)
    {
        return ent->removeItem(item_id, count);
    }
    else
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, entity_id);
        return -1;
    }
}

void lua_RemoveAllItems(int entity_id)
{
    std::shared_ptr<Character> ent = engine::engine_world.getCharacterByID(entity_id);

    if(ent)
    {
        ent->removeAllItems();
    }
    else
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, entity_id);
    }
}

int lua_GetItemsCount(int entity_id, int item_id)
{
    std::shared_ptr<Character> ent = engine::engine_world.getCharacterByID(entity_id);

    if(ent)
    {
        return ent->getItemsCount(item_id);
    }
    else
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, entity_id);
        return -1;
    }
}

void lua_CreateBaseItem(int item_id, int model_id, int world_model_id, int type, int count, const char* name)
{
    engine::engine_world.createItem(item_id, model_id, world_model_id, static_cast<gui::MenuItemType>(type), count, name ? name : std::string());
}

void lua_DeleteBaseItem(int id)
{
    engine::engine_world.deleteItem(id);
}

void lua_PrintItems(int entity_id)
{
    std::shared_ptr<Character> ent = engine::engine_world.getCharacterByID(entity_id);
    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, entity_id);
        return;
    }

    for(const InventoryNode& i : ent->m_inventory)
    {
        Console::instance().printf("item[id = %d]: count = %d", i.id, i.count);
    }
}

void lua_SetStateChangeRange(int id, int anim, int state, int dispatch, int frame_low, int frame_high, lua::Value next_anim, lua::Value next_frame)
{
    world::core::SkeletalModel* model = engine::engine_world.getModelByID(id);

    if(model == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_SKELETAL_MODEL, id);
        return;
    }

    if((anim < 0) || (anim + 1 > static_cast<int>(model->animations.size())))
    {
        Console::instance().warning(SYSWARN_WRONG_ANIM_NUMBER, anim);
        return;
    }

    world::animation::AnimationFrame* af = &model->animations[anim];
    for(uint16_t i = 0; i < af->stateChanges.size(); i++)
    {
        if(af->stateChanges[i].id == static_cast<uint32_t>(state))
        {
            if(dispatch >= 0 && dispatch < static_cast<int>(af->stateChanges[i].anim_dispatch.size()))
            {
                af->stateChanges[i].anim_dispatch[dispatch].frame_low = frame_low;
                af->stateChanges[i].anim_dispatch[dispatch].frame_high = frame_high;
                if(!next_anim.is<lua::Nil>() && !next_frame.is<lua::Nil>())
                {
                    af->stateChanges[i].anim_dispatch[dispatch].next_anim = next_anim;
                    af->stateChanges[i].anim_dispatch[dispatch].next_frame = next_frame;
                }
            }
            else
            {
                Console::instance().warning(SYSWARN_WRONG_DISPATCH_NUMBER, dispatch);
            }
            break;
        }
    }
}

std::tuple<int, float, float, float> lua_GetAnimCommandTransform(int id, int anim, int frame)
{
    world::core::SkeletalModel* model = engine::engine_world.getModelByID(id);
    if(model == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_SKELETAL_MODEL, id);
        return{};
    }

    if(anim < 0 || anim + 1 > static_cast<int>(model->animations.size()))
    {
        Console::instance().warning(SYSWARN_WRONG_ANIM_NUMBER, anim);
        return{};
    }

    if(frame < 0)                                                               // it is convenient to use -1 as a last frame number
    {
        frame = static_cast<int>(model->animations[anim].frames.size()) + frame;
    }

    if(frame < 0 || frame + 1 > static_cast<int>(model->animations[anim].frames.size()))
    {
        Console::instance().warning(SYSWARN_WRONG_FRAME_NUMBER, frame);
        return{};
    }

    return std::tuple<int, float, float, float>
    {
        model->animations[anim].frames[frame].command,
            model->animations[anim].frames[frame].move[0],
            model->animations[anim].frames[frame].move[1],
            model->animations[anim].frames[frame].move[2]
    };
}

void lua_SetAnimCommandTransform(int id, int anim, int frame, int flag, lua::Value dx, lua::Value dy, lua::Value dz)
{
    world::core::SkeletalModel* model = engine::engine_world.getModelByID(id);
    if(model == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_SKELETAL_MODEL, id);
        return;
    }

    if(anim < 0 || anim + 1 > static_cast<int>(model->animations.size()))
    {
        Console::instance().warning(SYSWARN_WRONG_ANIM_NUMBER, anim);
        return;
    }

    if(frame < 0)                                                               // it is convenient to use -1 as a last frame number
    {
        frame = static_cast<int>(model->animations[anim].frames.size()) + frame;
    }

    if(frame < 0 || frame + 1 > static_cast<int>(model->animations[anim].frames.size()))
    {
        Console::instance().warning(SYSWARN_WRONG_FRAME_NUMBER, frame);
        return;
    }

    model->animations[anim].frames[frame].command = flag;

    if(dx.is<lua::Number>() && dy.is<lua::Number>() && dz.is<lua::Number>())
        model->animations[anim].frames[frame].move = { dx,dy,dz };
}

void lua_SetAnimVerticalSpeed(int id, int anim, int frame, float speed)
{
    world::core::SkeletalModel* model = engine::engine_world.getModelByID(id);
    if(model == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_SKELETAL_MODEL, id);
        return;
    }

    if(anim < 0 || anim + 1 > static_cast<int>(model->animations.size()))
    {
        Console::instance().warning(SYSWARN_WRONG_ANIM_NUMBER, anim);
        return;
    }

    if(frame < 0)                                                               // it is convenient to use -1 as a last frame number
    {
        frame = static_cast<int>(model->animations[anim].frames.size()) + frame;
    }

    if(frame < 0 || frame + 1 > static_cast<int>(model->animations[anim].frames.size()))
    {
        Console::instance().warning(SYSWARN_WRONG_FRAME_NUMBER, frame);
        return;
    }

    model->animations[anim].frames[frame].v_Vertical = static_cast<btScalar>(speed);
}

uint32_t lua_SpawnEntity(int model_id, float x, float y, float z, float ax, float ay, float az, int room_id, lua::Value ov_id)
{
    btVector3 pos{ x,y,z }, ang{ ax,ay,az };

    uint32_t id = engine::engine_world.spawnEntity(model_id, room_id, &pos, &ang, ov_id.is<lua::Integer>() ? ov_id : -1);
    if(id == 0xFFFFFFFF)
    {
        return -1;
    }
    else
    {
        return id;
    }
}

bool lua_DeleteEntity(int id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent != nullptr)
    {
        if(ent->m_self->room) ent->m_self->room->removeEntity(ent.get());
        engine::engine_world.deleteEntity(id);
        return true;
    }
    else
    {
        return false;
    }
}

// Moveable script control section

std::tuple<float, float, float> lua_GetEntityVector(int id1, int id2)
{
    std::shared_ptr<world::Entity> e1 = engine::engine_world.getEntityByID(id1);
    if(e1 == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id1);
        return{};
    }
    std::shared_ptr<world::Entity> e2 = engine::engine_world.getEntityByID(id2);
    if(e2 == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id2);
        return{};
    }

    return std::tuple<float, float, float>
    {
        e2->m_transform.getOrigin()[0] - e1->m_transform.getOrigin()[0],
            e2->m_transform.getOrigin()[1] - e1->m_transform.getOrigin()[1],
            e2->m_transform.getOrigin()[2] - e1->m_transform.getOrigin()[2]
    };
}

float lua_GetEntityDistance(int id1, int id2)
{
    std::shared_ptr<world::Entity> e1 = engine::engine_world.getEntityByID(id1);
    if(!e1)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id1);
        return std::numeric_limits<float>::max();
    }
    std::shared_ptr<world::Entity> e2 = engine::engine_world.getEntityByID(id2);
    if(!e2)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id2);
        return std::numeric_limits<float>::max();
    }

    return e1->findDistance(*e2);
}

float lua_GetEntityDirDot(int id1, int id2)
{
    std::shared_ptr<world::Entity> e1 = engine::engine_world.getEntityByID(id1);
    if(!e1)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id2);
        return std::numeric_limits<float>::max();
    }
    std::shared_ptr<world::Entity> e2 = engine::engine_world.getEntityByID(id2);
    if(!e2)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id2);
        return std::numeric_limits<float>::max();
    }

    return e1->m_transform.getBasis().getColumn(1).dot(e2->m_transform.getBasis().getColumn(1));
}

bool lua_IsInRoom(int id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if((ent) && (ent->m_self->room))
    {
        if(ent->m_currentSector)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

std::tuple<float, float, float, float, float, float, uint32_t> lua_GetEntityPosition(int id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);
    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return{};
    }

    return std::tuple<float, float, float, float, float, float, uint32_t>
    {
        ent->m_transform.getOrigin()[0],
            ent->m_transform.getOrigin()[1],
            ent->m_transform.getOrigin()[2],
            ent->m_angles[0],
            ent->m_angles[1],
            ent->m_angles[2],
            ent->m_self->room->id
    };
}

std::tuple<float, float, float> lua_GetEntityAngles(int id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return{};
    }

    return std::tuple<float, float, float>
    {
        ent->m_angles[0],
            ent->m_angles[1],
            ent->m_angles[2]
    };
}

std::tuple<float, float, float> lua_GetEntityScaling(int id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(!ent)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return{};
    }

    return std::tuple<float, float, float>
    {
        ent->m_scaling[0],
            ent->m_scaling[1],
            ent->m_scaling[2]
    };
}

bool lua_SimilarSector(int id, float dx, float dy, float dz, bool ignore_doors, lua::Value ceiling)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(!ent)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return{};
    }

    auto next_pos = ent->m_transform.getOrigin() + (dx * ent->m_transform.getBasis().getColumn(0) + dy * ent->m_transform.getBasis().getColumn(1) + dz * ent->m_transform.getBasis().getColumn(2));

    world::RoomSector* curr_sector = ent->m_self->room->getSectorRaw(ent->m_transform.getOrigin());
    world::RoomSector* next_sector = ent->m_self->room->getSectorRaw(next_pos);

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
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(!ent)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return 0;
    }

    auto pos = ent->m_transform.getOrigin();

    if(dx.is<lua::Number>() && dy.is<lua::Number>() && dz.is<lua::Number>())
        pos += dx.to<btScalar>() * ent->m_transform.getBasis().getColumn(0) + dy.to<btScalar>() * ent->m_transform.getBasis().getColumn(1) + dz.to<btScalar>() * ent->m_transform.getBasis().getColumn(2);

    world::RoomSector* curr_sector = ent->m_self->room->getSectorRaw(pos);
    curr_sector = curr_sector->checkPortalPointer();
    btVector3 point = (ceiling.is<lua::Boolean>() && ceiling.to<bool>())
        ? curr_sector->getCeilingPoint()
        : curr_sector->getFloorPoint();

    return point[2];
}

void lua_SetEntityPosition(int id, float x, float y, float z, lua::Value ax, lua::Value ay, lua::Value az)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);
    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return;
    }
    ent->m_transform.getOrigin() = { x,y,z };
    if(ax.is<lua::Number>() && ay.is<lua::Number>() && az.is<lua::Number>())
        ent->m_angles = { ax,ay,az };
    ent->updateTransform();
    ent->updatePlatformPreStep();
}

void lua_SetEntityAngles(int id, float x, lua::Value y, lua::Value z)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
    }
    else
    {
        if(!y.is<lua::Number>() || !z.is<lua::Number>())
            ent->m_angles[0] = x;
        else
            ent->m_angles = { x,y,z };
        ent->updateTransform();
    }
}

void lua_SetEntityScaling(int id, float x, float y, float z)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(!ent)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
    }
    else
    {
        ent->m_scaling = { x,y,z };

        if(!ent->m_bf.bone_tags.empty() && !ent->m_bt.bt_body.empty())
        {
            for(size_t i = 0; i < ent->m_bf.bone_tags.size(); i++)
            {
                if(ent->m_bt.bt_body[i])
                {
                    engine::bt_engine_dynamicsWorld->removeRigidBody(ent->m_bt.bt_body[i].get());
                    ent->m_bt.bt_body[i]->getCollisionShape()->setLocalScaling(ent->m_scaling);
                    engine::bt_engine_dynamicsWorld->addRigidBody(ent->m_bt.bt_body[i].get());

                    ent->m_bt.bt_body[i]->activate();
                }
            }
        }

        ent->updateRigidBody(true);
    }
}

void lua_MoveEntityGlobal(int id, float x, float y, float z)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);
    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return;
    }
    ent->m_transform.getOrigin()[0] += x;
    ent->m_transform.getOrigin()[1] += y;
    ent->m_transform.getOrigin()[2] += z;

    ent->updateRigidBody(true);
    ent->ghostUpdate();
}

void lua_MoveEntityLocal(int id, float dx, float dy, float dz)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(!ent)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return;
    }

    ent->m_transform.getOrigin() += ent->m_transform.getBasis() * btVector3(dx, dy, dz);

    ent->updateRigidBody(true);
    ent->ghostUpdate();
}

void lua_MoveEntityToSink(int id, int sink_index)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(sink_index < 0 || sink_index > static_cast<int>(engine::engine_world.cameras_sinks.size()))
        return;
    world::StatCameraSink* sink = &engine::engine_world.cameras_sinks[sink_index];

    btVector3 ent_pos;  ent_pos[0] = ent->m_transform.getOrigin()[0];
    ent_pos[1] = ent->m_transform.getOrigin()[1];
    ent_pos[2] = ent->m_transform.getOrigin()[2];

    btVector3 sink_pos; sink_pos[0] = sink->x;
    sink_pos[1] = sink->y;
    sink_pos[2] = sink->z + 256.0f;

    assert(ent->m_currentSector != nullptr);
    world::RoomSector* ls = ent->m_currentSector->getLowestSector();
    assert(ls != nullptr);
    world::RoomSector* hs = ent->m_currentSector->getHighestSector();
    assert(hs != nullptr);
    if((sink_pos[2] > hs->ceiling) ||
       (sink_pos[2] < ls->floor))
    {
        sink_pos[2] = ent_pos[2];
    }

    btScalar dist = btDistance(ent_pos, sink_pos);
    if(dist == 0.0) dist = 1.0; // Prevents division by zero.

    btVector3 speed = ((sink_pos - ent_pos) / dist) * (static_cast<btScalar>(sink->room_or_strength) * 1.5);

    ent->m_transform.getOrigin()[0] += speed[0];
    ent->m_transform.getOrigin()[1] += speed[1];
    ent->m_transform.getOrigin()[2] += speed[2] * 16.0f;

    ent->updateRigidBody(true);
    ent->ghostUpdate();
}

void lua_MoveEntityToEntity(int id1, int id2, float speed_mult, lua::Value ignore_z)
{
    std::shared_ptr<world::Entity> ent1 = engine::engine_world.getEntityByID(id1);
    std::shared_ptr<world::Entity> ent2 = engine::engine_world.getEntityByID(id2);

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
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
    }
    else
    {
        if(!ry.is<lua::Number>() || !rz.is<lua::Number>())
            ent->m_angles += {rx, 0, 0};
        else
            ent->m_angles += {rx, ry, rz};

        ent->updateTransform();
        ent->updateRigidBody(true);
    }
}

void lua_RotateEntityToEntity(int id1, int id2, int axis, lua::Value speed_, lua::Value smooth_, lua::Value add_angle_)
{
    std::shared_ptr<world::Entity> ent1 = engine::engine_world.getEntityByID(id1);
    std::shared_ptr<world::Entity> ent2 = engine::engine_world.getEntityByID(id2);

    if((!ent1) || (!ent2))
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, ((!ent1)?id1:id2));
    }
    else if((axis < 0) || (axis > 2))
    {
        Console::instance().warning(SYSWARN_WRONG_AXIS, ((!ent1)?id1:id2));
    }
    else
    {
        btVector3 ent1_pos = ent1->m_transform.getOrigin();
        btVector3 ent2_pos = ent2->m_transform.getOrigin();
        btVector3 facing   = (ent1_pos - ent2_pos);

        btScalar *targ_angle;
        btScalar  theta;

        switch(axis)
        {
            case 0:
                targ_angle = ent1->m_angles + 0;
                theta      = btAtan2(-facing.x(), facing.y());
                break;
            case 1:
                targ_angle = ent1->m_angles + 1;
                theta      = btAtan2(facing.z(), facing.y());
                break;
            case 2:
                targ_angle = ent1->m_angles + 2;
                theta      = btAtan2(facing.x(), facing.z());
                break;
        }

        theta = btDegrees(theta);
        if(add_angle_.is<lua::Number>()) theta += static_cast<btScalar>(add_angle_);

        btScalar delta = *targ_angle - theta;

        if(ceil(delta) != 180.0)
        {
            if(speed_.is<lua::Number>())
            {
                btScalar speed = static_cast<btScalar>(speed_);

                if(fabs(delta) > speed)
                {
                    // Solve ~0-360 rotation cases.

                    if(abs(delta) > 180.0)
                    {
                        if(*targ_angle > theta)
                        {
                            delta = -((360.0 - *targ_angle) + theta);
                        }
                        else
                        {
                            delta = (360.0 - theta) + *targ_angle;
                        }
                    }

                    if(delta > 180.0)
                    {
                        *targ_angle = theta + 180.0;
                    }
                    else if((delta >= 0.0) && (delta < 180.0))
                    {
                        *targ_angle += speed;
                    }
                    else
                    {
                        *targ_angle -= speed;
                    }
                }

                if(fabs(delta) + speed >= 180.0)
                    *targ_angle = floor(theta) + 180.0;
            }
            else
            {
                *targ_angle = theta + 180.0;
            }
        }

        ent1->updateTransform();
        ent1->updateRigidBody(true);
    }
}



float lua_GetEntityOrientation(int id1, int id2, lua::Value add_angle_)
{
    std::shared_ptr<world::Entity> ent1 = engine::engine_world.getEntityByID(id1);
    std::shared_ptr<world::Entity> ent2 = engine::engine_world.getEntityByID(id2);

    if((!ent1) || (!ent2))
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, ((!ent1)?id1:id2));
        return 0;
    }
    else
    {
        btVector3 ent1_pos = ent1->m_transform.getOrigin();
        btVector3 ent2_pos = ent2->m_transform.getOrigin();
        btVector3 facing   = (ent2_pos - ent1_pos);

        btScalar theta = btDegrees(btAtan2(-facing.x(), facing.y()));
        if(add_angle_.is<lua::Number>()) theta += static_cast<btScalar>(add_angle_);

        return util::wrapAngle(ent2->m_angles[0] - theta);
    }
}


std::tuple<float, float, float> lua_GetEntitySpeed(int id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return{};
    }

    return std::tuple<float, float, float>
    {
        ent->m_speed[0],
            ent->m_speed[1],
            ent->m_speed[2]
    };
}

float lua_GetEntitySpeedLinear(int id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return 0;
    }

    return ent->m_speed.length();
}

void lua_SetEntitySpeed(int id, float vx, lua::Value vy, lua::Value vz)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
    }
    else
    {
        if(!vy.is<lua::Number>() || !vz.is<lua::Number>())
            ent->m_speed[0] = vx;
        else
            ent->m_speed = { vx,vy,vz };
        ent->updateCurrentSpeed();
    }
}

void lua_SetEntityAnim(int id, int anim, lua::Value frame, lua::Value otherModel)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
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
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return;
    }

    ent->m_bf.animations.anim_flags = anim_flag;
}

void lua_SetEntityBodyPartFlag(int id, int bone_id, int body_part_flag)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return;
    }

    if(bone_id < 0 || bone_id >= static_cast<int>(ent->m_bf.bone_tags.size()))
    {
        Console::instance().warning(SYSWARN_WRONG_OPTION_INDEX, bone_id);
        return;
    }

    ent->m_bf.bone_tags[bone_id].body_part = body_part_flag;
}

void lua_SetModelBodyPartFlag(int id, int bone_id, int body_part_flag)
{
    world::core::SkeletalModel* model = engine::engine_world.getModelByID(id);

    if(model == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_SKELETAL_MODEL, id);
        return;
    }

    if((bone_id < 0) || (bone_id >= model->mesh_count))
    {
        Console::instance().warning(SYSWARN_WRONG_OPTION_INDEX, bone_id);
        return;
    }

    model->mesh_tree[bone_id].body_part = body_part_flag;
}

std::tuple<int16_t, int16_t, uint32_t> lua_GetEntityAnim(int id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return{};
    }

    return std::tuple<int16_t, int16_t, uint32_t>
    {
        ent->m_bf.animations.current_animation,
            ent->m_bf.animations.current_frame,
            static_cast<uint32_t>(ent->m_bf.animations.model->animations[ent->m_bf.animations.current_animation].frames.size())
    };
}

bool lua_CanTriggerEntity(int id1, int id2, lua::Value rv, lua::Value ofsX, lua::Value ofsY, lua::Value ofsZ)
{
    std::shared_ptr<Character> e1 = engine::engine_world.getCharacterByID(id1);
    if(!e1 || !e1->m_command.action)
    {
        return false;
    }

    std::shared_ptr<world::Entity> e2 = engine::engine_world.getEntityByID(id2);
    if(!e2 || e1 == e2)
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
        offset = { ofsX,ofsY,ofsZ };
    else
        offset = e2->m_activationOffset;

    auto pos = e2->m_transform * offset;
    if((e1->m_transform.getBasis().getColumn(1).dot(e2->m_transform.getBasis().getColumn(1)) > 0.75) &&
       ((e1->m_transform.getOrigin() - pos).length2() < r))
    {
        return true;
    }

    return false;
}

bool lua_GetEntityVisibility(int id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return false;
    }

    return ent->m_visible;
}

void lua_SetEntityVisibility(int id, bool value)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return;
    }

    ent->m_visible = value;
}

bool lua_GetEntityEnability(int id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return false;
    }

    return ent->m_enabled;
}

bool lua_GetEntityActivity(int id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return false;
    }

    return ent->m_active;
}

void lua_SetEntityActivity(int id, bool value)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return;
    }

    ent->m_active = value;
}

std::tuple<int, bool, bool> lua_GetEntityTriggerLayout(int id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);
    if(ent == nullptr)
        return{};   // No entity found - return.

    return std::tuple<int, bool, bool>
    {
        ent->m_triggerLayout & ENTITY_TLAYOUT_MASK,   // mask
            (ent->m_triggerLayout & ENTITY_TLAYOUT_EVENT), // event
            (ent->m_triggerLayout & ENTITY_TLAYOUT_LOCK)   // lock
    };
}

void lua_SetEntityTriggerLayout(int id, int mask, lua::Value eventOrLayout, lua::Value lock)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return;
    }

    if(lock.is<lua::Boolean>())
    {
        uint8_t trigger_layout = ent->m_triggerLayout;
        trigger_layout &= ~static_cast<uint8_t>((ENTITY_TLAYOUT_MASK));  trigger_layout ^= mask;          // mask  - 00011111
        trigger_layout &= ~static_cast<uint8_t>((ENTITY_TLAYOUT_EVENT)); trigger_layout ^= eventOrLayout.to<bool>() << 5;   // event - 00100000
        trigger_layout &= ~static_cast<uint8_t>((ENTITY_TLAYOUT_LOCK));  trigger_layout ^= lock.to<bool>() << 6;   // lock  - 01000000
        ent->m_triggerLayout = trigger_layout;
    }
    else
    {
        ent->m_triggerLayout = eventOrLayout.to<int>();
    }
}

void lua_SetEntityLock(int id, bool lock)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);
    if(ent != nullptr)
    {
        uint8_t trigger_layout = ent->m_triggerLayout;
        trigger_layout &= ~static_cast<uint8_t>((ENTITY_TLAYOUT_LOCK));
        trigger_layout ^= lock << 6;   // lock  - 01000000
        ent->m_triggerLayout = trigger_layout;
    }
}

bool lua_GetEntityLock(int id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);
    if(ent != nullptr)
    {
        return (ent->m_triggerLayout & ENTITY_TLAYOUT_LOCK) >> 6;      // lock
    }
    return false;
}

void lua_SetEntityEvent(int id, bool event)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);
    if(ent != nullptr)
    {
        uint8_t trigger_layout = ent->m_triggerLayout;
        trigger_layout &= ~static_cast<uint8_t>((ENTITY_TLAYOUT_EVENT)); trigger_layout ^= event << 5;   // event - 00100000
        ent->m_triggerLayout = trigger_layout;
    }
}

bool lua_GetEntityEvent(int id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);
    if(ent != nullptr)
    {
        return (ent->m_triggerLayout & ENTITY_TLAYOUT_EVENT) >> 5;    // event
    }
    return false;
}

int lua_GetEntityMask(int id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);
    if(ent != nullptr)
    {
        return ent->m_triggerLayout & ENTITY_TLAYOUT_MASK;          // mask
    }
    return -1;
}

void lua_SetEntityMask(int id, int mask)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);
    if(ent != nullptr)
    {
        uint8_t trigger_layout = ent->m_triggerLayout;
        trigger_layout &= ~static_cast<uint8_t>((ENTITY_TLAYOUT_MASK));
        trigger_layout ^= mask;   // mask  - 00011111
        ent->m_triggerLayout = trigger_layout;
    }
}

bool lua_GetEntitySectorStatus(int id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);
    if(ent != nullptr)
    {
        return (ent->m_triggerLayout & ENTITY_TLAYOUT_SSTATUS) >> 7;
    }
    return true;
}

void lua_SetEntitySectorStatus(int id, bool status)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);
    if(ent != nullptr)
    {
        uint8_t trigger_layout = ent->m_triggerLayout;
        trigger_layout &= ~static_cast<uint8_t>((ENTITY_TLAYOUT_SSTATUS));
        trigger_layout ^= status << 7;   // sector_status  - 10000000
        ent->m_triggerLayout = trigger_layout;
    }
}

int lua_GetEntityOCB(int id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);
    if(ent == nullptr)
        return -1;   // No entity found - return.

    return ent->m_OCB;
}

void lua_SetEntityOCB(int id, int ocb)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);
    if(ent == nullptr)
        return;   // No entity found - return.

    ent->m_OCB = ocb;
}

std::tuple<bool, bool, bool, uint16_t, uint32_t> lua_GetEntityFlags(int id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return{};
    }

    return std::tuple<bool, bool, bool, uint16_t, uint32_t>
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
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(!ent)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
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
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return -1;
    }

    if(flag.is<lua::Integer>())
        return ent->m_typeFlags & static_cast<uint16_t>(flag);
    else
        return ent->m_typeFlags;
}

void lua_SetEntityTypeFlag(int id, uint16_t type_flag, lua::Value value)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return;
    }
    if(!value.is<lua::Boolean>())
    {
        ent->m_typeFlags ^= type_flag;
        return;
    }

    if(static_cast<bool>(value))
    {
        ent->m_typeFlags |= type_flag;
    }
    else
    {
        ent->m_typeFlags &= ~type_flag;
    }
}

bool lua_GetEntityStateFlag(int id, const char* whichCstr)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
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
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return;
    }

    bool* flag;
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
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return 0;
    }

    if(!flag.is<lua::Integer>())
        return ent->m_callbackFlags;
    else
        return ent->m_callbackFlags & static_cast<uint16_t>(flag);
}

void lua_SetEntityCallbackFlag(int id, uint32_t flag, lua::Value value)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return;
    }

    if(!value.is<lua::Boolean>())
    {
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
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);
    if(ent == nullptr)
        return std::numeric_limits<float>::max();   // No entity found - return.

    return ent->m_timer;
}

void lua_SetEntityTimer(int id, float timer)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);
    if(ent == nullptr)
        return;   // No entity found - return.

    ent->m_timer = timer;
}

uint16_t lua_GetEntityMoveType(int id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return -1;
    }

    return static_cast<uint16_t>(ent->m_moveType);
}

void lua_SetEntityMoveType(int id, uint16_t type)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent == nullptr)
        return;
    ent->m_moveType = static_cast<world::MoveType>(type);
}

int lua_GetEntityResponse(int id, int response)
{
    std::shared_ptr<Character> ent = engine::engine_world.getCharacterByID(id);

    if(ent)
    {
        switch(response)
        {
            case 0: return (ent->m_response.killed ? 1 : 0);
            case 1: return ent->m_response.vertical_collide;
            case 2: return ent->m_response.horizontal_collide;
            case 3: return static_cast<int>(ent->m_response.slide);
            case 4: return static_cast<int>(ent->m_response.lean);
            default: return 0;
        }
    }
    else
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return 0;
    }
}

void lua_SetEntityResponse(int id, int response, int value)
{
    std::shared_ptr<Character> ent = engine::engine_world.getCharacterByID(id);

    if(ent)
    {
        switch(response)
        {
            case 0:
                ent->m_response.killed = (value!=0);
                break;
            case 1:
                ent->m_response.vertical_collide = value;
                break;
            case 2:
                ent->m_response.horizontal_collide = value;
                break;
            case 3:
                ent->m_response.slide = static_cast<SlideType>(value);
                break;
            case 4:
                ent->m_response.lean = static_cast<LeanType>(value);
                break;
            default:
                break;
        }
    }
    else
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
    }
}

int16_t lua_GetEntityState(int id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return -1;
    }

    return ent->m_bf.animations.last_state;
}

uint32_t lua_GetEntityModel(int id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return -1;
    }

    return ent->m_bf.animations.model->id;
}

void lua_SetEntityState(int id, int16_t value, lua::Value next)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return;
    }

    ent->m_bf.animations.next_state = value;
    if(next.is<lua::Integer>())
        ent->m_bf.animations.last_state = next;
}

void lua_SetEntityRoomMove(int id, uint32_t room, uint16_t moveType, int dirFlag)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);
    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return;
    }

    if(room < engine::engine_world.rooms.size())
    {
        std::shared_ptr<world::Room> r = engine::engine_world.rooms[room];
        if(ent == engine::engine_world.character)
        {
            ent->m_self->room = r.get();
        }
        else if(ent->m_self->room != r.get())
        {
            if(ent->m_self->room != nullptr)
            {
                ent->m_self->room->removeEntity(ent.get());
            }
            r->addEntity(ent.get());
        }
    }
    ent->updateRoomPos();

    ent->m_moveType = static_cast<world::MoveType>(moveType);
    ent->m_dirFlag = dirFlag;
}

uint32_t lua_GetEntityMeshCount(int id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return 0;
    }

    return ent->m_bf.bone_tags.size();
}

void lua_SetEntityMeshswap(int id_dest, int id_src)
{
    std::shared_ptr<world::Entity> ent_dest;
    world::core::SkeletalModel* model_src;

    ent_dest = engine::engine_world.getEntityByID(id_dest);
    model_src = engine::engine_world.getModelByID(id_src);

    int meshes_to_copy = (ent_dest->m_bf.bone_tags.size() > model_src->mesh_count) ? (model_src->mesh_count) : (ent_dest->m_bf.bone_tags.size());

    for(int i = 0; i < meshes_to_copy; i++)
    {
        ent_dest->m_bf.bone_tags[i].mesh_base = model_src->mesh_tree[i].mesh_base;
        ent_dest->m_bf.bone_tags[i].mesh_skin = model_src->mesh_tree[i].mesh_skin;
    }
}

void lua_SetModelMeshReplaceFlag(int id, int bone, int flag)
{
    world::core::SkeletalModel* sm = engine::engine_world.getModelByID(id);
    if(sm != nullptr)
    {
        if((bone >= 0) && (bone < sm->mesh_count))
        {
            sm->mesh_tree[bone].replace_mesh = flag;
        }
        else
        {
            Console::instance().warning(SYSWARN_WRONG_BONE_NUMBER, bone);
        }
    }
    else
    {
        Console::instance().warning(SYSWARN_WRONG_MODEL_ID, id);
    }
}

void lua_SetModelAnimReplaceFlag(int id, int bone, int flag)
{
    world::core::SkeletalModel* sm = engine::engine_world.getModelByID(id);
    if(sm != nullptr)
    {
        if((bone >= 0) && (bone < sm->mesh_count))
        {
            sm->mesh_tree[bone].replace_anim = flag;
        }
        else
        {
            Console::instance().warning(SYSWARN_WRONG_BONE_NUMBER, bone);
        }
    }
    else
    {
        Console::instance().warning(SYSWARN_WRONG_MODEL_ID, id);
    }
}

void lua_CopyMeshFromModelToModel(int id1, int id2, int bone1, int bone2)
{
    world::core::SkeletalModel* sm1 = engine::engine_world.getModelByID(id1);
    if(sm1 == nullptr)
    {
        Console::instance().warning(SYSWARN_WRONG_MODEL_ID, id1);
        return;
    }

    world::core::SkeletalModel* sm2 = engine::engine_world.getModelByID(id2);
    if(sm2 == nullptr)
    {
        Console::instance().warning(SYSWARN_WRONG_MODEL_ID, id2);
        return;
    }

    if((bone1 >= 0) && (bone1 < sm1->mesh_count) && (bone2 >= 0) && (bone2 < sm2->mesh_count))
    {
        sm1->mesh_tree[bone1].mesh_base = sm2->mesh_tree[bone2].mesh_base;
    }
    else
    {
        Console::instance().warning(SYSWARN_WRONG_BONE_NUMBER, bone1);
    }
}

void lua_CreateEntityGhosts(int id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent && (ent->m_bf.bone_tags.size() > 0))
    {
        ent->createGhosts();
    }
}

void lua_PushEntityBody(int id, uint32_t body_number, float h_force, float v_force, bool resetFlag)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent && (body_number < ent->m_bf.bone_tags.size()) && (ent->m_bt.bt_body[body_number] != nullptr) && (ent->m_typeFlags & ENTITY_TYPE_DYNAMIC))
    {
        btScalar t = ent->m_angles[0] * util::RadPerDeg;

        btScalar ang1 = std::sin(t);
        btScalar ang2 = std::cos(t);

        btVector3 angle(-ang1 * h_force, ang2 * h_force, v_force);

        if(resetFlag)
            ent->m_bt.bt_body[body_number]->clearForces();

        ent->m_bt.bt_body[body_number]->setLinearVelocity(angle);
        ent->m_bt.bt_body[body_number]->setAngularVelocity(angle / 1024.0);
    }
    else
    {
        Console::instance().warning(SYSWARN_CANT_APPLY_FORCE, id);
    }
}

int lua_SetEntityBodyMass(lua_State *lua)
{
    int top = lua_gettop(lua);

    if(lua_gettop(lua) < 3)
    {
        Console::instance().warning(SYSWARN_WRONG_ARGS, "[entity_id, body_number, (mass / each body mass)]");
        return 0;
    }

    const auto id = lua_tounsigned(lua, 1);
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    int body_number = lua_tounsigned(lua, 2);
    body_number = (body_number < 1) ? (1) : (body_number);

    uint16_t argn = 3;
    bool dynamic = false;

    if(ent && (static_cast<int>(ent->m_bf.bone_tags.size()) >= body_number))
    {
        for(int i = 0; i < body_number; i++)
        {
            btVector3 inertia(0.0, 0.0, 0.0);

            btScalar mass = 0;
            if(top >= argn) mass = lua_tonumber(lua, argn);
            argn++;

            if(ent->m_bt.bt_body[i])
            {
                engine::bt_engine_dynamicsWorld->removeRigidBody(ent->m_bt.bt_body[i].get());

                ent->m_bt.bt_body[i]->getCollisionShape()->calculateLocalInertia(mass, inertia);

                ent->m_bt.bt_body[i]->setMassProps(mass, inertia);

                ent->m_bt.bt_body[i]->updateInertiaTensor();
                ent->m_bt.bt_body[i]->clearForces();

                ent->m_bt.bt_body[i]->getCollisionShape()->setLocalScaling(ent->m_scaling);

                btVector3 factor = (mass > 0.0) ? (btVector3(1.0, 1.0, 1.0)) : (btVector3(0.0, 0.0, 0.0));
                ent->m_bt.bt_body[i]->setLinearFactor(factor);
                ent->m_bt.bt_body[i]->setAngularFactor(factor);

                //ent->bt_body[i]->forceActivationState(DISABLE_DEACTIVATION);

                //ent->bt_body[i]->setCcdMotionThreshold(32.0);   // disable tunneling effect
                //ent->bt_body[i]->setCcdSweptSphereRadius(32.0);

                engine::bt_engine_dynamicsWorld->addRigidBody(ent->m_bt.bt_body[i].get());

                ent->m_bt.bt_body[i]->activate();

                //ent->bt_body[i]->getBroadphaseHandle()->m_collisionFilterGroup = 0xFFFF;
                //ent->bt_body[i]->getBroadphaseHandle()->m_collisionFilterMask  = 0xFFFF;

                //ent->self->object_type = OBJECT_ENTITY;
                //ent->bt_body[i]->setUserPointer(ent->self);

                if(mass > 0.0) dynamic = true;
            }
        }

        if(dynamic)
        {
            ent->m_typeFlags |= ENTITY_TYPE_DYNAMIC;
        }
        else
        {
            ent->m_typeFlags &= ~ENTITY_TYPE_DYNAMIC;
        }

        ent->updateRigidBody(true);
    }
    else
    {
        Console::instance().warning(SYSWARN_WRONG_ENTITY_OR_BODY, id, body_number);
    }

    return 0;
}

void lua_LockEntityBodyLinearFactor(int id, uint32_t body_number, lua::Value vfactor)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent && (body_number < ent->m_bf.bone_tags.size()) && (ent->m_bt.bt_body[body_number] != nullptr) && (ent->m_typeFlags & ENTITY_TYPE_DYNAMIC))
    {
        btScalar t = ent->m_angles[0] * util::RadPerDeg;
        btScalar ang1 = std::sin(t);
        btScalar ang2 = std::cos(t);
        btScalar ang3 = 1.0;

        if(vfactor.is<lua::Number>())
        {
            ang3 = std::abs(vfactor.to<float>());
            ang3 = (ang3 > 1.0) ? (1.0) : (ang3);
        }

        ent->m_bt.bt_body[body_number]->setLinearFactor(btVector3(std::abs(ang1), std::abs(ang2), ang3));
    }
    else
    {
        Console::instance().warning(SYSWARN_CANT_APPLY_FORCE, id);
    }
}

void lua_SetCharacterWeaponModel(int id, int weaponmodel, int state)
{
    std::shared_ptr<Character> ent = engine::engine_world.getCharacterByID(id);

    if(ent)
    {
        ent->setWeaponModel(weaponmodel, state);
    }
    else
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
    }
}

int lua_GetCharacterCurrentWeapon(int id)
{
    std::shared_ptr<Character> ent = engine::engine_world.getCharacterByID(id);

    if(ent)
    {
        return ent->m_currentWeapon;
    }
    else
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return -1;
    }
}

void lua_SetCharacterCurrentWeapon(int id, int weapon)
{
    std::shared_ptr<Character> ent = engine::engine_world.getCharacterByID(id);

    if(ent)
    {
        ent->m_currentWeapon = weapon;
    }
    else
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
    }
}

// Camera functions

void lua_CamShake(float power, float time, lua::Value id)
{
    if(!id.is<lua::Nil>() && id >= 0)
    {
        std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

        btVector3 cam_pos = render::renderer.camera()->getPosition();

        btScalar dist = ent->m_transform.getOrigin().distance(cam_pos);
        dist = (dist > world::MaxShakeDistance) ? (0) : (1.0 - (dist / world::MaxShakeDistance));

        power *= dist;
    }

    if(power > 0.0)
        render::renderer.camera()->shake(power, time);
}

void lua_FlashSetup(int alpha, int R, int G, int B, uint16_t fadeinSpeed, uint16_t fadeoutSpeed)
{
    gui::fadeSetup(gui::FaderType::Effect,
                  alpha,
                  R, G, B,
                  loader::BlendingMode::Multiply,
                  fadeinSpeed, fadeoutSpeed);
}

void lua_FlashStart()
{
    gui::fadeStart(gui::FaderType::Effect, gui::FaderDir::Timed);
}

void lua_FadeOut()
{
    gui::fadeStart(gui::FaderType::Black, gui::FaderDir::Out);
}

void lua_FadeIn()
{
    gui::fadeStart(gui::FaderType::Black, gui::FaderDir::In);
}

bool lua_FadeCheck()
{
    return gui::getFaderStatus(gui::FaderType::Black) != gui::FaderStatus::Idle;
}

// General gameplay functions

void lua_PlayStream(int id, lua::Value mask)
{
    if(id < 0)
    {
        Console::instance().warning(SYSWARN_WRONG_STREAM_ID);
        return;
    }

    if(!mask.is<lua::Nil>())
    {
        audio::streamPlay(id, mask.to<int>());
    }
    else
    {
        audio::streamPlay(id);
    }
}

void lua_StopStreams()
{
    audio::stopStreams();
}

void lua_PlaySound(int id, lua::Value ent_id)
{
    if(id < 0) return;

    if(static_cast<size_t>(id) >= engine::engine_world.audio_map.size())
    {
        Console::instance().warning(SYSWARN_WRONG_SOUND_ID, engine::engine_world.audio_map.size());
        return;
    }

    int eid = -1;
    if(!ent_id.is<lua::Nil>())
        eid = static_cast<int>(ent_id);
    if(eid < 0 || !engine::engine_world.getEntityByID(eid))
        eid = -1;

    audio::Error result;

    if(eid >= 0)
    {
        result = audio::send(id, audio::EmitterType::Entity, eid);
    }
    else
    {
        result = audio::send(id, audio::EmitterType::Global);
    }

    switch(result)
    {
        case audio::Error::NoChannel:
            Console::instance().warning(SYSWARN_AS_NOCHANNEL);
            break;

        case audio::Error::NoSample:
            Console::instance().warning(SYSWARN_AS_NOSAMPLE);
            break;

        default:
            break;
    }
}

void lua_StopSound(uint32_t id, lua::Value ent_id)
{
    if(id >= engine::engine_world.audio_map.size())
    {
        Console::instance().warning(SYSWARN_WRONG_SOUND_ID, engine::engine_world.audio_map.size());
        return;
    }

    int eid = -1;
    if(!ent_id.is<lua::Nil>())
        eid = static_cast<int>(ent_id);
    if(eid < 0 || engine::engine_world.getEntityByID(eid) == nullptr)
        eid = -1;

    audio::Error result;

    if(eid == -1)
    {
        result = audio::kill(id, audio::EmitterType::Global);
    }
    else
    {
        result = audio::kill(id, audio::EmitterType::Entity, eid);
    }

    if(result == audio::Error::NoSample || result == audio::Error::NoChannel)
        Console::instance().warning(SYSWARN_AK_NOTPLAYED, id);
}

int lua_GetLevel()
{
    return engine::Gameflow_Manager.getLevelID();
}

void lua_SetLevel(int id)
{
    Console::instance().notify(SYSNOTE_CHANGING_LEVEL, id);

    engine::Game_LevelTransition(id);
    engine::Gameflow_Manager.send(engine::Opcode::LevelComplete, id);    // Next level
}

void lua_SetGame(int gameId, lua::Value levelId)
{
    engine::Gameflow_Manager.setGameID(gameId);
    if(!levelId.is<lua::Nil>() && levelId >= 0)
        engine::Gameflow_Manager.setLevelID(levelId);

    const char* str = engine_lua["getTitleScreen"](int(engine::Gameflow_Manager.getGameID()));
    gui::fadeAssignPic(gui::FaderType::LoadScreen, str);
    gui::fadeStart(gui::FaderType::LoadScreen, gui::FaderDir::Out);

    Console::instance().notify(SYSNOTE_CHANGING_GAME, engine::Gameflow_Manager.getGameID());
    engine::Game_LevelTransition(engine::Gameflow_Manager.getLevelID());
    engine::Gameflow_Manager.send(engine::Opcode::LevelComplete, engine::Gameflow_Manager.getLevelID());
}

void lua_LoadMap(const char* mapName, lua::Value gameId, lua::Value mapId)
{
    Console::instance().notify(SYSNOTE_LOADING_MAP, mapName);

    if(mapName && mapName != engine::Gameflow_Manager.getLevelPath())
    {
        if(gameId.is<lua::Integer>() && gameId >= 0)
        {
            engine::Gameflow_Manager.setGameID(static_cast<int>(gameId));
        }
        if(mapId.is<lua::Integer>() && mapId >= 0)
        {
            engine::Gameflow_Manager.setLevelID(mapId);
        }
        char file_path[MAX_ENGINE_PATH];
        engine_lua.getLoadingScreen(engine::Gameflow_Manager.getLevelID(), file_path);
        gui::fadeAssignPic(gui::FaderType::LoadScreen, file_path);
        gui::fadeStart(gui::FaderType::LoadScreen, gui::FaderDir::In);
        engine::loadMap(mapName);
    }
}

// Flipped (alternate) room functions

void lua_SetFlipState(uint32_t group, bool state)
{
    if(group >= engine::engine_world.flip_data.size())
    {
        Console::instance().warning(SYSWARN_WRONG_FLIPMAP_INDEX);
        return;
    }

    if(engine::engine_world.flip_data[group].map == 0x1F)         // Check flipmap state.
    {
        std::vector< std::shared_ptr<world::Room> >::iterator current_room = engine::engine_world.rooms.begin();

        if(engine::engine_world.engineVersion > loader::Engine::TR3)
        {
            for(; current_room != engine::engine_world.rooms.end(); ++current_room)
            {
                if((*current_room)->alternate_group == group)    // Check if group is valid.
                {
                    if(state)
                        (*current_room)->swapToAlternate();
                    else
                        (*current_room)->swapToBase();
                }
            }

            engine::engine_world.flip_data[group].state = state;
        }
        else
        {
            for(; current_room != engine::engine_world.rooms.end(); ++current_room)
            {
                if(state)
                    (*current_room)->swapToAlternate();
                else
                    (*current_room)->swapToBase();
            }

            engine::engine_world.flip_data[0].state = state;    // In TR1-3, state is always global.
        }
    }
}

void lua_SetFlipMap(uint32_t group, int mask, int /*op*/)
{
    int op = (mask > AMASK_OP_XOR) ? (AMASK_OP_XOR) : (AMASK_OP_OR);

    if(group >= engine::engine_world.flip_data.size())
    {
        Console::instance().warning(SYSWARN_WRONG_FLIPMAP_INDEX);
        return;
    }

    if(op == AMASK_OP_XOR)
    {
        engine::engine_world.flip_data[group].map ^= mask;
    }
    else
    {
        engine::engine_world.flip_data[group].map |= mask;
    }
}

int lua_GetFlipMap(uint32_t group)
{
    if(group >= engine::engine_world.flip_data.size())
    {
        Console::instance().warning(SYSWARN_WRONG_FLIPMAP_INDEX);
        return 0;
    }

    return engine::engine_world.flip_data[group].map;
}

int lua_GetFlipState(uint32_t group)
{
    if(group >= engine::engine_world.flip_data.size())
    {
        Console::instance().warning(SYSWARN_WRONG_FLIPMAP_INDEX);
        return 0;
    }

    return engine::engine_world.flip_data[group].state;
}

/*
 * Generate UV rotate animations
 */

void lua_genUVRotateAnimation(int id)
{
    world::core::SkeletalModel* model = engine::engine_world.getModelByID(id);

    if(!model)
        return;

    if(model->mesh_tree.front().mesh_base->m_transparencyPolygons.empty())
        return;
    const world::core::Polygon& firstPolygon = model->mesh_tree.front().mesh_base->m_transparencyPolygons.front();
    if(firstPolygon.anim_id != 0)
        return;

    engine::engine_world.anim_sequences.emplace_back();
    world::animation::AnimSeq* seq = &engine::engine_world.anim_sequences.back();

    // Fill up new sequence with frame list.

    seq->anim_type = world::animation::AnimTextureType::Forward;
    seq->frame_lock = false;              // by default anim is playing
    seq->uvrotate = true;
    seq->frames.resize(16);
    seq->frame_list.resize(16);
    seq->reverse_direction = false;       // Needed for proper reverse-type start-up.
    seq->frame_rate        = 0.025f;      // Should be passed as 1 / FPS.
    seq->frame_time        = 0.0;         // Reset frame time to initial state.
    seq->current_frame     = 0;           // Reset current frame to zero.
    seq->frame_list[0] = 0;

    btScalar v_min, v_max;
    v_min = v_max = firstPolygon.vertices[0].tex_coord[1];

    for(size_t j = 1; j<firstPolygon.vertices.size(); j++)
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
    seq->uvrotate_speed = seq->uvrotate_max / static_cast<btScalar>(seq->frames.size());

    for(uint16_t j = 0; j < seq->frames.size(); j++)
    {
        seq->frames[j].tex_ind = firstPolygon.tex_index;
        seq->frames[j].mat[0] = 1.0;
        seq->frames[j].mat[1] = 0.0;
        seq->frames[j].mat[2] = 0.0;
        seq->frames[j].mat[3] = 1.0;
        seq->frames[j].move[0] = 0.0;
        seq->frames[j].move[1] = -(static_cast<btScalar>(j) * seq->uvrotate_speed);
    }

    for(world::core::Polygon& p : model->mesh_tree.front().mesh_base->m_transparencyPolygons)
    {
        p.anim_id = engine::engine_world.anim_sequences.size();
        for(world::core::Vertex& v : p.vertices)
        {
            v.tex_coord[1] = v_min + 0.5 * (v.tex_coord[1] - v_min) + seq->uvrotate_max;
        }
    }

    return;
}

namespace script
{
void ScriptEngine::exposeConstants()
{
    m_state.set("MOVE_STATIC_POS", static_cast<int>(world::MoveType::StaticPos));
    m_state.set("MOVE_KINEMATIC", static_cast<int>(world::MoveType::Kinematic));
    m_state.set("MOVE_ON_FLOOR", static_cast<int>(world::MoveType::OnFloor));
    m_state.set("MOVE_WADE", static_cast<int>(world::MoveType::Wade));
    m_state.set("MOVE_QUICKSAND", static_cast<int>(world::MoveType::Quicksand));
    m_state.set("MOVE_ON_WATER", static_cast<int>(world::MoveType::OnWater));
    m_state.set("MOVE_UNDERWATER", static_cast<int>(world::MoveType::Underwater));
    m_state.set("MOVE_FREE_FALLING", static_cast<int>(world::MoveType::FreeFalling));
    m_state.set("MOVE_CLIMBING", static_cast<int>(world::MoveType::Climbing));
    m_state.set("MOVE_MONKEYSWING", static_cast<int>(world::MoveType::Monkeyswing));
    m_state.set("MOVE_WALLS_CLIMB", static_cast<int>(world::MoveType::WallsClimb));
    m_state.set("MOVE_DOZY", static_cast<int>(world::MoveType::Dozy));

    // exposes a constant
#define EXPOSE_C(name) m_state.set(#name, name)
    // exposes a casted constant
#define EXPOSE_CC(name) m_state.set(#name, static_cast<int>(name))

    m_state.set("Game", lua::Table());
    m_state["Game"].set("I", static_cast<int>(loader::Game::TR1));
    m_state["Game"].set("I_DEMO", static_cast<int>(loader::Game::TR1Demo));
    m_state["Game"].set("I_UB", static_cast<int>(loader::Game::TR1UnfinishedBusiness));
    m_state["Game"].set("II", static_cast<int>(loader::Game::TR2));
    m_state["Game"].set("II_DEMO", static_cast<int>(loader::Game::TR2Demo));
    m_state["Game"].set("II_GOLD", static_cast<int>(loader::Game::TR2Gold));
    m_state["Game"].set("III", static_cast<int>(loader::Game::TR3));
    m_state["Game"].set("III_GOLD", static_cast<int>(loader::Game::TR3Gold));
    m_state["Game"].set("IV", static_cast<int>(loader::Game::TR4));
    m_state["Game"].set("IV_DEMO", static_cast<int>(loader::Game::TR4Demo));
    m_state["Game"].set("V", static_cast<int>(loader::Game::TR5));
    m_state["Game"].set("Unknown", static_cast<int>(loader::Game::Unknown));

    m_state.set("Engine", lua::Table());
    m_state["Engine"].set("I", static_cast<int>(loader::Engine::TR1));
    m_state["Engine"].set("II", static_cast<int>(loader::Engine::TR2));
    m_state["Engine"].set("III", static_cast<int>(loader::Engine::TR3));
    m_state["Engine"].set("IV", static_cast<int>(loader::Engine::TR4));
    m_state["Engine"].set("V", static_cast<int>(loader::Engine::TR5));
    m_state["Engine"].set("Unknown", static_cast<int>(loader::Engine::Unknown));

    m_state.set("StreamMethod", lua::Table());
    m_state["StreamMethod"].set("Track", static_cast<int>(audio::StreamMethod::Track));
    m_state["StreamMethod"].set("WAD", static_cast<int>(audio::StreamMethod::WAD));

    m_state.set("StreamType", lua::Table());
    m_state["StreamType"].set("Background", static_cast<int>(audio::StreamType::Background));
    m_state["StreamType"].set("Chat", static_cast<int>(audio::StreamType::Chat));
    m_state["StreamType"].set("Oneshot", static_cast<int>(audio::StreamType::Oneshot));

#if 0
    // Unused, but kept here for reference
    m_state.set("ENTITY_STATE_ENABLED", 0x0001);
    m_state.set("ENTITY_STATE_ACTIVE",  0x0002);
    m_state.set("ENTITY_STATE_VISIBLE", 0x0004);
#endif

    EXPOSE_C(ENTITY_TYPE_GENERIC);
    EXPOSE_C(ENTITY_TYPE_INTERACTIVE);
    EXPOSE_C(ENTITY_TYPE_TRIGGER_ACTIVATOR);
    EXPOSE_C(ENTITY_TYPE_HEAVYTRIGGER_ACTIVATOR);
    EXPOSE_C(ENTITY_TYPE_PICKABLE);
    EXPOSE_C(ENTITY_TYPE_TRAVERSE);
    EXPOSE_C(ENTITY_TYPE_TRAVERSE_FLOOR);
    EXPOSE_C(ENTITY_TYPE_DYNAMIC);
    EXPOSE_C(ENTITY_TYPE_ACTOR);
    EXPOSE_C(ENTITY_TYPE_COLLCHECK);

    EXPOSE_C(ENTITY_CALLBACK_NONE);
    EXPOSE_C(ENTITY_CALLBACK_ACTIVATE);
    EXPOSE_C(ENTITY_CALLBACK_DEACTIVATE);
    EXPOSE_C(ENTITY_CALLBACK_COLLISION);
    EXPOSE_C(ENTITY_CALLBACK_STAND);
    EXPOSE_C(ENTITY_CALLBACK_HIT);
    EXPOSE_C(ENTITY_CALLBACK_ROOMCOLLISION);

    EXPOSE_C(COLLISION_TYPE_NONE);
    EXPOSE_C(COLLISION_TYPE_STATIC);
    EXPOSE_C(COLLISION_TYPE_KINEMATIC);
    EXPOSE_C(COLLISION_TYPE_DYNAMIC);
    EXPOSE_C(COLLISION_TYPE_ACTOR);
    EXPOSE_C(COLLISION_TYPE_VEHICLE);
    EXPOSE_C(COLLISION_TYPE_GHOST);

    EXPOSE_C(COLLISION_SHAPE_BOX);
    EXPOSE_C(COLLISION_SHAPE_BOX_BASE);
    EXPOSE_C(COLLISION_SHAPE_SPHERE);
    EXPOSE_C(COLLISION_SHAPE_TRIMESH);
    EXPOSE_C(COLLISION_SHAPE_TRIMESH_CONVEX);

    EXPOSE_C(SECTOR_MATERIAL_MUD);
    EXPOSE_C(SECTOR_MATERIAL_SNOW);
    EXPOSE_C(SECTOR_MATERIAL_SAND);
    EXPOSE_C(SECTOR_MATERIAL_GRAVEL);
    EXPOSE_C(SECTOR_MATERIAL_ICE);
    EXPOSE_C(SECTOR_MATERIAL_WATER);
    EXPOSE_C(SECTOR_MATERIAL_STONE);
    EXPOSE_C(SECTOR_MATERIAL_WOOD);
    EXPOSE_C(SECTOR_MATERIAL_METAL);
    EXPOSE_C(SECTOR_MATERIAL_MARBLE);
    EXPOSE_C(SECTOR_MATERIAL_GRASS);
    EXPOSE_C(SECTOR_MATERIAL_CONCRETE);
    EXPOSE_C(SECTOR_MATERIAL_OLDWOOD);
    EXPOSE_C(SECTOR_MATERIAL_OLDMETAL);

    EXPOSE_C(ANIM_NORMAL_CONTROL);
    EXPOSE_C(ANIM_LOOP_LAST_FRAME);
    EXPOSE_C(ANIM_LOCK);

    using engine::ACT_ACTION;
    EXPOSE_CC(ACT_ACTION);

#define EXPOSE_KEY(name) m_state.set("KEY_" #name, static_cast<int>(SDLK_##name))
#define EXPOSE_KEY2(name,value) m_state.set("KEY_" #name, static_cast<int>(SDLK_##value))

    EXPOSE_KEY(BACKSPACE);
    EXPOSE_KEY(TAB);
    EXPOSE_KEY(RETURN);
    EXPOSE_KEY(ESCAPE);
    EXPOSE_KEY(SPACE);
    EXPOSE_KEY(EXCLAIM);
    EXPOSE_KEY(QUOTEDBL);
    EXPOSE_KEY(HASH);
    EXPOSE_KEY(DOLLAR);
    EXPOSE_KEY(PERCENT);
    EXPOSE_KEY(AMPERSAND);
    EXPOSE_KEY(QUOTE);
    EXPOSE_KEY(LEFTPAREN);
    EXPOSE_KEY(RIGHTPAREN);
    EXPOSE_KEY(ASTERISK);
    EXPOSE_KEY(PLUS);
    EXPOSE_KEY(COMMA);
    EXPOSE_KEY(MINUS);
    EXPOSE_KEY(PERIOD);
    EXPOSE_KEY(SLASH);
    EXPOSE_KEY(0);
    EXPOSE_KEY(1);
    EXPOSE_KEY(2);
    EXPOSE_KEY(3);
    EXPOSE_KEY(4);
    EXPOSE_KEY(5);
    EXPOSE_KEY(6);
    EXPOSE_KEY(7);
    EXPOSE_KEY(8);
    EXPOSE_KEY(9);
    EXPOSE_KEY(COLON);
    EXPOSE_KEY(SEMICOLON);
    EXPOSE_KEY(LESS);
    EXPOSE_KEY(EQUALS);
    EXPOSE_KEY(GREATER);
    EXPOSE_KEY(QUESTION);
    EXPOSE_KEY(AT);
    EXPOSE_KEY(LEFTBRACKET);
    EXPOSE_KEY(BACKSLASH);
    EXPOSE_KEY(RIGHTBRACKET);
    EXPOSE_KEY(CARET);
    EXPOSE_KEY(UNDERSCORE);
    EXPOSE_KEY(BACKQUOTE);
    EXPOSE_KEY2(A,a);
    EXPOSE_KEY2(B,b);
    EXPOSE_KEY2(C,c);
    EXPOSE_KEY2(D,d);
    EXPOSE_KEY2(E,e);
    EXPOSE_KEY2(F,f);
    EXPOSE_KEY2(G,g);
    EXPOSE_KEY2(H,h);
    EXPOSE_KEY2(I,i);
    EXPOSE_KEY2(J,j);
    EXPOSE_KEY2(K,k);
    EXPOSE_KEY2(L,l);
    EXPOSE_KEY2(M,m);
    EXPOSE_KEY2(N,n);
    EXPOSE_KEY2(O,o);
    EXPOSE_KEY2(P,p);
    EXPOSE_KEY2(Q,q);
    EXPOSE_KEY2(R,r);
    EXPOSE_KEY2(S,s);
    EXPOSE_KEY2(T,t);
    EXPOSE_KEY2(U,u);
    EXPOSE_KEY2(V,v);
    EXPOSE_KEY2(W,w);
    EXPOSE_KEY2(X,x);
    EXPOSE_KEY2(Y,y);
    EXPOSE_KEY2(Z,z);
    EXPOSE_KEY(DELETE);
    EXPOSE_KEY(CAPSLOCK);
    EXPOSE_KEY(F1);
    EXPOSE_KEY(F2);
    EXPOSE_KEY(F3);
    EXPOSE_KEY(F4);
    EXPOSE_KEY(F5);
    EXPOSE_KEY(F6);
    EXPOSE_KEY(F7);
    EXPOSE_KEY(F8);
    EXPOSE_KEY(F9);
    EXPOSE_KEY(F10);
    EXPOSE_KEY(F11);
    EXPOSE_KEY(F12);
    EXPOSE_KEY(PRINTSCREEN);
    EXPOSE_KEY(SCROLLLOCK);
    EXPOSE_KEY(PAUSE);
    EXPOSE_KEY(INSERT);
    EXPOSE_KEY(HOME);
    EXPOSE_KEY(PAGEUP);
    EXPOSE_KEY(END);
    EXPOSE_KEY(PAGEDOWN);
    EXPOSE_KEY(RIGHT);
    EXPOSE_KEY(LEFT);
    EXPOSE_KEY(DOWN);
    EXPOSE_KEY(UP);
    EXPOSE_KEY(NUMLOCKCLEAR);
    EXPOSE_KEY(KP_DIVIDE);
    EXPOSE_KEY(KP_MULTIPLY);
    EXPOSE_KEY(KP_MINUS);
    EXPOSE_KEY(KP_PLUS);
    EXPOSE_KEY(KP_ENTER);
    EXPOSE_KEY(KP_1);
    EXPOSE_KEY(KP_2);
    EXPOSE_KEY(KP_3);
    EXPOSE_KEY(KP_4);
    EXPOSE_KEY(KP_5);
    EXPOSE_KEY(KP_6);
    EXPOSE_KEY(KP_7);
    EXPOSE_KEY(KP_8);
    EXPOSE_KEY(KP_9);
    EXPOSE_KEY(KP_0);
    EXPOSE_KEY(KP_PERIOD);
    EXPOSE_KEY(APPLICATION);
    EXPOSE_KEY(POWER);
    EXPOSE_KEY(KP_EQUALS);
    EXPOSE_KEY(F13);
    EXPOSE_KEY(F14);
    EXPOSE_KEY(F15);
    EXPOSE_KEY(F16);
    EXPOSE_KEY(F17);
    EXPOSE_KEY(F18);
    EXPOSE_KEY(F19);
    EXPOSE_KEY(F20);
    EXPOSE_KEY(F21);
    EXPOSE_KEY(F22);
    EXPOSE_KEY(F23);
    EXPOSE_KEY(F24);
    EXPOSE_KEY(EXECUTE);
    EXPOSE_KEY(HELP);
    EXPOSE_KEY(MENU);
    EXPOSE_KEY(SELECT);
    EXPOSE_KEY(STOP);
    EXPOSE_KEY(AGAIN);
    EXPOSE_KEY(UNDO);
    EXPOSE_KEY(CUT);
    EXPOSE_KEY(COPY);
    EXPOSE_KEY(PASTE);
    EXPOSE_KEY(FIND);
    EXPOSE_KEY(MUTE);
    EXPOSE_KEY(VOLUMEUP);
    EXPOSE_KEY(VOLUMEDOWN);
    EXPOSE_KEY(KP_COMMA);
    EXPOSE_KEY(KP_EQUALSAS400);
    EXPOSE_KEY(ALTERASE);
    EXPOSE_KEY(SYSREQ);
    EXPOSE_KEY(CANCEL);
    EXPOSE_KEY(CLEAR);
    EXPOSE_KEY(PRIOR);
    EXPOSE_KEY(RETURN2);
    EXPOSE_KEY(SEPARATOR);
    EXPOSE_KEY(OUT);
    EXPOSE_KEY(OPER);
    EXPOSE_KEY(CLEARAGAIN);
    EXPOSE_KEY(CRSEL);
    EXPOSE_KEY(EXSEL);
    EXPOSE_KEY(KP_00);
    EXPOSE_KEY(KP_000);
    EXPOSE_KEY(THOUSANDSSEPARATOR);
    EXPOSE_KEY(DECIMALSEPARATOR);
    EXPOSE_KEY(CURRENCYUNIT);
    EXPOSE_KEY(CURRENCYSUBUNIT);
    EXPOSE_KEY(KP_LEFTPAREN);
    EXPOSE_KEY(KP_RIGHTPAREN);
    EXPOSE_KEY(KP_LEFTBRACE);
    EXPOSE_KEY(KP_RIGHTBRACE);
    EXPOSE_KEY(KP_TAB);
    EXPOSE_KEY(KP_BACKSPACE);
    EXPOSE_KEY(KP_A);
    EXPOSE_KEY(KP_B);
    EXPOSE_KEY(KP_C);
    EXPOSE_KEY(KP_D);
    EXPOSE_KEY(KP_E);
    EXPOSE_KEY(KP_F);
    EXPOSE_KEY(KP_XOR);
    EXPOSE_KEY(KP_POWER);
    EXPOSE_KEY(KP_PERCENT);
    EXPOSE_KEY(KP_LESS);
    EXPOSE_KEY(KP_GREATER);
    EXPOSE_KEY(KP_AMPERSAND);
    EXPOSE_KEY(KP_DBLAMPERSAND);
    EXPOSE_KEY(KP_VERTICALBAR);
    EXPOSE_KEY(KP_DBLVERTICALBAR);
    EXPOSE_KEY(KP_COLON);
    EXPOSE_KEY(KP_HASH);
    EXPOSE_KEY(KP_SPACE);
    EXPOSE_KEY(KP_AT);
    EXPOSE_KEY(KP_EXCLAM);
    EXPOSE_KEY(KP_MEMSTORE);
    EXPOSE_KEY(KP_MEMRECALL);
    EXPOSE_KEY(KP_MEMCLEAR);
    EXPOSE_KEY(KP_MEMADD);
    EXPOSE_KEY(KP_MEMSUBTRACT);
    EXPOSE_KEY(KP_MEMMULTIPLY);
    EXPOSE_KEY(KP_MEMDIVIDE);
    EXPOSE_KEY(KP_PLUSMINUS);
    EXPOSE_KEY(KP_CLEAR);
    EXPOSE_KEY(KP_CLEARENTRY);
    EXPOSE_KEY(KP_BINARY);
    EXPOSE_KEY(KP_OCTAL);
    EXPOSE_KEY(KP_DECIMAL);
    EXPOSE_KEY(KP_HEXADECIMAL);
    EXPOSE_KEY(LCTRL);
    EXPOSE_KEY(LSHIFT);
    EXPOSE_KEY(LALT);
    EXPOSE_KEY(LGUI);
    EXPOSE_KEY(RCTRL);
    EXPOSE_KEY(RSHIFT);
    EXPOSE_KEY(RALT);
    EXPOSE_KEY(RGUI);
    EXPOSE_KEY(MODE);
    EXPOSE_KEY(AUDIONEXT);
    EXPOSE_KEY(AUDIOPREV);
    EXPOSE_KEY(AUDIOSTOP);
    EXPOSE_KEY(AUDIOPLAY);
    EXPOSE_KEY(AUDIOMUTE);
    EXPOSE_KEY(MEDIASELECT);
    EXPOSE_KEY(WWW);
    EXPOSE_KEY(MAIL);
    EXPOSE_KEY(CALCULATOR);
    EXPOSE_KEY(COMPUTER);
    EXPOSE_KEY(AC_SEARCH);
    EXPOSE_KEY(AC_HOME);
    EXPOSE_KEY(AC_BACK);
    EXPOSE_KEY(AC_FORWARD);
    EXPOSE_KEY(AC_STOP);
    EXPOSE_KEY(AC_REFRESH);
    EXPOSE_KEY(AC_BOOKMARKS);
    EXPOSE_KEY(BRIGHTNESSDOWN);
    EXPOSE_KEY(BRIGHTNESSUP);
    EXPOSE_KEY(DISPLAYSWITCH);
    EXPOSE_KEY(KBDILLUMTOGGLE);
    EXPOSE_KEY(KBDILLUMDOWN);
    EXPOSE_KEY(KBDILLUMUP);
    EXPOSE_KEY(EJECT);
    EXPOSE_KEY(SLEEP);

#undef EXPOSE_KEY
#undef EXPOSE_KEY2

    m_state.set("JOY_1", 1000);
    m_state.set("JOY_2", 1001);
    m_state.set("JOY_3", 1002);
    m_state.set("JOY_4", 1003);
    m_state.set("JOY_5", 1004);
    m_state.set("JOY_6", 1005);
    m_state.set("JOY_7", 1006);
    m_state.set("JOY_8", 1007);
    m_state.set("JOY_9", 1008);
    m_state.set("JOY_10", 1009);
    m_state.set("JOY_11", 1010);
    m_state.set("JOY_12", 1011);
    m_state.set("JOY_13", 1012);
    m_state.set("JOY_14", 1013);
    m_state.set("JOY_15", 1014);
    m_state.set("JOY_16", 1015);
    m_state.set("JOY_17", 1016);
    m_state.set("JOY_18", 1017);
    m_state.set("JOY_19", 1018);
    m_state.set("JOY_20", 1019);
    m_state.set("JOY_21", 1020);
    m_state.set("JOY_22", 1021);
    m_state.set("JOY_23", 1022);
    m_state.set("JOY_24", 1023);
    m_state.set("JOY_25", 1024);
    m_state.set("JOY_26", 1025);
    m_state.set("JOY_27", 1026);
    m_state.set("JOY_28", 1027);
    m_state.set("JOY_29", 1028);
    m_state.set("JOY_30", 1029);
    m_state.set("JOY_31", 1030);
    m_state.set("JOY_32", 1031);
    m_state.set("JOY_POVUP", 1101);
    m_state.set("JOY_POVDOWN", 1104);
    m_state.set("JOY_POVLEFT", 1108);
    m_state.set("JOY_POVRIGHT", 1102);

    m_state.set("JOY_TRIGGERLEFT", 1204); // Only for XBOX360-like controllers - analog triggers.
    m_state.set("JOY_TRIGGERRIGHT", 1205);

    EXPOSE_C(COLLISION_TYPE_NONE);
    EXPOSE_C(COLLISION_TYPE_STATIC);
    EXPOSE_C(COLLISION_TYPE_KINEMATIC);
    EXPOSE_C(COLLISION_TYPE_DYNAMIC);
    EXPOSE_C(COLLISION_TYPE_ACTOR);
    EXPOSE_C(COLLISION_TYPE_VEHICLE);
    EXPOSE_C(COLLISION_TYPE_GHOST);

    EXPOSE_C(COLLISION_SHAPE_BOX);
    EXPOSE_C(COLLISION_SHAPE_BOX_BASE);
    EXPOSE_C(COLLISION_SHAPE_SPHERE);
    EXPOSE_C(COLLISION_SHAPE_TRIMESH);
    EXPOSE_C(COLLISION_SHAPE_TRIMESH_CONVEX);

    EXPOSE_CC(PARAM_HEALTH);
    EXPOSE_CC(PARAM_AIR);
    EXPOSE_CC(PARAM_STAMINA);
    EXPOSE_CC(PARAM_WARMTH);
    EXPOSE_CC(PARAM_POISON);
    EXPOSE_CC(PARAM_EXTRA1);
    EXPOSE_CC(PARAM_EXTRA2);
    EXPOSE_CC(PARAM_EXTRA3);
    EXPOSE_CC(PARAM_EXTRA4);

    EXPOSE_C(PARAM_ABSOLUTE_MAX);

    EXPOSE_C(BODY_PART_BODY_LOW);
    EXPOSE_C(BODY_PART_BODY_UPPER);
    EXPOSE_C(BODY_PART_HEAD);

    EXPOSE_C(BODY_PART_LEFT_HAND_1);
    EXPOSE_C(BODY_PART_LEFT_HAND_2);
    EXPOSE_C(BODY_PART_LEFT_HAND_3);
    EXPOSE_C(BODY_PART_RIGHT_HAND_1);
    EXPOSE_C(BODY_PART_RIGHT_HAND_2);
    EXPOSE_C(BODY_PART_RIGHT_HAND_3);

    EXPOSE_C(BODY_PART_LEFT_LEG_1);
    EXPOSE_C(BODY_PART_LEFT_LEG_2);
    EXPOSE_C(BODY_PART_LEFT_LEG_3);
    EXPOSE_C(BODY_PART_RIGHT_LEG_1);
    EXPOSE_C(BODY_PART_RIGHT_LEG_2);
    EXPOSE_C(BODY_PART_RIGHT_LEG_3);

    EXPOSE_C(HAIR_TR1);
    EXPOSE_C(HAIR_TR2);
    EXPOSE_C(HAIR_TR3);
    EXPOSE_C(HAIR_TR4_KID_1);
    EXPOSE_C(HAIR_TR4_KID_2);
    EXPOSE_C(HAIR_TR4_OLD);
    EXPOSE_C(HAIR_TR5_KID_1);
    EXPOSE_C(HAIR_TR5_KID_2);
    EXPOSE_C(HAIR_TR5_OLD);

    EXPOSE_C(M_PI);

    using gui::FontStyle;
    m_state.set( "FONTSTYLE_CONSOLE_INFO", static_cast<int>(FontStyle::ConsoleInfo) );
    m_state.set( "FONTSTYLE_CONSOLE_WARNING", static_cast<int>(FontStyle::ConsoleWarning) );
    m_state.set( "FONTSTYLE_CONSOLE_EVENT", static_cast<int>(FontStyle::ConsoleEvent) );
    m_state.set( "FONTSTYLE_CONSOLE_NOTIFY", static_cast<int>(FontStyle::ConsoleNotify) );
    m_state.set( "FONTSTYLE_MENU_TITLE", static_cast<int>(FontStyle::MenuTitle) );
    m_state.set( "FONTSTYLE_MENU_HEADING1", static_cast<int>(FontStyle::MenuHeading1) );
    m_state.set( "FONTSTYLE_MENU_HEADING2", static_cast<int>(FontStyle::MenuHeading2) );
    m_state.set( "FONTSTYLE_MENU_ITEM_ACTIVE", static_cast<int>(FontStyle::MenuItemActive) );
    m_state.set( "FONTSTYLE_MENU_ITEM_INACTIVE", static_cast<int>(FontStyle::MenuItemInactive) );
    m_state.set( "FONTSTYLE_MENU_CONTENT", static_cast<int>(FontStyle::MenuContent) );
    m_state.set( "FONTSTYLE_STATS_TITLE", static_cast<int>(FontStyle::StatsTitle) );
    m_state.set( "FONTSTYLE_STATS_CONTENT", static_cast<int>(FontStyle::StatsContent) );
    m_state.set( "FONTSTYLE_NOTIFIER", static_cast<int>(FontStyle::Notifier) );
    m_state.set( "FONTSTYLE_SAVEGAMELIST", static_cast<int>(FontStyle::SavegameList) );
    m_state.set( "FONTSTYLE_GENERIC", static_cast<int>(FontStyle::Generic) );

    using gui::FontType;
    m_state.set( "FONT_PRIMARY", static_cast<int>(FontType::Primary) );
    m_state.set( "FONT_SECONDARY", static_cast<int>(FontType::Secondary) );
    m_state.set( "FONT_CONSOLE", static_cast<int>(FontType::Console) );

#undef EXPOSE_C
#undef EXPOSE_CC
}

std::vector<std::string> ScriptEngine::getGlobals()
{
    std::vector<std::string> result;
    auto L = m_state.getState();
    lua_pushglobaltable(L);
    lua_pushnil(L);
    while (lua_next(L, -2) != 0)
    {
        result.emplace_back(lua_tostring(L, -2));
        lua_pop(L, 1);
    }
    lua_pop(L, 1);
    return result;
}

int ScriptEngine::print(lua_State* state)
{
    const int top = lua_gettop(state);

    if(top == 0)
    {
        Console::instance().addLine("nil", gui::FontStyle::ConsoleEvent);
        return 0;
    }

    for(int i = 1; i <= top; i++)
    {
        const char* str;
        switch(lua_type(state, i))
        {
            case LUA_TNONE:
                str = "<none>";
                break;
            case LUA_TNIL:
                str = "nil";
                break;
            case LUA_TBOOLEAN:
                str = lua_toboolean(state, i) ? "true" : "false";
                break;
            case LUA_TLIGHTUSERDATA:
                str = "<userdata>";
                break;
            case LUA_TNUMBER:
            case LUA_TSTRING:
                str = lua_tostring(state, i);
                break;
            case LUA_TTABLE:
                str = "<table>";
                break;
            case LUA_TFUNCTION:
                str = "<function>";
                break;
            case LUA_TUSERDATA:
                str = "<userdata>";
                break;
            case LUA_TTHREAD:
                str = "<thread>";
                break;
            default:
                str = "<invalid>";
                break;
        }

        Console::instance().addLine(str, gui::FontStyle::ConsoleEvent);
    }
    return 0;
}

// Called when something goes absolutely horribly wrong in Lua, and tries
// to produce some debug output. Lua calls abort afterwards, so sending
// the output to the internal console is not an option.

int ScriptEngine::panic(lua_State *lua)
{
    if(lua_gettop(lua) < 1)
    {
        fprintf(stderr, "Fatal lua error (no details provided).\n");
    }
    else
    {
        fprintf(stderr, "Fatal lua error: %s\n", lua_tostring(lua, 1));
    }
    fflush(stderr);
    return 0;
}

void MainEngine::registerMainFunctions()
{
    // Register globals

    set(CVAR_LUA_TABLE_NAME, lua::Table());

    engine::Game_RegisterLuaFunctions(*this);

    // Register script functions

    registerC("checkStack", std::function<void()>(std::bind(&MainEngine::checkStack, this)));
    registerC("dumpModel", lua_DumpModel);
    registerC("dumpRoom", lua_DumpRoom);
    registerC("setRoomEnabled", lua_SetRoomEnabled);

    registerC("playSound", lua_PlaySound);
    registerC("stopSound", lua_StopSound);

    registerC("playStream", lua_PlayStream);
    registerC("stopStreams", lua_StopStreams);

    registerC("setLevel", lua_SetLevel);
    registerC("getLevel", lua_GetLevel);

    registerC("setGame", lua_SetGame);
    registerC("loadMap", lua_LoadMap);

    registerC("camShake", lua_CamShake);

    registerC("fadeOut", lua_FadeOut);
    registerC("fadeIn", lua_FadeIn);
    registerC("fadeCheck", lua_FadeCheck);

    registerC("flashSetup", lua_FlashSetup);
    registerC("flashStart", lua_FlashStart);

    registerC("getEngineVersion", lua_GetEngineVersion);

    registerC("setFlipMap", lua_SetFlipMap);
    registerC("getFlipMap", lua_GetFlipMap);
    registerC("setFlipState", lua_SetFlipState);
    registerC("getFlipState", lua_GetFlipState);

    registerC("setModelCollisionMapSize", lua_SetModelCollisionMapSize);
    registerC("setModelCollisionMap", lua_SetModelCollisionMap);
    registerC("getAnimCommandTransform", lua_GetAnimCommandTransform);
    registerC("setAnimCommandTransform", lua_SetAnimCommandTransform);
    registerC("setStateChangeRange", lua_SetStateChangeRange);
    registerC("setAnimVerticalSpeed", lua_SetAnimVerticalSpeed);

    registerC("addItem", lua_AddItem);
    registerC("removeItem", lua_RemoveItem);
    registerC("removeAllItems", lua_RemoveAllItems);
    registerC("getItemsCount", lua_GetItemsCount);
    registerC("createBaseItem", lua_CreateBaseItem);
    registerC("deleteBaseItem", lua_DeleteBaseItem);
    registerC("printItems", lua_PrintItems);

    registerC("canTriggerEntity", lua_CanTriggerEntity);
    registerC("spawnEntity", lua_SpawnEntity);
    registerC("deleteEntity", lua_DeleteEntity);
    registerC("enableEntity", lua_EnableEntity);
    registerC("disableEntity", lua_DisableEntity);

    registerC("isInRoom", lua_IsInRoom);
    registerC("sameRoom", lua_SameRoom);
    registerC("sameSector", lua_SameSector);
    registerC("newSector", lua_NewSector);
    registerC("similarSector", lua_SimilarSector);
    registerC("getSectorHeight", lua_GetSectorHeight);

    registerC("moveEntityGlobal", lua_MoveEntityGlobal);
    registerC("moveEntityLocal", lua_MoveEntityLocal);
    registerC("moveEntityToSink", lua_MoveEntityToSink);
    registerC("moveEntityToEntity", lua_MoveEntityToEntity);
    registerC("rotateEntity", lua_RotateEntity);
    registerC("rotateEntityToEntity", lua_RotateEntityToEntity);

    registerC("getEntityModelID", lua_GetEntityModelID);

    registerC("getEntityVector", lua_GetEntityVector);
    registerC("getEntityDirDot", lua_GetEntityDirDot);
    registerC("getEntityOrientation", lua_GetEntityOrientation);
    registerC("getEntityDistance", lua_GetEntityDistance);
    registerC("getEntityPos", lua_GetEntityPosition);
    registerC("setEntityPos", lua_SetEntityPosition);
    registerC("getEntityAngles", lua_GetEntityAngles);
    registerC("setEntityAngles", lua_SetEntityAngles);
    registerC("getEntityScaling", lua_GetEntityScaling);
    registerC("setEntityScaling", lua_SetEntityScaling);
    registerC("getEntitySpeed", lua_GetEntitySpeed);
    registerC("setEntitySpeed", lua_SetEntitySpeed);
    registerC("getEntitySpeedLinear", lua_GetEntitySpeedLinear);
    registerC("setEntityCollision", lua_SetEntityCollision);
    registerC("setEntityCollisionFlags", lua_SetEntityCollisionFlags);
    registerC("getEntityAnim", lua_GetEntityAnim);
    registerC("setEntityAnim", lua_SetEntityAnim);
    registerC("setEntityAnimFlag", lua_SetEntityAnimFlag);
    registerC("setEntityBodyPartFlag", lua_SetEntityBodyPartFlag);
    registerC("setModelBodyPartFlag", lua_SetModelBodyPartFlag);
    registerC("getEntityModel", lua_GetEntityModel);
    registerC("getEntityVisibility", lua_GetEntityVisibility);
    registerC("setEntityVisibility", lua_SetEntityVisibility);
    registerC("getEntityActivity", lua_GetEntityActivity);
    registerC("setEntityActivity", lua_SetEntityActivity);
    registerC("getEntityEnability", lua_GetEntityEnability);
    registerC("getEntityOCB", lua_GetEntityOCB);
    registerC("setEntityOCB", lua_SetEntityOCB);
    registerC("getEntityTimer", lua_GetEntityTimer);
    registerC("setEntityTimer", lua_SetEntityTimer);
    registerC("getEntityFlags", lua_GetEntityFlags);
    registerC("setEntityFlags", lua_SetEntityFlags);
    registerC("getEntityTypeFlag", lua_GetEntityTypeFlag);
    registerC("setEntityTypeFlag", lua_SetEntityTypeFlag);
    registerC("getEntityStateFlag", lua_GetEntityStateFlag);
    registerC("setEntityStateFlag", lua_SetEntityStateFlag);
    registerC("getEntityCallbackFlag", lua_GetEntityCallbackFlag);
    registerC("setEntityCallbackFlag", lua_SetEntityCallbackFlag);
    registerC("getEntityState", lua_GetEntityState);
    registerC("setEntityState", lua_SetEntityState);
    registerC("setEntityRoomMove", lua_SetEntityRoomMove);
    registerC("getEntityMoveType", lua_GetEntityMoveType);
    registerC("setEntityMoveType", lua_SetEntityMoveType);
    registerC("getEntityResponse", lua_GetEntityResponse);
    registerC("setEntityResponse", lua_SetEntityResponse);
    registerC("getEntityMeshCount", lua_GetEntityMeshCount);
    registerC("setEntityMeshswap", lua_SetEntityMeshswap);
    registerC("setModelMeshReplaceFlag", lua_SetModelMeshReplaceFlag);
    registerC("setModelAnimReplaceFlag", lua_SetModelAnimReplaceFlag);
    registerC("copyMeshFromModelToModel", lua_CopyMeshFromModelToModel);

    registerC("createEntityGhosts", lua_CreateEntityGhosts);
    registerC("setEntityBodyMass", lua_SetEntityBodyMass);
    registerC("pushEntityBody", lua_PushEntityBody);
    registerC("lockEntityBodyLinearFactor", lua_LockEntityBodyLinearFactor);

    registerC("getEntityTriggerLayout", lua_GetEntityTriggerLayout);
    registerC("setEntityTriggerLayout", lua_SetEntityTriggerLayout);
    registerC("getEntityMask", lua_GetEntityMask);
    registerC("setEntityMask", lua_SetEntityMask);
    registerC("getEntityEvent", lua_GetEntityEvent);
    registerC("setEntityEvent", lua_SetEntityEvent);
    registerC("getEntityLock", lua_GetEntityLock);
    registerC("setEntityLock", lua_SetEntityLock);
    registerC("getEntitySectorStatus", lua_GetEntitySectorStatus);
    registerC("setEntitySectorStatus", lua_SetEntitySectorStatus);

    registerC("getEntityActivationOffset", lua_GetEntityActivationOffset);
    registerC("setEntityActivationOffset", lua_SetEntityActivationOffset);
    registerC("getEntitySectorIndex", lua_GetEntitySectorIndex);
    registerC("getEntitySectorFlags", lua_GetEntitySectorFlags);
    registerC("getEntitySectorMaterial", lua_GetEntitySectorMaterial);
    registerC("getEntitySubstanceState", lua_GetEntitySubstanceState);

    registerC("addEntityRagdoll", lua_AddEntityRagdoll);
    registerC("removeEntityRagdoll", lua_RemoveEntityRagdoll);

    registerC("getCharacterParam", lua_GetCharacterParam);
    registerC("setCharacterParam", lua_SetCharacterParam);
    registerC("changeCharacterParam", lua_ChangeCharacterParam);
    registerC("getCharacterCurrentWeapon", lua_GetCharacterCurrentWeapon);
    registerC("setCharacterCurrentWeapon", lua_SetCharacterCurrentWeapon);
    registerC("setCharacterWeaponModel", lua_SetCharacterWeaponModel);
    registerC("getCharacterCombatMode", lua_GetCharacterCombatMode);

    registerC("addCharacterHair", lua_AddCharacterHair);
    registerC("resetCharacterHair", lua_ResetCharacterHair);

    registerC("getSecretStatus", lua_GetSecretStatus);
    registerC("setSecretStatus", lua_SetSecretStatus);

    registerC("getActionState", lua_GetActionState);
    registerC("getActionChange", lua_GetActionChange);

    registerC("genUVRotateAnimation", lua_genUVRotateAnimation);

    registerC("getGravity", lua_GetGravity);
    registerC("setGravity", lua_SetGravity);
    registerC("dropEntity", lua_DropEntity);
    registerC("bind", &MainEngine::bindKey);

    registerC("addFont", lua_AddFont);
    registerC("deleteFont", lua_DeleteFont);
    registerC("addFontStyle", lua_AddFontStyle);
    registerC("deleteFontStyle", lua_DeleteFontStyle);
}
}   // end namespace script


// Parsing words

const char *script::MainEngine::parse_token(const char *data, char *token)
{
    ///@FIXME: token may be overflowed
    int c;
    int len;

    len = 0;
    token[0] = 0;

    if(!data)
    {
        return nullptr;
    }

    // skip whitespace
skipwhite:
    while((c = *data) <= ' ')
    {
        if(c == 0)
            return nullptr;                    // end of file;
        data++;
    }

    // skip // comments
    if(c == '/' && data[1] == '/')
    {
        while(*data && *data != '\n')
            data++;
        goto skipwhite;
    }

    // handle quoted strings specially
    if(c == '\"')
    {
        data++;
        while(1)
        {
            c = *data++;
            if(c == '\"' || !c)
            {
                token[len] = 0;
                return data;
            }
            token[len] = c;
            len++;
        }
    }

    // parse single characters
    if(c == '{' || c == '}' || c == ')' || c == '(' || c == '\'' || c == ':')
    {
        token[len] = c;
        len++;
        token[len] = 0;
        return data + 1;
    }

    // parse a regular word
    do
    {
        token[len] = c;
        data++;
        len++;
        c = *data;
        if(c == '{' || c == '}' || c == ')' || c == '(' || c == '\'' || c == ':')
        {
            break;
        }
    } while(c > 32);

    token[len] = 0;
    return data;
}

float script::MainEngine::parseFloat(const char **ch)
{
    char token[64];
    (*ch) = parse_token(*ch, token);
    if(token[0])
    {
        return atof(token);
    }
    return 0.0;
}

int script::MainEngine::parseInt(char **ch)
{
    char token[64];
    (*ch) = const_cast<char*>(parse_token(*ch, token));
    if(token[0])
    {
        return atoi(token);
    }
    return 0;
}

// Specific functions to get specific parameters from script.

int script::MainEngine::getGlobalSound(int global_sound_id)
{
    return call("getGlobalSound", static_cast<int>(engine::engine_world.engineVersion), global_sound_id);
}

int script::MainEngine::getSecretTrackNumber()
{
    return call("getSecretTrackNumber", static_cast<int>(engine::engine_world.engineVersion));
}

int script::MainEngine::getNumTracks()
{
    return call("getNumTracks", static_cast<int>(engine::engine_world.engineVersion));
}

bool script::MainEngine::getOverridedSamplesInfo(int *num_samples, int *num_sounds, char *sample_name_mask)
{
    const char* realPath;
    lua::tie(realPath, *num_sounds, *num_samples) = call("getOverridedSamplesInfo", static_cast<int>(engine::engine_world.engineVersion));

    strcpy(sample_name_mask, realPath);

    return *num_sounds != -1 && *num_samples != -1 && strcmp(realPath, "NONE") != 0;
}

bool script::MainEngine::getOverridedSample(int sound_id, int *first_sample_number, int *samples_count)
{
    lua::tie(*first_sample_number, *samples_count) = call("getOverridedSample", static_cast<int>(engine::engine_world.engineVersion), int(engine::Gameflow_Manager.getLevelID()), sound_id);
    return *first_sample_number != -1 && *samples_count != -1;
}

bool script::MainEngine::getSoundtrack(int track_index, char *file_path, audio::StreamMethod *load_method, audio::StreamType *stream_type)
{
    const char* realPath;
    int _load_method, _stream_type;

    lua::tie(realPath, _stream_type, _load_method) = call("getTrackInfo", static_cast<int>(engine::engine_world.engineVersion), track_index);
    if(file_path) strcpy(file_path, realPath);
    if(load_method) *load_method = static_cast<audio::StreamMethod>(_load_method);
    if(stream_type) *stream_type = static_cast<audio::StreamType>(_stream_type);
    return _stream_type != -1;
}

bool script::MainEngine::getString(int string_index, size_t string_size, char *buffer)
{
    const char* str = call("getString", string_index);
    strncpy(buffer, str, string_size);
    return true;
}

bool script::MainEngine::getSysNotify(int string_index, size_t string_size, char *buffer)
{
    const char* str = call("getSysNotify", string_index);
    strncpy(buffer, str, string_size);
    return true;
}

bool script::MainEngine::getLoadingScreen(int level_index, char *pic_path)
{
    const char* realPath = call("getLoadingScreen", int(engine::Gameflow_Manager.getGameID()), int(engine::Gameflow_Manager.getLevelID()), level_index);
    strncpy(pic_path, realPath, MAX_ENGINE_PATH);
    return true;
}


// System lua functions


void script::MainEngine::addKey(int keycode, bool state)
{
    call("addKey", keycode, state);
}

void script::MainEngine::execEntity(int id_callback, int id_object, int id_activator)
{
    if(id_activator >= 0)
        call("execEntity", id_callback, id_object, id_activator);
    else
        call("execEntity", id_callback, id_object);
}

void script::MainEngine::loopEntity(int object_id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(object_id);
    if(ent && ent->m_active)
    {
        call("loopEntity", object_id);
    }
}

void script::MainEngine::execEffect(int id, int caller, int operand)
{
    call("execFlipeffect", id, caller, operand);
}


// Parsing config file entries.

void script::ScriptEngine::parseControls(engine::ControlSettings *cs)
{
    cs->mouse_sensitivity = (*this)["controls"]["mouse_sensitivity"];
    cs->mouse_scale_x = (*this)["controls"]["mouse_scale_x"];
    cs->mouse_scale_y = (*this)["controls"]["mouse_scale_y"];
    cs->use_joy = (*this)["controls"]["use_joy"];
    cs->joy_number = (*this)["controls"]["joy_number"];
    cs->joy_rumble = (*this)["controls"]["joy_rumble"];
    cs->joy_axis_map[engine::AXIS_LOOK_X] = (*this)["controls"]["joy_look_axis_x"];
    cs->joy_axis_map[engine::AXIS_LOOK_Y] = (*this)["controls"]["joy_look_axis_y"];
    cs->joy_axis_map[engine::AXIS_MOVE_X] = (*this)["controls"]["joy_move_axis_x"];
    cs->joy_axis_map[engine::AXIS_MOVE_Y] = (*this)["controls"]["joy_move_axis_y"];
    cs->joy_look_invert_x = (*this)["controls"]["joy_look_invert_x"];
    cs->joy_look_invert_y = (*this)["controls"]["joy_look_invert_y"];
    cs->joy_look_sensitivity = (*this)["controls"]["joy_look_sensitivity"];
    cs->joy_look_deadzone = (*this)["controls"]["joy_look_deadzone"];
    cs->joy_move_invert_x = (*this)["controls"]["joy_move_invert_x"];
    cs->joy_move_invert_y = (*this)["controls"]["joy_move_invert_y"];
    cs->joy_move_sensitivity = (*this)["controls"]["joy_move_sensitivity"];
    cs->joy_move_deadzone = (*this)["controls"]["joy_move_deadzone"];
}

void script::ScriptEngine::parseScreen(engine::ScreenInfo *sc)
{
    sc->x = (*this)["screen"]["x"];
    sc->y = (*this)["screen"]["y"];
    sc->w = (*this)["screen"]["width"];
    sc->h = (*this)["screen"]["height"];
    sc->w = (*this)["screen"]["width"];
    sc->w_unit = sc->w / gui::ScreenMeteringResolution;
    sc->h = (*this)["screen"]["height"];
    sc->h_unit = sc->h / gui::ScreenMeteringResolution;
    sc->FS_flag = (*this)["screen"]["fullscreen"];
    sc->show_debuginfo = (*this)["screen"]["debug_info"];
    sc->fov = (*this)["screen"]["fov"];
    sc->vsync = (*this)["screen"]["vsync"];
}

void script::ScriptEngine::parseRender(render::RenderSettings *rs)
{
    rs->mipmap_mode = (*this)["render"]["mipmap_mode"];
    rs->mipmaps = (*this)["render"]["mipmaps"];
    rs->lod_bias = (*this)["render"]["lod_bias"];
    rs->anisotropy = (*this)["render"]["anisotropy"];
    rs->antialias = (*this)["render"]["antialias"];
    rs->antialias_samples = (*this)["render"]["antialias_samples"];
    rs->texture_border = (*this)["render"]["texture_border"];
    rs->save_texture_memory = (*this)["render"]["save_texture_memory"];
    rs->z_depth = (*this)["render"]["z_depth"];
    rs->fog_enabled = (*this)["render"]["fog_enabled"];
    rs->fog_start_depth = (*this)["render"]["fog_start_depth"];
    rs->fog_end_depth = (*this)["render"]["fog_end_depth"];
    rs->fog_color[0] = (*this)["render"]["fog_color"]["r"];
    rs->fog_color[0] /= 255.0;
    rs->fog_color[1] = (*this)["render"]["fog_color"]["g"];
    rs->fog_color[1] /= 255.0;
    rs->fog_color[2] = (*this)["render"]["fog_color"]["b"];
    rs->fog_color[2] /= 255.0;
    rs->fog_color[3] = 1;

    rs->use_gl3 = (*this)["render"]["use_gl3"];

    if(rs->z_depth != 8 && rs->z_depth != 16 && rs->z_depth != 24)
        rs->z_depth = 24;
}

void script::ScriptEngine::parseAudio(audio::Settings *as)
{
    as->music_volume = (*this)["audio"]["music_volume"];
    as->sound_volume = (*this)["audio"]["sound_volume"];
    as->use_effects = (*this)["audio"]["use_effects"].to<bool>();
    as->listener_is_player = (*this)["audio"]["listener_is_player"].to<bool>();
    as->stream_buffer_size = (*this)["audio"]["stream_buffer_size"];
    as->stream_buffer_size *= 1024;
    if(as->stream_buffer_size <= 0)
        as->stream_buffer_size = 128 * 1024;
    as->music_volume = (*this)["audio"]["music_volume"];
    as->music_volume = (*this)["audio"]["music_volume"];
}

void script::ScriptEngine::parseConsole(Console *cn)
{
    {
        float r = (*this)["console"]["background_color"]["r"];
        float g = (*this)["console"]["background_color"]["g"];
        float b = (*this)["console"]["background_color"]["b"];
        float a = (*this)["console"]["background_color"]["a"];
        cn->setBackgroundColor(r / 255, g / 255, b / 255, a / 255);
    }

    float tmpF = (*this)["console"]["spacing"];
    if(tmpF >= CON_MIN_LINE_INTERVAL && tmpF <= CON_MAX_LINE_INTERVAL)
        cn->setSpacing(tmpF);

    int tmpI = (*this)["console"]["line_size"];
    if(tmpI >= CON_MIN_LINE_SIZE && tmpI <= CON_MAX_LINE_SIZE)
        cn->setLineSize(tmpI);

    tmpI = (*this)["console"]["showing_lines"];
    if(tmpI >= CON_MIN_LINES && tmpI <= CON_MIN_LINES)
        cn->setVisibleLines(tmpI);

    tmpI = (*this)["console"]["log_size"];
    if(tmpI >= CON_MIN_LOG && tmpI <= CON_MAX_LOG)
        cn->setHistorySize(tmpI);

    tmpI = (*this)["console"]["lines_count"];
    if(tmpI >= CON_MIN_LOG && tmpI <= CON_MAX_LOG)
        cn->setBufferSize(tmpI);

    bool tmpB = (*this)["console"]["show"];
    cn->setVisible(tmpB);

    tmpF = (*this)["console"]["show_cursor_period"];
    cn->setShowCursorPeriod(tmpF);
}

void script::ScriptEngine::parseSystem(engine::SystemSettings *ss)
{
    ss->logging = (*this)["system"]["logging"];
}
