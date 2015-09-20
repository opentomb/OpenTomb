#include "game.h"

#include <cstdio>
#include <cstdlib>

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

#include "LuaState.h"

#include "audio/audio.h"
#include "character_controller.h"
#include "controls.h"
#include "engine.h"
#include "gameflow.h"
#include "gui/console.h"
#include "gui/fader.h"
#include "gui/gui.h"
#include "inventory.h"
#include "render/render.h"
#include "script/script.h"
#include "system.h"
#include "util/vmath.h"
#include "world/camera.h"
#include "world/character.h"
#include "world/entity.h"
#include "world/room.h"
#include "world/world.h"

extern btScalar time_scale;
extern script::MainEngine engine_lua;

namespace engine
{

btVector3 cam_angles = { 0.0, 0.0, 0.0 };

void Save_EntityTree(FILE **f, const std::map<uint32_t, std::shared_ptr<world::Entity> > &map);
void Save_Entity(FILE **f, std::shared_ptr<world::Entity> ent);

using gui::Console;

void lua_mlook(lua::Value mlook)
{
    if(!mlook.is<lua::Boolean>())
    {
        control_states.mouse_look = !control_states.mouse_look;
        Console::instance().printf("mlook = %d", control_states.mouse_look);
        return;
    }

    control_states.mouse_look = mlook;
    Console::instance().printf("mlook = %d", control_states.mouse_look);
}

void lua_freelook(lua::Value free)
{
    if(!free.is<lua::Boolean>())
    {
        control_states.free_look = !control_states.free_look;
        Console::instance().printf("free_look = %d", control_states.free_look);
        return;
    }

    control_states.free_look = free;
    Console::instance().printf("free_look = %d", control_states.free_look);
}

void lua_cam_distance(lua::Value distance)
{
    if(!distance.is<lua::Number>())
    {
        Console::instance().printf("cam_distance = %.2f", control_states.cam_distance);
        return;
    }

    control_states.cam_distance = distance;
    Console::instance().printf("cam_distance = %.2f", control_states.cam_distance);
}

void lua_noclip(lua::Value noclip)
{
    if(!noclip.is<lua::Boolean>())
    {
        control_states.noclip = !control_states.noclip;
    }
    else
    {
        control_states.noclip = noclip;
    }

    Console::instance().printf("noclip = %d", control_states.noclip);
}

void lua_debuginfo(lua::Value show)
{
    if(!show.is<lua::Boolean>())
    {
        screen_info.show_debuginfo = !screen_info.show_debuginfo;
    }
    else
    {
        screen_info.show_debuginfo = show;
    }

    Console::instance().printf("debug info = %d", screen_info.show_debuginfo);
}

void lua_timescale(lua::Value scale)
{
    if(!scale.is<lua::Number>())
    {
        if(time_scale == 1.0)
        {
            time_scale = 0.033f;
        }
        else
        {
            time_scale = 1.0;
        }
    }
    else
    {
        time_scale = scale;
    }

    Console::instance().printf("time_scale = %.3f", time_scale);
}

void Game_InitGlobals()
{
    control_states.free_look_speed = 3000.0;
    control_states.mouse_look = true;
    control_states.free_look = false;
    control_states.noclip = false;
    control_states.cam_distance = 800.0;
}

void Game_RegisterLuaFunctions(script::ScriptEngine& state)
{
    state.set("debuginfo", lua_debuginfo);
    state.set("mlook", lua_mlook);
    state.set("freelook", lua_freelook);
    state.set("noclip", lua_noclip);
    state.set("cam_distance", lua_cam_distance);
    state.set("timescale", lua_timescale);
}

/**
 * Load game state
 */
int Game_Load(const char* name)
{
    FILE *f;
    char *ch, local;

    local = 1;
    for(ch = const_cast<char*>(name); *ch; ch++)
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
        if(f == nullptr)
        {
            SYS_EXT_WARN("Can not read file \"%s\"", token);
            return 0;
        }
        fclose(f);
        engine_lua.clearTasks();
        try
        {
            engine_lua.doFile(token);
        }
        catch(lua::RuntimeError& error)
        {
            Sys_DebugLog(LUA_LOG_FILENAME, "%s", error.what());
        }
        catch(lua::LoadError& error)
        {
            Sys_DebugLog(LUA_LOG_FILENAME, "%s", error.what());
        }
    }
    else
    {
        f = fopen(name, "rb");
        if(f == nullptr)
        {
            SYS_EXT_WARN("Can not read file \"%s\"", name);
            return 0;
        }
        fclose(f);
        engine_lua.clearTasks();
        try
        {
            engine_lua.doFile(name);
        }
        catch(lua::RuntimeError& error)
        {
            Sys_DebugLog(LUA_LOG_FILENAME, "%s", error.what());
        }
        catch(lua::LoadError& error)
        {
            Sys_DebugLog(LUA_LOG_FILENAME, "%s", error.what());
        }
    }

    return 1;
}

void Save_EntityTree(FILE **f, const std::map<uint32_t, std::shared_ptr<world::Entity> >& map)
{
    for(std::map<uint32_t, std::shared_ptr<world::Entity> >::const_iterator it = map.begin();
    it != map.end();
        ++it)
    {
        Save_Entity(f, it->second);
    }
}

/**
 * Entity save function, based on engine lua scripts;
 */
void Save_Entity(FILE **f, std::shared_ptr<world::Entity> ent)
{
    if(ent == nullptr)
    {
        return;
    }

    if(ent->m_typeFlags & ENTITY_TYPE_SPAWNED)
    {
        uint32_t room_id = (ent->m_self->room) ? (ent->m_self->room->id) : (0xFFFFFFFF);
        fprintf(*f, "\nspawnEntity(%d, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %d, %d);", ent->m_bf.animations.model->id,
                ent->m_transform.getOrigin()[0], ent->m_transform.getOrigin()[1], ent->m_transform.getOrigin()[2],
                ent->m_angles[0], ent->m_angles[1], ent->m_angles[2], room_id, ent->id());
    }
    else
    {
        fprintf(*f, "\nsetEntityPos(%d, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f);", ent->id(),
                ent->m_transform.getOrigin()[0], ent->m_transform.getOrigin()[1], ent->m_transform.getOrigin()[2],
                ent->m_angles[0], ent->m_angles[1], ent->m_angles[2]);
    }

    fprintf(*f, "\nsetEntitySpeed(%d, %.2f, %.2f, %.2f);", ent->id(), ent->m_speed[0], ent->m_speed[1], ent->m_speed[2]);
    fprintf(*f, "\nsetEntityAnim(%d, %d, %d);", ent->id(), ent->m_bf.animations.current_animation, ent->m_bf.animations.current_frame);
    fprintf(*f, "\nsetEntityState(%d, %d, %d);", ent->id(), ent->m_bf.animations.next_state, ent->m_bf.animations.last_state);
    fprintf(*f, "\nsetEntityCollisionFlags(%d, %ld, %ld);", ent->id(), static_cast<long>(ent->m_self->collision_type), static_cast<long>(ent->m_self->collision_shape));

    if(ent->m_enabled)
    {
        fprintf(*f, "\nenableEntity(%d);", ent->id());
    }
    else
    {
        fprintf(*f, "\ndisableEntity(%d);", ent->id());
    }

    fprintf(*f, "\nsetEntityFlags(%d, %d, %d, %d, 0x%.4X, 0x%.8X);", ent->id(), ent->m_active, ent->m_enabled, ent->m_visible, ent->m_typeFlags, ent->m_callbackFlags);

    fprintf(*f, "\nsetEntityTriggerLayout(%d, 0x%.2X);", ent->id(), ent->m_triggerLayout);
    //setEntityMeshswap()

    if(ent->m_self->room != nullptr)
    {
        fprintf(*f, "\nsetEntityRoomMove(%d, %d, %d, %d);", ent->id(), ent->m_self->room->id, ent->m_moveType, static_cast<int>(ent->m_moveDir));
    }
    else
    {
        fprintf(*f, "\nsetEntityRoomMove(%d, nil, %d, %d);", ent->id(), ent->m_moveType, static_cast<int>(ent->m_moveDir));
    }

    if(auto ch = std::dynamic_pointer_cast<world::Character>(ent))
    {
        fprintf(*f, "\nremoveAllItems(%d);", ent->id());
        for(const InventoryNode& i : ch->m_inventory)
        {
            fprintf(*f, "\naddItem(%d, %d, %d);", ent->id(), i.id, i.count);
        }

        for(int i = 0; i < world::PARAM_SENTINEL; i++)
        {
            fprintf(*f, "\nsetCharacterParam(%d, %d, %.2f, %.2f);", ent->id(), i, ch->m_parameters.param[i], ch->m_parameters.maximum[i]);
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
    for(ch = const_cast<char*>(name); *ch; ch++)
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
        SYS_EXT_WARN("Can not create file \"%s\"", name);
        return 0;
    }

    fprintf(f, "loadMap(\"%s\", %d, %d);\n", Gameflow_Manager.getLevelPath().c_str(), Gameflow_Manager.getGameID(), Gameflow_Manager.getLevelID());

    // Save flipmap and flipped room states.

    for(uint32_t i = 0; i < engine_world.flip_data.size(); i++)
    {
        fprintf(f, "setFlipMap(%d, 0x%02X, 0);\n", i, engine_world.flip_data[i].map);
        fprintf(f, "setFlipState(%d, %s);\n", i, engine_world.flip_data[i].state ? "true" : "false");
    }

    Save_Entity(&f, engine_world.character);    // Save Lara.

    Save_EntityTree(&f, engine_world.entity_tree);

    fclose(f);

    return 1;
}

void Game_ApplyControls(std::shared_ptr<world::Entity> ent)
{
    int8_t look_logic[3];

    // Keyboard move logic

    std::array<int8_t, 3> move_logic;
    move_logic[0] = control_states.move_forward - control_states.move_backward;
    move_logic[1] = control_states.move_right - control_states.move_left;
    move_logic[2] = control_states.move_up - control_states.move_down;

    // Keyboard look logic

    look_logic[0] = control_states.look_left - control_states.look_right;
    look_logic[1] = control_states.look_down - control_states.look_up;
    look_logic[2] = control_states.look_roll_right - control_states.look_roll_left;

    // APPLY CONTROLS

    cam_angles[0] += 2.2f * engine_frame_time * look_logic[0];
    cam_angles[1] += 2.2f * engine_frame_time * look_logic[1];
    cam_angles[2] += 2.2f * engine_frame_time * look_logic[2];

    // FIXME: Duplicate code - do we need cam control with no world??
    if(!render::renderer.world())
    {
        if(control_mapper.use_joy)
        {
            if(control_mapper.joy_look_x != 0)
            {
                cam_angles[0] -= 0.015f * engine_frame_time * control_mapper.joy_look_x;
            }
            if(control_mapper.joy_look_y != 0)
            {
                cam_angles[1] -= 0.015f * engine_frame_time * control_mapper.joy_look_y;
            }
        }

        if(control_states.mouse_look)
        {
            cam_angles[0] -= 0.015f * control_states.look_axis_x;
            cam_angles[1] -= 0.015f * control_states.look_axis_y;
            control_states.look_axis_x = 0.0;
            control_states.look_axis_y = 0.0;
        }

        render::renderer.camera()->setRotation(cam_angles);
        btScalar dist = (control_states.state_walk) ? (control_states.free_look_speed * engine_frame_time * 0.3f) : (control_states.free_look_speed * engine_frame_time);
        render::renderer.camera()->moveAlong(dist * move_logic[0]);
        render::renderer.camera()->moveStrafe(dist * move_logic[1]);
        render::renderer.camera()->moveVertical(dist * move_logic[2]);

        return;
    }

    if(control_mapper.use_joy)
    {
        if(control_mapper.joy_look_x != 0)
        {
            cam_angles[0] -= engine_frame_time * control_mapper.joy_look_x;
        }
        if(control_mapper.joy_look_y != 0)
        {
            cam_angles[1] -= engine_frame_time * control_mapper.joy_look_y;
        }
    }

    if(control_states.mouse_look)
    {
        cam_angles[0] -= 0.015f * control_states.look_axis_x;
        cam_angles[1] -= 0.015f * control_states.look_axis_y;
        control_states.look_axis_x = 0.0;
        control_states.look_axis_y = 0.0;
    }

    if(control_states.free_look || !std::dynamic_pointer_cast<world::Character>(ent))
    {
        btScalar dist = (control_states.state_walk) ? (control_states.free_look_speed * engine_frame_time * 0.3f) : (control_states.free_look_speed * engine_frame_time);
        render::renderer.camera()->setRotation(cam_angles);
        render::renderer.camera()->moveAlong(dist * move_logic[0]);
        render::renderer.camera()->moveStrafe(dist * move_logic[1]);
        render::renderer.camera()->moveVertical(dist * move_logic[2]);
        render::renderer.camera()->m_currentRoom = Room_FindPosCogerrence(render::renderer.camera()->getPosition(), render::renderer.camera()->m_currentRoom);
    }
    else if(control_states.noclip)
    {
        btVector3 position;
        btScalar dist = (control_states.state_walk) ? (control_states.free_look_speed * engine_frame_time * 0.3f) : (control_states.free_look_speed * engine_frame_time);
        render::renderer.camera()->setRotation(cam_angles);
        render::renderer.camera()->moveAlong(dist * move_logic[0]);
        render::renderer.camera()->moveStrafe(dist * move_logic[1]);
        render::renderer.camera()->moveVertical(dist * move_logic[2]);
        render::renderer.camera()->m_currentRoom = Room_FindPosCogerrence(render::renderer.camera()->getPosition(), render::renderer.camera()->m_currentRoom);

        ent->m_angles[0] = cam_angles[0] * util::DegPerRad;
        position = render::renderer.camera()->getPosition() + render::renderer.camera()->getViewDir() * control_states.cam_distance;
        position[2] -= 512.0;
        ent->m_transform.getOrigin() = position;
        ent->updateTransform();
        ent->m_lerp_curr_transform = ent->m_transform;
    }
    else
    {
        std::shared_ptr<world::Character> ch = std::dynamic_pointer_cast<world::Character>(ent);
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
            if(ch->getItemsCount(ITEM_SMALL_MEDIPACK) > 0 && ch->changeParam(world::PARAM_HEALTH, 250))
            {
                ch->setParam(world::PARAM_POISON, 0);
                ch->removeItem(ITEM_SMALL_MEDIPACK, 1);
                engine::engine_world.audioEngine.send(TR_AUDIO_SOUND_MEDIPACK);
            }

            control_states.use_small_medi = !control_states.use_small_medi;
        }

        if(control_states.use_big_medi)
        {
            if(ch->getItemsCount(ITEM_LARGE_MEDIPACK) > 0 &&
               ch->changeParam(world::PARAM_HEALTH, LARA_PARAM_HEALTH_MAX))
            {
                ch->setParam(world::PARAM_POISON, 0);
                ch->removeItem(ITEM_LARGE_MEDIPACK, 1);
                engine::engine_world.audioEngine.send(TR_AUDIO_SOUND_MEDIPACK);
            }

            control_states.use_big_medi = !control_states.use_big_medi;
        }

        if(control_mapper.use_joy && (control_mapper.joy_move_x != 0))
        {
            ch->m_command.rot[0] += -2 * util::DegPerRad * engine_frame_time * control_mapper.joy_move_x;
        }
        else
        {
            ch->m_command.rot[0] += -2 * util::DegPerRad * engine_frame_time * static_cast<btScalar>(move_logic[1]);
        }

        if(control_mapper.use_joy && (control_mapper.joy_move_y != 0))
        {
            ch->m_command.rot[1] += -2 * util::DegPerRad * engine_frame_time * control_mapper.joy_move_y;
        }
        else
        {
            ch->m_command.rot[1] += 2 * util::DegPerRad * engine_frame_time * static_cast<btScalar>(move_logic[0]);
        }

        ch->m_command.move = move_logic;
    }
}

bool Cam_HasHit(std::shared_ptr<BtEngineClosestConvexResultCallback> cb, btTransform &cameraFrom, btTransform &cameraTo)
{
    btSphereShape cameraSphere(COLLISION_CAMERA_SPHERE_RADIUS);
    cameraSphere.setMargin(COLLISION_MARGIN_DEFAULT);
    cb->m_closestHitFraction = 1.0;
    cb->m_hitCollisionObject = nullptr;
    bt_engine_dynamicsWorld->convexSweepTest(&cameraSphere, cameraFrom, cameraTo, *cb);
    return cb->hasHit();
}

void Cam_FollowEntity(world::Camera *cam, std::shared_ptr<world::Entity> ent, btScalar dx, btScalar dz)
{
    btTransform cameraFrom, cameraTo;

    //Reset to initial
    cameraFrom.setIdentity();
    cameraTo.setIdentity();

    std::shared_ptr<BtEngineClosestConvexResultCallback> cb = ent->callbackForCamera();

    btVector3 cam_pos = cam->getPosition();

    ///@INFO Basic camera override, completely placeholder until a system classic-like is created
    if(!control_states.mouse_look)//If mouse look is off
    {
        float currentAngle = cam_angles[0] * util::RadPerDeg;  //Current is the current cam angle
        float targetAngle = ent->m_angles[0] * util::RadPerDeg; //Target is the target angle which is the entity's angle itself
        float rotSpeed = 2.0; //Speed of rotation

        ///@FIXME
        //If Lara is in a specific state we want to rotate -75 deg or +75 deg depending on camera collision
        if(ent->m_bf.animations.last_state == TR_STATE_LARA_REACH)
        {
            if(cam->m_targetDir == world::CameraTarget::Back)
            {
                btVector3 cam_pos2 = cam_pos;
                cameraFrom.setOrigin(cam_pos2);
                cam_pos2[0] += std::sin((ent->m_angles[0] - 90.0f) * util::RadPerDeg) * control_states.cam_distance;
                cam_pos2[1] -= std::cos((ent->m_angles[0] - 90.0f) * util::RadPerDeg) * control_states.cam_distance;
                cameraTo.setOrigin(cam_pos2);

                //If collided we want to go right otherwise stay left
                if(Cam_HasHit(cb, cameraFrom, cameraTo))
                {
                    cam_pos2 = cam_pos;
                    cameraFrom.setOrigin(cam_pos2);
                    cam_pos2[0] += std::sin((ent->m_angles[0] + 90.0f) * util::RadPerDeg) * control_states.cam_distance;
                    cam_pos2[1] -= std::cos((ent->m_angles[0] + 90.0f) * util::RadPerDeg) * control_states.cam_distance;
                    cameraTo.setOrigin(cam_pos2);

                    //If collided we want to go to back else right
                    if(Cam_HasHit(cb, cameraFrom, cameraTo))
                        cam->m_targetDir = world::CameraTarget::Back;
                    else
                        cam->m_targetDir = world::CameraTarget::Right;
                }
                else
                {
                    cam->m_targetDir = world::CameraTarget::Left;
                }
            }
        }
        else if(ent->m_bf.animations.last_state == TR_STATE_LARA_JUMP_BACK)
        {
            cam->m_targetDir = world::CameraTarget::Front;
        }
        else if(cam->m_targetDir != world::CameraTarget::Back)
        {
            cam->m_targetDir = world::CameraTarget::Back;
        }

        //If target mis-matches current we need to update the camera's angle to reach target!
        if(currentAngle != targetAngle)
        {
            switch(cam->m_targetDir)
            {
                case world::CameraTarget::Back:
                    targetAngle = (ent->m_angles[0]) * util::RadPerDeg;
                    break;
                case world::CameraTarget::Front:
                    targetAngle = (ent->m_angles[0] - 180.0f) * util::RadPerDeg;
                    break;
                case world::CameraTarget::Left:
                    targetAngle = (ent->m_angles[0] - 75.0f) * util::RadPerDeg;
                    break;
                case world::CameraTarget::Right:
                    targetAngle = (ent->m_angles[0] + 75.0f) * util::RadPerDeg;
                    break;
                default:
                    targetAngle = (ent->m_angles[0]) * util::RadPerDeg;//Same as TR_CAM_TARG_BACK (default pos)
                    break;
            }

            float d_angle = cam_angles[0] - targetAngle;
            if(d_angle > util::Rad90)
            {
                d_angle -= 1 * util::RadPerDeg;
            }
            if(d_angle < -util::Rad90)
            {
                d_angle += 1 * util::RadPerDeg;
            }
            cam_angles[0] = std::fmod(cam_angles[0] + std::atan2(std::sin(currentAngle - d_angle), std::cos(currentAngle + d_angle)) * (engine_frame_time * rotSpeed), util::Rad360); //Update camera's angle
        }
    }

    cam_pos = ent->camPosForFollowing(dz);

    //Code to manage screen shaking effects
    if((render::renderer.camera()->m_shakeTime > 0.0) && (render::renderer.camera()->m_shakeValue > 0.0))
    {
        cam_pos[0] += (std::fmod(rand(), std::abs(render::renderer.camera()->m_shakeValue)) - (render::renderer.camera()->m_shakeValue / 2.0f)) * render::renderer.camera()->m_shakeTime;
        cam_pos[1] += (std::fmod(rand(), std::abs(render::renderer.camera()->m_shakeValue)) - (render::renderer.camera()->m_shakeValue / 2.0f)) * render::renderer.camera()->m_shakeTime;
        cam_pos[2] += (std::fmod(rand(), std::abs(render::renderer.camera()->m_shakeValue)) - (render::renderer.camera()->m_shakeValue / 2.0f)) * render::renderer.camera()->m_shakeTime;
        render::renderer.camera()->m_shakeTime  = (render::renderer.camera()->m_shakeTime < 0.0)?(0.0f):(render::renderer.camera()->m_shakeTime)-engine_frame_time;
    }

    cameraFrom.setOrigin(cam_pos);
    cam_pos[2] += dz;
    cameraTo.setOrigin(cam_pos);
    if(Cam_HasHit(cb, cameraFrom, cameraTo))
    {
        cam_pos.setInterpolate3(cameraFrom.getOrigin(), cameraTo.getOrigin(), cb->m_closestHitFraction);
        cam_pos += cb->m_hitNormalWorld * 2.0;
    }

    if(dx != 0.0)
    {
        cameraFrom.setOrigin(cam_pos);
        cam_pos += dx * cam->getRightDir();
        cameraTo.setOrigin(cam_pos);
        if(Cam_HasHit(cb, cameraFrom, cameraTo))
        {
            cam_pos.setInterpolate3(cameraFrom.getOrigin(), cameraTo.getOrigin(), cb->m_closestHitFraction);
            cam_pos += cb->m_hitNormalWorld * 2.0;
        }

        cameraFrom.setOrigin(cam_pos);

        {
            float cos_ay =  cos(cam_angles[1]);
            float cam_dx =  sin(cam_angles[0]) * cos_ay;
            float cam_dy = -cos(cam_angles[0]) * cos_ay;
            float cam_dz = -sin(cam_angles[1]);
            cam_pos.m_floats[0] += cam_dx * control_states.cam_distance;
            cam_pos.m_floats[1] += cam_dy * control_states.cam_distance;
            cam_pos.m_floats[2] += cam_dz * control_states.cam_distance;
        }

        cameraTo.setOrigin(cam_pos);
        if(Cam_HasHit(cb, cameraFrom, cameraTo))
        {
            cam_pos.setInterpolate3(cameraFrom.getOrigin(), cameraTo.getOrigin(), cb->m_closestHitFraction);
            cam_pos += cb->m_hitNormalWorld * 2.0;
        }
    }

    //Update cam pos
    cam->setPosition( cam_pos );

    //Modify cam pos for quicksand rooms
    cam->m_currentRoom = Room_FindPosCogerrence(cam->getPosition() - btVector3(0, 0, 128), cam->m_currentRoom);
    if((cam->m_currentRoom != nullptr) && (cam->m_currentRoom->flags & TR_ROOM_FLAG_QUICKSAND))
    {
        btVector3 position = cam->getPosition();
        position[2] = cam->m_currentRoom->boundingBox.max[2] + 2.0f * 64.0f;
        cam->setPosition(position);
    }

    cam->setRotation(cam_angles);
    cam->m_currentRoom = Room_FindPosCogerrence(cam->getPosition(), cam->m_currentRoom);
}

void Game_UpdateAI()
{
    //for(ALL CHARACTERS, EXCEPT PLAYER)
    {
        // UPDATE AI commands
    }
}

inline btScalar Game_Tick(btScalar *game_logic_time)
{
    int t = static_cast<int>(*game_logic_time * world::animation::GameLogicFrameRate);
    btScalar dt = static_cast<btScalar>(t) / world::animation::GameLogicFrameRate;
    *game_logic_time -= dt;
    return dt;
}


void Game_Frame(btScalar time)
{
    static btScalar game_logic_time = 0.0;

    // clamp frameskip at max substeps - if more frames are dropped, slow everything down:
    if(time > btScalar(MAX_SIM_SUBSTEPS) / world::animation::GameLogicFrameRate)
    {
        time = btScalar(MAX_SIM_SUBSTEPS) / world::animation::GameLogicFrameRate;
        engine_frame_time = time;   // FIXME
    }
    game_logic_time += time;


    // GUI and controls should be updated at all times!
    Controls_PollSDLInput();

    // TODO: implement pause mechanism
    if(gui::update())
    {
        if(game_logic_time >= 1/world::animation::GameLogicFrameRate)
        {
            engine::engine_world.audioEngine.updateAudio();
            Game_Tick(&game_logic_time);
        }
        return;
    }

    // Translate input to character commands, move camera:
    // TODO: decouple cam movement
    Game_ApplyControls(engine_world.character);

    bt_engine_dynamicsWorld->stepSimulation(time, MAX_SIM_SUBSTEPS, 1/world::animation::GameLogicFrameRate);

    if(engine_world.character) {
        engine_world.character->updateInterpolation(time);

        if(!control_states.noclip && !control_states.free_look)
            Cam_FollowEntity(render::renderer.camera(), engine_world.character, 16.0, 128.0);
    }
    for(auto entityPair : engine_world.entity_tree)
    {
        entityPair.second->updateInterpolation(time);
    }

    Controls_RefreshStates();
    engine_world.updateAnimTextures();
}

void Game_Prepare()
{
    if(engine_world.character)
    {
        // Set character values to default.

        engine_world.character->setParamMaximum(world::PARAM_HEALTH, LARA_PARAM_HEALTH_MAX);
        engine_world.character->setParam(world::PARAM_HEALTH, LARA_PARAM_HEALTH_MAX);
        engine_world.character->setParamMaximum(world::PARAM_AIR, LARA_PARAM_AIR_MAX);
        engine_world.character->setParam(world::PARAM_AIR, LARA_PARAM_AIR_MAX);
        engine_world.character->setParamMaximum(world::PARAM_STAMINA, LARA_PARAM_STAMINA_MAX);
        engine_world.character->setParam(world::PARAM_STAMINA, LARA_PARAM_STAMINA_MAX);
        engine_world.character->setParamMaximum(world::PARAM_WARMTH, LARA_PARAM_WARMTH_MAX);
        engine_world.character->setParam(world::PARAM_WARMTH, LARA_PARAM_WARMTH_MAX);
        engine_world.character->setParamMaximum(world::PARAM_POISON, LARA_PARAM_POISON_MAX);
        engine_world.character->setParam(world::PARAM_POISON, 0);

        // Set character statistics to default.

        engine_world.character->m_statistics.distance = 0.0;
        engine_world.character->m_statistics.ammo_used = 0;
        engine_world.character->m_statistics.hits = 0;
        engine_world.character->m_statistics.kills = 0;
        engine_world.character->m_statistics.medipacks_used = 0;
        engine_world.character->m_statistics.saves_used = 0;
        engine_world.character->m_statistics.secrets_game = 0;
        engine_world.character->m_statistics.secrets_level = 0;
    }
    else if(!engine_world.rooms.empty())
    {
        // If there is no character present, move default camera position to
        // the first room (useful for TR1-3 cutscene levels).

        engine_camera.setPosition(engine_world.rooms[0]->boundingBox.max);
    }

    // Set gameflow parameters to default.
    // Reset secret trigger map.

    Gameflow_Manager.resetSecretStatus();
}

void Game_LevelTransition(uint16_t level_index)
{
    char file_path[MAX_ENGINE_PATH];

    engine_lua.getLoadingScreen(level_index, file_path);
    gui::fadeAssignPic(gui::FaderType::LoadScreen, file_path);
    gui::fadeStart(gui::FaderType::LoadScreen, gui::FaderDir::Out);

    engine::engine_world.audioEngine.endStreams();
}

} // namespace engine
