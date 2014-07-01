
#include <stdlib.h>
#include <stdio.h>

#include "bullet/btBulletCollisionCommon.h"
#include "bullet/btBulletDynamicsCommon.h"
extern "C" {
#include "lua/lua.h"
#include "lua/lualib.h"
#include "lua/lauxlib.h"
#include "lua/lstate.h"
}

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
#include "bounding_volume.h"
#include "character_controller.h"
#include "redblack.h"
#include "gameflow.h"
#include "gui.h"

btScalar cam_angles[3] = {0.0, 0.0, 0.0};
extern lua_State *engine_lua;

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
        Con_Printf("noclip = %d", control_states.noclip);
        return 0;
    }

    control_states.noclip = lua_tointeger(lua, 1);
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

    if(engine_lua)
    {
        lua_register(engine_lua, "mlook", lua_mlook);
        lua_register(engine_lua, "freelook", lua_freelook);
        lua_register(engine_lua, "noclip", lua_noclip);
        lua_register(engine_lua, "cam_distance", lua_cam_distance);
    }
}

/**
 * Load game state
 */
int Game_Load(const char* name)
{
    FILE *f;
    char *buf, *ch, token[512], local;
    long int buf_size;

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
        f = fopen(token, "rb");
    }
    else
    {
        f = fopen(name, "rb");
    }

    if(!f)
    {
        Sys_extWarn("Can not read file \"%s\"", name);
        return 0;
    }

    Engine_LuaClearTasks();

    fseek(f, 0, SEEK_END);
    buf_size = ftell(f) + 1;
    fseek(f, 0, SEEK_SET);
    ch = buf = (char*)malloc(buf_size * sizeof(char));
    fread(buf, buf_size, sizeof(char), f);
    fclose(f);

    CVAR_set_val_d("engine_version", SC_ParseInt(&ch));
    ch = parse_token(ch, token);
    if(strcmp(CVAR_get_val_s("game_level"), token))
    {
        CVAR_set_val_s("game_level", token);
        Engine_LoadMap(token);
    }
    while(ch = parse_token(ch, token))
    {
        if(!strcmp("LARA", token))
        {
            SC_ParseEntity(&ch, engine_world.Character);
        }
    }

    free(buf);
    return 1;
}


/**
 * Вспомогательная ф-я сохранения entity
 */
void Save_Entity(FILE **f, entity_p ent)
{
    int r, x, y;
    if(!ent)
    {
        return;
    }
    fprintf(*f, "\n{");
    fprintf(*f, "\n\tpos \t%f\t%f\t%f", ent->transform[12], ent->transform[13], ent->transform[14]);
    fprintf(*f, "\n\tangles \t%f\t%f\t%f", ent->angles[0], ent->angles[1], ent->angles[2]);
    fprintf(*f, "\n\tanim \t%d\t%d\t%f", ent->current_animation, ent->current_frame, ent->frame_time);
    r = -1;
    x = -1;
    y = -1;
    if(ent->self->room)
    {
        r = ent->self->room->ID;
    }
    x = 1;
    y = 1;
    fprintf(*f, "\n\troom \t%d\t%d\t%d", r, x, y);
    fprintf(*f, "\n\tmove \t%d", ent->move_type);
    fprintf(*f, "\n\tspeed \t%f\t%f\t%f", ent->speed.m_floats[0], ent->speed.m_floats[1], ent->speed.m_floats[2]);
    fprintf(*f, "\n}\n");
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

    fprintf(f, "%d\n", (int)CVAR_get_val_d("engine_version"));
    fprintf(f, "\"%s\"\n", CVAR_get_val_s("game_level"));
    fprintf(f, "\nLARA");
    Save_Entity(&f, engine_world.Character);
    fclose(f);

    return 1;
}

void Game_ApplyControls(struct entity_s *ent)
{
    int8_t move_logic[3];
    int8_t look_logic[3];

    /*
     * MOVE KB LOGIC
     */
    move_logic[0] = control_states.move_forward - control_states.move_backward;
    move_logic[1] = control_states.move_right - control_states.move_left;
    move_logic[2] = control_states.move_up - control_states.move_down;

    /*
     * VIEW KB LOGIC
     */

    look_logic[0] = control_states.look_left - control_states.look_right;
    look_logic[1] = control_states.look_down - control_states.look_up;
    look_logic[2] = control_states.look_roll_right - control_states.look_roll_left;

    /*
     * CONTROL  APPLY
     */

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
        Cam_MoveAlong(renderer.cam, control_states.free_look_speed * move_logic[0] * engine_frame_time);
        Cam_MoveStrafe(renderer.cam, control_states.free_look_speed * move_logic[1] * engine_frame_time);
        Cam_MoveVertical(renderer.cam, control_states.free_look_speed * move_logic[2] * engine_frame_time);

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
    if(control_states.free_look != 0)
    {
        Cam_SetRotation(renderer.cam, cam_angles);
        Cam_MoveAlong(renderer.cam, control_states.free_look_speed * move_logic[0] * engine_frame_time);
        Cam_MoveStrafe(renderer.cam, control_states.free_look_speed * move_logic[1] * engine_frame_time);
        Cam_MoveVertical(renderer.cam, control_states.free_look_speed * move_logic[2] * engine_frame_time);
        renderer.cam->current_room = Room_FindPosCogerrence(renderer.world, renderer.cam->pos, renderer.cam->current_room);
    }
    else if(control_states.noclip != 0)
    {
        btVector3 pos;
        Cam_SetRotation(renderer.cam, cam_angles);
        Cam_MoveAlong(renderer.cam, control_states.free_look_speed * move_logic[0] * engine_frame_time);
        Cam_MoveStrafe(renderer.cam, control_states.free_look_speed * move_logic[1] * engine_frame_time);
        Cam_MoveVertical(renderer.cam, control_states.free_look_speed * move_logic[2] * engine_frame_time);
        renderer.cam->current_room = Room_FindPosCogerrence(renderer.world, renderer.cam->pos, renderer.cam->current_room);

        ent->angles[0] = 180.0 * cam_angles[0] / M_PI;
        pos.m_floats[0] = renderer.cam->pos[0] + renderer.cam->view_dir[0] * control_states.cam_distance;
        pos.m_floats[1] = renderer.cam->pos[1] + renderer.cam->view_dir[1] * control_states.cam_distance;
        pos.m_floats[2] = renderer.cam->pos[2] + renderer.cam->view_dir[2] * control_states.cam_distance - 512.0;
        vec3_copy(ent->transform+12, pos.m_floats);
        Entity_UpdateRotation(ent);
    }
    else
    {
        // Apply controls to Lara
        ent->character->cmd.action = control_states.state_action;
        ent->character->cmd.jump = control_states.do_jump;
        ent->character->cmd.shift = control_states.state_walk;

        ent->character->cmd.roll = ((control_states.move_forward && control_states.move_backward) || control_states.do_roll);
        ent->character->cmd.sprint = control_states.state_sprint;              // New commands only for TR3 and above
        ent->character->cmd.crouch = control_states.state_crouch;

        if(control_states.use_small_medi)
        {
            if(Character_IncreaseHealth(ent, 250))
            {
                Audio_Send(TR_AUDIO_SOUND_MEDIPACK);
            }

            control_states.use_small_medi = !control_states.use_small_medi;
        }

        if(control_states.use_big_medi)
        {
            if(Character_IncreaseHealth(ent, CHARACTER_OPTION_HEALTH_MAX))
            {
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


void Cam_FollowEntity(struct camera_s *cam, struct entity_s *ent, btScalar dx, btScalar dz)
{
    btScalar alpha = cam_angles[0];
    btSphereShape cameraSphere(16.0);
    btTransform cameraFrom, cameraTo;
    btVector3 cam_pos, old_pos;
    bt_engine_ClosestConvexResultCallback *cb;

    vec3_copy(old_pos.m_floats, cam->pos);
    cam_pos.m_floats[0] = ent->transform[12] - 32.0 * ent->transform[4 + 0];
    cam_pos.m_floats[1] = ent->transform[13] - 32.0 * ent->transform[4 + 1];
    cam_pos.m_floats[2] = ent->transform[14] + 0.5 * (ent->bf.bb_max[2] /*+ ent->bf.bb_min[2]*/);

    cameraFrom.setIdentity();
    cameraFrom.setOrigin(cam_pos);
    cam_pos.m_floats[2] += dz;
    cameraTo.setIdentity();
    cameraTo.setOrigin(cam_pos);

    if(ent->character)
    {
        cb = ent->character->convex_cb;
    }
    else
    {
        cb = new bt_engine_ClosestConvexResultCallback(ent->self);
        cb->m_collisionFilterMask = btBroadphaseProxy::StaticFilter | btBroadphaseProxy::KinematicFilter;
    }

    cb->m_closestHitFraction = 1.0;
    cb->m_hitCollisionObject = NULL;
    bt_engine_dynamicsWorld->convexSweepTest(&cameraSphere, cameraFrom, cameraTo, *cb);
    if(cb->hasHit())
    {
        cam_pos.setInterpolate3(cameraFrom.getOrigin(), cameraTo.getOrigin(), cb->m_closestHitFraction);
        cam_pos += cb->m_hitNormalWorld * 2.0;
    }

    cameraFrom.setOrigin(cam_pos);
    cam_pos.m_floats[0] += dx * cam->right_dir[0];
    cam_pos.m_floats[1] += dx * cam->right_dir[1];
    cam_pos.m_floats[2] += dx * cam->right_dir[2];
    cameraTo.setOrigin(cam_pos);
    cb->m_closestHitFraction = 1.0;
    cb->m_hitCollisionObject = NULL;
    bt_engine_dynamicsWorld->convexSweepTest(&cameraSphere, cameraFrom, cameraTo, *cb);
    if(cb->hasHit())
    {
        cam_pos.setInterpolate3(cameraFrom.getOrigin(), cameraTo.getOrigin(), cb->m_closestHitFraction);
        cam_pos += cb->m_hitNormalWorld * 2.0;
    }

    cameraSphere.setLocalScaling(btVector3(0.8, 0.8, 0.8));
    cameraFrom.setOrigin(cam_pos);
    cam_pos.m_floats[0] += sin(alpha) * control_states.cam_distance;
    cam_pos.m_floats[1] -= cos(alpha) * control_states.cam_distance;
    cameraTo.setOrigin(cam_pos);
    cb->m_closestHitFraction = 1.0;
    cb->m_hitCollisionObject = NULL;
    bt_engine_dynamicsWorld->convexSweepTest(&cameraSphere, cameraFrom, cameraTo, *cb);
    if(cb->hasHit())
    {
        cam_pos.setInterpolate3(cameraFrom.getOrigin(), cameraTo.getOrigin(), cb->m_closestHitFraction);
        cam_pos += cb->m_hitNormalWorld * 2.0;
    }

    alpha = cam_pos.distance2(old_pos);
    if(alpha > 54.0 * 54.0 && alpha < 1024.0 * 1024.0)
    {
        cam_pos -= old_pos;
        cam_pos *= 54.0 * 60.0 * engine_frame_time / cam_pos.length();
        cam_pos += old_pos;
    }

    if(control_states.mouse_look == 0)//If mouse look is off
    {
        cam_angles[0] = (ent->angles[0] * (M_PI/180)); //TEMPORARY We convert the current entity's angle to radians!
    }

    vec3_copy(cam->pos, cam_pos.m_floats);
    Cam_SetRotation(cam, cam_angles);

    if(!ent->character)
    {
        delete[] cb;
    }
}

void Game_UpdateAllEntities(struct RedBlackNode_s *x)
{
    entity_p entity = (entity_p)x->data;

    if(Entity_Frame(entity, engine_frame_time))
    {
        Entity_UpdateRigidBody(entity);
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

    if(ent->character && ent->character->cmd.action && (ent->flags & ENTITY_CAN_TRIGGER))
    {
        Entity_CheckActivators(ent);
    }
    if(ent->character)
    {
        Character_ApplyCommands(ent, &ent->character->cmd);
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

    if(ent->character)
    {
        if(ent->character->cmd.action && (ent->flags & ENTITY_CAN_TRIGGER))
            Entity_CheckActivators(ent);

        if(ent->character->opt.health <= 0.0)
        {
            ent->character->cmd.kill = 1;   // Kill, if no HP.
        }
    }

    if(engine_world.entity_tree && engine_world.entity_tree->root)
    {
        Game_UpdateCharactersTree(engine_world.entity_tree->root);
    }
}

btScalar Game_Tick(btScalar *game_logic_time)
{
    int t;
    t = *game_logic_time / GAME_LOGIC_REFRESH_INTERVAL;
    *game_logic_time -= (btScalar)t * GAME_LOGIC_REFRESH_INTERVAL;
}

void Game_Frame(btScalar time)
{
    static btScalar game_logic_time  = 0.0;
                    game_logic_time += time;

    // If console is active, only thing to update is audio.

    if(con_base.show)
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
        Gameflow_Do();
        bt_engine_dynamicsWorld->stepSimulation(game_logic_time, 8);
        lua_DoTasks(engine_lua, game_logic_time);
        Game_UpdateAI();
        Audio_Update();
        if(engine_world.Character)
        {
            Character_UpdateValues(engine_world.Character);
        }

        Game_Tick(&game_logic_time);
    }

    // This must be called EVERY frame to max out smoothness.
    // Includes animations, camera movement, and so on.

    Game_ApplyControls(engine_world.Character);

    if(engine_world.Character && !control_states.noclip)
    {
        Character_ApplyCommands(engine_world.Character, &engine_world.Character->character->cmd);
        Entity_Frame(engine_world.Character, engine_frame_time);
        Cam_FollowEntity(renderer.cam, engine_world.Character, 128.0, 400.0);
    }

    if(engine_world.Character)
    {
        Game_UpdateCharacters();
    }

    if(engine_world.entity_tree && engine_world.entity_tree->root)
    {
        Game_UpdateAllEntities(engine_world.entity_tree->root);
    }

    Render_UpdateAnimTextures();
}

void Game_Prepare()
{
    // Set character values to default.
    
    Character_SetHealth(engine_world.Character, CHARACTER_OPTION_HEALTH_MAX);
    Character_SetAir(engine_world.Character   , CHARACTER_OPTION_AIR_MAX);
    Character_SetSprint(engine_world.Character, CHARACTER_OPTION_SPRINT_MAX);
    
    // Set gameflow parameters to default.
    
    // Reset secret trigger map.
    
    memset(gameflow_manager.SecretsTriggerMap, 0, sizeof(gameflow_manager.SecretsTriggerMap));
}

void Game_LevelTransition(uint16_t level_index)
{
    char file_path[256];
    lua_GetLoadingScreen(engine_lua, gameflow_manager.CurrentLevelID, level_index, file_path);
    Gui_FadeAssignPic(FADER_LOADSCREEN, file_path);
    Gui_FadeStart(FADER_LOADSCREEN, TR_FADER_DIR_OUT);
    Audio_EndStreams();
}
