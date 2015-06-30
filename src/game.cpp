
#include <cstdlib>
#include <cstdio>

#include <bullet/btBulletCollisionCommon.h>
#include <bullet/btBulletDynamicsCommon.h>
#include <lua.hpp>

#include "vmath.h"
#include "polygon.h"
#include "engine.h"
#include "controls.h"
#include "world.h"
#include "game.h"
#include "mesh.h"
#include "entity.h"
#include "camera.h"
#include "render.h"
#include "portal.h"
#include "system.h"
#include "script.h"
#include "console.h"
#include "anim_state_control.h"
#include "obb.h"
#include "character_controller.h"
#include "gameflow.h"
#include "gui.h"
#include "inventory.h"
#include "hair.h"
#include "ragdoll.h"

#include "luahelper.h"

btVector3 cam_angles = {0.0, 0.0, 0.0};

extern btScalar time_scale;
extern lua_State *engine_lua;

void Save_EntityTree(FILE **f, const std::map<uint32_t, std::shared_ptr<Entity> > &map);
void Save_Entity(FILE **f, std::shared_ptr<Entity> ent);

void lua_mlook2(lua::Int8 mlook)
{
    if(!mlook)
    {
        control_states.mouse_look = !control_states.mouse_look;
        ConsoleInfo::instance().printf("mlook = %d", control_states.mouse_look);
        return;
    }

    control_states.mouse_look = *mlook;
    ConsoleInfo::instance().printf("mlook = %d", control_states.mouse_look);
}

void lua_mlook1()
{
    lua_mlook2(lua::None);
}

void lua_freelook2(lua::Int8 free)
{
    if(!free)
    {
        control_states.free_look = !control_states.free_look;
        ConsoleInfo::instance().printf("free_look = %d", control_states.free_look);
        return;
    }

    control_states.free_look = *free;
    ConsoleInfo::instance().printf("free_look = %d", control_states.free_look);
}

void lua_freelook1()
{
    lua_freelook2(lua::None);
}

void lua_cam_distance2(lua::Float distance)
{
    if(!distance)
    {
        ConsoleInfo::instance().printf("cam_distance = %.2f", control_states.cam_distance);
        return;
    }

    control_states.cam_distance = *distance;
    ConsoleInfo::instance().printf("cam_distance = %.2f", control_states.cam_distance);
}

void lua_cam_distance1()
{
    lua_cam_distance2(lua::None);
}

void lua_noclip2(lua::Int8 noclip)
{
    if(!noclip)
    {
        control_states.noclip = !control_states.noclip;
    }
    else
    {
        control_states.noclip = *noclip;
    }

    ConsoleInfo::instance().printf("noclip = %d", control_states.noclip);
}

void lua_noclip1()
{
    lua_noclip2(lua::None);
}

void lua_debuginfo2(lua::Bool show)
{
    if(!show == 0)
    {
        screen_info.show_debuginfo = !screen_info.show_debuginfo;
    }
    else
    {
        screen_info.show_debuginfo = *show;
    }

    ConsoleInfo::instance().printf("debug info = %d", screen_info.show_debuginfo);
}

void lua_debuginfo1()
{
    lua_debuginfo2(lua::None);
}

void lua_timescale2(lua::Float scale)
{
    if(!scale)
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
        time_scale = *scale;
    }

    ConsoleInfo::instance().printf("time_scale = %.3f", time_scale);
}

void lua_timescale1()
{
    lua_timescale2(lua::None);
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
        lua_register(lua, "debuginfo", WRAP_FOR_LUA(lua_debuginfo2,lua_debuginfo1));
        lua_register(lua, "mlook", WRAP_FOR_LUA(lua_mlook2,lua_mlook1));
        lua_register(lua, "freelook", WRAP_FOR_LUA(lua_freelook2,lua_freelook1));
        lua_register(lua, "noclip", WRAP_FOR_LUA(lua_noclip2,lua_noclip1));
        lua_register(lua, "cam_distance", WRAP_FOR_LUA(lua_cam_distance2,lua_cam_distance1));
        lua_register(lua, "timescale", WRAP_FOR_LUA(lua_timescale2,lua_timescale1));
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


void Save_EntityTree(FILE **f, const std::map<uint32_t, std::shared_ptr<Entity> >& map)
{
    for(std::map<uint32_t, std::shared_ptr<Entity> >::const_iterator it = map.begin();
        it != map.end();
        ++it) {
        Save_Entity(f, it->second);
    }
}

/**
 * Entity save function, based on engine lua scripts;
 */
void Save_Entity(FILE **f, std::shared_ptr<Entity> ent)
{
    if(ent == NULL)
    {
        return;
    }

    if(ent->m_typeFlags & ENTITY_TYPE_SPAWNED)
    {
        uint32_t room_id = (ent->m_self->room)?(ent->m_self->room->id):(0xFFFFFFFF);
        fprintf(*f, "\nspawnEntity(%d, 0x%X, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %d);", ent->m_bf.animations.model->id, room_id,
                ent->m_transform.getOrigin()[0], ent->m_transform.getOrigin()[1], ent->m_transform.getOrigin()[2],
                ent->m_angles[0], ent->m_angles[1], ent->m_angles[2], ent->m_id);
    }
    else
    {
        fprintf(*f, "\nsetEntityPos(%d, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f);", ent->m_id,
                ent->m_transform.getOrigin()[0], ent->m_transform.getOrigin()[1], ent->m_transform.getOrigin()[2],
                ent->m_angles[0], ent->m_angles[1], ent->m_angles[2]);
    }

    fprintf(*f, "\nsetEntitySpeed(%d, %.2f, %.2f, %.2f);", ent->m_id, ent->m_speed[0], ent->m_speed[1], ent->m_speed[2]);
    fprintf(*f, "\nsetEntityAnim(%d, %d, %d);", ent->m_id, ent->m_bf.animations.current_animation, ent->m_bf.animations.current_frame);
    fprintf(*f, "\nsetEntityState(%d, %d, %d);", ent->m_id, ent->m_bf.animations.next_state, ent->m_bf.animations.last_state);
    fprintf(*f, "\nsetEntityCollisionFlags(%d, %d, %d);", ent->m_id, ent->m_self->collision_type, ent->m_self->collision_shape);

    if(ent->m_enabled)
    {
        fprintf(*f, "\nenableEntity(%d);", ent->m_id);
    }
    else
    {
        fprintf(*f, "\ndisableEntity(%d);", ent->m_id);
    }

    fprintf(*f, "\nsetEntityFlags(%d, %d, %d, %d, 0x%.4X, 0x%.8X);", ent->m_id, ent->m_active, ent->m_enabled, ent->m_visible, ent->m_typeFlags, ent->m_callbackFlags);

    fprintf(*f, "\nsetEntityTriggerLayout(%d, 0x%.2X);", ent->m_id, ent->m_triggerLayout);
    //setEntityMeshswap()

    if(ent->m_self->room != NULL)
    {
        fprintf(*f, "\nsetEntityRoomMove(%d, %d, %d, %d);", ent->m_id, ent->m_self->room->id, ent->m_moveType, ent->m_dirFlag);
    }
    else
    {
        fprintf(*f, "\nsetEntityRoomMove(%d, nil, %d, %d);", ent->m_id, ent->m_moveType, ent->m_dirFlag);
    }

    if(auto ch = std::dynamic_pointer_cast<Character>(ent))
    {
        fprintf(*f, "\nremoveAllItems(%d);", ent->m_id);
        for(const InventoryNode& i : ch->m_inventory)
        {
            fprintf(*f, "\naddItem(%d, %d, %d);", ent->m_id, i.id, i.count);
        }

        for(int i=0;i<PARAM_SENTINEL;i++)
        {
            fprintf(*f, "\nsetCharacterParam(%d, %d, %.2f, %.2f);", ent->m_id, i, ch->m_parameters.param[i], ch->m_parameters.maximum[i]);
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

    for(int i=0; i < engine_world.flip_data.size(); i++)
    {
        fprintf(f, "setFlipMap(%d, 0x%02X, 0);\n", i, engine_world.flip_data[i].map);
        fprintf(f, "setFlipState(%d, %d);\n", i, engine_world.flip_data[i].state);
    }

    Save_Entity(&f, engine_world.character);    // Save Lara.

    if(!engine_world.entity_tree.empty())
    {
        Save_EntityTree(&f, engine_world.entity_tree);
    }
    fclose(f);

    return 1;
}

void Game_ApplyControls(std::shared_ptr<Entity> ent)
{
    int8_t look_logic[3];

    // Keyboard move logic

    std::array<int8_t,3> move_logic;
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

    if(!renderer.world())
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

        renderer.camera()->setRotation(cam_angles);
        btScalar dist = (control_states.state_walk)?(control_states.free_look_speed * engine_frame_time * 0.3):(control_states.free_look_speed * engine_frame_time);
        renderer.camera()->moveAlong(dist * move_logic[0]);
        renderer.camera()->moveStrafe(dist * move_logic[1]);
        renderer.camera()->moveVertical(dist * move_logic[2]);

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

    if((control_states.free_look != 0) || !std::dynamic_pointer_cast<Character>(ent))
    {
        btScalar dist = (control_states.state_walk)?(control_states.free_look_speed * engine_frame_time * 0.3):(control_states.free_look_speed * engine_frame_time);
        renderer.camera()->setRotation(cam_angles);
        renderer.camera()->moveAlong(dist * move_logic[0]);
        renderer.camera()->moveStrafe(dist * move_logic[1]);
        renderer.camera()->moveVertical(dist * move_logic[2]);
        renderer.camera()->m_currentRoom = Room_FindPosCogerrence(renderer.camera()->m_pos, renderer.camera()->m_currentRoom);
    }
    else if(control_states.noclip != 0)
    {
        btVector3 pos;
        btScalar dist = (control_states.state_walk)?(control_states.free_look_speed * engine_frame_time * 0.3):(control_states.free_look_speed * engine_frame_time);
        renderer.camera()->setRotation(cam_angles);
        renderer.camera()->moveAlong(dist * move_logic[0]);
        renderer.camera()->moveStrafe(dist * move_logic[1]);
        renderer.camera()->moveVertical(dist * move_logic[2]);
        renderer.camera()->m_currentRoom = Room_FindPosCogerrence(renderer.camera()->m_pos, renderer.camera()->m_currentRoom);

        ent->m_angles[0] = 180.0 * cam_angles[0] / M_PI;
        pos[0] = renderer.camera()->m_pos[0] + renderer.camera()->m_viewDir[0] * control_states.cam_distance;
        pos[1] = renderer.camera()->m_pos[1] + renderer.camera()->m_viewDir[1] * control_states.cam_distance;
        pos[2] = renderer.camera()->m_pos[2] + renderer.camera()->m_viewDir[2] * control_states.cam_distance - 512.0;
        ent->m_transform.getOrigin() = pos;
        ent->updateTransform();
    }
    else
    {
        std::shared_ptr<Character> ch = std::dynamic_pointer_cast<Character>(ent);
        // Apply controls to Lara
        ch->m_command.action = control_states.state_action;
        ch->m_command.ready_weapon = control_states.do_draw_weapon;
        ch->m_command.jump = control_states.do_jump;
        ch->m_command.shift = control_states.state_walk;

        ch->m_command.roll = ((control_states.move_forward && control_states.move_backward) || control_states.do_roll);

        // New commands only for TR3 and above
        ch->m_command.sprint = control_states.state_sprint;
        ch->m_command.crouch = control_states.state_crouch;

        if(control_states.use_small_medi)
        {
            if(ch->getItemsCount(ITEM_SMALL_MEDIPACK) > 0 &&
               ch->changeParam(PARAM_HEALTH, 250))
            {
                ch->removeItem(ITEM_SMALL_MEDIPACK, 1);
                Audio_Send(TR_AUDIO_SOUND_MEDIPACK);
            }

            control_states.use_small_medi = !control_states.use_small_medi;
        }

        if(control_states.use_big_medi)
        {
            if(ch->getItemsCount(ITEM_LARGE_MEDIPACK) > 0 &&
               ch->changeParam(PARAM_HEALTH, LARA_PARAM_HEALTH_MAX))
            {
                ch->removeItem(ITEM_LARGE_MEDIPACK, 1);
                Audio_Send(TR_AUDIO_SOUND_MEDIPACK);
            }

            control_states.use_big_medi = !control_states.use_big_medi;
        }

        if((control_mapper.use_joy == 1) && (control_mapper.joy_move_x != 0 ))
        {
            ch->m_command.rot[0] = -360.0 / M_PI * engine_frame_time * control_mapper.joy_move_x;
        }
        else
        {
            ch->m_command.rot[0] = -360.0 / M_PI * engine_frame_time * (btScalar)move_logic[1];
        }

        if( (control_mapper.use_joy == 1) && (control_mapper.joy_move_y != 0 ) )
        {
            ch->m_command.rot[1] = -360.0 / M_PI * engine_frame_time * control_mapper.joy_move_y;
        }
        else
        {
            ch->m_command.rot[1] = 360.0 / M_PI * engine_frame_time * (btScalar)move_logic[0];
        }

        ch->m_command.move = move_logic;
    }
}


bool Cam_HasHit(std::shared_ptr<BtEngineClosestConvexResultCallback> cb, btTransform &cameraFrom, btTransform &cameraTo)
{
    btSphereShape cameraSphere(16.0);
    cb->m_closestHitFraction = 1.0;
    cb->m_hitCollisionObject = NULL;
    bt_engine_dynamicsWorld->convexSweepTest(&cameraSphere, cameraFrom, cameraTo, *cb);
    return cb->hasHit();
}


void Cam_FollowEntity(Camera *cam, std::shared_ptr<Entity> ent, btScalar dx, btScalar dz)
{
    btTransform cameraFrom, cameraTo;

    //Reset to initial
    cameraFrom.setIdentity();
    cameraTo.setIdentity();

    std::shared_ptr<BtEngineClosestConvexResultCallback> cb = ent->callbackForCamera();

    btVector3 cam_pos = cam->m_pos;

    ///@INFO Basic camera override, completely placeholder until a system classic-like is created
    if(control_states.mouse_look == 0)//If mouse look is off
    {
        float currentAngle = cam_angles[0] * (M_PI / 180.0);  //Current is the current cam angle
        float targetAngle  = ent->m_angles[0] * (M_PI / 180.0); //Target is the target angle which is the entity's angle itself
        float rotSpeed = 2.0; //Speed of rotation

        ///@FIXME
        //If Lara is in a specific state we want to rotate -75 deg or +75 deg depending on camera collision
        if(ent->m_bf.animations.last_state == TR_STATE_LARA_REACH)
        {
            if(cam->m_targetDir == TR_CAM_TARG_BACK)
            {
                btVector3 cam_pos2 = cam_pos;
                cameraFrom.setOrigin(cam_pos2);
                cam_pos2[0] += sinf((ent->m_angles[0] - 90.0) * (M_PI / 180.0)) * control_states.cam_distance;
                cam_pos2[1] -= cosf((ent->m_angles[0] - 90.0) * (M_PI / 180.0)) * control_states.cam_distance;
                cameraTo.setOrigin(cam_pos2);

                //If collided we want to go right otherwise stay left
                if(Cam_HasHit(cb, cameraFrom, cameraTo))
                {
                    cam_pos2 = cam_pos;
                    cameraFrom.setOrigin(cam_pos2);
                    cam_pos2[0] += sinf((ent->m_angles[0] + 90.0) * (M_PI / 180.0)) * control_states.cam_distance;
                    cam_pos2[1] -= cosf((ent->m_angles[0] + 90.0) * (M_PI / 180.0)) * control_states.cam_distance;
                    cameraTo.setOrigin(cam_pos2);

                    //If collided we want to go to back else right
                    Cam_HasHit(cb, cameraFrom, cameraTo) ? cam->m_targetDir = cam->m_targetDir = TR_CAM_TARG_BACK : cam->m_targetDir = TR_CAM_TARG_RIGHT;
                }
                else
                {
                    cam->m_targetDir = TR_CAM_TARG_LEFT;
                }
            }
        }
        else if(ent->m_bf.animations.last_state == TR_STATE_LARA_JUMP_BACK)
        {
            cam->m_targetDir = TR_CAM_TARG_FRONT;
        }
        else if(cam->m_targetDir != TR_CAM_TARG_BACK)
        {
            cam->m_targetDir = TR_CAM_TARG_BACK;//Reset to back
        }

        //If target mis-matches current we need to update the camera's angle to reach target!
        if(currentAngle != targetAngle)
        {
            switch(cam->m_targetDir)
            {
            case TR_CAM_TARG_BACK:
                targetAngle = (ent->m_angles[0]) * (M_PI / 180.0);
                break;
            case TR_CAM_TARG_FRONT:
                targetAngle = (ent->m_angles[0] - 180.0) * (M_PI / 180.0);
                break;
            case TR_CAM_TARG_LEFT:
                targetAngle = (ent->m_angles[0] - 75.0) * (M_PI / 180.0);
                break;
            case TR_CAM_TARG_RIGHT:
                targetAngle = (ent->m_angles[0] + 75.0) * (M_PI / 180.0);
                break;
            default:
                targetAngle = (ent->m_angles[0]) * (M_PI / 180.0);//Same as TR_CAM_TARG_BACK (default pos)
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

    cam_pos = ent->camPosForFollowing(dz);

    //Code to manage screen shaking effects
    if((renderer.camera()->m_shakeTime > 0.0) && (renderer.camera()->m_shakeValue > 0.0))
    {
        cam_pos[0] += ((rand() % abs(renderer.camera()->m_shakeValue)) - (renderer.camera()->m_shakeValue / 2)) * renderer.camera()->m_shakeTime;;
        cam_pos[1] += ((rand() % abs(renderer.camera()->m_shakeValue)) - (renderer.camera()->m_shakeValue / 2)) * renderer.camera()->m_shakeTime;;
        cam_pos[2] += ((rand() % abs(renderer.camera()->m_shakeValue)) - (renderer.camera()->m_shakeValue / 2)) * renderer.camera()->m_shakeTime;;
        renderer.camera()->m_shakeTime  = (renderer.camera()->m_shakeTime < 0.0)?(0.0):(renderer.camera()->m_shakeTime)-engine_frame_time;
    }

    cameraFrom.setOrigin(cam_pos);
    cam_pos[2] += dz;
    cameraTo.setOrigin(cam_pos);
    if(Cam_HasHit(cb, cameraFrom, cameraTo))
    {
        cam_pos.setInterpolate3(cameraFrom.getOrigin(), cameraTo.getOrigin(), cb->m_closestHitFraction);
        cam_pos += cb->m_hitNormalWorld * 2.0;
    }

    if (dx != 0.0)
    {
        cameraFrom.setOrigin(cam_pos);
        cam_pos += dx * cam->m_rightDir;
        cameraTo.setOrigin(cam_pos);
        if(Cam_HasHit(cb, cameraFrom, cameraTo))
        {
            cam_pos.setInterpolate3(cameraFrom.getOrigin(), cameraTo.getOrigin(), cb->m_closestHitFraction);
            cam_pos += cb->m_hitNormalWorld * 2.0;
        }

        cameraFrom.setOrigin(cam_pos);
        cam_pos[0] += sinf(cam_angles[0]) * control_states.cam_distance;
        cam_pos[1] -= cosf(cam_angles[0]) * control_states.cam_distance;
        cameraTo.setOrigin(cam_pos);
        if(Cam_HasHit(cb, cameraFrom, cameraTo))
        {
            cam_pos.setInterpolate3(cameraFrom.getOrigin(), cameraTo.getOrigin(), cb->m_closestHitFraction);
            cam_pos += cb->m_hitNormalWorld * 2.0;
        }
    }

    //Update cam pos
    cam->m_pos = cam_pos;

    //Modify cam pos for quicksand rooms
    cam->m_pos[2] -= 128.0;
    cam->m_currentRoom = Room_FindPosCogerrence(cam->m_pos, cam->m_currentRoom);
    cam->m_pos[2] += 128.0;
    if((cam->m_currentRoom != NULL) && (cam->m_currentRoom->flags & TR_ROOM_FLAG_QUICKSAND))
    {
        cam->m_pos[2] = cam->m_currentRoom->bb_max[2] + 2.0 * 64.0;
    }

    cam->setRotation(cam_angles);
    cam->m_currentRoom = Room_FindPosCogerrence(cam->m_pos, cam->m_currentRoom);
}


void Game_LoopEntities(std::map<uint32_t, std::shared_ptr<Entity> > &entities)
{
    for(auto entityPair : entities) {
        std::shared_ptr<Entity> entity = entityPair.second;
        if(entity->m_enabled)
        {
            entity->processSector();
            lua_LoopEntity(engine_lua, entity->m_id);
        }
    }
}


void Game_UpdateAllEntities(std::map<uint32_t, std::shared_ptr<Entity> > &entities)
{
    for(auto entityPair : entities) {
        std::shared_ptr<Entity> entity = entityPair.second;
        if(entity->m_typeFlags & ENTITY_TYPE_DYNAMIC)
        {
            entity->updateRigidBody(false);
        }
        else if(entity->frame(engine_frame_time))
        {
            entity->updateRigidBody(false);
        }
    }
}


void Game_UpdateAI()
{
    //for(ALL CHARACTERS, EXCEPT PLAYER)
    {
            // UPDATE AI commands
    }
}


void Game_UpdateCharactersTree(std::map<uint32_t, std::shared_ptr<Entity> >& entities)
{
    for(const auto& entPair : entities) {
        std::shared_ptr<Character> ent = std::dynamic_pointer_cast<Character>(entPair.second);
        if(!ent)
            continue;

        if(ent->m_command.action && (ent->m_typeFlags & ENTITY_TYPE_TRIGGER_ACTIVATOR))
        {
            ent->checkActivators();
        }
        if(ent->getParam(PARAM_HEALTH) <= 0.0)
        {
            ent->m_response.kill = 1;                                      // Kill, if no HP.
        }
        ent->applyCommands();
        ent->updateHair();
    }
}


void Game_UpdateCharacters()
{
    std::shared_ptr<Character> ent = engine_world.character;

    if(ent)
    {
        if(ent->m_command.action && (ent->m_typeFlags & ENTITY_TYPE_TRIGGER_ACTIVATOR))
        {
            ent->checkActivators();
        }
        if(ent->getParam(PARAM_HEALTH) <= 0.0)
        {
            ent->m_response.kill = 1;   // Kill, if no HP.
        }
        ent->updateHair();
    }

    if(!engine_world.entity_tree.empty())
    {
        Game_UpdateCharactersTree(engine_world.entity_tree);
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

    const bool is_entitytree = !engine_world.entity_tree.empty();
    const bool is_character  = (engine_world.character != NULL);

    // GUI and controls should be updated at all times!

    Controls_PollSDLInput();
    Gui_Update();

    ///@FIXME: I have no idea what's happening here! - Lwmte

    if(!ConsoleInfo::instance().isVisible() && control_states.gui_inventory && main_inventory_manager)
    {
        if((is_character) &&
           (main_inventory_manager->getCurrentState() == gui_InventoryManager::INVENTORY_DISABLED))
        {
            main_inventory_manager->setInventory(&engine_world.character->m_inventory);
            main_inventory_manager->send(gui_InventoryManager::INVENTORY_OPEN);
        }
        if(main_inventory_manager->getCurrentState() == gui_InventoryManager::INVENTORY_IDLE)
        {
            main_inventory_manager->send(gui_InventoryManager::INVENTORY_CLOSE);
        }
    }

    // If console or inventory is active, only thing to update is audio.
    if(ConsoleInfo::instance().isVisible() || main_inventory_manager->getCurrentState() != gui_InventoryManager::INVENTORY_DISABLED)
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
            engine_world.character->processSector();
            engine_world.character->updateParams();
            engine_world.character->checkCollisionCallbacks();   ///@FIXME: Must do it for ALL interactive entities!
        }

        if(is_entitytree)
            Game_LoopEntities(engine_world.entity_tree);
    }


    // This must be called EVERY frame to max out smoothness.
    // Includes animations, camera movement, and so on.

    Game_ApplyControls(engine_world.character);

    if(is_character)
    {
        if(engine_world.character->m_typeFlags & ENTITY_TYPE_DYNAMIC)
        {
            engine_world.character->updateRigidBody(false);
        }
        if(!control_states.noclip && !control_states.free_look)
        {
            engine_world.character->applyCommands();
            engine_world.character->frame(engine_frame_time);
            Cam_FollowEntity(renderer.camera(), engine_world.character, 16.0, 128.0);
        }
    }

    Game_UpdateCharacters();

    if(is_entitytree)
        Game_UpdateAllEntities(engine_world.entity_tree);

    bt_engine_dynamicsWorld->stepSimulation(time / 2.0, 0);
    bt_engine_dynamicsWorld->stepSimulation(time / 2.0, 0);

    Controls_RefreshStates();
    engine_world.updateAnimTextures();
}


void Game_Prepare()
{
    if(engine_world.character)
    {
        // Set character values to default.

        engine_world.character->setParamMaximum(PARAM_HEALTH , LARA_PARAM_HEALTH_MAX );
        engine_world.character->setParamMaximum(PARAM_HEALTH , LARA_PARAM_HEALTH_MAX );
        engine_world.character->setParamMaximum(PARAM_AIR    , LARA_PARAM_AIR_MAX    );
        engine_world.character->setParamMaximum(PARAM_AIR    , LARA_PARAM_AIR_MAX    );
        engine_world.character->setParamMaximum(PARAM_STAMINA, LARA_PARAM_STAMINA_MAX);
        engine_world.character->setParamMaximum(PARAM_STAMINA, LARA_PARAM_STAMINA_MAX);
        engine_world.character->setParamMaximum(PARAM_WARMTH,  LARA_PARAM_WARMTH_MAX );
        engine_world.character->setParamMaximum(PARAM_WARMTH , LARA_PARAM_WARMTH_MAX );

        // Set character statistics to default.

        engine_world.character->m_statistics.distance       = 0.0;
        engine_world.character->m_statistics.ammo_used      = 0;
        engine_world.character->m_statistics.hits           = 0;
        engine_world.character->m_statistics.kills          = 0;
        engine_world.character->m_statistics.medipacks_used = 0;
        engine_world.character->m_statistics.saves_used     = 0;
        engine_world.character->m_statistics.secrets_game   = 0;
        engine_world.character->m_statistics.secrets_level  = 0;
    }
    else if(!engine_world.rooms.empty())
    {
        // If there is no character present, move default camera position to
        // the first room (useful for TR1-3 cutscene levels).

        engine_camera.m_pos[0] = engine_world.rooms[0]->bb_max[0];
        engine_camera.m_pos[1] = engine_world.rooms[0]->bb_max[1];
        engine_camera.m_pos[2] = engine_world.rooms[0]->bb_max[2];
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
