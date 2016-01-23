#include "game.h"

#include <cstdio>

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

#include "LuaState.h"

#include "audio/audio.h"
#include "character_controller.h"
#include "controls.h"
#include "engine.h"
#include "engine/bullet.h"
#include "gameflow.h"
#include "gui/console.h"
#include "gui/fader.h"
#include "gui/fadermanager.h"
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

#include <glm/gtc/random.hpp>

#include <boost/log/trivial.hpp>
#include <boost/range/adaptors.hpp>

#include <fstream>

extern float time_scale;
extern script::MainEngine engine_lua;

constexpr float CameraCollisionSphereRadius = 16.0f;

namespace engine
{

void Save_EntityTree(std::ostream& f, const std::map<uint32_t, std::shared_ptr<world::Entity> > &map);
void Save_Entity(std::ostream& f, std::shared_ptr<world::Entity> ent);

using gui::Console;

void lua_mlook(lua::Value mlook)
{
    if(!mlook.is<lua::Boolean>())
    {
        control_states.mouse_look = !control_states.mouse_look;
        Console::instance().printf("mlook = %d", control_states.mouse_look);
        return;
    }

    control_states.mouse_look = mlook.toBool();
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

    control_states.free_look = free.toBool();
    Console::instance().printf("free_look = %d", control_states.free_look);
}

void lua_cam_distance(lua::Value distance)
{
    if(!distance.is<lua::Number>())
    {
        Console::instance().printf("cam_distance = %.2f", control_states.cam_distance);
        return;
    }

    control_states.cam_distance = distance.toFloat();
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
        control_states.noclip = noclip.toBool();
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
        screen_info.show_debuginfo = show.toBool();
    }

    Console::instance().printf("debug info = %d", screen_info.show_debuginfo);
}

void lua_timescale(lua::Value scale)
{
    if(!scale.is<lua::Number>())
    {
        if(util::fuzzyOne(time_scale))
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
        time_scale = scale.toFloat();
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
        if(*ch == '\\' || *ch == '/')
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
            BOOST_LOG_TRIVIAL(warning) << "Can not read file " << token;
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
            BOOST_LOG_TRIVIAL(error) << error.what();
        }
        catch(lua::LoadError& error)
        {
            BOOST_LOG_TRIVIAL(error) << error.what();
        }
    }
    else
    {
        f = fopen(name, "rb");
        if(f == nullptr)
        {
            BOOST_LOG_TRIVIAL(error) << "Can not read file " << name;
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
            BOOST_LOG_TRIVIAL(error) << error.what();
        }
        catch(lua::LoadError& error)
        {
            BOOST_LOG_TRIVIAL(error) << error.what();
        }
    }

    return 1;
}

void Save_EntityTree(std::ostream& f, const std::map<uint32_t, std::shared_ptr<world::Entity> >& map)
{
    for(const std::shared_ptr<world::Entity>& entity : map | boost::adaptors::map_values)
    {
        Save_Entity(f, entity);
    }
}

/**
 * Entity save function, based on engine lua scripts;
 */
void Save_Entity(std::ostream& f, std::shared_ptr<world::Entity> ent)
{
    if(ent == nullptr)
    {
        return;
    }

    if(ent->m_typeFlags & ENTITY_TYPE_SPAWNED)
    {
        world::ObjectId room_id = ent->getRoom() ? ent->getRoom()->getId() : 0xFFFFFFFF;
        f << boost::format("\nspawnEntity(%d, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %d, %d);")
                % ent->m_skeleton.getModel()->id
                % ent->m_transform[3][0]
                % ent->m_transform[3][1]
                % ent->m_transform[3][2]
                % ent->m_angles[0]
                % ent->m_angles[1]
                % ent->m_angles[2]
                % room_id
                % ent->getId();
    }
    else
    {
        f << boost::format("\nsetEntityPos(%d, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f);")
                % ent->getId()
                % ent->m_transform[3][0]
                % ent->m_transform[3][1]
                % ent->m_transform[3][2]
                % ent->m_angles[0]
                % ent->m_angles[1]
                % ent->m_angles[2];
    }

    f << boost::format("\nsetEntitySpeed(%d, %.2f, %.2f, %.2f);")
         % ent->getId()
         % ent->m_speed[0]
         % ent->m_speed[1]
         % ent->m_speed[2];
    f << boost::format("\nsetEntityAnim(%d, %d, %d);")
         % ent->getId()
         % ent->m_skeleton.getCurrentAnimation()
         % ent->m_skeleton.getCurrentFrame();
    f << boost::format("\nsetEntityState(%d, %d, %d);")
         % ent->getId()
         % ent->m_skeleton.getCurrentState()
         % ent->m_skeleton.getPreviousState();
    f << boost::format("\nsetEntityCollisionFlags(%d, %ld, %ld);")
         % ent->getId()
         % ent->getCollisionType()
         % ent->getCollisionShape();

    if(ent->m_enabled)
    {
        f << boost::format("\nenableEntity(%d);")
             % ent->getId();
    }
    else
    {
        f << boost::format("\ndisableEntity(%d);")
             % ent->getId();
    }

    f << boost::format("\nsetEntityFlags(%d, %d, %d, %d, 0x%.4X, 0x%.8X);")
         % ent->getId()
         % ent->m_active
         % ent->m_enabled
         % ent->m_visible
         % ent->m_typeFlags
         % ent->m_callbackFlags;

    f << boost::format("\nsetEntityTriggerLayout(%d, 0x%.2X);")
         % ent->getId()
         % ent->m_triggerLayout;
    //setEntityMeshswap()

    if(ent->getRoom() != nullptr)
    {
        f << boost::format("\nsetEntityRoomMove(%d, %d, %d, %d);")
             % ent->getId()
             % ent->getRoom()->getId()
             % ent->m_moveType
             % ent->m_moveDir;
    }
    else
    {
        f << boost::format("\nsetEntityRoomMove(%d, nil, %d, %d);")
             % ent->getId()
             % ent->m_moveType
             % ent->m_moveDir;
    }

    if(auto ch = std::dynamic_pointer_cast<world::Character>(ent))
    {
        f << boost::format("\nremoveAllItems(%d);")
             % ent->getId();
        for(const InventoryNode& i : ch->m_inventory)
        {
            f << boost::format("\naddItem(%d, %d, %d);")
                 % ent->getId()
                 % i.id
                 % i.count;
        }

        for(int i = 0; i < world::PARAM_SENTINEL; i++)
        {
            f << boost::format("\nsetCharacterParam(%d, %d, %.2f, %.2f);")
                 % ent->getId()
                 % i
                 % ch->m_parameters.param[i]
                 % ch->m_parameters.maximum[i];
        }
    }
}

/**
 * Save current game state
 */
bool Game_Save(const std::string& name)
{
    const bool local = (name.find_first_of("\\/") == std::string::npos);

    std::ofstream f;
    if(local)
    {
        f.open("save/" + name, std::ios::binary | std::ios::trunc);
    }
    else
    {
        f.open("name", std::ios::binary | std::ios::trunc);
    }

    if(!f.is_open())
    {
        BOOST_LOG_TRIVIAL(warning) << "Can not create file " << name;
        return false;
    }

    f << boost::format("loadMap(\"%s\", %d, %d);\n")
         % Gameflow::instance.getLevelPath()
         % Gameflow::instance.getGameID()
         % Gameflow::instance.getLevelID();

    // Save flipmap and flipped room states.

    for(size_t i = 0; i < engine_world.flip_data.size(); i++)
    {
        f << boost::format("setFlipMap(%d, 0x%02X, 0);\n")
             % i
             % engine_world.flip_data[i].map;
        f << boost::format("setFlipState(%d, %s);\n")
             % i
             % (engine_world.flip_data[i].state ? "true" : "false");
    }

    Save_Entity(f, engine_world.character);    // Save Lara.

    Save_EntityTree(f, engine_world.entity_tree);

    return true;
}

void Game_ApplyControls(std::shared_ptr<world::Entity> ent)
{
    // Keyboard move logic
    world::Movement moveLogic;
    moveLogic.setX(control_states.move_left, control_states.move_right);
    moveLogic.setY(control_states.move_up, control_states.move_down);
    moveLogic.setZ(control_states.move_forward, control_states.move_backward);

    // Keyboard look logic
    world::Movement lookLogic;
    lookLogic.setX(control_states.look_left, control_states.look_right);
    lookLogic.setY(control_states.look_up, control_states.look_down);
    lookLogic.setZ(control_states.look_roll_left, control_states.look_roll_right);

    // APPLY CONTROLS

    engine::engine_camera.rotate( lookLogic.getDistance(glm::radians(2.2f) * util::toSeconds(engine_frame_time)) );

    // FIXME: Duplicate code - do we need cam control with no world??
    if(!render::renderer.world())
    {
        if(control_mapper.use_joy)
        {
            if(control_mapper.joy_look_x != 0)
            {
                engine::engine_camera.rotate( {-world::CameraRotationSpeed * util::toSeconds(engine_frame_time) * control_mapper.joy_look_x, 0, 0} );
            }
            if(control_mapper.joy_look_y != 0)
            {
                engine::engine_camera.rotate( {0, -world::CameraRotationSpeed * util::toSeconds(engine_frame_time) * control_mapper.joy_look_y, 0} );
            }
        }

        if(control_states.mouse_look)
        {
            engine::engine_camera.rotate({ -world::CameraRotationSpeed * control_states.look_axis_x, -world::CameraRotationSpeed * control_states.look_axis_y, 0 });
            control_states.look_axis_x = 0.0;
            control_states.look_axis_y = 0.0;
        }

        render::renderer.camera()->applyRotation();
        glm::float_t dist = control_states.state_walk ? control_states.free_look_speed * util::toSeconds(engine_frame_time) * 0.3f : control_states.free_look_speed * util::toSeconds(engine_frame_time);
        render::renderer.camera()->move(moveLogic.getDistance(dist));

        return;
    }

    if(control_mapper.use_joy)
    {
        if(control_mapper.joy_look_x != 0)
        {
            engine::engine_camera.rotate({ -world::CameraRotationSpeed * util::toSeconds(engine_frame_time) * control_mapper.joy_look_x, 0, 0 });
        }
        if(control_mapper.joy_look_y != 0)
        {
            engine::engine_camera.rotate({ 0, -world::CameraRotationSpeed * util::toSeconds(engine_frame_time) * control_mapper.joy_look_y, 0 });
        }
    }

    if(control_states.mouse_look)
    {
        engine::engine_camera.rotate({ -world::CameraRotationSpeed * control_states.look_axis_x, -world::CameraRotationSpeed * control_states.look_axis_y, 0 });
        control_states.look_axis_x = 0.0;
        control_states.look_axis_y = 0.0;
    }

    if(control_states.free_look || !std::dynamic_pointer_cast<world::Character>(ent))
    {
        glm::float_t dist = control_states.state_walk ? control_states.free_look_speed * util::toSeconds(engine_frame_time) * 0.3f : control_states.free_look_speed * util::toSeconds(engine_frame_time);
        render::renderer.camera()->applyRotation();
        render::renderer.camera()->move(moveLogic.getDistance(dist));
        render::renderer.camera()->setCurrentRoom( Room_FindPosCogerrence(render::renderer.camera()->getPosition(), render::renderer.camera()->getCurrentRoom()) );
    }
    else if(control_states.noclip)
    {
        glm::float_t dist = control_states.state_walk ? control_states.free_look_speed * util::toSeconds(engine_frame_time) * 0.3f : control_states.free_look_speed * util::toSeconds(engine_frame_time);
        render::renderer.camera()->applyRotation();
        render::renderer.camera()->move(moveLogic.getDistance(dist));
        render::renderer.camera()->setCurrentRoom( Room_FindPosCogerrence(render::renderer.camera()->getPosition(), render::renderer.camera()->getCurrentRoom()) );

        ent->m_angles[0] = glm::degrees(engine::engine_camera.getAngles()[0]);
        glm::vec3 position = render::renderer.camera()->getPosition() + render::renderer.camera()->getViewDir() * control_states.cam_distance;
        position[2] -= 512.0;
        ent->m_transform[3] = glm::vec4(position, 1.0f);
        ent->updateTransform();
    }
    else
    {
        std::shared_ptr<world::Character> ch = std::dynamic_pointer_cast<world::Character>(ent);
        // Apply controls to Lara
        ch->m_command.action = control_states.state_action;
        ch->m_command.ready_weapon = control_states.do_draw_weapon;
        ch->m_command.jump = control_states.do_jump;
        ch->m_command.shift = control_states.state_walk;

        ch->m_command.roll = (control_states.move_forward && control_states.move_backward) || control_states.do_roll;

        // New commands only for TR3 and above
        ch->m_command.sprint = control_states.state_sprint;
        ch->m_command.crouch = control_states.state_crouch;

        if(control_states.use_small_medi)
        {
            if(ch->getItemsCount(ITEM_SMALL_MEDIPACK) > 0 && ch->changeParam(world::PARAM_HEALTH, 250))
            {
                ch->setParam(world::PARAM_POISON, 0);
                ch->removeItem(ITEM_SMALL_MEDIPACK, 1);
                engine::engine_world.audioEngine.send(audio::SoundMedipack);
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
                engine::engine_world.audioEngine.send(audio::SoundMedipack);
            }

            control_states.use_big_medi = !control_states.use_big_medi;
        }

        if(control_mapper.use_joy && control_mapper.joy_move_x != 0)
        {
            ch->m_command.rot[0] += glm::degrees(-2 * util::toSeconds(engine_frame_time) * control_mapper.joy_move_x);
        }
        else
        {
            ch->m_command.rot[0] += moveLogic.getDistanceX( glm::degrees(-2.0f) * util::toSeconds(engine_frame_time) );
        }

        if(control_mapper.use_joy && control_mapper.joy_move_y != 0)
        {
            ch->m_command.rot[1] += glm::degrees(-2 * util::toSeconds(engine_frame_time) * control_mapper.joy_move_y);
        }
        else
        {
            ch->m_command.rot[1] += moveLogic.getDistanceZ( glm::degrees(2.0f) * util::toSeconds(engine_frame_time) );
        }

        ch->m_command.move = moveLogic;
    }
}

bool Cam_HasHit(std::shared_ptr<BtEngineClosestConvexResultCallback> cb, btTransform &cameraFrom, btTransform &cameraTo)
{
    btSphereShape cameraSphere(CameraCollisionSphereRadius);
    cameraSphere.setMargin(COLLISION_MARGIN_DEFAULT);
    cb->m_closestHitFraction = 1.0;
    cb->m_hitCollisionObject = nullptr;
    BulletEngine::instance->dynamicsWorld->convexSweepTest(&cameraSphere, cameraFrom, cameraTo, *cb);
    return cb->hasHit();
}

void Cam_FollowEntity(world::Camera *cam, std::shared_ptr<world::Entity> ent, glm::float_t dx, glm::float_t dz)
{
    btTransform cameraFrom = btTransform::getIdentity();
    btTransform cameraTo = btTransform::getIdentity();

    std::shared_ptr<BtEngineClosestConvexResultCallback> cb = ent->callbackForCamera();

    glm::vec3 cam_pos = cam->getPosition();

    ///@INFO Basic camera override, completely placeholder until a system classic-like is created
    if(!control_states.mouse_look)//If mouse look is off
    {
        glm::float_t currentAngle = engine::engine_camera.getAngles()[0];  //Current is the current cam angle
        glm::float_t targetAngle = glm::radians(ent->m_angles[0]); //Target is the target angle which is the entity's angle itself

        ///@FIXME
        //If Lara is in a specific state we want to rotate -75 deg or +75 deg depending on camera collision
        if(ent->m_skeleton.getPreviousState() == world::LaraState::Reach)
        {
            if(cam->getTargetDir() == world::CameraTarget::Back)
            {
                glm::vec3 cam_pos2 = cam_pos;
                cameraFrom.setOrigin(util::convert(cam_pos2));
                cam_pos2[0] += glm::sin(glm::radians(ent->m_angles[0] - 90.0f)) * control_states.cam_distance;
                cam_pos2[1] -= glm::cos(glm::radians(ent->m_angles[0] - 90.0f)) * control_states.cam_distance;
                cameraTo.setOrigin(util::convert(cam_pos2));

                //If collided we want to go right otherwise stay left
                if(Cam_HasHit(cb, cameraFrom, cameraTo))
                {
                    cam_pos2 = cam_pos;
                    cameraFrom.setOrigin(util::convert(cam_pos2));
                    cam_pos2[0] += glm::sin(glm::radians(ent->m_angles[0] + 90.0f)) * control_states.cam_distance;
                    cam_pos2[1] -= glm::cos(glm::radians(ent->m_angles[0] + 90.0f)) * control_states.cam_distance;
                    cameraTo.setOrigin(util::convert(cam_pos2));

                    //If collided we want to go to back else right
                    if(Cam_HasHit(cb, cameraFrom, cameraTo))
                        cam->setTargetDir( world::CameraTarget::Back );
                    else
                        cam->setTargetDir( world::CameraTarget::Right );
                }
                else
                {
                    cam->setTargetDir( world::CameraTarget::Left );
                }
            }
        }
        else if(ent->m_skeleton.getPreviousState() == world::LaraState::JumpBack)
        {
            cam->setTargetDir( world::CameraTarget::Front );
        }
        else if(cam->getTargetDir() != world::CameraTarget::Back)
        {
            cam->setTargetDir( world::CameraTarget::Back );
        }

        //If target mis-matches current we need to update the camera's angle to reach target!
        if(currentAngle != targetAngle)
        {
            switch(cam->getTargetDir())
            {
                case world::CameraTarget::Back:
                    targetAngle = glm::radians(ent->m_angles[0]);
                    break;
                case world::CameraTarget::Front:
                    targetAngle = glm::radians(ent->m_angles[0] - 180.0f);
                    break;
                case world::CameraTarget::Left:
                    targetAngle = glm::radians(ent->m_angles[0] - 75.0f);
                    break;
                case world::CameraTarget::Right:
                    targetAngle = glm::radians(ent->m_angles[0] + 75.0f);
                    break;
                default:
                    targetAngle = glm::radians(ent->m_angles[0]);//Same as TR_CAM_TARG_BACK (default pos)
                    break;
            }

            engine::engine_camera.shake(currentAngle, targetAngle, engine_frame_time);
        }
    }

    cam_pos = ent->camPosForFollowing(dz);

    //Code to manage screen shaking effects
    if(render::renderer.camera()->getShakeTime().count()>0 && render::renderer.camera()->getShakeValue() > 0.0)
    {
        cam_pos += glm::ballRand(render::renderer.camera()->getShakeValue()/2.0f) * util::toSeconds(render::renderer.camera()->getShakeTime());
        render::renderer.camera()->setShakeTime( std::max(util::Duration(0), render::renderer.camera()->getShakeTime() - engine_frame_time) );
    }

    cameraFrom.setOrigin(util::convert(cam_pos));
    cam_pos[2] += dz;
    cameraTo.setOrigin(util::convert(cam_pos));
    if(Cam_HasHit(cb, cameraFrom, cameraTo))
    {
        cam_pos = glm::mix(util::convert(cameraFrom.getOrigin()), util::convert(cameraTo.getOrigin()), cb->m_closestHitFraction);
        cam_pos += util::convert(cb->m_hitNormalWorld * 2.0);
    }

    if(dx != 0.0)
    {
        cameraFrom.setOrigin(util::convert(cam_pos));
        cam_pos += dx * cam->getRightDir();
        cameraTo.setOrigin(util::convert(cam_pos));
        if(Cam_HasHit(cb, cameraFrom, cameraTo))
        {
            cam_pos = glm::mix(util::convert(cameraFrom.getOrigin()), util::convert(cameraTo.getOrigin()), cb->m_closestHitFraction);
            cam_pos += util::convert(cb->m_hitNormalWorld * 2.0);
        }

        cameraFrom.setOrigin(util::convert(cam_pos));

        {
            glm::float_t cos_ay =  glm::cos(engine::engine_camera.getAngles()[1]);
            glm::float_t cam_dx =  glm::sin(engine::engine_camera.getAngles()[0]) * cos_ay;
            glm::float_t cam_dy = -glm::cos(engine::engine_camera.getAngles()[0]) * cos_ay;
            glm::float_t cam_dz = -glm::sin(engine::engine_camera.getAngles()[1]);
            cam_pos[0] += cam_dx * control_states.cam_distance;
            cam_pos[1] += cam_dy * control_states.cam_distance;
            cam_pos[2] += cam_dz * control_states.cam_distance;
        }

        cameraTo.setOrigin(util::convert(cam_pos));
        if(Cam_HasHit(cb, cameraFrom, cameraTo))
        {
            cam_pos = glm::mix(util::convert(cameraFrom.getOrigin()), util::convert(cameraTo.getOrigin()), cb->m_closestHitFraction);
            cam_pos += util::convert(cb->m_hitNormalWorld * 2.0);
        }
    }

    //Update cam pos
    cam->setPosition( cam_pos );

    //Modify cam pos for quicksand rooms
    cam->setCurrentRoom( Room_FindPosCogerrence(cam->getPosition() - glm::vec3(0, 0, 128), cam->getCurrentRoom()) );
    if(cam->getCurrentRoom() && (cam->getCurrentRoom()->m_flags & TR_ROOM_FLAG_QUICKSAND))
    {
        glm::vec3 position = cam->getPosition();
        position[2] = cam->getCurrentRoom()->m_boundingBox.max[2] + 2.0f * 64.0f;
        cam->setPosition(position);
    }

    cam->applyRotation();
    cam->setCurrentRoom( Room_FindPosCogerrence(cam->getPosition(), cam->getCurrentRoom()) );
}

void Game_UpdateAI()
{
    //for(ALL CHARACTERS, EXCEPT PLAYER)
    {
        // UPDATE AI commands
    }
}

inline util::Duration Game_Tick(util::Duration *game_logic_time)
{
    int t = static_cast<int>(*game_logic_time / world::animation::GameLogicFrameTime);
    util::Duration dt = t * world::animation::GameLogicFrameTime;
    *game_logic_time -= dt;
    return dt;
}


void Game_Frame(util::Duration time)
{
    static util::Duration game_logic_time = util::Duration(0);
    static const util::Duration SimulationTime = MAX_SIM_SUBSTEPS * world::animation::GameLogicFrameTime;

    // clamp frameskip at max substeps - if more frames are dropped, slow everything down:
    if(time > SimulationTime)
    {
        time = SimulationTime;
        engine_frame_time = time;   // FIXME
    }
    game_logic_time += time;


    // GUI and controls should be updated at all times!
    Controls_PollSDLInput();

    // TODO: implement pause mechanism
    if(gui::Gui::instance->update())
    {
        if(game_logic_time >= world::animation::GameLogicFrameTime)
        {
            engine::engine_world.audioEngine.updateAudio();
            Game_Tick(&game_logic_time);
        }
        return;
    }

    // Translate input to character commands, move camera:
    // TODO: decouple cam movement
    Game_ApplyControls(engine_world.character);

    BulletEngine::instance->dynamicsWorld->stepSimulation(util::toSeconds(time), MAX_SIM_SUBSTEPS, util::toSeconds(world::animation::GameLogicFrameTime));

    if(engine_world.character) {
        engine_world.character->updateInterpolation();

        if(!control_states.noclip && !control_states.free_look)
            Cam_FollowEntity(render::renderer.camera(), engine_world.character, 16.0, 128.0);
    }
    for(const std::shared_ptr<world::Entity>& entity : engine_world.entity_tree | boost::adaptors::map_values)
    {
        entity->updateInterpolation();
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

        engine_camera.setPosition(engine_world.rooms[0]->m_boundingBox.max);
    }

    // Set gameflow parameters to default.
    // Reset secret trigger map.

    Gameflow::instance.resetSecretStatus();
}

void Game_LevelTransition(uint16_t level_index)
{
    gui::Gui::instance->faders.assignPicture(gui::FaderType::LoadScreen, engine_lua.getLoadingScreen(level_index));
    gui::Gui::instance->faders.start(gui::FaderType::LoadScreen, gui::FaderDir::Out);

    engine::engine_world.audioEngine.endStreams();
}

} // namespace engine
