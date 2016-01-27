#include "game.h"

#include <cstdio>

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

#include "LuaState.h"

#include "character_controller.h"
#include "engine.h"
#include "engine/bullet.h"
#include "gameflow.h"
#include "gui/console.h"
#include "gui/fader.h"
#include "gui/fadermanager.h"
#include "gui/gui.h"
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

constexpr float CameraCollisionSphereRadius = 16.0f;

namespace engine
{
void Save_EntityTree(std::ostream& f, const std::map<uint32_t, std::shared_ptr<world::Entity> > &map);
void Save_Entity(std::ostream& f, std::shared_ptr<world::Entity> ent);

using gui::Console;

void lua_mlook(world::World& world, lua::Value mlook)
{
    if(!mlook.is<lua::Boolean>())
    {
        world.m_engine->m_controlState.m_mouseLook = !world.m_engine->m_controlState.m_mouseLook;
        world.m_engine->m_gui.getConsole().printf("mlook = %d", world.m_engine->m_controlState.m_mouseLook);
        return;
    }

    world.m_engine->m_controlState.m_mouseLook = mlook.toBool();
    world.m_engine->m_gui.getConsole().printf("mlook = %d", world.m_engine->m_controlState.m_mouseLook);
}

void lua_freelook(world::World& world, lua::Value free)
{
    if(!free.is<lua::Boolean>())
    {
        world.m_engine->m_controlState.m_freeLook = !world.m_engine->m_controlState.m_freeLook;
        world.m_engine->m_gui.getConsole().printf("free_look = %d", world.m_engine->m_controlState.m_freeLook);
        return;
    }

    world.m_engine->m_controlState.m_freeLook = free.toBool();
    world.m_engine->m_gui.getConsole().printf("free_look = %d", world.m_engine->m_controlState.m_freeLook);
}

void lua_cam_distance(world::World& world, lua::Value distance)
{
    if(!distance.is<lua::Number>())
    {
        world.m_engine->m_gui.getConsole().printf("cam_distance = %.2f", world.m_engine->m_controlState.m_camDistance);
        return;
    }

    world.m_engine->m_controlState.m_camDistance = distance.toFloat();
    world.m_engine->m_gui.getConsole().printf("cam_distance = %.2f", world.m_engine->m_controlState.m_camDistance);
}

void lua_noclip(world::World& world, lua::Value noclip)
{
    if(!noclip.is<lua::Boolean>())
    {
        world.m_engine->m_controlState.m_noClip = !world.m_engine->m_controlState.m_noClip;
    }
    else
    {
        world.m_engine->m_controlState.m_noClip = noclip.toBool();
    }

    world.m_engine->m_gui.getConsole().printf("noclip = %d", world.m_engine->m_controlState.m_noClip);
}

void lua_debuginfo(world::World& world, lua::Value show)
{
    if(!show.is<lua::Boolean>())
    {
        world.m_engine->screen_info.show_debuginfo = !world.m_engine->screen_info.show_debuginfo;
    }
    else
    {
        world.m_engine->screen_info.show_debuginfo = show.toBool();
    }

    world.m_engine->m_gui.getConsole().printf("debug info = %d", world.m_engine->screen_info.show_debuginfo);
}

void lua_timescale(world::World& world, lua::Value scale)
{
    if(!scale.is<lua::Number>())
    {
        if(util::fuzzyOne(world.m_engine->time_scale))
        {
            world.m_engine->time_scale = 0.033f;
        }
        else
        {
            world.m_engine->time_scale = 1.0;
        }
    }
    else
    {
        world.m_engine->time_scale = scale.toFloat();
    }

    world.m_engine->m_gui.getConsole().printf("time_scale = %.3f", world.m_engine->time_scale);
}

void Game_InitGlobals(world::World& world)
{
    world.m_engine->m_controlState.m_freeLookSpeed = 3000.0;
    world.m_engine->m_controlState.m_mouseLook = true;
    world.m_engine->m_controlState.m_freeLook = false;
    world.m_engine->m_controlState.m_noClip = false;
    world.m_engine->m_controlState.m_camDistance = 800.0;
}

void Game_RegisterLuaFunctions(script::ScriptEngine& state)
{
    state.registerFunction("debuginfo", lua_debuginfo);
    state.registerFunction("mlook", lua_mlook);
    state.registerFunction("freelook", lua_freelook);
    state.registerFunction("noclip", lua_noclip);
    state.registerFunction("cam_distance", lua_cam_distance);
    state.registerFunction("timescale", lua_timescale);
}

/**
 * Load game state
 */
bool Game_Load(world::World& world, const std::string& name)
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
        world.m_engine->engine_lua.clearTasks();
        try
        {
            world.m_engine->engine_lua.doFile(token);
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
        world.m_engine->engine_lua.clearTasks();
        try
        {
            world.m_engine->engine_lua.doFile(name);
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
            % ent->m_skeleton.getModel()->getId()
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
        ch->saveGame(f);
    }
}

/**
 * Save current game state
 */
bool Game_Save(world::World& world, const std::string& name)
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
        % world.m_engine->gameflow.getLevelPath()
        % world.m_engine->gameflow.getGameID()
        % world.m_engine->gameflow.getLevelID();

    // Save flipmap and flipped room states.

    for(size_t i = 0; i < world.m_engine->m_world.m_flipData.size(); i++)
    {
        f << boost::format("setFlipMap(%d, 0x%02X, 0);\n")
            % i
            % world.m_engine->m_world.m_flipData[i].map;
        f << boost::format("setFlipState(%d, %s);\n")
            % i
            % (world.m_engine->m_world.m_flipData[i].state ? "true" : "false");
    }

    Save_Entity(f, world.m_engine->m_world.m_character);    // Save Lara.

    Save_EntityTree(f, world.m_engine->m_world.m_entities);

    return true;
}

void Game_ApplyControls(world::World& world)
{
    // Keyboard move logic
    world::Movement moveLogic;
    moveLogic.setX(world.m_engine->m_controlState.m_moveLeft, world.m_engine->m_controlState.m_moveRight);
    moveLogic.setY(world.m_engine->m_controlState.m_moveUp, world.m_engine->m_controlState.m_moveDown);
    moveLogic.setZ(world.m_engine->m_controlState.m_moveForward, world.m_engine->m_controlState.m_moveBackward);

    // Keyboard look logic
    world::Movement lookLogic;
    lookLogic.setX(world.m_engine->m_controlState.m_lookLeft, world.m_engine->m_controlState.m_lookRight);
    lookLogic.setY(world.m_engine->m_controlState.m_lookUp, world.m_engine->m_controlState.m_lookDown);
    lookLogic.setZ(world.m_engine->m_controlState.m_lookRollLeft, world.m_engine->m_controlState.m_lookRollRight);

    // APPLY CONTROLS

    world.m_engine->m_camera.rotate(lookLogic.getDistance(glm::radians(2.2f) * world.m_engine->getFrameTimeSecs()));

    // FIXME: Duplicate code - do we need cam control with no world??
    if(!world.m_engine->renderer.world())
    {
        if(world.m_engine->m_controlState.m_mouseLook)
        {
            world.m_engine->m_camera.rotate({ -world::CameraRotationSpeed * world.m_engine->m_controlState.m_lookAxisX,
                                                       -world::CameraRotationSpeed * world.m_engine->m_controlState.m_lookAxisY,
                                                       0 });
            world.m_engine->m_controlState.m_lookAxisX = 0.0;
            world.m_engine->m_controlState.m_lookAxisY = 0.0;
        }

        world.m_engine->renderer.camera()->applyRotation();
        glm::float_t dist = world.m_engine->m_controlState.m_stateWalk
            ? world.m_engine->m_controlState.m_freeLookSpeed * world.m_engine->getFrameTimeSecs() * 0.3f
            : world.m_engine->m_controlState.m_freeLookSpeed * world.m_engine->getFrameTimeSecs();
        world.m_engine->renderer.camera()->move(moveLogic.getDistance(dist));

        return;
    }

    if(world.m_engine->m_controlState.m_mouseLook)
    {
        world.m_engine->m_camera.rotate({ -world::CameraRotationSpeed * world.m_engine->m_controlState.m_lookAxisX,
                                                   -world::CameraRotationSpeed * world.m_engine->m_controlState.m_lookAxisY,
                                                   0 });
        world.m_engine->m_controlState.m_lookAxisX = 0.0;
        world.m_engine->m_controlState.m_lookAxisY = 0.0;
    }

    if(world.m_engine->m_controlState.m_freeLook || !world.m_character)
    {
        glm::float_t dist = world.m_engine->m_controlState.m_stateWalk
            ? world.m_engine->m_controlState.m_freeLookSpeed * world.m_engine->getFrameTimeSecs() * 0.3f
            : world.m_engine->m_controlState.m_freeLookSpeed * world.m_engine->getFrameTimeSecs();
        world.m_engine->renderer.camera()->applyRotation();
        world.m_engine->renderer.camera()->move(moveLogic.getDistance(dist));
        world.m_engine->renderer.camera()->setCurrentRoom(world.m_engine->m_world.Room_FindPosCogerrence(world.m_engine->renderer.camera()->getPosition(), world.m_engine->renderer.camera()->getCurrentRoom()));
    }
    else if(world.m_engine->m_controlState.m_noClip)
    {
        glm::float_t dist = world.m_engine->m_controlState.m_stateWalk
            ? world.m_engine->m_controlState.m_freeLookSpeed * world.m_engine->getFrameTimeSecs() * 0.3f
            : world.m_engine->m_controlState.m_freeLookSpeed * world.m_engine->getFrameTimeSecs();
        world.m_engine->renderer.camera()->applyRotation();
        world.m_engine->renderer.camera()->move(moveLogic.getDistance(dist));
        world.m_engine->renderer.camera()->setCurrentRoom(world.m_engine->m_world.Room_FindPosCogerrence(world.m_engine->renderer.camera()->getPosition(), world.m_engine->renderer.camera()->getCurrentRoom()));

        world.m_character->m_angles[0] = glm::degrees(world.m_engine->m_camera.getAngles()[0]);
        glm::vec3 position = world.m_engine->renderer.camera()->getPosition() + world.m_engine->renderer.camera()->getViewDir() * world.m_engine->m_controlState.m_camDistance;
        position[2] -= 512.0;
        world.m_character->m_transform[3] = glm::vec4(position, 1.0f);
        world.m_character->updateTransform();
    }
    else
    {
        world.m_character->applyControls(world.m_engine->m_controlState, moveLogic);
    }
}

bool Cam_HasHit(world::World& world, std::shared_ptr<BtEngineClosestConvexResultCallback> cb, btTransform &cameraFrom, btTransform &cameraTo)
{
    btSphereShape cameraSphere(CameraCollisionSphereRadius);
    cameraSphere.setMargin(COLLISION_MARGIN_DEFAULT);
    cb->m_closestHitFraction = 1.0;
    cb->m_hitCollisionObject = nullptr;
    world.m_engine->bullet.dynamicsWorld->convexSweepTest(&cameraSphere, cameraFrom, cameraTo, *cb);
    return cb->hasHit();
}

void Cam_FollowEntity(world::World& world, world::Camera *cam, glm::float_t dx, glm::float_t dz)
{
    btTransform cameraFrom = btTransform::getIdentity();
    btTransform cameraTo = btTransform::getIdentity();

    std::shared_ptr<BtEngineClosestConvexResultCallback> cb = world.m_character->callbackForCamera();

    glm::vec3 cam_pos = cam->getPosition();

    ///@INFO Basic camera override, completely placeholder until a system classic-like is created
    if(!world.m_engine->m_controlState.m_mouseLook)//If mouse look is off
    {
        glm::float_t currentAngle = world.m_engine->m_camera.getAngles()[0];  //Current is the current cam angle
        glm::float_t targetAngle = glm::radians(world.m_character->m_angles[0]); //Target is the target angle which is the entity's angle itself

        ///@FIXME
        //If Lara is in a specific state we want to rotate -75 deg or +75 deg depending on camera collision
        if(world.m_character->m_skeleton.getPreviousState() == world::LaraState::Reach)
        {
            if(cam->getTargetDir() == world::CameraTarget::Back)
            {
                glm::vec3 cam_pos2 = cam_pos;
                cameraFrom.setOrigin(util::convert(cam_pos2));
                cam_pos2[0] += glm::sin(glm::radians(world.m_character->m_angles[0] - 90.0f)) * world.m_engine->m_controlState.m_camDistance;
                cam_pos2[1] -= glm::cos(glm::radians(world.m_character->m_angles[0] - 90.0f)) * world.m_engine->m_controlState.m_camDistance;
                cameraTo.setOrigin(util::convert(cam_pos2));

                //If collided we want to go right otherwise stay left
                if(Cam_HasHit(world, cb, cameraFrom, cameraTo))
                {
                    cam_pos2 = cam_pos;
                    cameraFrom.setOrigin(util::convert(cam_pos2));
                    cam_pos2[0] += glm::sin(glm::radians(world.m_character->m_angles[0] + 90.0f)) * world.m_engine->m_controlState.m_camDistance;
                    cam_pos2[1] -= glm::cos(glm::radians(world.m_character->m_angles[0] + 90.0f)) * world.m_engine->m_controlState.m_camDistance;
                    cameraTo.setOrigin(util::convert(cam_pos2));

                    //If collided we want to go to back else right
                    if(Cam_HasHit(world, cb, cameraFrom, cameraTo))
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
        else if(world.m_character->m_skeleton.getPreviousState() == world::LaraState::JumpBack)
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
                    targetAngle = glm::radians(world.m_character->m_angles[0]);
                    break;
                case world::CameraTarget::Front:
                    targetAngle = glm::radians(world.m_character->m_angles[0] - 180.0f);
                    break;
                case world::CameraTarget::Left:
                    targetAngle = glm::radians(world.m_character->m_angles[0] - 75.0f);
                    break;
                case world::CameraTarget::Right:
                    targetAngle = glm::radians(world.m_character->m_angles[0] + 75.0f);
                    break;
                default:
                    targetAngle = glm::radians(world.m_character->m_angles[0]);//Same as TR_CAM_TARG_BACK (default pos)
                    break;
            }

            world.m_engine->m_camera.shake(currentAngle, targetAngle, world.m_engine->getFrameTime());
        }
    }

    cam_pos = world.m_character->camPosForFollowing(dz);

    //Code to manage screen shaking effects
    if(world.m_engine->renderer.camera()->getShakeTime().count() > 0 && world.m_engine->renderer.camera()->getShakeValue() > 0.0)
    {
        cam_pos += glm::ballRand(world.m_engine->renderer.camera()->getShakeValue() / 2.0f) * util::toSeconds(world.m_engine->renderer.camera()->getShakeTime());
        world.m_engine->renderer.camera()->setShakeTime(std::max(util::Duration(0), world.m_engine->renderer.camera()->getShakeTime() - world.m_engine->getFrameTime()));
    }

    cameraFrom.setOrigin(util::convert(cam_pos));
    cam_pos[2] += dz;
    cameraTo.setOrigin(util::convert(cam_pos));
    if(Cam_HasHit(world, cb, cameraFrom, cameraTo))
    {
        cam_pos = glm::mix(util::convert(cameraFrom.getOrigin()), util::convert(cameraTo.getOrigin()), cb->m_closestHitFraction);
        cam_pos += util::convert(cb->m_hitNormalWorld * 2.0);
    }

    if(dx != 0.0)
    {
        cameraFrom.setOrigin(util::convert(cam_pos));
        cam_pos += dx * cam->getRightDir();
        cameraTo.setOrigin(util::convert(cam_pos));
        if(Cam_HasHit(world, cb, cameraFrom, cameraTo))
        {
            cam_pos = glm::mix(util::convert(cameraFrom.getOrigin()), util::convert(cameraTo.getOrigin()), cb->m_closestHitFraction);
            cam_pos += util::convert(cb->m_hitNormalWorld * 2.0);
        }

        cameraFrom.setOrigin(util::convert(cam_pos));

        {
            glm::float_t cos_ay = glm::cos(world.m_engine->m_camera.getAngles()[1]);
            glm::float_t cam_dx = glm::sin(world.m_engine->m_camera.getAngles()[0]) * cos_ay;
            glm::float_t cam_dy = -glm::cos(world.m_engine->m_camera.getAngles()[0]) * cos_ay;
            glm::float_t cam_dz = -glm::sin(world.m_engine->m_camera.getAngles()[1]);
            cam_pos[0] += cam_dx * world.m_engine->m_controlState.m_camDistance;
            cam_pos[1] += cam_dy * world.m_engine->m_controlState.m_camDistance;
            cam_pos[2] += cam_dz * world.m_engine->m_controlState.m_camDistance;
        }

        cameraTo.setOrigin(util::convert(cam_pos));
        if(Cam_HasHit(world, cb, cameraFrom, cameraTo))
        {
            cam_pos = glm::mix(util::convert(cameraFrom.getOrigin()), util::convert(cameraTo.getOrigin()), cb->m_closestHitFraction);
            cam_pos += util::convert(cb->m_hitNormalWorld * 2.0);
        }
    }

    //Update cam pos
    cam->setPosition(cam_pos);

    //Modify cam pos for quicksand rooms
    cam->setCurrentRoom(world.m_engine->m_world.Room_FindPosCogerrence(cam->getPosition() - glm::vec3(0, 0, 128), cam->getCurrentRoom()));
    if(cam->getCurrentRoom() && (cam->getCurrentRoom()->getFlags() & TR_ROOM_FLAG_QUICKSAND))
    {
        glm::vec3 position = cam->getPosition();
        position[2] = cam->getCurrentRoom()->getBoundingBox().max[2] + 2.0f * 64.0f;
        cam->setPosition(position);
    }

    cam->applyRotation();
    cam->setCurrentRoom(world.m_engine->m_world.Room_FindPosCogerrence(cam->getPosition(), cam->getCurrentRoom()));
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

void Game_Frame(world::World& world, util::Duration time)
{
    static util::Duration game_logic_time = util::Duration(0);
    static const util::Duration SimulationTime = MAX_SIM_SUBSTEPS * world::animation::GameLogicFrameTime;

    // clamp frameskip at max substeps - if more frames are dropped, slow everything down:
    if(time > SimulationTime)
    {
        time = SimulationTime;
        world.m_engine->setFrameTime(time);   // FIXME
    }
    game_logic_time += time;

    // GUI and controls should be updated at all times!
    world.m_engine->m_inputHandler.poll();

    // TODO: implement pause mechanism
    if(world.m_engine->m_gui.update())
    {
        if(game_logic_time >= world::animation::GameLogicFrameTime)
        {
            world.m_engine->m_world.m_audioEngine.updateAudio();
            Game_Tick(&game_logic_time);
        }
        return;
    }

    // Translate input to character commands, move camera:
    // TODO: decouple cam movement
    Game_ApplyControls(world);

    world.m_engine->bullet.dynamicsWorld->stepSimulation(util::toSeconds(time), MAX_SIM_SUBSTEPS, util::toSeconds(world::animation::GameLogicFrameTime));

    if(world.m_engine->m_world.m_character)
    {
        world.m_engine->m_world.m_character->updateInterpolation();

        if(!world.m_engine->m_controlState.m_noClip && !world.m_engine->m_controlState.m_freeLook)
            Cam_FollowEntity(world, world.m_engine->renderer.camera(), 16.0, 128.0);
    }
    for(const std::shared_ptr<world::Entity>& entity : world.m_engine->m_world.m_entities | boost::adaptors::map_values)
    {
        entity->updateInterpolation();
    }

    world.m_engine->m_inputHandler.refreshStates();
    world.m_engine->m_world.updateAnimTextures();
}

void Game_Prepare(world::World& world)
{
    if(world.m_engine->m_world.m_character)
    {
        // Set character values to default.

        world.m_engine->m_world.m_character->setParamMaximum(world::CharParameterId::PARAM_HEALTH, LARA_PARAM_HEALTH_MAX);
        world.m_engine->m_world.m_character->setParam(world::CharParameterId::PARAM_HEALTH, LARA_PARAM_HEALTH_MAX);
        world.m_engine->m_world.m_character->setParamMaximum(world::CharParameterId::PARAM_AIR, LARA_PARAM_AIR_MAX);
        world.m_engine->m_world.m_character->setParam(world::CharParameterId::PARAM_AIR, LARA_PARAM_AIR_MAX);
        world.m_engine->m_world.m_character->setParamMaximum(world::CharParameterId::PARAM_STAMINA, LARA_PARAM_STAMINA_MAX);
        world.m_engine->m_world.m_character->setParam(world::CharParameterId::PARAM_STAMINA, LARA_PARAM_STAMINA_MAX);
        world.m_engine->m_world.m_character->setParamMaximum(world::CharParameterId::PARAM_WARMTH, LARA_PARAM_WARMTH_MAX);
        world.m_engine->m_world.m_character->setParam(world::CharParameterId::PARAM_WARMTH, LARA_PARAM_WARMTH_MAX);
        world.m_engine->m_world.m_character->setParamMaximum(world::CharParameterId::PARAM_POISON, LARA_PARAM_POISON_MAX);
        world.m_engine->m_world.m_character->setParam(world::CharParameterId::PARAM_POISON, 0);

        // Set character statistics to default.

        world.m_engine->m_world.m_character->resetStatistics();
    }
    else if(!world.m_engine->m_world.m_rooms.empty())
    {
        // If there is no character present, move default camera position to
        // the first room (useful for TR1-3 cutscene levels).

        world.m_engine->m_camera.setPosition(world.m_engine->m_world.m_rooms[0]->getBoundingBox().max);
    }

    // Set gameflow parameters to default.
    // Reset secret trigger map.

    world.m_engine->gameflow.resetSecretStatus();
}

void Game_LevelTransition(world::World& world, const boost::optional<int>& level)
{
    if(level)
        world.m_engine->m_gui.m_faders.assignPicture(gui::FaderType::LoadScreen, world.m_engine->engine_lua.getLoadingScreen(*level));
    else
        world.m_engine->m_gui.m_faders.assignPicture(gui::FaderType::LoadScreen, world.m_engine->engine_lua.getLoadingScreen(world.m_engine->gameflow.getLevelID()));
    world.m_engine->m_gui.m_faders.start(gui::FaderType::LoadScreen, gui::FaderDir::Out);

    world.m_engine->m_world.m_audioEngine.endStreams();
}
} // namespace engine
