
#include <stdlib.h>
#include <stdio.h>

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include "core/system.h"
#include "core/console.h"
#include "core/vmath.h"
#include "core/polygon.h"
#include "core/obb.h"
#include "render/camera.h"
#include "render/frustum.h"
#include "render/render.h"
#include "gui/gui_inventory.h"
#include "script/script.h"
#include "vt/tr_versions.h"
#include "audio/audio.h"
#include "engine.h"
#include "controls.h"
#include "room.h"
#include "world.h"
#include "game.h"
#include "skeletal_model.h"
#include "entity.h"
#include "trigger.h"
#include "character_controller.h"
#include "gameflow.h"
#include "inventory.h"

extern lua_State *engine_lua;

int Save_Entity(entity_p ent, void *data);

int lua_mlook(lua_State * lua)
{
    if(lua_gettop(lua) == 0)
    {
        control_states.mouse_look = !control_states.mouse_look;
        Con_Printf("mlook = %d", control_states.mouse_look);
        return 0;
    }

    control_states.mouse_look = lua_tointeger(lua, 1);
    Con_Printf("mlook = %d", control_states.mouse_look);
    return 0;
}


int lua_freelook(lua_State * lua)
{
    if(lua_gettop(lua) == 0)
    {
        control_states.free_look = !control_states.free_look;
        Con_Printf("free_look = %d", control_states.free_look);
        return 0;
    }

    control_states.free_look = lua_tointeger(lua, 1);
    Con_Printf("free_look = %d", control_states.free_look);
    return 0;
}


int lua_cam_distance(lua_State * lua)
{
    if(lua_gettop(lua) == 0)
    {
        Con_Printf("cam_distance = %.2f", control_states.cam_distance);
        return 0;
    }

    control_states.cam_distance = lua_tonumber(lua, 1);
    Con_Printf("cam_distance = %.2f", control_states.cam_distance);
    return 0;
}


int lua_noclip(lua_State * lua)
{
    if(lua_gettop(lua) == 0)
    {
        control_states.noclip = !control_states.noclip;
    }
    else
    {
        control_states.noclip = lua_tointeger(lua, 1);
    }

    Con_Printf("noclip = %d", control_states.noclip);
    return 0;
}


void Game_InitGlobals()
{
    control_states.free_look_speed = 3000.0;
    control_states.mouse_look = 1;
    control_states.free_look = 0;
    control_states.noclip = 0;
    control_states.cam_distance = 800.0;
}


void Game_RegisterLuaFunctions(struct lua_State *lua)
{
    if(lua != NULL)
    {
        lua_register(lua, "mlook", lua_mlook);
        lua_register(lua, "freelook", lua_freelook);
        lua_register(lua, "cam_distance", lua_cam_distance);
        lua_register(lua, "noclip", lua_noclip);
    }
}


/**
 * Load game state
 */
int Game_Load(const char* name)
{
    FILE *f;
    char *ch, local;

    local = 1;
    for(ch = (char*)name; *ch; ch++)
    {
        if((*ch == '\\') || (*ch == '/'))
        {
            local = 0;
            break;
        }
    }

    if(local)
    {
        char save_path[1024];
        size_t save_path_base_len = sizeof(save_path) - 1;
        strncpy(save_path, Engine_GetBasePath(), save_path_base_len);
        save_path[save_path_base_len] = 0;
        strncat(save_path, "save/", save_path_base_len - strlen(save_path));
        strncat(save_path, name, save_path_base_len - strlen(save_path));
        if(!Sys_FileFound(save_path, 0))
        {
            Sys_extWarn("Can not read file \"%s\"", save_path);
            return 0;
        }
        Script_LuaClearTasks();
        luaL_dofile(engine_lua, save_path);
    }
    else
    {
        f = fopen(name, "rb");
        if(f == NULL)
        {
            Sys_extWarn("Can not read file \"%s\"", name);
            return 0;
        }
        fclose(f);
        Script_LuaClearTasks();
        luaL_dofile(engine_lua, name);
    }

    return 1;
}

/**
 * Entity save function, based on engine lua scripts;
 */
int Save_Entity(entity_p ent, void *data)
{
    if(ent)
    {
        FILE **f = (FILE**)data;
        if(ent->type_flags & ENTITY_TYPE_SPAWNED)
        {
            uint32_t room_id = (ent->self->room) ? (ent->self->room->id) : (0xFFFFFFFF);
            fprintf(*f, "\nspawnEntity(%d, 0x%X, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %d);", ent->bf->animations.model->id, room_id,
                    ent->transform[12 + 0], ent->transform[12 + 1], ent->transform[12 + 2],
                    ent->angles[0], ent->angles[1], ent->angles[2], ent->id);
        }
        else
        {
            fprintf(*f, "\nsetEntityPos(%d, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f);", ent->id,
                    ent->transform[12 + 0], ent->transform[12 + 1], ent->transform[12 + 2],
                    ent->angles[0], ent->angles[1], ent->angles[2]);
        }

        if(ent->self->room)
        {
            fprintf(*f, "\nsetEntityRoomMove(%d, %d, %d, %d);", ent->id, ent->self->room->id, ent->move_type, ent->dir_flag);
        }
        else
        {
            fprintf(*f, "\nsetEntityRoomMove(%d, nil, %d, %d);", ent->id, ent->move_type, ent->dir_flag);
        }

        if(ent->bf->animations.model && ent->character)
        {
            fprintf(*f, "\nsetEntityBaseAnimModel(%d, %d);", ent->id, ent->bf->animations.model->id);
        }

        if(ent->activation_point)
        {
            activation_point_p ap = ent->activation_point;
            fprintf(*f, "\nsetEntityActivationOffset(%d, %.4f, %.4f, %.4f, %.4f);", ent->id, ap->offset[0], ap->offset[1], ap->offset[2], ap->offset[3]);
            fprintf(*f, "\nsetEntityActivationDirection(%d, %.4f, %.4f, %.4f, %.4f);", ent->id, ap->direction[0], ap->direction[1], ap->direction[2], ap->direction[3]);
        }

        for(uint16_t i = 0; i < ent->bf->bone_tag_count; ++i)
        {
            if(ent->bf->bone_tags[i].is_hidden)
            {
                fprintf(*f, "\nsetEntityBoneVisibility(%d, %d, false);", ent->id, i);
            }
        }

        char save_buff[32768] = {0};
        if(Script_GetEntitySaveData(engine_lua, ent->id, save_buff, sizeof(save_buff)) > 0)
        {
            fprintf(*f, "\n%s", save_buff);
        }

        ss_animation_p ss_anim = &ent->bf->animations;
        for(; ss_anim->next; ss_anim = ss_anim->next);

        for(; ss_anim; ss_anim = ss_anim->prev)
        {
            if(ss_anim->type != ANIM_TYPE_BASE)
            {
                if(ss_anim->model)
                {
                    fprintf(*f, "\nentitySSAnimEnsureExists(%d, %d, %d);", ent->id, ss_anim->type, ss_anim->model->id);
                }
                else
                {
                    fprintf(*f, "\nentitySSAnimEnsureExists(%d, %d, nil);", ent->id, ss_anim->type);
                }
            }
        }

        fprintf(*f, "\nremoveAllItems(%d);", ent->id);
        for(inventory_node_p i = ent->inventory; i; i = i->next)
        {
            fprintf(*f, "\naddItem(%d, %d, %d);", ent->id, i->id, i->count);
        }

        if(ent->character)
        {
            fprintf(*f, "\nsetCharacterClimbPoint(%d, %.2f, %.2f, %.2f);", ent->id,
                    ent->character->climb.point[0], ent->character->climb.point[1], ent->character->climb.point[2]);
            if(ent->character->target_id != ENTITY_ID_NONE)
            {
                fprintf(*f, "\nsetCharacterTarget(%d, %d);", ent->id, ent->character->target_id);
            }
            else
            {
                fprintf(*f, "\nsetCharacterTarget(%d);", ent->id);
            }

            fprintf(*f, "\nsetCharacterWeaponModel(%d, %d, %d);", ent->id, ent->character->current_weapon, ent->character->weapon_current_state);
            for(int i = 0; i < PARAM_LASTINDEX; i++)
            {
                fprintf(*f, "\nsetCharacterParam(%d, %d, %.2f, %.2f);", ent->id, i, ent->character->parameters.param[i], ent->character->parameters.maximum[i]);
            }
        }

        fprintf(*f, "\nsetEntityLinearSpeed(%d, %.2f);", ent->id, ent->linear_speed);
        fprintf(*f, "\nsetEntitySpeed(%d, %.2f, %.2f, %.2f);", ent->id, ent->speed[0], ent->speed[1], ent->speed[2]);

        fprintf(*f, "\nsetEntityFlags(%d, 0x%.4X, 0x%.4X, 0x%.8X);", ent->id, ent->state_flags, ent->type_flags, ent->callback_flags);
        fprintf(*f, "\nsetEntityCollisionFlags(%d, %d, %d, %d);", ent->id, ent->self->collision_group, ent->self->collision_shape, ent->self->collision_mask);
        fprintf(*f, "\nsetEntityTriggerLayout(%d, 0x%.2X);", ent->id, ent->trigger_layout);
        fprintf(*f, "\nsetEntityTimer(%d, %.3f);", ent->id, ent->timer);

        for(ss_anim = &ent->bf->animations; ss_anim; ss_anim = ss_anim->next)
        {
            if(ss_anim->model)
            {
                fprintf(*f, "\nsetEntityAnim(%d, %d, %d, %d, %d, %d);", ent->id, ss_anim->type, ss_anim->next_animation, ss_anim->next_frame, ss_anim->current_animation, ss_anim->current_frame);
                fprintf(*f, "\nsetEntityAnimStateHeavy(%d, %d, %d);", ent->id, ss_anim->type, ss_anim->next_state_heavy);
                fprintf(*f, "\nsetEntityAnimState(%d, %d, %d);", ent->id, ss_anim->type, ss_anim->next_state);
                fprintf(*f, "\nentitySSAnimSetTarget(%d, %d, %d, %.2f, %.2f, %.2f, %.6f, %.6f, %.6f);", ent->id, ss_anim->type, ss_anim->targeting_bone,
                    ss_anim->target[0], ss_anim->target[1], ss_anim->target[2],
                    ss_anim->bone_direction[0], ss_anim->bone_direction[1], ss_anim->bone_direction[2]);
                fprintf(*f, "\nentitySSAnimSetAxisMod(%d, %d, %.6f, %.6f, %.6f);", ent->id, ss_anim->type,
                    ss_anim->targeting_axis_mod[0], ss_anim->targeting_axis_mod[1], ss_anim->targeting_axis_mod[2]);
                fprintf(*f, "\nentitySSAnimSetTargetingLimit(%d, %d, %.6f, %.6f, %.6f, %.6f);", ent->id, ss_anim->type,
                    ss_anim->targeting_limit[0], ss_anim->targeting_limit[1], ss_anim->targeting_limit[2], ss_anim->targeting_limit[3]);
                fprintf(*f, "\nentitySSAnimSetCurrentRotation(%d, %d, %.6f, %.6f, %.6f, %.6f);", ent->id, ss_anim->type,
                    ss_anim->current_mod[0], ss_anim->current_mod[1], ss_anim->current_mod[2], ss_anim->current_mod[3]);
                fprintf(*f, "\nentitySSAnimSetExtFlags(%d, %d, %d, %d, %d);", ent->id, ss_anim->type, ss_anim->enabled,
                    ss_anim->anim_ext_flags, ss_anim->targeting_flags);
                fprintf(*f, "\nentitySSAnimSetEnable(%d, %d, %d);", ent->id, ss_anim->type, ss_anim->enabled);
            }
        }

        if(ent->no_fix_all)
        {
            fprintf(*f, "\nnoFixEntityCollision(%d, true);", ent->id);
        }
        if(ent->no_move)
        {
            fprintf(*f, "\nnoEntityMove(%d, true);", ent->id);
        }
    }

    return 0;
}

/**
 * Save current game state
 */
int Game_Save(const char* name)
{
    FILE *f;
    char local;

    local = 1;
    for(const char *ch = name; *ch; ch++)
    {
        if((*ch == '\\') || (*ch == '/'))
        {
            local = 0;
            break;
        }
    }

    if(local)
    {
        char save_path[1024];
        size_t save_path_base_len = sizeof(save_path) - 1;
        strncpy(save_path, Engine_GetBasePath(), save_path_base_len);
        save_path[save_path_base_len] = 0;
        strncat(save_path, "save/", save_path_base_len - strlen(save_path));
        strncat(save_path, name, save_path_base_len - strlen(save_path));
        f = fopen(save_path, "wb");
    }
    else
    {
        f = fopen(name, "wb");
    }

    if(!f)
    {
        Sys_extWarn("Can not create file \"%s\"", name);
        return 0;
    }

    fprintf(f, "loadMap(\"%s\", %d, %d);\n", Gameflow_GetCurrentLevelPathLocal(), Gameflow_GetCurrentGameID(), Gameflow_GetCurrentLevelID());

    // Save flipmap and flipped room states.
    uint8_t *flip_map;
    uint8_t *flip_state;
    uint32_t flip_count;
    World_GetFlipInfo(&flip_map, &flip_state, &flip_count);
    for(uint32_t i = 0; i < flip_count; i++)
    {
        fprintf(f, "setFlipMap(%d, 0x%02X, 0);\n", i, flip_map[i]);
        fprintf(f, "setFlipState(%d, %d);\n", i, flip_state[i]);
    }
    if(World_GetVersion() < TR_IV)
    {
        fprintf(f, "setGlobalFlipState(%d);\n", (int)World_GetGlobalFlipState());
    }

    int id = 0;
    room_p r = World_GetRoomByID(id);
    while(r)
    {
        if(r->alternate_room_next || r->alternate_room_prev)
        {
            fprintf(f, "setRoomActiveContent(%d, %d);\n", id, r->content->original_room_id);
        }
        r = World_GetRoomByID(++id);
    }

    char save_buffer[32768] = {0};
    if(Script_GetFlipEffectsSaveData(engine_lua, save_buffer, sizeof(save_buffer)) > 0)
    {
        fprintf(f, "\n%s\n", save_buffer);
    }

    World_IterateAllEntities(&Save_Entity, &f);

    fclose(f);

    return 1;
}


void Game_ApplyControls(struct entity_s *ent)
{
    int8_t move_logic[3];
    int8_t look_logic[3];

    // Keyboard move logic

    move_logic[0] = control_states.move_forward - control_states.move_backward;
    move_logic[1] = control_states.move_right - control_states.move_left;
    move_logic[2] = control_states.move_up - control_states.move_down;

    // Keyboard look logic

    look_logic[0] = control_states.look_left - control_states.look_right;
    look_logic[1] = control_states.look_down - control_states.look_up;
    look_logic[2] = control_states.look_roll_right - control_states.look_roll_left;

    // APPLY CONTROLS

    control_states.cam_angles[0] += 2.2 * engine_frame_time * look_logic[0];
    control_states.cam_angles[1] += 2.2 * engine_frame_time * look_logic[1];
    control_states.cam_angles[2] += 2.2 * engine_frame_time * look_logic[2];

    if(!World_GetRoomByID(0))
    {
        if(control_mapper.use_joy)
        {
            if(control_mapper.joy_look_x != 0)
            {
                control_states.cam_angles[0] -=0.015 * engine_frame_time * control_mapper.joy_look_x;

            }
            if(control_mapper.joy_look_y != 0)
            {
                control_states.cam_angles[1] -=0.015 * engine_frame_time * control_mapper.joy_look_y;
            }
        }

        if(control_states.mouse_look != 0)
        {
            control_states.cam_angles[0] -= 0.015 * control_states.look_axis_x;
            control_states.cam_angles[1] -= 0.015 * control_states.look_axis_y;
            control_states.look_axis_x = 0.0;
            control_states.look_axis_y = 0.0;
        }

        Cam_SetRotation(&engine_camera, control_states.cam_angles);
        float dist = (control_states.state_walk) ? (control_states.free_look_speed * engine_frame_time * 0.3f) : (control_states.free_look_speed * engine_frame_time);
        Cam_MoveAlong(&engine_camera, dist * move_logic[0]);
        Cam_MoveStrafe(&engine_camera, dist * move_logic[1]);
        Cam_MoveVertical(&engine_camera, dist * move_logic[2]);

        return;
    }

    if(control_mapper.use_joy)
    {
        if(control_mapper.joy_look_x != 0)
        {
            control_states.cam_angles[0] -=engine_frame_time * control_mapper.joy_look_x;
        }
        if(control_mapper.joy_look_y != 0)
        {
            control_states.cam_angles[1] -=engine_frame_time * control_mapper.joy_look_y;
        }
    }

    if(control_states.mouse_look != 0)
    {
        control_states.cam_angles[0] -= 0.015 * control_states.look_axis_x;
        control_states.cam_angles[1] -= 0.015 * control_states.look_axis_y;
        control_states.look_axis_x = 0.0;
        control_states.look_axis_y = 0.0;
    }

    if(control_states.free_look || !ent || !ent->character)
    {
        float dist = (control_states.state_walk) ? (control_states.free_look_speed * engine_frame_time * 0.3f) : (control_states.free_look_speed * engine_frame_time);
        Cam_SetRotation(&engine_camera, control_states.cam_angles);
        Cam_MoveAlong(&engine_camera, dist * move_logic[0]);
        Cam_MoveStrafe(&engine_camera, dist * move_logic[1]);
        Cam_MoveVertical(&engine_camera, dist * move_logic[2]);
        engine_camera.current_room = World_FindRoomByPosCogerrence(engine_camera.gl_transform + 12, engine_camera.current_room);
    }
    else if(control_states.noclip)
    {
        float pos[3];
        float dist = (control_states.state_walk) ? (control_states.free_look_speed * engine_frame_time * 0.3f) : (control_states.free_look_speed * engine_frame_time);
        Cam_SetRotation(&engine_camera, control_states.cam_angles);
        Cam_MoveAlong(&engine_camera, dist * move_logic[0]);
        Cam_MoveStrafe(&engine_camera, dist * move_logic[1]);
        Cam_MoveVertical(&engine_camera, dist * move_logic[2]);
        engine_camera.current_room = World_FindRoomByPosCogerrence(engine_camera.gl_transform + 12, engine_camera.current_room);

        ent->angles[0] = 180.0 * control_states.cam_angles[0] / M_PI;
        pos[0] = engine_camera.gl_transform[12 + 0] + engine_camera.gl_transform[8 + 0] * control_states.cam_distance;
        pos[1] = engine_camera.gl_transform[12 + 1] + engine_camera.gl_transform[8 + 1] * control_states.cam_distance;
        pos[2] = engine_camera.gl_transform[12 + 2] + engine_camera.gl_transform[8 + 2] * control_states.cam_distance - 512.0f;
        vec3_copy(ent->transform + 12, pos);
        Entity_UpdateTransform(ent);
        Entity_UpdateRoomPos(ent);
        Entity_UpdateRigidBody(ent, 1);
        Entity_GhostUpdate(ent);
        Entity_FixPenetrations(ent, NULL, NULL, COLLISION_FILTER_CHARACTER);
    }
    else
    {
        // Apply controls to Lara
        ent->character->cmd.action = control_states.state_action;
        ent->character->cmd.ready_weapon = control_states.do_draw_weapon;
        ent->character->cmd.jump = control_states.do_jump;
        ent->character->cmd.shift = control_states.state_walk;

        ent->character->cmd.roll = ((control_states.move_forward && control_states.move_backward) || control_states.do_roll);

        // New commands only for TR3 and above
        ent->character->cmd.sprint = control_states.state_sprint;
        ent->character->cmd.crouch = control_states.state_crouch;

        if(control_states.use_small_medi)
        {
            Script_UseItem(engine_lua, ITEM_SMALL_MEDIPACK, ent->id);
            control_states.use_small_medi = 0;
        }

        if(control_states.use_big_medi)
        {
            Script_UseItem(engine_lua, ITEM_LARGE_MEDIPACK, ent->id);
            control_states.use_big_medi = 0;
        }

        if((control_mapper.use_joy == 1) && (control_mapper.joy_move_x != 0))
        {
            ent->character->cmd.rot[0] = (control_mapper.joy_move_x > 0) ? (-1) : (1);
        }
        else
        {
            ent->character->cmd.rot[0] = -move_logic[1];
        }

        if( (control_mapper.use_joy == 1) && (control_mapper.joy_move_y != 0 ) )
        {
            ent->character->cmd.rot[1] = (control_mapper.joy_move_y > 0) ? (-1) : (1);
        }
        else
        {
            ent->character->cmd.rot[1] = move_logic[0];
        }

        vec3_copy(ent->character->cmd.move, move_logic);
    }
}


int Game_UpdateEntity(entity_p ent, void *data)
{
    if(ent && (ent != World_GetPlayer()) && (!ent->self->room || (ent->self->room == ent->self->room->real_room)))
    {
        if(ent->character)
        {
            Character_Update(ent);
        }
        if(ent->state_flags & ENTITY_STATE_ENABLED)
        {
            Entity_ProcessSector(ent);
            Script_LoopEntity(engine_lua, ent);
        }
        Entity_Frame(ent, engine_frame_time);
        Entity_UpdateRigidBody(ent, ent->character != NULL);
        Entity_UpdateRoomPos(ent);
    }

    return 0;
}


void Game_Frame(float time)
{
    entity_p player = World_GetPlayer();

    // GUI and controls should be updated at all times!
    if(!Con_IsShown() && control_states.gui_inventory && main_inventory_manager)
    {
        if(player &&
          (main_inventory_manager->getCurrentState() == gui_InventoryManager::INVENTORY_DISABLED))
        {
            main_inventory_manager->setInventory(&player->inventory, player->id);
            main_inventory_manager->send(gui_InventoryManager::INVENTORY_OPEN);
        }
        if(main_inventory_manager->getCurrentState() == gui_InventoryManager::INVENTORY_IDLE)
        {
            main_inventory_manager->send(gui_InventoryManager::INVENTORY_CLOSE);
        }
    }

    // If console or inventory is active, only thing to update is audio.
    if(Con_IsShown() || main_inventory_manager->getCurrentState() != gui_InventoryManager::INVENTORY_DISABLED)
    {
        return;
    }

    // In game mode
    Script_DoTasks(engine_lua, time);

    // This must be called EVERY frame to max out smoothness.
    // Includes animations, camera movement, and so on.
    if(player && player->character)
    {
        if(engine_camera_state.state != CAMERA_STATE_FLYBY)
        {
            Game_ApplyControls(player);
        }
        else
        {
            memset(&player->character->cmd, 0x00, sizeof(player->character->cmd));
        }

        if(!control_states.noclip)
        {
            Character_Update(player);
            Script_LoopEntity(engine_lua, player);   ///@TODO: fix that hack (refactoring)
            if(player->character->target_id == ENTITY_ID_NONE)
            {
                entity_p target = Character_FindTarget(player);
                if(target)
                {
                    player->character->target_id = target->id;
                }
            }
            else if(player->character->weapon_current_state != WEAPON_STATE_HIDE)
            {
                entity_p target = World_GetEntityByID(player->character->target_id);
                if(!target || !Character_IsTargetAccessible(player, target))
                {
                    player->character->target_id = ENTITY_ID_NONE;
                }
            }
        }
        Entity_Frame(player, time);
        Entity_UpdateRigidBody(player, 1);
        Entity_UpdateRoomPos(player);
    }
    else if(control_states.free_look)
    {
        Game_ApplyControls(NULL);
    }

    if(control_states.look)
    {
        if(engine_camera_state.state == CAMERA_STATE_FLYBY)
        {
            engine_camera_state.state = CAMERA_STATE_NORMAL;
            Cam_SetFovAspect(&engine_camera, screen_info.fov, engine_camera.aspect);
        }
        engine_camera_state.time = 0.0f;
    }

    if(!control_states.noclip && !control_states.free_look)
    {
        entity_p target = World_GetEntityByID(engine_camera_state.target_id);
        target = (target) ? (target) : (player);
        if(engine_camera_state.state == CAMERA_STATE_FLYBY)
        {
            Cam_PlayFlyBy(&engine_camera_state, time);
        }
        else
        {
            if(engine_camera_state.sink)
            {
                if(engine_camera_state.move)
                {
                    Cam_MoveTo(&engine_camera, engine_camera_state.sink->pos, engine_frame_time * TR_METERING_SECTORSIZE * (float)engine_camera_state.move);
                }
                else
                {
                    vec3_copy(engine_camera.gl_transform + 12, engine_camera_state.sink->pos);
                }

                if(target)
                {
                    float pos[3];
                    if(target->character)
                    {
                        Mat4_vec3_mul(pos, target->transform, target->bf->bone_tags[target->character->bone_head].full_transform + 12);
                    }
                    else
                    {
                        vec3_copy(pos, target->transform + 12);
                    }
                    Cam_LookTo(&engine_camera, pos);
                }
            }
            else if(player)
            {
                engine_camera_state.entity_offset_x = 16.0f;
                engine_camera_state.entity_offset_z = 128.0f;
                Cam_FollowEntity(&engine_camera, &engine_camera_state, player);
                if(!control_states.look && target && (engine_camera_state.state == CAMERA_STATE_LOOK_AT))
                {
                    Character_LookAt(player, target->transform + 12);
                    Cam_LookTo(&engine_camera, target->transform + 12);
                }
                else
                {
                    Character_ClearLookAt(player);
                }
            }

            engine_camera_state.time -= engine_frame_time;
            if(engine_camera_state.time < 0.0f)
            {
                if(target && (engine_camera_state.state == CAMERA_STATE_LOOK_AT))
                {
                    target->state_flags |= ENTITY_STATE_NO_CAM_TARGETABLE;
                }
                engine_camera_state.state = CAMERA_STATE_NORMAL;
                engine_camera_state.time = 0.0f;
                engine_camera_state.move = 0;
                engine_camera_state.sink = NULL;
                engine_camera_state.target_id = ENTITY_ID_NONE;
                Cam_SetFovAspect(&engine_camera, screen_info.fov, engine_camera.aspect);
            }
        }
    }

    World_IterateAllEntities(Game_UpdateEntity, NULL);

    Physics_StepSimulation(time);

    Controls_RefreshStates();
    renderer.UpdateAnimTextures();
}


void Game_Prepare()
{
    entity_p player = World_GetPlayer();
    if(player && player->character)
    {
        // Set character values to default.

        Character_SetParamMaximum(player, PARAM_HEALTH , LARA_PARAM_HEALTH_MAX );
        Character_SetParam       (player, PARAM_HEALTH , LARA_PARAM_HEALTH_MAX );
        Character_SetParamMaximum(player, PARAM_AIR    , LARA_PARAM_AIR_MAX    );
        Character_SetParam       (player, PARAM_AIR    , LARA_PARAM_AIR_MAX    );
        Character_SetParamMaximum(player, PARAM_STAMINA, LARA_PARAM_STAMINA_MAX);
        Character_SetParam       (player, PARAM_STAMINA, LARA_PARAM_STAMINA_MAX);
        Character_SetParamMaximum(player, PARAM_WARMTH,  LARA_PARAM_WARMTH_MAX );
        Character_SetParam       (player, PARAM_WARMTH , LARA_PARAM_WARMTH_MAX );

        // Set character statistics to default.

        player->character->statistics.distance       = 0.0;
        player->character->statistics.ammo_used      = 0;
        player->character->statistics.hits           = 0;
        player->character->statistics.kills          = 0;
        player->character->statistics.medipacks_used = 0;
        player->character->statistics.saves_used     = 0;
        player->character->statistics.secrets_game   = 0;
        player->character->statistics.secrets_level  = 0;

        vec3_copy(engine_camera.gl_transform + 12, player->transform + 12);
        engine_camera.gl_transform[12 + 2] += player->character->height;
        engine_camera.ang[0] = player->angles[0] + 180.0f;
        engine_camera.current_room = player->self->room;
    }
    else
    {
        // If there is no character present, move default camera position to
        // the first room (useful for TR1-3 cutscene levels).
        room_p room = World_GetRoomByID(0);
        if(room)
        {
            engine_camera.gl_transform[12 + 0] = room->bb_max[0];
            engine_camera.gl_transform[12 + 1] = room->bb_max[1];
            engine_camera.gl_transform[12 + 2] = room->bb_max[2];
        }
    }

    // Set gameflow parameters to default.
    // Reset secret trigger map.
    Gameflow_ResetSecrets();///@UNIMPLEMENTED We should save the secrets to a save file prior to resetting!
}


void Game_PlayFlyBy(uint32_t sequence_id, int once)
{
    if(engine_camera_state.state != CAMERA_STATE_FLYBY)
    {
        for(flyby_camera_sequence_p s = World_GetFlyBySequences(); s; s = s->next)
        {
            if((s->start->sequence == (int)sequence_id) && (!once || !s->locked))
            {
                engine_camera_state.state = CAMERA_STATE_FLYBY;
                engine_camera_state.flyby = s;
                s->locked = (once != 0x00);
                engine_camera_state.time = 0.0f;
                break;
            }
        }
    }
}


void Game_SetCameraTarget(uint32_t entity_id)
{
    entity_p ent = World_GetEntityByID(entity_id);
    engine_camera_state.move = 0;
    engine_camera_state.target_id = entity_id;
    if(ent && !engine_camera_state.sink && !(ent->state_flags & ENTITY_STATE_NO_CAM_TARGETABLE))
    {
        engine_camera_state.state = CAMERA_STATE_LOOK_AT;
        engine_camera_state.time = 1.0f;
    }
}


void Game_SetCamera(uint32_t camera_id, int once, int move, float timer)
{
    static_camera_sink_p sink = World_GetStaticCameraSink(camera_id);
    if(sink && !sink->locked)
    {
        engine_camera_state.move = move;
        engine_camera_state.time = timer;
        engine_camera_state.sink = sink;
        sink->locked |= 0x01 & once;
        engine_camera_state.state = CAMERA_STATE_FIXED;
        if(engine_camera_state.target_id == ENTITY_ID_NONE)
        {
            engine_camera_state.move = 4;
        }
    }
}


void Game_StopFlyBy()
{
    engine_camera_state.state = CAMERA_STATE_NORMAL;
    engine_camera_state.flyby = NULL;
    Cam_SetFovAspect(&engine_camera, screen_info.fov, engine_camera.aspect);
}
