#include "game.h"

#include <cstdio>

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

#include "LuaState.h"

#include "character_controller.h"
#include "engine.h"
#include "engine/bullet.h"
#include "gui/console.h"
#include "gui/fader.h"
#include "gui/gui.h"
#include "render/render.h"
#include "script/script.h"
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

void lua_mlook(Engine& engine, lua::Value mlook)
{
    if(!mlook.is<lua::Boolean>())
    {
        engine.m_controlState.m_mouseLook = !engine.m_controlState.m_mouseLook;
        engine.m_gui.getConsole().printf("mlook = %d", engine.m_controlState.m_mouseLook);
        return;
    }

    engine.m_controlState.m_mouseLook = mlook.toBool();
    engine.m_gui.getConsole().printf("mlook = %d", engine.m_controlState.m_mouseLook);
}

void lua_freelook(Engine& engine, lua::Value free)
{
    if(!free.is<lua::Boolean>())
    {
        engine.m_controlState.m_freeLook = !engine.m_controlState.m_freeLook;
        engine.m_gui.getConsole().printf("free_look = %d", engine.m_controlState.m_freeLook);
        return;
    }

    engine.m_controlState.m_freeLook = free.toBool();
    engine.m_gui.getConsole().printf("free_look = %d", engine.m_controlState.m_freeLook);
}

void lua_cam_distance(Engine& engine, lua::Value distance)
{
    if(!distance.is<lua::Number>())
    {
        engine.m_gui.getConsole().printf("cam_distance = %.2f", engine.m_controlState.m_camDistance);
        return;
    }

    engine.m_controlState.m_camDistance = distance.toFloat();
    engine.m_gui.getConsole().printf("cam_distance = %.2f", engine.m_controlState.m_camDistance);
}

void lua_noclip(Engine& engine, lua::Value noclip)
{
    if(!noclip.is<lua::Boolean>())
    {
        engine.m_controlState.m_noClip = !engine.m_controlState.m_noClip;
    }
    else
    {
        engine.m_controlState.m_noClip = noclip.toBool();
    }

    engine.m_gui.getConsole().printf("noclip = %d", engine.m_controlState.m_noClip);
}

void lua_debuginfo(Engine& engine, lua::Value show)
{
    if(!show.is<lua::Boolean>())
    {
        engine.screen_info.show_debuginfo = !engine.screen_info.show_debuginfo;
    }
    else
    {
        engine.screen_info.show_debuginfo = show.toBool();
    }

    engine.m_gui.getConsole().printf("debug info = %d", engine.screen_info.show_debuginfo);
}

void lua_timescale(Engine& engine, lua::Value scale)
{
    if(!scale.is<lua::Number>())
    {
        if(util::fuzzyOne(engine.time_scale))
        {
            engine.time_scale = 0.033f;
        }
        else
        {
            engine.time_scale = 1.0;
        }
    }
    else
    {
        engine.time_scale = scale.toFloat();
    }

    engine.m_gui.getConsole().printf("time_scale = %.3f", engine.time_scale);
}

void Game_InitGlobals(Engine& engine)
{
    engine.m_controlState.m_freeLookSpeed = 3000.0;
    engine.m_controlState.m_mouseLook = true;
    engine.m_controlState.m_freeLook = false;
    engine.m_controlState.m_noClip = false;
    engine.m_controlState.m_camDistance = 800.0;
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
bool Game_Load(Engine& engine, const std::string& name)
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
        engine.engine_lua.clearTasks();
        try
        {
            engine.engine_lua.doFile(token);
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
        engine.engine_lua.clearTasks();
        try
        {
            engine.engine_lua.doFile(name);
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
bool Game_Save(Engine& engine, const std::string& name)
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
        % engine.gameflow.getLevelPath()
        % engine.gameflow.getGameID()
        % engine.gameflow.getLevelID();

    // Save flipmap and flipped room states.

    for(size_t i = 0; i < engine.m_world.m_flipData.size(); i++)
    {
        f << boost::format("setFlipMap(%d, 0x%02X, 0);\n")
            % i
            % engine.m_world.m_flipData[i].map;
        f << boost::format("setFlipState(%d, %s);\n")
            % i
            % (engine.m_world.m_flipData[i].state ? "true" : "false");
    }

    Save_Entity(f, engine.m_world.m_character);    // Save Lara.

    Save_EntityTree(f, engine.m_world.m_entities);

    return true;
}

void Game_ApplyControls(Engine& engine)
{
    // Keyboard move logic
    world::Movement moveLogic;
    moveLogic.setX(engine.m_controlState.m_moveLeft, engine.m_controlState.m_moveRight);
    moveLogic.setY(engine.m_controlState.m_moveUp, engine.m_controlState.m_moveDown);
    moveLogic.setZ(engine.m_controlState.m_moveForward, engine.m_controlState.m_moveBackward);

    // Keyboard look logic
    world::Movement lookLogic;
    lookLogic.setX(engine.m_controlState.m_lookLeft, engine.m_controlState.m_lookRight);
    lookLogic.setY(engine.m_controlState.m_lookUp, engine.m_controlState.m_lookDown);
    lookLogic.setZ(engine.m_controlState.m_lookRollLeft, engine.m_controlState.m_lookRollRight);

    // APPLY CONTROLS

    engine.m_camera.rotate(lookLogic.getDistance(glm::radians(2.2f) * engine.getFrameTimeSecs()));

    // FIXME: Duplicate code - do we need cam control with no world??
    if(!engine.renderer.world())
    {
        if(engine.m_controlState.m_mouseLook)
        {
            engine.m_camera.rotate({ -world::CameraRotationSpeed * engine.m_controlState.m_lookAxisX,
                                                       -world::CameraRotationSpeed * engine.m_controlState.m_lookAxisY,
                                                       0 });
            engine.m_controlState.m_lookAxisX = 0.0;
            engine.m_controlState.m_lookAxisY = 0.0;
        }

        engine.renderer.camera()->applyRotation();
        glm::float_t dist = engine.m_controlState.m_stateWalk
            ? engine.m_controlState.m_freeLookSpeed * engine.getFrameTimeSecs() * 0.3f
            : engine.m_controlState.m_freeLookSpeed * engine.getFrameTimeSecs();
        engine.renderer.camera()->move(moveLogic.getDistance(dist));

        return;
    }

    if(engine.m_controlState.m_mouseLook)
    {
        engine.m_camera.rotate({ -world::CameraRotationSpeed * engine.m_controlState.m_lookAxisX,
                                                   -world::CameraRotationSpeed * engine.m_controlState.m_lookAxisY,
                                                   0 });
        engine.m_controlState.m_lookAxisX = 0.0;
        engine.m_controlState.m_lookAxisY = 0.0;
    }

    if(engine.m_controlState.m_freeLook || !engine.m_world.m_character)
    {
        glm::float_t dist = engine.m_controlState.m_stateWalk
            ? engine.m_controlState.m_freeLookSpeed * engine.getFrameTimeSecs() * 0.3f
            : engine.m_controlState.m_freeLookSpeed * engine.getFrameTimeSecs();
        engine.renderer.camera()->applyRotation();
        engine.renderer.camera()->move(moveLogic.getDistance(dist));
        engine.renderer.camera()->setCurrentRoom(engine.m_world.Room_FindPosCogerrence(engine.renderer.camera()->getPosition(), engine.renderer.camera()->getCurrentRoom()));
    }
    else if(engine.m_controlState.m_noClip)
    {
        glm::float_t dist = engine.m_controlState.m_stateWalk
            ? engine.m_controlState.m_freeLookSpeed * engine.getFrameTimeSecs() * 0.3f
            : engine.m_controlState.m_freeLookSpeed * engine.getFrameTimeSecs();
        engine.renderer.camera()->applyRotation();
        engine.renderer.camera()->move(moveLogic.getDistance(dist));
        engine.renderer.camera()->setCurrentRoom(engine.m_world.Room_FindPosCogerrence(engine.renderer.camera()->getPosition(), engine.renderer.camera()->getCurrentRoom()));

        engine.m_world.m_character->m_angles[0] = glm::degrees(engine.m_camera.getAngles()[0]);
        glm::vec3 position = engine.renderer.camera()->getPosition() + engine.renderer.camera()->getViewDir() * engine.m_controlState.m_camDistance;
        position[2] -= 512.0;
        engine.m_world.m_character->m_transform[3] = glm::vec4(position, 1.0f);
        engine.m_world.m_character->updateTransform();
    }
    else
    {
        engine.m_world.m_character->applyControls(engine.m_controlState, moveLogic);
    }
}

bool Cam_HasHit(Engine& engine, std::shared_ptr<BtEngineClosestConvexResultCallback> cb, btTransform &cameraFrom, btTransform &cameraTo)
{
    btSphereShape cameraSphere(CameraCollisionSphereRadius);
    cameraSphere.setMargin(COLLISION_MARGIN_DEFAULT);
    cb->m_closestHitFraction = 1.0;
    cb->m_hitCollisionObject = nullptr;
    engine.bullet.dynamicsWorld->convexSweepTest(&cameraSphere, cameraFrom, cameraTo, *cb);
    return cb->hasHit();
}

void Cam_FollowEntity(Engine& engine, world::Camera *cam, glm::float_t dx, glm::float_t dz)
{
    btTransform cameraFrom = btTransform::getIdentity();
    btTransform cameraTo = btTransform::getIdentity();

    std::shared_ptr<BtEngineClosestConvexResultCallback> cb = engine.m_world.m_character->callbackForCamera();

    glm::vec3 cam_pos = cam->getPosition();

    ///@INFO Basic camera override, completely placeholder until a system classic-like is created
    if(!engine.m_controlState.m_mouseLook)//If mouse look is off
    {
        glm::float_t currentAngle = engine.m_camera.getAngles()[0];  //Current is the current cam angle
        glm::float_t targetAngle = glm::radians(engine.m_world.m_character->m_angles[0]); //Target is the target angle which is the entity's angle itself

        ///@FIXME
        //If Lara is in a specific state we want to rotate -75 deg or +75 deg depending on camera collision
        if(engine.m_world.m_character->m_skeleton.getPreviousState() == world::LaraState::Reach)
        {
            if(cam->getTargetDir() == world::CameraTarget::Back)
            {
                glm::vec3 cam_pos2 = cam_pos;
                cameraFrom.setOrigin(util::convert(cam_pos2));
                cam_pos2[0] += glm::sin(glm::radians(engine.m_world.m_character->m_angles[0] - 90.0f)) * engine.m_controlState.m_camDistance;
                cam_pos2[1] -= glm::cos(glm::radians(engine.m_world.m_character->m_angles[0] - 90.0f)) * engine.m_controlState.m_camDistance;
                cameraTo.setOrigin(util::convert(cam_pos2));

                //If collided we want to go right otherwise stay left
                if(Cam_HasHit(engine, cb, cameraFrom, cameraTo))
                {
                    cam_pos2 = cam_pos;
                    cameraFrom.setOrigin(util::convert(cam_pos2));
                    cam_pos2[0] += glm::sin(glm::radians(engine.m_world.m_character->m_angles[0] + 90.0f)) * engine.m_controlState.m_camDistance;
                    cam_pos2[1] -= glm::cos(glm::radians(engine.m_world.m_character->m_angles[0] + 90.0f)) * engine.m_controlState.m_camDistance;
                    cameraTo.setOrigin(util::convert(cam_pos2));

                    //If collided we want to go to back else right
                    if(Cam_HasHit(engine, cb, cameraFrom, cameraTo))
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
        else if(engine.m_world.m_character->m_skeleton.getPreviousState() == world::LaraState::JumpBack)
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
                    targetAngle = glm::radians(engine.m_world.m_character->m_angles[0]);
                    break;
                case world::CameraTarget::Front:
                    targetAngle = glm::radians(engine.m_world.m_character->m_angles[0] - 180.0f);
                    break;
                case world::CameraTarget::Left:
                    targetAngle = glm::radians(engine.m_world.m_character->m_angles[0] - 75.0f);
                    break;
                case world::CameraTarget::Right:
                    targetAngle = glm::radians(engine.m_world.m_character->m_angles[0] + 75.0f);
                    break;
                default:
                    targetAngle = glm::radians(engine.m_world.m_character->m_angles[0]);//Same as TR_CAM_TARG_BACK (default pos)
                    break;
            }

            engine.m_camera.shake(currentAngle, targetAngle, engine.getFrameTime());
        }
    }

    cam_pos = engine.m_world.m_character->camPosForFollowing(dz);

    //Code to manage screen shaking effects
    if(engine.renderer.camera()->getShakeTime().count() > 0 && engine.renderer.camera()->getShakeValue() > 0.0)
    {
        cam_pos += glm::ballRand(engine.renderer.camera()->getShakeValue() / 2.0f) * util::toSeconds(engine.renderer.camera()->getShakeTime());
        engine.renderer.camera()->setShakeTime(std::max(util::Duration(0), engine.renderer.camera()->getShakeTime() - engine.getFrameTime()));
    }

    cameraFrom.setOrigin(util::convert(cam_pos));
    cam_pos[2] += dz;
    cameraTo.setOrigin(util::convert(cam_pos));
    if(Cam_HasHit(engine, cb, cameraFrom, cameraTo))
    {
        cam_pos = glm::mix(util::convert(cameraFrom.getOrigin()), util::convert(cameraTo.getOrigin()), cb->m_closestHitFraction);
        cam_pos += util::convert(cb->m_hitNormalWorld * 2.0);
    }

    if(dx != 0.0)
    {
        cameraFrom.setOrigin(util::convert(cam_pos));
        cam_pos += dx * cam->getRightDir();
        cameraTo.setOrigin(util::convert(cam_pos));
        if(Cam_HasHit(engine, cb, cameraFrom, cameraTo))
        {
            cam_pos = glm::mix(util::convert(cameraFrom.getOrigin()), util::convert(cameraTo.getOrigin()), cb->m_closestHitFraction);
            cam_pos += util::convert(cb->m_hitNormalWorld * 2.0);
        }

        cameraFrom.setOrigin(util::convert(cam_pos));

        {
            glm::float_t cos_ay = glm::cos(engine.m_camera.getAngles()[1]);
            glm::float_t cam_dx = glm::sin(engine.m_camera.getAngles()[0]) * cos_ay;
            glm::float_t cam_dy = -glm::cos(engine.m_camera.getAngles()[0]) * cos_ay;
            glm::float_t cam_dz = -glm::sin(engine.m_camera.getAngles()[1]);
            cam_pos[0] += cam_dx * engine.m_controlState.m_camDistance;
            cam_pos[1] += cam_dy * engine.m_controlState.m_camDistance;
            cam_pos[2] += cam_dz * engine.m_controlState.m_camDistance;
        }

        cameraTo.setOrigin(util::convert(cam_pos));
        if(Cam_HasHit(engine, cb, cameraFrom, cameraTo))
        {
            cam_pos = glm::mix(util::convert(cameraFrom.getOrigin()), util::convert(cameraTo.getOrigin()), cb->m_closestHitFraction);
            cam_pos += util::convert(cb->m_hitNormalWorld * 2.0);
        }
    }

    //Update cam pos
    cam->setPosition(cam_pos);

    //Modify cam pos for quicksand rooms
    cam->setCurrentRoom(engine.m_world.Room_FindPosCogerrence(cam->getPosition() - glm::vec3(0, 0, 128), cam->getCurrentRoom()));
    if(cam->getCurrentRoom() && (cam->getCurrentRoom()->getFlags() & TR_ROOM_FLAG_QUICKSAND))
    {
        glm::vec3 position = cam->getPosition();
        position[2] = cam->getCurrentRoom()->getBoundingBox().max[2] + 2.0f * 64.0f;
        cam->setPosition(position);
    }

    cam->applyRotation();
    cam->setCurrentRoom(engine.m_world.Room_FindPosCogerrence(cam->getPosition(), cam->getCurrentRoom()));
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

void Game_Frame(Engine& engine, util::Duration time)
{
    static util::Duration game_logic_time = util::Duration(0);
    static const util::Duration SimulationTime = MAX_SIM_SUBSTEPS * world::animation::GameLogicFrameTime;

    // clamp frameskip at max substeps - if more frames are dropped, slow everything down:
    if(time > SimulationTime)
    {
        time = SimulationTime;
        engine.setFrameTime(time);   // FIXME
    }
    game_logic_time += time;

    // GUI and controls should be updated at all times!
    engine.m_inputHandler.poll();

    // TODO: implement pause mechanism
    if(engine.m_gui.update())
    {
        if(game_logic_time >= world::animation::GameLogicFrameTime)
        {
            engine.m_world.m_audioEngine.updateAudio();
            Game_Tick(&game_logic_time);
        }
        return;
    }

    // Translate input to character commands, move camera:
    // TODO: decouple cam movement
    Game_ApplyControls(engine);

    engine.bullet.dynamicsWorld->stepSimulation(util::toSeconds(time), MAX_SIM_SUBSTEPS, util::toSeconds(world::animation::GameLogicFrameTime));

    if(engine.m_world.m_character)
    {
        engine.m_world.m_character->updateInterpolation();

        if(!engine.m_controlState.m_noClip && !engine.m_controlState.m_freeLook)
            Cam_FollowEntity(engine, engine.renderer.camera(), 16.0, 128.0);
    }
    for(const std::shared_ptr<world::Entity>& entity : engine.m_world.m_entities | boost::adaptors::map_values)
    {
        entity->updateInterpolation();
    }

    engine.m_inputHandler.refreshStates();
    engine.m_world.updateAnimTextures();
}

void Game_Prepare(Engine& engine)
{
    if(engine.m_world.m_character)
    {
        // Set character values to default.

        engine.m_world.m_character->setParamMaximum(world::CharParameterId::PARAM_HEALTH, LARA_PARAM_HEALTH_MAX);
        engine.m_world.m_character->setParam(world::CharParameterId::PARAM_HEALTH, LARA_PARAM_HEALTH_MAX);
        engine.m_world.m_character->setParamMaximum(world::CharParameterId::PARAM_AIR, LARA_PARAM_AIR_MAX);
        engine.m_world.m_character->setParam(world::CharParameterId::PARAM_AIR, LARA_PARAM_AIR_MAX);
        engine.m_world.m_character->setParamMaximum(world::CharParameterId::PARAM_STAMINA, LARA_PARAM_STAMINA_MAX);
        engine.m_world.m_character->setParam(world::CharParameterId::PARAM_STAMINA, LARA_PARAM_STAMINA_MAX);
        engine.m_world.m_character->setParamMaximum(world::CharParameterId::PARAM_WARMTH, LARA_PARAM_WARMTH_MAX);
        engine.m_world.m_character->setParam(world::CharParameterId::PARAM_WARMTH, LARA_PARAM_WARMTH_MAX);
        engine.m_world.m_character->setParamMaximum(world::CharParameterId::PARAM_POISON, LARA_PARAM_POISON_MAX);
        engine.m_world.m_character->setParam(world::CharParameterId::PARAM_POISON, 0);

        // Set character statistics to default.

        engine.m_world.m_character->resetStatistics();
    }
    else if(!engine.m_world.m_rooms.empty())
    {
        // If there is no character present, move default camera position to
        // the first room (useful for TR1-3 cutscene levels).

        engine.m_camera.setPosition(engine.m_world.m_rooms[0]->getBoundingBox().max);
    }

    // Set gameflow parameters to default.
    // Reset secret trigger map.

    engine.gameflow.resetSecretStatus();
}

void Game_LevelTransition(Engine& engine, const boost::optional<int>& level)
{
    if(level)
        engine.m_gui.m_faders.assignPicture(gui::FaderType::LoadScreen, engine.engine_lua.getLoadingScreen(*level));
    else
        engine.m_gui.m_faders.assignPicture(gui::FaderType::LoadScreen, engine.engine_lua.getLoadingScreen(engine.gameflow.getLevelID()));
    engine.m_gui.m_faders.start(gui::FaderType::LoadScreen, gui::FaderDir::Out);

    engine.m_world.m_audioEngine.endStreams();
}
} // namespace engine
