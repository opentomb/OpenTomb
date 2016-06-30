
#include <stdlib.h>
#include <stdio.h>

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include "core/system.h"
#include "core/obb.h"
#include "render/camera.h"
#include "controls.h"
#include "room.h"
#include "world.h"
#include "skeletal_model.h"
#include "entity.h"
//#include "script.h"
//#include "trigger.h"
#include "anim_state_control.h"
#include "character_controller.h"
#include "gameflow.h"
//#include "gui.h"


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


int Cam_CheckCollision(struct camera_s *cam, entity_s *ent, float angle)
{
    float cameraFrom[3], cameraTo[3];

    vec3_copy(cameraFrom, cam->pos);
    cameraTo[0] = cameraFrom[0] + sinf((ent->angles[0] + angle) * (M_PI / 180.0)) * control_states.cam_distance;
    cameraTo[1] = cameraFrom[1] - cosf((ent->angles[0] + angle) * (M_PI / 180.0)) * control_states.cam_distance;
    cameraTo[2] = cameraFrom[2];

    //Collision check
    if(Physics_SphereTest(NULL, cameraFrom, cameraTo, 16.0f, ent->self))
    {
        return 1;
    }

    return 0;
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
