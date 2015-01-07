
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
#include "obb.h"
#include "character_controller.h"
#include "redblack.h"
#include "gameflow.h"
#include "gui.h"
#include "inventory.h"

btScalar cam_angles[3] = {0.0, 0.0, 0.0};
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
        fprintf(*f, "\nspawnEntity(%d, 0x%X, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %d);", ent->bf.model->id, room_id,
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
    fprintf(*f, "\nsetEntityAnim(%d, %d, %d);", ent->id, ent->bf.current_animation, ent->bf.current_frame);
    fprintf(*f, "\nsetEntityState(%d, %d, %d);", ent->id, ent->bf.next_state, ent->bf.last_state);
    fprintf(*f, "\nsetEntityCollision(%d, %d);", ent->id, ent->self->collide_flag);
    if(ent->state_flags & ENTITY_STATE_ENABLED)
    {
        fprintf(*f, "\nenableEntity(%d);", ent->id);
    }
    else
    {
        fprintf(*f, "\ndisableEntity(%d);", ent->id);
    }
    fprintf(*f, "\nsetEntityFlags(%d, 0x%.4X, 0x%.4X, 0x%.8X);", ent->id, ent->state_flags, ent->type_flags, ent->callback_flags);
    fprintf(*f, "\nsetEntityActivationMask(%d, 0x%.4X);", ent->id, ent->activation_mask);
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

    fprintf(f, "setFlipmap(%d);\n",   engine_world.room_flipmap);
    fprintf(f, "setFlipstate(%d);\n", engine_world.room_flipstate);

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

    if(ent == NULL)                                                             ///@PARANOID
    {
        control_states.free_look = 1;
    }

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


void Cam_FollowEntity(struct camera_s *cam, struct entity_s *ent, btScalar dx, btScalar dz)
{
    btScalar alpha = cam_angles[0];
    btSphereShape cameraSphere(16.0);
    btTransform cameraFrom, cameraTo;
    btVector3 cam_pos, old_pos;
    bt_engine_ClosestConvexResultCallback *cb;

    vec3_copy(old_pos.m_floats, cam->pos);

    if((ent->character != NULL) && (ent->character->cam_follow_center > 0))
    {
        vec3_copy(cam_pos.m_floats, ent->obb->centre);
        ent->character->cam_follow_center--;
    }
    else
    {
        btScalar temp[16], transform[16];
        glGetFloatv (GL_MODELVIEW_MATRIX, temp);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glMultMatrixbt(ent->transform);
        glMultMatrixbt(ent->bf.bone_tags->full_transform);
        glGetFloatv (GL_MODELVIEW_MATRIX, transform);

        glLoadIdentity();
        glMultMatrixbt(temp);

        // bone_tags->mesh->centre[0] ent->bf.pos[0]*0.5 (ent->bf.bone_tags+ent->bf.bone_tag_count)->full_transform[12]  - 32.0 *  ent->transform[4 + 1]
        cam_pos.m_floats[0] = ((transform[12]));
        cam_pos.m_floats[1] = ((transform[13]));
        cam_pos.m_floats[2] = ((transform[14]) + dz ); // + 0.5  * (ent->bf.bb_max[2])
    }

    float shake_value   = renderer.cam->shake_value;
    float shake_time    = renderer.cam->shake_time;

    if((shake_time > 0.0) && (shake_value > 0.0))
    {
        float shake_value_x = ((rand() % abs(shake_value)) - (shake_value / 2)) * shake_time;
        float shake_value_y = ((rand() % abs(shake_value)) - (shake_value / 2)) * shake_time;
        float shake_value_z = ((rand() % abs(shake_value)) - (shake_value / 2)) * shake_time;

        cam_pos.m_floats[0] += shake_value_x;
        cam_pos.m_floats[1] += shake_value_y;
        cam_pos.m_floats[2] += shake_value_z;

        renderer.cam->shake_time -= engine_frame_time;
        renderer.cam->shake_time  = (renderer.cam->shake_time < 0.0)?(0.0):(renderer.cam->shake_time);
    }

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
        cam_angles[0] = (ent->angles[0] * (M_PI/180.0)); //TEMPORARY We convert the current entity's angle to radians!
    }

    vec3_copy(cam->pos, cam_pos.m_floats);

    cam->pos[2] -= 128.0;
    cam->current_room = Room_FindPosCogerrence(&engine_world, cam->pos, cam->current_room);
    cam->pos[2] += 128.0;

    if((cam->current_room != NULL) && (cam->current_room->flags & TR_ROOM_FLAG_QUICKSAND))
    {
        cam->pos[2] = cam->current_room->bb_max[2] + 2.0 * 64.0;
    }

    Cam_SetRotation(cam, cam_angles);

    cam->current_room = Room_FindPosCogerrence(&engine_world, cam->pos, cam->current_room);

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
            ent->character->cmd.kill = 1;                                       // Kill, if no HP.
        }

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

    if(ent && ent->character)
    {
        if(ent->character->cmd.action && (ent->type_flags & ENTITY_TYPE_TRIGGER_ACTIVATOR))
        {
            Entity_CheckActivators(ent);
        }
        if(Character_GetParam(ent, PARAM_HEALTH) <= 0.0)
        {
            ent->character->cmd.kill = 1;   // Kill, if no HP.
        }
    }

    if(engine_world.entity_tree && engine_world.entity_tree->root)
    {
        Game_UpdateCharactersTree(engine_world.entity_tree->root);
    }
}


__inline btScalar Game_Tick(btScalar *game_logic_time)
{
    int t;
    t = *game_logic_time / GAME_LOGIC_REFRESH_INTERVAL;
    *game_logic_time -= (btScalar)t * GAME_LOGIC_REFRESH_INTERVAL;
    return *game_logic_time;
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
        int32_t t = game_logic_time / GAME_LOGIC_REFRESH_INTERVAL;
        btScalar dt = (btScalar)t * GAME_LOGIC_REFRESH_INTERVAL;
        game_logic_time -= dt;
        Gameflow_Do();
        bt_engine_dynamicsWorld->stepSimulation(dt, 8);
        lua_DoTasks(engine_lua, dt);
        Game_UpdateAI();
        Audio_Update();
        if(engine_world.Character)
        {
            Character_UpdateParams(engine_world.Character);
        }
    }

    // This must be called EVERY frame to max out smoothness.
    // Includes animations, camera movement, and so on.

    if(engine_world.Character != NULL)
    {
        Game_ApplyControls(engine_world.Character);
    }

    if((engine_world.Character != NULL) && !control_states.noclip)
    {
        Character_ApplyCommands(engine_world.Character, &engine_world.Character->character->cmd);
        Entity_Frame(engine_world.Character, engine_frame_time);
        Cam_FollowEntity(renderer.cam, engine_world.Character, 0.0, 128.0); // 128.0 400.0
    }

    Game_UpdateCharacters();

    if((engine_world.entity_tree != NULL) && (engine_world.entity_tree->root != NULL))
    {
        Game_UpdateAllEntities(engine_world.entity_tree->root);
    }

    Controls_RefreshStates();
    Render_UpdateAnimTextures();
}

void Game_Prepare()
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

    // Set gameflow parameters to default.
    // Reset secret trigger map.

    memset(gameflow_manager.SecretsTriggerMap, 0, sizeof(gameflow_manager.SecretsTriggerMap));
}

void Game_LevelTransition(uint16_t level_index)
{
    char file_path[MAX_ENGINE_PATH];
    lua_GetLoadingScreen(engine_lua, level_index, file_path);
    Gui_FadeAssignPic(FADER_LOADSCREEN, file_path);
    Gui_FadeStart(FADER_LOADSCREEN, TR_FADER_DIR_OUT);
    Audio_EndStreams();
}
