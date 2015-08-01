
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "script.h"
#include "system.h"
#include "console.h"
#include "entity.h"
#include "world.h"
#include "engine.h"
#include "controls.h"
#include "game.h"
#include "gameflow.h"
#include "anim_state_control.h"
#include "character_controller.h"
#include "vmath.h"
#include "render.h"
#include "audio.h"
#include "strings.h"
#include "hair.h"
#include "ragdoll.h"

#include <lua.hpp>
#include "LuaState.h"


/*
 * debug functions
 */

void lua_CheckStack()
{
    ConsoleInfo::instance().notify(SYSNOTE_LUA_STACK_INDEX, lua_gettop(engine_lua.getState()));
}

int lua_Print(lua_State* state)
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
        ConsoleInfo::instance().warning(SYSWARN_WRONG_MODEL_ID, id);
        return;
    }

    for(int i=0;i<sm->mesh_count;i++)
    {
        ConsoleInfo::instance().printf("mesh[%d] = %d", i, sm->mesh_tree[i].mesh_base->m_id);
    }
}

void lua_DumpRoom(lua::Value id)
{
    if(id.is<lua::Nil>()) {
        Engine_DumpRoom(engine_camera.m_currentRoom);
        return;
    }
    if(id.is<lua::Integer>() && static_cast<uint32_t>(id) >= engine_world.rooms.size())
    {
        ConsoleInfo::instance().warning(SYSWARN_WRONG_ROOM, static_cast<int>(id));
        return;
    }
    Engine_DumpRoom(engine_world.rooms[id].get());
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
		return;
    }

    if((arg >= 0) && (arg < model->collision_map.size()) &&
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

uint32_t lua_GetEntitySubstanceState(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent != NULL)
    {
        return static_cast<uint32_t>(ent->getSubstanceState());
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
    btVector3 move = ent->m_speed * time;
    move += g * 0.5 * time * time;
    ent->m_speed += g * time;

    BtEngineClosestRayResultCallback cb(ent->m_self);
    btVector3 from, to;
    from = ent->m_transform * ent->m_bf.centre;
    from[2] = ent->m_transform.getOrigin()[2];
    to = from + move;
    //to[2] -= (ent->m_bf.bb_max[2] - ent->m_bf.bb_min[2]);
    bt_engine_dynamicsWorld->rayTest(from, to, cb);

    if(cb.hasHit())
    {
        EngineContainer* cont = (EngineContainer*)cb.m_collisionObject->getUserPointer();

        if(!only_room.is<lua::Boolean>() || !only_room.to<bool>() || (only_room.to<bool>() && (cont->object_type == OBJECT_ROOM_BASE)))
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

void lua_ChangeCharacterParam(int id, int parameter, lua::Value value)
{
    std::shared_ptr<Character> ent = engine_world.getCharacterByID(id);

    if(parameter >= PARAM_SENTINEL)
    {
        ConsoleInfo::instance().warning(SYSWARN_WRONG_OPTION_INDEX, PARAM_SENTINEL);
        return;
    }

    if(ent && (value.is<lua::Number>() || value.is<lua::Integer>()))
    {
        if(value.is<lua::Number>())
            ent->changeParam(parameter, value.to<lua::Number>());
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
    if((secret_number > TR_GAMEFLOW_MAX_SECRETS) || (secret_number < 0))
		return;   // No such secret - return

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
        ConsoleInfo::instance().printf("item[id = %d]: count = %d", i.id, i.count);
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

uint32_t lua_SpawnEntity(int model_id, float x, float y, float z, float ax, float ay, float az, int room_id, lua::Value ov_id)
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

bool lua_DeleteEntity(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent != nullptr)
    {
        if(ent->m_self->room) ent->m_self->room->removeEntity(ent.get());
        engine_world.deleteEntity(id);
        return true;
    }
    else
    {
        return false;
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

    return e1->m_transform.getBasis().getColumn(1).dot(e2->m_transform.getBasis().getColumn(1));
}

bool lua_IsInRoom(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

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

std::tuple<float,float,float,float,float,float,uint32_t> lua_GetEntityPosition(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);
    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return {};
    }

    return std::tuple<float,float,float,float,float,float,uint32_t>
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

    auto next_pos = ent->m_transform.getOrigin() + (dx * ent->m_transform.getBasis().getColumn(0) + dy * ent->m_transform.getBasis().getColumn(1) + dz * ent->m_transform.getBasis().getColumn(2));

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
        pos += dx.to<btScalar>() * ent->m_transform.getBasis().getColumn(0) + dy.to<btScalar>() * ent->m_transform.getBasis().getColumn(1) + dz.to<btScalar>() * ent->m_transform.getBasis().getColumn(2);

    RoomSector* curr_sector = ent->m_self->room->getSectorRaw(pos);
    curr_sector = curr_sector->checkPortalPointer();
    btVector3 point = (ceiling.is<lua::Boolean>() && ceiling.to<bool>())
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
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
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
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(!ent)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return;
    }

    ent->m_transform.getOrigin() += ent->m_transform.getBasis() * btVector3(dx, dy, dz);

    ent->updateRigidBody(true);
    ent->ghostUpdate();
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
    sink_pos[2] = sink->z + 256.0f;

    assert( ent->m_currentSector != nullptr );
    RoomSector* ls = ent->m_currentSector->getLowestSector();
    assert( ls != nullptr );
    RoomSector* hs = ent->m_currentSector->getHighestSector();
    assert( hs != nullptr );
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
    ent->m_transform.getOrigin()[2] += speed[2] * 16.0f;

    ent->updateRigidBody(true);
    ent->ghostUpdate();
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
        offset = {ofsX,ofsY,ofsZ};
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

    if(ent == NULL)
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
         ent->m_triggerLayout & ENTITY_TLAYOUT_MASK,   // mask
        (ent->m_triggerLayout & ENTITY_TLAYOUT_EVENT), // event
        (ent->m_triggerLayout & ENTITY_TLAYOUT_LOCK)   // lock
    };
}

void lua_SetEntityTriggerLayout(int id, int mask, lua::Value eventOrLayout, lua::Value lock)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
        return;
    }

    if(lock.is<lua::Boolean>()) {
        uint8_t trigger_layout = ent->m_triggerLayout;
        trigger_layout &= ~(uint8_t)(ENTITY_TLAYOUT_MASK);  trigger_layout ^= mask;          // mask  - 00011111
        trigger_layout &= ~(uint8_t)(ENTITY_TLAYOUT_EVENT); trigger_layout ^= eventOrLayout.to<bool>() << 5;   // event - 00100000
        trigger_layout &= ~(uint8_t)(ENTITY_TLAYOUT_LOCK);  trigger_layout ^= lock.to<bool>() << 6;   // lock  - 01000000
        ent->m_triggerLayout = trigger_layout;
    }
    else
    {
        ent->m_triggerLayout = eventOrLayout.to<int>();
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
    return true;
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
            ConsoleInfo::instance().warning(SYSWARN_WRONG_BONE_NUMBER, bone);
        }
    }
    else
    {
        ConsoleInfo::instance().warning(SYSWARN_WRONG_MODEL_ID, id);
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
            ConsoleInfo::instance().warning(SYSWARN_WRONG_BONE_NUMBER, bone);
        }
    }
    else
    {
        ConsoleInfo::instance().warning(SYSWARN_WRONG_MODEL_ID, id);
    }
}

void lua_CopyMeshFromModelToModel(int id1, int id2, int bone1, int bone2)
{
    SkeletalModel* sm1 = engine_world.getModelByID(id1);
    if(sm1 == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_WRONG_MODEL_ID, id1);
        return;
    }

    SkeletalModel* sm2 = engine_world.getModelByID(id2);
    if(sm2 == NULL)
    {
        ConsoleInfo::instance().warning(SYSWARN_WRONG_MODEL_ID, id2);
        return;
    }

    if((bone1 >= 0) && (bone1 < sm1->mesh_count) && (bone2 >= 0) && (bone2 < sm2->mesh_count))
    {
        sm1->mesh_tree[bone1].mesh_base = sm2->mesh_tree[bone2].mesh_base;
    }
    else
    {
        ConsoleInfo::instance().warning(SYSWARN_WRONG_BONE_NUMBER, bone1);
    }
}

void lua_CreateEntityGhosts(int id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent && (ent->m_bf.bone_tags.size() > 0))
    {
        ent->createGhosts();
    }
}

void lua_PushEntityBody(int id, uint32_t body_number, float h_force, float v_force, bool resetFlag)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent && (body_number < ent->m_bf.bone_tags.size()) && (ent->m_bt.bt_body[body_number] != NULL) && (ent->m_typeFlags & ENTITY_TYPE_DYNAMIC))
    {
        btScalar t = ent->m_angles[0] * RadPerDeg;

        btScalar ang1 = std::sin(t);
        btScalar ang2 = std::cos(t);

        btVector3 angle (-ang1 * h_force, ang2 * h_force, v_force);

        if(resetFlag)
            ent->m_bt.bt_body[body_number]->clearForces();

        ent->m_bt.bt_body[body_number]->setLinearVelocity(angle);
        ent->m_bt.bt_body[body_number]->setAngularVelocity(angle / 1024.0);
    }
    else
    {
        ConsoleInfo::instance().warning(SYSWARN_CANT_APPLY_FORCE, id);
    }
}

int lua_SetEntityBodyMass(lua_State *lua)
{
    int top = lua_gettop(lua);

    if(lua_gettop(lua) < 3)
    {
        ConsoleInfo::instance().warning(SYSWARN_WRONG_ARGS, "[entity_id, body_number, (mass / each body mass)]");
        return 0;
    }

    const auto id = lua_tounsigned(lua, 1);
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    int body_number = lua_tounsigned(lua, 2);
    body_number = (body_number < 1)?(1):(body_number);

    uint16_t argn  = 3;
    bool dynamic = false;

    if(ent && (static_cast<int>(ent->m_bf.bone_tags.size()) >= body_number))
    {
        for(int i=0; i<body_number; i++)
        {
            btVector3 inertia (0.0, 0.0, 0.0);

            btScalar mass = 0;
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
        ConsoleInfo::instance().warning(SYSWARN_WRONG_ENTITY_OR_BODY, id, body_number);
    }

    return 0;
}

void lua_LockEntityBodyLinearFactor(int id, uint32_t body_number, lua::Value vfactor)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(id);

    if(ent && (body_number < ent->m_bf.bone_tags.size()) && (ent->m_bt.bt_body[body_number] != NULL) && (ent->m_typeFlags & ENTITY_TYPE_DYNAMIC))
    {
        btScalar t    = ent->m_angles[0] * RadPerDeg;
        btScalar ang1 = std::sin(t);
        btScalar ang2 = std::cos(t);
        btScalar ang3 = 1.0;

        if(vfactor.is<lua::Number>())
        {
            ang3 = std::abs(vfactor.to<float>());
            ang3 = (ang3 > 1.0)?(1.0f):(ang3);
        }

        ent->m_bt.bt_body[body_number]->setLinearFactor(btVector3(std::abs(ang1), std::abs(ang2), ang3));
    }
    else
    {
        ConsoleInfo::instance().warning(SYSWARN_CANT_APPLY_FORCE, id);
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
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
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
        ConsoleInfo::instance().warning(SYSWARN_NO_ENTITY, id);
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
        dist = (dist > TR_CAM_MAX_SHAKE_DISTANCE)?(0):(1.0f - (dist / TR_CAM_MAX_SHAKE_DISTANCE));

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
        Audio_StreamPlay(id, mask.to<int>());
    }
    else
    {
        Audio_StreamPlay(id);
    }
}

void lua_StopStreams()
{
    Audio_StopStreams();
}

void lua_PlaySound(int id, lua::Value ent_id)
{
    if(id < 0) return;

    if(static_cast<size_t>(id) >= engine_world.audio_map.size())
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
    ConsoleInfo::instance().notify(SYSNOTE_LOADING_MAP, mapName);

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
    seq->frame_rate        = 0.025f;  // Should be passed as 1 / FPS.
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

    seq->uvrotate_max = 0.5f * (v_max - v_min);
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
            v.tex_coord[1] = v_min + 0.5f * (v.tex_coord[1] - v_min) + seq->uvrotate_max;
        }
    }

    return;
}

// Called when something goes absolutely horribly wrong in Lua, and tries
// to produce some debug output. Lua calls abort afterwards, so sending
// the output to the internal console is not an option.

static int lua_Panic(lua_State *lua)
{
    if (lua_gettop(lua) < 1) {
        fprintf(stderr, "Fatal lua error (no details provided).\n");
    } else {
        fprintf(stderr, "Fatal lua error: %s\n", lua_tostring(lua, 1));
    }
    fflush(stderr);
    return 0;
}

void Script_LuaInit()
{
    Script_LuaRegisterFuncs(engine_lua);
    lua_atpanic(engine_lua.getState(), lua_Panic);

    // Load script loading order (sic!)

    luaL_dofile(engine_lua.getState(), "scripts/loadscript.lua");
}

void Script_LuaClearTasks()
{
    engine_lua["clearTasks"]();
}


void Script_LuaRegisterFuncs(lua::State& state)
{
    /*
     * register globals
     */
    state.set(CVAR_LUA_TABLE_NAME, lua::Table());

    Game_RegisterLuaFunctions(state);

    // Register script functions

    lua_registerc(state, "print", lua_Print);

    lua_registerc(state, "checkStack", lua_CheckStack);
    lua_registerc(state, "dumpModel", lua_DumpModel);
    lua_registerc(state, "dumpRoom", lua_DumpRoom);
    lua_registerc(state, "setRoomEnabled", lua_SetRoomEnabled);

    lua_registerc(state, "playSound", lua_PlaySound);
    lua_registerc(state, "stopSound", lua_StopSound);

    lua_registerc(state, "playStream", lua_PlayStream);
    lua_registerc(state, "stopStreams", lua_StopStreams);

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
    lua_registerc(state, "deleteEntity", lua_DeleteEntity);
    lua_registerc(state, "enableEntity", lua_EnableEntity);
    lua_registerc(state, "disableEntity", lua_DisableEntity);

    lua_registerc(state, "isInRoom", lua_IsInRoom);
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

    lua_registerc(state, "createEntityGhosts", lua_CreateEntityGhosts);
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
    lua_registerc(state, "getEntitySubstanceState", lua_GetEntitySubstanceState);

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

/*
 * MISC
 */

const char *parse_token(const char *data, char *token)
{
    ///@FIXME: token may be overflowed
    int c;
    int len;

    len = 0;
    token[0] = 0;

    if(!data)
    {
        return NULL;
    }

// skip whitespace
    skipwhite:
    while((c = *data) <= ' ')
    {
        if(c == 0)
            return NULL;                    // end of file;
        data++;
    }

// skip // comments
    if (c=='/' && data[1] == '/')
    {
        while (*data && *data != '\n')
            data++;
        goto skipwhite;
    }

// handle quoted strings specially
    if (c == '\"')
    {
        data++;
        while (1)
        {
            c = *data++;
            if (c=='\"' || !c)
            {
                token[len] = 0;
                return data;
            }
            token[len] = c;
            len++;
        }
    }


// parse single characters
    if (c=='{' || c=='}'|| c==')'|| c=='(' || c=='\'' || c==':')
    {
        token[len] = c;
        len++;
        token[len] = 0;
        return data+1;
    }


// parse a regular word
    do
    {
        token[len] = c;
        data++;
        len++;
        c = *data;
        if (c=='{' || c=='}'|| c==')'|| c=='(' || c=='\'' || c==':')
        {
            break;
        }
    } while (c>32);

    token[len] = 0;
    return data;
}

float Script_ParseFloat(const char **ch)
{
    char token[64];
    (*ch) = parse_token(*ch, token);
    if(token[0])
    {
        return atof(token);
    }
    return 0.0;
}

int Script_ParseInt(char **ch)
{
    char token[64];
    (*ch) = const_cast<char*>( parse_token(*ch, token) );
    if(token[0])
    {
        return atoi(token);
    }
    return 0;
}

/*
 *   Specific functions to get specific parameters from script.
 */

int lua_GetGlobalSound(lua::State& state, int global_sound_id)
{
    return state["getGlobalSound"](engine_world.version, global_sound_id);
}

int lua_GetSecretTrackNumber(lua::State& state)
{
    return state["getSecretTrackNumber"](engine_world.version);
}

int lua_GetNumTracks(lua::State& state)
{
    return state["getNumTracks"](engine_world.version);
}

bool lua_GetOverridedSamplesInfo(lua::State& state, int *num_samples, int *num_sounds, char *sample_name_mask)
{
    const char* realPath;
    lua::tie(realPath, *num_sounds, *num_samples) = state["getOverridedSamplesInfo"](engine_world.version);

    strcpy(sample_name_mask, realPath);

    return *num_sounds != -1 && *num_samples != -1 && strcmp(realPath, "NONE")!=0;
}

bool lua_GetOverridedSample(lua::State& state, int sound_id, int *first_sample_number, int *samples_count)
{
    lua::tie(*first_sample_number, *samples_count) = state["getOverridedSample"](engine_world.version, int(gameflow_manager.CurrentLevelID), sound_id);
    return *first_sample_number != -1 && *samples_count != -1;
}

bool lua_GetSoundtrack(lua::State& state, int track_index, char *file_path, int *load_method, int *stream_type)
{
    const char* realPath;
    int _load_method, _stream_type;

    lua::tie(realPath, _stream_type, _load_method) = state["getTrackInfo"](engine_world.version, track_index);
    if(file_path) strcpy(file_path, realPath);
    if(load_method) *load_method = _load_method;
    if(stream_type) *stream_type = _stream_type;
    return _stream_type != -1;
}

bool lua_GetString(lua::State& state, int string_index, size_t string_size, char *buffer)
{
    const char* str = state["getString"](string_index);
    strncpy(buffer, str, string_size);
    return true;
}

bool lua_GetSysNotify(lua::State& state, int string_index, size_t string_size, char *buffer)
{
    const char* str = state["getSysNotify"](string_index);
    strncpy(buffer, str, string_size);
    return true;
}

bool lua_GetLoadingScreen(lua::State& state, int level_index, char *pic_path)
{
    const char* realPath = state["getLoadingScreen"](int(gameflow_manager.CurrentGameID), int(gameflow_manager.CurrentLevelID), level_index);
    strncpy(pic_path, realPath, MAX_ENGINE_PATH);
    return true;
}

/*
 * System lua functions
 */

void lua_Clean(lua::State& state)
{
    state["tlist_Clear"]();
    state["entfuncs_Clear"]();
    state["fe_Clear"]();
}

void lua_Prepare(lua::State& state)
{
    state["fe_Prepare"]();
}

void lua_DoTasks(lua::State& state, btScalar time)
{
    state.set( "frame_time", time );
    state["doTasks"]();
    state["clearKeys"]();
}

void lua_AddKey(lua::State& lstate, int keycode, bool state)
{
    lstate["addKey"](keycode, state);
}

void lua_ExecEntity(lua::State& state, int id_callback, int id_object, int id_activator)
{
    if(id_activator >= 0)
        state["execEntity"](id_callback, id_object, id_activator);
    else
        state["execEntity"](id_callback, id_object);
}

void lua_LoopEntity(lua::State& state, int object_id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(object_id);
    if(ent && ent->m_active) {
        state["loopEntity"](object_id);
    }
}

void lua_ExecEffect(lua::State& state, int id, int caller, int operand)
{
    state["execFlipeffect"](id, caller, operand);
}

/*
 * Game structures parse
 */

void lua_ParseControls(lua::State& state, struct ControlSettings *cs)
{
    cs->mouse_sensitivity = state["controls"]["mouse_sensitivity"];
    cs->use_joy = state["controls"]["use_joy"];
    cs->joy_number = state["controls"]["joy_number"];
    cs->joy_rumble = state["controls"]["joy_rumble"];
    cs->joy_axis_map[AXIS_MOVE_X] = state["controls"]["joy_look_axis_x"];
    cs->joy_axis_map[AXIS_MOVE_Y] = state["controls"]["joy_look_axis_y"];
    cs->joy_look_invert_x = state["controls"]["joy_look_invert_x"];
    cs->joy_look_invert_y = state["controls"]["joy_look_invert_y"];
    cs->joy_look_sensitivity = state["controls"]["joy_look_sensitivity"];
    cs->joy_look_deadzone = state["controls"]["joy_look_deadzone"];
    cs->joy_move_invert_x = state["controls"]["joy_move_invert_x"];
    cs->joy_move_invert_y = state["controls"]["joy_move_invert_y"];
    cs->joy_move_sensitivity = state["controls"]["joy_move_sensitivity"];
    cs->joy_move_deadzone = state["controls"]["joy_move_deadzone"];
}

void lua_ParseScreen(lua::State& state, struct ScreenInfo *sc)
{
    sc->x = state["screen"]["x"];
    sc->y = state["screen"]["y"];
    sc->w = state["screen"]["width"];
    sc->h = state["screen"]["height"];
    sc->w = state["screen"]["width"];
    sc->w_unit = sc->w / GUI_SCREEN_METERING_RESOLUTION;
    sc->h = state["screen"]["height"];
    sc->h_unit = sc->h / GUI_SCREEN_METERING_RESOLUTION;
    sc->FS_flag = state["screen"]["fullscreen"];
    sc->show_debuginfo = state["screen"]["debug_info"];
    sc->fov = state["screen"]["fov"];
}

void lua_ParseRender(lua::State& state, struct RenderSettings *rs)
{
    rs->mipmap_mode = state["render"]["mipmap_mode"];
    rs->mipmaps = state["render"]["mipmaps"];
    rs->lod_bias = state["render"]["lod_bias"];
    rs->anisotropy = state["render"]["anisotropy"];
    rs->antialias = state["render"]["antialias"];
    rs->antialias_samples = state["render"]["antialias_samples"];
    rs->texture_border = state["render"]["texture_border"];
    rs->save_texture_memory = state["render"]["save_texture_memory"];
    rs->z_depth = state["render"]["z_depth"];
    rs->fog_enabled = state["render"]["fog_enabled"];
    rs->fog_start_depth = state["render"]["fog_start_depth"];
    rs->fog_end_depth = state["render"]["fog_end_depth"];
    rs->fog_color[0] = state["render"]["fog_color"]["r"];
    rs->fog_color[0] /= 255.0;
    rs->fog_color[1] = state["render"]["fog_color"]["g"];
    rs->fog_color[1] /= 255.0;
    rs->fog_color[2] = state["render"]["fog_color"]["b"];
    rs->fog_color[2] /= 255.0;
    rs->fog_color[3] = 1;
    if(rs->z_depth != 8 && rs->z_depth != 16 && rs->z_depth != 24)
    {
        rs->z_depth = 24;
    }
}

void lua_ParseAudio(lua::State& state, struct AudioSettings *as)
{
    as->music_volume = state["audio"]["music_volume"];
    as->sound_volume = state["audio"]["sound_volume"];
    as->use_effects = state["audio"]["use_effects"].to<bool>();
    as->listener_is_player = state["audio"]["listener_is_player"].to<bool>();
    as->stream_buffer_size = state["audio"]["stream_buffer_size"];
    as->stream_buffer_size *= 1024;
    if(as->stream_buffer_size <= 0)
        as->stream_buffer_size = 128 * 1024;
    as->music_volume = state["audio"]["music_volume"];
    as->music_volume = state["audio"]["music_volume"];
}

void lua_ParseConsole(lua::State& state, ConsoleInfo *cn)
{
    {
        float r = state["console"]["background_color"]["r"];
        float g = state["console"]["background_color"]["g"];
        float b = state["console"]["background_color"]["b"];
        float a = state["console"]["background_color"]["a"];
        cn->setBackgroundColor(r/255, g/255, b/255, a/255);
    }

    float tmpF = state["console"]["spacing"];
    if(tmpF >= CON_MIN_LINE_INTERVAL && tmpF <= CON_MAX_LINE_INTERVAL)
        cn->setSpacing( tmpF );

    int tmpI = state["console"]["line_size"];
    if(tmpI >= CON_MIN_LINE_SIZE && tmpI <= CON_MAX_LINE_SIZE)
        cn->setLineSize( tmpI );

    tmpI = state["console"]["showing_lines"];
    if(tmpI >= CON_MIN_LINES && tmpI <= CON_MIN_LINES)
        cn->setVisibleLines( tmpI );

    tmpI = state["console"]["log_size"];
    if(tmpI >= CON_MIN_LOG && tmpI <= CON_MAX_LOG)
        cn->setHistorySize( tmpI );

    tmpI = state["console"]["lines_count"];
    if(tmpI >= CON_MIN_LOG && tmpI <= CON_MAX_LOG)
        cn->setBufferSize( tmpI );

    bool tmpB = state["console"]["show"];
    cn->setVisible( tmpB );

    tmpF = state["console"]["show_cursor_period"];
    cn->setShowCursorPeriod( tmpF );
}
