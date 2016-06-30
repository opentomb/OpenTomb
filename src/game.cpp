
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
#include "core/redblack.h"
#include "core/polygon.h"
#include "core/obb.h"
#include "render/camera.h"
#include "render/frustum.h"
#include "render/render.h"
#include "engine.h"
#include "physics.h"
#include "controls.h"
#include "room.h"
#include "world.h"
#include "game.h"
#include "audio.h"
#include "skeletal_model.h"
#include "entity.h"
#include "script.h"
#include "trigger.h"
#include "anim_state_control.h"
#include "character_controller.h"
#include "gameflow.h"
#include "gui.h"
#include "inventory.h"

extern lua_State *engine_lua;

void Save_EntityTree(FILE **f, RedBlackNode_p n);
void Save_Entity(FILE **f, entity_p ent);
void Cam_PlayFlyBy(float time);


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


int lua_debuginfo(lua_State * lua)
{
    if(lua_gettop(lua) == 0)
    {
        screen_info.debug_view_state++;
    }
    else
    {
        screen_info.debug_view_state = lua_tointeger(lua, 1);
    }
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


void Game_RegisterLuaFunctions(lua_State *lua)
{
    if(lua != NULL)
    {
        lua_register(lua, "debuginfo", lua_debuginfo);
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
        char token[512];
        snprintf(token, 512, "save/%s", name);
        f = fopen(token, "rb");
        if(f == NULL)
        {
            Sys_extWarn("Can not read file \"%s\"", token);
            return 0;
        }
        fclose(f);
        Script_LuaClearTasks();
        luaL_dofile(engine_lua, token);
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


void Save_EntityTree(FILE **f, RedBlackNode_p n)
{
    if(n->left != NULL)
    {
        Save_EntityTree(f, n->left);
    }
    Save_Entity(f, (entity_p)n->data);
    if(n->right != NULL)
    {
        Save_EntityTree(f, n->right);
    }
}

/**
 * Entity save function, based on engine lua scripts;
 */
void Save_Entity(FILE **f, entity_p ent)
{
    if(ent == NULL)
    {
        return;
    }

    if(ent->type_flags & ENTITY_TYPE_SPAWNED)
    {
        uint32_t room_id = (ent->self->room)?(ent->self->room->id):(0xFFFFFFFF);
        fprintf(*f, "\nspawnEntity(%d, 0x%X, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %d);", ent->bf->animations.model->id, room_id,
                ent->transform[12+0], ent->transform[12+1], ent->transform[12+2],
                ent->angles[0], ent->angles[1], ent->angles[2], ent->id);
    }
    else
    {
        fprintf(*f, "\nsetEntityPos(%d, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f);", ent->id,
                ent->transform[12+0], ent->transform[12+1], ent->transform[12+2],
                ent->angles[0], ent->angles[1], ent->angles[2]);
    }

    fprintf(*f, "\nsetEntitySpeed(%d, %.2f, %.2f, %.2f);", ent->id, ent->speed[0], ent->speed[1], ent->speed[2]);
    fprintf(*f, "\nsetEntityAnim(%d, %d, %d);", ent->id, ent->bf->animations.current_animation, ent->bf->animations.current_frame);
    fprintf(*f, "\nsetEntityState(%d, %d, %d);", ent->id, ent->bf->animations.next_state, ent->bf->animations.last_state);

    fprintf(*f, "\nsetEntityFlags(%d, 0x%.4X, 0x%.4X, 0x%.8X);", ent->id, ent->state_flags, ent->type_flags, ent->callback_flags);
    fprintf(*f, "\nsetEntityCollisionFlags(%d, %d, %d);", ent->id, ent->self->collision_type, ent->self->collision_shape);
    fprintf(*f, "\nsetEntityTriggerLayout(%d, 0x%.2X);", ent->id, ent->trigger_layout);
    //setEntityMeshswap()

    if(ent->self->room != NULL)
    {
        fprintf(*f, "\nsetEntityRoomMove(%d, %d, %d, %d);", ent->id, ent->self->room->id, ent->move_type, ent->dir_flag);
    }
    else
    {
        fprintf(*f, "\nsetEntityRoomMove(%d, nil, %d, %d);", ent->id, ent->move_type, ent->dir_flag);
    }

    if(ent->character != NULL)
    {
        fprintf(*f, "\nremoveAllItems(%d);", ent->id);
        for(inventory_node_p i = ent->character->inventory; i; i = i->next)
        {
            fprintf(*f, "\naddItem(%d, %d, %d);", ent->id, i->id, i->count);
        }

        for(int i = 0; i < PARAM_LASTINDEX; i++)
        {
            fprintf(*f, "\nsetCharacterParam(%d, %d, %.2f, %.2f);", ent->id, i, ent->character->parameters.param[i], ent->character->parameters.maximum[i]);
        }
    }
}

/**
 * Save current game state
 */
int Game_Save(const char* name)
{
    FILE *f;
    char local, *ch, token[512];

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
        snprintf(token, 512, "save/%s", name);
        f = fopen(token, "wb");
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

    fprintf(f, "loadMap(\"%s\", %d, %d);\n", gameflow_manager.CurrentLevelPath, gameflow_manager.CurrentGameID, gameflow_manager.CurrentLevelID);

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

    Save_Entity(&f, World_GetPlayer());    // Save Lara.

    RedBlackNode_p root = World_GetEntityTreeRoot();
    if(root)
    {
        Save_EntityTree(&f, root);
    }
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
        float dist = (control_states.state_walk)?(control_states.free_look_speed * engine_frame_time * 0.3):(control_states.free_look_speed * engine_frame_time);
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

    if((control_states.free_look != 0) || !ent || !ent->character)
    {
        float dist = (control_states.state_walk)?(control_states.free_look_speed * engine_frame_time * 0.3):(control_states.free_look_speed * engine_frame_time);
        Cam_SetRotation(&engine_camera, control_states.cam_angles);
        Cam_MoveAlong(&engine_camera, dist * move_logic[0]);
        Cam_MoveStrafe(&engine_camera, dist * move_logic[1]);
        Cam_MoveVertical(&engine_camera, dist * move_logic[2]);
        engine_camera.current_room = World_FindRoomByPosCogerrence(engine_camera.pos, engine_camera.current_room);
    }
    else if(control_states.noclip != 0)
    {
        float pos[3];
        float dist = (control_states.state_walk)?(control_states.free_look_speed * engine_frame_time * 0.3):(control_states.free_look_speed * engine_frame_time);
        Cam_SetRotation(&engine_camera, control_states.cam_angles);
        Cam_MoveAlong(&engine_camera, dist * move_logic[0]);
        Cam_MoveStrafe(&engine_camera, dist * move_logic[1]);
        Cam_MoveVertical(&engine_camera, dist * move_logic[2]);
        engine_camera.current_room = World_FindRoomByPosCogerrence(engine_camera.pos, engine_camera.current_room);

        ent->angles[0] = 180.0 * control_states.cam_angles[0] / M_PI;
        pos[0] = engine_camera.pos[0] + engine_camera.view_dir[0] * control_states.cam_distance;
        pos[1] = engine_camera.pos[1] + engine_camera.view_dir[1] * control_states.cam_distance;
        pos[2] = engine_camera.pos[2] + engine_camera.view_dir[2] * control_states.cam_distance - 512.0;
        vec3_copy(ent->transform+12, pos);
        Entity_UpdateTransform(ent);
        Entity_UpdateRigidBody(ent, 1);
        Entity_GhostUpdate(ent);
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
            if((Inventory_GetItemsCount(ent->character->inventory, ITEM_SMALL_MEDIPACK) > 0) &&
               (Character_ChangeParam(ent, PARAM_HEALTH, 250)))
            {
                Inventory_RemoveItem(&ent->character->inventory, ITEM_SMALL_MEDIPACK, 1);
                Audio_Send(TR_AUDIO_SOUND_MEDIPACK);
            }

            control_states.use_small_medi = !control_states.use_small_medi;
        }

        if(control_states.use_big_medi)
        {
            if((Inventory_GetItemsCount(ent->character->inventory, ITEM_LARGE_MEDIPACK) > 0) &&
               (Character_ChangeParam(ent, PARAM_HEALTH, LARA_PARAM_HEALTH_MAX)))
            {
                Inventory_RemoveItem(&ent->character->inventory, ITEM_LARGE_MEDIPACK, 1);
                Audio_Send(TR_AUDIO_SOUND_MEDIPACK);
            }

            control_states.use_big_medi = !control_states.use_big_medi;
        }

        if((control_mapper.use_joy == 1) && (control_mapper.joy_move_x != 0 ))
        {
            ent->character->cmd.rot[0] = (control_mapper.joy_move_x > 0) ? (-1) : (1);
        }
        else
        {
            ent->character->cmd.rot[0] = -move_logic[1];
        }

        if( (control_mapper.use_joy == 1) && (control_mapper.joy_move_y != 0 ) )
        {
            ent->character->cmd.rot[1] = (control_mapper.joy_move_y > 0) ? (1) : (-1);
        }
        else
        {
            ent->character->cmd.rot[1] = move_logic[0];
        }

        vec3_copy(ent->character->cmd.move, move_logic);
    }
}


void Cam_PlayFlyBy(float time)
{
    if(engine_camera_state.state == CAMERA_STATE_FLYBY)
    {
        const float max_time = engine_camera_state.flyby->pos_x->base_points_count - 1;
        float speed = Spline_Get(engine_camera_state.flyby->speed, engine_camera_state.time);
        engine_camera_state.time += time * speed / (1024.0f + 512.0f);
        if(engine_camera_state.time <= max_time)
        {
            FlyBySequence_SetCamera(engine_camera_state.flyby, &engine_camera, engine_camera_state.time);
        }
        else
        {
            engine_camera_state.state = CAMERA_STATE_NORMAL;
            engine_camera_state.flyby = NULL;
            engine_camera_state.time = 0.0f;
            Cam_SetFovAspect(&engine_camera, screen_info.fov, engine_camera.aspect);
        }
    }
}


void Cam_FollowEntity(struct camera_s *cam, struct entity_s *ent, float dx, float dz)
{
    float cam_pos[3], cameraFrom[3], cameraTo[3];
    collision_result_t cb;
    entity_p target = World_GetEntityByID(engine_camera_state.target_id);

    if(target && (engine_camera_state.state == CAMERA_STATE_FIXED))
    {
        cam->pos[0] = engine_camera_state.sink->x;
        cam->pos[1] = engine_camera_state.sink->y;
        cam->pos[2] = engine_camera_state.sink->z;
        cam->current_room = World_GetRoomByID(engine_camera_state.sink->room_or_strength);

        if(target->character)
        {
            ss_bone_tag_p btag = target->bf->bone_tags;
            float target_pos[3];
            for(uint16_t i = 0; i < target->bf->bone_tag_count; i++)
            {
                if(target->bf->bone_tags[i].body_part & BODY_PART_HEAD)
                {
                    btag = target->bf->bone_tags + i;
                    break;
                }
            }
            Mat4_vec3_mul(target_pos, target->transform, btag->full_transform + 12);
            Cam_LookTo(cam, target_pos);
        }
        else
        {
            Cam_LookTo(cam, target->transform + 12);
        }

        engine_camera_state.time -= engine_frame_time;
        if(engine_camera_state.time <= 0.0f)
        {
            entity_p player = World_GetPlayer();
            engine_camera_state.state = CAMERA_STATE_NORMAL;
            engine_camera_state.time = 0.0f;
            engine_camera_state.sink = NULL;
            engine_camera_state.target_id = (player) ? (player->id) : (-1);
            Cam_SetFovAspect(cam, screen_info.fov, cam->aspect);
        }
        return;
    }

    vec3_copy(cam_pos, cam->pos);
    ///@INFO Basic camera override, completely placeholder until a system classic-like is created
    if(control_states.mouse_look == 0)//If mouse look is off
    {
        float currentAngle = control_states.cam_angles[0] * (M_PI / 180.0);     //Current is the current cam angle
        float targetAngle  = ent->angles[0] * (M_PI / 180.0); //Target is the target angle which is the entity's angle itself
        float rotSpeed = 2.0; //Speed of rotation

        ///@FIXME
        //If Lara is in a specific state we want to rotate -75 deg or +75 deg depending on camera collision
        if(ent->bf->animations.last_state == TR_STATE_LARA_REACH)
        {
            if(cam->target_dir == TR_CAM_TARG_BACK)
            {
                vec3_copy(cameraFrom, cam_pos);
                cameraTo[0] = cameraFrom[0] + sinf((ent->angles[0] - 90.0) * (M_PI / 180.0)) * control_states.cam_distance;
                cameraTo[1] = cameraFrom[1] - cosf((ent->angles[0] - 90.0) * (M_PI / 180.0)) * control_states.cam_distance;
                cameraTo[2] = cameraFrom[2];

                //If collided we want to go right otherwise stay left
                if(Physics_SphereTest(NULL, cameraFrom, cameraTo, 16.0f, ent->self))
                {
                    cameraTo[0] = cameraFrom[0] + sinf((ent->angles[0] + 90.0) * (M_PI / 180.0)) * control_states.cam_distance;
                    cameraTo[1] = cameraFrom[1] - cosf((ent->angles[0] + 90.0) * (M_PI / 180.0)) * control_states.cam_distance;
                    cameraTo[2] = cameraFrom[2];

                    //If collided we want to go to back else right
                    if(Physics_SphereTest(NULL, cameraFrom, cameraTo, 16.0f, ent->self))
                    {
                        cam->target_dir = TR_CAM_TARG_BACK;
                    }
                    else
                    {
                        cam->target_dir = TR_CAM_TARG_RIGHT;
                    }
                }
                else
                {
                    cam->target_dir = TR_CAM_TARG_LEFT;
                }
            }
        }
        else if(ent->bf->animations.last_state == TR_STATE_LARA_JUMP_BACK)
        {
            cam->target_dir = TR_CAM_TARG_FRONT;
        }
        else if(cam->target_dir != TR_CAM_TARG_BACK)
        {
            cam->target_dir = TR_CAM_TARG_BACK;//Reset to back
        }

        //If target mis-matches current we need to update the camera's angle to reach target!
        if(currentAngle != targetAngle)
        {
            switch(cam->target_dir)
            {
            case TR_CAM_TARG_BACK:
                targetAngle = (ent->angles[0]) * (M_PI / 180.0);
                break;
            case TR_CAM_TARG_FRONT:
                targetAngle = (ent->angles[0] - 180.0) * (M_PI / 180.0);
                break;
            case TR_CAM_TARG_LEFT:
                targetAngle = (ent->angles[0] - 75.0) * (M_PI / 180.0);
                break;
            case TR_CAM_TARG_RIGHT:
                targetAngle = (ent->angles[0] + 75.0) * (M_PI / 180.0);
                break;
            default:
                targetAngle = (ent->angles[0]) * (M_PI / 180.0);
                break;
            }

            float d_angle = control_states.cam_angles[0] - targetAngle;
            if(d_angle > M_PI / 2.0)
            {
                d_angle -= M_PI/180.0;
            }
            if(d_angle < -M_PI / 2.0)
            {
                d_angle += M_PI/180.0;
            }
            control_states.cam_angles[0] = fmodf(control_states.cam_angles[0] + atan2f(sinf(currentAngle - d_angle), cosf(currentAngle + d_angle)) * (engine_frame_time * rotSpeed), M_PI * 2.0); //Update camera's angle
        }
    }

    if((ent->character != NULL) && (ent->character->cam_follow_center > 0))
    {
        vec3_copy(cam_pos, ent->obb->centre);
        ent->character->cam_follow_center--;
    }
    else
    {
        Mat4_vec3_mul(cam_pos, ent->transform, ent->bf->bone_tags->full_transform+12);
        cam_pos[2] += dz;
    }

    //Code to manage screen shaking effects
    /*if((engine_camera_state.time > 0.0) && (engine_camera_state.shake_value > 0.0))
    {
        cam_pos[0] += ((rand() % abs(engine_camera_state.shake_value)) - (engine_camera_state.shake_value / 2)) * engine_camera_state.time;;
        cam_pos[1] += ((rand() % abs(engine_camera_state.shake_value)) - (engine_camera_state.shake_value / 2)) * engine_camera_state.time;;
        cam_pos[2] += ((rand() % abs(engine_camera_state.shake_value)) - (engine_camera_state.shake_value / 2)) * engine_camera_state.time;;
        engine_camera_state.time  = (engine_camera_state.time < 0.0)?(0.0):(engine_camera_state.time)-engine_frame_time;
    }*/

    vec3_copy(cameraFrom, cam_pos);
    cam_pos[2] += dz;
    vec3_copy(cameraTo, cam_pos);

    if(Physics_SphereTest(&cb, cameraFrom, cameraTo, 16.0f, ent->self))
    {
        vec3_add_mul(cam_pos, cb.point, cb.normale, 2.0);
    }

    if (dx != 0.0)
    {
        vec3_copy(cameraFrom, cam_pos);
        cam_pos[0] += dx * cam->right_dir[0];
        cam_pos[1] += dx * cam->right_dir[1];
        cam_pos[2] += dx * cam->right_dir[2];
        vec3_copy(cameraTo, cam_pos);

        if(Physics_SphereTest(&cb, cameraFrom, cameraTo, 16.0f, ent->self))
        {
            vec3_add_mul(cam_pos, cb.point, cb.normale, 2.0);
        }

        vec3_copy(cameraFrom, cam_pos);
        if(engine_camera_state.state == CAMERA_STATE_LOOK_AT)
        {
            entity_p target = World_GetEntityByID(engine_camera_state.target_id);
            if(target && target != World_GetPlayer())
            {
                float dir2d[2], dist;
                dir2d[0] = target->transform[12 + 0] - cam->pos[0];
                dir2d[1] = target->transform[12 + 1] - cam->pos[1];
                dist = control_states.cam_distance / sqrtf(dir2d[0] * dir2d[0] + dir2d[1] * dir2d[1]);
                cam_pos[0] -= dir2d[0] * dist;
                cam_pos[1] -= dir2d[1] * dist;
            }
        }
        else
        {
            cam_pos[0] += sinf(control_states.cam_angles[0]) * control_states.cam_distance;
            cam_pos[1] -= cosf(control_states.cam_angles[0]) * control_states.cam_distance;
        }
        vec3_copy(cameraTo, cam_pos);

        if(Physics_SphereTest(&cb, cameraFrom, cameraTo, 16.0f, ent->self))
        {
            vec3_add_mul(cam_pos, cb.point, cb.normale, 2.0);
        }
    }

    //Update cam pos
    vec3_copy(cam->pos, cam_pos);

    //Modify cam pos for quicksand rooms
    cam->pos[2] -= 128.0;
    cam->current_room = World_FindRoomByPosCogerrence(cam->pos, cam->current_room);
    cam->pos[2] += 128.0;
    if((cam->current_room != NULL) && (cam->current_room->flags & TR_ROOM_FLAG_QUICKSAND))
    {
        cam->pos[2] = cam->current_room->bb_max[2] + 2.0 * 64.0;
    }

    if(engine_camera_state.state == CAMERA_STATE_LOOK_AT)
    {
        entity_p target = World_GetEntityByID(engine_camera_state.target_id);
        if(target)
        {
            Cam_LookTo(cam, target->transform + 12);
            engine_camera_state.time -= engine_frame_time;
            if(engine_camera_state.time <= 0.0f)
            {
                engine_camera_state.target_id = (World_GetPlayer()) ? (World_GetPlayer()->id) : (-1);
                engine_camera_state.state = CAMERA_STATE_NORMAL;
                engine_camera_state.time = 0.0f;
                engine_camera_state.sink = NULL;
            }
        }
    }
    else
    {
        Cam_SetRotation(cam, control_states.cam_angles);
    }
    cam->current_room = World_FindRoomByPosCogerrence(cam->pos, cam->current_room);
}


void Game_LoopEntities(struct RedBlackNode_s *x)
{
    entity_p entity = (entity_p)x->data;

    if(entity->state_flags & ENTITY_STATE_ENABLED)
    {
        Entity_ProcessSector(entity);
        Script_LoopEntity(engine_lua, entity->id);
    }

    if(x->left != NULL)
    {
        Game_LoopEntities(x->left);
    }
    if(x->right != NULL)
    {
        Game_LoopEntities(x->right);
    }
}


void Game_UpdateAllEntities(struct RedBlackNode_s *x)
{
    Entity_Frame((entity_p)x->data, engine_frame_time);
    Entity_UpdateRigidBody((entity_p)x->data, 0);

    if(x->left != NULL)
    {
        Game_UpdateAllEntities(x->left);
    }
    if(x->right != NULL)
    {
        Game_UpdateAllEntities(x->right);
    }
}


void Game_UpdateAI()
{
    entity_p ent = NULL;
    //for(ALL CHARACTERS, EXCEPT PLAYER)
    {
        if(ent)
        {
            // UPDATE AI commands
        }
    }
}


void Game_UpdateCharactersTree(struct RedBlackNode_s *x)
{
    entity_p ent = (entity_p)x->data;

    if(ent && ent->character)
    {
        if(ent->character->cmd.action && (ent->type_flags & ENTITY_TYPE_TRIGGER_ACTIVATOR))
        {
            Entity_CheckActivators(ent);
        }
        if(Character_GetParam(ent, PARAM_HEALTH) <= 0.0)
        {
            ent->character->resp.kill = 1;                                      // Kill, if no HP.
        }
        Character_ApplyCommands(ent);

        for(int h = 0; h < ent->character->hair_count; h++)
        {
            Hair_Update(ent->character->hairs[h], ent->physics);
        }
    }

    if(x->left != NULL)
    {
        Game_UpdateCharactersTree(x->left);
    }
    if(x->right != NULL)
    {
        Game_UpdateCharactersTree(x->right);
    }
}


void Game_UpdateCharacters()
{
    entity_p ent = World_GetPlayer();

    if(ent && ent->character)
    {
        if(ent->character->cmd.action && (ent->type_flags & ENTITY_TYPE_TRIGGER_ACTIVATOR))
        {
            Entity_CheckActivators(ent);
        }
        if(Character_GetParam(ent, PARAM_HEALTH) <= 0.0)
        {
            ent->character->resp.kill = 0;   // Kill, if no HP.
        }
        for(int h = 0; h < ent->character->hair_count; h++)
        {
            Hair_Update(ent->character->hairs[h], ent->physics);
        }
    }

    RedBlackNode_p root = World_GetEntityTreeRoot();
    if(root)
    {
        Game_UpdateCharactersTree(root);
    }
}


void Game_Frame(float time)
{
    entity_p player = World_GetPlayer();
    bool is_entitytree = (World_GetEntityTreeRoot() != NULL);
    bool is_character  = (player != NULL);

    // GUI and controls should be updated at all times!

    Gui_Update();

    ///@FIXME: I have no idea what's happening here! - Lwmte

    if(!Con_IsShown() && control_states.gui_inventory && main_inventory_manager)
    {
        if((is_character) &&
           (main_inventory_manager->getCurrentState() == gui_InventoryManager::INVENTORY_DISABLED))
        {
            main_inventory_manager->setInventory(&player->character->inventory);
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

    Script_DoTasks(engine_lua, time);
    Game_UpdateAI();
    if(is_character)
    {
        Entity_ProcessSector(player);
        Character_UpdateParams(player);
        Entity_CheckCollisionCallbacks(player);                                 ///@FIXME: Must do it for ALL interactive entities!
    }

    if(is_entitytree)
    {
        Game_LoopEntities(World_GetEntityTreeRoot());
    }

    // This must be called EVERY frame to max out smoothness.
    // Includes animations, camera movement, and so on.
    if(engine_camera_state.state != CAMERA_STATE_FLYBY)
    {
        Game_ApplyControls(player);
    }

    Cam_PlayFlyBy(time);

    if(is_character)
    {
        if(player->type_flags & ENTITY_TYPE_DYNAMIC)
        {
            Entity_UpdateRigidBody(player, 0);
        }
        if(!control_states.noclip && !control_states.free_look)
        {
            Character_ApplyCommands(player);
            Entity_Frame(player, engine_frame_time);
            if(engine_camera_state.state != CAMERA_STATE_FLYBY)
            {
                Cam_FollowEntity(&engine_camera, player, 16.0, 128.0);
            }
            if(engine_camera_state.state == CAMERA_STATE_LOOK_AT)
            {
                entity_p target = World_GetEntityByID(engine_camera_state.target_id);
                if(target)
                {
                    Character_LookAt(player, target->obb->centre);
                }
            }
            else
            {
                Character_ClearLookAt(player);
            }
        }
    }

    Game_UpdateCharacters();

    if(is_entitytree)
    {
        Game_UpdateAllEntities(World_GetEntityTreeRoot());
    }

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
    }
    else
    {
        // If there is no character present, move default camera position to
        // the first room (useful for TR1-3 cutscene levels).
        room_p room = World_GetRoomByID(0);
        if(room)
        {
            engine_camera.pos[0] = room->bb_max[0];
            engine_camera.pos[1] = room->bb_max[1];
            engine_camera.pos[2] = room->bb_max[2];
        }
    }

    // Set gameflow parameters to default.
    // Reset secret trigger map.
    memset(gameflow_manager.SecretsTriggerMap, 0, sizeof(gameflow_manager.SecretsTriggerMap));
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


void Game_SetCameraTarget(uint32_t entity_id, float timer)
{
    entity_p ent = World_GetEntityByID(entity_id);
    engine_camera_state.target_id = entity_id;
    if(ent && (engine_camera_state.state == CAMERA_STATE_NORMAL))
    {
        engine_camera_state.state = CAMERA_STATE_LOOK_AT;
        engine_camera_state.time = timer;
    }
}

// if timer == 0 then camera set is permanent
void Game_SetCamera(uint32_t camera_id, int once, int move, float timer)
{
    static_camera_sink_p sink = World_GetstaticCameraSink(camera_id);
    if(sink)
    {
        if(engine_camera_state.state != CAMERA_STATE_FLYBY)
        {
            engine_camera_state.state = CAMERA_STATE_FIXED;
            engine_camera_state.sink = sink;
            engine_camera_state.time = timer;
            engine_camera_state.move = move;
        }
    }
}


void Game_StopFlyBy()
{
    engine_camera_state.state = CAMERA_STATE_NORMAL;
    engine_camera_state.flyby = NULL;
    Cam_SetFovAspect(&engine_camera, screen_info.fov, engine_camera.aspect);
}


void Game_LevelTransition(uint16_t level_index)
{
    char file_path[MAX_ENGINE_PATH];
    Script_GetLoadingScreen(engine_lua, level_index, file_path);
    if(!Gui_LoadScreenAssignPic(file_path))
    {
        Gui_LoadScreenAssignPic("resource/graphics/legal.png");
    }
    Audio_EndStreams();
}
