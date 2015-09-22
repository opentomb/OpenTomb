#include "animation.h"

#include "engine/engine.h"
#include "engine/game.h"
#include "world/entity.h"
#include "world/skeletalmodel.h"

namespace world
{
namespace animation
{

void SSBoneFrame::fromModel(SkeletalModel* model)
{
    hasSkin = false;
    boundingBox.min.setZero();
    boundingBox.max.setZero();
    center.setZero();
    position.setZero();
    animations = SSAnimation();

    animations.next = nullptr;
    animations.onFrame = nullptr;
    animations.model = model;
    bone_tags.resize(model->mesh_count);

    int stack = 0;
    std::vector<SSBoneTag*> parents(bone_tags.size());
    parents[0] = nullptr;
    bone_tags[0].parent = nullptr;                                             // root
    for(size_t i = 0; i<bone_tags.size(); i++)
    {
        bone_tags[i].index = i;
        bone_tags[i].mesh_base = model->mesh_tree[i].mesh_base;
        bone_tags[i].mesh_skin = model->mesh_tree[i].mesh_skin;
        if(bone_tags[i].mesh_skin)
            hasSkin = true;
        bone_tags[i].mesh_slot = nullptr;
        bone_tags[i].body_part = model->mesh_tree[i].body_part;

        bone_tags[i].offset = model->mesh_tree[i].offset;
        bone_tags[i].qrotate = { 0,0,0,0 };
        bone_tags[i].transform.setIdentity();
        bone_tags[i].full_transform.setIdentity();

        if(i > 0)
        {
            bone_tags[i].parent = &bone_tags[i - 1];
            if(model->mesh_tree[i].flag & 0x01)                                 // POP
            {
                if(stack > 0)
                {
                    bone_tags[i].parent = parents[stack];
                    stack--;
                }
            }
            if(model->mesh_tree[i].flag & 0x02)                                 // PUSH
            {
                if(stack + 1 < static_cast<int16_t>(model->mesh_count))
                {
                    stack++;
                    parents[stack] = bone_tags[i].parent;
                }
            }
        }
    }
}

void SSBoneFrame::itemFrame(btScalar time)
{
    animations.stepAnimation(time);
    updateCurrentBoneFrame();
}

void SSBoneFrame::updateCurrentBoneFrame()
{
    animation::BoneFrame* next_bf = &animations.model->animations[animations.current_animation].frames[animations.current_frame];
    animation::BoneFrame* last_bf = &animations.model->animations[animations.lerp_last_animation].frames[animations.lerp_last_frame];

    boundingBox.max = last_bf->boundingBox.max.lerp(next_bf->boundingBox.max, animations.lerp);
    boundingBox.min = last_bf->boundingBox.min.lerp(next_bf->boundingBox.min, animations.lerp);
    center = last_bf->center.lerp(next_bf->center, animations.lerp);
    position = last_bf->position.lerp(next_bf->position, animations.lerp);

    for(uint16_t k = 0; k < last_bf->bone_tags.size(); k++)
    {
        bone_tags[k].offset = last_bf->bone_tags[k].offset.lerp(next_bf->bone_tags[k].offset, animations.lerp);
        bone_tags[k].transform.getOrigin() = bone_tags[k].offset;
        bone_tags[k].transform.getOrigin()[3] = 1.0;
        if(k == 0)
        {
            bone_tags[k].transform.getOrigin() += position;
            bone_tags[k].qrotate = util::Quat_Slerp(last_bf->bone_tags[k].qrotate, next_bf->bone_tags[k].qrotate, animations.lerp);
        }
        else
        {
            animation::BoneTag* ov_src_btag = &last_bf->bone_tags[k];
            animation::BoneTag* ov_next_btag = &next_bf->bone_tags[k];
            btScalar ov_lerp = animations.lerp;
            for(animation::SSAnimation* ov_anim = animations.next; ov_anim != nullptr; ov_anim = ov_anim->next)
            {
                if(ov_anim->model == nullptr || !ov_anim->model->mesh_tree[k].replace_anim)
                    continue;

                animation::BoneFrame* ov_last_bf = &ov_anim->model->animations[ov_anim->lerp_last_animation].frames[ov_anim->lerp_last_frame];
                animation::BoneFrame* ov_next_bf = &ov_anim->model->animations[ov_anim->current_animation].frames[ov_anim->current_frame];
                ov_src_btag = &ov_last_bf->bone_tags[k];
                ov_next_btag = &ov_next_bf->bone_tags[k];
                ov_lerp = ov_anim->lerp;
                break;
            }
            bone_tags[k].qrotate = util::Quat_Slerp(ov_src_btag->qrotate, ov_next_btag->qrotate, ov_lerp);
        }
        bone_tags[k].transform.setRotation(bone_tags[k].qrotate);
    }

    /*
     * build absolute coordinate matrix system
     */
    bone_tags[0].full_transform = bone_tags[0].transform;
    for(size_t k = 1; k < last_bf->bone_tags.size(); k++)
    {
        bone_tags[k].full_transform = bone_tags[k].parent->full_transform * bone_tags[k].transform;
    }
}

void SSAnimation::setAnimation(int animation, int frame, int another_model)
{
    if(!model || animation >= static_cast<int>(model->animations.size()))
    {
        return;
    }
    // FIXME: is anim < 0 actually happening?
    animation = (animation < 0) ? (0) : (animation);

    if(another_model >= 0)
    {
        SkeletalModel* new_model = engine::engine_world.getModelByID(another_model);
        if(!new_model || animation >= static_cast<int>(new_model->animations.size()))
            return;
        model = new_model;
    }

    AnimationFrame* anim = &model->animations[animation];

    if(frame < 0)
        frame = anim->frames.size() - 1 - ((-frame - 1) % anim->frames.size());
    else
        frame %= anim->frames.size();

    current_animation = animation;
    current_frame = frame;
    last_state = anim->state_id;
    next_state = anim->state_id;

    //    m_bf.animations.lerp = 0.0f;
    //    m_bf.animations.frame_time = 0.0f;
}

/**
* Find a possible state change to \c stateid
* \param[in]      stateid  the desired target state
* \param[in,out]  animid   reference to anim id, receives found anim
* \param[in,out]  frameid  reference to frame id, receives found frame
* \return  true if state is found, false otherwise
*/
bool SSAnimation::findStateChange(LaraState stateid, uint16_t& animid_inout, uint16_t& frameid_inout)
{
    const std::vector<StateChange>& stclist = model->animations[animid_inout].stateChanges;

    for(const StateChange& stc : stclist)
    {
        if(stc.id == stateid)
        {
            for(const AnimDispatch& dispatch : stc.anim_dispatch)
            {
                if(frameid_inout >= dispatch.frame_low
                   && frameid_inout <= dispatch.frame_high)
                {
                    animid_inout = dispatch.next_anim;
                    frameid_inout = dispatch.next_frame;
                    return true;
                }
            }
        }
    }
    return false;
}

/**
* Advance animation frame
* @param time          time step for animation
* @param cmdEntity     optional entity for which doAnimCommand is called
* @return  ENTITY_ANIM_NONE if frame is unchanged (time<rate), ENTITY_ANIM_NEWFRAME or ENTITY_ANIM_NEWANIM
*/
AnimUpdate SSAnimation::stepAnimation(btScalar time, Entity* cmdEntity)
{
    AnimUpdate stepResult = AnimUpdate::NewFrame;

    // --------
    // FIXME: hack for reverse framesteps (weaponanims):
    if(time < 0.0f)
    {
        frame_time -= time;
        if(frame_time + 1 / animation::GameLogicFrameRate / 2.0f < 1/FrameRate)
        {
            lerp = frame_time * FrameRate;
            return AnimUpdate::None;
        }
        else
        {
            lerp_last_animation = current_animation;
            lerp_last_frame = current_frame;
            frame_time = 0.0f;
            lerp = 0.0f;
            if(current_frame > 0)
            {
                current_frame--;
                stepResult = AnimUpdate::NewFrame;
            }
            else
            {
                current_frame = getCurrentAnimationFrame().frames.size() - 1;
                stepResult = AnimUpdate::NewAnim;
            }
        }
        return stepResult;
    }
    // --------

    frame_time += time;
    if(frame_time + 1 / animation::GameLogicFrameRate / 2.0f < 1/FrameRate)
    {
        lerp = frame_time * FrameRate; // re-sync
        return AnimUpdate::None;
    }

    lerp_last_animation = current_animation;
    lerp_last_frame = current_frame;
    frame_time = 0.0f;
    lerp = 0.0f;

    uint16_t frame_id = current_frame + 1;

    // check anim flags:
    if(mode == SSAnimationMode::LoopLastFrame)
    {
        if(frame_id >= static_cast<int>(getCurrentAnimationFrame().frames.size()))
        {
            current_frame = getCurrentAnimationFrame().frames.size() - 1;
            return AnimUpdate::NewFrame;    // period time has passed so it's a new frame, or should this be none?
        }
    }
    else if(mode == SSAnimationMode::Locked)
    {
        current_frame = 0;
        return AnimUpdate::NewFrame;
    }
    else if(mode == SSAnimationMode::WeaponCompat)
    {
        if(frame_id >= getCurrentAnimationFrame().frames.size())
        {
            frame_id = 0;
            return AnimUpdate::NewAnim;
        }
        else
        {
            current_frame = frame_id;
            return AnimUpdate::NewFrame;
        }
    }

    // check state change:
    uint16_t anim_id = current_animation;
    if(next_state != last_state)
    {
        if(findStateChange(next_state, anim_id, frame_id))
        {
            last_state = model->animations[anim_id].state_id;
            next_state = last_state;
            stepResult = AnimUpdate::NewAnim;
        }
    }

    // check end of animation:
    if(frame_id >= model->animations[anim_id].frames.size())
    {
        if(cmdEntity)
        {
            for(AnimCommand acmd : model->animations[anim_id].animCommands)  // end-of-anim cmdlist
            {
                cmdEntity->doAnimCommand(acmd);
            }
        }
        if(model->animations[anim_id].next_anim)
        {
            frame_id = model->animations[anim_id].next_frame;
            anim_id = model->animations[anim_id].next_anim->id;

            // some overlay anims may have invalid nextAnim/nextFrame values:
            if(anim_id < model->animations.size() && frame_id < model->animations[anim_id].frames.size())
            {
                last_state = model->animations[anim_id].state_id;
                next_state = last_state;
            }
            else
            {
                // invalid values:
                anim_id = current_animation;
                frame_id = 0;
            }
        }
        else
        {
            frame_id = 0;
        }
        stepResult = AnimUpdate::NewAnim;
    }

    current_animation = anim_id;
    current_frame = frame_id;

    if(cmdEntity)
    {
        for(AnimCommand acmd : model->animations[anim_id].frames[frame_id].animCommands)  // frame cmdlist
        {
            cmdEntity->doAnimCommand(acmd);
        }
    }
    return stepResult;
}

const AnimationFrame &SSAnimation::getCurrentAnimationFrame() const
{
    return model->animations[current_animation];
}

} // namespace animation
} // namespace world
