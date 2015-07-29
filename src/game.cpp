
#include <stdlib.h>
#include <stdio.h>

#include "bullet/btBulletCollisionCommon.h"
#include "bullet/btBulletDynamicsCommon.h"
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
#include "engine.h"
#include "engine_lua.h"
#include "engine_bullet.h"
#include "controls.h"
#include "world.h"
#include "game.h"
#include "mesh.h"
#include "entity.h"
#include "camera.h"
#include "render.h"
#include "portal.h"
#include "script.h"
#include "anim_state_control.h"
#include "character_controller.h"
#include "gameflow.h"
#include "gui.h"
#include "inventory.h"
#include "hair.h"
#include "ragdoll.h"

btScalar cam_angles[3] = {0.0, 0.0, 0.0};

extern btScalar time_scale;
extern lua_State *engine_lua;

void Save_EntityTree(FILE **f, RedBlackNode_p n);
void Save_Entity(FILE **f, entity_p ent);

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
        screen_info.show_debuginfo = !screen_info.show_debuginfo;
    }
    else
    {
        screen_info.show_debuginfo = lua_tointeger(lua, 1);
    }

    Con_Printf("debug info = %d", screen_info.show_debuginfo);
    return 0;
}

int lua_timescale(lua_State * lua)
{
    if(lua_gettop(lua) == 0)
    {
        if(time_scale == 1.0)
        {
            time_scale = 0.033;
        }
        else
        {
            time_scale = 1.0;
        }
    }
    else
    {
        time_scale = lua_tonumber(lua, 1);
    }

    Con_Printf("time_scale = %.3f", time_scale);
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
        lua_register(lua, "noclip", lua_noclip);
        lua_register(lua, "cam_distance", lua_cam_distance);
        lua_register(lua, "timescale", lua_timescale);
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
    for(ch=(char*)name;*ch;ch++)
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
        Engine_LuaClearTasks();
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
        Engine_LuaClearTasks();
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
        fprintf(*f, "\nspawnEntity(%d, 0x%X, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %d);", ent->bf.animations.model->id, room_id,
                ent->transform[12+0], ent->transform[12+1], ent->transform[12+2],
                ent->angles[0], ent->angles[1], ent->angles[2], ent->id);
    }
    else
    {
        fprintf(*f, "\nsetEntityPos(%d, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f);", ent->id,
                ent->transform[12+0], ent->transform[12+1], ent->transform[12+2],
                ent->angles[0], ent->angles[1], ent->angles[2]);
    }

    fprintf(*f, "\nsetEntitySpeed(%d, %.2f, %.2f, %.2f);", ent->id, ent->speed.m_floats[0], ent->speed.m_floats[1], ent->speed.m_floats[2]);
    fprintf(*f, "\nsetEntityAnim(%d, %d, %d);", ent->id, ent->bf.animations.current_animation, ent->bf.animations.current_frame);
    fprintf(*f, "\nsetEntityState(%d, %d, %d);", ent->id, ent->bf.animations.next_state, ent->bf.animations.last_state);
    fprintf(*f, "\nsetEntityCollisionFlags(%d, %d, %d);", ent->id, ent->self->collision_type, ent->self->collision_shape);

    if(ent->state_flags & ENTITY_STATE_ENABLED)
    {
        fprintf(*f, "\nenableEntity(%d);", ent->id);
    }
    else
    {
        fprintf(*f, "\ndisableEntity(%d);", ent->id);
    }

    fprintf(*f, "\nsetEntityFlags(%d, 0x%.4X, 0x%.4X, 0x%.8X);", ent->id, ent->state_flags, ent->type_flags, ent->callback_flags);

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
        for(inventory_node_p i=ent->character->inventory;i!=NULL;i=i->next)
        {
            fprintf(*f, "\naddItem(%d, %d, %d);", ent->id, i->id, i->count);
        }

        for(int i=0;i<PARAM_LASTINDEX;i++)
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
    for(ch=(char*)name;*ch;ch++)
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

    for(int i=0; i < engine_world.flip_count; i++)
    {
        fprintf(f, "setFlipMap(%d, 0x%02X, 0);\n", i, engine_world.flip_map[i]);
        fprintf(f, "setFlipState(%d, %d);\n", i, engine_world.flip_state[i]);
    }

    Save_Entity(&f, engine_world.Character);    // Save Lara.

    if((engine_world.entity_tree != NULL) && (engine_world.entity_tree->root != NULL))
    {
        Save_EntityTree(&f, engine_world.entity_tree->root);
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

    cam_angles[0] += 2.2 * engine_frame_time * look_logic[0];
    cam_angles[1] += 2.2 * engine_frame_time * look_logic[1];
    cam_angles[2] += 2.2 * engine_frame_time * look_logic[2];

    if(!renderer.world)
    {
        if(control_mapper.use_joy)
        {
            if(control_mapper.joy_look_x != 0)
            {
                cam_angles[0] -=0.015 * engine_frame_time * control_mapper.joy_look_x;

            }
            if(control_mapper.joy_look_y != 0)
            {
                cam_angles[1] -=0.015 * engine_frame_time * control_mapper.joy_look_y;
            }
        }

        if(control_states.mouse_look != 0)
        {
            cam_angles[0] -= 0.015 * control_states.look_axis_x;
            cam_angles[1] -= 0.015 * control_states.look_axis_y;
            control_states.look_axis_x = 0.0;
            control_states.look_axis_y = 0.0;
        }

        Cam_SetRotation(renderer.cam, cam_angles);
        btScalar dist = (control_states.state_walk)?(control_states.free_look_speed * engine_frame_time * 0.3):(control_states.free_look_speed * engine_frame_time);
        Cam_MoveAlong(renderer.cam, dist * move_logic[0]);
        Cam_MoveStrafe(renderer.cam, dist * move_logic[1]);
        Cam_MoveVertical(renderer.cam, dist * move_logic[2]);

        return;
    }

    if(control_mapper.use_joy)
    {
        if(control_mapper.joy_look_x != 0)
        {
            cam_angles[0] -=engine_frame_time * control_mapper.joy_look_x;
        }
        if(control_mapper.joy_look_y != 0)
        {
            cam_angles[1] -=engine_frame_time * control_mapper.joy_look_y;
        }
    }

    if(control_states.mouse_look != 0)
    {
        cam_angles[0] -= 0.015 * control_states.look_axis_x;
        cam_angles[1] -= 0.015 * control_states.look_axis_y;
        control_states.look_axis_x = 0.0;
        control_states.look_axis_y = 0.0;
    }

    if((control_states.free_look != 0) || !IsCharacter(ent))
    {
        btScalar dist = (control_states.state_walk)?(control_states.free_look_speed * engine_frame_time * 0.3):(control_states.free_look_speed * engine_frame_time);
        Cam_SetRotation(renderer.cam, cam_angles);
        Cam_MoveAlong(renderer.cam, dist * move_logic[0]);
        Cam_MoveStrafe(renderer.cam, dist * move_logic[1]);
        Cam_MoveVertical(renderer.cam, dist * move_logic[2]);
        renderer.cam->current_room = Room_FindPosCogerrence(renderer.cam->pos, renderer.cam->current_room);
    }
    else if(control_states.noclip != 0)
    {
        btVector3 pos;
        btScalar dist = (control_states.state_walk)?(control_states.free_look_speed * engine_frame_time * 0.3):(control_states.free_look_speed * engine_frame_time);
        Cam_SetRotation(renderer.cam, cam_angles);
        Cam_MoveAlong(renderer.cam, dist * move_logic[0]);
        Cam_MoveStrafe(renderer.cam, dist * move_logic[1]);
        Cam_MoveVertical(renderer.cam, dist * move_logic[2]);
        renderer.cam->current_room = Room_FindPosCogerrence(renderer.cam->pos, renderer.cam->current_room);

        ent->angles[0] = 180.0 * cam_angles[0] / M_PI;
        pos.m_floats[0] = renderer.cam->pos[0] + renderer.cam->view_dir[0] * control_states.cam_distance;
        pos.m_floats[1] = renderer.cam->pos[1] + renderer.cam->view_dir[1] * control_states.cam_distance;
        pos.m_floats[2] = renderer.cam->pos[2] + renderer.cam->view_dir[2] * control_states.cam_distance - 512.0;
        vec3_copy(ent->transform+12, pos.m_floats);
        Entity_UpdateTransform(ent);
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
            if((Character_GetItemsCount(ent, ITEM_SMALL_MEDIPACK) > 0) &&
               (Character_ChangeParam(ent, PARAM_HEALTH, 250)))
            {
                Character_RemoveItem(ent, ITEM_SMALL_MEDIPACK, 1);
                Audio_Send(TR_AUDIO_SOUND_MEDIPACK);
            }

            control_states.use_small_medi = !control_states.use_small_medi;
        }

        if(control_states.use_big_medi)
        {
            if((Character_GetItemsCount(ent, ITEM_LARGE_MEDIPACK) > 0) &&
               (Character_ChangeParam(ent, PARAM_HEALTH, LARA_PARAM_HEALTH_MAX)))
            {
                Character_RemoveItem(ent, ITEM_LARGE_MEDIPACK, 1);
                Audio_Send(TR_AUDIO_SOUND_MEDIPACK);
            }

            control_states.use_big_medi = !control_states.use_big_medi;
        }

        if((control_mapper.use_joy == 1) && (control_mapper.joy_move_x != 0 ))
        {
            ent->character->cmd.rot[0] = -360.0 / M_PI * engine_frame_time * control_mapper.joy_move_x;
        }
        else
        {
            ent->character->cmd.rot[0] = -360.0 / M_PI * engine_frame_time * (btScalar)move_logic[1];
        }

        if( (control_mapper.use_joy == 1) && (control_mapper.joy_move_y != 0 ) )
        {
            ent->character->cmd.rot[1] = -360.0 / M_PI * engine_frame_time * control_mapper.joy_move_y;
        }
        else
        {
            ent->character->cmd.rot[1] = 360.0 / M_PI * engine_frame_time * (btScalar)move_logic[0];
        }

        vec3_copy(ent->character->cmd.move, move_logic);
    }
}


bool Cam_HasHit(bt_engine_ClosestConvexResultCallback *cb, btTransform &cameraFrom, btTransform &cameraTo)
{
    btSphereShape cameraSphere(16.0);
    cb->m_closestHitFraction = 1.0;
    cb->m_hitCollisionObject = NULL;
    bt_engine_dynamicsWorld->convexSweepTest(&cameraSphere, cameraFrom, cameraTo, *cb);
    return cb->hasHit();
}


void Cam_FollowEntity(struct camera_s *cam, struct entity_s *ent, btScalar dx, btScalar dz)
{
    btTransform cameraFrom, cameraTo;
    btVector3 cam_pos(cam->pos[0], cam->pos[1], cam->pos[2]), cam_pos2;
    bt_engine_ClosestConvexResultCallback *cb;

    //Reset to initial
    cameraFrom.setIdentity();
    cameraTo.setIdentity();

    if(ent->character)
    {
        cb = ent->character->convex_cb;
    }
    else
    {
        cb = new bt_engine_ClosestConvexResultCallback(ent->self);
        cb->m_collisionFilterMask = btBroadphaseProxy::StaticFilter | btBroadphaseProxy::KinematicFilter;
    }

    ///@INFO Basic camera override, completely placeholder until a system classic-like is created
    if(control_states.mouse_look == 0)//If mouse look is off
    {
        float currentAngle = cam_angles[0] * (M_PI / 180.0);  //Current is the current cam angle
        float targetAngle  = ent->angles[0] * (M_PI / 180.0); //Target is the target angle which is the entity's angle itself
        float rotSpeed = 2.0; //Speed of rotation

        ///@FIXME
        //If Lara is in a specific state we want to rotate -75 deg or +75 deg depending on camera collision
        if(ent->bf.animations.last_state == TR_STATE_LARA_REACH)
        {
            if(cam->target_dir == TR_CAM_TARG_BACK)
            {
                cam_pos2 = cam_pos;
                cameraFrom.setOrigin(cam_pos2);
                cam_pos2.m_floats[0] += sinf((ent->angles[0] - 90.0) * (M_PI / 180.0)) * control_states.cam_distance;
                cam_pos2.m_floats[1] -= cosf((ent->angles[0] - 90.0) * (M_PI / 180.0)) * control_states.cam_distance;
                cameraTo.setOrigin(cam_pos2);

                //If collided we want to go right otherwise stay left
                if(Cam_HasHit(cb, cameraFrom, cameraTo))
                {
                    cam_pos2 = cam_pos;
                    cameraFrom.setOrigin(cam_pos2);
                    cam_pos2.m_floats[0] += sinf((ent->angles[0] + 90.0) * (M_PI / 180.0)) * control_states.cam_distance;
                    cam_pos2.m_floats[1] -= cosf((ent->angles[0] + 90.0) * (M_PI / 180.0)) * control_states.cam_distance;
                    cameraTo.setOrigin(cam_pos2);

                    //If collided we want to go to back else right
                    Cam_HasHit(cb, cameraFrom, cameraTo) ? cam->target_dir = cam->target_dir = TR_CAM_TARG_BACK : cam->target_dir = TR_CAM_TARG_RIGHT;
                }
                else
                {
                    cam->target_dir = TR_CAM_TARG_LEFT;
                }
            }
        }
        else if(ent->bf.animations.last_state == TR_STATE_LARA_JUMP_BACK)
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
                targetAngle = (ent->angles[0]) * (M_PI / 180.0);//Same as TR_CAM_TARG_BACK (default pos)
                break;
            }

            float d_angle = cam_angles[0] - targetAngle;
            if(d_angle > M_PI / 2.0)
            {
                d_angle -= M_PI/180.0;
            }
            if(d_angle < -M_PI / 2.0)
            {
                d_angle += M_PI/180.0;
            }
            cam_angles[0] = fmodf(cam_angles[0] + atan2f(sinf(currentAngle - d_angle), cosf(currentAngle + d_angle)) * (engine_frame_time * rotSpeed), M_PI * 2.0); //Update camera's angle
        }
    }

    if((ent->character != NULL) && (ent->character->cam_follow_center > 0))
    {
        vec3_copy(cam_pos.m_floats, ent->obb->centre);
        ent->character->cam_follow_center--;
    }
    else
    {
        Mat4_vec3_mul(cam_pos.m_floats, ent->transform, ent->bf.bone_tags->full_transform+12);
        cam_pos.m_floats[2] += dz;
    }

    //Code to manage screen shaking effects
    if((renderer.cam->shake_time > 0.0) && (renderer.cam->shake_value > 0.0))
    {
        cam_pos.m_floats[0] += ((rand() % abs(renderer.cam->shake_value)) - (renderer.cam->shake_value / 2)) * renderer.cam->shake_time;;
        cam_pos.m_floats[1] += ((rand() % abs(renderer.cam->shake_value)) - (renderer.cam->shake_value / 2)) * renderer.cam->shake_time;;
        cam_pos.m_floats[2] += ((rand() % abs(renderer.cam->shake_value)) - (renderer.cam->shake_value / 2)) * renderer.cam->shake_time;;
        renderer.cam->shake_time  = (renderer.cam->shake_time < 0.0)?(0.0):(renderer.cam->shake_time)-engine_frame_time;
    }

    cameraFrom.setOrigin(cam_pos);
    cam_pos.m_floats[2] += dz;
    cameraTo.setOrigin(cam_pos);
    if(Cam_HasHit(cb, cameraFrom, cameraTo))
    {
        cam_pos.setInterpolate3(cameraFrom.getOrigin(), cameraTo.getOrigin(), cb->m_closestHitFraction);
        cam_pos += cb->m_hitNormalWorld * 2.0;
    }

    if (dx != 0.0)
    {
        cameraFrom.setOrigin(cam_pos);
        cam_pos.m_floats[0] += dx * cam->right_dir[0];
        cam_pos.m_floats[1] += dx * cam->right_dir[1];
        cam_pos.m_floats[2] += dx * cam->right_dir[2];
        cameraTo.setOrigin(cam_pos);
        if(Cam_HasHit(cb, cameraFrom, cameraTo))
        {
            cam_pos.setInterpolate3(cameraFrom.getOrigin(), cameraTo.getOrigin(), cb->m_closestHitFraction);
            cam_pos += cb->m_hitNormalWorld * 2.0;
        }

        cameraFrom.setOrigin(cam_pos);
        cam_pos.m_floats[0] += sinf(cam_angles[0]) * control_states.cam_distance;
        cam_pos.m_floats[1] -= cosf(cam_angles[0]) * control_states.cam_distance;
        cameraTo.setOrigin(cam_pos);
        if(Cam_HasHit(cb, cameraFrom, cameraTo))
        {
            cam_pos.setInterpolate3(cameraFrom.getOrigin(), cameraTo.getOrigin(), cb->m_closestHitFraction);
            cam_pos += cb->m_hitNormalWorld * 2.0;
        }
    }

    //Update cam pos
    vec3_copy(cam->pos, cam_pos.m_floats);

    //Modify cam pos for quicksand rooms
    cam->pos[2] -= 128.0;
    cam->current_room = Room_FindPosCogerrence(cam->pos, cam->current_room);
    cam->pos[2] += 128.0;
    if((cam->current_room != NULL) && (cam->current_room->flags & TR_ROOM_FLAG_QUICKSAND))
    {
        cam->pos[2] = cam->current_room->bb_max[2] + 2.0 * 64.0;
    }

    Cam_SetRotation(cam, cam_angles);
    cam->current_room = Room_FindPosCogerrence(cam->pos, cam->current_room);

    if(!ent->character)
        delete[] cb;
}


void Game_LoopEntities(struct RedBlackNode_s *x)
{
    entity_p entity = (entity_p)x->data;

    if(entity->state_flags & ENTITY_STATE_ENABLED)
    {
        Entity_ProcessSector(entity);
        lua_LoopEntity(engine_lua, entity->id);
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
    entity_p entity = (entity_p)x->data;

    if(entity->type_flags & ENTITY_TYPE_DYNAMIC)
    {
        Entity_UpdateRigidBody(entity, 0);
    }
    else if(Entity_Frame(entity, engine_frame_time))
    {
        Entity_UpdateRigidBody(entity, 0);
    }

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
        Hair_Update(ent);
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
    entity_p ent = engine_world.Character;

    if(ent && ent->character)
    {
        Character_SetParam(ent, PARAM_HEALTH, -1.0f);
        if(ent->character->cmd.action && (ent->type_flags & ENTITY_TYPE_TRIGGER_ACTIVATOR))
        {
            Entity_CheckActivators(ent);
        }
        if(Character_GetParam(ent, PARAM_HEALTH) <= 0.0)
        {
            ent->character->resp.kill = 0;   // Kill, if no HP.
        }
        Hair_Update(ent);
    }

    if(engine_world.entity_tree && engine_world.entity_tree->root)
    {
        Game_UpdateCharactersTree(engine_world.entity_tree->root);
    }
}

__inline btScalar Game_Tick(btScalar *game_logic_time)
{
    int t = *game_logic_time / GAME_LOGIC_REFRESH_INTERVAL;
    btScalar dt = (btScalar)t * GAME_LOGIC_REFRESH_INTERVAL;
    *game_logic_time -= dt;
    return dt;
}


void Game_Frame(btScalar time)
{
    static btScalar game_logic_time  = 0.0;
                    game_logic_time += time;

    bool is_entitytree = ((engine_world.entity_tree != NULL) && (engine_world.entity_tree->root != NULL));
    bool is_character  = (engine_world.Character != NULL);

    // GUI and controls should be updated at all times!

    Controls_PollSDLInput();
    Gui_Update();

    ///@FIXME: I have no idea what's happening here! - Lwmte

    if(!Con_IsShown() && control_states.gui_inventory && main_inventory_manager)
    {
        if((is_character) &&
           (main_inventory_manager->getCurrentState() == gui_InventoryManager::INVENTORY_DISABLED))
        {
            main_inventory_manager->setInventory(&engine_world.Character->character->inventory);
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
        if(game_logic_time >= GAME_LOGIC_REFRESH_INTERVAL)
        {
            Audio_Update();
            Game_Tick(&game_logic_time);
        }
        return;
    }


    // We're going to update main logic with a fixed step.
    // This allows to conserve CPU resources and keep everything in sync!

    if(game_logic_time >= GAME_LOGIC_REFRESH_INTERVAL)
    {
        btScalar dt = Game_Tick(&game_logic_time);
        lua_DoTasks(engine_lua, dt);
        Game_UpdateAI();
        Audio_Update();

        if(is_character)
        {
            Entity_ProcessSector(engine_world.Character);
            Character_UpdateParams(engine_world.Character);
            Entity_CheckCollisionCallbacks(engine_world.Character);   ///@FIXME: Must do it for ALL interactive entities!
        }

        if(is_entitytree) Game_LoopEntities(engine_world.entity_tree->root);
    }


    // This must be called EVERY frame to max out smoothness.
    // Includes animations, camera movement, and so on.

    Game_ApplyControls(engine_world.Character);

    if(is_character)
    {
        if(engine_world.Character->type_flags & ENTITY_TYPE_DYNAMIC)
        {
            Entity_UpdateRigidBody(engine_world.Character, 0);
        }
        if(!control_states.noclip && !control_states.free_look)
        {
            Character_ApplyCommands(engine_world.Character);
            Entity_Frame(engine_world.Character, engine_frame_time);
            Cam_FollowEntity(renderer.cam, engine_world.Character, 16.0, 128.0);
        }
    }

    Game_UpdateCharacters();

    if(is_entitytree) Game_UpdateAllEntities(engine_world.entity_tree->root);

    bt_engine_dynamicsWorld->stepSimulation(time / 2.0, 0);
    bt_engine_dynamicsWorld->stepSimulation(time / 2.0, 0);

    Controls_RefreshStates();
    Render_UpdateAnimTextures();
}


void Game_Prepare()
{
    if(IsCharacter(engine_world.Character))
    {
        // Set character values to default.

        Character_SetParamMaximum(engine_world.Character, PARAM_HEALTH , LARA_PARAM_HEALTH_MAX );
        Character_SetParam       (engine_world.Character, PARAM_HEALTH , LARA_PARAM_HEALTH_MAX );
        Character_SetParamMaximum(engine_world.Character, PARAM_AIR    , LARA_PARAM_AIR_MAX    );
        Character_SetParam       (engine_world.Character, PARAM_AIR    , LARA_PARAM_AIR_MAX    );
        Character_SetParamMaximum(engine_world.Character, PARAM_STAMINA, LARA_PARAM_STAMINA_MAX);
        Character_SetParam       (engine_world.Character, PARAM_STAMINA, LARA_PARAM_STAMINA_MAX);
        Character_SetParamMaximum(engine_world.Character, PARAM_WARMTH,  LARA_PARAM_WARMTH_MAX );
        Character_SetParam       (engine_world.Character, PARAM_WARMTH , LARA_PARAM_WARMTH_MAX );

        // Set character statistics to default.

        engine_world.Character->character->statistics.distance       = 0.0;
        engine_world.Character->character->statistics.ammo_used      = 0;
        engine_world.Character->character->statistics.hits           = 0;
        engine_world.Character->character->statistics.kills          = 0;
        engine_world.Character->character->statistics.medipacks_used = 0;
        engine_world.Character->character->statistics.saves_used     = 0;
        engine_world.Character->character->statistics.secrets_game   = 0;
        engine_world.Character->character->statistics.secrets_level  = 0;
    }
    else if(engine_world.room_count > 0)
    {
        // If there is no character present, move default camera position to
        // the first room (useful for TR1-3 cutscene levels).

        engine_camera.pos[0] = engine_world.rooms[0].bb_max[0];
        engine_camera.pos[1] = engine_world.rooms[0].bb_max[1];
        engine_camera.pos[2] = engine_world.rooms[0].bb_max[2];
    }

    // Set gameflow parameters to default.
    // Reset secret trigger map.

    memset(gameflow_manager.SecretsTriggerMap, 0, sizeof(gameflow_manager.SecretsTriggerMap));
}


void Game_LevelTransition(uint16_t level_index)
{
    char file_path[MAX_ENGINE_PATH];
    lua_GetLoadingScreen(engine_lua, level_index, file_path);
    Gui_FadeAssignPic(FADER_LOADSCREEN, file_path);
    Gui_FadeStart(FADER_LOADSCREEN, GUI_FADER_DIR_OUT);
    Audio_EndStreams();
}
