#include "anim_state_control.h"

#include <cstdio>
#include <cstdlib>

#include "character_controller.h"
#include "engine/engine.h"
#include "loader/game.h"
#include "system.h"
#include "world/character.h"
#include "world/entity.h"
#include "world/resource.h"
#include "world/room.h"
#include "world/world.h"

#define LEFT_LEG                    (3)
#define RIGHT_LEG                   (6)

#define OSCILLATE_HANG_USE 0

namespace world
{
using animation::SSAnimation;

void ent_stop_traverse(Character* ent, SSAnimation* ss_anim, world::animation::AnimUpdate state)
{
    if(state == world::animation::AnimUpdate::NewAnim)
    {
        btVector3& v = ent->m_traversedObject->m_transform.getOrigin();
        int i = static_cast<int>(v[0] / MeteringSectorSize);
        v[0] = i * MeteringSectorSize + 512.0f;
        i = static_cast<int>(v[1] / MeteringSectorSize);
        v[1] = i * MeteringSectorSize + 512.0f;
        ent->m_traversedObject->updateRigidBody(true);
        ent->m_traversedObject = nullptr;
        ss_anim->onFrame = nullptr;
    }
}

void ent_set_on_floor(Character* ent, SSAnimation* ss_anim, world::animation::AnimUpdate state)
{
    if(state == world::animation::AnimUpdate::NewAnim)
    {
        ent->m_moveType = MoveType::OnFloor;
        ent->m_transform.getOrigin()[2] = ent->m_heightInfo.floor_point[2];
        ent->ghostUpdate();
        ss_anim->onFrame = nullptr;
    }
}

void ent_set_turn_fast(std::shared_ptr<world::Entity> ent, SSAnimation* ss_anim, world::animation::AnimUpdate state)
{
    if(state == world::animation::AnimUpdate::NewAnim)
    {
        ent->m_bf.animations.next_state = TR_STATE_LARA_TURN_FAST;
        ss_anim->onFrame = nullptr;
    }
}

void ent_set_on_floor_after_climb(Character* ent, SSAnimation* ss_anim, world::animation::AnimUpdate /*state*/)
{
    // FIXME: this is more like an end-of-anim operation
    if(ss_anim->current_animation != ss_anim->lerp_last_animation)
    {
        ent->m_transform.getOrigin() = ent->m_climb.point;

        // FIXME: position adjust after climb
        btVector3 climbfix(0, ent->m_climbR, 0);
        ent->m_transform.getOrigin() = ent->m_climb.point + ent->m_transform.getBasis() * climbfix;

        ent->m_moveType = MoveType::OnFloor;
        ss_anim->onFrame = nullptr;
    }
}

void ent_set_underwater(Character* ent, SSAnimation* ss_anim, world::animation::AnimUpdate state)
{
    if(state == world::animation::AnimUpdate::NewAnim)
    {
        ent->m_moveType = MoveType::Underwater;
        ss_anim->onFrame = nullptr;
    }
}

void ent_set_free_falling(Character* ent, SSAnimation* ss_anim, world::animation::AnimUpdate state)
{
    if(state == world::animation::AnimUpdate::NewAnim)
    {
        ent->m_moveType = MoveType::FreeFalling;
        ss_anim->onFrame = nullptr;
    }
}

void ent_set_cmd_slide(Character* ent, SSAnimation* ss_anim, world::animation::AnimUpdate state)
{
    if(state == world::animation::AnimUpdate::NewAnim)
    {
        ent->m_response.slide = SlideType::Back;
        ss_anim->onFrame = nullptr;
    }
}

void ent_correct_diving_angle(Character* ent, SSAnimation* ss_anim, world::animation::AnimUpdate state)
{
    if(state == world::animation::AnimUpdate::NewAnim)
    {
        ent->m_angles[1] = -45.0;
        ent->updateTransform();
        ss_anim->onFrame = nullptr;
    }
}

void ent_to_on_water(Character* ent, SSAnimation* ss_anim, world::animation::AnimUpdate state)
{
    if(state == world::animation::AnimUpdate::NewAnim)
    {
        ent->m_transform.getOrigin()[2] = ent->m_heightInfo.transition_level;
        ent->ghostUpdate();
        ent->m_moveType = MoveType::OnWater;
        ss_anim->onFrame = nullptr;
    }
}

void ent_climb_out_of_water(Character* ent, SSAnimation* ss_anim, world::animation::AnimUpdate state)
{
    if(state == world::animation::AnimUpdate::NewAnim)
    {
        const auto& v = ent->m_climb.point;

        ent->m_transform.getOrigin() = v + ent->m_transform.getBasis().getColumn(1) * 48.0;             // temporary stick
        ent->m_transform.getOrigin()[2] = v[2];
        ent->ghostUpdate();
        ss_anim->onFrame = nullptr;
    }
}

void ent_to_edge_climb(Character* ent, SSAnimation* ss_anim, world::animation::AnimUpdate state)
{
    if(state == world::animation::AnimUpdate::NewAnim)
    {
        btScalar *v = ent->m_climb.point;

        ent->m_transform.getOrigin()[0] = v[0] - ent->m_transform.getBasis().getColumn(1)[0] * ent->m_bf.boundingBox.max[1];
        ent->m_transform.getOrigin()[1] = v[1] - ent->m_transform.getBasis().getColumn(1)[1] * ent->m_bf.boundingBox.max[1];
        ent->m_transform.getOrigin()[2] = v[2] - ent->m_bf.boundingBox.max[2];
        ent->ghostUpdate();
        ss_anim->onFrame = nullptr;
    }
}

void ent_to_monkey_swing(Character* ent, SSAnimation* ss_anim, world::animation::AnimUpdate state)
{
    if(state == world::animation::AnimUpdate::NewAnim)
    {
        ent->m_moveType = MoveType::Monkeyswing;
        ent->m_transform.getOrigin()[2] = ent->m_heightInfo.ceiling_point[2] - ent->m_bf.boundingBox.max[2];
        ent->ghostUpdate();
        ss_anim->onFrame = nullptr;
    }
}

void ent_to_tightrope(Character* ent, SSAnimation* ss_anim, world::animation::AnimUpdate state)
{
    if(state == world::animation::AnimUpdate::NewAnim)
    {
        ent->m_moveType = MoveType::Climbing;
        ent->ghostUpdate();
        ss_anim->onFrame = nullptr;
    }
}

void ent_from_tightrope(Character* ent, SSAnimation* ss_anim, world::animation::AnimUpdate state)
{
    if(state == world::animation::AnimUpdate::NewAnim)
    {
        ent->m_moveType = MoveType::OnFloor;
        ent->ghostUpdate();
        ss_anim->onFrame = nullptr;
    }
}

void ent_crawl_to_climb(Character* ent, SSAnimation* ss_anim, world::animation::AnimUpdate state)
{
    if(state == world::animation::AnimUpdate::NewAnim)
    {
        CharacterCommand* cmd = &ent->m_command;

        if(!cmd->action)
        {
            ent->setAnimation(TR_ANIMATION_LARA_START_FREE_FALL, 0);
            ent->m_moveType = MoveType::FreeFalling;
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

// State control is based on original TR system, where several animations may
// belong to the same state.

void Character::state_func()
{
    int i;
    btVector3& pos = m_transform.getOrigin();
    btScalar t;
    btVector3 global_offset, move;

    HeightInfo next_fc, *curr_fc;
    curr_fc = &m_heightInfo;
    next_fc.sp = curr_fc->sp;
    next_fc.cb = m_rayCb;
    next_fc.cb->m_closestHitFraction = 1.0;
    next_fc.cb->m_collisionObject = nullptr;
    next_fc.ccb = m_convexCb;
    next_fc.ccb->m_closestHitFraction = 1.0;
    next_fc.ccb->m_hitCollisionObject = nullptr;
    m_bt.no_fix_body_parts = 0x00000000;

    m_bf.animations.mode = animation::SSAnimationMode::NormalControl;
    updateCurrentHeight();

    bool low_vertical_space = (curr_fc->floor_hit && curr_fc->ceiling_hit && (curr_fc->ceiling_point[2] - curr_fc->floor_point[2] < m_height - engine::LaraHangVerticalEpsilon));
    const bool last_frame = static_cast<int>(m_bf.animations.model->animations[m_bf.animations.current_animation].frames.size()) <= m_bf.animations.current_frame + 1;

    if(m_response.killed)  // Stop any music, if Lara is dead.
    {
        engine::engine_world.endStreams(audio::StreamType::Oneshot);
        engine::engine_world.endStreams(audio::StreamType::Chat);
    }

    StepType nextStep = StepType::Horizontal;


    switch(m_bf.animations.last_state)
    {

        // Normal land animations

        case TR_STATE_LARA_STOP:

            // Reset directional flag only on intermediate animation!

            if(m_bf.animations.current_animation == TR_ANIMATION_LARA_STAY_SOLID)
            {
                m_dirFlag = ENT_STAY;
            }

            if(m_moveType == MoveType::OnFloor)
                m_bt.no_fix_body_parts = BODY_PART_HANDS | BODY_PART_LEGS;

            m_command.rot[0] = 0;
            m_command.crouch |= low_vertical_space;
            lean(0.0);

            if((m_climb.can_hang &&
                (m_climb.next_z_space >= m_height - engine::LaraHangVerticalEpsilon) &&
                (m_moveType == MoveType::Climbing)) ||
               (m_bf.animations.current_animation == TR_ANIMATION_LARA_STAY_SOLID))
            {
                m_moveType = MoveType::OnFloor;
            }

            if(m_moveType == MoveType::OnFloor)
            {
                m_bt.no_fix_body_parts = BODY_PART_LEGS_2 | BODY_PART_LEGS_3;
            }

            if(m_moveType == MoveType::FreeFalling)
            {
                setAnimation(TR_ANIMATION_LARA_START_FREE_FALL, 0);
                m_dirFlag = ENT_STAY;
            }
            else if(m_response.killed)
            {
                m_bf.animations.next_state = TR_STATE_LARA_DEATH;
            }
            else if(m_response.slide == SlideType::Front)
            {
                audio::send(TR_AUDIO_SOUND_LANDING, audio::EmitterType::Entity, id());

                if(m_command.jump)
                {
                    m_dirFlag = ENT_MOVE_FORWARD;
                    setAnimation(TR_ANIMATION_LARA_JUMP_FORWARD_BEGIN, 0);
                }
                else
                {
                    setAnimation(TR_ANIMATION_LARA_SLIDE_FORWARD, 0);
                }
            }
            else if(m_response.slide == SlideType::Back)
            {
                if(m_command.jump)
                {
                    m_dirFlag = ENT_MOVE_BACKWARD;
                    setAnimation(TR_ANIMATION_LARA_JUMP_BACK_BEGIN, 0);
                    audio::send(TR_AUDIO_SOUND_LANDING, audio::EmitterType::Entity, id());
                }
                else
                {
                    setAnimation(TR_ANIMATION_LARA_START_SLIDE_BACKWARD, 0);
                }
            }
            else if(m_command.jump)
            {
                if(curr_fc->quicksand == QuicksandPosition::None)
                    m_bf.animations.next_state = TR_STATE_LARA_JUMP_PREPARE;  // Jump sideways
            }
            else if(m_command.roll)
            {
                if(curr_fc->quicksand == QuicksandPosition::None && m_bf.animations.current_animation != TR_ANIMATION_LARA_CLIMB_2CLICK)
                {
                    m_dirFlag = ENT_MOVE_FORWARD;
                    setAnimation(TR_ANIMATION_LARA_ROLL_BEGIN, 0);
                }
            }
            else if(m_command.crouch)
            {
                if(curr_fc->quicksand == QuicksandPosition::None)
                    m_bf.animations.next_state = TR_STATE_LARA_CROUCH_IDLE;
            }
            else if(m_command.action && findTraverse())
            {
                m_bf.animations.next_state = TR_STATE_LARA_PUSHABLE_GRAB;
                if(m_transform.getBasis().getColumn(1).x() > 0.9)
                {
                    t = -m_traversedObject->m_bf.boundingBox.min[0] + 72.0f;
                }
                else if(m_transform.getBasis().getColumn(1).x() < -0.9)
                {
                    t = m_traversedObject->m_bf.boundingBox.max[0] + 72.0f;
                }
                else if(m_transform.getBasis().getColumn(1).y() > 0.9)
                {
                    t = -m_traversedObject->m_bf.boundingBox.min[1] + 72.0f;
                }
                else if(m_transform.getBasis().getColumn(1).y() < -0.9)
                {
                    t = m_traversedObject->m_bf.boundingBox.max[1] + 72.0f;
                }
                else
                {
                    t = 512.0 + 72.0;  ///@PARANOID
                }
                const btVector3& v = m_traversedObject->m_transform.getOrigin();
                pos[0] = v[0] - m_transform.getBasis().getColumn(1).x() * t;
                pos[1] = v[1] - m_transform.getBasis().getColumn(1).y() * t;
            }
            else if(m_command.move[0] == 1)
            {
                if(m_command.shift)
                {
                    move = m_transform.getBasis().getColumn(1) * engine::PenetrationTestOffset;
                    global_offset = m_transform.getBasis().getColumn(1) * engine::WalkForwardOffset;
                    global_offset[2] += m_bf.boundingBox.max[2];
                    global_offset += pos;
                    Character::getHeightInfo(global_offset, &next_fc);
                    if(((checkNextPenetration(move) == 0) || (m_response.horizontal_collide == 0x00)) &&
                       (next_fc.floor_hit && (next_fc.floor_point[2] > pos[2] - m_maxStepUpHeight) && (next_fc.floor_point[2] <= pos[2] + m_maxStepUpHeight)))
                    {
                        m_moveType = MoveType::OnFloor;
                        m_dirFlag = ENT_MOVE_FORWARD;
                        if((curr_fc->water || curr_fc->quicksand != QuicksandPosition::None) && curr_fc->floor_hit && (curr_fc->transition_level - curr_fc->floor_point[2] > m_wadeDepth))
                        {
                            m_bf.animations.next_state = TR_STATE_LARA_WADE_FORWARD;
                        }
                        else
                        {
                            m_bf.animations.next_state = TR_STATE_LARA_WALK_FORWARD;
                        }
                    }
                }       // end IF CMD->SHIFT
                else
                {
                    move = m_transform.getBasis().getColumn(1) * engine::PenetrationTestOffset;
                    global_offset = m_transform.getBasis().getColumn(1) * engine::RunForwardOffset;
                    global_offset[2] += m_bf.boundingBox.max[2];
                    checkNextStep(global_offset, &next_fc);
                    if(((checkNextPenetration(move) == 0) || (m_response.horizontal_collide == 0x00)) && !hasStopSlant(next_fc))
                    {
                        m_moveType = MoveType::OnFloor;
                        m_dirFlag = ENT_MOVE_FORWARD;
                        if((curr_fc->water || curr_fc->quicksand != QuicksandPosition::None) && curr_fc->floor_hit && (curr_fc->transition_level - curr_fc->floor_point[2] > m_wadeDepth))
                        {
                            m_bf.animations.next_state = TR_STATE_LARA_WADE_FORWARD;
                        }
                        else
                        {
                            m_bf.animations.next_state = TR_STATE_LARA_RUN_FORWARD;
                        }
                    }
                }

                if(m_command.action &&
                   ((m_bf.animations.current_animation == TR_ANIMATION_LARA_STAY_IDLE) ||
                    (m_bf.animations.current_animation == TR_ANIMATION_LARA_STAY_SOLID) ||
                    (m_bf.animations.current_animation == TR_ANIMATION_LARA_WALL_SMASH_LEFT) ||
                    (m_bf.animations.current_animation == TR_ANIMATION_LARA_WALL_SMASH_RIGHT)))
                {
                    t = m_forwardSize + engine::LaraTryHangWallOffset;
                    global_offset = m_transform.getBasis().getColumn(1) * t;

                    global_offset[2] += 0.5 * DEFAULT_CLIMB_UP_HEIGHT;
                    m_climb = checkClimbability(global_offset, &next_fc, 0.5 * DEFAULT_CLIMB_UP_HEIGHT);
                    if(m_climb.edge_hit &&
                       (m_climb.next_z_space >= m_height - engine::LaraHangVerticalEpsilon) &&
                       (pos[2] + m_maxStepUpHeight < next_fc.floor_point[2]) &&
                       (pos[2] + 2944.0 >= next_fc.floor_point[2]) &&
                       (next_fc.floor_normale[2] >= m_criticalSlantZComponent)) // trying to climb on
                    {
                        if(pos[2] + 640.0 >= next_fc.floor_point[2])
                        {
                            m_angles[0] = m_climb.edge_z_ang;
                            pos[2] = next_fc.floor_point[2] - 512.0f;
                            m_climb.point = next_fc.floor_point;
                            setAnimation(TR_ANIMATION_LARA_CLIMB_2CLICK, 0);
                            m_bt.no_fix_all = true;
                            m_bf.animations.onFrame = ent_set_on_floor_after_climb;
                            break;
                        }
                        else if(pos[2] + 896.0 >= next_fc.floor_point[2])
                        {
                            m_angles[0] = m_climb.edge_z_ang;
                            pos[2] = next_fc.floor_point[2] - 768.0f;
                            m_climb.point = next_fc.floor_point;
                            setAnimation(TR_ANIMATION_LARA_CLIMB_3CLICK, 0);
                            m_bt.no_fix_all = true;
                            m_bf.animations.onFrame = ent_set_on_floor_after_climb;
                            break;
                        }
                    }   // end IF MOVE_LITTLE_CLIMBING

                    global_offset[2] += 0.5 * DEFAULT_CLIMB_UP_HEIGHT;
                    m_climb = checkClimbability(global_offset, &next_fc, DEFAULT_CLIMB_UP_HEIGHT);
                    if(m_climb.edge_hit &&
                       (m_climb.next_z_space >= m_height - engine::LaraHangVerticalEpsilon) &&
                       (pos[2] + m_maxStepUpHeight < next_fc.floor_point[2]) &&
                       (pos[2] + 2944.0 >= next_fc.floor_point[2]))  // Trying to climb on
                    {
                        if(pos[2] + 1920.0 >= next_fc.floor_point[2])
                        {
                            // Fixme: grabheight/gravity values
                            const btScalar grabheight = 800.0f;  // Lara arms-up...estimated
                            const btScalar distance = next_fc.floor_point[2]-pos[2] - grabheight;
                            const btScalar gravity = 6;          // based on tr gravity accel (6 units / tick^2)
                            m_vspeed_override = 3.0f + sqrt(gravity * 2.0f * distance);
                            m_bf.animations.next_state = TR_STATE_LARA_JUMP_UP;
                            break;
                        }
                    }   // end IF MOVE_BIG_CLIMBING

                    m_climb = checkWallsClimbability();
                    if(m_climb.wall_hit != ClimbType::None)
                    {
                        m_bf.animations.next_state = TR_STATE_LARA_JUMP_UP;
                        break;
                    }
                }
            }       // end CMD->MOVE FORWARD
            else if(m_command.move[0] == -1)
            {
                if(m_command.shift)
                {
                    move = m_transform.getBasis().getColumn(1) * -engine::PenetrationTestOffset;
                    if((checkNextPenetration(move) == 0) || (m_response.horizontal_collide == 0x00))
                    {
                        global_offset = m_transform.getBasis().getColumn(1) * -engine::WalkBackOffset;
                        global_offset[2] += m_bf.boundingBox.max[2];
                        global_offset += pos;
                        Character::getHeightInfo(global_offset, &next_fc);
                        if((next_fc.floor_hit && (next_fc.floor_point[2] > pos[2] - m_maxStepUpHeight) && (next_fc.floor_point[2] <= pos[2] + m_maxStepUpHeight)))
                        {
                            m_dirFlag = ENT_MOVE_BACKWARD;
                            m_bf.animations.next_state = TR_STATE_LARA_WALK_BACK;
                        }
                    }
                }
                else    // RUN BACK
                {
                    move = m_transform.getBasis().getColumn(1) * -engine::PenetrationTestOffset;
                    if((checkNextPenetration(move) == 0) || (m_response.horizontal_collide == 0x00))
                    {
                        m_dirFlag = ENT_MOVE_BACKWARD;
                        if((curr_fc->water || curr_fc->quicksand != QuicksandPosition::None) && curr_fc->floor_hit && (curr_fc->transition_level - curr_fc->floor_point[2] > m_wadeDepth))
                        {
                            m_bf.animations.next_state = TR_STATE_LARA_WALK_BACK;
                        }
                        else
                        {
                            m_bf.animations.next_state = TR_STATE_LARA_RUN_BACK;
                        }
                    }
                }
            }       // end CMD->MOVE BACK
            else if(m_command.move[1] == 1)
            {
                if(m_command.shift)
                {
                    move = m_transform.getBasis().getColumn(0) * engine::PenetrationTestOffset;
                    if((checkNextPenetration(move) == 0) || (m_response.horizontal_collide == 0x00))
                    {
                        global_offset = m_transform.getBasis().getColumn(0) * engine::RunForwardOffset;
                        global_offset[2] += m_bf.boundingBox.max[2];
                        if((m_response.horizontal_collide == 0) && isLittleStep(checkNextStep(global_offset, &next_fc)))
                        {
                            m_command.rot[0] = 0.0;
                            m_dirFlag = ENT_MOVE_RIGHT;
                            m_bf.animations.next_state = TR_STATE_LARA_WALK_RIGHT;
                        }
                    }
                }       //end IF CMD->SHIFT
                else
                {
                    m_bf.animations.next_state = TR_STATE_LARA_TURN_RIGHT_SLOW;
                }
            }       // end MOVE RIGHT
            else if(m_command.move[1] == -1)
            {
                if(m_command.shift)
                {
                    move = m_transform.getBasis().getColumn(0) * -engine::PenetrationTestOffset;
                    if((checkNextPenetration(move) == 0) || (m_response.horizontal_collide == 0x00))
                    {
                        global_offset = m_transform.getBasis().getColumn(0) * -engine::RunForwardOffset;
                        global_offset[2] += m_bf.boundingBox.max[2];
                        if((m_response.horizontal_collide == 0) && isLittleStep(checkNextStep(global_offset, &next_fc)))
                        {
                            m_command.rot[0] = 0.0;
                            m_dirFlag = ENT_MOVE_LEFT;
                            m_bf.animations.next_state = TR_STATE_LARA_WALK_LEFT;
                        }
                    }
                }       //end IF CMD->SHIFT
                else
                {
                    m_bf.animations.next_state = TR_STATE_LARA_TURN_LEFT_SLOW;
                }
            }       // end MOVE LEFT
            break;

        case TR_STATE_LARA_JUMP_PREPARE:

            m_bt.no_fix_body_parts = BODY_PART_LEGS | BODY_PART_HANDS | BODY_PART_HEAD;
            m_command.rot[0] = 0;
            lean(0.0);

            if(m_response.slide == SlideType::Back)      // Slide checking is only for jumps direction correction!
            {
                setAnimation(TR_ANIMATION_LARA_JUMP_BACK_BEGIN, 0);
                m_command.move[0] = -1;
            }
            else if(m_response.slide == SlideType::Front)
            {
                setAnimation(TR_ANIMATION_LARA_JUMP_FORWARD_BEGIN, 0);
                m_command.move[0] = 1;
            }
            if((curr_fc->water || curr_fc->quicksand != QuicksandPosition::None) && curr_fc->floor_hit && (curr_fc->transition_level - curr_fc->floor_point[2] > m_wadeDepth))
            {
                //Stay, directional jumps are not allowed whilst in wade depth
            }
            else if(m_command.move[0] == 1)
            {
                m_dirFlag = ENT_MOVE_FORWARD;
                move = m_transform.getBasis().getColumn(1) * engine::PenetrationTestOffset;
                if(checkNextPenetration(move) == 0)
                {
                    m_bf.animations.next_state = TR_STATE_LARA_JUMP_FORWARD;  // Jump forward
                }
            }
            else if(m_command.move[0] == -1)
            {
                m_dirFlag = ENT_MOVE_BACKWARD;
                move = m_transform.getBasis().getColumn(1) * -engine::PenetrationTestOffset;
                if(checkNextPenetration(move) == 0)
                {
                    m_bf.animations.next_state = TR_STATE_LARA_JUMP_BACK;  // Jump backward
                }
            }
            else if(m_command.move[1] == 1)
            {
                m_dirFlag = ENT_MOVE_RIGHT;
                move = m_transform.getBasis().getColumn(0) * engine::PenetrationTestOffset;
                if(checkNextPenetration(move) == 0)
                {
                    m_bf.animations.next_state = TR_STATE_LARA_JUMP_LEFT;  // Jump right
                }
            }
            else if(m_command.move[1] == -1)
            {
                m_dirFlag = ENT_MOVE_LEFT;
                move = m_transform.getBasis().getColumn(0) * -engine::PenetrationTestOffset;
                if(checkNextPenetration(move) == 0)
                {
                    m_bf.animations.next_state = TR_STATE_LARA_JUMP_RIGHT;  // Jump left
                }
            }
            break;

        case TR_STATE_LARA_JUMP_BACK:

            m_bt.no_fix_body_parts = BODY_PART_LEGS | BODY_PART_HANDS | BODY_PART_HEAD;
            m_command.rot[0] = 0.0;

            if(m_response.vertical_collide & 0x01 || m_moveType == MoveType::OnFloor)
            {
                if(curr_fc->quicksand != QuicksandPosition::None)
                {
                    setAnimation(TR_ANIMATION_LARA_STAY_IDLE, 0);
                }
                else
                {
                    m_bf.animations.next_state = TR_STATE_LARA_STOP;  // Landing
                }
            }
            else if(m_response.horizontal_collide & 0x01)
            {
                engine::Controls_JoyRumble(200.0, 200);
                setAnimation(TR_ANIMATION_LARA_SMASH_JUMP, 0);
                m_dirFlag = ENT_MOVE_FORWARD;
                updateCurrentSpeed(true);
            }
            else if((m_moveType == MoveType::Underwater) || (m_speed[2] <= -FREE_FALL_SPEED_2))
            {
                m_bf.animations.next_state = TR_STATE_LARA_FREEFALL;  // Free falling
            }
            else if(m_command.roll)
            {
                m_bf.animations.next_state = TR_STATE_LARA_JUMP_ROLL;
            }
            break;

        case TR_STATE_LARA_JUMP_LEFT:

            m_bt.no_fix_body_parts = BODY_PART_LEGS | BODY_PART_HANDS | BODY_PART_HEAD;
            m_command.rot[0] = 0.0;

            if(m_response.vertical_collide & 0x01 || m_moveType == MoveType::OnFloor)
            {
                if(curr_fc->quicksand != QuicksandPosition::None)
                {
                    setAnimation(TR_ANIMATION_LARA_STAY_IDLE, 0);
                }
                else
                {
                    m_bf.animations.next_state = TR_STATE_LARA_STOP;  // Landing
                }
            }
            else if(m_response.horizontal_collide & 0x01)
            {
                engine::Controls_JoyRumble(200.0, 200);
                setAnimation(TR_ANIMATION_LARA_SMASH_JUMP, 0);
                m_dirFlag = ENT_MOVE_RIGHT;
                updateCurrentSpeed(true);
            }
            else
            {
                m_bf.animations.next_state = TR_STATE_LARA_FREEFALL;
            }
            break;

        case TR_STATE_LARA_JUMP_RIGHT:

            m_bt.no_fix_body_parts = BODY_PART_LEGS | BODY_PART_HANDS | BODY_PART_HEAD;
            m_command.rot[0] = 0.0;

            if(m_response.vertical_collide & 0x01 || m_moveType == MoveType::OnFloor)
            {
                if(curr_fc->quicksand != QuicksandPosition::None)
                {
                    setAnimation(TR_ANIMATION_LARA_STAY_IDLE, 0);
                }
                else
                {
                    m_bf.animations.next_state = TR_STATE_LARA_STOP;  // Landing
                }
            }
            else if(m_response.horizontal_collide & 0x01)
            {
                engine::Controls_JoyRumble(200.0, 200);
                setAnimation(TR_ANIMATION_LARA_SMASH_JUMP, 0);
                m_dirFlag = ENT_MOVE_LEFT;
                updateCurrentSpeed(true);
            }
            else
            {
                m_bf.animations.next_state = TR_STATE_LARA_FREEFALL;
            }
            break;

        case TR_STATE_LARA_RUN_BACK:
            m_dirFlag = ENT_MOVE_BACKWARD;

            if(m_moveType == MoveType::FreeFalling)
            {
                m_dirFlag = ENT_MOVE_FORWARD;
                setAnimation(TR_ANIMATION_LARA_FREE_FALL_BACK, 0);
            }
            else if(m_response.horizontal_collide & 0x01)
            {
                setAnimation(TR_ANIMATION_LARA_CLIMB_2CLICK_END, 0);
            }
            break;

        case TR_STATE_LARA_TURN_LEFT_SLOW:
        case TR_STATE_LARA_TURN_RIGHT_SLOW:
            m_command.rot[0] *= 0.7f;
            m_dirFlag = ENT_STAY;
            lean(0.0);
            m_bt.no_fix_body_parts = BODY_PART_LEGS_2 | BODY_PART_LEGS_3;

            if(m_command.move[0] == 1)
            {
                Substance substance_state = getSubstanceState();
                if((substance_state == Substance::None) ||
                   (substance_state == Substance::WaterShallow))
                {
                    if(m_command.shift)
                    {
                        m_bf.animations.next_state = TR_STATE_LARA_WALK_FORWARD;
                        m_dirFlag = ENT_MOVE_FORWARD;
                    }
                    else
                    {
                        m_bf.animations.next_state = TR_STATE_LARA_RUN_FORWARD;
                        m_dirFlag = ENT_MOVE_FORWARD;
                    }
                }
                else
                {
                    m_bf.animations.next_state = TR_STATE_LARA_WADE_FORWARD;
                    m_dirFlag = ENT_MOVE_FORWARD;
                }
            }
            else if(((m_bf.animations.last_state == TR_STATE_LARA_TURN_LEFT_SLOW) && (m_command.move[1] == -1)) ||
                    ((m_bf.animations.last_state == TR_STATE_LARA_TURN_RIGHT_SLOW) && (m_command.move[1] == 1)))
            {
                Substance substance_state = getSubstanceState();
                if(last_frame &&
                   (substance_state != Substance::WaterWade) &&
                   (substance_state != Substance::QuicksandConsumed) &&
                   (substance_state != Substance::QuicksandShallow))
                {
                    m_bf.animations.next_state = TR_STATE_LARA_TURN_FAST;
                }
            }
            else
            {
                m_bf.animations.next_state = TR_STATE_LARA_STOP;
            }
            break;

        case TR_STATE_LARA_TURN_FAST:
            // 65 - wade
            m_dirFlag = ENT_STAY;
            m_bt.no_fix_body_parts = BODY_PART_LEGS_2 | BODY_PART_LEGS_3;
            lean(0.0);

            if(m_moveType == MoveType::FreeFalling)
            {
                setAnimation(TR_ANIMATION_LARA_START_FREE_FALL, 0);
            }
            else if(m_command.move[0] == 1 && !m_command.jump && !m_command.crouch && m_command.shift)
            {
                m_bf.animations.next_state = TR_STATE_LARA_WALK_FORWARD;
                m_dirFlag = ENT_MOVE_FORWARD;
            }
            else if(m_command.move[0] == 1 && !m_command.jump && !m_command.crouch && !m_command.shift)
            {
                m_bf.animations.next_state = TR_STATE_LARA_RUN_FORWARD;
                m_dirFlag = ENT_MOVE_FORWARD;
            }
            else if(m_command.move[1] == 0)
            {
                m_bf.animations.next_state = TR_STATE_LARA_STOP;
            }
            break;

            // Run and walk animations

        case TR_STATE_LARA_RUN_FORWARD:
            global_offset = m_transform.getBasis().getColumn(1) * engine::RunForwardOffset;
            global_offset[2] += m_bf.boundingBox.max[2];
            nextStep = checkNextStep(global_offset, &next_fc);
            m_dirFlag = ENT_MOVE_FORWARD;
            m_command.crouch |= low_vertical_space;

            if(m_moveType == MoveType::OnFloor)
                m_bt.no_fix_body_parts = BODY_PART_HANDS | BODY_PART_LEGS;

            lean(6.0);

            if(m_moveType == MoveType::FreeFalling)
            {
                setAnimation(TR_ANIMATION_LARA_FREE_FALL_FORWARD, 0);
            }
            else if(m_response.killed)
            {
                m_bf.animations.next_state = TR_STATE_LARA_DEATH;
            }
            else if(m_response.slide == SlideType::Front)
            {
                setAnimation(TR_ANIMATION_LARA_SLIDE_FORWARD, 0);
            }
            else if(m_response.slide == SlideType::Back)
            {
                setAnimation(TR_ANIMATION_LARA_START_SLIDE_BACKWARD, 0);
                m_dirFlag = ENT_MOVE_BACKWARD;
            }
            else if(hasStopSlant(next_fc))
            {
                m_dirFlag = ENT_STAY;
                setAnimation(TR_ANIMATION_LARA_STAY_IDLE, 0);
            }
            else if(m_command.crouch)
            {
                m_bf.animations.next_state = TR_STATE_LARA_CROUCH_IDLE;
            }
            else if((m_command.move[0] == 1) && !m_command.crouch && (next_fc.floor_normale[2] >= m_criticalSlantZComponent) && nextStep == StepType::UpBig)
            {
                m_dirFlag = ENT_STAY;
                i = getAnimDispatchCase(2);  // Select correct anim dispatch.
                if(i == 0)
                {
                    setAnimation(TR_ANIMATION_LARA_RUN_UP_STEP_RIGHT, 0);
                    pos[2] = next_fc.floor_point[2];
                    m_dirFlag = ENT_MOVE_FORWARD;
                }
                else //if(i == 1)
                {
                    setAnimation(TR_ANIMATION_LARA_RUN_UP_STEP_LEFT, 0);
                    pos[2] = next_fc.floor_point[2];
                    m_dirFlag = ENT_MOVE_FORWARD;
                }
            }
            else if(m_response.horizontal_collide & 0x01)
            {
                global_offset = m_transform.getBasis().getColumn(1) * engine::RunForwardOffset;
                global_offset[2] += 1024.0;
                if(m_bf.animations.current_animation == TR_ANIMATION_LARA_STAY_TO_RUN)
                {
                    setAnimation(TR_ANIMATION_LARA_STAY_IDLE, 0);
                }
                else
                {
                    engine::Controls_JoyRumble(200.0, 200);

                    if(m_command.move[0] == 1)
                    {
                        i = getAnimDispatchCase(2);
                        if(i == 1)
                        {
                            setAnimation(TR_ANIMATION_LARA_WALL_SMASH_LEFT, 0);
                        }
                        else
                        {
                            setAnimation(TR_ANIMATION_LARA_WALL_SMASH_RIGHT, 0);
                        }
                    }
                    else
                    {
                        setAnimation(TR_ANIMATION_LARA_STAY_SOLID, 0);
                    }
                }
                updateCurrentSpeed(false);
            }
            else if(m_command.move[0] == 1)  // If we continue running...
            {
                if((curr_fc->water || curr_fc->quicksand != QuicksandPosition::None) && curr_fc->floor_hit && (curr_fc->transition_level - curr_fc->floor_point[2] > m_wadeDepth))
                {
                    m_bf.animations.next_state = TR_STATE_LARA_WADE_FORWARD;
                }
                else if(m_command.shift)
                {
                    m_bf.animations.next_state = TR_STATE_LARA_WALK_FORWARD;
                }
                else if(m_command.jump)
                {
                    m_bf.animations.next_state = TR_STATE_LARA_JUMP_FORWARD;
                }
                else if(m_command.roll)
                {
                    m_dirFlag = ENT_MOVE_FORWARD;
                    setAnimation(TR_ANIMATION_LARA_ROLL_BEGIN, 0);
                }
                else if(m_command.sprint)
                {
                    m_bf.animations.next_state = TR_STATE_LARA_SPRINT;
                }
            }
            else
            {
                m_bf.animations.next_state = TR_STATE_LARA_STOP;
            }
            break;

        case TR_STATE_LARA_SPRINT:
            global_offset = m_transform.getBasis().getColumn(1) * engine::RunForwardOffset;
            lean(12.0);
            global_offset[2] += m_bf.boundingBox.max[2];
            nextStep = checkNextStep(global_offset, &next_fc);
            m_command.crouch |= low_vertical_space;

            if(m_moveType == MoveType::OnFloor)
            {
                m_bt.no_fix_body_parts = BODY_PART_LEGS;
            }

            if(!getParam(PARAM_STAMINA))
            {
                m_bf.animations.next_state = TR_STATE_LARA_RUN_FORWARD;
            }
            else if(m_moveType == MoveType::FreeFalling)
            {
                setAnimation(TR_ANIMATION_LARA_FREE_FALL_FORWARD, 0);
            }
            else if(m_response.killed)
            {
                m_bf.animations.next_state = TR_STATE_LARA_RUN_FORWARD;  // Normal run then die
            }
            else if(m_response.slide == SlideType::Front)
            {
                setAnimation(TR_ANIMATION_LARA_SLIDE_FORWARD, 0);
            }
            else if(m_response.slide == SlideType::Back)
            {
                setAnimation(TR_ANIMATION_LARA_START_SLIDE_BACKWARD, 0);
            }
            else if((next_fc.floor_normale[2] < m_criticalSlantZComponent) && nextStep > StepType::Horizontal)
            {
                m_currentSpeed = 0.0;
                setAnimation(TR_ANIMATION_LARA_STAY_IDLE, 0);
            }
            else if((next_fc.floor_normale[2] >= m_criticalSlantZComponent) && nextStep == StepType::UpBig)
            {
                m_bf.animations.next_state = TR_STATE_LARA_RUN_FORWARD;  // Interrupt sprint
            }
            else if(m_response.horizontal_collide & 0x01)
            {
                engine::Controls_JoyRumble(200.0, 200);

                i = getAnimDispatchCase(2);
                if(i == 1)
                {
                    setAnimation(TR_ANIMATION_LARA_WALL_SMASH_LEFT, 0);
                }
                else if(i == 0)
                {
                    setAnimation(TR_ANIMATION_LARA_WALL_SMASH_RIGHT, 0);
                }
                updateCurrentSpeed(false);
            }
            else if(!m_command.sprint)
            {
                if(m_command.move[0] == 1)
                {
                    m_bf.animations.next_state = TR_STATE_LARA_RUN_FORWARD;
                }
                else
                {
                    m_bf.animations.next_state = TR_STATE_LARA_STOP;
                }
            }
            else
            {
                if(m_command.jump == 1)
                {
                    m_bf.animations.next_state = TR_STATE_LARA_SPRINT_ROLL;
                }
                else if(m_command.roll == 1)
                {
                    m_dirFlag = ENT_MOVE_FORWARD;
                    setAnimation(TR_ANIMATION_LARA_ROLL_BEGIN, 0);
                }
                else if(m_command.crouch)
                {
                    m_bf.animations.next_state = TR_STATE_LARA_CROUCH_IDLE;
                }
                else if(m_command.move[0] == 0)
                {
                    m_bf.animations.next_state = TR_STATE_LARA_STOP;
                }
            }
            break;

        case TR_STATE_LARA_WALK_FORWARD:
            m_command.rot[0] *= 0.4f;
            lean(0.0);

            global_offset = m_transform.getBasis().getColumn(1) * engine::WalkForwardOffset;
            global_offset[2] += m_bf.boundingBox.max[2];
            nextStep = checkNextStep(global_offset, &next_fc);
            m_dirFlag = ENT_MOVE_FORWARD;

            if(m_moveType == MoveType::OnFloor)
            {
                m_bt.no_fix_body_parts = BODY_PART_LEGS;
            }

            if(m_moveType == MoveType::FreeFalling)
            {
                setAnimation(TR_ANIMATION_LARA_START_FREE_FALL, 0);
            }
            else if(m_response.killed)
            {
                m_bf.animations.next_state = TR_STATE_LARA_STOP;
            }
            else if((next_fc.floor_normale[2] >= m_criticalSlantZComponent) && nextStep == StepType::UpBig)
            {
                // Climb up

                m_dirFlag = ENT_STAY;
                i = getAnimDispatchCase(2);
                if(i == 1)
                {
                    setAnimation(TR_ANIMATION_LARA_WALK_UP_STEP_RIGHT, 0);
                    pos = next_fc.floor_point;
                    m_moveType = MoveType::OnFloor;
                    m_dirFlag = ENT_MOVE_FORWARD;
                }
                else
                {
                    setAnimation(TR_ANIMATION_LARA_WALK_UP_STEP_LEFT, 0);
                    pos = next_fc.floor_point;
                    m_moveType = MoveType::OnFloor;
                    m_dirFlag = ENT_MOVE_FORWARD;
                }
            }
            else if((next_fc.floor_normale[2] >= m_criticalSlantZComponent) && nextStep == StepType::DownBig)
            {
                // Climb down

                m_dirFlag = ENT_STAY;
                i = getAnimDispatchCase(2);
                if(i == 1)
                {
                    setAnimation(TR_ANIMATION_LARA_WALK_DOWN_RIGHT, 0);
                    m_climb.point = next_fc.floor_point;
                    pos = next_fc.floor_point;
                    m_moveType = MoveType::OnFloor;
                    m_dirFlag = ENT_MOVE_FORWARD;
                }
                else //if(i == 0)
                {
                    setAnimation(TR_ANIMATION_LARA_WALK_DOWN_LEFT, 0);
                    m_climb.point = next_fc.floor_point;
                    pos = next_fc.floor_point;
                    m_moveType = MoveType::OnFloor;
                    m_dirFlag = ENT_MOVE_FORWARD;
                }
            }
            else if((m_response.horizontal_collide & 0x01) || !isWakableStep(nextStep) || (low_vertical_space))
            {
                // Too high!

                m_dirFlag = ENT_STAY;
                setAnimation(TR_ANIMATION_LARA_STAY_IDLE, 0);
            }
            else if(m_command.move[0] != 1)
            {
                m_bf.animations.next_state = TR_STATE_LARA_STOP;
            }
            else if((curr_fc->water || curr_fc->quicksand != QuicksandPosition::None) && curr_fc->floor_hit && (curr_fc->transition_level - curr_fc->floor_point[2] > m_wadeDepth))
            {
                m_bf.animations.next_state = TR_STATE_LARA_WADE_FORWARD;
            }
            else if(m_command.move[0] == 1 && !m_command.crouch && !m_command.shift)
            {
                m_bf.animations.next_state = TR_STATE_LARA_RUN_FORWARD;
            }
            break;

        case TR_STATE_LARA_WADE_FORWARD:
            m_command.rot[0] *= 0.4f;
            m_dirFlag = ENT_MOVE_FORWARD;

            if(m_heightInfo.quicksand != QuicksandPosition::None)
            {
                m_currentSpeed = std::min(m_currentSpeed, MAX_SPEED_QUICKSAND);
            }

            if(m_command.move[0] == 1)
            {
                move = m_transform.getBasis().getColumn(1) * engine::PenetrationTestOffset;
                checkNextPenetration(move);
            }

            if(m_response.killed)
            {
                m_bf.animations.next_state = TR_STATE_LARA_STOP;
            }

            if(!curr_fc->floor_hit || m_moveType == MoveType::FreeFalling)  // Free fall, then swim
            {
                setAnimation(TR_ANIMATION_LARA_START_FREE_FALL, 0);
            }
            else if(curr_fc->water)
            {
                if((curr_fc->transition_level - curr_fc->floor_point[2] <= m_wadeDepth))
                {
                    // run / walk case
                    if((m_command.move[0] == 1) && (m_response.horizontal_collide == 0))
                    {
                        m_bf.animations.next_state = TR_STATE_LARA_RUN_FORWARD;
                    }
                    else
                    {
                        m_bf.animations.next_state = TR_STATE_LARA_STOP;
                    }
                }
                else if(curr_fc->transition_level - curr_fc->floor_point[2] > (m_height - m_swimDepth))
                {
                    // Swim case
                    if(curr_fc->transition_level - curr_fc->floor_point[2] > m_height + m_maxStepUpHeight)
                    {
                        setAnimation(TR_ANIMATION_LARA_START_FREE_FALL, 0);                                    // swim underwater
                    }
                    else
                    {
                        setAnimation(TR_ANIMATION_LARA_ONWATER_IDLE, 0);                                       // swim onwater
                        m_moveType = MoveType::OnWater;
                        pos[2] = curr_fc->transition_level;
                    }
                }
                else if(curr_fc->transition_level - curr_fc->floor_point[2] > m_wadeDepth)              // wade case
                {
                    if((m_command.move[0] != 1) || (m_response.horizontal_collide != 0))
                    {
                        m_bf.animations.next_state = TR_STATE_LARA_STOP;
                    }
                }
            }
            else                                                                // no water, stay or run / walk
            {
                if((m_command.move[0] == 1) && (m_response.horizontal_collide == 0))
                {
                    if(curr_fc->quicksand == QuicksandPosition::None)
                    {
                        m_bf.animations.next_state = TR_STATE_LARA_RUN_FORWARD;
                    }
                }
                else
                {
                    m_bf.animations.next_state = TR_STATE_LARA_STOP;
                }
            }
            break;

        case TR_STATE_LARA_WALK_BACK:
            m_command.rot[0] *= 0.4f;
            m_dirFlag = ENT_MOVE_BACKWARD;

            if(m_heightInfo.quicksand != QuicksandPosition::None)
            {
                m_currentSpeed = std::min(m_currentSpeed, MAX_SPEED_QUICKSAND);
            }

            global_offset = m_transform.getBasis().getColumn(1) * -engine::WalkBackOffset;
            global_offset[2] += m_bf.boundingBox.max[2];
            nextStep = checkNextStep(global_offset, &next_fc);
            if(m_moveType == MoveType::FreeFalling)
            {
                setAnimation(TR_ANIMATION_LARA_START_FREE_FALL, 0);
            }
            else if(curr_fc->water && (curr_fc->floor_point[2] + m_height < curr_fc->transition_level))
            {
                setAnimation(TR_ANIMATION_LARA_ONWATER_SWIM_BACK, 0);
                m_bf.animations.next_state = TR_STATE_LARA_ONWATER_BACK;
                m_moveType = MoveType::OnWater;
            }
            else if(!isWakableStep(nextStep))
            {
                m_dirFlag = ENT_STAY;
                setAnimation(TR_ANIMATION_LARA_CLIMB_2CLICK_END, 0);
            }
            else if((next_fc.floor_normale[2] >= m_criticalSlantZComponent) && nextStep == StepType::DownBig)
            {
                if(!m_bt.no_fix_all)
                {
                    int frames_count = static_cast<int>(m_bf.animations.model->animations[TR_ANIMATION_LARA_WALK_DOWN_BACK_LEFT].frames.size());
                    int frames_count2 = (frames_count + 1) / 2;
                    if((m_bf.animations.current_frame >= 0) && (m_bf.animations.current_frame <= frames_count2))
                    {
                        setAnimation(TR_ANIMATION_LARA_WALK_DOWN_BACK_LEFT, m_bf.animations.current_frame);
                        m_dirFlag = ENT_MOVE_BACKWARD;
                        m_transform.getOrigin()[2] -= (curr_fc->floor_point[2] - next_fc.floor_point[2]);
                        m_bt.no_fix_all = true;
                    }
                    else if((m_bf.animations.current_frame >= frames_count) && (m_bf.animations.current_frame <= frames_count + frames_count2))
                    {
                        setAnimation(TR_ANIMATION_LARA_WALK_DOWN_BACK_RIGHT, m_bf.animations.current_frame - frames_count);
                        m_dirFlag = ENT_MOVE_BACKWARD;
                        m_transform.getOrigin()[2] -= (curr_fc->floor_point[2] - next_fc.floor_point[2]);
                        m_bt.no_fix_all = true;
                    }
                    else
                    {
                        m_dirFlag = ENT_STAY;                               // waiting for correct frame
                    }
                }
            }
            else if((m_command.move[0] == -1) && (m_command.shift || m_heightInfo.quicksand != QuicksandPosition::None))
            {
                m_dirFlag = ENT_MOVE_BACKWARD;
                m_bf.animations.next_state = TR_STATE_LARA_WALK_BACK;
            }
            else
            {
                m_bf.animations.next_state = TR_STATE_LARA_STOP;
            }
            break;

        case TR_STATE_LARA_WALK_LEFT:
            m_command.rot[0] = 0;
            m_dirFlag = ENT_MOVE_LEFT;
            if(m_moveType == MoveType::FreeFalling)
            {
                setAnimation(TR_ANIMATION_LARA_START_FREE_FALL, 0);
            }
            else if(m_command.move[1] == -1 && m_command.shift)
            {
                global_offset = m_transform.getBasis().getColumn(0) * -engine::RunForwardOffset;  // not an error - RUN_... more correct here
                global_offset[2] += m_bf.boundingBox.max[2];
                global_offset += pos;
                Character::getHeightInfo(global_offset, &next_fc);
                if(next_fc.floor_hit && (next_fc.floor_point[2] > pos[2] - m_maxStepUpHeight) && (next_fc.floor_point[2] <= pos[2] + m_maxStepUpHeight))
                {
                    if(!curr_fc->water || (curr_fc->floor_point[2] + m_height > curr_fc->transition_level)) // if (floor_hit == 0) then we went to MoveType::FreeFalling.
                    {
                        // continue walking
                    }
                    else
                    {
                        m_bf.animations.next_state = TR_STATE_LARA_ONWATER_LEFT;
                        m_bf.animations.onFrame = ent_to_on_water;
                    }
                }
                else
                {
                    m_dirFlag = ENT_STAY;
                    setAnimation(TR_ANIMATION_LARA_STAY_SOLID, 0);
                }
            }
            else
            {
                m_bf.animations.next_state = TR_STATE_LARA_STOP;
            }
            break;

        case TR_STATE_LARA_WALK_RIGHT:
            m_command.rot[0] = 0;
            m_dirFlag = ENT_MOVE_RIGHT;
            if(m_moveType == MoveType::FreeFalling)
            {
                setAnimation(TR_ANIMATION_LARA_START_FREE_FALL, 0);
            }
            else if(m_command.move[1] == 1 && m_command.shift)
            {
                // Not a error - RUN_... constant is more correct here
                global_offset = m_transform.getBasis().getColumn(0) * engine::RunForwardOffset;
                global_offset[2] += m_bf.boundingBox.max[2];
                global_offset += pos;
                Character::getHeightInfo(global_offset, &next_fc);
                if(next_fc.floor_hit && (next_fc.floor_point[2] > pos[2] - m_maxStepUpHeight) && (next_fc.floor_point[2] <= pos[2] + m_maxStepUpHeight))
                {
                    if(!curr_fc->water || (curr_fc->floor_point[2] + m_height > curr_fc->transition_level)) // if (floor_hit == 0) then we went to MoveType::FreeFalling.
                    {
                        // continue walking
                    }
                    else
                    {
                        m_bf.animations.next_state = TR_STATE_LARA_ONWATER_RIGHT;
                        m_bf.animations.onFrame = ent_to_on_water;
                    }
                }
                else
                {
                    m_dirFlag = ENT_STAY;
                    setAnimation(TR_ANIMATION_LARA_STAY_SOLID, 0);
                }
            }
            else
            {
                m_bf.animations.next_state = TR_STATE_LARA_STOP;
            }
            break;

            // Slide animations

        case TR_STATE_LARA_SLIDE_BACK:
            m_command.rot[0] = 0;
            lean(0.0);
            m_dirFlag = ENT_MOVE_BACKWARD;

            if(m_moveType == MoveType::FreeFalling)
            {
                if(m_command.action)
                {
                    m_speed[0] = -m_transform.getBasis().getColumn(1)[0] * 128.0f;
                    m_speed[1] = -m_transform.getBasis().getColumn(1)[1] * 128.0f;
                }

                setAnimation(TR_ANIMATION_LARA_FREE_FALL_BACK, 0);
            }
            else if(m_response.slide == SlideType::None)
            {
                m_bf.animations.next_state = TR_STATE_LARA_STOP;
            }
            else if(m_response.slide != SlideType::None && m_command.jump)
            {
                m_bf.animations.next_state = TR_STATE_LARA_JUMP_BACK;
            }
            else
            {
                break;
            }

            audio::kill(TR_AUDIO_SOUND_SLIDING, audio::EmitterType::Entity, id());
            break;

        case TR_STATE_LARA_SLIDE_FORWARD:
            m_command.rot[0] = 0;
            lean(0.0);
            m_dirFlag = ENT_MOVE_FORWARD;

            if(m_moveType == MoveType::FreeFalling)
            {
                setAnimation(TR_ANIMATION_LARA_FREE_FALL_FORWARD, 0);
            }
            else if(m_response.slide == SlideType::None)
            {
                if((m_command.move[0] == 1) && (engine::engine_world.engineVersion >= loader::Engine::TR3))
                {
                    m_bf.animations.next_state = TR_STATE_LARA_RUN_FORWARD;
                }
                else
                {
                    m_bf.animations.next_state = TR_STATE_LARA_STOP;                  // stop
                }
            }
            else if(m_response.slide != SlideType::None && m_command.jump)
            {
                m_bf.animations.next_state = TR_STATE_LARA_JUMP_FORWARD;               // jump
            }
            else
            {
                break;
            }

            audio::kill(TR_AUDIO_SOUND_SLIDING, audio::EmitterType::Entity, id());
            break;

            // Miscellaneous animations

        case TR_STATE_LARA_PUSHABLE_GRAB:
            m_moveType = MoveType::OnFloor;
            m_bt.no_fix_all = true;
            m_command.rot[0] = 0.0;

            if(m_command.action)  //If Lara is grabbing the block
            {
                int tf = checkTraverse(*m_traversedObject);
                m_dirFlag = ENT_STAY;
                m_bf.animations.mode = animation::SSAnimationMode::LoopLastFrame;  //We hold it (loop last frame)

                if((m_command.move[0] == 1) && (tf & Character::TraverseForward))  // If player presses up, then push
                {
                    m_dirFlag = ENT_MOVE_FORWARD;
                    m_bf.animations.mode = animation::SSAnimationMode::NormalControl;
                    m_bf.animations.next_state = TR_STATE_LARA_PUSHABLE_PUSH;
                }
                else if((m_command.move[0] == -1) && (tf & Character::TraverseBackward))  //If player presses down, then pull
                {
                    m_dirFlag = ENT_MOVE_BACKWARD;
                    m_bf.animations.mode = animation::SSAnimationMode::NormalControl;
                    m_bf.animations.next_state = TR_STATE_LARA_PUSHABLE_PULL;
                }
            }
            else  //Lara has let go of the block
            {
                m_dirFlag = ENT_STAY;
                m_bf.animations.mode = animation::SSAnimationMode::NormalControl;  // We're no longer looping last frame
                m_bf.animations.next_state = TR_STATE_LARA_STOP;   // Switch to next Lara state
            }
            break;

        case TR_STATE_LARA_PUSHABLE_PUSH:
            m_bt.no_fix_all = true;
            m_bf.animations.onFrame = ent_stop_traverse;
            m_command.rot[0] = 0.0;
            m_camFollowCenter = 64;
            i = static_cast<int>(m_bf.animations.model->animations[m_bf.animations.current_animation].frames.size());

            if(!m_command.action || !(Character::TraverseForward & checkTraverse(*m_traversedObject)))   //For TOMB4/5 If Lara is pushing and action let go, don't push
            {
                m_bf.animations.next_state = TR_STATE_LARA_STOP;
            }

            if((m_traversedObject != nullptr) && (m_bf.animations.current_frame > 16) && (m_bf.animations.current_frame < i - 16)) ///@FIXME: magick 16
            {
                bool was_traversed = false;

                if(m_transform.getBasis().getColumn(1)[0] > 0.9)
                {
                    t = m_transform.getOrigin()[0] + (m_bf.boundingBox.max[1] - m_traversedObject->m_bf.boundingBox.min[0] - 32.0f);
                    if(t > m_traversedObject->m_transform.getOrigin()[0])
                    {
                        m_traversedObject->m_transform.getOrigin()[0] = t;
                        was_traversed = true;
                    }
                }
                else if(m_transform.getBasis().getColumn(1)[0] < -0.9)
                {
                    t = m_transform.getOrigin()[0] - (m_bf.boundingBox.max[1] + m_traversedObject->m_bf.boundingBox.max[0] - 32.0f);
                    if(t < m_traversedObject->m_transform.getOrigin()[0])
                    {
                        m_traversedObject->m_transform.getOrigin()[0] = t;
                        was_traversed = true;
                    }
                }
                else if(m_transform.getBasis().getColumn(1)[1] > 0.9)
                {
                    t = m_transform.getOrigin()[1] + (m_bf.boundingBox.max[1] - m_traversedObject->m_bf.boundingBox.min[1] - 32.0f);
                    if(t > m_traversedObject->m_transform.getOrigin()[1])
                    {
                        m_traversedObject->m_transform.getOrigin()[1] = t;
                        was_traversed = true;
                    }
                }
                else if(m_transform.getBasis().getColumn(1)[1] < -0.9)
                {
                    t = m_transform.getOrigin()[1] - (m_bf.boundingBox.max[1] + m_traversedObject->m_bf.boundingBox.max[1] - 32.0f);
                    if(t < m_traversedObject->m_transform.getOrigin()[1])
                    {
                        m_traversedObject->m_transform.getOrigin()[1] = t;
                        was_traversed = true;
                    }
                }

                if(engine::engine_world.engineVersion > loader::Engine::TR3)
                {
                    if(was_traversed)
                    {
                        if(engine::engine_world.findSource(TR_AUDIO_SOUND_PUSHABLE, audio::EmitterType::Entity, id()) == -1)
                            audio::send(TR_AUDIO_SOUND_PUSHABLE, audio::EmitterType::Entity, id());
                    }
                    else
                    {
                        audio::kill(TR_AUDIO_SOUND_PUSHABLE, audio::EmitterType::Entity, id());
                    }
                }
                else
                {
                    if((m_bf.animations.current_frame == 49) ||
                       (m_bf.animations.current_frame == 110) ||
                       (m_bf.animations.current_frame == 142))
                    {
                        if(engine::engine_world.findSource(TR_AUDIO_SOUND_PUSHABLE, audio::EmitterType::Entity, id()) == -1)
                            audio::send(TR_AUDIO_SOUND_PUSHABLE, audio::EmitterType::Entity, id());
                    }
                }

                m_traversedObject->updateRigidBody(true);
            }
            else
            {
                if(engine::engine_world.engineVersion > loader::Engine::TR3)
                {
                    audio::kill(TR_AUDIO_SOUND_PUSHABLE, audio::EmitterType::Entity, id());
                }
            }
            break;

        case TR_STATE_LARA_PUSHABLE_PULL:
            m_bt.no_fix_all = true;
            m_bf.animations.onFrame = ent_stop_traverse;
            m_command.rot[0] = 0.0;
            m_camFollowCenter = 64;
            i = static_cast<int>(m_bf.animations.model->animations[m_bf.animations.current_animation].frames.size());

            if(!m_command.action || !(Character::TraverseBackward & checkTraverse(*m_traversedObject)))   //For TOMB4/5 If Lara is pulling and action let go, don't pull
            {
                m_bf.animations.next_state = TR_STATE_LARA_STOP;
            }

            if((m_traversedObject != nullptr) && (m_bf.animations.current_frame > 20) && (m_bf.animations.current_frame < i - 16)) ///@FIXME: magick 20
            {
                bool was_traversed = false;

                if(m_transform.getBasis().getColumn(1)[0] > 0.9)
                {
                    t = m_transform.getOrigin()[0] + (m_bf.boundingBox.max[1] - m_traversedObject->m_bf.boundingBox.min[0] - 32.0f);
                    if(t < m_traversedObject->m_transform.getOrigin()[0])
                    {
                        m_traversedObject->m_transform.getOrigin()[0] = t;
                        was_traversed = true;
                    }
                }
                else if(m_transform.getBasis().getColumn(1)[0] < -0.9)
                {
                    t = m_transform.getOrigin()[0] - (m_bf.boundingBox.max[1] + m_traversedObject->m_bf.boundingBox.max[0] - 32.0f);
                    if(t > m_traversedObject->m_transform.getOrigin()[0])
                    {
                        m_traversedObject->m_transform.getOrigin()[0] = t;
                        was_traversed = true;
                    }
                }
                else if(m_transform.getBasis().getColumn(1)[1] > 0.9)
                {
                    t = m_transform.getOrigin()[1] + (m_bf.boundingBox.max[1] - m_traversedObject->m_bf.boundingBox.min[1] - 32.0f);
                    if(t < m_traversedObject->m_transform.getOrigin()[1])
                    {
                        m_traversedObject->m_transform.getOrigin()[1] = t;
                        was_traversed = true;
                    }
                }
                else if(m_transform.getBasis().getColumn(1)[1] < -0.9)
                {
                    t = m_transform.getOrigin()[1] - (m_bf.boundingBox.max[1] + m_traversedObject->m_bf.boundingBox.max[1] - 32.0f);
                    if(t > m_traversedObject->m_transform.getOrigin()[1])
                    {
                        m_traversedObject->m_transform.getOrigin()[1] = t;
                        was_traversed = true;
                    }
                }

                if(engine::engine_world.engineVersion > loader::Engine::TR3)
                {
                    if(was_traversed)
                    {
                        if(engine::engine_world.findSource(TR_AUDIO_SOUND_PUSHABLE, audio::EmitterType::Entity, id()) == -1)

                            audio::send(TR_AUDIO_SOUND_PUSHABLE, audio::EmitterType::Entity, id());
                    }
                    else
                    {
                        audio::kill(TR_AUDIO_SOUND_PUSHABLE, audio::EmitterType::Entity, id());
                    }
                }
                else
                {
                    if((m_bf.animations.current_frame == 40) ||
                       (m_bf.animations.current_frame == 92) ||
                       (m_bf.animations.current_frame == 124) ||
                       (m_bf.animations.current_frame == 156))
                    {
                        if(engine::engine_world.findSource(TR_AUDIO_SOUND_PUSHABLE, audio::EmitterType::Entity, id()) == -1)
                            audio::send(TR_AUDIO_SOUND_PUSHABLE, audio::EmitterType::Entity, id());
                    }
                }

                m_traversedObject->updateRigidBody(true);
            }
            else
            {
                if(engine::engine_world.engineVersion > loader::Engine::TR3)
                {
                    audio::kill(TR_AUDIO_SOUND_PUSHABLE, audio::EmitterType::Entity, id());
                }
            }
            break;

        case TR_STATE_LARA_ROLL_FORWARD:
            m_bt.no_fix_body_parts = BODY_PART_LEGS;
            break;

        case TR_STATE_LARA_ROLL_BACKWARD:
            m_bt.no_fix_body_parts = BODY_PART_HANDS;
            if(m_moveType == MoveType::FreeFalling)
            {
                setAnimation(TR_ANIMATION_LARA_FREE_FALL_FORWARD, 0);
            }
            else if(low_vertical_space)
            {
                m_dirFlag = ENT_STAY;
            }
            else if(m_response.slide == SlideType::Front)
            {
                setAnimation(TR_ANIMATION_LARA_SLIDE_FORWARD, 0);
            }
            else if(m_response.slide == SlideType::Back)
            {
                setAnimation(TR_ANIMATION_LARA_START_SLIDE_BACKWARD, 0);
            }
            break;

            //Climbing animations

        case TR_STATE_LARA_JUMP_UP:
            m_command.rot[0] = 0.0;
            if(m_command.action && (m_moveType != MoveType::WallsClimb) && (m_moveType != MoveType::Climbing))
            {
                t = engine::LaraTryHangWallOffset + engine::LaraHangWallDistance;
                global_offset = m_transform.getBasis().getColumn(1) * t;
                global_offset[2] += m_bf.boundingBox.max[2] + engine::LaraHangVerticalEpsilon + engine::engine_frame_time * m_speed[2];
                m_climb = checkClimbability(global_offset, &next_fc, 0.0);
                if(m_climb.edge_hit)
                {
                    m_climb.point = m_climb.edge_point;
                    m_angles[0] = m_climb.edge_z_ang;
                    updateTransform();
                    m_moveType = MoveType::Climbing;                             // hang on
                    m_speed.setZero();

                    pos[0] = m_climb.point[0] - (engine::LaraHangWallDistance)* m_transform.getBasis().getColumn(1)[0];
                    pos[1] = m_climb.point[1] - (engine::LaraHangWallDistance)* m_transform.getBasis().getColumn(1)[1];
                    pos[2] = m_climb.point[2] - m_bf.boundingBox.max[2] + engine::LaraHangVerticalOffset;
                }
                else
                {
                    m_climb = checkWallsClimbability();
                    if(m_climb.wall_hit != ClimbType::None &&
                       (m_speed[2] < 0.0)) // Only hang if speed is lower than zero.
                    {
                        // Fix the position to the TR metering step.
                        m_transform.getOrigin()[2] = std::floor(m_transform.getOrigin()[2] / MeteringStep) * MeteringStep;
                        m_moveType = MoveType::WallsClimb;
                        setAnimation(TR_ANIMATION_LARA_HANG_IDLE, -1);
                        break;
                    }
                }
            }

            if(m_command.move[0] == 1)
            {
                m_dirFlag = ENT_MOVE_FORWARD;
            }
            else if(m_command.move[0] == -1)
            {
                m_dirFlag = ENT_MOVE_BACKWARD;
            }
            else if(m_command.move[1] == 1)
            {
                m_dirFlag = ENT_MOVE_RIGHT;
            }
            else if(m_command.move[1] == -1)
            {
                m_dirFlag = ENT_MOVE_LEFT;
            }
            else
            {
                m_dirFlag = ENT_MOVE_FORWARD;///Lara can move forward towards walls in this state
            }

            if(m_moveType == MoveType::Underwater)
            {
                m_angles[1] = -45.0;
                m_command.rot[1] = 0.0;
                updateTransform();
                setAnimation(TR_ANIMATION_LARA_FREE_FALL_TO_UNDERWATER, 0);
            }
            else if(m_command.action && curr_fc->ceiling_climb && (curr_fc->ceiling_hit) && (pos[2] + m_bf.boundingBox.max[2] > curr_fc->ceiling_point[2] - 64.0))
            {
                m_bf.animations.next_state = TR_STATE_LARA_MONKEYSWING_IDLE;
                m_bf.animations.onFrame = ent_to_monkey_swing;
            }
            else if(m_command.action && (m_moveType == MoveType::Climbing))
            {
                m_bf.animations.next_state = TR_STATE_LARA_HANG;
                setAnimation(TR_ANIMATION_LARA_HANG_IDLE, -1);
            }
            else if((m_response.vertical_collide & 0x01) || (m_moveType == MoveType::OnFloor))
            {
                m_bf.animations.next_state = TR_STATE_LARA_STOP;  // Landing immediately
            }
            else
            {
                if(m_speed[2] < -FREE_FALL_SPEED_2)  // Next free fall stage
                {
                    m_moveType = MoveType::FreeFalling;
                    m_bf.animations.next_state = TR_STATE_LARA_FREEFALL;
                }
                break;
            }
            break;

        case TR_STATE_LARA_REACH:
            m_bt.no_fix_body_parts = BODY_PART_LEGS | BODY_PART_HANDS_1 | BODY_PART_HANDS_2;
            m_command.rot[0] = 0.0;
            if(m_moveType == MoveType::Underwater)
            {
                m_angles[1] = -45.0;
                m_command.rot[1] = 0.0;
                updateTransform();
                setAnimation(TR_ANIMATION_LARA_FREE_FALL_TO_UNDERWATER, 0);
                break;
            }

            if(m_command.action && (m_moveType == MoveType::FreeFalling))
            {
                t = engine::LaraTryHangWallOffset + engine::LaraHangWallDistance;
                global_offset = m_transform.getBasis().getColumn(1) * t;
                global_offset[2] += m_bf.boundingBox.max[2] + engine::LaraHangVerticalEpsilon + engine::engine_frame_time * m_speed[2];
                m_climb = checkClimbability(global_offset, &next_fc, 0.0);
                if(m_climb.edge_hit && m_climb.can_hang)
                {
                    m_climb.point = m_climb.edge_point;
                    m_angles[0] = m_climb.edge_z_ang;
                    updateTransform();
                    m_moveType = MoveType::Climbing;  // Hang on
                    m_speed.setZero();
                }

                // If Lara is moving backwards off the ledge we want to move Lara slightly forwards
                // depending on the current angle.
                if((m_dirFlag == ENT_MOVE_BACKWARD) && (m_moveType == MoveType::Climbing))
                {
                    pos[0] = m_climb.point[0] - m_transform.getBasis().getColumn(1)[0] * (m_forwardSize + 16.0f);
                    pos[1] = m_climb.point[1] - m_transform.getBasis().getColumn(1)[1] * (m_forwardSize + 16.0f);
                }
            }

            if(((m_moveType != MoveType::OnFloor)) && m_command.action && curr_fc->ceiling_climb && (curr_fc->ceiling_hit) && (pos[2] + m_bf.boundingBox.max[2] > curr_fc->ceiling_point[2] - 64.0))
            {
                m_bf.animations.next_state = TR_STATE_LARA_MONKEYSWING_IDLE;
                m_bf.animations.onFrame = ent_to_monkey_swing;
                break;
            }
            if(((m_response.vertical_collide & 0x01) || (m_moveType == MoveType::OnFloor)) && (!m_command.action || !m_climb.can_hang))
            {
                m_bf.animations.next_state = TR_STATE_LARA_STOP;  // Middle landing
                break;
            }

            if((m_speed[2] < -FREE_FALL_SPEED_2))
            {
                m_moveType = MoveType::FreeFalling;
                m_bf.animations.next_state = TR_STATE_LARA_FREEFALL;
                break;
            }

            if(m_moveType == MoveType::Climbing)
            {
                m_speed.setZero();
                m_bf.animations.next_state = TR_STATE_LARA_HANG;
                m_bf.animations.onFrame = ent_to_edge_climb;
#if OSCILLATE_HANG_USE
                move = ent->transform.getBasis().getColumn(1) * engine::PenetrationTestOffset;
                if(Entity_CheckNextPenetration(ent, cmd, move) == 0)
                {
                    ent->setAnimation(TR_ANIMATION_LARA_OSCILLATE_HANG_ON, 0);
                    ent_to_edge_climb(ent);
                }
#endif
            }
            break;

            // The code here prevents Lara's UGLY move in end of "climb on" states.
            // Do not delete ent_set_on_floor_after_climb callback here!

        case TR_STATE_LARA_HANDSTAND:
        case TR_STATE_LARA_CLIMBING:
        case TR_STATE_LARA_CLIMB_TO_CRAWL:
            m_command.rot[0] = 0;
            m_bt.no_fix_all = true;
            //ss_anim->onFrame = ent_set_on_floor_after_climb; // @FIXME: BUGGY
            break;

        case TR_STATE_LARA_HANG:
            m_command.rot[0] = 0.0;

            if(m_moveType == MoveType::WallsClimb)
            {
                if(m_command.action)
                {
                    if(m_climb.wall_hit == ClimbType::FullBody && (m_command.move[0] == 0) && (m_command.move[1] == 0))
                    {
                        m_bf.animations.next_state = TR_STATE_LARA_LADDER_IDLE;
                    }
                    else if(m_command.move[0] == 1)  // UP
                    {
                        setAnimation(TR_ANIMATION_LARA_LADDER_UP_HANDS, 0);
                    }
                    else if(m_command.move[0] == -1)  // DOWN
                    {
                        setAnimation(TR_ANIMATION_LARA_LADDER_DOWN_HANDS, 0);
                    }
                    else if(m_command.move[1] == 1)
                    {
                        m_dirFlag = ENT_MOVE_RIGHT;
                        setAnimation(TR_ANIMATION_LARA_CLIMB_RIGHT, 0);  // Edge climb right
                    }
                    else if(m_command.move[1] == -1)
                    {
                        m_dirFlag = ENT_MOVE_LEFT;
                        setAnimation(TR_ANIMATION_LARA_CLIMB_LEFT, 0);  // Edge climb left
                    }
                    else if(m_climb.wall_hit == ClimbType::None)
                    {
                        m_moveType = MoveType::FreeFalling;
                        setAnimation(TR_ANIMATION_LARA_STOP_HANG_VERTICAL, 0);  // Fall down
                    }
                    else
                    {
                        m_bf.animations.mode = animation::SSAnimationMode::LoopLastFrame;  // Disable shake
                    }
                }
                else
                {
                    m_moveType = MoveType::FreeFalling;
                    setAnimation(TR_ANIMATION_LARA_TRY_HANG_VERTICAL, 0);  // Fall down
                }
                break;
            }

            if(!m_response.killed && m_command.action)  // We have to update climb point every time so entity can move
            {
                t = engine::LaraTryHangWallOffset + engine::LaraHangWallDistance;
                global_offset = m_transform.getBasis().getColumn(1) * t;
                global_offset[2] += m_bf.boundingBox.max[2] + engine::LaraHangVerticalEpsilon;
                m_climb = checkClimbability(global_offset, &next_fc, 0.0);
                if(m_climb.can_hang)
                {
                    m_climb.point = m_climb.edge_point;
                    m_angles[0] = m_climb.edge_z_ang;
                    updateTransform();
                    m_moveType = MoveType::Climbing;  // Hang on
                }
            }
            else
            {
                m_moveType = MoveType::FreeFalling;
                setAnimation(TR_ANIMATION_LARA_TRY_HANG_VERTICAL, 0);  // Fall down
                break;
            }

            if(m_moveType == MoveType::Climbing)
            {
                if(m_command.move[0] == 1)
                {
                    if(m_climb.edge_hit && (m_climb.next_z_space >= 512.0) && ((m_climb.next_z_space < m_height - engine::LaraHangVerticalEpsilon) || m_command.crouch))
                    {
                        m_climb.point = m_climb.edge_point;
                        m_bf.animations.next_state = TR_STATE_LARA_CLIMB_TO_CRAWL;  // Crawlspace climb
                    }
                    else if(m_climb.edge_hit && (m_climb.next_z_space >= m_height - engine::LaraHangVerticalEpsilon))
                    {
                        engine::Sys_DebugLog(LOG_FILENAME, "Zspace = %f", m_climb.next_z_space);
                        m_climb.point = m_climb.edge_point;
                        m_bf.animations.next_state = (m_command.shift) ? (TR_STATE_LARA_HANDSTAND) : (TR_STATE_LARA_CLIMBING);               // climb up
                    }
                    else
                    {
                        pos[0] = m_climb.point[0] - (engine::LaraHangWallDistance)* m_transform.getBasis().getColumn(1)[0];
                        pos[1] = m_climb.point[1] - (engine::LaraHangWallDistance)* m_transform.getBasis().getColumn(1)[1];
                        pos[2] = m_climb.point[2] - m_bf.boundingBox.max[2] + engine::LaraHangVerticalOffset;
                        m_speed.setZero();
                        m_bf.animations.mode = animation::SSAnimationMode::LoopLastFrame;  // Disable shake
                    }
                }
                else if(m_command.move[0] == -1)  // Check walls climbing
                {
                    m_climb = checkWallsClimbability();
                    if(m_climb.wall_hit != ClimbType::None)
                    {
                        m_moveType = MoveType::WallsClimb;
                    }
                    m_bf.animations.mode = animation::SSAnimationMode::LoopLastFrame;  // Disable shake
                }
                else if(m_command.move[1] == -1)
                {
                    move = m_transform.getBasis().getColumn(0) * -engine::PenetrationTestOffset;
                    if((checkNextPenetration(move) == 0) || (m_response.horizontal_collide == 0x00)) //we only want lara to shimmy when last frame is reached!
                    {
                        m_dirFlag = ENT_MOVE_LEFT;
                        setAnimation(TR_ANIMATION_LARA_CLIMB_LEFT, 0);
                    }
                    else
                    {
                        m_bf.animations.mode = animation::SSAnimationMode::LoopLastFrame;  // Disable shake
                    }
                }
                else if(m_command.move[1] == 1)
                {
                    move = m_transform.getBasis().getColumn(0) * engine::PenetrationTestOffset;
                    if((checkNextPenetration(move) == 0) || (m_response.horizontal_collide == 0x00)) //we only want lara to shimmy when last frame is reached!
                    {
                        m_dirFlag = ENT_MOVE_RIGHT;
                        setAnimation(TR_ANIMATION_LARA_CLIMB_RIGHT, 0);
                    }
                    else
                    {
                        m_bf.animations.mode = animation::SSAnimationMode::LoopLastFrame;  // Disable shake
                    }
                }
                else
                {
                    m_bf.animations.mode = animation::SSAnimationMode::LoopLastFrame;  // Disable shake
                    pos[0] = m_climb.point[0] - (engine::LaraHangWallDistance)* m_transform.getBasis().getColumn(1)[0];
                    pos[1] = m_climb.point[1] - (engine::LaraHangWallDistance)* m_transform.getBasis().getColumn(1)[1];
                    pos[2] = m_climb.point[2] - m_bf.boundingBox.max[2] + engine::LaraHangVerticalOffset;
                    m_speed.setZero();
                }
            }
            else if(m_command.action && curr_fc->ceiling_climb && (curr_fc->ceiling_hit) && (pos[2] + m_bf.boundingBox.max[2] > curr_fc->ceiling_point[2] - 64.0))
            {
                m_bf.animations.next_state = TR_STATE_LARA_MONKEYSWING_IDLE;
                m_bf.animations.onFrame = ent_to_monkey_swing;
            }
            else
            {
                m_moveType = MoveType::FreeFalling;
                setAnimation(TR_ANIMATION_LARA_TRY_HANG_VERTICAL, 0);  // Fall down
            }
            break;

        case TR_STATE_LARA_LADDER_IDLE:
            m_command.rot[0] = 0;
            m_moveType = MoveType::WallsClimb;
            m_dirFlag = ENT_STAY;
            m_camFollowCenter = 64;
            if(m_moveType == MoveType::Climbing)
            {
                m_bf.animations.next_state = TR_STATE_LARA_CLIMBING;
                break;
            }
            if(!m_command.action)
            {
                m_moveType = MoveType::FreeFalling;
                setAnimation(TR_ANIMATION_LARA_STOP_HANG_VERTICAL, 0);  // Fall down
            }
            else if(m_command.jump)
            {
                m_bf.animations.next_state = TR_STATE_LARA_JUMP_BACK;
                m_dirFlag = ENT_MOVE_BACKWARD;
            }
            else if(m_command.move[0] == 1)
            {
                t = engine::LaraTryHangWallOffset + engine::LaraHangWallDistance;
                global_offset = m_transform.getBasis().getColumn(1) * t;
                global_offset[2] += m_bf.boundingBox.max[2] + engine::LaraHangVerticalEpsilon;
                m_climb = checkClimbability(global_offset, &next_fc, 0.0);
                if(m_climb.edge_hit && (m_climb.next_z_space >= 512.0))
                {
                    m_moveType = MoveType::Climbing;
                    m_bf.animations.next_state = TR_STATE_LARA_CLIMBING;
                }
                else if((!curr_fc->ceiling_hit) || (pos[2] + m_bf.boundingBox.max[2] < curr_fc->ceiling_point[2]))
                {
                    m_bf.animations.next_state = TR_STATE_LARA_LADDER_UP;
                }
            }
            else if(m_command.move[0] == -1)
            {
                m_bf.animations.next_state = TR_STATE_LARA_LADDER_DOWN;
            }
            else if(m_command.move[1] == 1)
            {
                m_bf.animations.next_state = TR_STATE_LARA_LADDER_RIGHT;
            }
            else if(m_command.move[1] == -1)
            {
                m_bf.animations.next_state = TR_STATE_LARA_LADDER_LEFT;
            }
            break;

        case TR_STATE_LARA_LADDER_LEFT:
            m_dirFlag = ENT_MOVE_LEFT;
            if(!m_command.action || m_climb.wall_hit == ClimbType::None)
            {
                m_bf.animations.next_state = TR_STATE_LARA_HANG;
            }
            else
            {
                m_bf.animations.next_state = TR_STATE_LARA_LADDER_IDLE;
            }
            break;

        case TR_STATE_LARA_LADDER_RIGHT:
            m_dirFlag = ENT_MOVE_RIGHT;
            if(!m_command.action || m_climb.wall_hit == ClimbType::None)
            {
                m_bf.animations.next_state = TR_STATE_LARA_HANG;
            }
            else
            {
                m_bf.animations.next_state = TR_STATE_LARA_LADDER_IDLE;
            }
            break;

        case TR_STATE_LARA_LADDER_UP:
            m_camFollowCenter = 64;
            if(m_moveType == MoveType::Climbing)
            {
                m_bf.animations.next_state = TR_STATE_LARA_LADDER_IDLE;
                break;
            }

            if(m_command.action && m_climb.wall_hit != ClimbType::None)
            {
                t = engine::LaraTryHangWallOffset + engine::LaraHangWallDistance;
                global_offset = m_transform.getBasis().getColumn(1) * t;
                global_offset[2] += m_bf.boundingBox.max[2] + engine::LaraHangVerticalEpsilon;
                m_climb = checkClimbability(global_offset, &next_fc, 0.0);
                if(m_climb.edge_hit && (m_climb.next_z_space >= 512.0))
                {
                    m_moveType = MoveType::Climbing;
                    m_bf.animations.next_state = TR_STATE_LARA_LADDER_IDLE;
                }
                else if((m_command.move[0] <= 0) && (curr_fc->ceiling_hit || (pos[2] + m_bf.boundingBox.max[2] >= curr_fc->ceiling_point[2])))
                {
                    m_bf.animations.next_state = TR_STATE_LARA_LADDER_IDLE;
                }

                if(curr_fc->ceiling_hit && (pos[2] + m_bf.boundingBox.max[2] > curr_fc->ceiling_point[2]))
                {
                    pos[2] = curr_fc->ceiling_point[2] - m_bf.boundingBox.max[2];
                }
            }
            else
            {
                // Free fall after stop
                m_bf.animations.next_state = TR_STATE_LARA_LADDER_IDLE;
            }
            break;

        case TR_STATE_LARA_LADDER_DOWN:
            m_camFollowCenter = 64;
            if(m_command.action && m_climb.wall_hit != ClimbType::None && (m_command.move[1] < 0))
            {
                if(m_climb.wall_hit != ClimbType::FullBody)
                {
                    m_bf.animations.next_state = TR_STATE_LARA_LADDER_IDLE;
                }
            }
            else
            {
                m_bf.animations.next_state = TR_STATE_LARA_LADDER_IDLE;
            }
            break;

        case TR_STATE_LARA_SHIMMY_LEFT:
            m_bt.no_fix_body_parts = BODY_PART_LEGS;

            m_command.rot[0] = 0.0;
            m_dirFlag = ENT_MOVE_LEFT;
            if(!m_command.action)
            {
                m_speed.setZero();
                m_moveType = MoveType::FreeFalling;
                setAnimation(TR_ANIMATION_LARA_TRY_HANG_VERTICAL, 0); // fall down
                break;
            }

            if(m_moveType == MoveType::WallsClimb)
            {
                if(m_climb.wall_hit == ClimbType::None)
                {
                    m_moveType = MoveType::FreeFalling;
                    setAnimation(TR_ANIMATION_LARA_STOP_HANG_VERTICAL, 0); // fall down
                }
            }
            else
            {
                t = engine::LaraTryHangWallOffset + engine::LaraHangWallDistance;
                global_offset = m_transform.getBasis().getColumn(1) * t;
                global_offset[2] += engine::LaraHangSensorZ + engine::LaraHangVerticalEpsilon;
                m_climb = checkClimbability(global_offset, &next_fc, 0.0);
                if(m_climb.edge_hit)
                {
                    m_climb.point = m_climb.edge_point;
                    m_angles[0] = m_climb.edge_z_ang;
                    updateTransform();
                    m_moveType = MoveType::Climbing;                             // hang on
                    pos[0] = m_climb.point[0] - (engine::LaraHangWallDistance)* m_transform.getBasis().getColumn(1)[0];
                    pos[1] = m_climb.point[1] - (engine::LaraHangWallDistance)* m_transform.getBasis().getColumn(1)[1];
                    pos[2] = m_climb.point[2] - m_bf.boundingBox.max[2] + engine::LaraHangVerticalOffset;
                    m_speed.setZero();
                }
                else
                {
                    m_moveType = MoveType::FreeFalling;
                    setAnimation(TR_ANIMATION_LARA_STOP_HANG_VERTICAL, 0); // fall down
                    break;
                }
            }

            if(m_command.move[1] == -1)
            {
                move = m_transform.getBasis().getColumn(0) * -engine::PenetrationTestOffset;
                if((checkNextPenetration(move) > 0) && (m_response.horizontal_collide != 0x00))
                {
                    m_bf.animations.next_state = TR_STATE_LARA_HANG;
                }
            }
            else
            {
                m_bf.animations.next_state = TR_STATE_LARA_HANG;
            }
            break;

        case TR_STATE_LARA_SHIMMY_RIGHT:
            m_bt.no_fix_body_parts = BODY_PART_LEGS;

            m_command.rot[0] = 0.0;
            m_dirFlag = ENT_MOVE_RIGHT;
            if(!m_command.action)
            {
                m_speed.setZero();
                m_moveType = MoveType::FreeFalling;
                setAnimation(TR_ANIMATION_LARA_TRY_HANG_VERTICAL, 0); // fall down
                break;
            }

            if(m_moveType == MoveType::WallsClimb)
            {
                if(m_climb.wall_hit == ClimbType::None)
                {
                    m_moveType = MoveType::FreeFalling;
                    setAnimation(TR_ANIMATION_LARA_STOP_HANG_VERTICAL, 0); // fall down
                }
            }
            else
            {
                t = engine::LaraTryHangWallOffset + engine::LaraHangWallDistance;
                global_offset = m_transform.getBasis().getColumn(1) * t;
                global_offset[2] += engine::LaraHangSensorZ + engine::LaraHangVerticalEpsilon;
                m_climb = checkClimbability(global_offset, &next_fc, 0.0);
                if(m_climb.edge_hit)
                {
                    m_climb.point = m_climb.edge_point;
                    m_angles[0] = m_climb.edge_z_ang;
                    updateTransform();
                    m_moveType = MoveType::Climbing;                             // hang on
                    pos[0] = m_climb.point[0] - (engine::LaraHangWallDistance)* m_transform.getBasis().getColumn(1)[0];
                    pos[1] = m_climb.point[1] - (engine::LaraHangWallDistance)* m_transform.getBasis().getColumn(1)[1];
                    pos[2] = m_climb.point[2] - m_bf.boundingBox.max[2] + engine::LaraHangVerticalOffset;
                    m_speed.setZero();
                }
                else
                {
                    m_moveType = MoveType::FreeFalling;
                    setAnimation(TR_ANIMATION_LARA_STOP_HANG_VERTICAL, 0); // fall down
                    break;
                }
            }

            if(m_command.move[1] == 1)
            {
                move = m_transform.getBasis().getColumn(0) * engine::PenetrationTestOffset;
                if((checkNextPenetration(move) > 0) && (m_response.horizontal_collide != 0x00))
                {
                    m_bf.animations.next_state = TR_STATE_LARA_HANG;
                }
            }
            else
            {
                m_bf.animations.next_state = TR_STATE_LARA_HANG;
            }
            break;

        case TR_STATE_LARA_ONWATER_EXIT:
            m_command.rot[0] *= 0.0;
            m_bt.no_fix_all = true;
            m_bf.animations.onFrame = ent_set_on_floor_after_climb;
            break;

        case TR_STATE_LARA_JUMP_FORWARD:
        case TR_STATE_LARA_FALL_BACKWARD:
            m_bt.no_fix_body_parts = BODY_PART_HANDS | BODY_PART_LEGS | BODY_PART_HEAD;
            lean(4.0);

            if((m_response.vertical_collide & 0x01) || (m_moveType == MoveType::OnFloor))
            {
                if(m_self->room->flags & TR_ROOM_FLAG_QUICKSAND)
                {
                    setAnimation(TR_ANIMATION_LARA_STAY_IDLE, 0);
                }
                else if(!m_command.action && (m_command.move[0] == 1) && !m_command.crouch)
                {
                    m_moveType = MoveType::OnFloor;
                    m_bf.animations.next_state = TR_STATE_LARA_RUN_FORWARD;
                }
                else
                {
                    m_bf.animations.next_state = TR_STATE_LARA_STOP;
                }
            }
            else if(m_moveType == MoveType::Underwater)
            {
                m_angles[1] = -45.0;
                m_command.rot[1] = 0.0;
                updateTransform();
                setAnimation(TR_ANIMATION_LARA_FREE_FALL_TO_UNDERWATER, 0);
            }
            else if(m_response.horizontal_collide & 0x01)
            {
                setAnimation(TR_ANIMATION_LARA_SMASH_JUMP, 0);
                m_dirFlag = ENT_MOVE_BACKWARD;
                updateCurrentSpeed(true);
            }
            else if(m_speed[2] <= -FREE_FALL_SPEED_2)
            {
                m_bf.animations.next_state = TR_STATE_LARA_FREEFALL;                    // free falling
            }
            else if(m_command.action)
            {
                m_bf.animations.next_state = TR_STATE_LARA_REACH;
            }
            else if(m_command.shift)
            {
                m_bf.animations.next_state = TR_STATE_LARA_SWANDIVE_BEGIN;              // fly like fish
            }
            else if(m_speed[2] <= -FREE_FALL_SPEED_2)
            {
                m_bf.animations.next_state = TR_STATE_LARA_FREEFALL;                    // free falling
            }
            else if(m_command.roll)
            {
                m_bf.animations.next_state = TR_STATE_LARA_JUMP_ROLL;
            }
            break;

            // Freefall and underwater cases.

        case TR_STATE_LARA_UNDERWATER_DIVING:
            m_angles[1] = -45.0;
            m_command.rot[1] = 0.0;
            updateTransform();
            m_bf.animations.onFrame = ent_correct_diving_angle;
            break;

        case TR_STATE_LARA_FREEFALL:
            m_bt.no_fix_body_parts = BODY_PART_HANDS | BODY_PART_LEGS;
            lean(1.0);

            if((int(m_speed[2]) <= -FREE_FALL_SPEED_CRITICAL) &&
               (int(m_speed[2]) >= (-FREE_FALL_SPEED_CRITICAL - 100)))
            {
                m_speed[2] = -FREE_FALL_SPEED_CRITICAL - 101;
                audio::send(TR_AUDIO_SOUND_LARASCREAM, audio::EmitterType::Entity, id());       // Scream
            }
            else if(m_speed[2] <= -FREE_FALL_SPEED_MAXSAFE)
            {
                //Reset these to zero so Lara is only falling downwards
                m_speed[0] = 0.0;
                m_speed[1] = 0.0;
            }

            if(m_moveType == MoveType::Underwater)
            {
                m_angles[1] = -45.0;
                m_command.rot[1] = 0.0;
                updateTransform();                                     // needed here to fix underwater in wall collision bug
                setAnimation(TR_ANIMATION_LARA_FREE_FALL_TO_UNDERWATER, 0);
                audio::kill(TR_AUDIO_SOUND_LARASCREAM, audio::EmitterType::Entity, id());       // Stop scream

                // Splash sound is hardcoded, beginning with TR3.
                if(engine::engine_world.engineVersion > loader::Engine::TR2)
                {
                    audio::send(TR_AUDIO_SOUND_SPLASH, audio::EmitterType::Entity, id());
                }
            }
            else if((m_response.vertical_collide & 0x01) || (m_moveType == MoveType::OnFloor))
            {
                if(m_self->room->flags & TR_ROOM_FLAG_QUICKSAND)
                {
                    setAnimation(TR_ANIMATION_LARA_STAY_IDLE, 0);
                    audio::kill(TR_AUDIO_SOUND_LARASCREAM, audio::EmitterType::Entity, id());
                }
                else if(m_speed[2] <= -FREE_FALL_SPEED_MAXSAFE)
                {
                    if(!changeParam(PARAM_HEALTH, (m_speed[2] + FREE_FALL_SPEED_MAXSAFE) / 2))
                    {
                        m_response.killed = true;
                        setAnimation(TR_ANIMATION_LARA_LANDING_DEATH, 0);
                        engine::Controls_JoyRumble(200.0, 500);
                    }
                    else
                    {
                        setAnimation(TR_ANIMATION_LARA_LANDING_HARD, 0);
                    }
                }
                else if(m_speed[2] <= -FREE_FALL_SPEED_2)
                {
                    setAnimation(TR_ANIMATION_LARA_LANDING_HARD, 0);
                }
                else
                {
                    setAnimation(TR_ANIMATION_LARA_LANDING_MIDDLE, 0);
                }

                if(m_response.killed)
                {
                    m_bf.animations.next_state = TR_STATE_LARA_DEATH;
                    audio::kill(TR_AUDIO_SOUND_LARASCREAM, audio::EmitterType::Entity, id());
                }
            }
            else if(m_command.action)
            {
                m_dirFlag = ENT_MOVE_FORWARD;
                m_bf.animations.next_state = TR_STATE_LARA_REACH;
            }
            break;

        case TR_STATE_LARA_SWANDIVE_BEGIN:
            m_command.rot[0] *= 0.4f;
            if(m_response.vertical_collide & 0x01 || m_moveType == MoveType::OnFloor)
            {
                m_bf.animations.next_state = TR_STATE_LARA_STOP;                        // landing - roll
            }
            else if(m_moveType == MoveType::Underwater)
            {
                m_bf.animations.next_state = TR_STATE_LARA_UNDERWATER_DIVING;
            }
            else
            {
                m_bf.animations.next_state = TR_STATE_LARA_SWANDIVE_END;                // next stage
            }
            break;

        case TR_STATE_LARA_SWANDIVE_END:
            m_command.rot[0] = 0.0;

            //Reset these to zero so Lara is only falling downwards
            m_speed[0] = 0.0;
            m_speed[1] = 0.0;

            if((m_response.vertical_collide & 0x01) || (m_moveType == MoveType::OnFloor))
            {
                if(curr_fc->quicksand != QuicksandPosition::None)
                {
                    m_response.killed = true;
                    setParam(PARAM_HEALTH, 0.0);
                    setParam(PARAM_AIR, 0.0);
                    setAnimation(TR_ANIMATION_LARA_LANDING_DEATH, -1);
                }
                else
                {
                    setParam(PARAM_HEALTH, 0.0);
                    m_bf.animations.next_state = TR_STATE_LARA_DEATH;
                }
            }
            else if(m_moveType == MoveType::Underwater)
            {
                m_bf.animations.next_state = TR_STATE_LARA_UNDERWATER_DIVING;
            }
            else if(m_command.jump)
            {
                m_bf.animations.next_state = TR_STATE_LARA_JUMP_ROLL;
            }
            break;

            // Water animations.

        case TR_STATE_LARA_UNDERWATER_STOP:
            if(m_moveType != MoveType::Underwater && m_moveType != MoveType::OnWater)
            {
                setAnimation(0, 0);
            }
            else if(m_response.killed)
            {
                m_bf.animations.next_state = TR_STATE_LARA_WATER_DEATH;
            }
            else if(m_command.roll)
            {
                setAnimation(TR_ANIMATION_LARA_UNDERWATER_ROLL_BEGIN, 0);
            }
            else if(m_command.jump)
            {
                m_bf.animations.next_state = TR_STATE_LARA_UNDERWATER_FORWARD;
            }
            else if(m_moveType == MoveType::OnWater)
            {
                m_inertiaLinear = 0.0;
                m_bf.animations.next_state = TR_STATE_LARA_ONWATER_STOP;
                setAnimation(TR_ANIMATION_LARA_UNDERWATER_TO_ONWATER, 0); // go to the air
            }
            break;

        case TR_STATE_LARA_WATER_DEATH:
            if(m_moveType != MoveType::OnWater)
            {
                pos[2] += (MeteringSectorSize / 4) * engine::engine_frame_time;     // go to the air
            }
            break;

        case TR_STATE_LARA_UNDERWATER_FORWARD:
            if(m_moveType != MoveType::Underwater && m_moveType != MoveType::OnWater)
            {
                setAnimation(0, 0);
            }
            else if(m_response.killed)
            {
                m_bf.animations.next_state = TR_STATE_LARA_WATER_DEATH;
            }
            else if(curr_fc->floor_hit && curr_fc->water && (curr_fc->transition_level - curr_fc->floor_point[2] <= m_maxStepUpHeight))
            {
                setAnimation(TR_ANIMATION_LARA_UNDERWATER_TO_WADE, 0); // go to the air
                m_bf.animations.next_state = TR_STATE_LARA_STOP;
                m_climb.point = curr_fc->floor_point;  ///@FIXME: without it Lara are pulled high up, but this string was not been here.
                m_moveType = MoveType::OnFloor;
            }
            else if(m_command.roll)
            {
                setAnimation(TR_ANIMATION_LARA_UNDERWATER_ROLL_BEGIN, 0);
            }
            else if(m_moveType == MoveType::OnWater)
            {
                m_inertiaLinear = 0.0;
                m_bf.animations.next_state = TR_STATE_LARA_ONWATER_STOP;
                setAnimation(TR_ANIMATION_LARA_UNDERWATER_TO_ONWATER, 0); // go to the air
            }
            else if(!m_command.jump)
            {
                m_bf.animations.next_state = TR_STATE_LARA_UNDERWATER_INERTIA;
            }
            break;

        case TR_STATE_LARA_UNDERWATER_INERTIA:
            if(m_moveType == MoveType::OnWater)
            {
                m_inertiaLinear = 0.0;
                setAnimation(TR_ANIMATION_LARA_UNDERWATER_TO_ONWATER, 0); // go to the air
            }
            else if(m_response.killed)
            {
                m_bf.animations.next_state = TR_STATE_LARA_WATER_DEATH;
            }
            else if(m_command.roll)
            {
                setAnimation(TR_ANIMATION_LARA_UNDERWATER_ROLL_BEGIN, 0);
            }
            else if(m_command.jump)
            {
                m_bf.animations.next_state = TR_STATE_LARA_UNDERWATER_FORWARD;
            }
            else
            {
                m_bf.animations.next_state = TR_STATE_LARA_UNDERWATER_STOP;
            }
            break;

        case TR_STATE_LARA_ONWATER_STOP:
            if(m_response.killed)
            {
                m_bf.animations.next_state = TR_STATE_LARA_WATER_DEATH;
            }
            else if((m_command.move[0] == 1) || m_command.jump)                    // dive works correct only after TR_STATE_LARA_ONWATER_FORWARD
            {
                m_dirFlag = ENT_MOVE_FORWARD;
                m_bf.animations.next_state = TR_STATE_LARA_ONWATER_FORWARD;
            }
            else if(m_command.move[0] == -1)
            {
                m_dirFlag = ENT_MOVE_BACKWARD;
                m_bf.animations.next_state = TR_STATE_LARA_ONWATER_BACK;
            }
            else if(m_command.move[1] == -1)
            {
                if(m_command.shift)
                {
                    m_dirFlag = ENT_MOVE_LEFT;
                    m_command.rot[0] = 0.0;
                    m_bf.animations.next_state = TR_STATE_LARA_ONWATER_LEFT;
                }
                else
                {
                    // rotate on water
                }
            }
            else if(m_command.move[1] == 1)
            {
                if(m_command.shift)
                {
                    m_dirFlag = ENT_MOVE_RIGHT;
                    m_command.rot[0] = 0.0;
                    m_bf.animations.next_state = TR_STATE_LARA_ONWATER_RIGHT;
                }
                else
                {
                    // rotate on water
                }
            }
            else if(m_moveType == MoveType::Underwater)
            {
                m_moveType = MoveType::OnWater;
            }
            break;

        case TR_STATE_LARA_ONWATER_FORWARD:
            m_bt.no_fix_body_parts = BODY_PART_HANDS;
            m_moveType = MoveType::OnWater;

            if(m_response.killed)
            {
                m_bf.animations.next_state = TR_STATE_LARA_WATER_DEATH;
            }
            else if(m_command.jump)
            {
                t = pos[2];
                Character::getHeightInfo(pos, &next_fc);
                pos[2] = t;
                m_bf.animations.next_state = TR_STATE_LARA_UNDERWATER_FORWARD;
                m_bf.animations.onFrame = ent_set_underwater;                          // dive
            }
            else if(m_command.move[0] == 1)
            {
                if(m_command.action)
                {
                    if(m_moveType != MoveType::Climbing)
                    {
                        t = engine::LaraTryHangWallOffset + engine::LaraHangWallDistance;
                        global_offset = m_transform.getBasis().getColumn(1) * t;
                        global_offset[2] += engine::LaraHangVerticalEpsilon;                        // inc for water_surf.z
                        m_climb = checkClimbability(global_offset, &next_fc, 0.0);
                        if(m_climb.edge_hit)
                        {
                            low_vertical_space = true;
                        }
                        else
                        {
                            low_vertical_space = false;
                            global_offset[2] += m_maxStepUpHeight + engine::LaraHangVerticalEpsilon;
                            m_climb = checkClimbability(global_offset, &next_fc, 0.0);
                        }

                        if(m_climb.edge_hit && (m_climb.next_z_space >= m_height - engine::LaraHangVerticalEpsilon))// && (climb->edge_point[2] - pos[2] < ent->character->max_step_up_height))   // max_step_up_height is not correct value here
                        {
                            m_dirFlag = ENT_STAY;
                            m_moveType = MoveType::Climbing;
                            m_bt.no_fix_all = true;
                            m_angles[0] = m_climb.edge_z_ang;
                            updateTransform();
                            m_climb.point = m_climb.edge_point;
                        }
                    }

                    if(m_moveType == MoveType::Climbing)
                    {
                        m_speed.setZero();
                        m_command.rot[0] = 0.0;
                        m_bt.no_fix_all = true;
                        if(low_vertical_space)
                        {
                            setAnimation(TR_ANIMATION_LARA_ONWATER_TO_LAND_LOW, 0);
                        }
                        else
                        {
                            setAnimation(TR_ANIMATION_LARA_CLIMB_OUT_OF_WATER, 0);
                        }
                        ent_climb_out_of_water(this, &m_bf.animations, world::animation::AnimUpdate::NewAnim);
                    }
                }
                else if(!curr_fc->floor_hit || (pos[2] - m_height > curr_fc->floor_point[2] - m_swimDepth))
                {
                    //ent->last_state = ent->last_state;                          // swim forward
                }
                else
                {
                    m_bf.animations.next_state = TR_STATE_LARA_WADE_FORWARD;
                    m_bf.animations.onFrame = ent_set_on_floor;                        // to wade
                }
            }
            else
            {
                m_bf.animations.next_state = TR_STATE_LARA_ONWATER_STOP;
            }
            break;

        case TR_STATE_LARA_ONWATER_BACK:
            if(m_command.move[0] == -1 && !m_command.jump)
            {
                if(!curr_fc->floor_hit || (curr_fc->floor_point[2] + m_height < curr_fc->transition_level))
                {
                    //ent->current_state = TR_STATE_CURRENT;                      // continue swimming
                }
                else
                {
                    m_bf.animations.next_state = TR_STATE_LARA_ONWATER_STOP;
                }
            }
            else
            {
                m_bf.animations.next_state = TR_STATE_LARA_ONWATER_STOP;
            }
            break;

        case TR_STATE_LARA_ONWATER_LEFT:
            m_command.rot[0] = 0.0;
            if(!m_command.jump)
            {
                if(m_command.move[1] == -1 && m_command.shift)
                {
                    if(!curr_fc->floor_hit || (pos[2] - m_height > curr_fc->floor_point[2]))
                    {
                        // walk left
                        m_bf.animations.next_state = TR_STATE_LARA_ONWATER_LEFT;
                    }
                    else
                    {
                        // walk left
                        m_bf.animations.next_state = TR_STATE_LARA_WALK_LEFT;
                        m_bf.animations.onFrame = ent_set_on_floor;
                    }
                }
                else
                {
                    m_bf.animations.next_state = TR_STATE_LARA_ONWATER_STOP;
                }
            }
            else
            {
                m_bf.animations.next_state = TR_STATE_LARA_UNDERWATER_DIVING;
            }
            break;

        case TR_STATE_LARA_ONWATER_RIGHT:
            m_command.rot[0] = 0.0;
            if(!m_command.jump)
            {
                if(m_command.move[1] == 1 && m_command.shift)
                {
                    if(!curr_fc->floor_hit || (pos[2] - m_height > curr_fc->floor_point[2]))
                    {
                        // swim RIGHT
                        m_bf.animations.next_state = TR_STATE_LARA_ONWATER_RIGHT;
                    }
                    else
                    {
                        // walk left
                        m_bf.animations.next_state = TR_STATE_LARA_WALK_RIGHT;
                        m_bf.animations.onFrame = ent_set_on_floor;
                    }
                }
                else
                {
                    m_bf.animations.next_state = TR_STATE_LARA_ONWATER_STOP;
                }
            }
            else
            {
                m_bf.animations.next_state = TR_STATE_LARA_UNDERWATER_DIVING;
            }
            break;

            // Crouch

        case TR_STATE_LARA_CROUCH_IDLE:
            m_dirFlag = ENT_MOVE_FORWARD;
            m_bt.no_fix_body_parts = BODY_PART_HANDS_2 | BODY_PART_HANDS_3 | BODY_PART_LEGS_3;
            move[0] = pos[0];
            move[1] = pos[1];
            move[2] = pos[2] + 0.5f * (m_bf.boundingBox.max[2] - m_bf.boundingBox.min[2]);
            Character::getHeightInfo(move, &next_fc);

            lean(0.0);

            if(!m_command.crouch && !low_vertical_space)
            {
                m_bf.animations.next_state = TR_STATE_LARA_STOP;                        // Back to stand
            }
            else if((m_command.move[0] != 0) || m_response.killed)
            {
                m_bf.animations.next_state = TR_STATE_LARA_CRAWL_IDLE;                  // Both forward & back provoke crawl stage
            }
            else if(m_command.jump)
            {
                m_bf.animations.next_state = TR_STATE_LARA_CROUCH_ROLL;                 // Crouch roll
            }
            else
            {
                if(engine::engine_world.engineVersion > loader::Engine::TR3)
                {
                    if(m_command.move[1] == 1)
                    {
                        m_dirFlag = ENT_MOVE_FORWARD;
                        m_bf.animations.next_state = TR_STATE_LARA_CROUCH_TURN_RIGHT;
                    }
                    else if(m_command.move[1] == -1)
                    {
                        m_dirFlag = ENT_MOVE_FORWARD;
                        m_bf.animations.next_state = TR_STATE_LARA_CROUCH_TURN_LEFT;
                    }
                }
                else
                {
                    m_command.rot[0] = 0.0;
                }
            }
            break;

        case TR_STATE_LARA_CROUCH_ROLL:
        case TR_STATE_LARA_SPRINT_ROLL:
            m_command.rot[0] = 0.0;
            lean(0.0);
            if(m_moveType == MoveType::FreeFalling)
            {
                setAnimation(TR_ANIMATION_LARA_FREE_FALL_FORWARD, 0);
            }

            move = m_transform.getBasis().getColumn(1) * engine::PenetrationTestOffset;
            if((checkNextPenetration(move) > 0) && (m_response.horizontal_collide != 0x00))  // Smash into wall
            {
                m_bf.animations.next_state = TR_STATE_LARA_STOP;
            }
            break;

        case TR_STATE_LARA_CRAWL_IDLE:
            m_dirFlag = ENT_MOVE_FORWARD;
            m_bt.no_fix_body_parts = BODY_PART_HANDS_2 | BODY_PART_HANDS_3 | BODY_PART_LEGS_3;
            if(m_response.killed)
            {
                m_dirFlag = ENT_STAY;
                m_bf.animations.next_state = TR_STATE_LARA_DEATH;
            }
            else if(m_command.move[1] == -1)
            {
                m_dirFlag = ENT_MOVE_FORWARD;
                setAnimation(TR_ANIMATION_LARA_CRAWL_TURN_LEFT, 0);
            }
            else if(m_command.move[1] == 1)
            {
                m_dirFlag = ENT_MOVE_FORWARD;
                setAnimation(TR_ANIMATION_LARA_CRAWL_TURN_RIGHT, 0);
            }
            else if(m_command.move[0] == 1)
            {
                move = m_transform.getBasis().getColumn(1) * engine::PenetrationTestOffset;
                if((checkNextPenetration(move) == 0) || (m_response.horizontal_collide == 0x00))
                {
                    global_offset = m_transform.getBasis().getColumn(1) * engine::CrawlForwardOffset;
                    global_offset[2] += 0.5f * (m_bf.boundingBox.max[2] + m_bf.boundingBox.min[2]);
                    global_offset += pos;
                    Character::getHeightInfo(global_offset, &next_fc);
                    if((next_fc.floor_point[2] < pos[2] + m_minStepUpHeight) &&
                       (next_fc.floor_point[2] > pos[2] - m_minStepUpHeight))
                    {
                        m_bf.animations.next_state = TR_STATE_LARA_CRAWL_FORWARD;           // In TR4+, first state is crawlspace jump
                    }
                }
            }
            else if(m_command.move[0] == -1)
            {
                move = m_transform.getBasis().getColumn(1) * -engine::PenetrationTestOffset;
                if((checkNextPenetration(move) == 0) || (m_response.horizontal_collide == 0x00))
                {
                    global_offset = m_transform.getBasis().getColumn(1) * -engine::CrawlForwardOffset;
                    global_offset[2] += 0.5f * (m_bf.boundingBox.max[2] + m_bf.boundingBox.min[2]);
                    global_offset += pos;
                    Character::getHeightInfo(global_offset, &next_fc);
                    if((next_fc.floor_point[2] < pos[2] + m_minStepUpHeight) &&
                       (next_fc.floor_point[2] > pos[2] - m_minStepUpHeight))
                    {
                        m_dirFlag = ENT_MOVE_BACKWARD;
                        m_bf.animations.next_state = TR_STATE_LARA_CRAWL_BACK;
                    }
                    else if(m_command.action && (m_response.horizontal_collide == 0) &&
                            (next_fc.floor_point[2] < pos[2] - m_height))
                    {
                        auto temp = pos;                                       // save entity position
                        pos[0] = next_fc.floor_point[0];
                        pos[1] = next_fc.floor_point[1];
                        global_offset = m_transform.getBasis().getColumn(1) * 0.5 * engine::CrawlForwardOffset;
                        global_offset[2] += 128.0;
                        curr_fc->floor_hit = next_fc.floor_hit;
                        curr_fc->floor_point = next_fc.floor_point;
                        curr_fc->floor_normale = next_fc.floor_normale;
                        curr_fc->floor_obj = next_fc.floor_obj;
                        curr_fc->ceiling_hit = next_fc.ceiling_hit;
                        curr_fc->ceiling_point = next_fc.ceiling_point;
                        curr_fc->ceiling_normale = next_fc.ceiling_normale;
                        curr_fc->ceiling_obj = next_fc.ceiling_obj;

                        m_climb = checkClimbability(global_offset, &next_fc, 1.5f * m_bf.boundingBox.max[2]);
                        pos = temp;                                       // restore entity position
                        if(m_climb.can_hang)
                        {
                            m_angles[0] = m_climb.edge_z_ang;
                            m_dirFlag = ENT_MOVE_BACKWARD;
                            m_moveType = MoveType::Climbing;
                            m_climb.point = m_climb.edge_point;
                            m_bf.animations.next_state = TR_STATE_LARA_CRAWL_TO_CLIMB;
                        }
                    }
                }
            }
            else if(!m_command.crouch)
            {
                m_bf.animations.next_state = TR_STATE_LARA_CROUCH_IDLE;                // Back to crouch.
            }
            break;

        case TR_STATE_LARA_CRAWL_TO_CLIMB:
            m_bt.no_fix_all = true;
            m_bf.animations.onFrame = ent_crawl_to_climb;
            break;

        case TR_STATE_LARA_CRAWL_FORWARD:
            m_dirFlag = ENT_MOVE_FORWARD;
            m_bt.no_fix_body_parts = BODY_PART_HANDS_2 | BODY_PART_HANDS_3 | BODY_PART_LEGS_3;
            m_command.rot[0] = m_command.rot[0] * 0.5f;
            move = m_transform.getBasis().getColumn(1) * engine::PenetrationTestOffset;
            if((checkNextPenetration(move) > 0) && (m_response.horizontal_collide != 0x00))
            {
                m_dirFlag = ENT_STAY;
                setAnimation(TR_ANIMATION_LARA_CRAWL_IDLE, 0);
                break;
            }
            global_offset = m_transform.getBasis().getColumn(1) * engine::CrawlForwardOffset;
            global_offset[2] += 0.5f * (m_bf.boundingBox.max[2] + m_bf.boundingBox.min[2]);
            global_offset += pos;
            Character::getHeightInfo(global_offset, &next_fc);

            if((m_command.move[0] != 1) || m_response.killed)
            {
                m_bf.animations.next_state = TR_STATE_LARA_CRAWL_IDLE; // Stop
            }
            else if((next_fc.floor_point[2] >= pos[2] + m_minStepUpHeight) ||
                    (next_fc.floor_point[2] <= pos[2] - m_minStepUpHeight))
            {
                m_dirFlag = ENT_STAY;
                setAnimation(TR_ANIMATION_LARA_CRAWL_IDLE, 0);
            }
            break;

        case TR_STATE_LARA_CRAWL_BACK:
            m_dirFlag = ENT_MOVE_FORWARD;   // Absurd? No, Core Design.
            m_bt.no_fix_body_parts = BODY_PART_HANDS_2 | BODY_PART_HANDS_3 | BODY_PART_LEGS_3;
            m_command.rot[0] = m_command.rot[0] * 0.5f;
            move = m_transform.getBasis().getColumn(1) * -engine::PenetrationTestOffset;
            if((checkNextPenetration(move) > 0) && (m_response.horizontal_collide != 0x00))
            {
                m_dirFlag = ENT_STAY;
                setAnimation(TR_ANIMATION_LARA_CRAWL_IDLE, 0);
                break;
            }
            global_offset = m_transform.getBasis().getColumn(1) * -engine::CrawlForwardOffset;
            global_offset[2] += 0.5f * (m_bf.boundingBox.max[2] + m_bf.boundingBox.min[2]);
            global_offset += pos;
            Character::getHeightInfo(global_offset, &next_fc);
            if((m_command.move[0] != -1) || m_response.killed)
            {
                m_bf.animations.next_state = TR_STATE_LARA_CRAWL_IDLE; // Stop
            }
            else if((next_fc.floor_point[2] >= pos[2] + m_minStepUpHeight) ||
                    (next_fc.floor_point[2] <= pos[2] - m_minStepUpHeight))
            {
                m_dirFlag = ENT_STAY;
                setAnimation(TR_ANIMATION_LARA_CRAWL_IDLE, 0);
            }
            break;

        case TR_STATE_LARA_CRAWL_TURN_LEFT:
            m_dirFlag = ENT_MOVE_FORWARD;
            m_bt.no_fix_body_parts = BODY_PART_HANDS_2 | BODY_PART_HANDS_3 | BODY_PART_LEGS_3;
            m_command.rot[0] *= ((m_bf.animations.current_frame > 3) && (m_bf.animations.current_frame < 14)) ? (1.0f) : (0.0f);

            if((m_command.move[1] != -1) || m_response.killed)
            {
                m_bf.animations.next_state = TR_STATE_LARA_CRAWL_IDLE; // stop
            }
            break;

        case TR_STATE_LARA_CRAWL_TURN_RIGHT:
            m_dirFlag = ENT_MOVE_FORWARD;
            m_bt.no_fix_body_parts = BODY_PART_HANDS_2 | BODY_PART_HANDS_3 | BODY_PART_LEGS_3;
            m_command.rot[0] *= ((m_bf.animations.current_frame > 3) && (m_bf.animations.current_frame < 14)) ? (1.0f) : (0.0f);

            if((m_command.move[1] != 1) || m_response.killed)
            {
                m_bf.animations.next_state = TR_STATE_LARA_CRAWL_IDLE; // stop
            }
            break;

        case TR_STATE_LARA_CROUCH_TURN_LEFT:
        case TR_STATE_LARA_CROUCH_TURN_RIGHT:
            m_bt.no_fix_body_parts = BODY_PART_HANDS_2 | BODY_PART_HANDS_3 | BODY_PART_LEGS_3;
            m_command.rot[0] *= ((m_bf.animations.current_frame > 3) && (m_bf.animations.current_frame < 23)) ? (0.6f) : (0.0f);

            if((m_command.move[1] == 0) || m_response.killed)
            {
                m_bf.animations.next_state = TR_STATE_LARA_CROUCH_IDLE;
            }
            break;

            // Monkeyswing

        case TR_STATE_LARA_MONKEYSWING_IDLE:
            m_command.rot[0] = 0.0;
            m_dirFlag = ENT_STAY;
            ///@FIXME: stick for TR3+ monkey swing fix... something wrong with anim 150
            if(m_command.action && (m_moveType != MoveType::Monkeyswing) && curr_fc->ceiling_climb && (curr_fc->ceiling_hit) && (pos[2] + m_bf.boundingBox.max[2] > curr_fc->ceiling_point[2] - 96.0))
            {
                m_moveType = MoveType::Monkeyswing;
                setAnimation(TR_ANIMATION_LARA_MONKEY_IDLE, 0);
                m_bf.animations.next_state = TR_STATE_LARA_MONKEYSWING_IDLE;
                pos[2] = m_heightInfo.ceiling_point[2] - m_bf.boundingBox.max[2];
            }

            if((m_moveType != MoveType::Monkeyswing) || !m_command.action)
            {
                setAnimation(TR_ANIMATION_LARA_TRY_HANG_VERTICAL, 0);
                m_dirFlag = ENT_STAY;
                m_moveType = MoveType::FreeFalling;
            }
            else if(m_command.shift && (m_command.move[1] == -1))
            {
                m_bf.animations.next_state = TR_STATE_LARA_MONKEYSWING_LEFT;
            }
            else if(m_command.shift && (m_command.move[1] == 1))
            {
                m_bf.animations.next_state = TR_STATE_LARA_MONKEYSWING_RIGHT;
            }
            else if(m_command.move[0] == 1)
            {
                m_dirFlag = ENT_MOVE_FORWARD;
                m_bf.animations.next_state = TR_STATE_LARA_MONKEYSWING_FORWARD;
            }
            else if(m_command.move[1] == -1)
            {
                m_bf.animations.next_state = TR_STATE_LARA_MONKEYSWING_TURN_LEFT;
            }
            else if(m_command.move[1] == 1)
            {
                m_bf.animations.next_state = TR_STATE_LARA_MONKEYSWING_TURN_RIGHT;
            }
            break;

        case TR_STATE_LARA_MONKEYSWING_TURN_LEFT:
            m_command.rot[0] *= 0.5;
            if((m_moveType != MoveType::Monkeyswing) || !m_command.action)
            {
                setAnimation(TR_ANIMATION_LARA_TRY_HANG_VERTICAL, 0);
                m_dirFlag = ENT_STAY;
                m_moveType = MoveType::FreeFalling;
            }
            else if(m_command.move[1] != -1)
            {
                m_bf.animations.next_state = TR_STATE_LARA_MONKEYSWING_IDLE;
            }
            break;

        case TR_STATE_LARA_MONKEYSWING_TURN_RIGHT:
            m_command.rot[0] *= 0.5;
            if((m_moveType != MoveType::Monkeyswing) || !m_command.action)
            {
                setAnimation(TR_ANIMATION_LARA_TRY_HANG_VERTICAL, 0);
                m_dirFlag = ENT_STAY;
                m_moveType = MoveType::FreeFalling;
            }
            else if(m_command.move[1] != 1)
            {
                m_bf.animations.next_state = TR_STATE_LARA_MONKEYSWING_IDLE;
            }
            break;

        case TR_STATE_LARA_MONKEYSWING_FORWARD:
            m_command.rot[0] *= 0.45f;
            m_dirFlag = ENT_MOVE_FORWARD;

            if((m_moveType != MoveType::Monkeyswing) || !m_command.action)
            {
                setAnimation(TR_ANIMATION_LARA_TRY_HANG_VERTICAL, 0);
                m_moveType = MoveType::FreeFalling;
            }
            else if(m_command.move[0] != 1)
            {
                m_bf.animations.next_state = TR_STATE_LARA_MONKEYSWING_IDLE;
            }
            break;

        case TR_STATE_LARA_MONKEYSWING_LEFT:
            m_command.rot[0] = 0.0;
            m_dirFlag = ENT_MOVE_LEFT;

            if((m_moveType != MoveType::Monkeyswing) || !m_command.action)
            {
                setAnimation(TR_ANIMATION_LARA_TRY_HANG_VERTICAL, 0);
                m_moveType = MoveType::FreeFalling;
            }
            else if(m_command.move[0] != 1)
            {
                m_bf.animations.next_state = TR_STATE_LARA_MONKEYSWING_IDLE;
            }
            break;

        case TR_STATE_LARA_MONKEYSWING_RIGHT:
            m_command.rot[0] = 0.0;
            m_dirFlag = ENT_MOVE_RIGHT;

            if((m_moveType != MoveType::Monkeyswing) || !m_command.action)
            {
                setAnimation(TR_ANIMATION_LARA_TRY_HANG_VERTICAL, 0);
                m_moveType = MoveType::FreeFalling;
            }
            else if(m_command.move[0] != 1)
            {
                m_bf.animations.next_state = TR_STATE_LARA_MONKEYSWING_IDLE;
            }
            break;

            // Tightrope

        case TR_STATE_LARA_TIGHTROPE_ENTER:
            m_command.rot[0] = 0.0;
            m_bt.no_fix_all = true;
            m_dirFlag = ENT_MOVE_FORWARD;
            m_bf.animations.onFrame = ent_to_tightrope;
            m_bf.animations.next_state = TR_STATE_LARA_TIGHTROPE_IDLE;
            break;

        case TR_STATE_LARA_TIGHTROPE_EXIT:
            m_command.rot[0] = 0.0;
            m_bt.no_fix_all = true;
            m_dirFlag = ENT_MOVE_FORWARD;
            m_bf.animations.onFrame = ent_from_tightrope;
            m_bf.animations.next_state = TR_STATE_LARA_STOP;
            break;

        case TR_STATE_LARA_TIGHTROPE_IDLE:
            m_command.rot[0] = 0.0;

            if(m_bf.animations.current_animation == TR_ANIMATION_LARA_TIGHTROPE_STAND)
            {
                if(m_response.lean == LeanType::Left)
                {
                    m_bf.animations.next_state = TR_STATE_LARA_TIGHTROPE_BALANCING_LEFT;
                    m_response.lean = LeanType::None;
                    break;
                }
                else if(m_response.lean == LeanType::Right)
                {
                    m_bf.animations.next_state = TR_STATE_LARA_TIGHTROPE_BALANCING_RIGHT;
                    m_response.lean = LeanType::None;
                    break;
                }
                else if(last_frame)
                {
                    uint16_t chance_to_fall = rand() % 0x7FFF;

                    if(chance_to_fall > 0x5FFF)
                        m_bf.animations.next_state = TR_STATE_LARA_TIGHTROPE_BALANCING_LEFT;
                    else if(chance_to_fall < 0x2000)
                        m_bf.animations.next_state = TR_STATE_LARA_TIGHTROPE_BALANCING_RIGHT;
                }
            }

            if((m_command.roll) || (m_command.move[0] == -1))
            {
                setAnimation(TR_ANIMATION_LARA_TIGHTROPE_TURN, 0);
                m_dirFlag = ENT_MOVE_FORWARD;
            }
            else if(m_command.move[0] == 1)
            {
                m_bf.animations.next_state = TR_STATE_LARA_TIGHTROPE_FORWARD;
            }
            break;

        case TR_STATE_LARA_TIGHTROPE_FORWARD:
            m_command.rot[0] = 0.0;
            m_dirFlag = ENT_MOVE_FORWARD;

            if(m_command.move[0] != 1)
            {
                m_bf.animations.next_state = TR_STATE_LARA_TIGHTROPE_IDLE;
            }
            else
            {
                uint16_t chance_to_unbal = rand() % 0x7FFF;
                if(chance_to_unbal < 0x00FF)
                {
                    m_bf.animations.next_state = TR_STATE_LARA_TIGHTROPE_IDLE;

                    if(chance_to_unbal > 0x007F)
                        m_response.lean = LeanType::Left;
                    else
                        m_response.lean = LeanType::Right;
                }
            }
            break;

        case TR_STATE_LARA_TIGHTROPE_BALANCING_RIGHT:
            m_command.rot[0] = 0.0;

            if((m_bf.animations.current_animation == TR_ANIMATION_LARA_TIGHTROPE_FALL_RIGHT) && (last_frame))
            {
                m_moveType = MoveType::FreeFalling;
                m_transform.getOrigin() += m_transform.getBasis() * btVector3(256.0, 192.0, -640.0);
                setAnimation(TR_ANIMATION_LARA_FREE_FALL_LONG, 0);
            }
            else if((m_bf.animations.current_animation == TR_ANIMATION_LARA_TIGHTROPE_LOOSE_RIGHT) && (m_bf.animations.current_frame >= m_bf.animations.model->animations[m_bf.animations.current_animation].frames.size() / 2) && (m_command.move[1] == -1))
            {
                // MAGIC: mirroring animation offset.
                setAnimation(TR_ANIMATION_LARA_TIGHTROPE_RECOVER_RIGHT, m_bf.animations.model->animations[m_bf.animations.current_animation].frames.size()-m_bf.animations.current_frame);
            }
            break;

        case TR_STATE_LARA_TIGHTROPE_BALANCING_LEFT:
            m_command.rot[0] = 0.0;

            if((m_bf.animations.current_animation == TR_ANIMATION_LARA_TIGHTROPE_FALL_LEFT) && (last_frame))
            {
                m_moveType = MoveType::FreeFalling;
                setAnimation(TR_ANIMATION_LARA_FREE_FALL_LONG, 0);
                m_transform.getOrigin() += m_transform.getBasis() * btVector3(-256.0, 192.0, -640.0);
            }
            else if((m_bf.animations.current_animation == TR_ANIMATION_LARA_TIGHTROPE_LOOSE_LEFT) && (m_bf.animations.current_frame >= m_bf.animations.model->animations[m_bf.animations.current_animation].frames.size() / 2) && (m_command.move[1] == 1))
            {
                // MAGIC: mirroring animation offset.
                setAnimation(TR_ANIMATION_LARA_TIGHTROPE_RECOVER_LEFT, m_bf.animations.model->animations[m_bf.animations.current_animation].frames.size()-m_bf.animations.current_frame);
            }
            break;


            // Intermediate animations are processed automatically.

        default:
            m_command.rot[0] = 0.0;
            if((m_moveType == MoveType::Monkeyswing) || (m_moveType == MoveType::WallsClimb))
            {
                if(!m_command.action)
                {
                    setAnimation(TR_ANIMATION_LARA_START_FREE_FALL, 0);
                    m_dirFlag = ENT_STAY;
                    m_moveType = MoveType::FreeFalling;
                }
            }
            break;
    };

    // Extra animation control.

    switch(m_bf.animations.current_animation)
    {
        case TR_ANIMATION_LARA_STAY_JUMP_SIDES:
            m_bt.no_fix_body_parts |= BODY_PART_HEAD;
            break;

        case TR_ANIMATION_LARA_TRY_HANG_SOLID:
        case TR_ANIMATION_LARA_FLY_FORWARD_TRY_HANG:
            if((m_moveType == MoveType::FreeFalling) && m_command.action &&
               (m_speed[0] * m_transform.getBasis().getColumn(1)[0] + m_speed[1] * m_transform.getBasis().getColumn(1)[1] < 0.0))
            {
                m_speed[0] = -m_speed[0];
                m_speed[1] = -m_speed[1];
            }
            break;

        case TR_ANIMATION_LARA_AH_BACKWARD:
        case TR_ANIMATION_LARA_AH_FORWARD:
        case TR_ANIMATION_LARA_AH_LEFT:
        case TR_ANIMATION_LARA_AH_RIGHT:
            if(m_bf.animations.current_frame > 12)
                setAnimation(TR_ANIMATION_LARA_STAY_SOLID, 0);
            break;
    };
}
} // namespace world
