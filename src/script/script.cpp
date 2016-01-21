#include "script.h"

#include <cstring>

#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>

#include <boost/log/trivial.hpp>

#include "LuaState.h"

#include "audio/settings.h"
#include "character_controller.h"
#include "engine/controls.h"
#include "engine/engine.h"
#include "engine/game.h"
#include "engine/gameflow.h"
#include "engine/system.h"
#include "gui/console.h"
#include "gui/fader.h"
#include "gui/fadermanager.h"
#include "inventory.h"
#include "render/render.h"
#include "strings.h"
#include "util/helpers.h"
#include "util/vmath.h"
#include "world/character.h"
#include "world/core/basemesh.h"
#include "world/entity.h"
#include "world/hair.h"
#include "world/ragdoll.h"
#include "world/room.h"
#include "world/skeletalmodel.h"
#include "world/world.h"

// Debug functions

using gui::Console;

script::MainEngine engine_lua;

void script::ScriptEngine::checkStack()
{
    Console::instance().notify(SYSNOTE_LUA_STACK_INDEX, lua_gettop(m_state.getState()));
}

void lua_DumpModel(world::ModelId id)
{
    world::SkeletalModel* sm = engine::engine_world.getModelByID(id);
    if(sm == nullptr)
    {
        Console::instance().warning(SYSWARN_WRONG_MODEL_ID, id);
        return;
    }

    for(size_t i = 0; i < sm->meshes.size(); i++)
    {
        Console::instance().printf("mesh[%d] = %d", static_cast<int>(i), sm->meshes[i].mesh_base->m_id);
    }
}

void lua_DumpRoom(lua::Value id)
{
    if(!id.is<lua::Unsigned>())
    {
        engine::dumpRoom(engine::engine_camera.getCurrentRoom());
        return;
    }
    if(id.to<world::ModelId>() >= engine::engine_world.rooms.size())
    {
        Console::instance().warning(SYSWARN_WRONG_ROOM, id.to<world::ModelId>());
        return;
    }
    engine::dumpRoom(engine::engine_world.rooms[id.to<world::ModelId>()].get());
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

void lua_SetModelCollisionMapSize(world::ModelId id, size_t size)
{
    world::SkeletalModel* model = engine::engine_world.getModelByID(id);
    if(model == nullptr)
    {
        Console::instance().warning(SYSWARN_MODELID_OVERFLOW, id);
        return;
    }

    if(size < model->meshes.size())
    {
        model->collision_map.resize(size);
    }
}

void lua_SetModelCollisionMap(world::ModelId id, size_t arg, size_t val)
{
    /// engine_world.skeletal_models[id] != engine::engine_world.getModelByID(lua_tointeger(lua, 1));
    world::SkeletalModel* model = engine::engine_world.getModelByID(id);
    if(model == nullptr)
    {
        Console::instance().warning(SYSWARN_MODELID_OVERFLOW, id);
        return;
    }

    if(   arg < model->collision_map.size()
       && val < model->meshes.size())
    {
        model->collision_map[arg] = val;
    }
}

void lua_EnableEntity(world::ObjectId id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);
    if(ent)
        ent->enable();
}

void lua_DisableEntity(world::ObjectId id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);
    if(ent)
        ent->disable();
}

void lua_SetEntityCollision(world::ObjectId id, bool val)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);
    if(ent)
    {
        if(val)
            ent->m_skeleton.enableCollision();
        else
            ent->m_skeleton.disableCollision();
    }
}

void lua_SetEntityCollisionFlags(world::ObjectId id, lua::Value ctype, lua::Value enableCollision, lua::Value cshape)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);
    if(ent == nullptr)
        return;

    if(ctype.is<lua::Integer>() && enableCollision.is<lua::Boolean>())
    {
        ent->setCollisionType( static_cast<world::CollisionType>(ctype.toInt()) );
        if(enableCollision.to<lua::Boolean>())
        {
            ent->m_skeleton.enableCollision();
        }
        else
        {
            ent->m_skeleton.disableCollision();
        }
    }
    if(cshape.is<lua::Integer>())
    {
        ent->setCollisionShape( static_cast<world::CollisionShape>(cshape.toInt()) );
    }
}

lua::Any lua_GetEntitySectorFlags(world::ObjectId id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent != nullptr && ent->m_currentSector)
    {
        return ent->m_currentSector->flags;
    }
    return {};
}

lua::Any lua_GetEntitySectorIndex(world::ObjectId id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent != nullptr && ent->m_currentSector)
    {
        return ent->m_currentSector->trig_index;
    }
    return {};
}

lua::Any lua_GetEntitySectorMaterial(world::ObjectId id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent != nullptr && ent->m_currentSector)
    {
        return ent->m_currentSector->material;
    }
    return {};
}

lua::Any lua_GetEntitySubstanceState(world::ObjectId id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent != nullptr)
    {
        return static_cast<int>(ent->getSubstanceState());
    }
    return {};
}

lua::Any lua_SameRoom(world::ObjectId id1, world::ObjectId id2)
{
    std::shared_ptr<world::Entity> ent1 = engine::engine_world.getEntityByID(id1);
    std::shared_ptr<world::Entity> ent2 = engine::engine_world.getEntityByID(id2);

    if(ent1 && ent2)
    {
        return ent1->getRoom() == ent2->getRoom();
    }

    return {};
}

lua::Any lua_SameSector(world::ObjectId id1, world::ObjectId id2)
{
    std::shared_ptr<world::Entity> ent1 = engine::engine_world.getEntityByID(id1);
    std::shared_ptr<world::Entity> ent2 = engine::engine_world.getEntityByID(id2);

    if(ent1 && ent2 && ent1->m_currentSector && ent2->m_currentSector)
    {
        return ent1->m_currentSector->trig_index == ent2->m_currentSector->trig_index;
    }

    return {};
}

lua::Any lua_NewSector(world::ObjectId id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent != nullptr)
    {
        return ent->m_currentSector == ent->m_lastSector;
    }
    return {};
}

std::tuple<float, float, float> lua_GetGravity()
{
    btVector3 g = engine::bt_engine_dynamicsWorld->getGravity();
    return std::make_tuple( g[0], g[1], g[2] );
}

void lua_SetGravity(float x, lua::Value y, lua::Value z)                                             // function to be exported to Lua
{
    btVector3 g(x, y.is<btScalar>() ? y.to<btScalar>() : 0, z.is<btScalar>() ? z.to<btScalar>() : 0);
    engine::bt_engine_dynamicsWorld->setGravity(g);
    Console::instance().printf("gravity = (%.3f, %.3f, %.3f)", g[0], g[1], g[2]);
}

bool lua_DropEntity(world::ObjectId id, float time, lua::Value only_room)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);
    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return false;
    }

    glm::vec3 move = ent->applyGravity(util::MilliSeconds(static_cast<int>(time*1000)));

    engine::BtEngineClosestRayResultCallback cb(ent.get());
    glm::vec3 from = glm::vec3(ent->m_transform * glm::vec4(ent->m_skeleton.getBoundingBox().getCenter(), 1.0f));
    from[2] = ent->m_transform[3][2];
    glm::vec3 to = from + move;
    //to[2] -= (ent->m_bf.bb_max[2] - ent->m_bf.bb_min[2]);
    engine::bt_engine_dynamicsWorld->rayTest(util::convert(from), util::convert(to), cb);

    if(cb.hasHit())
    {
        world::Object* object = static_cast<world::Object*>(cb.m_collisionObject->getUserPointer());

        if(!only_room.is<lua::Boolean>() || !only_room.to<bool>() || (only_room.to<bool>() && dynamic_cast<world::Room*>(object)))
        {
            move = glm::mix(from, to, cb.m_closestHitFraction);
            ent->m_transform[3][2] = move[2];

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
        ent->m_transform[3] += glm::vec4(move, 0);
        ent->updateRigidBody(true);
        return false;
    }
}

lua::Any lua_GetEntityModelID(world::ObjectId id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);
    if(ent == nullptr)
        return {};

    if(ent->m_skeleton.getModel())
    {
        return ent->m_skeleton.getModel()->id;
    }
    return {};
}

lua::Any lua_GetEntityActivationOffset(world::ObjectId id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);
    if(ent == nullptr)
        return {};

    return std::make_tuple(
        ent->m_activationOffset[0],
        ent->m_activationOffset[1],
        ent->m_activationOffset[2],
        ent->m_activationRadius);
}

void lua_SetEntityActivationOffset(world::ObjectId id, float x, float y, float z, lua::Value r)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);
    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return;
    }

    ent->m_activationOffset = { x,y,z };
    if(r.is<lua::Number>())
        ent->m_activationRadius = r.to<glm::float_t>();
}

lua::Any lua_GetCharacterParam(world::ObjectId id, int parameter)
{
    std::shared_ptr<world::Character> ent = engine::engine_world.getCharacterByID(id);

    if(parameter >= world::PARAM_SENTINEL)
    {
        Console::instance().warning(SYSWARN_WRONG_OPTION_INDEX, world::PARAM_SENTINEL);
        return {};
    }

    if(ent)
    {
        return ent->getParam(parameter);
    }
    else
    {
        Console::instance().warning(SYSWARN_NO_CHARACTER, id);
        return {};
    }
}

void lua_SetCharacterParam(world::ObjectId id, int parameter, float value, lua::Value max_value)
{
    std::shared_ptr<world::Character> ent = engine::engine_world.getCharacterByID(id);

    if(parameter >= world::PARAM_SENTINEL)
    {
        Console::instance().warning(SYSWARN_WRONG_OPTION_INDEX, world::PARAM_SENTINEL);
        return;
    }

    if(!ent)
    {
        Console::instance().warning(SYSWARN_NO_CHARACTER, id);
        return;
    }
    
    if(!max_value.is<lua::Number>())
    {
        ent->setParam(parameter, value);
    }
    else
    {
        ent->m_parameters.param[parameter] = value;
        ent->m_parameters.maximum[parameter] = max_value.toFloat();
    }
}

lua::Any lua_GetCharacterCombatMode(world::ObjectId id)
{
    std::shared_ptr<world::Character> ent = engine::engine_world.getCharacterByID(id);

    if(ent)
    {
        return static_cast<int>(ent->m_currentWeaponState);
    }

    return {};
}

void lua_ChangeCharacterParam(world::ObjectId id, int parameter, lua::Value value)
{
    std::shared_ptr<world::Character> ent = engine::engine_world.getCharacterByID(id);

    if(parameter >= world::PARAM_SENTINEL)
    {
        Console::instance().warning(SYSWARN_WRONG_OPTION_INDEX, world::PARAM_SENTINEL);
        return;
    }

    if(ent && value.is<float>())
    {
        if(value.is<float>())
            ent->changeParam(parameter, value.to<float>());
    }
    else
    {
        Console::instance().warning(SYSWARN_NO_CHARACTER, id);
    }
}

void lua_AddCharacterHair(world::ObjectId ent_id, int setup_index)
{
    std::shared_ptr<world::Character> ent = engine::engine_world.getCharacterByID(ent_id);

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

void lua_ResetCharacterHair(world::ObjectId ent_id)
{
    std::shared_ptr<world::Character> ent = engine::engine_world.getCharacterByID(ent_id);

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

void lua_AddEntityRagdoll(world::ObjectId ent_id, int setup_index)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(ent_id);

    if(ent)
    {
        world::RagdollSetup ragdoll_setup;

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

void lua_RemoveEntityRagdoll(world::ObjectId ent_id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(ent_id);

    if(ent)
    {
        if(!ent->m_skeleton.getModel()->bt_joints.empty())
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

lua::Any lua_GetActionState(int act)
{
    if(act < 0 || act >= engine::ACT_LASTINDEX)
    {
        Console::instance().warning(SYSWARN_WRONG_ACTION_NUMBER);
        return {};
    }
    else
    {
        return engine::control_mapper.action_map[act].state;
    }
}

lua::Any lua_GetActionChange(int act)
{
    if(act < 0 || act >= engine::ACT_LASTINDEX)
    {
        Console::instance().warning(SYSWARN_WRONG_ACTION_NUMBER);
        return {};
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
        return;
    }
    engine::control_mapper.action_map[act].primary = primary;
    if(secondary.is<lua::Integer>())
        engine::control_mapper.action_map[act].secondary = secondary.to<int>();
}

void lua_AddFont(int index, const char* path, int size)
{
    if(!gui::FontManager::instance->addFont(static_cast<gui::FontType>(index), size, path))
    {
        Console::instance().warning(SYSWARN_CANT_CREATE_FONT, gui::FontManager::instance->getFontCount(), gui::MaxFonts);
    }
}

void lua_AddFontStyle(int style_index,
                      float color_R, float color_G, float color_B, float color_A,
                      bool shadowed, bool fading, bool rect, float rect_border,
                      float rect_R, float rect_G, float rect_B, float rect_A,
                      bool hide)
{
    if(!gui::FontManager::instance->addFontStyle(static_cast<gui::FontStyle>(style_index),
                                       {color_R, color_G, color_B, color_A},
                                       shadowed, fading,
                                       rect, rect_border, {rect_R, rect_G, rect_B, rect_A},
                                       hide))
    {
        Console::instance().warning(SYSWARN_CANT_CREATE_STYLE, gui::FontManager::instance->getFontStyleCount(), gui::FontStyle::Sentinel);
    }
}

void lua_DeleteFont(int fontindex)
{
    if(!gui::FontManager::instance->removeFont(static_cast<gui::FontType>(fontindex)))
    {
        Console::instance().warning(SYSWARN_CANT_REMOVE_FONT);
    }
}

void lua_DeleteFontStyle(int styleindex)
{
    if(!gui::FontManager::instance->removeFontStyle(static_cast<gui::FontStyle>(styleindex)))
    {
        Console::instance().warning(SYSWARN_CANT_REMOVE_STYLE);
    }
}

lua::Any lua_AddItem(world::ObjectId entity_id, int item_id, lua::Value count)
{
    std::shared_ptr<world::Character> ent = engine::engine_world.getCharacterByID(entity_id);

    if(ent)
    {
        return ent->addItem(item_id, count.is<int32_t>() ? count.to<int32_t>() : -1);
    }
    else
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, entity_id);
        return {};
    }
}

lua::Any lua_RemoveItem(world::ObjectId entity_id, int item_id, int count)
{
    std::shared_ptr<world::Character> ent = engine::engine_world.getCharacterByID(entity_id);

    if(ent)
    {
        return ent->removeItem(item_id, count);
    }
    else
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, entity_id);
        return {};
    }
}

void lua_RemoveAllItems(world::ObjectId entity_id)
{
    std::shared_ptr<world::Character> ent = engine::engine_world.getCharacterByID(entity_id);

    if(ent)
    {
        ent->removeAllItems();
    }
    else
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, entity_id);
    }
}

lua::Any lua_GetItemsCount(world::ObjectId entity_id, int item_id)
{
    std::shared_ptr<world::Character> ent = engine::engine_world.getCharacterByID(entity_id);

    if(ent)
    {
        return ent->getItemsCount(item_id);
    }
    else
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, entity_id);
        return {};
    }
}

void lua_CreateBaseItem(int item_id, int model_id, int world_model_id, int type, int count, const char* name)
{
    engine::engine_world.createItem(item_id, model_id, world_model_id, static_cast<MenuItemType>(type), count, name ? name : std::string());
}

void lua_DeleteBaseItem(int id)
{
    engine::engine_world.deleteItem(id);
}

void lua_PrintItems(world::ObjectId entity_id)
{
    std::shared_ptr<world::Character> ent = engine::engine_world.getCharacterByID(entity_id);
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

void lua_SetStateChangeRange(world::ModelId id, int anim, int state, int dispatch, int frame_low, int frame_high, lua::Value next_anim, lua::Value next_frame)
{
    world::SkeletalModel* model = engine::engine_world.getModelByID(id);

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

    world::animation::Animation* af = &model->animations[anim];
    world::animation::StateChange* stc = af->findStateChangeByID(static_cast<world::LaraState>(state));
    if(stc != nullptr)
    {
        if(dispatch >= 0 && dispatch < static_cast<int>(stc->dispatches.size()))
        {
            stc->dispatches[dispatch].start = frame_low;
            stc->dispatches[dispatch].end = frame_high;
            if(!next_anim.is<lua::Nil>() && !next_frame.is<lua::Nil>())
            {
                stc->dispatches[dispatch].next.animation = next_anim.to<world::animation::AnimationId>();
                stc->dispatches[dispatch].next.frame = next_frame.to<size_t>();
            }
        }
        else
        {
            Console::instance().warning(SYSWARN_WRONG_DISPATCH_NUMBER, dispatch);
        }
    }
}

void lua_SetAnimEndCommands(world::ModelId id, int anim, lua::Value table)
{
    world::SkeletalModel* model = engine::engine_world.getModelByID(id);
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

    if(!table.is<lua::Table>())
    {
        Console::instance().warning(SYSWARN_WRONG_ARGS, "entid, anim, {{cmd,p1,p2,p3},{...}}");
        return;
    }

    model->animations[anim].finalAnimCommands.clear();

    for(int i=1; table[i].is<lua::Table>(); i++)
    {
        if(   table[i][1].is<lua::Number>()
           && table[i][2].is<lua::Number>()
           && table[i][3].is<lua::Number>()
           && table[i][4].is<lua::Number>())
        {
            model->animations[anim].finalAnimCommands.push_back({
                                                                    static_cast<world::animation::AnimCommandOpcode>(table[i][1].to<int>()),
                                                                    table[i][2].to<int>(),
                                                                    table[i][3].to<int>(),
                                                                    table[i][4].to<int>()});
        }
        else
        {
            Console::instance().warning(SYSWARN_WRONG_ARGS, "entid, anim, {{cmd,p1,p2,p3},{...}}");
            break;
        }
    }
}

lua::Any lua_SpawnEntity(world::ModelId model_id, float x, float y, float z, float ax, float ay, float az, world::ObjectId room_id, lua::Value ov_id)
{
    glm::vec3 position{ x,y,z }, ang{ ax,ay,az };

    boost::optional<world::ObjectId> object;
    if(ov_id.is<world::ObjectId>())
        object = ov_id.to<world::ObjectId>();
    auto id = engine::engine_world.spawnEntity(model_id, room_id, &position, &ang, object);
    if(!id)
        return {};
    else
        return *id;
}

bool lua_DeleteEntity(world::ObjectId id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);
    if(!ent)
        return false;

    if(ent->getRoom())
        ent->getRoom()->removeEntity(ent.get());
    engine::engine_world.deleteEntity(id);
    return true;
}

// Moveable script control section

lua::Any lua_GetEntityVector(world::ObjectId id1, world::ObjectId id2)
{
    std::shared_ptr<world::Entity> e1 = engine::engine_world.getEntityByID(id1);
    if(e1 == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id1);
        return {};
    }
    std::shared_ptr<world::Entity> e2 = engine::engine_world.getEntityByID(id2);
    if(e2 == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id2);
        return {};
    }

    return std::make_tuple(
        e2->m_transform[3][0] - e1->m_transform[3][0],
        e2->m_transform[3][1] - e1->m_transform[3][1],
        e2->m_transform[3][2] - e1->m_transform[3][2]
    );
}

lua::Any lua_GetEntityDistance(world::ObjectId id1, world::ObjectId id2)
{
    std::shared_ptr<world::Entity> e1 = engine::engine_world.getEntityByID(id1);
    if(!e1)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id1);
        return {};
    }
    std::shared_ptr<world::Entity> e2 = engine::engine_world.getEntityByID(id2);
    if(!e2)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id2);
        return {};
    }

    return e1->findDistance(*e2);
}

lua::Any lua_GetEntityDirDot(world::ObjectId id1, world::ObjectId id2)
{
    std::shared_ptr<world::Entity> e1 = engine::engine_world.getEntityByID(id1);
    if(!e1)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id2);
        return {};
    }
    std::shared_ptr<world::Entity> e2 = engine::engine_world.getEntityByID(id2);
    if(!e2)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id2);
        return {};
    }

    return glm::dot(e1->m_transform[1], e2->m_transform[1]);
}

bool lua_IsInRoom(world::ObjectId id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent && ent->getRoom())
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

lua::Any lua_GetEntityPosition(world::ObjectId id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);
    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return {};
    }

    return std::make_tuple
    (
        ent->m_transform[3][0],
        ent->m_transform[3][1],
        ent->m_transform[3][2],
        ent->m_angles[0],
        ent->m_angles[1],
        ent->m_angles[2],
        ent->getRoom()->getId()
    );
}

lua::Any lua_GetEntityAngles(world::ObjectId id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return {};
    }

    return std::make_tuple(
        ent->m_angles[0],
        ent->m_angles[1],
        ent->m_angles[2]
    );
}

lua::Any lua_GetEntityScaling(int id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(!ent)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return {};
    }

    return std::make_tuple(
        ent->m_scaling[0],
        ent->m_scaling[1],
        ent->m_scaling[2]
    );
}

lua::Any lua_SimilarSector(world::ObjectId id, float dx, float dy, float dz, bool ignore_doors, lua::Value ceiling)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(!ent)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return {};
    }

    glm::vec3 next_pos = glm::vec3(ent->m_transform[3] + (dx * ent->m_transform[0] + dy * ent->m_transform[1] + dz * ent->m_transform[2]));

    world::RoomSector* curr_sector = ent->getRoom()->getSectorRaw(glm::vec3(ent->m_transform[3]));
    world::RoomSector* next_sector = ent->getRoom()->getSectorRaw(next_pos);

    curr_sector = curr_sector->checkPortalPointer();
    next_sector = next_sector->checkPortalPointer();

    if(ceiling.is<lua::Boolean>() && ceiling.toBool())
    {
        return curr_sector->similarCeiling(next_sector, ignore_doors);
    }
    else
    {
        return curr_sector->similarFloor(next_sector, ignore_doors);
    }
}

lua::Any lua_GetSectorHeight(world::ObjectId id, lua::Value ceiling, lua::Value dx, lua::Value dy, lua::Value dz)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(!ent)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return {};
    }

    auto position = ent->m_transform[3];

    if(dx.is<lua::Number>() && dy.is<lua::Number>() && dz.is<lua::Number>())
        position += dx.toFloat() * ent->m_transform[0] + dy.toFloat() * ent->m_transform[1] + dz.toFloat() * ent->m_transform[2];

    world::RoomSector* curr_sector = ent->getRoom()->getSectorRaw(glm::vec3(position));
    curr_sector = curr_sector->checkPortalPointer();
    glm::vec3 point = ceiling.is<lua::Boolean>() && ceiling.to<bool>()
        ? curr_sector->getCeilingPoint()
        : curr_sector->getFloorPoint();

    return point[2];
}

void lua_SetEntityPosition(world::ObjectId id, float x, float y, float z, lua::Value ax, lua::Value ay, lua::Value az)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);
    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return;
    }
    ent->m_transform[3] = { x,y,z,0 };
    if(ax.is<lua::Number>() && ay.is<lua::Number>() && az.is<lua::Number>())
        ent->m_angles = { ax.toNumber(), ay.toNumber(), az.toNumber() };
    ent->updateTransform();
    ent->updatePlatformPreStep();
}

void lua_SetEntityAngles(world::ObjectId id, float x, lua::Value y, lua::Value z)
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
            ent->m_angles = { x, y.toNumber(), z.toNumber() };
        ent->updateTransform();
    }
}

void lua_SetEntityScaling(world::ObjectId id, float x, float y, float z)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(!ent)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
    }
    else
    {
        ent->m_scaling = { x,y,z };

        for(const world::animation::Bone& bone : ent->m_skeleton.getBones())
        {
            if(!bone.bt_body)
                continue;

            engine::bt_engine_dynamicsWorld->removeRigidBody(bone.bt_body.get());
            bone.bt_body->getCollisionShape()->setLocalScaling(util::convert(ent->m_scaling));
            engine::bt_engine_dynamicsWorld->addRigidBody(bone.bt_body.get());

            bone.bt_body->activate();
        }

        ent->updateRigidBody(true);
    }
}

void lua_MoveEntityGlobal(world::ObjectId id, float x, float y, float z)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);
    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return;
    }
    ent->m_transform[3][0] += x;
    ent->m_transform[3][1] += y;
    ent->m_transform[3][2] += z;

    ent->updateRigidBody(true);
    ent->ghostUpdate();
}

void lua_MoveEntityLocal(world::ObjectId id, float dx, float dy, float dz)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(!ent)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return;
    }

    ent->m_transform = glm::translate(ent->m_transform, glm::vec3(dx, dy, dz));

    ent->updateRigidBody(true);
    ent->ghostUpdate();
}

void lua_MoveEntityToSink(world::ObjectId id, int sink_index)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(sink_index < 0 || sink_index > static_cast<int>(engine::engine_world.cameras_sinks.size()))
        return;
    world::StatCameraSink* sink = &engine::engine_world.cameras_sinks[sink_index];

    glm::vec3 ent_pos;  ent_pos[0] = ent->m_transform[3][0];
    ent_pos[1] = ent->m_transform[3][1];
    ent_pos[2] = ent->m_transform[3][2];

    glm::vec3 sink_pos = sink->position + glm::vec3(0, 0, 256.0f);

    BOOST_ASSERT(ent->m_currentSector != nullptr);
    world::RoomSector* ls = ent->m_currentSector->getLowestSector();
    BOOST_ASSERT(ls != nullptr);
    world::RoomSector* hs = ent->m_currentSector->getHighestSector();
    BOOST_ASSERT(hs != nullptr);
    if(sink_pos[2] > hs->ceiling || sink_pos[2] < ls->floor)
    {
        sink_pos[2] = ent_pos[2];
    }

    glm::float_t dist = glm::distance(ent_pos, sink_pos);
    if(util::fuzzyZero(dist))
        dist = 1.0; // Prevents division by zero.

    glm::vec3 speed = (sink_pos - ent_pos) / dist * (sink->room_or_strength * 1.5f);

    ent->m_transform[3] += glm::vec4(speed, 0.0f);

    ent->updateRigidBody(true);
    ent->ghostUpdate();
}

void lua_MoveEntityToEntity(world::ObjectId id1, world::ObjectId id2, float speed_mult, lua::Value ignore_z)
{
    std::shared_ptr<world::Entity> ent1 = engine::engine_world.getEntityByID(id1);
    std::shared_ptr<world::Entity> ent2 = engine::engine_world.getEntityByID(id2);

    glm::vec3 ent1_pos; ent1_pos[0] = ent1->m_transform[3][0];
    ent1_pos[1] = ent1->m_transform[3][1];
    ent1_pos[2] = ent1->m_transform[3][2];

    glm::vec3 ent2_pos; ent2_pos[0] = ent2->m_transform[3][0];
    ent2_pos[1] = ent2->m_transform[3][1];
    ent2_pos[2] = ent2->m_transform[3][2];

    glm::float_t dist = glm::distance(ent1_pos, ent2_pos);
    if(util::fuzzyZero(dist))
        dist = 1.0; // Prevents division by zero.

    glm::vec3 speed = (ent2_pos - ent1_pos) / dist * speed_mult; // FIXME!

    ent1->m_transform[3][0] += speed[0];
    ent1->m_transform[3][1] += speed[1];
    if(!ignore_z.is<lua::Boolean>() || !ignore_z.toBool())
        ent1->m_transform[3][2] += speed[2];
    ent1->updatePlatformPreStep();
    ent1->updateRigidBody(true);
}

void lua_RotateEntity(world::ObjectId id, float rx, lua::Value ry, lua::Value rz)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
    }
    else
    {
        if(!ry.is<lua::Number>() || !rz.is<lua::Number>())
            ent->m_angles += glm::vec3{rx, 0, 0};
        else
            ent->m_angles += glm::vec3{rx, ry.toNumber(), rz.toNumber()};

        ent->updateTransform();
        ent->updateRigidBody(true);
    }
}

void lua_RotateEntityToEntity(world::ObjectId id1, world::ObjectId id2, int axis, lua::Value speed_, lua::Value /*smooth_*/, lua::Value add_angle_)
{
    std::shared_ptr<world::Entity> ent1 = engine::engine_world.getEntityByID(id1);
    std::shared_ptr<world::Entity> ent2 = engine::engine_world.getEntityByID(id2);

    if(!ent1 || !ent2)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, !ent1 ? id1 : id2);
    }
    else if(axis < 0 || axis > 2)
    {
        Console::instance().warning(SYSWARN_WRONG_AXIS, !ent1 ? id1 : id2);
    }
    else
    {
        glm::vec3 ent1_pos( ent1->m_transform[3] );
        glm::vec3 ent2_pos( ent2->m_transform[3] );
        glm::vec3 facing(ent1_pos - ent2_pos);

        glm::float_t *targ_angle = nullptr;
        glm::float_t  theta = 0;

        switch(axis)
        {
            case 0:
                targ_angle = &ent1->m_angles.x;
                theta      = glm::atan(-facing.x, facing.y);
                break;
            case 1:
                targ_angle = &ent1->m_angles.y;
                theta      = glm::atan(facing.z, facing.y);
                break;
            case 2:
                targ_angle = &ent1->m_angles.z;
                theta      = glm::atan(facing.x, facing.z);
                break;
        }

        BOOST_ASSERT(targ_angle != nullptr);

        theta = glm::degrees(theta);
        if(add_angle_.is<lua::Number>())
            theta += add_angle_.to<glm::float_t>();

        glm::float_t delta = *targ_angle - theta;

        if(ceil(delta) != 180.0)
        {
            if(speed_.is<lua::Number>())
            {
                glm::float_t speed = speed_.to<glm::float_t>();

                if(glm::abs(delta) > speed)
                {
                    // Solve ~0-360 rotation cases.

                    if(abs(delta) > 180.0)
                    {
                        if(*targ_angle > theta)
                        {
                            delta = -(360.0f - *targ_angle + theta);
                        }
                        else
                        {
                            delta = 360.0f - theta + *targ_angle;
                        }
                    }

                    if(delta > 180.0)
                    {
                        *targ_angle = theta + 180.0f;
                    }
                    else if(delta >= 0.0 && delta < 180.0)
                    {
                        *targ_angle += speed;
                    }
                    else
                    {
                        *targ_angle -= speed;
                    }
                }

                if(glm::abs(delta) + speed >= 180.0)
                    *targ_angle = floor(theta) + 180.0f;
            }
            else
            {
                *targ_angle = theta + 180.0f;
            }
        }

        ent1->updateTransform();
        ent1->updateRigidBody(true);
    }
}



lua::Any lua_GetEntityOrientation(world::ObjectId id1, world::ObjectId id2, lua::Value add_angle_)
{
    std::shared_ptr<world::Entity> ent1 = engine::engine_world.getEntityByID(id1);
    std::shared_ptr<world::Entity> ent2 = engine::engine_world.getEntityByID(id2);

    if(!ent1 || !ent2)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, !ent1 ? id1 : id2);
        return {};
    }

    glm::vec3 ent1_pos(ent1->m_transform[3]);
    glm::vec3 ent2_pos(ent2->m_transform[3]);
    glm::vec3 facing(ent2_pos - ent1_pos);

    glm::float_t theta = glm::degrees(glm::atan(-facing.x, facing.y));
    if(add_angle_.is<lua::Number>())
        theta += add_angle_.to<glm::float_t>();

    return util::wrapAngle(ent2->m_angles[0] - theta);
}


lua::Any lua_GetEntitySpeed(world::ObjectId id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return {};
    }

    return std::make_tuple(
        ent->m_speed[0],
        ent->m_speed[1],
        ent->m_speed[2]
    );
}

lua::Any lua_GetEntitySpeedLinear(world::ObjectId id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return {};
    }

    return glm::length(ent->m_speed);
}

void lua_SetEntitySpeed(world::ObjectId id, float vx, lua::Value vy, lua::Value vz)
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
            ent->m_speed = { vx, vy.toNumber(), vz.toNumber() };
        ent->updateCurrentSpeed();
    }
}

void lua_SetEntityAnim(world::ObjectId id, world::animation::AnimationId anim, lua::Value frame)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return;
    }

    if(frame.is<int>())
        ent->setAnimation(anim, frame.to<int>());
    else
        ent->setAnimation(anim);
}

void lua_SetEntityAnimFlag(world::ObjectId id, int mode)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return;
    }

    ent->m_skeleton.setAnimationMode(static_cast<world::animation::AnimationMode>(mode));
}

void lua_SetEntityBodyPartFlag(world::ObjectId id, int bone_id, int body_part_flag)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return;
    }

    if(bone_id < 0 || bone_id >= static_cast<int>(ent->m_skeleton.getBoneCount()))
    {
        Console::instance().warning(SYSWARN_WRONG_OPTION_INDEX, bone_id);
        return;
    }

    ent->m_skeleton.setBodyPartFlag(bone_id, body_part_flag);
}

void lua_SetModelBodyPartFlag(world::ModelId id, int bone_id, int body_part_flag)
{
    world::SkeletalModel* model = engine::engine_world.getModelByID(id);

    if(model == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_SKELETAL_MODEL, id);
        return;
    }

    if(bone_id < 0 || static_cast<size_t>(bone_id) >= model->meshes.size())
    {
        Console::instance().warning(SYSWARN_WRONG_OPTION_INDEX, bone_id);
        return;
    }

    model->meshes[bone_id].body_part = body_part_flag;
}

lua::Any lua_GetEntityAnim(world::ObjectId id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return {};
    }

    return std::make_tuple(
        ent->m_skeleton.getCurrentAnimation(),
        ent->m_skeleton.getCurrentFrame(),
        ent->m_skeleton.getModel()->animations[ent->m_skeleton.getCurrentAnimation()].getFrameDuration()
    );
}

bool lua_CanTriggerEntity(world::ObjectId id1, world::ObjectId id2, lua::Value rv, lua::Value ofsX, lua::Value ofsY, lua::Value ofsZ)
{
    std::shared_ptr<world::Character> e1 = engine::engine_world.getCharacterByID(id1);
    if(!e1 || !e1->m_command.action)
    {
        return false;
    }

    std::shared_ptr<world::Entity> e2 = engine::engine_world.getEntityByID(id2);
    if(!e2 || e1 == e2)
    {
        return false;
    }

    glm::float_t r;
    if(!rv.is<lua::Number>() || rv.toNumber()<0)
        r = e2->m_activationRadius;
    else
        r = rv.to<glm::float_t>();

    glm::vec3 offset;
    if(ofsX.is<lua::Number>() && ofsY.is<lua::Number>() && ofsZ.is<lua::Number>())
        offset = { ofsX.toNumber(), ofsY.toNumber(), ofsZ.toNumber() };
    else
        offset = e2->m_activationOffset;

    glm::vec3 position = glm::vec3(e2->m_transform * glm::vec4(offset, 1.0f));
    if(glm::dot(e1->m_transform[1], e2->m_transform[1]) > 0.75 &&
       glm::distance(glm::vec3(e1->m_transform[3]), position) < r)
    {
        return true;
    }

    return false;
}

lua::Any lua_GetEntityVisibility(world::ObjectId id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return {};
    }

    return ent->m_visible;
}

void lua_SetEntityVisibility(world::ObjectId id, bool value)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return;
    }

    ent->m_visible = value;
}

lua::Any lua_GetEntityEnability(world::ObjectId id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return {};
    }

    return ent->m_enabled;
}

lua::Any lua_GetEntityActivity(world::ObjectId id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return {};
    }

    return ent->m_active;
}

void lua_SetEntityActivity(world::ObjectId id, bool value)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return;
    }

    ent->m_active = value;
}

lua::Any lua_GetEntityTriggerLayout(world::ObjectId id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);
    if(ent == nullptr)
        return {};   // No entity found - return.

    return std::make_tuple(
        ent->m_triggerLayout & ENTITY_TLAYOUT_MASK,   // mask
        (ent->m_triggerLayout & ENTITY_TLAYOUT_EVENT) != 0, // event
        (ent->m_triggerLayout & ENTITY_TLAYOUT_LOCK) != 0   // lock
    );
}

void lua_SetEntityTriggerLayout(world::ObjectId id, int mask, lua::Value eventOrLayout, lua::Value lock)
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

void lua_SetEntityLock(world::ObjectId id, bool lock)
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

lua::Any lua_GetEntityLock(world::ObjectId id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);
    if(ent != nullptr)
    {
        return (ent->m_triggerLayout & ENTITY_TLAYOUT_LOCK) >> 6 != 0;      // lock
    }
    return {};
}

void lua_SetEntityEvent(world::ObjectId id, bool event)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);
    if(ent != nullptr)
    {
        uint8_t trigger_layout = ent->m_triggerLayout;
        trigger_layout &= ~static_cast<uint8_t>((ENTITY_TLAYOUT_EVENT)); trigger_layout ^= event << 5;   // event - 00100000
        ent->m_triggerLayout = trigger_layout;
    }
}

lua::Any lua_GetEntityEvent(world::ObjectId id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);
    if(ent != nullptr)
    {
        return (ent->m_triggerLayout & ENTITY_TLAYOUT_EVENT) >> 5 != 0;    // event
    }
    return {};
}

lua::Any lua_GetEntityMask(world::ObjectId id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);
    if(ent != nullptr)
    {
        return ent->m_triggerLayout & ENTITY_TLAYOUT_MASK;          // mask
    }
    return {};
}

void lua_SetEntityMask(world::ObjectId id, int mask)
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

lua::Any lua_GetEntitySectorStatus(world::ObjectId id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);
    if(ent != nullptr)
    {
        return (ent->m_triggerLayout & ENTITY_TLAYOUT_SSTATUS) >> 7 != 0;
    }
    return {};
}

void lua_SetEntitySectorStatus(world::ObjectId id, bool status)
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

lua::Any lua_GetEntityOCB(world::ObjectId id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);
    if(ent == nullptr)
        return {};   // No entity found - return.

    return ent->m_OCB;
}

void lua_SetEntityOCB(world::ObjectId id, int ocb)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);
    if(ent == nullptr)
        return;   // No entity found - return.

    ent->m_OCB = ocb;
}

lua::Any lua_GetEntityFlags(world::ObjectId id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return {};
    }

    return std::make_tuple(
        ent->m_active,
        ent->m_enabled,
        ent->m_visible,
        ent->m_typeFlags,
        ent->m_callbackFlags
    );
}

void lua_SetEntityFlags(world::ObjectId id, bool active, bool enabled, bool visible, int typeFlags, lua::Value cbFlags)
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
    if(cbFlags.is<uint32_t>())
        ent->m_callbackFlags = cbFlags.to<uint32_t>();
}

lua::Any lua_GetEntityTypeFlag(world::ObjectId id, lua::Value flag)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return {};
    }

    if(flag.is<lua::Integer>())
        return ent->m_typeFlags & flag.toInt();
    else
        return ent->m_typeFlags;
}

void lua_SetEntityTypeFlag(world::ObjectId id, int type_flag, lua::Value value)
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

    if(value.toBool())
    {
        ent->m_typeFlags |= type_flag;
    }
    else
    {
        ent->m_typeFlags &= ~type_flag;
    }
}

lua::Any lua_GetEntityStateFlag(world::ObjectId id, const char* whichCstr)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return {};
    }

    std::string which = whichCstr;
    if(which == "active")
        return ent->m_active;
    else if(which == "enabled")
        return ent->m_enabled;
    else if(which == "visible")
        return ent->m_visible;
    else
        return {};
}

void lua_SetEntityStateFlag(world::ObjectId id, const char* whichCstr, lua::Value value)
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
        *flag = value.toBool();
    else
        *flag = !*flag;
}

lua::Any lua_GetEntityCallbackFlag(world::ObjectId id, lua::Value flag)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return {};
    }

    if(!flag.is<lua::Integer>())
        return ent->m_callbackFlags;
    else
        return ent->m_callbackFlags & flag.toInt();
}

void lua_SetEntityCallbackFlag(world::ObjectId id, int flag, lua::Value value)
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

    if(value.toBool())
    {
        ent->m_callbackFlags |= flag;
    }
    else
    {
        ent->m_callbackFlags &= ~flag;
    }
}

lua::Any lua_GetEntityTimer(world::ObjectId id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);
    if(ent == nullptr)
        return {};   // No entity found - return.

    return ent->m_timer;
}

void lua_SetEntityTimer(world::ObjectId id, float timer)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);
    if(ent == nullptr)
        return;   // No entity found - return.

    ent->m_timer = timer;
}

lua::Any lua_GetEntityMoveType(world::ObjectId id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return {};
    }

    return static_cast<int>(ent->m_moveType);
}

void lua_SetEntityMoveType(world::ObjectId id, int type)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent == nullptr)
        return;
    ent->m_moveType = static_cast<world::MoveType>(type);
}

lua::Any lua_GetEntityResponse(world::ObjectId id, int response)
{
    std::shared_ptr<world::Character> ent = engine::engine_world.getCharacterByID(id);

    if(ent)
    {
        switch(response)
        {
            case 0: return ent->m_response.killed ? 1 : 0;
            case 1: return ent->m_response.vertical_collide;
            case 2: return ent->m_response.horizontal_collide;
            case 3: return static_cast<int>(ent->m_response.slide);
            case 4: return static_cast<int>(ent->m_response.lean);
            default: return {};
        }
    }
    else
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return {};
    }
}

void lua_SetEntityResponse(world::ObjectId id, int response, int value)
{
    std::shared_ptr<world::Character> ent = engine::engine_world.getCharacterByID(id);

    if(ent)
    {
        switch(response)
        {
            case 0:
                ent->m_response.killed = value!=0;
                break;
            case 1:
                ent->m_response.vertical_collide = value;
                break;
            case 2:
                ent->m_response.horizontal_collide = value;
                break;
            case 3:
                ent->m_response.slide = static_cast<world::MovementWalk>(value);
                break;
            case 4:
                ent->m_response.lean = static_cast<world::MovementStrafe>(value);
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

lua::Any lua_GetEntityState(world::ObjectId id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return {};
    }

    return static_cast<int>(ent->m_skeleton.getPreviousState());
}

lua::Any lua_GetEntityModel(world::ObjectId id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return {};
    }

    return ent->m_skeleton.getModel()->id;
}

void lua_SetEntityState(world::ObjectId id, int16_t value, lua::Value next)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return;
    }

    ent->m_skeleton.setCurrentState( static_cast<world::LaraState>(value) );
    if(next.is<lua::Integer>())
        ent->m_skeleton.setPreviousState( static_cast<world::LaraState>(next.toInt()) );
}

void lua_SetEntityRoomMove(world::ObjectId id, size_t room, int moveType, int dirFlag)
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
            ent->setRoom( r.get() );
        }
        else if(ent->getRoom() != r.get())
        {
            if(ent->getRoom() != nullptr)
            {
                ent->getRoom()->removeEntity(ent.get());
            }
            r->addEntity(ent.get());
        }
    }
    ent->updateRoomPos();

    ent->m_moveType = static_cast<world::MoveType>(moveType);
    ent->m_moveDir = static_cast<world::MoveDirection>(dirFlag);
}

lua::Any lua_GetEntityMeshCount(world::ObjectId id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent == nullptr)
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return {};
    }

    return static_cast<int>( ent->m_skeleton.getBoneCount() );
}

void lua_SetEntityMeshswap(world::ObjectId id_dest, world::ModelId id_src)
{
    std::shared_ptr<world::Entity> ent_dest = engine::engine_world.getEntityByID(id_dest);
    world::SkeletalModel* model_src = engine::engine_world.getModelByID(id_src);

    ent_dest->m_skeleton.copyMeshBinding(model_src);
}

void lua_SetModelMeshReplaceFlag(world::ModelId id, size_t bone, int flag)
{
    world::SkeletalModel* sm = engine::engine_world.getModelByID(id);
    if(sm != nullptr)
    {
        if(bone < sm->meshes.size())
        {
            sm->meshes[bone].replace_mesh = flag;
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

void lua_SetModelAnimReplaceFlag(world::ModelId id, size_t bone, bool flag)
{
    world::SkeletalModel* sm = engine::engine_world.getModelByID(id);
    if(sm != nullptr)
    {
        if(bone < sm->meshes.size())
        {
            sm->meshes[bone].replace_anim = flag;
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

void lua_CopyMeshFromModelToModel(world::ModelId id1, world::ModelId id2, size_t bone1, size_t bone2)
{
    world::SkeletalModel* sm1 = engine::engine_world.getModelByID(id1);
    if(sm1 == nullptr)
    {
        Console::instance().warning(SYSWARN_WRONG_MODEL_ID, id1);
        return;
    }

    world::SkeletalModel* sm2 = engine::engine_world.getModelByID(id2);
    if(sm2 == nullptr)
    {
        Console::instance().warning(SYSWARN_WRONG_MODEL_ID, id2);
        return;
    }

    if(bone1 < sm1->meshes.size() && bone2 < sm2->meshes.size())
    {
        sm1->meshes[bone1].mesh_base = sm2->meshes[bone2].mesh_base;
    }
    else
    {
        Console::instance().warning(SYSWARN_WRONG_BONE_NUMBER, bone1);
    }
}

void lua_CreateEntityGhosts(world::ObjectId id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(ent && ent->m_skeleton.getBoneCount() > 0)
    {
        ent->m_skeleton.createGhosts(*ent);
    }
}

void lua_PushEntityBody(world::ObjectId id, size_t body_number, float h_force, float v_force, bool resetFlag)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(   ent
       && body_number < ent->m_skeleton.getBoneCount()
       && ent->m_skeleton.getBones()[body_number].bt_body != nullptr
       && (ent->m_typeFlags & ENTITY_TYPE_DYNAMIC) != 0 )
    {
        glm::float_t t = glm::radians(ent->m_angles[0]);

        glm::float_t ang1 = glm::sin(t);
        glm::float_t ang2 = glm::cos(t);

        btVector3 angle(-ang1 * h_force, ang2 * h_force, v_force);

        if(resetFlag)
            ent->m_skeleton.getBones()[body_number].bt_body->clearForces();

        ent->m_skeleton.getBones()[body_number].bt_body->setLinearVelocity(angle);
        ent->m_skeleton.getBones()[body_number].bt_body->setAngularVelocity(angle / 1024.0);
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

    const world::ObjectId id = static_cast<world::ObjectId>(lua_tointeger(lua, 1));
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    size_t body_number = std::max(lua::Integer(1), lua_tointeger(lua, 2));

    int argn = 3;
    bool dynamic = false;

    if(ent && ent->m_skeleton.getBoneCount() >= body_number)
    {
        for(size_t i = 0; i < body_number; i++)
        {
            btVector3 inertia(0.0, 0.0, 0.0);

            glm::float_t mass = 0;
            if(top >= argn)
                mass = static_cast<glm::float_t>( lua_tonumber(lua, argn) );
            argn++;

            world::animation::Bone& bone = ent->m_skeleton.bone(i);

            if(!bone.bt_body)
                continue;

            engine::bt_engine_dynamicsWorld->removeRigidBody(bone.bt_body.get());

            bone.bt_body->getCollisionShape()->calculateLocalInertia(mass, inertia);

            bone.bt_body->setMassProps(mass, inertia);

            bone.bt_body->updateInertiaTensor();
            bone.bt_body->clearForces();

            bone.bt_body->getCollisionShape()->setLocalScaling(util::convert(ent->m_scaling));

            btVector3 factor = mass > 0.0 ? btVector3(1.0, 1.0, 1.0) : btVector3(0.0, 0.0, 0.0);
            bone.bt_body->setLinearFactor(factor);
            bone.bt_body->setAngularFactor(factor);

            //ent->bt_body[i]->forceActivationState(DISABLE_DEACTIVATION);

            //ent->bt_body[i]->setCcdMotionThreshold(32.0);   // disable tunneling effect
            //ent->bt_body[i]->setCcdSweptSphereRadius(32.0);

            engine::bt_engine_dynamicsWorld->addRigidBody(bone.bt_body.get());

            bone.bt_body->activate();

            //ent->bt_body[i]->getBroadphaseHandle()->m_collisionFilterGroup = 0xFFFF;
            //ent->bt_body[i]->getBroadphaseHandle()->m_collisionFilterMask  = 0xFFFF;

            //ent->self->object_type = OBJECT_ENTITY;
            //ent->bt_body[i]->setUserPointer(ent->self);

            if(mass > 0.0)
                dynamic = true;
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

void lua_LockEntityBodyLinearFactor(world::ObjectId id, size_t body_number, lua::Value vfactor)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id);

    if(   ent
       && body_number < ent->m_skeleton.getBoneCount()
       && ent->m_skeleton.getBones()[body_number].bt_body != nullptr
       && (ent->m_typeFlags & ENTITY_TYPE_DYNAMIC) != 0 )
    {
        glm::float_t t = glm::float_t(ent->m_angles[0]);
        glm::float_t ang1 = glm::sin(t);
        glm::float_t ang2 = glm::cos(t);
        glm::float_t ang3 = 1.0;

        if(vfactor.is<lua::Number>())
        {
            ang3 = glm::min(glm::abs(vfactor.to<float>()), 1.0f);
        }

        ent->m_skeleton.getBones()[body_number].bt_body->setLinearFactor(btVector3(glm::abs(ang1), glm::abs(ang2), ang3));
    }
    else
    {
        Console::instance().warning(SYSWARN_CANT_APPLY_FORCE, id);
    }
}

void lua_SetCharacterWeaponModel(world::ObjectId id, int weaponmodel, int state)
{
    std::shared_ptr<world::Character> ent = engine::engine_world.getCharacterByID(id);

    if(ent)
    {
        ent->setWeaponModel(weaponmodel, state!=0);
    }
    else
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
    }
}

lua::Any lua_GetCharacterCurrentWeapon(world::ObjectId id)
{
    std::shared_ptr<world::Character> ent = engine::engine_world.getCharacterByID(id);

    if(ent)
    {
        return ent->m_currentWeapon;
    }
    else
    {
        Console::instance().warning(SYSWARN_NO_ENTITY, id);
        return {};
    }
}

void lua_SetCharacterCurrentWeapon(world::ObjectId id, world::ModelId weapon)
{
    std::shared_ptr<world::Character> ent = engine::engine_world.getCharacterByID(id);

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
    if(id.is<world::ObjectId>())
    {
        std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(id.to<world::ObjectId>());

        glm::vec3 cam_pos = render::renderer.camera()->getPosition();

        glm::float_t dist = glm::distance(glm::vec3(ent->m_transform[3]), cam_pos);
        dist = dist > world::MaxShakeDistance ? 0 : 1.0f - dist / world::MaxShakeDistance;

        power *= dist;
    }

    if(power > 0.0)
        render::renderer.camera()->shake(power, util::fromSeconds(time));
}

void lua_FlashSetup(int alpha, int R, int G, int B, int fadeinSpeed, int fadeoutSpeed)
{
    gui::FaderManager::instance->setup(gui::FaderType::Effect,
                   alpha,
                   R, G, B,
                   loader::BlendingMode::Multiply,
                   util::fromSeconds(fadeinSpeed/1000.0f), util::fromSeconds(fadeoutSpeed/1000.0f));
}

void lua_FlashStart()
{
    gui::FaderManager::instance->start(gui::FaderType::Effect, gui::FaderDir::Timed);
}

void lua_FadeOut()
{
    gui::FaderManager::instance->start(gui::FaderType::Black, gui::FaderDir::Out);
}

void lua_FadeIn()
{
    gui::FaderManager::instance->start(gui::FaderType::Black, gui::FaderDir::In);
}

bool lua_FadeCheck()
{
    return gui::FaderManager::instance->getStatus(gui::FaderType::Black) != gui::FaderStatus::Idle;
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
        engine::engine_world.audioEngine.streamPlay(id, mask.to<uint8_t>());
    }
    else
    {
        engine::engine_world.audioEngine.streamPlay(id, 0);
    }
}

void lua_StopStreams()
{
    engine::engine_world.audioEngine.stopStreams();
}

void lua_PlaySound(audio::SoundId id, lua::Value ent_id)
{
    if(id >= engine::engine_world.audioEngine.getSoundIdMapSize())
    {
        Console::instance().warning(SYSWARN_WRONG_SOUND_ID, engine::engine_world.audioEngine.getSoundIdMapSize());
        return;
    }

    boost::optional<world::ObjectId> eid = boost::none;
    if(ent_id.is<world::ObjectId>())
        eid = ent_id.to<world::ObjectId>();
    if(!eid || !engine::engine_world.getEntityByID(*eid))
        eid = boost::none;

    audio::Error result;

    if(eid)
    {
        result = engine::engine_world.audioEngine.send(id, audio::EmitterType::Entity, eid);
    }
    else
    {
        result = engine::engine_world.audioEngine.send(id, audio::EmitterType::Global);
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

void lua_StopSound(audio::SoundId id, lua::Value ent_id)
{
    if(id >= engine::engine_world.audioEngine.getSoundIdMapSize())
    {
        Console::instance().warning(SYSWARN_WRONG_SOUND_ID, engine::engine_world.audioEngine.getSoundIdMapSize());
        return;
    }

    boost::optional<world::ObjectId> eid = boost::none;
    if(ent_id.is<world::ObjectId>())
        eid = ent_id.to<world::ObjectId>();
    if(!eid || !engine::engine_world.getEntityByID(*eid))
        eid = boost::none;

    audio::Error result;

    if(!eid)
    {
        result = engine::engine_world.audioEngine.kill(id, audio::EmitterType::Global);
    }
    else
    {
        result = engine::engine_world.audioEngine.kill(id, audio::EmitterType::Entity, eid);
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
    if(levelId.is<uint32_t>())
        engine::Gameflow_Manager.setLevelID(levelId.to<uint32_t>());

    const char* str = engine_lua["getTitleScreen"](int(engine::Gameflow_Manager.getGameID())).toCStr();
    gui::FaderManager::instance->assignPicture(gui::FaderType::LoadScreen, str);
    gui::FaderManager::instance->start(gui::FaderType::LoadScreen, gui::FaderDir::Out);

    Console::instance().notify(SYSNOTE_CHANGING_GAME, engine::Gameflow_Manager.getGameID());
    engine::Game_LevelTransition(engine::Gameflow_Manager.getLevelID());
    engine::Gameflow_Manager.send(engine::Opcode::LevelComplete, engine::Gameflow_Manager.getLevelID());
}

void lua_LoadMap(const char* mapName, lua::Value gameId, lua::Value mapId)
{
    Console::instance().notify(SYSNOTE_LOADING_MAP, mapName);

    if(mapName && mapName != engine::Gameflow_Manager.getLevelPath())
    {
        if(gameId.is<int8_t>() && gameId.to<int8_t>() >= 0)
        {
            engine::Gameflow_Manager.setGameID(gameId.to<int8_t>());
        }
        if(mapId.is<uint32_t>())
        {
            engine::Gameflow_Manager.setLevelID(mapId.to<uint32_t>());
        }
        char file_path[MAX_ENGINE_PATH];
        engine_lua.getLoadingScreen(engine::Gameflow_Manager.getLevelID(), file_path);
        gui::FaderManager::instance->assignPicture(gui::FaderType::LoadScreen, file_path);
        gui::FaderManager::instance->start(gui::FaderType::LoadScreen, gui::FaderDir::In);
        engine::loadMap(mapName);
    }
}

// Flipped (alternate) room functions

void lua_SetFlipState(size_t group, bool state)
{
    if(group >= engine::engine_world.flip_data.size())
    {
        Console::instance().warning(SYSWARN_WRONG_FLIPMAP_INDEX);
        return;
    }

    if(engine::engine_world.flip_data[group].map == 0x1F)         // Check flipmap state.
    {
        if(engine::engine_world.engineVersion > loader::Engine::TR3)
        {
            for(std::shared_ptr<world::Room> currentRoom : engine::engine_world.rooms)
            {
                if(currentRoom->m_alternateGroup == group)    // Check if group is valid.
                {
                    if(state)
                        currentRoom->swapToAlternate();
                    else
                        currentRoom->swapToBase();
                }
            }

            engine::engine_world.flip_data[group].state = state;
        }
        else
        {
            for(std::shared_ptr<world::Room> currentRoom : engine::engine_world.rooms)
            {
                if(state)
                    currentRoom->swapToAlternate();
                else
                    currentRoom->swapToBase();
            }

            engine::engine_world.flip_data[0].state = state;    // In TR1-3, state is always global.
        }
    }
}

void lua_SetFlipMap(size_t group, int mask, int /*op*/)
{
    int op = mask > AMASK_OP_XOR ? AMASK_OP_XOR : AMASK_OP_OR;

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

lua::Any lua_GetFlipMap(size_t group)
{
    if(group >= engine::engine_world.flip_data.size())
    {
        Console::instance().warning(SYSWARN_WRONG_FLIPMAP_INDEX);
        return {};
    }

    return engine::engine_world.flip_data[group].map;
}

lua::Any lua_GetFlipState(size_t group)
{
    if(group >= engine::engine_world.flip_data.size())
    {
        Console::instance().warning(SYSWARN_WRONG_FLIPMAP_INDEX);
        return {};
    }

    return engine::engine_world.flip_data[group].state;
}

/*
 * Generate UV rotate animations
 */

void lua_genUVRotateAnimation(world::ModelId id)
{
    world::SkeletalModel* model = engine::engine_world.getModelByID(id);

    if(!model)
        return;

    if(model->meshes.front().mesh_base->m_transparencyPolygons.empty())
        return;
    const world::core::Polygon& firstPolygon = model->meshes.front().mesh_base->m_transparencyPolygons.front();
    if(firstPolygon.textureAnimationId)
        return;

    engine::engine_world.textureAnimations.emplace_back();
    world::animation::TextureAnimationSequence* seq = &engine::engine_world.textureAnimations.back();

    // Fill up new sequence with frame list.

    seq->textureType = world::animation::TextureAnimationType::Forward;
    seq->frame_lock = false;              // by default anim is playing
    seq->uvrotate = true;
    seq->keyFrames.resize(16);
    seq->textureIndices.resize(16);
    seq->reverse = false;       // Needed for proper reverse-type start-up.
    seq->timePerFrame = util::MilliSeconds(25);      // Should be passed as 1 / FPS.
    seq->frameTime = util::Duration::zero();         // Reset frame time to initial state.
    seq->currentFrame     = 0;           // Reset current frame to zero.
    seq->textureIndices[0] = 0;

    glm::float_t v_min, v_max;
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

    seq->uvrotateMax = 0.5f * (v_max - v_min);
    seq->uvrotateSpeed = seq->uvrotateMax / seq->keyFrames.size();

    for(size_t j = 0; j < seq->keyFrames.size(); j++)
    {
        seq->keyFrames[j].textureIndex = firstPolygon.textureIndex;
        seq->keyFrames[j].coordinateTransform = glm::mat2(1.0f);
        seq->keyFrames[j].move.x = 0.0;
        seq->keyFrames[j].move.y = -seq->uvrotateSpeed * j;
    }

    for(world::core::Polygon& p : model->meshes.front().mesh_base->m_transparencyPolygons)
    {
        BOOST_ASSERT(!engine::engine_world.textureAnimations.empty());
        p.textureAnimationId = engine::engine_world.textureAnimations.size() - 1;
        for(world::core::Vertex& v : p.vertices)
        {
            v.tex_coord[1] = v_min + 0.5f * (v.tex_coord[1] - v_min) + seq->uvrotateMax;
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
#define EXPOSE_CCNS(ns, name) m_state.set(#name, static_cast<int>(ns::name))


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

    m_state.set("GlobalSoundId", lua::Table());
#define EXPOSE_GLOBAL_SOUND_ID(name) m_state["GlobalSoundId"].set(#name, static_cast<int>(audio::GlobalSoundId::name))
    EXPOSE_GLOBAL_SOUND_ID(MenuOpen);
    EXPOSE_GLOBAL_SOUND_ID(MenuClose);
    EXPOSE_GLOBAL_SOUND_ID(MenuPage);
    EXPOSE_GLOBAL_SOUND_ID(MenuSelect);
    EXPOSE_GLOBAL_SOUND_ID(MenuWeapon);
    EXPOSE_GLOBAL_SOUND_ID(MenuClang);
    EXPOSE_GLOBAL_SOUND_ID(MovingWall);
    EXPOSE_GLOBAL_SOUND_ID(SpikeHit);
#undef EXPOSE_GLOBAL_SOUND_ID

    m_state.set("SoundId", lua::Table());
#define EXPOSE_SOUND_ID(name) m_state["SoundId"].set(#name, audio::Sound##name)
    EXPOSE_SOUND_ID(Bubble);
    EXPOSE_SOUND_ID(CurtainMove);
    EXPOSE_SOUND_ID(DiscBladeHit);
    EXPOSE_SOUND_ID(DiscgunShoot);
    EXPOSE_SOUND_ID(Doorbell);
    EXPOSE_SOUND_ID(DoorOpen);
    EXPOSE_SOUND_ID(EagleDying);
    EXPOSE_SOUND_ID(Explosion);
    EXPOSE_SOUND_ID(FromWater);
    EXPOSE_SOUND_ID(GenDeath);
    EXPOSE_SOUND_ID(Helicopter);
    EXPOSE_SOUND_ID(HolsterIn);
    EXPOSE_SOUND_ID(HolsterOut);
    EXPOSE_SOUND_ID(Landing);
    EXPOSE_SOUND_ID(LaraBreath);
    EXPOSE_SOUND_ID(LaraInjury);
    EXPOSE_SOUND_ID(LaraScream);
    EXPOSE_SOUND_ID(Medipack);
    EXPOSE_SOUND_ID(MenuRotate);
    EXPOSE_SOUND_ID(No);
    EXPOSE_SOUND_ID(Pushable);
    EXPOSE_SOUND_ID(Reload);
    EXPOSE_SOUND_ID(Ricochet);
    EXPOSE_SOUND_ID(ShotPistols);
    EXPOSE_SOUND_ID(ShotShotgun);
    EXPOSE_SOUND_ID(ShotUzi);
    EXPOSE_SOUND_ID(Sliding);
    EXPOSE_SOUND_ID(Spike);
    EXPOSE_SOUND_ID(SpikedMetalDoorSlide);
    EXPOSE_SOUND_ID(Splash);
    EXPOSE_SOUND_ID(Swim);
    EXPOSE_SOUND_ID(TigerGrowl);
    EXPOSE_SOUND_ID(TR123MenuClose);
    EXPOSE_SOUND_ID(TR123MenuOpen);
    EXPOSE_SOUND_ID(TR123MenuPage);
    EXPOSE_SOUND_ID(TR123MenuWeapon);
    EXPOSE_SOUND_ID(TR13MenuSelect);
    EXPOSE_SOUND_ID(TR1DartShoot);
    EXPOSE_SOUND_ID(TR1MenuClang);
    EXPOSE_SOUND_ID(TR2345MenuClang);
    EXPOSE_SOUND_ID(TR2MenuSelect);
    EXPOSE_SOUND_ID(TR2MovingWall);
    EXPOSE_SOUND_ID(TR2SpikeHit);
    EXPOSE_SOUND_ID(TR3DartgunShoot);
    EXPOSE_SOUND_ID(TR3MovingWall);
    EXPOSE_SOUND_ID(TR3SpikeHit);
    EXPOSE_SOUND_ID(TR45MenuOpenClose);
    EXPOSE_SOUND_ID(TR45MenuPage);
    EXPOSE_SOUND_ID(TR45MenuSelect);
    EXPOSE_SOUND_ID(TR45MenuWeapon);
    EXPOSE_SOUND_ID(Underwater);
    EXPOSE_SOUND_ID(UseKey);
    EXPOSE_SOUND_ID(WadeShallow);
#undef EXPOSE_SOUND_ID

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

    m_state.set("COLLISION_TYPE_NONE", static_cast<int>(world::CollisionType::None));
    m_state.set("COLLISION_TYPE_STATIC", static_cast<int>(world::CollisionType::Static));
    m_state.set("COLLISION_TYPE_KINEMATIC", static_cast<int>(world::CollisionType::Kinematic));
    m_state.set("COLLISION_TYPE_DYNAMIC", static_cast<int>(world::CollisionType::Dynamic));
    m_state.set("COLLISION_TYPE_ACTOR", static_cast<int>(world::CollisionType::Actor));
    m_state.set("COLLISION_TYPE_VEHICLE", static_cast<int>(world::CollisionType::Vehicle));
    m_state.set("COLLISION_TYPE_GHOST", static_cast<int>(world::CollisionType::Ghost));

    m_state.set("COLLISION_SHAPE_BOX", static_cast<int>(world::CollisionShape::Box));
    m_state.set("COLLISION_SHAPE_BOX_BASE", static_cast<int>(world::CollisionShape::BoxBase));
    m_state.set("COLLISION_SHAPE_SPHERE", static_cast<int>(world::CollisionShape::Sphere));
    m_state.set("COLLISION_SHAPE_TRIMESH", static_cast<int>(world::CollisionShape::TriMesh));
    m_state.set("COLLISION_SHAPE_TRIMESH_CONVEX", static_cast<int>(world::CollisionShape::TriMeshConvex));

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

    m_state.set("AnimMode", lua::Table());
    m_state["AnimMode"].set("NormalControl", static_cast<int>(world::animation::AnimationMode::NormalControl));
    m_state["AnimMode"].set("LoopLastFrame", static_cast<int>(world::animation::AnimationMode::LoopLastFrame));
    m_state["AnimMode"].set("WeaponCompat", static_cast<int>(world::animation::AnimationMode::WeaponCompat));
    m_state["AnimMode"].set("Locked", static_cast<int>(world::animation::AnimationMode::Locked));

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

    EXPOSE_CCNS(world, PARAM_HEALTH);
    EXPOSE_CCNS(world, PARAM_AIR);
    EXPOSE_CCNS(world, PARAM_STAMINA);
    EXPOSE_CCNS(world, PARAM_WARMTH);
    EXPOSE_CCNS(world, PARAM_POISON);
    EXPOSE_CCNS(world, PARAM_EXTRA1);
    EXPOSE_CCNS(world, PARAM_EXTRA2);
    EXPOSE_CCNS(world, PARAM_EXTRA3);
    EXPOSE_CCNS(world, PARAM_EXTRA4);

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

    m_state.set("M_PI", SIMD_PI);

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
    const auto top = lua_gettop(lua);
    if(top < 1)
    {
        BOOST_LOG_TRIVIAL(error) << "Fatal lua error (no details provided)";
    }
    else
    {
        const auto msg = lua_tostring(lua, top);
        BOOST_LOG_TRIVIAL(error) << "Fatal lua error: " << (msg ? msg : "<null>");
        BOOST_LOG_TRIVIAL(error) << "Backtrace:";
    }
    return 0;
}

void MainEngine::registerMainFunctions()
{
    // Register globals

    set(CVAR_LUA_TABLE_NAME, lua::Table());

    engine::Game_RegisterLuaFunctions(*this);

    // Register script functions

    auto self = this;
    registerC("checkStack", std::function<void()>([self]{self->checkStack();}));
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
    registerC("setAnimEndCommands", lua_SetAnimEndCommands);
    registerC("setStateChangeRange", lua_SetStateChangeRange);

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
    *ch = parse_token(*ch, token);
    if(token[0])
    {
        return std::stof(token);
    }
    return 0.0f;
}

int script::MainEngine::parseInt(char **ch)
{
    char token[64];
    *ch = const_cast<char*>(parse_token(*ch, token));
    if(token[0])
    {
        return atoi(token);
    }
    return 0;
}

// Specific functions to get specific parameters from script.

boost::optional<audio::SoundId> script::MainEngine::getGlobalSound(audio::GlobalSoundId global_sound_id)
{
    auto result = call("getGlobalSound", static_cast<int>(global_sound_id));
    if(result.is<audio::SoundId>())
        return result.to<audio::SoundId>();
    else
        return boost::none;
}

int script::MainEngine::getSecretTrackNumber()
{
    return call("getSecretTrackNumber", static_cast<int>(engine::engine_world.engineVersion)).to<int>();
}

int script::MainEngine::getNumTracks()
{
    return call("getNumTracks", static_cast<int>(engine::engine_world.engineVersion)).to<int>();
}

bool script::MainEngine::getOverridedSamplesInfo(int& num_samples, int& num_sounds, std::string& sample_name_mask)
{
    lua::tie(sample_name_mask, num_sounds, num_samples) = call("getOverridedSamplesInfo", static_cast<int>(engine::engine_world.engineVersion));

    return num_sounds != -1 && num_samples != -1 && sample_name_mask != "NONE";
}

bool script::MainEngine::getOverridedSample(int sound_id, int& first_sample_number, int& samples_count)
{
    lua::tie(first_sample_number, samples_count) = call("getOverridedSample", static_cast<int>(engine::engine_world.engineVersion), int(engine::Gameflow_Manager.getLevelID()), sound_id);
    return first_sample_number != -1 && samples_count != -1;
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
    const char* str = call("getString", string_index).toCStr();
    strncpy(buffer, str, string_size);
    return true;
}

bool script::MainEngine::getSysNotify(int string_index, size_t string_size, char *buffer)
{
    const char* str = call("getSysNotify", string_index).toCStr();
    strncpy(buffer, str, string_size);
    return true;
}

bool script::MainEngine::getLoadingScreen(int level_index, char *pic_path)
{
    const char* realPath = call("getLoadingScreen", int(engine::Gameflow_Manager.getGameID()), int(engine::Gameflow_Manager.getLevelID()), level_index).toCStr();
    strncpy(pic_path, realPath, MAX_ENGINE_PATH);
    return true;
}


// System lua functions


void script::MainEngine::addKey(int keycode, bool state)
{
    call("addKey", keycode, state);
}

void script::MainEngine::execEntity(int id_callback, world::ObjectId id_object, const boost::optional<world::ObjectId>& id_activator)
{
    if(id_activator)
        call("execEntity", id_callback, id_object, *id_activator);
    else
        call("execEntity", id_callback, id_object);
}

void script::MainEngine::loopEntity(world::ObjectId object_id)
{
    std::shared_ptr<world::Entity> ent = engine::engine_world.getEntityByID(object_id);
    if(ent && ent->m_active)
    {
        call("loopEntity", object_id);
    }
}

void script::MainEngine::execEffect(int id, const boost::optional<world::ObjectId>& caller, const boost::optional<world::ObjectId>& operand)
{
    call("execFlipeffect", id, caller ? *caller : -1, operand ? *operand : -1);
}


// Parsing config file entries.

void script::ScriptEngine::parseControls(engine::ControlSettings& cs) const
{
    cs.mouse_sensitivity = (*this)["controls"]["mouse_sensitivity"].toFloat();
    cs.mouse_scale_x = (*this)["controls"]["mouse_scale_x"].toFloat();
    cs.mouse_scale_y = (*this)["controls"]["mouse_scale_y"].toFloat();
    cs.use_joy = (*this)["controls"]["use_joy"].toBool();
    cs.joy_number = (*this)["controls"]["joy_number"].to<int>();
    cs.joy_rumble = (*this)["controls"]["joy_rumble"].toBool();
    cs.joy_axis_map[engine::AXIS_LOOK_X] = (*this)["controls"]["joy_look_axis_x"].to<int>();
    cs.joy_axis_map[engine::AXIS_LOOK_Y] = (*this)["controls"]["joy_look_axis_y"].to<int>();
    cs.joy_axis_map[engine::AXIS_MOVE_X] = (*this)["controls"]["joy_move_axis_x"].to<int>();
    cs.joy_axis_map[engine::AXIS_MOVE_Y] = (*this)["controls"]["joy_move_axis_y"].to<int>();
    cs.joy_look_invert_x = (*this)["controls"]["joy_look_invert_x"].toBool();
    cs.joy_look_invert_y = (*this)["controls"]["joy_look_invert_y"].toBool();
    cs.joy_look_sensitivity = (*this)["controls"]["joy_look_sensitivity"].toFloat();
    cs.joy_look_deadzone = (*this)["controls"]["joy_look_deadzone"].to<int16_t>();
    cs.joy_move_invert_x = (*this)["controls"]["joy_move_invert_x"].toBool();
    cs.joy_move_invert_y = (*this)["controls"]["joy_move_invert_y"].toBool();
    cs.joy_move_sensitivity = (*this)["controls"]["joy_move_sensitivity"].toFloat();
    cs.joy_move_deadzone = (*this)["controls"]["joy_move_deadzone"].to<int16_t>();
}

void script::ScriptEngine::parseScreen(engine::ScreenInfo& sc) const
{
    sc.x = (*this)["screen"]["x"].to<int16_t>();
    sc.y = (*this)["screen"]["y"].to<int16_t>();
    sc.w = (*this)["screen"]["width"].to<int16_t>();
    sc.h = (*this)["screen"]["height"].to<int16_t>();
    sc.w_unit = sc.w / gui::ScreenMeteringResolution;
    sc.h_unit = sc.h / gui::ScreenMeteringResolution;
    sc.FS_flag = (*this)["screen"]["fullscreen"].toBool();
    sc.show_debuginfo = (*this)["screen"]["debug_info"].toBool();
    sc.fov = (*this)["screen"]["fov"].toFloat();
    sc.vsync = (*this)["screen"]["vsync"].toBool();
}

void script::ScriptEngine::parseRender(render::RenderSettings& rs) const
{
    rs.mipmap_mode = (*this)["render"]["mipmap_mode"].to<uint32_t>();
    rs.mipmaps = (*this)["render"]["mipmaps"].to<uint32_t>();
    rs.lod_bias = (*this)["render"]["lod_bias"].toFloat();
    rs.anisotropy = (*this)["render"]["anisotropy"].to<uint32_t>();
    rs.antialias = (*this)["render"]["antialias"].toBool();
    rs.antialias_samples = (*this)["render"]["antialias_samples"].to<int>();
    rs.texture_border = (*this)["render"]["texture_border"].to<int>();
    rs.save_texture_memory = (*this)["render"]["save_texture_memory"].toBool();
    rs.z_depth = (*this)["render"]["z_depth"].to<int>();
    rs.fog_enabled = (*this)["render"]["fog_enabled"].toBool();
    rs.fog_start_depth = (*this)["render"]["fog_start_depth"].toFloat();
    rs.fog_end_depth = (*this)["render"]["fog_end_depth"].toFloat();
    rs.fog_color[0] = (*this)["render"]["fog_color"]["r"].toFloat();
    rs.fog_color[0] /= 255.0;
    rs.fog_color[1] = (*this)["render"]["fog_color"]["g"].toFloat();
    rs.fog_color[1] /= 255.0;
    rs.fog_color[2] = (*this)["render"]["fog_color"]["b"].toFloat();
    rs.fog_color[2] /= 255.0;
    rs.fog_color[3] = 1;

    rs.use_gl3 = (*this)["render"]["use_gl3"].toBool();

    if(rs.z_depth != 8 && rs.z_depth != 16 && rs.z_depth != 24)
        rs.z_depth = 24;
}

void script::ScriptEngine::parseAudio(audio::Settings& as) const
{
    as.music_volume = (*this)["audio"]["music_volume"].to<ALfloat>();
    as.sound_volume = (*this)["audio"]["sound_volume"].to<ALfloat>();
    as.use_effects = (*this)["audio"]["use_effects"].toBool();
    as.listener_is_player = (*this)["audio"]["listener_is_player"].toBool();
    as.stream_buffer_size = (*this)["audio"]["stream_buffer_size"].to<int>();
    as.stream_buffer_size *= 1024;
    if(as.stream_buffer_size <= 0)
        as.stream_buffer_size = 128 * 1024;
    as.music_volume = (*this)["audio"]["music_volume"].to<ALfloat>();
    as.music_volume = (*this)["audio"]["music_volume"].to<ALfloat>();
}

void script::ScriptEngine::parseConsole(gui::Console& cn) const
{
    {
        float r = (*this)["console"]["background_color"]["r"].toFloat();
        float g = (*this)["console"]["background_color"]["g"].toFloat();
        float b = (*this)["console"]["background_color"]["b"].toFloat();
        float a = (*this)["console"]["background_color"]["a"].toFloat();
        cn.setBackgroundColor(r / 255, g / 255, b / 255, a / 255);
    }

    float tmpF = (*this)["console"]["spacing"].toFloat();
    if(tmpF >= CON_MIN_LINE_INTERVAL && tmpF <= CON_MAX_LINE_INTERVAL)
        cn.setSpacing(tmpF);

    int tmpI = (*this)["console"]["line_size"].to<int>();
    if(tmpI >= CON_MIN_LINE_SIZE && tmpI <= CON_MAX_LINE_SIZE)
        cn.setLineSize(tmpI);

    tmpI = (*this)["console"]["showing_lines"].to<int>();
    if(tmpI >= CON_MIN_LINES && tmpI <= CON_MIN_LINES)
        cn.setVisibleLines(tmpI);

    tmpI = (*this)["console"]["log_size"].to<int>();
    if(tmpI >= CON_MIN_LOG && tmpI <= CON_MAX_LOG)
        cn.setHistorySize(tmpI);

    tmpI = (*this)["console"]["lines_count"].to<int>();
    if(tmpI >= CON_MIN_LOG && tmpI <= CON_MAX_LOG)
        cn.setBufferSize(tmpI);

    bool tmpB = (*this)["console"]["show"].toBool();
    cn.setVisible(tmpB);

    tmpI = (*this)["console"]["show_cursor_period"].to<int>();
    cn.setShowCursorPeriod(util::MilliSeconds(tmpI));
}

void script::ScriptEngine::parseSystem(engine::SystemSettings& ss) const
{
    ss.logging = (*this)["system"]["logging"].toBool();
}
