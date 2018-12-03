
#include <SDL2/SDL.h>
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/system.h"
#include "core/gl_util.h"
#include "core/gl_font.h"
#include "core/gl_text.h"
#include "core/console.h"
#include "core/vmath.h"
#include "render/camera.h"
#include "render/render.h"
#include "render/shader_manager.h"
#include "render/bsp_tree.h"
#include "physics/physics.h"
#include "engine.h"
#include "controls.h"
#include "mesh.h"
#include "skeletal_model.h"
#include "entity.h"
#include "character_controller.h"
#include "room.h"
#include "trigger.h"
#include "world.h"


static ss_bone_frame_t  g_test_model = {0};
static int32_t          g_test_model_index = 0;
static float            ray_test_point[3] = {0.0f, 0.0f, 0.0f};
engine_container_p      last_cont = NULL;


void SetTestModel(int index);


void ClearTestModel()
{
    SSBoneFrame_Clear(&g_test_model);
}


void Test_SecondaryMouseDown()
{
    float from[3], to[3];
    engine_container_t cam_cont;
    collision_result_t cb;

    vec3_copy(from, engine_camera.transform.M4x4 + 12);
    vec3_add_mul(to, from, engine_camera.transform.M4x4 + 8, 32768.0f);

    cam_cont.next = NULL;
    cam_cont.object = NULL;
    cam_cont.object_type = 0;
    cam_cont.room = engine_camera.current_room;

    if(Physics_RayTest(&cb, from, to, &cam_cont, COLLISION_MASK_ALL))
    {
        if(cb.obj && cb.obj->object_type != OBJECT_BULLET_MISC)
        {
            last_cont = cb.obj;
            vec3_copy(ray_test_point, cb.point);
        }
    }
    
    if(last_cont && last_cont->object_type == OBJECT_ENTITY)
    {
        entity_p player = World_GetPlayer();
        if(player && player->character)
        {
            Character_SetTarget(player, ((entity_p)last_cont->object)->id);
        }
    }
    else
    {
        entity_p player = World_GetPlayer();
        if(player && player->character)
        {
            Character_SetTarget(player, ENTITY_ID_NONE);
        }
    }
}


void TestModelApplyKey(int key)
{
    switch(key)
    {
        case SDL_SCANCODE_LEFTBRACKET:
            SetTestModel(--g_test_model_index);
            break;

        case SDL_SCANCODE_RIGHTBRACKET:
            SetTestModel(++g_test_model_index);
            break;

        case SDL_SCANCODE_O:
            if(g_test_model.animations.prev_animation > 0)
            {
                Anim_SetAnimation(&g_test_model.animations, g_test_model.animations.prev_animation - 1, 0);
            }
            break;

        case SDL_SCANCODE_P:
            Anim_SetAnimation(&g_test_model.animations, g_test_model.animations.prev_animation + 1, 0);
            break;

        default:
            break;
    }
}


void SetTestModel(int index)
{
    skeletal_model_p sm;
    uint32_t sm_count;
    World_GetSkeletalModelsInfo(&sm, &sm_count);

    index = (index >= 0) ? (index) : (sm_count - 1);
    index = (index >= sm_count) ? (0) : (index);

    if(sm_count > 0)
    {
        g_test_model_index = index;
        SSBoneFrame_Clear(&g_test_model);
        SSBoneFrame_CreateFromModel(&g_test_model, sm + index);
    }
}


void ShowModelView(float time)
{
    static engine_transform_t tr;
    static float test_model_angles[3] = {45.0f, 45.0f, 0.0f};
    static float test_model_dist = 1024.0f;
    static float test_model_z_offset = 256.0f;
    uint32_t sm_count;
    skeletal_model_p sm = NULL;

    World_GetSkeletalModelsInfo(&sm, &sm_count);
    if((g_test_model_index >= 0) && (g_test_model_index < sm_count))
    {
        sm += g_test_model_index;
    }

    if(sm && (g_test_model.animations.model == sm))
    {
        float subModelView[16], subModelViewProjection[16];
        float *cam_pos = engine_camera.transform.M4x4 + 12;
        animation_frame_p af = sm->animations + g_test_model.animations.prev_animation;
        const int current_light_number = 0;
        const lit_shader_description *shader = renderer.shaderManager->getEntityShader(current_light_number);

        if(control_states.actions[ACT_LOOKRIGHT].state || control_states.actions[ACT_RIGHT].state)
        {
            test_model_angles[0] += time * 256.0f;
        }
        if(control_states.actions[ACT_LOOKLEFT].state || control_states.actions[ACT_LEFT].state)
        {
            test_model_angles[0] -= time * 256.0f;
        }
        if(control_states.actions[ACT_LOOKUP].state)
        {
            test_model_angles[1] += time * 256.0f;
        }
        if(control_states.actions[ACT_LOOKDOWN].state)
        {
            test_model_angles[1] -= time * 256.0f;
        }
        if(control_states.actions[ACT_UP].state && (test_model_dist >= 8.0f))
        {
            test_model_dist -= time * 512.0f;
        }
        if(control_states.actions[ACT_DOWN].state)
        {
            test_model_dist += time * 512.0f;
        }
        if(control_states.actions[ACT_JUMP].state)
        {
            test_model_z_offset += time * 512.0f;
        }
        if(control_states.actions[ACT_CROUCH].state)
        {
            test_model_z_offset -= time * 512.0f;
        }

        g_test_model.transform = &tr;
        Mat4_E_macro(tr.M4x4);
        Mat4_E_macro(engine_camera.transform.M4x4);
        engine_camera.transform.angles[0] = test_model_angles[0];
        engine_camera.transform.angles[1] = test_model_angles[1] + 90.0f;
        engine_camera.transform.angles[2] = test_model_angles[2];
        Mat4_SetAnglesZXY(engine_camera.transform.M4x4, engine_camera.transform.angles);
        cam_pos[0] = -engine_camera.transform.M4x4[8 + 0] * test_model_dist;
        cam_pos[1] = -engine_camera.transform.M4x4[8 + 1] * test_model_dist;
        cam_pos[2] = -engine_camera.transform.M4x4[8 + 2] * test_model_dist + test_model_z_offset;
        Cam_Apply(&engine_camera);

        if(Anim_IncTime(&g_test_model.animations, time))
        {
            Anim_SetAnimation(&g_test_model.animations, g_test_model.animations.current_animation, 0);
        }
        SSBoneFrame_Update(&g_test_model, 0.0f);

        Mat4_Mat4_mul(subModelView, engine_camera.gl_view_mat, tr.M4x4);
        Mat4_Mat4_mul(subModelViewProjection, engine_camera.gl_view_proj_mat, tr.M4x4);
        qglUseProgramObjectARB(shader->program);

        {
            GLfloat ambient_component[4] = {1.0f, 1.0f, 1.0f, 1.0f};
            qglUniform4fvARB(shader->light_ambient, 1, ambient_component);
            qglUniform1fARB(shader->dist_fog, 65536.0f);
        }
        renderer.DrawSkeletalModel(shader, &g_test_model, subModelView, subModelViewProjection);
        renderer.debugDrawer->DrawAxis(4096.0f, tr.M4x4);

        for(int i = 0; i < g_test_model.bone_tag_count; ++i)
        {
            ss_bone_tag_p bf = g_test_model.bone_tags + i;
            Mat4_vec3_mul_macro(tr.M4x4, bf->current_transform, bf->mesh_base->centre);
            renderer.OutTextXYZ(tr.M4x4[0], tr.M4x4[1], tr.M4x4[2], "%d", i);
        }

        {
            const float dy = -18.0f * screen_info.scale_factor;
            float y = (float)screen_info.h + dy;

            GLText_OutTextXY(30.0f, y += dy, "MODEL[%d]; state: %d", (int)sm->id, (int)af->state_id);
            GLText_OutTextXY(30.0f, y += dy, "anim: %d of %d", (int)g_test_model.animations.prev_animation, (int)sm->animation_count);
            GLText_OutTextXY(30.0f, y += dy, "frame: %d of %d, %d", (int)g_test_model.animations.prev_frame, (int)af->max_frame, (int)af->frames_count);
            GLText_OutTextXY(30.0f, y += dy, "next a: %d,next f: %d", (int)af->next_anim->id, (int)af->next_frame);

            for(animation_command_p cmd = af->commands; cmd; cmd = cmd->next)
            {
                GLText_OutTextXY(30.0f, y += dy, "command[%d][frame = %d]: {%.1f,  %.1f,  %.1f}",
                    (int)cmd->id, (int)cmd->frame, cmd->data[0], cmd->data[1], cmd->data[2]);
            }

            y = (float)screen_info.h + dy;
            for(uint16_t i = 0; i < af->state_change_count; ++i)
            {
                state_change_p stc = af->state_change + i;
                GLText_OutTextXY(screen_info.w - 350, y += dy, "state change[%d]:", (int)stc->id);
                for(uint16_t j = 0; j < stc->anim_dispatch_count; ++j)
                {
                    anim_dispatch_p disp = stc->anim_dispatch + j;
                    GLText_OutTextXY(screen_info.w - 320, y += dy, "frame_int(%d, %d), to (%d, %d):", (int)disp->frame_low, (int)disp->frame_high, (int)disp->next_anim, (int)disp->next_frame);
                }
            }
        }
    }
    else
    {
        SetTestModel(g_test_model_index);
    }
}


void ShowDebugInfo()
{
    float y = (float)screen_info.h;
    const float dy = -18.0f * screen_info.scale_factor;
    extern engine_container_p last_cont;
    
    if(last_cont && (screen_info.debug_view_state != debug_view_state_e::model_view))
    {
        GLText_OutTextXY(30.0f, y += dy, "VIEW: Selected object");
        switch(last_cont->object_type)
        {
            case OBJECT_ENTITY:
                GLText_OutTextXY(30.0f, y += dy, "cont_entity: id = %d, model = %d, room = %d", ((entity_p)last_cont->object)->id, ((entity_p)last_cont->object)->bf->animations.model->id, (last_cont->room) ? (last_cont->room->id) : (-1));
                break;

            case OBJECT_STATIC_MESH:
                GLText_OutTextXY(30.0f, y += dy, "cont_static: id = %d, room = %d", ((static_mesh_p)last_cont->object)->object_id, (last_cont->room) ? (last_cont->room->id) : (-1));
                break;

            case OBJECT_ROOM_BASE:
                {
                    room_p room = (room_p)last_cont->object;
                    room_sector_p rs = Room_GetSectorRaw(room, ray_test_point);
                    if(rs != NULL)
                    {
                        renderer.debugDrawer->SetColor(0.0, 1.0, 0.0);
                        renderer.debugDrawer->DrawSectorDebugLines(rs);
                        GLText_OutTextXY(30.0f, y += dy, "cont_room: (id = %d, sx = %d, sy = %d)", room->id, rs->index_x, rs->index_y);
                        GLText_OutTextXY(30.0f, y += dy, "room_below = %d, room_above = %d", (rs->room_below) ? (rs->room_below->id) : (-1), (rs->room_above) ? (rs->room_above->id) : (-1));
                        if(rs->trigger)
                        {
                            char trig_type[64];
                            char trig_func[64];
                            char trig_mask[16];
                            Trigger_TrigTypeToStr(trig_type, 64, rs->trigger->sub_function);
                            Trigger_TrigMaskToStr(trig_mask, rs->trigger->mask);
                            GLText_OutTextXY(30.0f, y += dy, "trig(sub = %s, val = 0x%X, mask = 0b%s, timer = %d)", trig_type, rs->trigger->function_value, trig_mask, rs->trigger->timer);
                            for(trigger_command_p cmd = rs->trigger->commands; cmd; cmd = cmd->next)
                            {
                                entity_p trig_obj = World_GetEntityByID(cmd->operands);
                                if(trig_obj)
                                {
                                    renderer.debugDrawer->SetColor(0.0, 0.0, 1.0);
                                    renderer.debugDrawer->DrawBBox(trig_obj->bf->bb_min, trig_obj->bf->bb_max, trig_obj->transform.M4x4);
                                    Trigger_TrigMaskToStr(trig_mask, trig_obj->trigger_layout);
                                    gl_text_line_p text = renderer.OutTextXYZ(trig_obj->transform.M4x4[12 + 0], trig_obj->transform.M4x4[12 + 1], trig_obj->transform.M4x4[12 + 2], "(id = 0x%X, layout = 0b%s)", trig_obj->id, trig_mask);
                                    if(text)
                                    {
                                        text->x_align = GLTEXT_ALIGN_CENTER;
                                    }
                                }
                                Trigger_TrigCmdToStr(trig_func, 64, cmd->function);
                                if(cmd->function == TR_FD_TRIGFUNC_SET_CAMERA)
                                {
                                    GLText_OutTextXY(30.0f, y += dy, "   cmd(func = %s, op = 0x%X, cam_id = 0x%X, cam_move = %d, cam_timer = %d)", trig_func, cmd->operands, cmd->camera.index, cmd->camera.move, cmd->camera.timer);
                                }
                                else
                                {
                                    GLText_OutTextXY(30.0f, y += dy, "   cmd(func = %s, op = 0x%X)", trig_func, cmd->operands);
                                }
                            }
                        }
                    }
                }
                break;
        }
    }

    switch(screen_info.debug_view_state)
    {
        case debug_view_state_e::player_anim:
            {
                GLText_OutTextXY(30.0f, y += dy, "VIEW: Lara anim");
                entity_p ent = World_GetPlayer();
                if(ent && ent->character)
                {
                    animation_frame_p anim = ent->bf->animations.model->animations + ent->bf->animations.prev_animation;
                    GLText_OutTextXY(30.0f, y += dy, "curr_st = %03d, next_st = %03d", anim->state_id, ent->bf->animations.target_state);
                    GLText_OutTextXY(30.0f, y += dy, "curr_anim = %03d, curr_frame = %03d, next_anim = %03d, next_frame = %03d", ent->bf->animations.prev_animation, ent->bf->animations.prev_frame, ent->bf->animations.current_animation, ent->bf->animations.current_frame);
                    GLText_OutTextXY(30.0f, y += dy, "anim_next_anim = %03d, anim_next_frame = %03d", anim->next_anim->id, anim->next_frame);
                    GLText_OutTextXY(30.0f, y += dy, "posX = %f, posY = %f, posZ = %f", ent->transform.M4x4[12], ent->transform.M4x4[13], ent->transform.M4x4[14]);
                }
            }
            break;

        case debug_view_state_e::sector_info:
            {
                entity_p ent = World_GetPlayer();
                GLText_OutTextXY(30.0f, y += dy, "VIEW: Sector info");
                if(engine_camera.current_room)
                {
                    GLText_OutTextXY(30.0f, y += dy, "cam_room = (id = %d)", engine_camera.current_room->id);
                }
                if(ent && ent->self->room)
                {
                    GLText_OutTextXY(30.0f, y += dy, "char_pos = (%.1f, %.1f, %.1f)", ent->transform.M4x4[12 + 0], ent->transform.M4x4[12 + 1], ent->transform.M4x4[12 + 2]);
                    room_p room = ent->self->room;
                    room_sector_p rs = Room_GetSectorRaw(room, ent->transform.M4x4 + 12);
                    if(rs != NULL)
                    {
                        renderer.debugDrawer->SetColor(0.0f, 1.0f, 0.0f);
                        renderer.debugDrawer->DrawSectorDebugLines(rs);
                        GLText_OutTextXY(30.0f, y += dy, "room = (id = %d, sx = %d, sy = %d)", room->id, rs->index_x, rs->index_y);
                        GLText_OutTextXY(30.0f, y += dy, "room_below = %d, room_above = %d", (rs->room_below) ? (rs->room_below->id) : (-1), (rs->room_above) ? (rs->room_above->id) : (-1));
                        for(int i = 0; i < room->content->overlapped_room_list_size; ++i)
                        {
                            GLText_OutTextXY(30.0f, y += dy, "overlapped room (id = %d)", room->content->overlapped_room_list[i]->id);
                        }
                        for(int i = 0; i < room->content->near_room_list_size; ++i)
                        {
                            GLText_OutTextXY(30.0f, y += dy, "near room (id = %d)", room->content->near_room_list[i]->id);
                        }
                        if(rs->trigger)
                        {
                            char trig_type[64];
                            char trig_func[64];
                            char trig_mask[16];
                            Trigger_TrigTypeToStr(trig_type, 64, rs->trigger->sub_function);
                            Trigger_TrigMaskToStr(trig_mask, rs->trigger->mask);
                            GLText_OutTextXY(30.0f, y += dy, "trig(sub = %s, val = 0x%X, mask = 0b%s, timer = %d)", trig_type, rs->trigger->function_value, trig_mask, rs->trigger->timer);
                            for(trigger_command_p cmd = rs->trigger->commands; cmd; cmd = cmd->next)
                            {
                                entity_p trig_obj = World_GetEntityByID(cmd->operands);
                                if(trig_obj)
                                {
                                    renderer.debugDrawer->SetColor(0.0f, 0.0f, 1.0f);
                                    renderer.debugDrawer->DrawBBox(trig_obj->bf->bb_min, trig_obj->bf->bb_max, trig_obj->transform.M4x4);
                                    Trigger_TrigMaskToStr(trig_mask, trig_obj->trigger_layout);
                                    gl_text_line_p text = renderer.OutTextXYZ(trig_obj->transform.M4x4[12 + 0], trig_obj->transform.M4x4[12 + 1], trig_obj->transform.M4x4[12 + 2], "(id = 0x%X, layout = 0b%s)", trig_obj->id, trig_mask);
                                    if(text)
                                    {
                                        text->x_align = GLTEXT_ALIGN_CENTER;
                                    }
                                }
                                Trigger_TrigCmdToStr(trig_func, 64, cmd->function);
                                if(cmd->function == TR_FD_TRIGFUNC_SET_CAMERA)
                                {
                                    GLText_OutTextXY(30.0f, y += dy, "   cmd(func = %s, op = 0x%X, cam_id = 0x%X, cam_move = %d, cam_timer = %d)", trig_func, cmd->operands, cmd->camera.index, cmd->camera.move, cmd->camera.timer);
                                }
                                else
                                {
                                    GLText_OutTextXY(30.0f, y += dy, "   cmd(func = %s, op = 0x%X)", trig_func, cmd->operands);
                                }
                            }
                        }
                    }
                }
            }
            break;

        case debug_view_state_e::room_objects:
            {
                entity_p ent = World_GetPlayer();
                GLText_OutTextXY(30.0f, y += dy, "VIEW: Room objects");
                if(ent && ent->self->room)
                {
                    room_p r = ent->self->room;
                    for(engine_container_p cont = r->containers; cont; cont = cont->next)
                    {
                        if(cont->object_type == OBJECT_ENTITY)
                        {
                            entity_p e = (entity_p)cont->object;
                            gl_text_line_p text = renderer.OutTextXYZ(e->transform.M4x4[12 + 0], e->transform.M4x4[12 + 1], e->transform.M4x4[12 + 2], "(entity[0x%X])", e->id);
                            if(text)
                            {
                                text->x_align = GLTEXT_ALIGN_CENTER;
                            }
                        }
                    }

                    for(uint32_t i = 0; i < r->content->static_mesh_count; ++i)
                    {
                        static_mesh_p sm = r->content->static_mesh + i;
                        gl_text_line_p text = renderer.OutTextXYZ(sm->pos[0], sm->pos[1], sm->pos[2], "(static[0x%X])", sm->object_id);
                        if(text)
                        {
                            text->x_align = GLTEXT_ALIGN_CENTER;
                        }
                    }
                }
            }
            break;

        case debug_view_state_e::ai_boxes:
            {
                entity_p ent = World_GetPlayer();
                GLText_OutTextXY(30.0f, y += dy, "VIEW: AI boxes");
                if(ent && ent->self->sector && ent->self->sector->box)
                {
                    room_box_p box = ent->self->sector->box;
                    GLText_OutTextXY(30.0f, y += dy, "box = %d, floor = %d", (int)box->id, (int)box->bb_min[2]);
                    GLText_OutTextXY(30.0f, y += dy, "blockable = %d, blocked = %d", (int)box->is_blockable, (int)box->is_blocked);
                    GLText_OutTextXY(30.0f, y += dy, "fly = %d", (int)box->zone[0].FlyZone);
                    GLText_OutTextXY(30.0f, y += dy, "zones = %d, %d, %d, %d", (int)box->zone[0].GroundZone1, (int)box->zone[0].GroundZone2, (int)box->zone[0].GroundZone3, (int)box->zone[0].GroundZone4);
                    for(box_overlap_p ov = box->overlaps; ov; ov++)
                    {
                        GLText_OutTextXY(30.0f, y += dy, "overlap = %d", (int)ov->box);
                        if(ov->end)
                        {
                            break;
                        }
                    }

                    float tr[16];
                    Mat4_E_macro(tr);
                    renderer.debugDrawer->DrawBBox(box->bb_min, box->bb_max, tr);
                    if(ent->character && last_cont && (last_cont->object_type == OBJECT_ENTITY))
                    {
                        entity_p foe = (entity_p)last_cont->object;
                        if(foe->character && foe->self->sector)
                        {
                            Character_UpdatePath(foe, ent->self->sector);
                            if(foe->character->path_dist > 0)
                            {
                                renderer.debugDrawer->SetColor(0.0f, 0.0f, 0.0f);
                                for(int i = 0; i < foe->character->path_dist; ++i)
                                {
                                    renderer.debugDrawer->DrawBBox(foe->character->path[i]->bb_min, foe->character->path[i]->bb_max, tr);
                                }

                                GLfloat red[3] = {1.0f, 0.0f, 0.0f};
                                GLfloat from[3], to[3];
                                vec3_copy(from, foe->self->sector->pos);
                                from[2] = foe->transform.M4x4[12 + 2] + TR_METERING_STEP;
                                for(int i = 1; i < foe->character->path_dist; ++i)
                                {
                                    Room_GetOverlapCenter(foe->character->path[i], foe->character->path[i - 1], to);
                                    renderer.debugDrawer->DrawLine(from, to, red, red);
                                    vec3_copy(from, to);
                                }
                                vec3_copy(to, ent->self->sector->pos);
                                to[2] = ent->transform.M4x4[12 + 2] + TR_METERING_STEP;
                                renderer.debugDrawer->DrawLine(from, to, red, red);
                            }
                        }
                    }
                }
            }
            break;

        case debug_view_state_e::bsp_info:
            GLText_OutTextXY(30.0f, y += dy, "VIEW: BSP tree info");
            if(renderer.dynamicBSP)
            {
                GLText_OutTextXY(30.0f, y += dy, "input polygons = %07d", renderer.dynamicBSP->GetInputPolygonsCount());
                GLText_OutTextXY(30.0f, y += dy, "added polygons = %07d", renderer.dynamicBSP->GetAddedPolygonsCount());
            }
            break;

        case debug_view_state_e::model_view:
            GLText_OutTextXY(30.0f, y += dy, "VIEW: MODELS ANIM (use o, p, [, ], w, s, space, v and arrows)");
            break;
    };
}
