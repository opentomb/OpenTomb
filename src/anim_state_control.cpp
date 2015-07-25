
#include <cstdlib>
#include <cstdio>

#include "vmath.h"
#include "polygon.h"
#include "engine.h"
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
#include "character_controller.h"
#include "vt/tr_versions.h"
#include "resource.h"

/*
 * WALL CLIMB:
 * preframe do {save pos}
 * postframe do {if(out) {load pos; do command;}}
 */

#define LEFT_LEG                    (3)
#define RIGHT_LEG                   (6)

#define PENETRATION_TEST_OFFSET     (48.0)        ///@TODO: tune it!
#define WALK_FORWARD_OFFSET         (96.0)        ///@FIXME: find real offset
#define WALK_BACK_OFFSET            (16.0)
#define WALK_FORWARD_STEP_UP        (256.0)       // by bone frame bb
#define RUN_FORWARD_OFFSET          (128.0)       ///@FIXME: find real offset
#define RUN_FORWARD_STEP_UP         (320.0)       // by bone frame bb
#define CRAWL_FORWARD_OFFSET        (256.0)
#define LARA_HANG_WALL_DISTANCE     (128.0 - 24.0)
#define LARA_HANG_VERTICAL_EPSILON  (64.0)
#define LARA_HANG_VERTICAL_OFFSET   (12.0)        // in original is 0, in real life hands are little more higher than edge
#define LARA_TRY_HANG_WALL_OFFSET   (72.0)        // It works more stable than 32 or 128
#define LARA_HANG_SENSOR_Z          (800.0)       // It works more stable than 1024 (after collision critical fix, of course)

#define OSCILLATE_HANG_USE 0

void ent_stop_traverse(Character* ent, SSAnimation* ss_anim, int state)
{
    if(state == ENTITY_ANIM_NEWANIM)
    {
        btVector3& v = ent->m_traversedObject->m_transform.getOrigin();
        int i = v[0] / TR_METERING_SECTORSIZE;
        v[0] = i * TR_METERING_SECTORSIZE + 512.0;
        i = v[1] / TR_METERING_SECTORSIZE;
        v[1] = i * TR_METERING_SECTORSIZE + 512.0;
        ent->m_traversedObject->updateRigidBody(true);
        ent->m_traversedObject = NULL;
        ss_anim->onFrame = nullptr;
    }
}

void ent_set_on_floor(Character* ent, SSAnimation* ss_anim, int state)
{
    if(state == ENTITY_ANIM_NEWANIM)
    {
        ent->m_moveType = MOVE_ON_FLOOR;
        ent->m_transform.getOrigin()[2] = ent->m_heightInfo.floor_point[2];
        ent->ghostUpdate();
        ss_anim->onFrame = nullptr;
    }
}

void ent_set_turn_fast(std::shared_ptr<Entity> ent, SSAnimation* ss_anim, int state)
{
    if(state == ENTITY_ANIM_NEWANIM)
    {
        ent->m_bf.animations.next_state = TR_STATE_LARA_TURN_FAST;
        ss_anim->onFrame = nullptr;
    }
}

void ent_set_on_floor_after_climb(Character* ent, SSAnimation* ss_anim, int state)
{
    if(state == ENTITY_ANIM_NEWANIM)
    {
        AnimationFrame* af = &ss_anim->model->animations[ ss_anim->current_animation ];
        if(ss_anim->current_frame >= static_cast<int>(af->frames.size() - 1))
        {
            auto move = ent->m_transform * ent->m_bf.bone_tags[0].full_transform.getOrigin();
            ent->setAnimation(af->next_anim->id, af->next_frame);
            auto p = ent->m_transform * ent->m_bf.bone_tags[0].full_transform.getOrigin();
            move -= p;
            ent->m_transform.getOrigin() += move;
            ent->m_transform.getOrigin()[2] = ent->m_climb.point[2];
            Entity::updateCurrentBoneFrame(&ent->m_bf, &ent->m_transform);
            ent->updateRigidBody(false);
            ent->ghostUpdate();
            ent->m_moveType = MOVE_ON_FLOOR;
            ss_anim->onFrame = nullptr;
        }
    }
}

void ent_set_underwater(Character* ent, SSAnimation* ss_anim, int state)
{
    if(state == ENTITY_ANIM_NEWANIM)
    {
        ent->m_moveType = MOVE_UNDERWATER;
        ss_anim->onFrame = nullptr;
    }
}

void ent_set_free_falling(Character* ent, SSAnimation* ss_anim, int state)
{
    if(state == ENTITY_ANIM_NEWANIM)
    {
        ent->m_moveType = MOVE_FREE_FALLING;
        ss_anim->onFrame = nullptr;
    }
}

void ent_set_cmd_slide(Character* ent, SSAnimation* ss_anim, int state)
{
    if(state == ENTITY_ANIM_NEWANIM)
    {
        ent->m_response.slide = CHARACTER_SLIDE_BACK;
        ss_anim->onFrame = nullptr;
    }
}

void ent_correct_diving_angle(Character* ent, SSAnimation* ss_anim, int state)
{
    if(state == ENTITY_ANIM_NEWANIM)
    {
        ent->m_angles[1] = -45.0;
        ent->updateTransform();
        ss_anim->onFrame = nullptr;
    }
}

void ent_to_on_water(Character* ent, SSAnimation* ss_anim, int state)
{
    if(state == ENTITY_ANIM_NEWANIM)
    {
        ent->m_transform.getOrigin()[2] = ent->m_heightInfo.transition_level;
        ent->ghostUpdate();
        ent->m_moveType = MOVE_ON_WATER;
        ss_anim->onFrame = nullptr;
    }
}

void ent_climb_out_of_water(Character* ent, SSAnimation* ss_anim, int state)
{
    if(state == ENTITY_ANIM_NEWANIM)
    {
        const btVector3& v = ent->m_climb.point;

        ent->m_transform.getOrigin() = v + ent->m_transform.getBasis().getColumn(1) * 48.0;             // temporary stick
        ent->m_transform.getOrigin()[2] = v[2];
        ent->ghostUpdate();
        ss_anim->onFrame = nullptr;
    }
}

void ent_to_edge_climb(Character* ent, SSAnimation* ss_anim, int state)
{
    if(state == ENTITY_ANIM_NEWANIM)
    {
        btScalar *v = ent->m_climb.point;

        ent->m_transform.getOrigin()[0] = v[0] - ent->m_transform.getBasis().getColumn(1)[0] * ent->m_bf.bb_max[1];
        ent->m_transform.getOrigin()[1] = v[1] - ent->m_transform.getBasis().getColumn(1)[1] * ent->m_bf.bb_max[1];
        ent->m_transform.getOrigin()[2] = v[2] - ent->m_bf.bb_max[2];
        ent->ghostUpdate();
        ss_anim->onFrame = nullptr;
    }
}

void ent_to_monkey_swing(Character* ent, SSAnimation* ss_anim, int state)
{
    if(state == ENTITY_ANIM_NEWANIM)
    {
        ent->m_moveType = MOVE_MONKEYSWING;
        ent->m_transform.getOrigin()[2] = ent->m_heightInfo.ceiling_point[2] - ent->m_bf.bb_max[2];
        ent->ghostUpdate();
        ss_anim->onFrame = nullptr;
    }
}

void ent_crawl_to_climb(Character* ent, SSAnimation* ss_anim, int state)
{
    if(state == ENTITY_ANIM_NEWANIM)
    {
        CharacterCommand* cmd = &ent->m_command;

        if(!cmd->action)
        {
            ent->setAnimation(TR_ANIMATION_LARA_START_FREE_FALL, 0);
            ent->m_moveType = MOVE_FREE_FALLING;
            ent->m_dirFlag = ENT_MOVE_BACKWARD;
        }
        else
        {
            ent->setAnimation(TR_ANIMATION_LARA_HANG_IDLE, -1);
        }

        ent->m_bt.no_fix_all = false;
        ent_to_edge_climb(ent, ss_anim, state);
        ss_anim->onFrame = nullptr;
    }
}

/**
 * Current animation != current state - use original TR state concept!
 */
int State_Control_Lara(Character* character, struct SSAnimation *ss_anim)
{
    btVector3& pos = character->m_transform.getOrigin();
    btScalar t;
    btVector3 global_offset, move;
    CharacterCommand* cmd = &character->m_command;
    CharacterResponse* resp = &character->m_response;

    HeightInfo* curr_fc = &character->m_heightInfo;
    HeightInfo next_fc;
    next_fc.sp = curr_fc->sp;
    next_fc.cb = character->m_rayCb;
    next_fc.cb->m_closestHitFraction = 1.0;
    next_fc.cb->m_collisionObject = nullptr;
    next_fc.ccb = character->m_convexCb;
    next_fc.ccb->m_closestHitFraction = 1.0;
    next_fc.ccb->m_hitCollisionObject = nullptr;
    character->m_bt.no_fix_body_parts = 0x00000000;

    ss_anim->anim_flags = ANIM_NORMAL_CONTROL;
    character->updateCurrentHeight();

    bool low_vertical_space = (curr_fc->floor_hit && curr_fc->ceiling_hit && (curr_fc->ceiling_point[2] - curr_fc->floor_point[2] < character->m_height - LARA_HANG_VERTICAL_EPSILON));
    const bool last_frame = static_cast<int>( ss_anim->model->animations[ss_anim->current_animation].frames.size() ) <= ss_anim->current_frame + 1;

    if(resp->kill == 1)   // Stop any music, if Lara is dead.
    {
        Audio_EndStreams(TR_AUDIO_STREAM_TYPE_ONESHOT);
        Audio_EndStreams(TR_AUDIO_STREAM_TYPE_CHAT);
    }

 /*
 * - On floor animations
 * - Climbing animations
 * - Landing animations
 * - Free fall animations
 * - Water animations
 */
    switch(ss_anim->last_state)
    {
        /*
         * Base onfloor animations
         */
        case TR_STATE_LARA_STOP:
            // Reset directional flag only on intermediate animation!

            if(ss_anim->current_animation == TR_ANIMATION_LARA_STAY_SOLID)
            {
                character->m_dirFlag = ENT_STAY;
            }

            cmd->rot[0] = 0;
            cmd->crouch |= low_vertical_space;
            character->lean(cmd, 0.0);

            if( (character->m_climb.can_hang &&
                (character->m_climb.next_z_space >= character->m_height - LARA_HANG_VERTICAL_EPSILON) &&
                (character->m_moveType == MOVE_CLIMBING)) ||
                (ss_anim->current_animation == TR_ANIMATION_LARA_STAY_SOLID) )
            {
                character->m_moveType = MOVE_ON_FLOOR;
            }

            if(character->m_moveType == MOVE_ON_FLOOR)
            {
                character->m_bt.no_fix_body_parts = BODY_PART_LEGS_2 | BODY_PART_LEGS_3;
            }

            if(character->m_moveType == MOVE_FREE_FALLING)
            {
                character->setAnimation(TR_ANIMATION_LARA_START_FREE_FALL, 0);
                character->m_dirFlag = ENT_STAY;
            }
            else if(resp->kill == 1)
            {
                ss_anim->next_state = TR_STATE_LARA_DEATH;
            }
            else if(resp->slide == CHARACTER_SLIDE_FRONT)
            {
                Audio_Send(TR_AUDIO_SOUND_LANDING, TR_AUDIO_EMITTER_ENTITY, character->id());

                if(cmd->jump)
                {
                    character->m_dirFlag = ENT_MOVE_FORWARD;
                    character->setAnimation(TR_ANIMATION_LARA_JUMP_FORWARD_BEGIN, 0);
                }
                else
                {
                    character->setAnimation(TR_ANIMATION_LARA_SLIDE_FORWARD, 0);
                }
            }
            else if(resp->slide == CHARACTER_SLIDE_BACK)
            {
                if(cmd->jump)
                {
                    character->m_dirFlag = ENT_MOVE_BACKWARD;
                    character->setAnimation(TR_ANIMATION_LARA_JUMP_BACK_BEGIN, 0);
                    Audio_Send(TR_AUDIO_SOUND_LANDING, TR_AUDIO_EMITTER_ENTITY, character->id());
                }
                else
                {
                    character->setAnimation(TR_ANIMATION_LARA_START_SLIDE_BACKWARD, 0);
                }
            }
            else if(cmd->jump)
            {
                if(!curr_fc->quicksand)
                    ss_anim->next_state = TR_STATE_LARA_JUMP_PREPARE;       // jump sideways
            }
            else if(cmd->roll)
            {
                if(!curr_fc->quicksand && ss_anim->current_animation != TR_ANIMATION_LARA_CLIMB_2CLICK)
                {
                    character->m_dirFlag = ENT_MOVE_FORWARD;
                    character->setAnimation(TR_ANIMATION_LARA_ROLL_BEGIN, 0);
                }
            }
            else if(cmd->crouch)
            {
                if(!curr_fc->quicksand)
                    ss_anim->next_state = TR_STATE_LARA_CROUCH_IDLE;
            }
            else if(cmd->action && character->findTraverse())
            {
                ss_anim->next_state = TR_STATE_LARA_PUSHABLE_GRAB;
                if(character->m_transform.getBasis().getColumn(1).x() > 0.9)
                {
                    t = -character->m_traversedObject->m_bf.bb_min[0] + 72.0;
                }
                else if(character->m_transform.getBasis().getColumn(1).x() < -0.9)
                {
                    t = character->m_traversedObject->m_bf.bb_max[0] + 72.0;
                }
                else if(character->m_transform.getBasis().getColumn(1).y() > 0.9)
                {
                    t = -character->m_traversedObject->m_bf.bb_min[1] + 72.0;
                }
                else if(character->m_transform.getBasis().getColumn(1).y() < -0.9)
                {
                    t = character->m_traversedObject->m_bf.bb_max[1] + 72.0;
                }
                else
                {
                    t = 512.0 + 72.0;                                           ///@PARANOID
                }
                const btVector3& v = character->m_traversedObject->m_transform.getOrigin();
                pos[0] = v[0] - character->m_transform.getBasis().getColumn(1).x() * t;
                pos[1] = v[1] - character->m_transform.getBasis().getColumn(1).y() * t;
            }
            else if(cmd->move[0] == 1)
            {
                if(cmd->shift)
                {
                    move = character->m_transform.getBasis().getColumn(1) * PENETRATION_TEST_OFFSET;
                    global_offset = character->m_transform.getBasis().getColumn(1) * WALK_FORWARD_OFFSET;
                    global_offset[2] += character->m_bf.bb_max[2];
                    global_offset += pos;
                    Character::getHeightInfo(global_offset, &next_fc);
                    if(((character->checkNextPenetration(move) == 0) || (character->m_response.horizontal_collide == 0x00)) &&
                       (next_fc.floor_hit && (next_fc.floor_point[2] > pos[2] - character->m_maxStepUpHeight) && (next_fc.floor_point[2] <= pos[2] + character->m_maxStepUpHeight)))
                    {
                        character->m_moveType = MOVE_ON_FLOOR;
                        character->m_dirFlag = ENT_MOVE_FORWARD;
                        if((curr_fc->water || curr_fc->quicksand) && curr_fc->floor_hit && (curr_fc->transition_level - curr_fc->floor_point[2] > character->m_wadeDepth))
                        {
                            ss_anim->next_state = TR_STATE_LARA_WADE_FORWARD;
                        }
                        else
                        {
                            ss_anim->next_state = TR_STATE_LARA_WALK_FORWARD;
                        }
                    }
                }       // end IF CMD->SHIFT
                else
                {
                    move = character->m_transform.getBasis().getColumn(1) * PENETRATION_TEST_OFFSET;
                    global_offset = character->m_transform.getBasis().getColumn(1) * RUN_FORWARD_OFFSET;
                    global_offset[2] += character->m_bf.bb_max[2];
                    character->checkNextStep(global_offset, &next_fc);
                    if(((character->checkNextPenetration(move) == 0) || (character->m_response.horizontal_collide == 0x00)) && !character->hasStopSlant(next_fc))
                    {
                        character->m_moveType = MOVE_ON_FLOOR;
                        character->m_dirFlag = ENT_MOVE_FORWARD;
                        if((curr_fc->water || curr_fc->quicksand) && curr_fc->floor_hit && (curr_fc->transition_level - curr_fc->floor_point[2] > character->m_wadeDepth))
                        {
                            ss_anim->next_state = TR_STATE_LARA_WADE_FORWARD;
                        }
                        else
                        {
                            ss_anim->next_state = TR_STATE_LARA_RUN_FORWARD;
                        }
                    }
                }

                if( cmd->action &&
                    ((ss_anim->current_animation == TR_ANIMATION_LARA_STAY_IDLE)        ||
                     (ss_anim->current_animation == TR_ANIMATION_LARA_STAY_SOLID)       ||
                     (ss_anim->current_animation == TR_ANIMATION_LARA_WALL_SMASH_LEFT)  ||
                     (ss_anim->current_animation == TR_ANIMATION_LARA_WALL_SMASH_RIGHT)) )
                {
                    t = character->m_forwardSize + LARA_TRY_HANG_WALL_OFFSET;
                    global_offset = character->m_transform.getBasis().getColumn(1) * t;

                    global_offset[2] += 0.5 * DEFAULT_CLIMB_UP_HEIGHT;
                    character->m_climb = character->checkClimbability(global_offset, &next_fc, 0.5 * DEFAULT_CLIMB_UP_HEIGHT);
                    if(  character->m_climb.edge_hit != ClimbType::NoClimb                                       &&
                        (character->m_climb.next_z_space >= character->m_height - LARA_HANG_VERTICAL_EPSILON)    &&
                        (pos[2] + character->m_maxStepUpHeight < next_fc.floor_point[2])             &&
                        (pos[2] + 2944.0 >= next_fc.floor_point[2])                                  &&
                        (next_fc.floor_normale[2] >= character->m_criticalSlantZComponent)  ) // trying to climb on
                    {
                        if(pos[2] + 640.0 >= next_fc.floor_point[2])
                        {
                            character->m_angles[0] = character->m_climb.edge_z_ang;
                            pos[2] = next_fc.floor_point[2] - 512.0;
                            character->m_climb.point = next_fc.floor_point;
                            character->setAnimation(TR_ANIMATION_LARA_CLIMB_2CLICK, 0);
                            character->m_bt.no_fix_all = true;
                            ss_anim->onFrame = ent_set_on_floor_after_climb;
                            break;
                        }
                        else if(pos[2] + 896.0 >= next_fc.floor_point[2])
                        {
                            character->m_angles[0] = character->m_climb.edge_z_ang;
                            pos[2] = next_fc.floor_point[2] - 768.0;
                            character->m_climb.point = next_fc.floor_point;
                            character->setAnimation(TR_ANIMATION_LARA_CLIMB_3CLICK, 0);
                            character->m_bt.no_fix_all = true;
                            ss_anim->onFrame = ent_set_on_floor_after_climb;
                            break;
                        }
                    }   // end IF MOVE_LITTLE_CLIMBING

                    global_offset[2] += 0.5 * DEFAULT_CLIMB_UP_HEIGHT;
                    character->m_climb = character->checkClimbability(global_offset, &next_fc, DEFAULT_CLIMB_UP_HEIGHT);
                    if(  character->m_climb.edge_hit != ClimbType::NoClimb                                       &&
                        (character->m_climb.next_z_space >= character->m_height - LARA_HANG_VERTICAL_EPSILON)    &&
                        (pos[2] + character->m_maxStepUpHeight < next_fc.floor_point[2])             &&
                        (pos[2] + 2944.0 >= next_fc.floor_point[2])                                  &&
                        (next_fc.floor_normale[2] >= character->m_criticalSlantZComponent)  ) // trying to climb on
                    {
                        if(pos[2] + 1920.0 >= next_fc.floor_point[2])
                        {
                            ss_anim->next_state = TR_STATE_LARA_JUMP_UP;
                            break;
                        }
                    }   // end IF MOVE_BIG_CLIMBING

                    character->m_climb = character->checkWallsClimbability();
                    if(character->m_climb.wall_hit != ClimbType::NoClimb)
                    {
                        ss_anim->next_state = TR_STATE_LARA_JUMP_UP;
                        break;
                    }
                }
            }       // end CMD->MOVE FORWARD
            else if(cmd->move[0] == -1)
            {
                if(cmd->shift)
                {
                    move = character->m_transform.getBasis().getColumn(1) * -PENETRATION_TEST_OFFSET;
                    if((character->checkNextPenetration(move) == 0) || (character->m_response.horizontal_collide == 0x00))
                    {
                        global_offset = character->m_transform.getBasis().getColumn(1) * -WALK_BACK_OFFSET;
                        global_offset[2] += character->m_bf.bb_max[2];
                        global_offset += pos;
                        Character::getHeightInfo(global_offset, &next_fc);
                        if((next_fc.floor_hit && (next_fc.floor_point[2] > pos[2] - character->m_maxStepUpHeight) && (next_fc.floor_point[2] <= pos[2] + character->m_maxStepUpHeight)))
                        {
                            character->m_dirFlag = ENT_MOVE_BACKWARD;
                            ss_anim->next_state = TR_STATE_LARA_WALK_BACK;
                        }
                    }
                }
                else    // RUN BACK
                {
                    move = character->m_transform.getBasis().getColumn(1) * -PENETRATION_TEST_OFFSET;
                    if((character->checkNextPenetration(move) == 0) || (character->m_response.horizontal_collide == 0x00))
                    {
                        character->m_dirFlag = ENT_MOVE_BACKWARD;
                        if((curr_fc->water || curr_fc->quicksand) && curr_fc->floor_hit && (curr_fc->transition_level - curr_fc->floor_point[2] > character->m_wadeDepth))
                        {
                            ss_anim->next_state = TR_STATE_LARA_WALK_BACK;
                        }
                        else
                        {
                            ss_anim->next_state = TR_STATE_LARA_RUN_BACK;
                        }
                    }
                }
            }       // end CMD->MOVE BACK
            else if(cmd->move[1] == 1)
            {
                if(cmd->shift)
                {
                    move = character->m_transform.getBasis().getColumn(0) * PENETRATION_TEST_OFFSET;
                    if((character->checkNextPenetration(move) == 0) || (character->m_response.horizontal_collide == 0x00))
                    {
                        global_offset = character->m_transform.getBasis().getColumn(0) * RUN_FORWARD_OFFSET;
                        global_offset[2] += character->m_bf.bb_max[2];
                        NextStepInfo i = character->checkNextStep(global_offset, &next_fc);
                        if((resp->horizontal_collide == 0) && (i >= NextStepInfo::DownLittle && i <= NextStepInfo::UpLittle))
                        {
                            cmd->rot[0] = 0.0;
                            character->m_dirFlag = ENT_MOVE_RIGHT;
                            ss_anim->next_state = TR_STATE_LARA_WALK_RIGHT;
                        }
                    }
                }       //end IF CMD->SHIFT
                else
                {
                    ss_anim->next_state = TR_STATE_LARA_TURN_RIGHT_SLOW;
                }
            }       // end MOVE RIGHT
            else if(cmd->move[1] == -1)
            {
                if(cmd->shift)
                {
                    move = character->m_transform.getBasis().getColumn(0) * -PENETRATION_TEST_OFFSET;
                    if((character->checkNextPenetration(move) == 0) || (character->m_response.horizontal_collide == 0x00))
                    {
                        global_offset = character->m_transform.getBasis().getColumn(0) * -RUN_FORWARD_OFFSET;
                        global_offset[2] += character->m_bf.bb_max[2];
                        NextStepInfo i = character->checkNextStep(global_offset, &next_fc);
                        if((resp->horizontal_collide == 0) && (i >= NextStepInfo::DownLittle && i <= NextStepInfo::UpLittle))
                        {
                            cmd->rot[0] = 0.0;
                            character->m_dirFlag = ENT_MOVE_LEFT;
                            ss_anim->next_state = TR_STATE_LARA_WALK_LEFT;
                        }
                    }
                }       //end IF CMD->SHIFT
                else
                {
                    ss_anim->next_state = TR_STATE_LARA_TURN_LEFT_SLOW;
                }
            }       // end MOVE LEFT
            break;

        case TR_STATE_LARA_JUMP_PREPARE:
            cmd->rot[0] = 0;
            character->lean(cmd, 0.0);

            if(resp->slide == CHARACTER_SLIDE_BACK)      // Slide checking is only for jumps direction correction!
            {
                character->setAnimation(TR_ANIMATION_LARA_JUMP_BACK_BEGIN, 0);
                cmd->move[0] = -1;
            }
            else if(resp->slide == CHARACTER_SLIDE_FRONT)
            {
                character->setAnimation(TR_ANIMATION_LARA_JUMP_FORWARD_BEGIN, 0);
                cmd->move[0] = 1;
            }
            if((curr_fc->water || curr_fc->quicksand) && curr_fc->floor_hit && (curr_fc->transition_level - curr_fc->floor_point[2] > character->m_wadeDepth))
            {
                //Stay, directional jumps are not allowed whilst in wade depth
            }
            else if(cmd->move[0] == 1)
            {
                character->m_dirFlag = ENT_MOVE_FORWARD;
                move = character->m_transform.getBasis().getColumn(1) * PENETRATION_TEST_OFFSET;
                if(character->checkNextPenetration(move) == 0)
                {
                    ss_anim->next_state = TR_STATE_LARA_JUMP_FORWARD;           // jump forward
                }
            }
            else if(cmd->move[0] ==-1)
            {
                character->m_dirFlag = ENT_MOVE_BACKWARD;
                move = character->m_transform.getBasis().getColumn(1) * -PENETRATION_TEST_OFFSET;
                if(character->checkNextPenetration(move) == 0)
                {
                    ss_anim->next_state = TR_STATE_LARA_JUMP_BACK;              // jump backward
                }
            }
            else if(cmd->move[1] == 1)
            {
                character->m_dirFlag = ENT_MOVE_RIGHT;
                move = character->m_transform.getBasis().getColumn(0) * PENETRATION_TEST_OFFSET;
                if(character->checkNextPenetration(move) == 0)
                {
                    ss_anim->next_state = TR_STATE_LARA_JUMP_LEFT;              // jump right
                }
            }
            else if(cmd->move[1] ==-1)
            {
                character->m_dirFlag = ENT_MOVE_LEFT;
                move = character->m_transform.getBasis().getColumn(0) * -PENETRATION_TEST_OFFSET;
                if(character->checkNextPenetration(move) == 0)
                {
                    ss_anim->next_state = TR_STATE_LARA_JUMP_RIGHT;             // jump left
                }
            }
            break;

        case TR_STATE_LARA_JUMP_BACK:
            cmd->rot[0] = 0.0;
            if(resp->vertical_collide & 0x01 || character->m_moveType == MOVE_ON_FLOOR)
            {
                if(curr_fc->quicksand)
                {
                    character->setAnimation(TR_ANIMATION_LARA_STAY_IDLE, 0);
                }
                else
                {
                    ss_anim->next_state = TR_STATE_LARA_STOP;       // landing
                }
            }
            else if(resp->horizontal_collide & 0x01)
            {
                Controls_JoyRumble(200.0, 200);
                character->setAnimation(TR_ANIMATION_LARA_SMASH_JUMP, 0);
                character->m_dirFlag = ENT_MOVE_FORWARD;
                character->updateCurrentSpeed(true);
            }
            else if((character->m_moveType == MOVE_UNDERWATER) || (character->m_speed[2] <= -FREE_FALL_SPEED_2))
            {
                ss_anim->next_state = TR_STATE_LARA_FREEFALL;                   // free falling
            }
            else if(cmd->roll)
            {
                ss_anim->next_state = TR_STATE_LARA_JUMP_ROLL;
            }
            break;

        case TR_STATE_LARA_JUMP_LEFT:
            cmd->rot[0] = 0.0;
            if(resp->vertical_collide & 0x01 || character->m_moveType == MOVE_ON_FLOOR)
            {
                if(curr_fc->quicksand)
                {
                    character->setAnimation(TR_ANIMATION_LARA_STAY_IDLE, 0);
                }
                else
                {
                    ss_anim->next_state = TR_STATE_LARA_STOP;       // landing
                }
            }
            else if(resp->horizontal_collide & 0x01)
            {
                Controls_JoyRumble(200.0, 200);
                character->setAnimation(TR_ANIMATION_LARA_SMASH_JUMP, 0);
                character->m_dirFlag = ENT_MOVE_RIGHT;
                character->updateCurrentSpeed(true);
            }
            else
            {
                ss_anim->next_state = TR_STATE_LARA_FREEFALL;
            }
            break;

        case TR_STATE_LARA_JUMP_RIGHT:
            cmd->rot[0] = 0.0;
            if(resp->vertical_collide & 0x01 || character->m_moveType == MOVE_ON_FLOOR)
            {
                if(curr_fc->quicksand)
                {
                    character->setAnimation(TR_ANIMATION_LARA_STAY_IDLE, 0);
                }
                else
                {
                    ss_anim->next_state = TR_STATE_LARA_STOP;       // landing
                }
            }
            else if(resp->horizontal_collide & 0x01)
            {
                Controls_JoyRumble(200.0, 200);
                character->setAnimation(TR_ANIMATION_LARA_SMASH_JUMP, 0);
                character->m_dirFlag = ENT_MOVE_LEFT;
                character->updateCurrentSpeed(true);
            }
            else
            {
                ss_anim->next_state = TR_STATE_LARA_FREEFALL;
            }
            break;

        case TR_STATE_LARA_RUN_BACK:
            character->m_dirFlag = ENT_MOVE_BACKWARD;

            if(character->m_moveType == MOVE_FREE_FALLING)
            {
                character->m_dirFlag = ENT_MOVE_FORWARD;
                character->setAnimation(TR_ANIMATION_LARA_FREE_FALL_BACK, 0);
            }
            else if(resp->horizontal_collide & 0x01)
            {
                character->setAnimation(TR_ANIMATION_LARA_CLIMB_2CLICK_END, 0);
            }
            break;


        case TR_STATE_LARA_TURN_LEFT_SLOW:
        case TR_STATE_LARA_TURN_RIGHT_SLOW:
            cmd->rot[0] *= 0.7;
            character->m_dirFlag = ENT_STAY;
            character->lean(cmd, 0.0);
            character->m_bt.no_fix_body_parts = BODY_PART_LEGS_2 | BODY_PART_LEGS_3;

            if(cmd->move[0] == 1)
            {
                Substance substance_state = character->getSubstanceState();
                if((substance_state == Substance::None) ||
                   (substance_state == Substance::WaterShallow))
                {
                    if(cmd->shift)
                    {
                        ss_anim->next_state = TR_STATE_LARA_WALK_FORWARD;
                        character->m_dirFlag = ENT_MOVE_FORWARD;
                    }
                    else
                    {
                        ss_anim->next_state = TR_STATE_LARA_RUN_FORWARD;
                        character->m_dirFlag = ENT_MOVE_FORWARD;
                    }
                }
                else
                {
                    ss_anim->next_state = TR_STATE_LARA_WADE_FORWARD;
                    character->m_dirFlag = ENT_MOVE_FORWARD;
                }

            }
            else if(((ss_anim->last_state == TR_STATE_LARA_TURN_LEFT_SLOW ) && (cmd->move[1] == -1)) ||
                    ((ss_anim->last_state == TR_STATE_LARA_TURN_RIGHT_SLOW) && (cmd->move[1] ==  1))  )
            {
                Substance substance_state = character->getSubstanceState();
                if(last_frame &&
                   (substance_state != Substance::WaterWade) &&
                   (substance_state != Substance::QuicksandConsumed) &&
                   (substance_state != Substance::QuicksandShallow))
                 {
                     ss_anim->next_state = TR_STATE_LARA_TURN_FAST;
                 }
            }
            else
            {
                ss_anim->next_state = TR_STATE_LARA_STOP;
            }
            break;

        case TR_STATE_LARA_TURN_FAST:
            // 65 - wade
            character->m_dirFlag = ENT_STAY;
            character->m_bt.no_fix_body_parts = BODY_PART_LEGS_2 | BODY_PART_LEGS_3;
            character->lean(cmd, 0.0);

            if(character->m_moveType == MOVE_FREE_FALLING)
            {
                character->setAnimation(TR_ANIMATION_LARA_START_FREE_FALL, 0);
            }
            else if(cmd->move[0] == 1 && !cmd->jump && !cmd->crouch && cmd->shift)
            {
                ss_anim->next_state = TR_STATE_LARA_WALK_FORWARD;
                character->m_dirFlag = ENT_MOVE_FORWARD;
            }
            else if(cmd->move[0] == 1 && !cmd->jump && !cmd->crouch && !cmd->shift)
            {
                ss_anim->next_state = TR_STATE_LARA_RUN_FORWARD;
                character->m_dirFlag = ENT_MOVE_FORWARD;
            }
            else if(cmd->move[1] == 0)
            {
                ss_anim->next_state = TR_STATE_LARA_STOP;
            }
            break;

            /*
             * RUN AND WALK animations section
             */
        case TR_STATE_LARA_RUN_FORWARD:
        {
            global_offset = character->m_transform.getBasis().getColumn(1) * RUN_FORWARD_OFFSET;
            global_offset[2] += character->m_bf.bb_max[2];
            NextStepInfo i = character->checkNextStep(global_offset, &next_fc);
            character->m_dirFlag = ENT_MOVE_FORWARD;
            cmd->crouch |= low_vertical_space;

            if(character->m_moveType == MOVE_ON_FLOOR)
            {
                character->m_bt.no_fix_body_parts = BODY_PART_HANDS | BODY_PART_LEGS;;
            }
            character->lean(cmd, 6.0);

            if(character->m_moveType == MOVE_FREE_FALLING)
            {
                character->setAnimation(TR_ANIMATION_LARA_FREE_FALL_FORWARD, 0);
            }
            else if(resp->kill == 1)
            {
                ss_anim->next_state = TR_STATE_LARA_DEATH;
            }
            else if(resp->slide == CHARACTER_SLIDE_FRONT)
            {
                character->setAnimation(TR_ANIMATION_LARA_SLIDE_FORWARD, 0);
            }
            else if(resp->slide == CHARACTER_SLIDE_BACK)
            {
                character->setAnimation(TR_ANIMATION_LARA_START_SLIDE_BACKWARD, 0);
                character->m_dirFlag = ENT_MOVE_BACKWARD;
            }
            else if(character->hasStopSlant(next_fc))
            {
                character->m_dirFlag = ENT_STAY;
                character->setAnimation(TR_ANIMATION_LARA_STAY_IDLE, 0);
            }
            else if(cmd->crouch)
            {
                ss_anim->next_state = TR_STATE_LARA_CROUCH_IDLE;
            }
            else if((cmd->move[0] == 1) && !cmd->crouch && (next_fc.floor_normale[2] >= character->m_criticalSlantZComponent) && (i == NextStepInfo::UpBig))
            {
                character->m_dirFlag = ENT_STAY;
                int dispCase = character->getAnimDispatchCase(2);                         // MOST CORRECT STATECHANGE!!!
                if(dispCase == 0)
                {
                    character->setAnimation(TR_ANIMATION_LARA_RUN_UP_STEP_RIGHT, 0);
                    pos[2] = next_fc.floor_point[2];
                    character->m_dirFlag = ENT_MOVE_FORWARD;
                }
                else //if(i == 1)
                {
                    character->setAnimation(TR_ANIMATION_LARA_RUN_UP_STEP_LEFT, 0);
                    pos[2] = next_fc.floor_point[2];
                    character->m_dirFlag = ENT_MOVE_FORWARD;
                }
            }
            else if(resp->horizontal_collide & 0x01)
            {
                global_offset = character->m_transform.getBasis().getColumn(1) * RUN_FORWARD_OFFSET;
                global_offset[2] += 1024.0;
                if(ss_anim->current_animation == TR_ANIMATION_LARA_STAY_TO_RUN)
                {
                    character->setAnimation(TR_ANIMATION_LARA_STAY_IDLE, 0);
                }
                else
                {
                    Controls_JoyRumble(200.0, 200);

                    if(cmd->move[0] == 1)
                    {
                        int dispCase = character->getAnimDispatchCase(2);
                        if(dispCase == 1)
                        {
                            character->setAnimation(TR_ANIMATION_LARA_WALL_SMASH_LEFT, 0);
                        }
                        else
                        {
                            character->setAnimation(TR_ANIMATION_LARA_WALL_SMASH_RIGHT, 0);
                        }
                    }
                    else
                    {
                        character->setAnimation(TR_ANIMATION_LARA_STAY_SOLID, 0);
                    }
                }
                character->updateCurrentSpeed(false);
            }
            else if(cmd->move[0] == 1)                                          // If we continue running...
            {
                if((curr_fc->water || curr_fc->quicksand) && curr_fc->floor_hit && (curr_fc->transition_level - curr_fc->floor_point[2] > character->m_wadeDepth))
                {
                    ss_anim->next_state = TR_STATE_LARA_WADE_FORWARD;
                }
                else if(cmd->shift)
                {
                    ss_anim->next_state = TR_STATE_LARA_WALK_FORWARD;
                }
                else if(cmd->jump && (ss_anim->last_animation != TR_ANIMATION_LARA_STAY_TO_RUN))
                {
                    ss_anim->next_state = TR_STATE_LARA_JUMP_FORWARD;
                }
                else if(cmd->roll)
                {
                    character->m_dirFlag = ENT_MOVE_FORWARD;
                    character->setAnimation(TR_ANIMATION_LARA_ROLL_BEGIN, 0);
                }
                else if(cmd->sprint)
                {
                    ss_anim->next_state = TR_STATE_LARA_SPRINT;
                }
            }
            else
            {
                ss_anim->next_state = TR_STATE_LARA_STOP;
            }
            break;
        }

        case TR_STATE_LARA_SPRINT:
        {
            global_offset = character->m_transform.getBasis().getColumn(1) * RUN_FORWARD_OFFSET;
            character->lean(cmd, 12.0);
            global_offset[2] += character->m_bf.bb_max[2];
            NextStepInfo i = character->checkNextStep(global_offset, &next_fc);
            cmd->crouch |= low_vertical_space;

            if(character->m_moveType == MOVE_ON_FLOOR)
            {
                character->m_bt.no_fix_body_parts = BODY_PART_LEGS_1 | BODY_PART_LEGS_2 | BODY_PART_LEGS_3;
            }

            if(!character->getParam(PARAM_STAMINA))
            {
                ss_anim->next_state = TR_STATE_LARA_RUN_FORWARD;
            }
            else if(character->m_moveType == MOVE_FREE_FALLING)
            {
                character->setAnimation(TR_ANIMATION_LARA_FREE_FALL_FORWARD, 0);
            }
            else if(resp->kill == 1)
            {
                ss_anim->next_state = TR_STATE_LARA_RUN_FORWARD;    // Normal run then die
            }
            else if(resp->slide == CHARACTER_SLIDE_FRONT)
            {
                character->setAnimation(TR_ANIMATION_LARA_SLIDE_FORWARD, 0);
            }
            else if(resp->slide == CHARACTER_SLIDE_BACK)
            {
                character->setAnimation(TR_ANIMATION_LARA_START_SLIDE_BACKWARD, 0);
            }
            else if((next_fc.floor_normale[2] < character->m_criticalSlantZComponent) && (i > NextStepInfo::Horizontal))
            {
                character->m_currentSpeed = 0.0;
                character->setAnimation(TR_ANIMATION_LARA_STAY_IDLE, 0);       ///@FIXME: maybe RUN_TO_STAY
            }
            else if((next_fc.floor_normale[2] >= character->m_criticalSlantZComponent) && (i == NextStepInfo::UpBig))
            {
                ss_anim->next_state = TR_STATE_LARA_RUN_FORWARD;     // Interrupt sprint
            }
            else if(resp->horizontal_collide & 0x01)
            {
                Controls_JoyRumble(200.0, 200);

                int dispCase = character->getAnimDispatchCase(2);                         // tested!
                if(dispCase == 1)
                {
                    character->setAnimation(TR_ANIMATION_LARA_WALL_SMASH_LEFT, 0);
                }
                else if(dispCase == 0)
                {
                    character->setAnimation(TR_ANIMATION_LARA_WALL_SMASH_RIGHT, 0);
                }
                character->updateCurrentSpeed(false);
            }
            else if(!cmd->sprint)
            {
                if(cmd->move[0] == 1)
                {
                    ss_anim->next_state = TR_STATE_LARA_RUN_FORWARD;
                }
                else
                {
                    ss_anim->next_state = TR_STATE_LARA_STOP;
                }
            }
            else
            {
                if(cmd->jump == 1)
                {
                    ss_anim->next_state = TR_STATE_LARA_SPRINT_ROLL;
                }
                else if(cmd->roll == 1)
                {
                    character->m_dirFlag = ENT_MOVE_FORWARD;
                    character->setAnimation(TR_ANIMATION_LARA_ROLL_BEGIN, 0);
                }
                else if(cmd->crouch)
                {
                    ss_anim->next_state = TR_STATE_LARA_CROUCH_IDLE;
                }
                else if(cmd->move[0] == 0)
                {
                    ss_anim->next_state = TR_STATE_LARA_STOP;
                }
            }
            break;
        }

        case TR_STATE_LARA_WALK_FORWARD:
        {
            cmd->rot[0] *= 0.4;
            character->lean(cmd, 0.0);

            global_offset = character->m_transform.getBasis().getColumn(1) * WALK_FORWARD_OFFSET;
            global_offset[2] += character->m_bf.bb_max[2];
            NextStepInfo i = character->checkNextStep(global_offset, &next_fc);
            character->m_dirFlag = ENT_MOVE_FORWARD;

            if(character->m_moveType == MOVE_ON_FLOOR)
            {
                character->m_bt.no_fix_body_parts = BODY_PART_LEGS_1 | BODY_PART_LEGS_2 | BODY_PART_LEGS_3;
            }

            if(character->m_moveType == MOVE_FREE_FALLING)
            {
                character->setAnimation(TR_ANIMATION_LARA_START_FREE_FALL, 0);
            }
            else if(resp->kill == 1)
            {
                ss_anim->next_state = TR_STATE_LARA_STOP;
            }
            else if((next_fc.floor_normale[2] >= character->m_criticalSlantZComponent) && (i == NextStepInfo::UpBig))
            {
                /*
                 * climb up
                 */
                character->m_dirFlag = ENT_STAY;
                int dispCase = character->getAnimDispatchCase(2);
                if(dispCase == 1)
                {
                    character->setAnimation(TR_ANIMATION_LARA_WALK_UP_STEP_RIGHT, 0);
                    pos = next_fc.floor_point;
                    character->m_moveType = MOVE_ON_FLOOR;
                    character->m_dirFlag = ENT_MOVE_FORWARD;
                }
                else
                {
                    character->setAnimation(TR_ANIMATION_LARA_WALK_UP_STEP_LEFT, 0);
                    pos = next_fc.floor_point;
                    character->m_moveType = MOVE_ON_FLOOR;
                    character->m_dirFlag = ENT_MOVE_FORWARD;
                }
            }
            else if((next_fc.floor_normale[2] >= character->m_criticalSlantZComponent) && (i == NextStepInfo::DownBig))
            {
                /*
                 * climb down
                 */
                character->m_dirFlag = ENT_STAY;
                int dispCase = character->getAnimDispatchCase(2);
                if(dispCase == 1)
                {
                    character->setAnimation(TR_ANIMATION_LARA_WALK_DOWN_RIGHT, 0);
                    character->m_climb.point = next_fc.floor_point;
                    pos = next_fc.floor_point;
                    character->m_moveType = MOVE_ON_FLOOR;
                    character->m_dirFlag = ENT_MOVE_FORWARD;
                }
                else //if(i == 0)
                {
                    character->setAnimation(TR_ANIMATION_LARA_WALK_DOWN_LEFT, 0);
                    character->m_climb.point = next_fc.floor_point;
                    pos = next_fc.floor_point;
                    character->m_moveType = MOVE_ON_FLOOR;
                    character->m_dirFlag = ENT_MOVE_FORWARD;
                }
            }
            else if((resp->horizontal_collide & 0x01) || (i < NextStepInfo::DownBig || i > NextStepInfo::UpBig) || (low_vertical_space))
            {
                /*
                 * too high
                 */
                character->m_dirFlag = ENT_STAY;
                character->setAnimation(TR_ANIMATION_LARA_STAY_IDLE, 0);
            }
            else if(cmd->move[0] != 1)
            {
                ss_anim->next_state = TR_STATE_LARA_STOP;
            }
            else if((curr_fc->water || curr_fc->quicksand) && curr_fc->floor_hit && (curr_fc->transition_level - curr_fc->floor_point[2] > character->m_wadeDepth))
            {
                ss_anim->next_state = TR_STATE_LARA_WADE_FORWARD;
            }
            else if(cmd->move[0] == 1 && !cmd->crouch && !cmd->shift)
            {
                ss_anim->next_state = TR_STATE_LARA_RUN_FORWARD;
            }
            break;
        }


        case TR_STATE_LARA_WADE_FORWARD:
            cmd->rot[0] *= 0.4;
            character->m_dirFlag = ENT_MOVE_FORWARD;

            if(character->m_heightInfo.quicksand)
            {
                character->m_currentSpeed = (character->m_currentSpeed > MAX_SPEED_QUICKSAND)?MAX_SPEED_QUICKSAND:character->m_currentSpeed;
            }

            if(cmd->move[0] == 1)
            {
                move = character->m_transform.getBasis().getColumn(1) * PENETRATION_TEST_OFFSET;
                character->checkNextPenetration(move);
            }

            if(resp->kill == 1)
            {
                ss_anim->next_state = TR_STATE_LARA_STOP;
            }

            if(!curr_fc->floor_hit || character->m_moveType == MOVE_FREE_FALLING)      // free fall, next swim
            {
                character->setAnimation(TR_ANIMATION_LARA_START_FREE_FALL, 0);
            }
            else if(curr_fc->water)
            {
                if((curr_fc->transition_level - curr_fc->floor_point[2] <= character->m_wadeDepth))
                {
                    // run / walk case
                    if((cmd->move[0] == 1) && (resp->horizontal_collide == 0))
                    {
                        ss_anim->next_state = TR_STATE_LARA_RUN_FORWARD;
                    }
                    else
                    {
                        ss_anim->next_state = TR_STATE_LARA_STOP;
                    }
                }
                else if(curr_fc->transition_level - curr_fc->floor_point[2] > (character->m_height - character->m_swimDepth))
                {
                    // swim case
                    if(curr_fc->transition_level - curr_fc->floor_point[2] > character->m_height + character->m_maxStepUpHeight)
                    {
                        character->setAnimation(TR_ANIMATION_LARA_START_FREE_FALL, 0);                                    // swim underwater
                    }
                    else
                    {
                        character->setAnimation(TR_ANIMATION_LARA_ONWATER_IDLE, 0);                                       // swim onwater
                        character->m_moveType = MOVE_ON_WATER;
                        pos[2] = curr_fc->transition_level;
                    }
                }
                else if(curr_fc->transition_level - curr_fc->floor_point[2] > character->m_wadeDepth)              // wade case
                {
                    if((cmd->move[0] != 1) || (resp->horizontal_collide != 0))
                    {
                        ss_anim->next_state = TR_STATE_LARA_STOP;
                    }
                }
            }
            else                                                                // no water, stay or run / walk
            {
                if((cmd->move[0] == 1) && (resp->horizontal_collide == 0))
                {
                    if(!curr_fc->quicksand)
                    {
                        ss_anim->next_state = TR_STATE_LARA_RUN_FORWARD;
                    }
                }
                else
                {
                    ss_anim->next_state = TR_STATE_LARA_STOP;
                }
            }
            break;

        case TR_STATE_LARA_WALK_BACK:
        {
            cmd->rot[0] *= 0.4;
            character->m_dirFlag = ENT_MOVE_BACKWARD;

            if(character->m_heightInfo.quicksand)
            {
                character->m_currentSpeed = (character->m_currentSpeed > MAX_SPEED_QUICKSAND)?MAX_SPEED_QUICKSAND:character->m_currentSpeed;
            }

            global_offset = character->m_transform.getBasis().getColumn(1) * -WALK_BACK_OFFSET;
            global_offset[2] += character->m_bf.bb_max[2];
            NextStepInfo i = character->checkNextStep(global_offset, &next_fc);
            if(character->m_moveType == MOVE_FREE_FALLING)
            {
                character->setAnimation(TR_ANIMATION_LARA_START_FREE_FALL, 0);
            }
            else if(curr_fc->water && (curr_fc->floor_point[2] + character->m_height < curr_fc->transition_level))
            {
                character->setAnimation(TR_ANIMATION_LARA_ONWATER_SWIM_BACK, 0);
                ss_anim->next_state = TR_STATE_LARA_ONWATER_BACK;
                character->m_moveType = MOVE_ON_WATER;
            }
            else if((i < NextStepInfo::DownBig) || (i > NextStepInfo::UpBig))
            {
                character->m_dirFlag = ENT_STAY;
                character->setAnimation(TR_ANIMATION_LARA_CLIMB_2CLICK_END, 0);
            }
            else if((next_fc.floor_normale[2] >= character->m_criticalSlantZComponent) && (i == NextStepInfo::DownBig))
            {
                if(!character->m_bt.no_fix_all)
                {
                    int frames_count = ss_anim->model->animations[TR_ANIMATION_LARA_WALK_DOWN_BACK_LEFT].frames.size();
                    int frames_count2 = (frames_count + 1) / 2;
                    if((ss_anim->current_frame >= 0) && (ss_anim->current_frame <= frames_count2))
                    {
                        character->setAnimation(TR_ANIMATION_LARA_WALK_DOWN_BACK_LEFT, ss_anim->current_frame);
                        character->m_dirFlag = ENT_MOVE_BACKWARD;
                        character->m_transform.getOrigin()[2] -= (curr_fc->floor_point[2] - next_fc.floor_point[2]);
                        character->m_bt.no_fix_all = true;
                    }
                    else if((ss_anim->current_frame >= frames_count) && (ss_anim->current_frame <= frames_count + frames_count2))
                    {
                        character->setAnimation(TR_ANIMATION_LARA_WALK_DOWN_BACK_RIGHT, ss_anim->current_frame - frames_count);
                        character->m_dirFlag = ENT_MOVE_BACKWARD;
                        character->m_transform.getOrigin()[2] -= (curr_fc->floor_point[2] - next_fc.floor_point[2]);
                        character->m_bt.no_fix_all = true;
                    }
                    else
                    {
                        character->m_dirFlag = ENT_STAY;                               // waiting for correct frame
                    }
                }
            }
            else if((cmd->move[0] == -1) && (cmd->shift || character->m_heightInfo.quicksand))
            {
                character->m_dirFlag = ENT_MOVE_BACKWARD;
                ss_anim->next_state = TR_STATE_LARA_WALK_BACK;
            }
            else
            {
                ss_anim->next_state = TR_STATE_LARA_STOP;
            }
            break;
        }

        case TR_STATE_LARA_WALK_LEFT:
            cmd->rot[0] = 0;
            character->m_dirFlag = ENT_MOVE_LEFT;
            if(character->m_moveType == MOVE_FREE_FALLING)
            {
                character->setAnimation(TR_ANIMATION_LARA_START_FREE_FALL, 0);
            }
            else if(cmd->move[1] == -1 && cmd->shift)
            {
                global_offset = character->m_transform.getBasis().getColumn(0) * -RUN_FORWARD_OFFSET;  // not an error - RUN_... more correct here
                global_offset[2] += character->m_bf.bb_max[2];
                global_offset += pos;
                Character::getHeightInfo(global_offset, &next_fc);
                if(next_fc.floor_hit && (next_fc.floor_point[2] > pos[2] - character->m_maxStepUpHeight) && (next_fc.floor_point[2] <= pos[2] + character->m_maxStepUpHeight))
                {
                    if(!curr_fc->water || (curr_fc->floor_point[2] + character->m_height > curr_fc->transition_level)) // if (floor_hit == 0) then we went to MOVE_FREE_FALLING.
                    {
                        // continue walking
                    }
                    else
                    {
                        ss_anim->next_state = TR_STATE_LARA_ONWATER_LEFT;
                        ss_anim->onFrame = ent_to_on_water;
                    }
                }
                else
                {
                    character->m_dirFlag = ENT_STAY;
                    character->setAnimation(TR_ANIMATION_LARA_STAY_SOLID, 0);
                }
            }
            else
            {
                ss_anim->next_state = TR_STATE_LARA_STOP;
            }
            break;

        case TR_STATE_LARA_WALK_RIGHT:
            cmd->rot[0] = 0;
            character->m_dirFlag = ENT_MOVE_RIGHT;
            if(character->m_moveType == MOVE_FREE_FALLING)
            {
                character->setAnimation(TR_ANIMATION_LARA_START_FREE_FALL, 0);
            }
            else if(cmd->move[1] == 1 && cmd->shift)
            {
                global_offset = character->m_transform.getBasis().getColumn(0) * RUN_FORWARD_OFFSET;// not an error - RUN_... more correct here
                global_offset[2] += character->m_bf.bb_max[2];
                global_offset += pos;
                Character::getHeightInfo(global_offset, &next_fc);
                if(next_fc.floor_hit && (next_fc.floor_point[2] > pos[2] - character->m_maxStepUpHeight) && (next_fc.floor_point[2] <= pos[2] + character->m_maxStepUpHeight))
                {
                    if(!curr_fc->water || (curr_fc->floor_point[2] + character->m_height > curr_fc->transition_level)) // if (floor_hit == 0) then we went to MOVE_FREE_FALLING.
                    {
                        // continue walking
                    }
                    else
                    {
                        ss_anim->next_state = TR_STATE_LARA_ONWATER_RIGHT;
                        ss_anim->onFrame = ent_to_on_water;
                    }
                }
                else
                {
                    character->m_dirFlag = ENT_STAY;
                    character->setAnimation(TR_ANIMATION_LARA_STAY_SOLID, 0);
                }
            }
            else
            {
                ss_anim->next_state = TR_STATE_LARA_STOP;
            }
            break;

            /*
             * Slide animations section
             */
        case TR_STATE_LARA_SLIDE_BACK:
            cmd->rot[0] = 0;
            character->lean(cmd, 0.0);
            character->m_dirFlag = ENT_MOVE_BACKWARD;

            if(character->m_moveType == MOVE_FREE_FALLING)
            {
                if(cmd->action)
                {
                    character->m_speed[0] = -character->m_transform.getBasis().getColumn(1)[0] * 128.0;
                    character->m_speed[1] = -character->m_transform.getBasis().getColumn(1)[1] * 128.0;
                }

                character->setAnimation(TR_ANIMATION_LARA_FREE_FALL_FORWARD, 0);
            }
            else if(resp->slide == CHARACTER_SLIDE_NONE)
            {
                ss_anim->next_state = TR_STATE_LARA_STOP;
            }
            else if(resp->slide != CHARACTER_SLIDE_NONE && cmd->jump)
            {
                ss_anim->next_state = TR_STATE_LARA_JUMP_BACK;
            }
            else
            {
                break;
            }

            Audio_Kill(TR_AUDIO_SOUND_SLIDING, TR_AUDIO_EMITTER_ENTITY, character->id());
            break;

        case TR_STATE_LARA_SLIDE_FORWARD:
            cmd->rot[0] = 0;
            character->lean(cmd, 0.0);
            character->m_dirFlag = ENT_MOVE_FORWARD;

            if(character->m_moveType == MOVE_FREE_FALLING)
            {
                character->m_speed[0] *= 0.2;
                character->m_speed[1] *= 0.2;
                character->setAnimation(TR_ANIMATION_LARA_FREE_FALL_FORWARD, 0);
            }
            else if(resp->slide == CHARACTER_SLIDE_NONE)
            {
                if((cmd->move[0] == 1) && (engine_world.version >= TR_III))
                {
                     ss_anim->next_state = TR_STATE_LARA_RUN_FORWARD;
                }
                else
                {
                     ss_anim->next_state = TR_STATE_LARA_STOP;                  // stop
                }
            }
            else if(resp->slide != CHARACTER_SLIDE_NONE && cmd->jump)
            {
                ss_anim->next_state = TR_STATE_LARA_JUMP_FORWARD;               // jump
            }
            else
            {
                break;
            }

            Audio_Kill(TR_AUDIO_SOUND_SLIDING, TR_AUDIO_EMITTER_ENTITY, character->id());
            break;

            /*
             * Misk animations
             */
        case TR_STATE_LARA_PUSHABLE_GRAB:
            character->m_moveType = MOVE_ON_FLOOR;
            character->m_bt.no_fix_all = true;
            cmd->rot[0] = 0.0;

            if(cmd->action)//If Lara is grabbing the block
            {
                int tf = character->checkTraverse(*character->m_traversedObject);
                character->m_dirFlag = ENT_STAY;
                ss_anim->anim_flags = ANIM_LOOP_LAST_FRAME;                     //We hold it (loop last frame)

                if((cmd->move[0] == 1) && (tf & Character::TraverseForward))                          //If player press up push
                {
                    character->m_dirFlag = ENT_MOVE_FORWARD;
                    ss_anim->anim_flags = ANIM_NORMAL_CONTROL;
                    ss_anim->next_state = TR_STATE_LARA_PUSHABLE_PUSH;
                }
                else if((cmd->move[0] == -1) && (tf & Character::TraverseBackward))                    //If player press down pull
                {
                    character->m_dirFlag = ENT_MOVE_BACKWARD;
                    ss_anim->anim_flags = ANIM_NORMAL_CONTROL;
                    ss_anim->next_state = TR_STATE_LARA_PUSHABLE_PULL;
                }
            }
            else//Lara has let go of the block
            {
                character->m_dirFlag = ENT_STAY;
                ss_anim->anim_flags = ANIM_NORMAL_CONTROL;                      //We no longer loop last frame
                ss_anim->next_state = TR_STATE_LARA_STOP;                       //Switch to next Lara state
            }
            break;

        case TR_STATE_LARA_PUSHABLE_PUSH:
        {
            character->m_bt.no_fix_all = true;
            ss_anim->onFrame = ent_stop_traverse;
            cmd->rot[0] = 0.0;
            character->m_camFollowCenter = 64;
            int i = ss_anim->model->animations[ss_anim->current_animation].frames.size();

            if(!cmd->action || !(Character::TraverseForward & character->checkTraverse(*character->m_traversedObject)))   //For TOMB4/5 If Lara is pushing and action let go, don't push
            {
                ss_anim->next_state = TR_STATE_LARA_STOP;
            }

            if((character->m_traversedObject != NULL) && (ss_anim->current_frame > 16) && (ss_anim->current_frame < i - 16)) ///@FIXME: magick 16
            {
                bool was_traversed = false;

                if(character->m_transform.getBasis().getColumn(1)[0] > 0.9)
                {
                    t = character->m_transform.getOrigin()[0] + (character->m_bf.bb_max[1] - character->m_traversedObject->m_bf.bb_min[0] - 32.0);
                    if(t > character->m_traversedObject->m_transform.getOrigin()[0])
                    {
                        character->m_traversedObject->m_transform.getOrigin()[0] = t;
                        was_traversed = true;
                    }
                }
                else if(character->m_transform.getBasis().getColumn(1)[0] < -0.9)
                {
                    t = character->m_transform.getOrigin()[0] - (character->m_bf.bb_max[1] + character->m_traversedObject->m_bf.bb_max[0] - 32.0);
                    if(t < character->m_traversedObject->m_transform.getOrigin()[0])
                    {
                        character->m_traversedObject->m_transform.getOrigin()[0] = t;
                        was_traversed = true;
                    }
                }
                else if(character->m_transform.getBasis().getColumn(1)[1] > 0.9)
                {
                    t = character->m_transform.getOrigin()[1] + (character->m_bf.bb_max[1] - character->m_traversedObject->m_bf.bb_min[1] - 32.0);
                    if(t > character->m_traversedObject->m_transform.getOrigin()[1])
                    {
                        character->m_traversedObject->m_transform.getOrigin()[1] = t;
                        was_traversed = true;
                    }
                }
                else if(character->m_transform.getBasis().getColumn(1)[1] < -0.9)
                {
                    t = character->m_transform.getOrigin()[1] - (character->m_bf.bb_max[1] + character->m_traversedObject->m_bf.bb_max[1] - 32.0);
                    if(t < character->m_traversedObject->m_transform.getOrigin()[1])
                    {
                        character->m_traversedObject->m_transform.getOrigin()[1] = t;
                        was_traversed = true;
                    }
                }

                if(engine_world.version > TR_III)
                {
                    if(was_traversed)
                    {
                        if(Audio_IsEffectPlaying(TR_AUDIO_SOUND_PUSHABLE,TR_AUDIO_EMITTER_ENTITY,character->id()) == -1)
                            Audio_Send(TR_AUDIO_SOUND_PUSHABLE, TR_AUDIO_EMITTER_ENTITY, character->id());
                    }
                    else
                    {
                        Audio_Kill(TR_AUDIO_SOUND_PUSHABLE, TR_AUDIO_EMITTER_ENTITY, character->id());
                    }
                }
                else
                {
                    if( (ss_anim->current_frame == 49)   ||
                        (ss_anim->current_frame == 110)  ||
                        (ss_anim->current_frame == 142)   )
                    {
                        if(Audio_IsEffectPlaying(TR_AUDIO_SOUND_PUSHABLE,TR_AUDIO_EMITTER_ENTITY,character->id()) == -1)
                            Audio_Send(TR_AUDIO_SOUND_PUSHABLE, TR_AUDIO_EMITTER_ENTITY, character->id());
                    }
                }

                character->m_traversedObject->updateRigidBody(true);
            }
            else
            {
                if(engine_world.version > TR_III)
                {
                    Audio_Kill(TR_AUDIO_SOUND_PUSHABLE, TR_AUDIO_EMITTER_ENTITY, character->id());
                }
            }
            break;
        }

        case TR_STATE_LARA_PUSHABLE_PULL:
        {
            character->m_bt.no_fix_all = true;
            ss_anim->onFrame = ent_stop_traverse;
            cmd->rot[0] = 0.0;
            character->m_camFollowCenter = 64;
            int i = ss_anim->model->animations[ss_anim->current_animation].frames.size();

            if(!cmd->action || !(Character::TraverseBackward & character->checkTraverse(*character->m_traversedObject)))   //For TOMB4/5 If Lara is pulling and action let go, don't pull
            {
                ss_anim->next_state = TR_STATE_LARA_STOP;
            }

            if((character->m_traversedObject != NULL) && (ss_anim->current_frame > 20) && (ss_anim->current_frame < i - 16)) ///@FIXME: magick 20
            {
                bool was_traversed = false;

                if(character->m_transform.getBasis().getColumn(1)[0] > 0.9)
                {
                    t = character->m_transform.getOrigin()[0] + (character->m_bf.bb_max[1] - character->m_traversedObject->m_bf.bb_min[0] - 32.0);
                    if(t < character->m_traversedObject->m_transform.getOrigin()[0])
                    {
                        character->m_traversedObject->m_transform.getOrigin()[0] = t;
                        was_traversed = true;
                    }
                }
                else if(character->m_transform.getBasis().getColumn(1)[0] < -0.9)
                {
                    t = character->m_transform.getOrigin()[0] - (character->m_bf.bb_max[1] + character->m_traversedObject->m_bf.bb_max[0] - 32.0);
                    if(t > character->m_traversedObject->m_transform.getOrigin()[0])
                    {
                        character->m_traversedObject->m_transform.getOrigin()[0] = t;
                        was_traversed = true;
                    }
                }
                else if(character->m_transform.getBasis().getColumn(1)[1] > 0.9)
                {
                    t = character->m_transform.getOrigin()[1] + (character->m_bf.bb_max[1] - character->m_traversedObject->m_bf.bb_min[1] - 32.0);
                    if(t < character->m_traversedObject->m_transform.getOrigin()[1])
                    {
                        character->m_traversedObject->m_transform.getOrigin()[1] = t;
                        was_traversed = true;
                    }
                }
                else if(character->m_transform.getBasis().getColumn(1)[1] < -0.9)
                {
                    t = character->m_transform.getOrigin()[1] - (character->m_bf.bb_max[1] + character->m_traversedObject->m_bf.bb_max[1] - 32.0);
                    if(t > character->m_traversedObject->m_transform.getOrigin()[1])
                    {
                        character->m_traversedObject->m_transform.getOrigin()[1] = t;
                        was_traversed = true;
                    }
                }

                if(engine_world.version > TR_III)
                {
                    if(was_traversed)
                    {
                        if(Audio_IsEffectPlaying(TR_AUDIO_SOUND_PUSHABLE,TR_AUDIO_EMITTER_ENTITY,character->id()) == -1)

                            Audio_Send(TR_AUDIO_SOUND_PUSHABLE, TR_AUDIO_EMITTER_ENTITY, character->id());
                    }
                    else
                    {
                        Audio_Kill(TR_AUDIO_SOUND_PUSHABLE, TR_AUDIO_EMITTER_ENTITY, character->id());
                    }
                }
                else
                {
                    if( (ss_anim->current_frame == 40)  ||
                        (ss_anim->current_frame == 92)  ||
                        (ss_anim->current_frame == 124) ||
                        (ss_anim->current_frame == 156)  )
                    {
                        if(Audio_IsEffectPlaying(TR_AUDIO_SOUND_PUSHABLE,TR_AUDIO_EMITTER_ENTITY,character->id()) == -1)
                            Audio_Send(TR_AUDIO_SOUND_PUSHABLE, TR_AUDIO_EMITTER_ENTITY, character->id());
                    }
                }

                character->m_traversedObject->updateRigidBody(true);
            }
            else
            {
                if(engine_world.version > TR_III)
                {
                    Audio_Kill(TR_AUDIO_SOUND_PUSHABLE, TR_AUDIO_EMITTER_ENTITY, character->id());
                }
            }
            break;
        }

        case TR_STATE_LARA_ROLL_FORWARD:
            break;

        case TR_STATE_LARA_ROLL_BACKWARD:
            if(character->m_moveType == MOVE_FREE_FALLING)
            {
                character->setAnimation(TR_ANIMATION_LARA_FREE_FALL_FORWARD, 0);
            }
            else if(low_vertical_space)
            {
                character->m_dirFlag = ENT_STAY;
            }
            else if(resp->slide == CHARACTER_SLIDE_FRONT)
            {
                character->setAnimation(TR_ANIMATION_LARA_SLIDE_FORWARD, 0);
            }
            else if(resp->slide == CHARACTER_SLIDE_BACK)
            {
                character->setAnimation(TR_ANIMATION_LARA_START_SLIDE_BACKWARD, 0);
            }
            break;

        /*
         * Climbing section
         */
        case TR_STATE_LARA_JUMP_UP:
            cmd->rot[0] = 0.0;
            if(cmd->action && (character->m_moveType != MOVE_WALLS_CLIMB) && (character->m_moveType != MOVE_CLIMBING))
            {
                t = LARA_TRY_HANG_WALL_OFFSET + LARA_HANG_WALL_DISTANCE;
                global_offset = character->m_transform.getBasis().getColumn(1) * t;
                global_offset[2] += character->m_bf.bb_max[2] + LARA_HANG_VERTICAL_EPSILON + engine_frame_time * character->m_speed[2];
                character->m_climb = character->checkClimbability(global_offset, &next_fc, 0.0);
                if(character->m_climb.edge_hit != ClimbType::NoClimb)
                {
                    character->m_climb.point = character->m_climb.edge_point;
                    character->m_angles[0] = character->m_climb.edge_z_ang;
                    character->updateTransform();
                    character->m_moveType = MOVE_CLIMBING;                             // hang on
                    character->m_speed.setZero();

                    pos[0] = character->m_climb.point[0] - (LARA_HANG_WALL_DISTANCE) * character->m_transform.getBasis().getColumn(1)[0];
                    pos[1] = character->m_climb.point[1] - (LARA_HANG_WALL_DISTANCE) * character->m_transform.getBasis().getColumn(1)[1];
                    pos[2] = character->m_climb.point[2] - character->m_bf.bb_max[2] + LARA_HANG_VERTICAL_OFFSET;
                }
                else
                {
                    character->m_climb = character->checkWallsClimbability();
                    if((character->m_climb.wall_hit != ClimbType::NoClimb) &&
                       (character->m_speed[2] < 0.0)) // Only hang if speed is lower than zero.
                    {
                        // Fix the position to the TR metering step.
                        character->m_transform.getOrigin()[2] = std::floor(character->m_transform.getOrigin()[2] / TR_METERING_STEP) * TR_METERING_STEP;
                        character->m_moveType = MOVE_WALLS_CLIMB;
                        character->setAnimation(TR_ANIMATION_LARA_HANG_IDLE, -1);
                        break;
                    }
                }
            }

            if(cmd->move[0] == 1)
            {
                character->m_dirFlag = ENT_MOVE_FORWARD;
            }
            else if(cmd->move[0] == -1)
            {
                character->m_dirFlag = ENT_MOVE_BACKWARD;
            }
            else if(cmd->move[1] == 1)
            {
                character->m_dirFlag = ENT_MOVE_RIGHT;
            }
            else if(cmd->move[1] == -1)
            {
                character->m_dirFlag = ENT_MOVE_LEFT;
            }
            else
            {
                character->m_dirFlag = ENT_STAY;
            }

            if(character->m_moveType == MOVE_UNDERWATER)
            {
                character->m_angles[1] = -45.0;
                cmd->rot[1] = 0.0;
                character->updateTransform();
                character->setAnimation(TR_ANIMATION_LARA_FREE_FALL_TO_UNDERWATER, 0);
            }
            else if(cmd->action && curr_fc->ceiling_climb && (curr_fc->ceiling_hit) && (pos[2] + character->m_bf.bb_max[2] > curr_fc->ceiling_point[2] - 64.0))
            {
                ss_anim->next_state = TR_STATE_LARA_MONKEYSWING_IDLE;
                ss_anim->onFrame = ent_to_monkey_swing;
            }
            else if(cmd->action && (character->m_moveType == MOVE_CLIMBING))
            {
                ss_anim->next_state = TR_STATE_LARA_HANG;
                character->setAnimation(TR_ANIMATION_LARA_HANG_IDLE, -1);
            }
            else if((resp->vertical_collide & 0x01) || (character->m_moveType == MOVE_ON_FLOOR))
            {
                ss_anim->next_state = TR_STATE_LARA_STOP;                        // landing immediately
            }
            else
            {
                if(character->m_speed[2] < -FREE_FALL_SPEED_2)                 // next free fall stage
                {
                    character->m_moveType = MOVE_FREE_FALLING;
                    ss_anim->next_state = TR_STATE_LARA_FREEFALL;
                }
                break;
            }
            break;

        case TR_STATE_LARA_REACH:
            cmd->rot[0] = 0.0;
            if(character->m_moveType == MOVE_UNDERWATER)
            {
                character->m_angles[1] = -45.0;
                cmd->rot[1] = 0.0;
                character->updateTransform();
                character->setAnimation(TR_ANIMATION_LARA_FREE_FALL_TO_UNDERWATER, 0);
                break;
            }

            if(cmd->action && (character->m_moveType == MOVE_FREE_FALLING))
            {
                t = LARA_TRY_HANG_WALL_OFFSET + LARA_HANG_WALL_DISTANCE;
                global_offset = character->m_transform.getBasis().getColumn(1) * t;
                global_offset[2] += character->m_bf.bb_max[2] + LARA_HANG_VERTICAL_EPSILON + engine_frame_time * character->m_speed[2];
                character->m_climb = character->checkClimbability(global_offset, &next_fc, 0.0);
                if(character->m_climb.edge_hit != ClimbType::NoClimb && character->m_climb.can_hang)
                {
                    character->m_climb.point = character->m_climb.edge_point;
                    character->m_angles[0] = character->m_climb.edge_z_ang;
                    character->updateTransform();
                    character->m_moveType = MOVE_CLIMBING;                             // hang on
                    character->m_speed.setZero();
                }

                // If Lara is moving backwards off the ledge we want to move Lara slightly forwards
                // depending on the current angle.
                if((character->m_dirFlag == ENT_MOVE_BACKWARD) && (character->m_moveType == MOVE_CLIMBING))
                {
                    pos[0] = character->m_climb.point[0] - character->m_transform.getBasis().getColumn(1)[0] * (character->m_forwardSize + 16.0);
                    pos[1] = character->m_climb.point[1] - character->m_transform.getBasis().getColumn(1)[1] * (character->m_forwardSize + 16.0);
                }
            }

            if(((character->m_moveType != MOVE_ON_FLOOR)) && cmd->action && curr_fc->ceiling_climb && (curr_fc->ceiling_hit) && (pos[2] + character->m_bf.bb_max[2] > curr_fc->ceiling_point[2] - 64.0))
            {
                ss_anim->next_state = TR_STATE_LARA_MONKEYSWING_IDLE;
                ss_anim->onFrame = ent_to_monkey_swing;
                break;
            }
            if(((resp->vertical_collide & 0x01) || (character->m_moveType == MOVE_ON_FLOOR)) && (!cmd->action || !character->m_climb.can_hang))
            {
                ss_anim->next_state = TR_STATE_LARA_STOP;                       // middle landing
                break;
            }

            if((character->m_speed[2] < -FREE_FALL_SPEED_2))
            {
                character->m_moveType = MOVE_FREE_FALLING;
                ss_anim->next_state = TR_STATE_LARA_FREEFALL;
                break;
            }

            if(character->m_moveType == MOVE_CLIMBING)
            {
                character->m_speed.setZero();
                ss_anim->next_state = TR_STATE_LARA_HANG;
                ss_anim->onFrame = ent_to_edge_climb;
#if OSCILLATE_HANG_USE
                move = ent->transform.getBasis().getColumn(1) * PENETRATION_TEST_OFFSET;
                if(Entity_CheckNextPenetration(ent, cmd, move) == 0)
                {
                    ent->setAnimation(TR_ANIMATION_LARA_OSCILLATE_HANG_ON, 0);
                    ent_to_edge_climb(ent);
                }
#endif
            }
            break;

            /*other code here prevents to UGLY Lara's move in end of "climb on", do not loose ent_set_on_floor_after_climb callback here!*/
        case TR_STATE_LARA_HANDSTAND:
        case TR_STATE_LARA_CLIMBING:
        case TR_STATE_LARA_CLIMB_TO_CRAWL:
            cmd->rot[0] = 0;
            character->m_bt.no_fix_all = true;
            ss_anim->onFrame = ent_set_on_floor_after_climb;
            break;

        case TR_STATE_LARA_HANG:
            cmd->rot[0] = 0.0;

            if(character->m_moveType == MOVE_WALLS_CLIMB)
            {
                if(cmd->action)
                {
                    if((character->m_climb.wall_hit == ClimbType::FullClimb) && (cmd->move[0] == 0) && (cmd->move[1] == 0))
                    {
                        ss_anim->next_state = TR_STATE_LARA_LADDER_IDLE;
                    }
                    else if(cmd->move[0] == 1)             // UP
                    {
                        character->setAnimation(TR_ANIMATION_LARA_LADDER_UP_HANDS, 0);
                    }
                    else if(cmd->move[0] ==-1)             // DOWN
                    {
                        character->setAnimation(TR_ANIMATION_LARA_LADDER_DOWN_HANDS, 0);
                    }
                    else if(cmd->move[1] == 1)
                    {
                        character->m_dirFlag = ENT_MOVE_RIGHT;
                        character->setAnimation(TR_ANIMATION_LARA_CLIMB_RIGHT, 0); // edge climb right
                    }
                    else if(cmd->move[1] ==-1)
                    {
                        character->m_dirFlag = ENT_MOVE_LEFT;
                        character->setAnimation(TR_ANIMATION_LARA_CLIMB_LEFT, 0); // edge climb left
                    }
                    else if(character->m_climb.wall_hit == ClimbType::NoClimb)
                    {
                        character->m_moveType = MOVE_FREE_FALLING;
                        character->setAnimation(TR_ANIMATION_LARA_STOP_HANG_VERTICAL, 0); // fall down
                    }
                    else
                    {
                        ss_anim->anim_flags = ANIM_LOOP_LAST_FRAME;             // disable shake
                    }
                }
                else
                {
                    character->m_moveType = MOVE_FREE_FALLING;
                    character->setAnimation(TR_ANIMATION_LARA_TRY_HANG_VERTICAL, 0); // fall down
                }
                break;
            }

            if((resp->kill == 0) && cmd->action)                         // we have to update climb point every time so entity can move
            {
                t = LARA_TRY_HANG_WALL_OFFSET + LARA_HANG_WALL_DISTANCE;
                global_offset = character->m_transform.getBasis().getColumn(1) * t;
                global_offset[2] += character->m_bf.bb_max[2] + LARA_HANG_VERTICAL_EPSILON;
                character->m_climb = character->checkClimbability(global_offset, &next_fc, 0.0);
                if(character->m_climb.can_hang)
                {
                    character->m_climb.point = character->m_climb.edge_point;
                    character->m_angles[0] = character->m_climb.edge_z_ang;
                    character->updateTransform();
                    character->m_moveType = MOVE_CLIMBING;                             // hang on
                }
            }
            else
            {
                character->m_moveType = MOVE_FREE_FALLING;
                character->setAnimation(TR_ANIMATION_LARA_TRY_HANG_VERTICAL, 0); // fall down
                break;
            }

            if(character->m_moveType == MOVE_CLIMBING)
            {
                if(cmd->move[0] == 1)
                {
                    if(character->m_climb.edge_hit != ClimbType::NoClimb && (character->m_climb.next_z_space >= 512.0) && ((character->m_climb.next_z_space < character->m_height - LARA_HANG_VERTICAL_EPSILON) || cmd->crouch))
                    {
                        character->m_climb.point = character->m_climb.edge_point;
                        ss_anim->next_state = TR_STATE_LARA_CLIMB_TO_CRAWL;     // crawlspace climb
                    }
                    else if(character->m_climb.edge_hit != ClimbType::NoClimb && (character->m_climb.next_z_space >= character->m_height - LARA_HANG_VERTICAL_EPSILON))
                    {
                        Sys_DebugLog(LOG_FILENAME, "Zspace = %f", character->m_climb.next_z_space);
                        character->m_climb.point = character->m_climb.edge_point;
                        ss_anim->next_state = (cmd->shift)?(TR_STATE_LARA_HANDSTAND):(TR_STATE_LARA_CLIMBING);               // climb up
                    }
                    else
                    {
                        pos[0] = character->m_climb.point[0] - (LARA_HANG_WALL_DISTANCE) * character->m_transform.getBasis().getColumn(1)[0];
                        pos[1] = character->m_climb.point[1] - (LARA_HANG_WALL_DISTANCE) * character->m_transform.getBasis().getColumn(1)[1];
                        pos[2] = character->m_climb.point[2] - character->m_bf.bb_max[2] + LARA_HANG_VERTICAL_OFFSET;
                        character->m_speed.setZero();
                        ss_anim->anim_flags = ANIM_LOOP_LAST_FRAME;             // disable shake
                    }
                }
                else if(cmd->move[0] ==-1)                                      // check walls climbing
                {
                    character->m_climb = character->checkWallsClimbability();
                    if(character->m_climb.wall_hit != ClimbType::NoClimb)
                    {
                        character->m_moveType = MOVE_WALLS_CLIMB;
                    }
                    ss_anim->anim_flags = ANIM_LOOP_LAST_FRAME;                 // disable shake
                }
                else if(cmd->move[1] ==-1)
                {
                    move = character->m_transform.getBasis().getColumn(0) * -PENETRATION_TEST_OFFSET;
                    if((character->checkNextPenetration(move) == 0) || (character->m_response.horizontal_collide == 0x00)) //we only want lara to shimmy when last frame is reached!
                    {
                        character->m_moveType = ENT_MOVE_LEFT;
                        character->setAnimation(TR_ANIMATION_LARA_CLIMB_LEFT, 0);
                    }
                    else
                    {
                        ss_anim->anim_flags = ANIM_LOOP_LAST_FRAME;             // disable shake
                    }
                }
                else if(cmd->move[1] == 1)
                {
                    move = character->m_transform.getBasis().getColumn(0) * PENETRATION_TEST_OFFSET;
                    if((character->checkNextPenetration(move) == 0) || (character->m_response.horizontal_collide == 0x00)) //we only want lara to shimmy when last frame is reached!
                    {
                        character->m_moveType = ENT_MOVE_RIGHT;
                        character->setAnimation(TR_ANIMATION_LARA_CLIMB_RIGHT, 0);
                    }
                    else
                    {
                        ss_anim->anim_flags = ANIM_LOOP_LAST_FRAME;             // disable shake
                    }
                }
                else
                {
                    ss_anim->anim_flags = ANIM_LOOP_LAST_FRAME;                 // disable shake
                    pos[0] = character->m_climb.point[0] - (LARA_HANG_WALL_DISTANCE) * character->m_transform.getBasis().getColumn(1)[0];
                    pos[1] = character->m_climb.point[1] - (LARA_HANG_WALL_DISTANCE) * character->m_transform.getBasis().getColumn(1)[1];
                    pos[2] = character->m_climb.point[2] - character->m_bf.bb_max[2] + LARA_HANG_VERTICAL_OFFSET;
                    character->m_speed.setZero();
                }
            }
            else if(cmd->action && curr_fc->ceiling_climb && (curr_fc->ceiling_hit) && (pos[2] + character->m_bf.bb_max[2] > curr_fc->ceiling_point[2] - 64.0))
            {
                ss_anim->next_state = TR_STATE_LARA_MONKEYSWING_IDLE;
                ss_anim->onFrame = ent_to_monkey_swing;
            }
            else
            {
                character->m_moveType = MOVE_FREE_FALLING;
                character->setAnimation(TR_ANIMATION_LARA_TRY_HANG_VERTICAL, 0); // fall down
            }
            break;

        case TR_STATE_LARA_LADDER_IDLE:
            cmd->rot[0] = 0;
            character->m_moveType = MOVE_WALLS_CLIMB;
            character->m_dirFlag = ENT_STAY;
            character->m_camFollowCenter = 64;
            if(character->m_moveType == MOVE_CLIMBING)
            {
                ss_anim->next_state = TR_STATE_LARA_CLIMBING;
                break;
            }
            if(!cmd->action)
            {
                character->m_moveType = MOVE_FREE_FALLING;
                character->setAnimation(TR_ANIMATION_LARA_STOP_HANG_VERTICAL, 0); // fall down
            }
            else if(cmd->jump)
            {
                ss_anim->next_state = TR_STATE_LARA_JUMP_BACK;
                character->m_dirFlag = ENT_MOVE_BACKWARD;
            }
            else if(cmd->move[0] == 1)
            {
                t = LARA_TRY_HANG_WALL_OFFSET + LARA_HANG_WALL_DISTANCE;
                global_offset = character->m_transform.getBasis().getColumn(1) * t;
                global_offset[2] += character->m_bf.bb_max[2] + LARA_HANG_VERTICAL_EPSILON;
                character->m_climb = character->checkClimbability(global_offset, &next_fc, 0.0);
                if(character->m_climb.edge_hit != ClimbType::NoClimb && (character->m_climb.next_z_space >= 512.0))
                {
                    character->m_moveType = MOVE_CLIMBING;
                    ss_anim->next_state = TR_STATE_LARA_CLIMBING;
                }
                else if((!curr_fc->ceiling_hit) || (pos[2] + character->m_bf.bb_max[2] < curr_fc->ceiling_point[2]))
                {
                    ss_anim->next_state = TR_STATE_LARA_LADDER_UP;
                }
            }
            else if(cmd->move[0] == -1)
            {
                ss_anim->next_state = TR_STATE_LARA_LADDER_DOWN;
            }
            else if(cmd->move[1] == 1)
            {
                ss_anim->next_state = TR_STATE_LARA_LADDER_RIGHT;
            }
            else if(cmd->move[1] == -1)
            {
                ss_anim->next_state = TR_STATE_LARA_LADDER_LEFT;
            }
            break;

        case TR_STATE_LARA_LADDER_LEFT:
            character->m_dirFlag = ENT_MOVE_LEFT;
            if(!cmd->action || (character->m_climb.wall_hit == ClimbType::NoClimb))
            {
                ss_anim->next_state = TR_STATE_LARA_HANG;
            }
            else
            {
                ss_anim->next_state = TR_STATE_LARA_LADDER_IDLE;
            }
            break;

        case TR_STATE_LARA_LADDER_RIGHT:
            character->m_dirFlag = ENT_MOVE_RIGHT;
            if(!cmd->action || (character->m_climb.wall_hit == ClimbType::NoClimb))
            {
                ss_anim->next_state = TR_STATE_LARA_HANG;
            }
            else
            {
                ss_anim->next_state = TR_STATE_LARA_LADDER_IDLE;
            }
            break;

        case TR_STATE_LARA_LADDER_UP:
            character->m_camFollowCenter = 64;
            if(character->m_moveType == MOVE_CLIMBING)
            {
                ss_anim->next_state = TR_STATE_LARA_LADDER_IDLE;
                break;
            }

            if(cmd->action && character->m_climb.wall_hit != ClimbType::NoClimb)
            {
                t = LARA_TRY_HANG_WALL_OFFSET + LARA_HANG_WALL_DISTANCE;
                global_offset = character->m_transform.getBasis().getColumn(1) * t;
                global_offset[2] += character->m_bf.bb_max[2] + LARA_HANG_VERTICAL_EPSILON;
                character->m_climb = character->checkClimbability(global_offset, &next_fc, 0.0);
                if(character->m_climb.edge_hit != ClimbType::NoClimb && (character->m_climb.next_z_space >= 512.0))
                {
                    character->m_moveType = MOVE_CLIMBING;
                    ss_anim->next_state = TR_STATE_LARA_LADDER_IDLE;
                }
                else if((cmd->move[0] <= 0) && (curr_fc->ceiling_hit || (pos[2] + character->m_bf.bb_max[2] >= curr_fc->ceiling_point[2])))
                {
                    ss_anim->next_state = TR_STATE_LARA_LADDER_IDLE;
                }

                if(curr_fc->ceiling_hit && (pos[2] + character->m_bf.bb_max[2] > curr_fc->ceiling_point[2]))
                {
                    pos[2] = curr_fc->ceiling_point[2] - character->m_bf.bb_max[2];
                }
            }
            else
            {
                // Free fall after stop
                ss_anim->next_state = TR_STATE_LARA_LADDER_IDLE;
            }
            break;

        case TR_STATE_LARA_LADDER_DOWN:
            character->m_camFollowCenter = 64;
            if(cmd->action && character->m_climb.wall_hit != ClimbType::NoClimb && (cmd->move[1] < 0))
            {
                if(character->m_climb.wall_hit != ClimbType::FullClimb)
                {
                    ss_anim->next_state = TR_STATE_LARA_LADDER_IDLE;
                }
            }
            else
            {
                ss_anim->next_state = TR_STATE_LARA_LADDER_IDLE;
            }
            break;

        case TR_STATE_LARA_SHIMMY_LEFT:
            character->m_bt.no_fix_body_parts = BODY_PART_LEGS_1 | BODY_PART_LEGS_2 | BODY_PART_LEGS_3;

            cmd->rot[0] = 0.0;
            character->m_dirFlag = ENT_MOVE_LEFT;
            if(!cmd->action)
            {
                character->m_speed.setZero();
                character->m_moveType = MOVE_FREE_FALLING;
                character->setAnimation(TR_ANIMATION_LARA_TRY_HANG_VERTICAL, 0); // fall down
                break;
            }

            if(character->m_moveType == MOVE_WALLS_CLIMB)
            {
                if(character->m_climb.wall_hit == ClimbType::NoClimb)
                {
                    character->m_moveType = MOVE_FREE_FALLING;
                    character->setAnimation(TR_ANIMATION_LARA_STOP_HANG_VERTICAL, 0); // fall down
                }
            }
            else
            {
                t = LARA_TRY_HANG_WALL_OFFSET + LARA_HANG_WALL_DISTANCE;
                global_offset = character->m_transform.getBasis().getColumn(1) * t;
                global_offset[2] += LARA_HANG_SENSOR_Z + LARA_HANG_VERTICAL_EPSILON;
                character->m_climb = character->checkClimbability(global_offset, &next_fc, 0.0);
                if(character->m_climb.edge_hit != ClimbType::NoClimb)
                {
                    character->m_climb.point = character->m_climb.edge_point;
                    character->m_angles[0] = character->m_climb.edge_z_ang;
                    character->updateTransform();
                    character->m_moveType = MOVE_CLIMBING;                             // hang on
                    pos[0] = character->m_climb.point[0] - (LARA_HANG_WALL_DISTANCE) * character->m_transform.getBasis().getColumn(1)[0];
                    pos[1] = character->m_climb.point[1] - (LARA_HANG_WALL_DISTANCE) * character->m_transform.getBasis().getColumn(1)[1];
                    pos[2] = character->m_climb.point[2] - character->m_bf.bb_max[2] + LARA_HANG_VERTICAL_OFFSET;
                    character->m_speed.setZero();
                }
                else
                {
                    character->m_moveType = MOVE_FREE_FALLING;
                    character->setAnimation(TR_ANIMATION_LARA_STOP_HANG_VERTICAL, 0); // fall down
                    break;
                }
            }

            if(cmd->move[1] ==-1)
            {
                move = character->m_transform.getBasis().getColumn(0) * -PENETRATION_TEST_OFFSET;
                if((character->checkNextPenetration(move) > 0) && (character->m_response.horizontal_collide != 0x00))
                {
                    ss_anim->next_state = TR_STATE_LARA_HANG;
                }
            }
            else
            {
                ss_anim->next_state = TR_STATE_LARA_HANG;
            }
            break;

        case TR_STATE_LARA_SHIMMY_RIGHT:
            character->m_bt.no_fix_body_parts = BODY_PART_LEGS_1 | BODY_PART_LEGS_2 | BODY_PART_LEGS_3;

            cmd->rot[0] = 0.0;
            character->m_dirFlag = ENT_MOVE_RIGHT;
            if(!cmd->action)
            {
                character->m_speed.setZero();
                character->m_moveType = MOVE_FREE_FALLING;
                character->setAnimation(TR_ANIMATION_LARA_TRY_HANG_VERTICAL, 0); // fall down
                break;
            }

            if(character->m_moveType == MOVE_WALLS_CLIMB)
            {
                if(character->m_climb.wall_hit == ClimbType::NoClimb)
                {
                    character->m_moveType = MOVE_FREE_FALLING;
                    character->setAnimation(TR_ANIMATION_LARA_STOP_HANG_VERTICAL, 0); // fall down
                }
            }
            else
            {
                t = LARA_TRY_HANG_WALL_OFFSET + LARA_HANG_WALL_DISTANCE;
                global_offset = character->m_transform.getBasis().getColumn(1) * t;
                global_offset[2] += LARA_HANG_SENSOR_Z + LARA_HANG_VERTICAL_EPSILON;
                character->m_climb = character->checkClimbability(global_offset, &next_fc, 0.0);
                if(character->m_climb.edge_hit != ClimbType::NoClimb)
                {
                    character->m_climb.point = character->m_climb.edge_point;
                    character->m_angles[0] = character->m_climb.edge_z_ang;
                    character->updateTransform();
                    character->m_moveType = MOVE_CLIMBING;                             // hang on
                    pos[0] = character->m_climb.point[0] - (LARA_HANG_WALL_DISTANCE) * character->m_transform.getBasis().getColumn(1)[0];
                    pos[1] = character->m_climb.point[1] - (LARA_HANG_WALL_DISTANCE) * character->m_transform.getBasis().getColumn(1)[1];
                    pos[2] = character->m_climb.point[2] - character->m_bf.bb_max[2] + LARA_HANG_VERTICAL_OFFSET;
                    character->m_speed.setZero();
                }
                else
                {
                    character->m_moveType = MOVE_FREE_FALLING;
                    character->setAnimation(TR_ANIMATION_LARA_STOP_HANG_VERTICAL, 0); // fall down
                    break;
                }
            }

            if(cmd->move[1] == 1)
            {
                move = character->m_transform.getBasis().getColumn(0) * PENETRATION_TEST_OFFSET;
                if((character->checkNextPenetration(move) > 0) && (character->m_response.horizontal_collide != 0x00))
                {
                    ss_anim->next_state = TR_STATE_LARA_HANG;
                }
            }
            else
            {
                ss_anim->next_state = TR_STATE_LARA_HANG;
            }
            break;

        case TR_STATE_LARA_ONWATER_EXIT:
            cmd->rot[0] *= 0.0;
            character->m_bt.no_fix_all = true;
            ss_anim->onFrame = ent_set_on_floor_after_climb;
            break;

        case TR_STATE_LARA_JUMP_FORWARD:
        case TR_STATE_LARA_FALL_BACKWARD:
            character->m_bt.no_fix_body_parts = BODY_PART_LEGS_1 | BODY_PART_LEGS_2 | BODY_PART_LEGS_3;
            character->lean(cmd, 4.0);

            if((resp->vertical_collide & 0x01) || (character->m_moveType == MOVE_ON_FLOOR))
            {
                if(character->m_self->room->flags & TR_ROOM_FLAG_QUICKSAND)
                {
                    character->setAnimation(TR_ANIMATION_LARA_STAY_IDLE, 0);
                }
                else if(!cmd->action && (cmd->move[0] == 1) && !cmd->crouch)
                {
                    character->m_moveType = MOVE_ON_FLOOR;
                    ss_anim->next_state = TR_STATE_LARA_RUN_FORWARD;
                }
                else
                {
                    ss_anim->next_state = TR_STATE_LARA_STOP;
                }
            }
            else if(character->m_moveType == MOVE_UNDERWATER)
            {
                character->m_angles[1] = -45.0;
                cmd->rot[1] = 0.0;
                character->updateTransform();
                character->setAnimation(TR_ANIMATION_LARA_FREE_FALL_TO_UNDERWATER, 0);
            }
            else if(resp->horizontal_collide & 0x01)
            {
                character->setAnimation(TR_ANIMATION_LARA_SMASH_JUMP, 0);
                character->m_dirFlag = ENT_MOVE_BACKWARD;
                character->updateCurrentSpeed(true);
            }
            else if(character->m_speed[2] <= -FREE_FALL_SPEED_2)
            {
                ss_anim->next_state = TR_STATE_LARA_FREEFALL;                    // free falling
            }
            else if(cmd->action)
            {
                ss_anim->next_state = TR_STATE_LARA_REACH;
            }
            else if(cmd->shift)
            {
                ss_anim->next_state = TR_STATE_LARA_SWANDIVE_BEGIN;              // fly like fish
            }
            else if(character->m_speed[2] <= -FREE_FALL_SPEED_2)
            {
                ss_anim->next_state = TR_STATE_LARA_FREEFALL;                    // free falling
            }
            else if(cmd->roll)
            {
                ss_anim->next_state = TR_STATE_LARA_JUMP_ROLL;
            }
            break;

            /*
             * FREE FALL TO UNDERWATER CASES
             */
        case TR_STATE_LARA_UNDERWATER_DIVING:
            character->m_angles[1] = -45.0;
            cmd->rot[1] = 0.0;
            character->updateTransform();
            ss_anim->onFrame = ent_correct_diving_angle;
            break;

        case TR_STATE_LARA_FREEFALL:
            character->lean(cmd, 1.0);

            if( (int(character->m_speed[2]) <=  -FREE_FALL_SPEED_CRITICAL) &&
                (int(character->m_speed[2]) >= (-FREE_FALL_SPEED_CRITICAL-100)) )
            {
                character->m_speed[2] = -FREE_FALL_SPEED_CRITICAL-101;
                Audio_Send(TR_AUDIO_SOUND_LARASCREAM, TR_AUDIO_EMITTER_ENTITY, character->id());       // Scream
            }
            else if(character->m_speed[2] <= -FREE_FALL_SPEED_MAXSAFE)
            {
                //Reset these to zero so Lara is only falling downwards
                character->m_speed[0] = 0.0;
                character->m_speed[1] = 0.0;
            }

            if(character->m_moveType == MOVE_UNDERWATER)
            {
                character->m_angles[1] = -45.0;
                cmd->rot[1] = 0.0;
                character->updateTransform();                                     // needed here to fix underwater in wall collision bug
                character->setAnimation(TR_ANIMATION_LARA_FREE_FALL_TO_UNDERWATER, 0);
                Audio_Kill(TR_AUDIO_SOUND_LARASCREAM, TR_AUDIO_EMITTER_ENTITY, character->id());       // Stop scream

                // Splash sound is hardcoded, beginning with TR3.
                if(engine_world.version > TR_II)
                {
                    Audio_Send(TR_AUDIO_SOUND_SPLASH, TR_AUDIO_EMITTER_ENTITY, character->id());
                }
            }
            else if((resp->vertical_collide & 0x01) || (character->m_moveType == MOVE_ON_FLOOR))
            {
                if(character->m_self->room->flags & TR_ROOM_FLAG_QUICKSAND)
                {
                    character->setAnimation(TR_ANIMATION_LARA_STAY_IDLE, 0);
                    Audio_Kill(TR_AUDIO_SOUND_LARASCREAM, TR_AUDIO_EMITTER_ENTITY, character->id());
                }
                else if(character->m_speed[2] <= -FREE_FALL_SPEED_MAXSAFE)
                {
                    if(!character->changeParam(PARAM_HEALTH, (character->m_speed[2] + FREE_FALL_SPEED_MAXSAFE) / 2))
                    {
                        resp->kill = 1;
                        character->setAnimation(TR_ANIMATION_LARA_LANDING_DEATH, 0);
                        Controls_JoyRumble(200.0, 500);
                    }
                    else
                    {
                        character->setAnimation(TR_ANIMATION_LARA_LANDING_HARD, 0);
                    }
                }
                else if(character->m_speed[2] <= -FREE_FALL_SPEED_2)
                {
                    character->setAnimation(TR_ANIMATION_LARA_LANDING_HARD, 0);
                }
                else
                {
                    character->setAnimation(TR_ANIMATION_LARA_LANDING_MIDDLE, 0);
                }

                if(resp->kill == 1)
                {
                    ss_anim->next_state = TR_STATE_LARA_DEATH;
                    Audio_Kill(TR_AUDIO_SOUND_LARASCREAM, TR_AUDIO_EMITTER_ENTITY, character->id());
                }
            }
            else if(cmd->action)
            {
                character->m_dirFlag = ENT_MOVE_FORWARD;
                ss_anim->next_state = TR_STATE_LARA_REACH;
            }
            break;

        case TR_STATE_LARA_SWANDIVE_BEGIN:
            cmd->rot[0] *= 0.4;
            if(resp->vertical_collide & 0x01 || character->m_moveType == MOVE_ON_FLOOR)
            {
                ss_anim->next_state = TR_STATE_LARA_STOP;                        // landing - roll
            }
            else if(character->m_moveType == MOVE_UNDERWATER)
            {
                ss_anim->next_state = TR_STATE_LARA_UNDERWATER_DIVING;
            }
            else
            {
                ss_anim->next_state = TR_STATE_LARA_SWANDIVE_END;                // next stage
            }
            break;

        case TR_STATE_LARA_SWANDIVE_END:
            cmd->rot[0] = 0.0;

            //Reset these to zero so Lara is only falling downwards
            character->m_speed[0] = 0.0;
            character->m_speed[1] = 0.0;

            if((resp->vertical_collide & 0x01) || (character->m_moveType == MOVE_ON_FLOOR))
            {
                if(curr_fc->quicksand)
                {
                    resp->kill = 1;
                    character->setParam(PARAM_HEALTH, 0.0);
                    character->setParam(PARAM_AIR, 0.0);
                    character->setAnimation(TR_ANIMATION_LARA_LANDING_DEATH, -1);
                }
                else
                {
                    character->setParam(PARAM_HEALTH, 0.0);
                    ss_anim->next_state = TR_STATE_LARA_DEATH;
                }
            }
            else if(character->m_moveType == MOVE_UNDERWATER)
            {
                ss_anim->next_state = TR_STATE_LARA_UNDERWATER_DIVING;
            }
            else if(cmd->jump)
            {
                ss_anim->next_state = TR_STATE_LARA_JUMP_ROLL;
            }
            break;

            /*
             * WATER ANIMATIONS
             */
        case TR_STATE_LARA_UNDERWATER_STOP:
            if(character->m_moveType != MOVE_UNDERWATER && character->m_moveType != MOVE_ON_WATER)
            {
                character->setAnimation(0, 0);
            }
            else if(resp->kill == 1)
            {
                ss_anim->next_state = TR_STATE_LARA_WATER_DEATH;
            }
            else if(cmd->roll)
            {
                character->setAnimation(TR_ANIMATION_LARA_UNDERWATER_ROLL_BEGIN, 0);
            }
            else if(cmd->jump)
            {
                ss_anim->next_state = TR_STATE_LARA_UNDERWATER_FORWARD;
            }
            break;

        case TR_STATE_LARA_WATER_DEATH:
            if(character->m_moveType != MOVE_ON_WATER)
            {
                pos[2] += (TR_METERING_SECTORSIZE / 4) * engine_frame_time;     // go to the air
            }
            break;


        case TR_STATE_LARA_UNDERWATER_FORWARD:
            if(character->m_moveType != MOVE_UNDERWATER && character->m_moveType != MOVE_ON_WATER)
            {
                character->setAnimation(0, 0);
            }
            else if(resp->kill == 1)
            {
                ss_anim->next_state = TR_STATE_LARA_WATER_DEATH;
            }
            else if(curr_fc->floor_hit && curr_fc->water && (curr_fc->transition_level - curr_fc->floor_point[2] <= character->m_maxStepUpHeight))
            {
                character->setAnimation(TR_ANIMATION_LARA_UNDERWATER_TO_WADE, 0); // go to the air
                ss_anim->next_state = TR_STATE_LARA_STOP;
                character->m_climb.point = curr_fc->floor_point;  ///@FIXME: without it Lara are pulled high up, but this string was not been here.
                character->m_moveType = MOVE_ON_FLOOR;
            }
            else if(cmd->roll)
            {
                character->setAnimation(TR_ANIMATION_LARA_UNDERWATER_ROLL_BEGIN, 0);
            }
            else if(cmd->jump)
            {
                if(character->m_moveType == MOVE_ON_WATER)
                {
                    character->m_inertiaLinear = 0.0;
                    ss_anim->next_state = TR_STATE_LARA_ONWATER_STOP;
                    character->setAnimation(TR_ANIMATION_LARA_UNDERWATER_TO_ONWATER, 0); // go to the air
                }
            }
            else
            {
                ss_anim->next_state = TR_STATE_LARA_UNDERWATER_INERTIA;
            }
            break;

        case TR_STATE_LARA_UNDERWATER_INERTIA:
            if(character->m_moveType == MOVE_ON_WATER)
            {
                character->m_inertiaLinear = 0.0;
                character->setAnimation(TR_ANIMATION_LARA_UNDERWATER_TO_ONWATER, 0); // go to the air
            }
            else if(resp->kill == 1)
            {
                ss_anim->next_state = TR_STATE_LARA_WATER_DEATH;
            }
            else if(cmd->roll)
            {
                character->setAnimation(TR_ANIMATION_LARA_UNDERWATER_ROLL_BEGIN, 0);
            }
            else if(cmd->jump)
            {
                ss_anim->next_state = TR_STATE_LARA_UNDERWATER_FORWARD;
            }
            else
            {
                ss_anim->next_state = TR_STATE_LARA_UNDERWATER_STOP;
            }
            break;

        case TR_STATE_LARA_ONWATER_STOP:
            if(cmd->action && (cmd->move[0] == 1) && (character->m_moveType != MOVE_CLIMBING))
            {
                t = LARA_TRY_HANG_WALL_OFFSET + LARA_HANG_WALL_DISTANCE;
                global_offset = character->m_transform.getBasis().getColumn(1) * t;
                global_offset[2] += LARA_HANG_VERTICAL_EPSILON;                        // inc for water_surf.z
                character->m_climb = character->checkClimbability(global_offset, &next_fc, 0.0);
                if(character->m_climb.edge_hit != ClimbType::NoClimb)
                {
                    low_vertical_space = true;
                }
                else
                {
                    low_vertical_space = false;
                    global_offset[2] += character->m_maxStepUpHeight + LARA_HANG_VERTICAL_EPSILON;
                    character->m_climb = character->checkClimbability(global_offset, &next_fc, 0.0);
                }

                if(character->m_climb.edge_hit != ClimbType::NoClimb && (character->m_climb.next_z_space >= character->m_height - LARA_HANG_VERTICAL_EPSILON))// && (character->m_climb.edge_point[2] - pos[2] < ent->character->max_step_up_height))   // max_step_up_height is not correct value here
                {
                    character->m_dirFlag = ENT_STAY;
                    character->m_moveType = MOVE_CLIMBING;
                    character->m_bt.no_fix_all = true;
                    character->m_angles[0] = character->m_climb.edge_z_ang;
                    character->updateTransform();
                    character->m_climb.point = character->m_climb.edge_point;
                }
            }

            if(character->m_moveType == MOVE_CLIMBING)
            {
                character->m_speed.setZero();
                cmd->rot[0] = 0.0;
                character->m_bt.no_fix_all = true;
                if(low_vertical_space)
                {
                    character->setAnimation(TR_ANIMATION_LARA_ONWATER_TO_LAND_LOW, 0);
                    ent_climb_out_of_water(character, ss_anim, ENTITY_ANIM_NEWANIM);
                }
                else
                {
                    ss_anim->next_state = TR_STATE_LARA_STOP;
                    ss_anim->onFrame = ent_climb_out_of_water;
                }
            }
            else if(resp->kill == 1)
            {
                ss_anim->next_state = TR_STATE_LARA_WATER_DEATH;
            }
            else if((cmd->move[0] == 1) || cmd->jump)                    // dive works correct only after TR_STATE_LARA_ONWATER_FORWARD
            {
                character->m_dirFlag = ENT_MOVE_FORWARD;
                ss_anim->next_state = TR_STATE_LARA_ONWATER_FORWARD;
            }
            else if(cmd->move[0] ==-1)
            {
                character->m_dirFlag = ENT_MOVE_BACKWARD;
                ss_anim->next_state = TR_STATE_LARA_ONWATER_BACK;
            }
            else if(cmd->move[1] ==-1)
            {
                if(cmd->shift)
                {
                    character->m_dirFlag = ENT_MOVE_LEFT;
                    cmd->rot[0] = 0.0;
                    ss_anim->next_state = TR_STATE_LARA_ONWATER_LEFT;
                }
                else
                {
                    // rotate on water
                }
            }
            else if(cmd->move[1] == 1)
            {
                if(cmd->shift)
                {
                    character->m_dirFlag = ENT_MOVE_RIGHT;
                    cmd->rot[0] = 0.0;
                    ss_anim->next_state = TR_STATE_LARA_ONWATER_RIGHT;
                }
                else
                {
                    // rotate on water
                }
            }
            else if(character->m_moveType == MOVE_UNDERWATER)
            {
                character->m_moveType = MOVE_ON_WATER;
            }
            break;

        case TR_STATE_LARA_ONWATER_FORWARD:
            character->m_moveType = MOVE_ON_WATER;
            if(resp->kill)
            {
                ss_anim->next_state = TR_STATE_LARA_WATER_DEATH;
            }
            else if(cmd->jump)
            {
                t = pos[2];
                Character::getHeightInfo(pos, &next_fc);
                pos[2] = t;
                ss_anim->next_state = TR_STATE_LARA_UNDERWATER_FORWARD;
                ss_anim->onFrame = ent_set_underwater;                          // dive
            }
            else if((cmd->move[0] == 1) && !cmd->action)
            {
                if(!curr_fc->floor_hit || (pos[2] - character->m_height > curr_fc->floor_point[2]- character->m_swimDepth))
                {
                    //ent->last_state = ent->last_state;                          // swim forward
                }
                else
                {
                    ss_anim->next_state = TR_STATE_LARA_WADE_FORWARD;
                    ss_anim->onFrame = ent_set_on_floor;                        // to wade
                }
            }
            else
            {
                ss_anim->next_state = TR_STATE_LARA_ONWATER_STOP;
            }
            break;

        case TR_STATE_LARA_ONWATER_BACK:
            if(cmd->move[0] == -1 && !cmd->jump)
            {
                if(!curr_fc->floor_hit || (curr_fc->floor_point[2] + character->m_height < curr_fc->transition_level))
                {
                    //ent->current_state = TR_STATE_CURRENT;                      // continue swimming
                }
                else
                {
                    ss_anim->next_state = TR_STATE_LARA_ONWATER_STOP;
                }
            }
            else
            {
                ss_anim->next_state = TR_STATE_LARA_ONWATER_STOP;
            }
            break;

        case TR_STATE_LARA_ONWATER_LEFT:
            cmd->rot[0] = 0.0;
            if(!cmd->jump)
            {
                if(cmd->move[1] ==-1 && cmd->shift)
                {
                    if(!curr_fc->floor_hit || (pos[2] - character->m_height > curr_fc->floor_point[2]))
                    {
                        // walk left
                        ss_anim->next_state = TR_STATE_LARA_ONWATER_LEFT;
                    }
                    else
                    {
                        // walk left
                        ss_anim->next_state = TR_STATE_LARA_WALK_LEFT;
                        ss_anim->onFrame = ent_set_on_floor;
                    }
                }
                else
                {
                    ss_anim->next_state = TR_STATE_LARA_ONWATER_STOP;
                }
            }
            else
            {
                ss_anim->next_state = TR_STATE_LARA_UNDERWATER_DIVING;
            }
            break;

        case TR_STATE_LARA_ONWATER_RIGHT:
            cmd->rot[0] = 0.0;
            if(!cmd->jump)
            {
                if(cmd->move[1] == 1 && cmd->shift)
                {
                    if(!curr_fc->floor_hit || (pos[2] - character->m_height > curr_fc->floor_point[2]))
                    {
                        // swim RIGHT
                        ss_anim->next_state = TR_STATE_LARA_ONWATER_RIGHT;
                    }
                    else
                    {
                        // walk left
                        ss_anim->next_state = TR_STATE_LARA_WALK_RIGHT;
                        ss_anim->onFrame = ent_set_on_floor;
                    }
                }
                else
                {
                    ss_anim->next_state = TR_STATE_LARA_ONWATER_STOP;
                }
            }
            else
            {
                ss_anim->next_state = TR_STATE_LARA_UNDERWATER_DIVING;
            }
            break;

            /*
             * CROUCH SECTION
             */
        case TR_STATE_LARA_CROUCH_IDLE:
            character->m_dirFlag = ENT_MOVE_FORWARD;
            character->m_bt.no_fix_body_parts = BODY_PART_HANDS_2 | BODY_PART_HANDS_3 | BODY_PART_LEGS_3;
            move[0] = pos[0];
            move[1] = pos[1];
            move[2] = pos[2] + 0.5 * (character->m_bf.bb_max[2] - character->m_bf.bb_min[2]);
            Character::getHeightInfo(move, &next_fc);

            character->lean(cmd, 0.0);

            if(!cmd->crouch && !low_vertical_space)
            {
                ss_anim->next_state = TR_STATE_LARA_STOP;                        // Back to stand
            }
            else if((cmd->move[0] != 0) || (resp->kill == 1))
            {
                ss_anim->next_state = TR_STATE_LARA_CRAWL_IDLE;                  // Both forward & back provoke crawl stage
            }
            else if(cmd->jump)
            {
                ss_anim->next_state = TR_STATE_LARA_CROUCH_ROLL;                 // Crouch roll
            }
            else
            {
                if(engine_world.version > TR_III)
                {
                    if(cmd->move[1] == 1)
                    {
                        character->m_dirFlag = ENT_MOVE_FORWARD;
                        ss_anim->next_state = TR_STATE_LARA_CROUCH_TURN_RIGHT;
                    }
                    else if(cmd->move[1] == -1)
                    {
                        character->m_dirFlag = ENT_MOVE_FORWARD;
                        ss_anim->next_state = TR_STATE_LARA_CROUCH_TURN_LEFT;
                    }
                }
                else
                {
                    cmd->rot[0] = 0.0;
                }
            }
            break;

        case TR_STATE_LARA_CROUCH_ROLL:
        case TR_STATE_LARA_SPRINT_ROLL:
            cmd->rot[0] = 0.0;
            character->lean(cmd, 0.0);
            if(character->m_moveType == MOVE_FREE_FALLING)
            {
                character->m_speed[0] *= 0.5;
                character->m_speed[1] *= 0.5;
                character->setAnimation(TR_ANIMATION_LARA_FREE_FALL_FORWARD, 0);
            }

            move = character->m_transform.getBasis().getColumn(1) * PENETRATION_TEST_OFFSET;
            if((character->checkNextPenetration(move) > 0) && (resp->horizontal_collide != 0x00))  // Smash into wall
            {
                ss_anim->next_state = TR_STATE_LARA_STOP;
            }
            break;

        case TR_STATE_LARA_CRAWL_IDLE:
            character->m_dirFlag = ENT_MOVE_FORWARD;
            character->m_bt.no_fix_body_parts = BODY_PART_HANDS_2 | BODY_PART_HANDS_3 | BODY_PART_LEGS_3;
            if(resp->kill == 1)
            {
                character->m_dirFlag = ENT_STAY;
                ss_anim->next_state = TR_STATE_LARA_DEATH;
            }
            else if(cmd->move[1] == -1)
            {
                character->m_dirFlag = ENT_MOVE_FORWARD;
                character->setAnimation(TR_ANIMATION_LARA_CRAWL_TURN_LEFT, 0);
            }
            else if(cmd->move[1] == 1)
            {
                character->m_dirFlag = ENT_MOVE_FORWARD;
                character->setAnimation(TR_ANIMATION_LARA_CRAWL_TURN_RIGHT, 0);
            }
            else if(cmd->move[0] == 1)
            {
                move = character->m_transform.getBasis().getColumn(1) * PENETRATION_TEST_OFFSET;
                if((character->checkNextPenetration(move) == 0) || (resp->horizontal_collide == 0x00))
                {
                    global_offset = character->m_transform.getBasis().getColumn(1) * CRAWL_FORWARD_OFFSET;
                    global_offset[2] += 0.5 * (character->m_bf.bb_max[2] + character->m_bf.bb_min[2]);
                    global_offset += pos;
                    Character::getHeightInfo(global_offset, &next_fc);
                    if((next_fc.floor_point[2] < pos[2] + character->m_minStepUpHeight) &&
                       (next_fc.floor_point[2] > pos[2] - character->m_minStepUpHeight))
                    {
                        ss_anim->next_state = TR_STATE_LARA_CRAWL_FORWARD;           // In TR4+, first state is crawlspace jump
                    }
                }
            }
            else if(cmd->move[0] == -1)
            {
                move = character->m_transform.getBasis().getColumn(1) * -PENETRATION_TEST_OFFSET;
                if((character->checkNextPenetration(move) == 0) || (resp->horizontal_collide == 0x00))
                {
                    global_offset = character->m_transform.getBasis().getColumn(1) * -CRAWL_FORWARD_OFFSET;
                    global_offset[2] += 0.5 * (character->m_bf.bb_max[2] + character->m_bf.bb_min[2]);
                    global_offset += pos;
                    Character::getHeightInfo(global_offset, &next_fc);
                    if((next_fc.floor_point[2] < pos[2] + character->m_minStepUpHeight) &&
                       (next_fc.floor_point[2] > pos[2] - character->m_minStepUpHeight))
                    {
                        character->m_dirFlag = ENT_MOVE_BACKWARD;
                        ss_anim->next_state = TR_STATE_LARA_CRAWL_BACK;
                    }
                    else if(cmd->action && (resp->horizontal_collide == 0) &&
                       (next_fc.floor_point[2] < pos[2] - character->m_height))
                    {
                        auto temp = pos;                                       // save entity position
                        pos[0] = next_fc.floor_point[0];
                        pos[1] = next_fc.floor_point[1];
                        global_offset = character->m_transform.getBasis().getColumn(1) * 0.5 * CRAWL_FORWARD_OFFSET;
                        global_offset[2] += 128.0;
                        curr_fc->floor_hit = next_fc.floor_hit;
                        curr_fc->floor_point = next_fc.floor_point;
                        curr_fc->floor_normale = next_fc.floor_normale;
                        curr_fc->floor_obj = next_fc.floor_obj;
                        curr_fc->ceiling_hit = next_fc.ceiling_hit;
                        curr_fc->ceiling_point = next_fc.ceiling_point;
                        curr_fc->ceiling_normale = next_fc.ceiling_normale;
                        curr_fc->ceiling_obj = next_fc.ceiling_obj;

                        character->m_climb = character->checkClimbability(global_offset, &next_fc, 1.5 * character->m_bf.bb_max[2]);
                        pos = temp;                                       // restore entity position
                        if(character->m_climb.can_hang)
                        {
                            character->m_angles[0] = character->m_climb.edge_z_ang;
                            character->m_dirFlag = ENT_MOVE_BACKWARD;
                            character->m_moveType = MOVE_CLIMBING;
                            character->m_climb.point = character->m_climb.edge_point;
                            ss_anim->next_state = TR_STATE_LARA_CRAWL_TO_CLIMB;
                        }
                    }
                }
            }
            else if(!cmd->crouch)
            {
                ss_anim->next_state = TR_STATE_LARA_CROUCH_IDLE;                // Back to crouch.
            }
            break;

        case TR_STATE_LARA_CRAWL_TO_CLIMB:
            character->m_bt.no_fix_all = true;
            ss_anim->onFrame = ent_crawl_to_climb;
            break;

        case TR_STATE_LARA_CRAWL_FORWARD:
            character->m_dirFlag = ENT_MOVE_FORWARD;
            character->m_bt.no_fix_body_parts = BODY_PART_HANDS_2 | BODY_PART_HANDS_3 | BODY_PART_LEGS_3;
            cmd->rot[0] = cmd->rot[0] * 0.5;
            move = character->m_transform.getBasis().getColumn(1) * PENETRATION_TEST_OFFSET;
            if((character->checkNextPenetration(move) > 0) && (resp->horizontal_collide != 0x00))
            {
                character->m_dirFlag = ENT_STAY;
                character->setAnimation(TR_ANIMATION_LARA_CRAWL_IDLE, 0);
                break;
            }
            global_offset = character->m_transform.getBasis().getColumn(1) * CRAWL_FORWARD_OFFSET;
            global_offset[2] += 0.5 * (character->m_bf.bb_max[2] + character->m_bf.bb_min[2]);
            global_offset += pos;
            Character::getHeightInfo(global_offset, &next_fc);

            if((cmd->move[0] != 1) || (resp->kill == 1))
            {
                ss_anim->next_state = TR_STATE_LARA_CRAWL_IDLE; // Stop
            }
            else if( (next_fc.floor_point[2] >= pos[2] + character->m_minStepUpHeight) ||
                     (next_fc.floor_point[2] <= pos[2] - character->m_minStepUpHeight)  )
            {
                character->m_dirFlag = ENT_STAY;
                character->setAnimation(TR_ANIMATION_LARA_CRAWL_IDLE, 0);
            }
            break;

        case TR_STATE_LARA_CRAWL_BACK:
            character->m_dirFlag = ENT_MOVE_FORWARD;   // Absurd? No, Core Design.
            character->m_bt.no_fix_body_parts = BODY_PART_HANDS_2 | BODY_PART_HANDS_3 | BODY_PART_LEGS_3;
            cmd->rot[0] = cmd->rot[0] * 0.5;
            move = character->m_transform.getBasis().getColumn(1) * -PENETRATION_TEST_OFFSET;
            if((character->checkNextPenetration(move) > 0) && (resp->horizontal_collide != 0x00))
            {
                character->m_dirFlag = ENT_STAY;
                character->setAnimation(TR_ANIMATION_LARA_CRAWL_IDLE, 0);
                break;
            }
            global_offset = character->m_transform.getBasis().getColumn(1) * -CRAWL_FORWARD_OFFSET;
            global_offset[2] += 0.5 * (character->m_bf.bb_max[2] + character->m_bf.bb_min[2]);
            global_offset += pos;
            Character::getHeightInfo(global_offset, &next_fc);
            if((cmd->move[0] != -1) || (resp->kill == 1))
            {
                ss_anim->next_state = TR_STATE_LARA_CRAWL_IDLE; // Stop
            }
            else if( (next_fc.floor_point[2] >= pos[2] + character->m_minStepUpHeight)   ||
                     (next_fc.floor_point[2] <= pos[2] - character->m_minStepUpHeight)    )
            {
                character->m_dirFlag = ENT_STAY;
                character->setAnimation(TR_ANIMATION_LARA_CRAWL_IDLE, 0);
            }
            break;

        case TR_STATE_LARA_CRAWL_TURN_LEFT:
            character->m_dirFlag = ENT_MOVE_FORWARD;
            character->m_bt.no_fix_body_parts = BODY_PART_HANDS_2 | BODY_PART_HANDS_3 | BODY_PART_LEGS_3;
            cmd->rot[0] *= ((ss_anim->current_frame > 3) && (ss_anim->current_frame < 14))?(1.0):(0.0);

            if((cmd->move[1] != -1) || (resp->kill == 1))
            {
                ss_anim->next_state = TR_STATE_LARA_CRAWL_IDLE; // stop
            }
            break;

        case TR_STATE_LARA_CRAWL_TURN_RIGHT:
            character->m_dirFlag = ENT_MOVE_FORWARD;
            character->m_bt.no_fix_body_parts = BODY_PART_HANDS_2 | BODY_PART_HANDS_3 | BODY_PART_LEGS_3;
            cmd->rot[0] *= ((ss_anim->current_frame > 3) && (ss_anim->current_frame < 14))?(1.0):(0.0);

            if((cmd->move[1] != 1) || (resp->kill == 1))
            {
                ss_anim->next_state = TR_STATE_LARA_CRAWL_IDLE; // stop
            }
            break;

        case TR_STATE_LARA_CROUCH_TURN_LEFT:
        case TR_STATE_LARA_CROUCH_TURN_RIGHT:
            character->m_bt.no_fix_body_parts = BODY_PART_HANDS_2 | BODY_PART_HANDS_3 | BODY_PART_LEGS_3;
            cmd->rot[0] *= ((ss_anim->current_frame > 3) && (ss_anim->current_frame < 23))?(0.6):(0.0);

            if((cmd->move[1] == 0) || (resp->kill == 1))
            {
                ss_anim->next_state = TR_STATE_LARA_CROUCH_IDLE;
            }
            break;

            /*
             * CLIMB MONKEY
             */
        case TR_STATE_LARA_MONKEYSWING_IDLE:
            cmd->rot[0] = 0.0;
            character->m_dirFlag = ENT_STAY;
            ///@FIXME: stick for TR_III+ monkey swing fix... something wrong with anim 150
            if(cmd->action && (character->m_moveType != MOVE_MONKEYSWING) && curr_fc->ceiling_climb && (curr_fc->ceiling_hit) && (pos[2] + character->m_bf.bb_max[2] > curr_fc->ceiling_point[2] - 96.0))
            {
                character->m_moveType = MOVE_MONKEYSWING;
                character->setAnimation(TR_ANIMATION_LARA_MONKEY_IDLE, 0);
                ss_anim->next_state = TR_STATE_LARA_MONKEYSWING_IDLE;
                pos[2] = character->m_heightInfo.ceiling_point[2] - character->m_bf.bb_max[2];
            }

            if((character->m_moveType != MOVE_MONKEYSWING) || !cmd->action)
            {
                character->setAnimation(TR_ANIMATION_LARA_TRY_HANG_VERTICAL, 0);
                character->m_dirFlag = ENT_STAY;
                character->m_moveType = MOVE_FREE_FALLING;
            }
            else if(cmd->shift && (cmd->move[1] ==-1))
            {
                ss_anim->next_state = TR_STATE_LARA_MONKEYSWING_LEFT;
            }
            else if(cmd->shift && (cmd->move[1] == 1))
            {
                ss_anim->next_state = TR_STATE_LARA_MONKEYSWING_RIGHT;
            }
            else if(cmd->move[0] == 1)
            {
                character->m_dirFlag = ENT_MOVE_FORWARD;
                ss_anim->next_state = TR_STATE_LARA_MONKEYSWING_FORWARD;
            }
            else if(cmd->move[1] ==-1)
            {
                ss_anim->next_state = TR_STATE_LARA_MONKEYSWING_TURN_LEFT;
            }
            else if(cmd->move[1] == 1)
            {
                ss_anim->next_state = TR_STATE_LARA_MONKEYSWING_TURN_RIGHT;
            }
            break;

        case TR_STATE_LARA_MONKEYSWING_TURN_LEFT:
            cmd->rot[0] *= 0.5;
            if((character->m_moveType != MOVE_MONKEYSWING) || !cmd->action)
            {
                character->setAnimation(TR_ANIMATION_LARA_TRY_HANG_VERTICAL, 0);
                character->m_dirFlag = ENT_STAY;
                character->m_moveType = MOVE_FREE_FALLING;
            }
            else if(cmd->move[1] != -1)
            {
                ss_anim->next_state = TR_STATE_LARA_MONKEYSWING_IDLE;
            }
            break;

        case TR_STATE_LARA_MONKEYSWING_TURN_RIGHT:
            cmd->rot[0] *= 0.5;
            if((character->m_moveType != MOVE_MONKEYSWING) || !cmd->action)
            {
                character->setAnimation(TR_ANIMATION_LARA_TRY_HANG_VERTICAL, 0);
                character->m_dirFlag = ENT_STAY;
                character->m_moveType = MOVE_FREE_FALLING;
            }
            else if(cmd->move[1] != 1)
            {
                ss_anim->next_state = TR_STATE_LARA_MONKEYSWING_IDLE;
            }
            break;

        case TR_STATE_LARA_MONKEYSWING_FORWARD:
            cmd->rot[0] *= 0.45;
            character->m_dirFlag = ENT_MOVE_FORWARD;

            if((character->m_moveType != MOVE_MONKEYSWING) || !cmd->action)
            {
                character->setAnimation(TR_ANIMATION_LARA_TRY_HANG_VERTICAL, 0);
                character->m_moveType = MOVE_FREE_FALLING;
            }
            else if(cmd->move[0] != 1)
            {
                ss_anim->next_state = TR_STATE_LARA_MONKEYSWING_IDLE;
            }
            break;

        case TR_STATE_LARA_MONKEYSWING_LEFT:
            cmd->rot[0] = 0.0;
            character->m_dirFlag = ENT_MOVE_LEFT;

            if((character->m_moveType != MOVE_MONKEYSWING) || !cmd->action)
            {
                character->setAnimation(TR_ANIMATION_LARA_TRY_HANG_VERTICAL, 0);
                character->m_moveType = MOVE_FREE_FALLING;
            }
            else if(cmd->move[0] != 1)
            {
                ss_anim->next_state = TR_STATE_LARA_MONKEYSWING_IDLE;
            }
            break;

        case TR_STATE_LARA_MONKEYSWING_RIGHT:
            cmd->rot[0] = 0.0;
            character->m_dirFlag = ENT_MOVE_RIGHT;

            if((character->m_moveType != MOVE_MONKEYSWING) || !cmd->action)
            {
                character->setAnimation(TR_ANIMATION_LARA_TRY_HANG_VERTICAL, 0);
                character->m_moveType = MOVE_FREE_FALLING;
            }
            else if(cmd->move[0] != 1)
            {
                ss_anim->next_state = TR_STATE_LARA_MONKEYSWING_IDLE;
            }
            break;

            /*
             * intermediate animations are processed automatically.
             */
        default:
            cmd->rot[0] = 0.0;
            if((character->m_moveType == MOVE_MONKEYSWING) || (character->m_moveType == MOVE_WALLS_CLIMB))
            {
                if(!cmd->action)
                {
                    character->setAnimation(TR_ANIMATION_LARA_START_FREE_FALL, 0);
                    character->m_dirFlag = ENT_STAY;
                    character->m_moveType = MOVE_FREE_FALLING;
                }
            }
            break;
    };

    /*
     * additional animations control
     */
    switch(ss_anim->current_animation)
    {
        case TR_ANIMATION_LARA_STAY_JUMP_SIDES:
            character->m_bt.no_fix_body_parts |= BODY_PART_HEAD;
            break;

        case TR_ANIMATION_LARA_TRY_HANG_SOLID:
        case TR_ANIMATION_LARA_FLY_FORWARD_TRY_HANG:
            if((character->m_moveType == MOVE_FREE_FALLING) && character->m_command.action &&
               (character->m_speed[0] * character->m_transform.getBasis().getColumn(1)[0] + character->m_speed[1] * character->m_transform.getBasis().getColumn(1)[1] < 0.0))
            {
                character->m_speed[0] = -character->m_speed[0];
                character->m_speed[1] = -character->m_speed[1];
            }
            break;
    };

    return 0;
}

/*
 * 187: wall climb up 2 hand
 * 188: wall climb down 2 hand
 * 189: stand mr. Poher
 * 194: 4 point climb to 2 hand climb
 * 201, 202 - right-left wall climb 4 point to 2 point
 * 395 4 point climb down to 2 hand climb // 2 ID
 * 421 - crawl to jump forward-down
 */
