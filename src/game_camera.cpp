
#include <stdlib.h>
#include <stdio.h>

#include "core/vmath.h"
#include "core/obb.h"
#include "core/system.h"
#include "render/camera.h"
#include "physics/physics.h"
#include "engine.h"
#include "controls.h"
#include "room.h"
#include "world.h"
#include "skeletal_model.h"
#include "entity.h"
#include "character_controller.h"


void Cam_PlayFlyBy(struct camera_state_s *cam_state, float time)
{
    const float max_time = cam_state->flyby->pos_x->base_points_count - 1;
    float speed = Spline_Get(cam_state->flyby->speed, cam_state->time);
    cam_state->time += time * speed / (1024.0f + 512.0f);
    if(cam_state->time <= max_time)
    {
        FlyBySequence_SetCamera(cam_state->flyby, &engine_camera, cam_state->time);
    }
    else
    {
        cam_state->state = CAMERA_STATE_NORMAL;
        cam_state->flyby = NULL;
        cam_state->time = 0.0f;
        Cam_SetFovAspect(&engine_camera, screen_info.fov, engine_camera.aspect);
    }
}


void Cam_FollowEntity(struct camera_s *cam, struct camera_state_s *cam_state, struct entity_s *ent)
{
    float cam_pos[3], cameraFrom[3], cameraTo[3];
    collision_result_t cb;
    const int16_t filter = COLLISION_GROUP_STATIC_ROOM | COLLISION_GROUP_STATIC_OBLECT | COLLISION_GROUP_KINEMATIC;
    const float test_r = 16.0f;

    vec3_copy(cam_pos, cam->gl_transform + 12);
    ///@INFO Basic camera override, completely placeholder until a system classic-like is created

    if(control_states.mouse_look == 0)//If mouse look is off
    {
        float currentAngle = control_states.cam_angles[0] * (M_PI / 180.0f);    //Current is the current cam angle
        float targetAngle  = ent->angles[0] * (M_PI / 180.0f); //Target is the target angle which is the entity's angle itself
        float rotSpeed = 2.0f; //Speed of rotation
        ///@FIXME
        //If Lara is in a specific state we want to rotate -75 deg or +75 deg depending on camera collision
        //if(current_state == TR_STATE_LARA_REACH)
        if((ent->move_type == MOVE_FREE_FALLING) && (ent->dir_flag == ENT_MOVE_FORWARD))
        {
            if(cam_state->target_dir == TR_CAM_TARG_BACK)
            {
                vec3_copy(cameraFrom, cam_pos);
                cameraTo[0] = cameraFrom[0] + sinf((ent->angles[0] - 90.0f) * (M_PI / 180.0f)) * control_states.cam_distance;
                cameraTo[1] = cameraFrom[1] - cosf((ent->angles[0] - 90.0f) * (M_PI / 180.0f)) * control_states.cam_distance;
                cameraTo[2] = cameraFrom[2];

                //If collided we want to go right otherwise stay left
                if(Physics_SphereTest(NULL, cameraFrom, cameraTo, test_r, ent->self, filter))
                {
                    cameraTo[0] = cameraFrom[0] + sinf((ent->angles[0] + 90.0f) * (M_PI / 180.0f)) * control_states.cam_distance;
                    cameraTo[1] = cameraFrom[1] - cosf((ent->angles[0] + 90.0f) * (M_PI / 180.0f)) * control_states.cam_distance;
                    cameraTo[2] = cameraFrom[2];

                    //If collided we want to go to back else right
                    if(Physics_SphereTest(NULL, cameraFrom, cameraTo, test_r, ent->self, filter))
                    {
                        cam_state->target_dir = TR_CAM_TARG_BACK;
                    }
                    else
                    {
                        cam_state->target_dir = TR_CAM_TARG_RIGHT;
                    }
                }
                else
                {
                    cam_state->target_dir = TR_CAM_TARG_LEFT;
                }
            }
        }
        else if((ent->move_type == MOVE_FREE_FALLING) && (ent->dir_flag == ENT_MOVE_BACKWARD))
        {
            cam_state->target_dir = TR_CAM_TARG_FRONT;
        }
        else if(cam_state->target_dir != TR_CAM_TARG_BACK)
        {
            cam_state->target_dir = TR_CAM_TARG_BACK;//Reset to back
        }

        //If target mis-matches current we need to update the camera's angle to reach target!
        if(currentAngle != targetAngle)
        {
            switch(cam_state->target_dir)
            {
            case TR_CAM_TARG_BACK:
                targetAngle = (ent->angles[0]) * (M_PI / 180.0f);
                break;
            case TR_CAM_TARG_FRONT:
                targetAngle = (ent->angles[0] - 180.0f) * (M_PI / 180.0f);
                break;
            case TR_CAM_TARG_LEFT:
                targetAngle = (ent->angles[0] - 75.0f) * (M_PI / 180.0f);
                break;
            case TR_CAM_TARG_RIGHT:
                targetAngle = (ent->angles[0] + 75.0f) * (M_PI / 180.0f);
                break;
            default:
                targetAngle = (ent->angles[0]) * (M_PI / 180.0f);
                break;
            }

            float d_angle = control_states.cam_angles[0] - targetAngle;
            if(d_angle > M_PI / 2.0f)
            {
                d_angle -= M_PI/180.0f;
            }
            if(d_angle < -M_PI / 2.0f)
            {
                d_angle += M_PI/180.0f;
            }
            control_states.cam_angles[0] = fmodf(control_states.cam_angles[0] + atan2f(sinf(currentAngle - d_angle), cosf(currentAngle + d_angle)) * (engine_frame_time * rotSpeed), M_PI * 2.0f); //Update camera's angle
        }
    }

    if((ent->character != NULL) && (ent->character->cam_follow_center > 0))
    {
        vec3_copy(cam_pos, ent->obb->centre);
        ent->character->cam_follow_center--;
    }
    else
    {
        Mat4_vec3_mul(cam_pos, ent->transform, ent->bf->bone_tags->full_transform + 12);
    }

    //Code to manage screen shaking effects
    /*if((cam_state->time > 0.0) && (cam_state->shake_value > 0.0))
    {
        cam_pos[0] += ((rand() % abs(cam_state->shake_value)) - (cam_state->shake_value / 2)) * cam_state->time;;
        cam_pos[1] += ((rand() % abs(cam_state->shake_value)) - (cam_state->shake_value / 2)) * cam_state->time;;
        cam_pos[2] += ((rand() % abs(cam_state->shake_value)) - (cam_state->shake_value / 2)) * cam_state->time;;
        cam_state->time  = (cam_state->time < 0.0)?(0.0):(cam_state->time)-engine_frame_time;
    }*/

    vec3_copy(cameraFrom, cam_pos);
    vec3_copy(cameraTo, cam_pos);
    cameraTo[2] += 2.0f * cam_state->entity_offset_z;

    if(Physics_SphereTest(&cb, cameraFrom, cameraTo, test_r, ent->self, filter))
    {
        vec3_add_mul(cam_pos, cb.point, cb.normale, 2.5f * test_r);
    }
    else
    {
        vec3_copy(cam_pos, cameraTo);
    }

    if (cam_state->entity_offset_x != 0.0f)
    {
        vec3_copy(cameraFrom, cam_pos);
        cam_pos[0] += cam_state->entity_offset_x * cam->gl_transform[0 + 0];
        cam_pos[1] += cam_state->entity_offset_x * cam->gl_transform[0 + 1];
        cam_pos[2] += cam_state->entity_offset_x * cam->gl_transform[0 + 2];
        vec3_copy(cameraTo, cam_pos);

        if(Physics_SphereTest(&cb, cameraFrom, cameraTo, test_r, ent->self, filter))
        {
            vec3_add_mul(cam_pos, cb.point, cb.normale, 2.0f);
        }

        vec3_copy(cameraFrom, cam_pos);
        if(cam_state->state == CAMERA_STATE_LOOK_AT)
        {
            entity_p target = World_GetEntityByID(cam_state->target_id);
            if(target && target != World_GetPlayer())
            {
                float dir2d[2], dist;
                dir2d[0] = target->transform[12 + 0] - cam->gl_transform[12 + 0];
                dir2d[1] = target->transform[12 + 1] - cam->gl_transform[12 + 1];
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

        if(Physics_SphereTest(&cb, cameraFrom, cameraTo, test_r, ent->self, filter))
        {
            vec3_add_mul(cam_pos, cb.point, cb.normale, 2.0f);
        }
    }

    //Update cam pos
    vec3_copy(cam->gl_transform + 12, cam_pos);

    // check quicksand
    {
        cam_pos[2] -= 128.0f;
        room_p check_room = World_FindRoomByPosCogerrence(cam_pos, cam->current_room);
        cam_pos[2] += 128.0f;
        if(check_room && (check_room->content->room_flags & TR_ROOM_FLAG_QUICKSAND))
        {
            cam->gl_transform[12 + 2] = check_room->bb_max[2] + 128.0f;
        }
    }

    Cam_SetRotation(cam, control_states.cam_angles);
}
