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
        Engine::instance.m_controlState.m_mouseLook = !Engine::instance.m_controlState.m_mouseLook;
        Console::instance().printf("mlook = %d", Engine::instance.m_controlState.m_mouseLook);
        return;
    }

    Engine::instance.m_controlState.m_mouseLook = mlook.toBool();
    Console::instance().printf("mlook = %d", Engine::instance.m_controlState.m_mouseLook);
}

void lua_freelook(lua::Value free)
{
    if(!free.is<lua::Boolean>())
    {
        Engine::instance.m_controlState.m_freeLook = !Engine::instance.m_controlState.m_freeLook;
        Console::instance().printf("free_look = %d", Engine::instance.m_controlState.m_freeLook);
        return;
    }

    Engine::instance.m_controlState.m_freeLook = free.toBool();
    Console::instance().printf("free_look = %d", Engine::instance.m_controlState.m_freeLook);
}

void lua_cam_distance(lua::Value distance)
{
    if(!distance.is<lua::Number>())
    {
        Console::instance().printf("cam_distance = %.2f", Engine::instance.m_controlState.m_camDistance);
        return;
    }

    Engine::instance.m_controlState.m_camDistance = distance.toFloat();
    Console::instance().printf("cam_distance = %.2f", Engine::instance.m_controlState.m_camDistance);
}

void lua_noclip(lua::Value noclip)
{
    if(!noclip.is<lua::Boolean>())
    {
        Engine::instance.m_controlState.m_noClip = !Engine::instance.m_controlState.m_noClip;
    }
    else
    {
        Engine::instance.m_controlState.m_noClip = noclip.toBool();
    }

    Console::instance().printf("noclip = %d", Engine::instance.m_controlState.m_noClip);
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
    Engine::instance.m_controlState.m_freeLookSpeed = 3000.0;
    Engine::instance.m_controlState.m_mouseLook = true;
    Engine::instance.m_controlState.m_freeLook = false;
    Engine::instance.m_controlState.m_noClip = false;
    Engine::instance.m_controlState.m_camDistance = 800.0;
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
bool Game_Load(const std::string& name)
{
    const bool local = (name.find_first_of("\\/") != std::string::npos);

    if(local)
    {
        std::string token = "save/" + name;
        FILE* f = fopen(token.c_str(), "rb");
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
        FILE* f = fopen(name.c_str(), "rb");
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
        f.open(name, std::ios::binary | std::ios::trunc);
    }

    if(!f.is_open())
    {
        BOOST_LOG_TRIVIAL(warning) << "Cannot create file " << name;
        return false;
    }

    f << boost::format("loadMap(\"%s\", %d, %d);\n")
        % Gameflow::instance.getLevelPath()
        % Gameflow::instance.getGameID()
        % Gameflow::instance.getLevelID();

    // Save flipmap and flipped room states.

    for(size_t i = 0; i < Engine::instance.m_world.flip_data.size(); i++)
    {
        f << boost::format("setFlipMap(%d, 0x%02X, 0);\n")
            % i
            % Engine::instance.m_world.flip_data[i].map;
        f << boost::format("setFlipState(%d, %s);\n")
            % i
            % (Engine::instance.m_world.flip_data[i].state ? "true" : "false");
    }

    Save_Entity(f, Engine::instance.m_world.character);    // Save Lara.

    Save_EntityTree(f, Engine::instance.m_world.entity_tree);

    return true;
}

void Game_ApplyControls(std::shared_ptr<world::Entity> ent)
{
    // Keyboard move logic
    world::Movement moveLogic;
    moveLogic.setX(Engine::instance.m_controlState.m_moveLeft, Engine::instance.m_controlState.m_moveRight);
    moveLogic.setY(Engine::instance.m_controlState.m_moveUp, Engine::instance.m_controlState.m_moveDown);
    moveLogic.setZ(Engine::instance.m_controlState.m_moveForward, Engine::instance.m_controlState.m_moveBackward);

    // Keyboard look logic
    world::Movement lookLogic;
    lookLogic.setX(Engine::instance.m_controlState.m_lookLeft, Engine::instance.m_controlState.m_lookRight);
    lookLogic.setY(Engine::instance.m_controlState.m_lookUp, Engine::instance.m_controlState.m_lookDown);
    lookLogic.setZ(Engine::instance.m_controlState.m_lookRollLeft, Engine::instance.m_controlState.m_lookRollRight);

    // APPLY CONTROLS

    engine::Engine::instance.m_camera.rotate(lookLogic.getDistance(glm::radians(2.2f) * util::toSeconds(Engine::instance.m_frameTime)));

    // FIXME: Duplicate code - do we need cam control with no world??
    if(!render::renderer.world())
    {
        if(ControlSettings::instance.use_joy)
        {
            if(ControlSettings::instance.joy_look_x != 0)
            {
                engine::Engine::instance.m_camera.rotate({ -world::CameraRotationSpeed * util::toSeconds(Engine::instance.m_frameTime) * ControlSettings::instance.joy_look_x, 0, 0 });
            }
            if(ControlSettings::instance.joy_look_y != 0)
            {
                engine::Engine::instance.m_camera.rotate({ 0, -world::CameraRotationSpeed * util::toSeconds(Engine::instance.m_frameTime) * ControlSettings::instance.joy_look_y, 0 });
            }
        }

        if(Engine::instance.m_controlState.m_mouseLook)
        {
            engine::Engine::instance.m_camera.rotate({ -world::CameraRotationSpeed * Engine::instance.m_controlState.m_lookAxisX,
                                                            -world::CameraRotationSpeed * Engine::instance.m_controlState.m_lookAxisY,
                                                            0 });
            Engine::instance.m_controlState.m_lookAxisX = 0.0;
            Engine::instance.m_controlState.m_lookAxisY = 0.0;
        }

        render::renderer.camera()->applyRotation();
        glm::float_t dist = Engine::instance.m_controlState.m_stateWalk
            ? Engine::instance.m_controlState.m_freeLookSpeed * util::toSeconds(Engine::instance.m_frameTime) * 0.3f
            : Engine::instance.m_controlState.m_freeLookSpeed * util::toSeconds(Engine::instance.m_frameTime);
        render::renderer.camera()->move(moveLogic.getDistance(dist));

        return;
    }

    if(ControlSettings::instance.use_joy)
    {
        if(ControlSettings::instance.joy_look_x != 0)
        {
            engine::Engine::instance.m_camera.rotate({ -world::CameraRotationSpeed * util::toSeconds(Engine::instance.m_frameTime) * ControlSettings::instance.joy_look_x, 0, 0 });
        }
        if(ControlSettings::instance.joy_look_y != 0)
        {
            engine::Engine::instance.m_camera.rotate({ 0, -world::CameraRotationSpeed * util::toSeconds(Engine::instance.m_frameTime) * ControlSettings::instance.joy_look_y, 0 });
        }
    }

    if(Engine::instance.m_controlState.m_mouseLook)
    {
        engine::Engine::instance.m_camera.rotate({ -world::CameraRotationSpeed * Engine::instance.m_controlState.m_lookAxisX,
                                                        -world::CameraRotationSpeed * Engine::instance.m_controlState.m_lookAxisY,
                                                        0 });
        Engine::instance.m_controlState.m_lookAxisX = 0.0;
        Engine::instance.m_controlState.m_lookAxisY = 0.0;
    }

    if(Engine::instance.m_controlState.m_freeLook || !std::dynamic_pointer_cast<world::Character>(ent))
    {
        glm::float_t dist = Engine::instance.m_controlState.m_stateWalk
            ? Engine::instance.m_controlState.m_freeLookSpeed * util::toSeconds(Engine::instance.m_frameTime) * 0.3f
            : Engine::instance.m_controlState.m_freeLookSpeed * util::toSeconds(Engine::instance.m_frameTime);
        render::renderer.camera()->applyRotation();
        render::renderer.camera()->move(moveLogic.getDistance(dist));
        render::renderer.camera()->setCurrentRoom(Room_FindPosCogerrence(render::renderer.camera()->getPosition(), render::renderer.camera()->getCurrentRoom()));
    }
    else if(Engine::instance.m_controlState.m_noClip)
    {
        glm::float_t dist = Engine::instance.m_controlState.m_stateWalk
            ? Engine::instance.m_controlState.m_freeLookSpeed * util::toSeconds(Engine::instance.m_frameTime) * 0.3f
            : Engine::instance.m_controlState.m_freeLookSpeed * util::toSeconds(Engine::instance.m_frameTime);
        render::renderer.camera()->applyRotation();
        render::renderer.camera()->move(moveLogic.getDistance(dist));
        render::renderer.camera()->setCurrentRoom(Room_FindPosCogerrence(render::renderer.camera()->getPosition(), render::renderer.camera()->getCurrentRoom()));

        ent->m_angles[0] = glm::degrees(engine::Engine::instance.m_camera.getAngles()[0]);
        glm::vec3 position = render::renderer.camera()->getPosition() + render::renderer.camera()->getViewDir() * Engine::instance.m_controlState.m_camDistance;
        position[2] -= 512.0;
        ent->m_transform[3] = glm::vec4(position, 1.0f);
        ent->updateTransform();
    }
    else
    {
        std::shared_ptr<world::Character> ch = std::dynamic_pointer_cast<world::Character>(ent);
        // Apply controls to Lara
        ch->m_command.action = Engine::instance.m_controlState.m_stateAction;
        ch->m_command.ready_weapon = Engine::instance.m_controlState.m_doDrawWeapon;
        ch->m_command.jump = Engine::instance.m_controlState.m_doJump;
        ch->m_command.shift = Engine::instance.m_controlState.m_stateWalk;

        ch->m_command.roll = (Engine::instance.m_controlState.m_moveForward && Engine::instance.m_controlState.m_moveBackward) || Engine::instance.m_controlState.m_doRoll;

        // New commands only for TR3 and above
        ch->m_command.sprint = Engine::instance.m_controlState.m_stateSprint;
        ch->m_command.crouch = Engine::instance.m_controlState.m_stateCrouch;

        if(Engine::instance.m_controlState.m_useSmallMedi)
        {
            if(ch->getItemsCount(ITEM_SMALL_MEDIPACK) > 0 && ch->changeParam(world::PARAM_HEALTH, 250))
            {
                ch->setParam(world::PARAM_POISON, 0);
                ch->removeItem(ITEM_SMALL_MEDIPACK, 1);
                engine::Engine::instance.m_world.audioEngine.send(audio::SoundMedipack);
            }

            Engine::instance.m_controlState.m_useSmallMedi = !Engine::instance.m_controlState.m_useSmallMedi;
        }

        if(Engine::instance.m_controlState.m_useBigMedi)
        {
            if(ch->getItemsCount(ITEM_LARGE_MEDIPACK) > 0 &&
               ch->changeParam(world::PARAM_HEALTH, LARA_PARAM_HEALTH_MAX))
            {
                ch->setParam(world::PARAM_POISON, 0);
                ch->removeItem(ITEM_LARGE_MEDIPACK, 1);
                engine::Engine::instance.m_world.audioEngine.send(audio::SoundMedipack);
            }

            Engine::instance.m_controlState.m_useBigMedi = !Engine::instance.m_controlState.m_useBigMedi;
        }

        if(ControlSettings::instance.use_joy && ControlSettings::instance.joy_move_x != 0)
        {
            ch->m_command.rot[0] += glm::degrees(-2 * util::toSeconds(Engine::instance.m_frameTime) * ControlSettings::instance.joy_move_x);
        }
        else
        {
            ch->m_command.rot[0] += moveLogic.getDistanceX(glm::degrees(-2.0f) * util::toSeconds(Engine::instance.m_frameTime));
        }

        if(ControlSettings::instance.use_joy && ControlSettings::instance.joy_move_y != 0)
        {
            ch->m_command.rot[1] += glm::degrees(-2 * util::toSeconds(Engine::instance.m_frameTime) * ControlSettings::instance.joy_move_y);
        }
        else
        {
            ch->m_command.rot[1] += moveLogic.getDistanceZ(glm::degrees(2.0f) * util::toSeconds(Engine::instance.m_frameTime));
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
    if(!Engine::instance.m_controlState.m_mouseLook)//If mouse look is off
    {
        glm::float_t currentAngle = engine::Engine::instance.m_camera.getAngles()[0];  //Current is the current cam angle
        glm::float_t targetAngle = glm::radians(ent->m_angles[0]); //Target is the target angle which is the entity's angle itself

        ///@FIXME
        //If Lara is in a specific state we want to rotate -75 deg or +75 deg depending on camera collision
        if(ent->m_skeleton.getPreviousState() == world::LaraState::Reach)
        {
            if(cam->getTargetDir() == world::CameraTarget::Back)
            {
                glm::vec3 cam_pos2 = cam_pos;
                cameraFrom.setOrigin(util::convert(cam_pos2));
                cam_pos2[0] += glm::sin(glm::radians(ent->m_angles[0] - 90.0f)) * Engine::instance.m_controlState.m_camDistance;
                cam_pos2[1] -= glm::cos(glm::radians(ent->m_angles[0] - 90.0f)) * Engine::instance.m_controlState.m_camDistance;
                cameraTo.setOrigin(util::convert(cam_pos2));

                //If collided we want to go right otherwise stay left
                if(Cam_HasHit(cb, cameraFrom, cameraTo))
                {
                    cam_pos2 = cam_pos;
                    cameraFrom.setOrigin(util::convert(cam_pos2));
                    cam_pos2[0] += glm::sin(glm::radians(ent->m_angles[0] + 90.0f)) * Engine::instance.m_controlState.m_camDistance;
                    cam_pos2[1] -= glm::cos(glm::radians(ent->m_angles[0] + 90.0f)) * Engine::instance.m_controlState.m_camDistance;
                    cameraTo.setOrigin(util::convert(cam_pos2));

                    //If collided we want to go to back else right
                    if(Cam_HasHit(cb, cameraFrom, cameraTo))
                        cam->setTargetDir(world::CameraTarget::Back);
                    else
                        cam->setTargetDir(world::CameraTarget::Right);
                }
                else
                {
                    cam->setTargetDir(world::CameraTarget::Left);
                }
            }
        }
        else if(ent->m_skeleton.getPreviousState() == world::LaraState::JumpBack)
        {
            cam->setTargetDir(world::CameraTarget::Front);
        }
        else if(cam->getTargetDir() != world::CameraTarget::Back)
        {
            cam->setTargetDir(world::CameraTarget::Back);
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

            engine::Engine::instance.m_camera.shake(currentAngle, targetAngle, Engine::instance.m_frameTime);
        }
    }

    cam_pos = ent->camPosForFollowing(dz);

    //Code to manage screen shaking effects
    if(render::renderer.camera()->getShakeTime().count() > 0 && render::renderer.camera()->getShakeValue() > 0.0)
    {
        cam_pos += glm::ballRand(render::renderer.camera()->getShakeValue() / 2.0f) * util::toSeconds(render::renderer.camera()->getShakeTime());
        render::renderer.camera()->setShakeTime(std::max(util::Duration(0), render::renderer.camera()->getShakeTime() - Engine::instance.m_frameTime));
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
            glm::float_t cos_ay = glm::cos(engine::Engine::instance.m_camera.getAngles()[1]);
            glm::float_t cam_dx = glm::sin(engine::Engine::instance.m_camera.getAngles()[0]) * cos_ay;
            glm::float_t cam_dy = -glm::cos(engine::Engine::instance.m_camera.getAngles()[0]) * cos_ay;
            glm::float_t cam_dz = -glm::sin(engine::Engine::instance.m_camera.getAngles()[1]);
            cam_pos[0] += cam_dx * Engine::instance.m_controlState.m_camDistance;
            cam_pos[1] += cam_dy * Engine::instance.m_controlState.m_camDistance;
            cam_pos[2] += cam_dz * Engine::instance.m_controlState.m_camDistance;
        }

        cameraTo.setOrigin(util::convert(cam_pos));
        if(Cam_HasHit(cb, cameraFrom, cameraTo))
        {
            cam_pos = glm::mix(util::convert(cameraFrom.getOrigin()), util::convert(cameraTo.getOrigin()), cb->m_closestHitFraction);
            cam_pos += util::convert(cb->m_hitNormalWorld * 2.0);
        }
    }

    //Update cam pos
    cam->setPosition(cam_pos);

    //Modify cam pos for quicksand rooms
    cam->setCurrentRoom(Room_FindPosCogerrence(cam->getPosition() - glm::vec3(0, 0, 128), cam->getCurrentRoom()));
    if(cam->getCurrentRoom() && (cam->getCurrentRoom()->m_flags & TR_ROOM_FLAG_QUICKSAND))
    {
        glm::vec3 position = cam->getPosition();
        position[2] = cam->getCurrentRoom()->m_boundingBox.max[2] + 2.0f * 64.0f;
        cam->setPosition(position);
    }

    cam->applyRotation();
    cam->setCurrentRoom(Room_FindPosCogerrence(cam->getPosition(), cam->getCurrentRoom()));
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
        Engine::instance.m_frameTime = time;   // FIXME
    }
    game_logic_time += time;

    // GUI and controls should be updated at all times!
    ControlSettings::instance.pollSDLInput();

    // TODO: implement pause mechanism
    if(gui::Gui::instance->update())
    {
        if(game_logic_time >= world::animation::GameLogicFrameTime)
        {
            engine::Engine::instance.m_world.audioEngine.updateAudio();
            Game_Tick(&game_logic_time);
        }
        return;
    }

    // Translate input to character commands, move camera:
    // TODO: decouple cam movement
    Game_ApplyControls(Engine::instance.m_world.character);

    BulletEngine::instance->dynamicsWorld->stepSimulation(util::toSeconds(time), MAX_SIM_SUBSTEPS, util::toSeconds(world::animation::GameLogicFrameTime));

    if(Engine::instance.m_world.character)
    {
        Engine::instance.m_world.character->updateInterpolation();

        if(!Engine::instance.m_controlState.m_noClip && !Engine::instance.m_controlState.m_freeLook)
            Cam_FollowEntity(render::renderer.camera(), Engine::instance.m_world.character, 16.0, 128.0);
    }
    for(const std::shared_ptr<world::Entity>& entity : Engine::instance.m_world.entity_tree | boost::adaptors::map_values)
    {
        entity->updateInterpolation();
    }

    ControlSettings::instance.refreshStates();
    Engine::instance.m_world.updateAnimTextures();
}

void Game_Prepare()
{
    if(Engine::instance.m_world.character)
    {
        // Set character values to default.

        Engine::instance.m_world.character->setParamMaximum(world::PARAM_HEALTH, LARA_PARAM_HEALTH_MAX);
        Engine::instance.m_world.character->setParam(world::PARAM_HEALTH, LARA_PARAM_HEALTH_MAX);
        Engine::instance.m_world.character->setParamMaximum(world::PARAM_AIR, LARA_PARAM_AIR_MAX);
        Engine::instance.m_world.character->setParam(world::PARAM_AIR, LARA_PARAM_AIR_MAX);
        Engine::instance.m_world.character->setParamMaximum(world::PARAM_STAMINA, LARA_PARAM_STAMINA_MAX);
        Engine::instance.m_world.character->setParam(world::PARAM_STAMINA, LARA_PARAM_STAMINA_MAX);
        Engine::instance.m_world.character->setParamMaximum(world::PARAM_WARMTH, LARA_PARAM_WARMTH_MAX);
        Engine::instance.m_world.character->setParam(world::PARAM_WARMTH, LARA_PARAM_WARMTH_MAX);
        Engine::instance.m_world.character->setParamMaximum(world::PARAM_POISON, LARA_PARAM_POISON_MAX);
        Engine::instance.m_world.character->setParam(world::PARAM_POISON, 0);

        // Set character statistics to default.

        Engine::instance.m_world.character->m_statistics.distance = 0.0;
        Engine::instance.m_world.character->m_statistics.ammo_used = 0;
        Engine::instance.m_world.character->m_statistics.hits = 0;
        Engine::instance.m_world.character->m_statistics.kills = 0;
        Engine::instance.m_world.character->m_statistics.medipacks_used = 0;
        Engine::instance.m_world.character->m_statistics.saves_used = 0;
        Engine::instance.m_world.character->m_statistics.secrets_game = 0;
        Engine::instance.m_world.character->m_statistics.secrets_level = 0;
    }
    else if(!Engine::instance.m_world.rooms.empty())
    {
        // If there is no character present, move default camera position to
        // the first room (useful for TR1-3 cutscene levels).

        Engine::instance.m_camera.setPosition(Engine::instance.m_world.rooms[0]->m_boundingBox.max);
    }

    // Set gameflow parameters to default.
    // Reset secret trigger map.

    Gameflow::instance.resetSecretStatus();
}

void Game_LevelTransition(uint16_t level_index)
{
    gui::Gui::instance->faders.assignPicture(gui::FaderType::LoadScreen, engine_lua.getLoadingScreen(level_index));
    gui::Gui::instance->faders.start(gui::FaderType::LoadScreen, gui::FaderDir::Out);

    engine::Engine::instance.m_world.audioEngine.endStreams();
}
} // namespace engine