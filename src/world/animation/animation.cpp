#include "animation.h"

#include "engine/engine.h"
#include "engine/game.h"
#include "render/vertex_array.h"
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
    parents[0] = NULL;
    bone_tags[0].parent = NULL;                                             // root
    for(uint16_t i = 0; i<bone_tags.size(); i++)
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

void BoneFrame_Copy(BoneFrame *dst, BoneFrame *src)
{
    dst->bone_tags.resize(src->bone_tags.size());
    dst->position = src->position;
    dst->center = src->center;
    dst->boundingBox.max = src->boundingBox.max;
    dst->boundingBox.min = src->boundingBox.min;

    dst->animCommands = src->animCommands;

    for(uint16_t i = 0; i < dst->bone_tags.size(); i++)
    {
        dst->bone_tags[i].qrotate = src->bone_tags[i].qrotate;
        dst->bone_tags[i].offset = src->bone_tags[i].offset;
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

    period = 1.0f / TR_FRAME_RATE;
    //    m_bf.animations.lerp = 0.0f;
    //    m_bf.animations.frame_time = 0.0f;
    return;
}

/**
* Find a possible state change to \c stateid
* \param[in]      stateid  the desired target state
* \param[in,out]  animid   reference to anim id, receives found anim
* \param[in,out]  frameid  reference to frame id, receives found frame
* \return  true if state is found, false otherwise
*/
bool SSAnimation::findStateChange(uint32_t stateid, uint16_t& animid_inout, uint16_t& frameid_inout)
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
    uint16_t frame_id, anim_id;
    const std::vector<AnimationFrame>& animlist = model->animations;
    AnimUpdate stepResult = AnimUpdate::NewFrame;

    anim_id = current_animation;
    frame_id = current_frame;

    // --------
    // FIXME: hack for reverse framesteps (weaponanims):
    if(time < 0.0f)
    {
        frame_time -= time;
        if(frame_time + GAME_LOGIC_REFRESH_INTERVAL / 2.0f < period)
        {
            lerp = frame_time / period;
            stepResult = AnimUpdate::None;
        }
        else
        {
            lerp_last_animation = anim_id;
            lerp_last_frame = frame_id;
            frame_time = 0.0f;
            lerp = 0.0f;
            if(frame_id > 0)
            {
                frame_id--;
                stepResult = AnimUpdate::NewFrame;
            }
            else
            {
                frame_id = animlist[anim_id].frames.size() - 1;
                stepResult = AnimUpdate::NewAnim;
            }
            current_frame = frame_id;
        }
        return stepResult;
    }
    // --------

    frame_time += time;
    if(frame_time + GAME_LOGIC_REFRESH_INTERVAL / 2.0f < period)
    {
        lerp = frame_time / period; // re-sync
        return AnimUpdate::None;
    }

    lerp_last_animation = anim_id;
    lerp_last_frame = frame_id;
    frame_time = 0.0f;
    lerp = 0.0f;

    frame_id++;

    // check anim flags:
    if(mode == SSAnimationMode::LoopLastFrame)
    {
        if(frame_id >= static_cast<int>(animlist[anim_id].frames.size()))
        {
            current_frame = animlist[anim_id].frames.size() - 1;
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
        if(frame_id >= animlist[anim_id].frames.size())
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
    if(next_state != last_state)
    {
        if(findStateChange(next_state, anim_id, frame_id))
        {
            last_state = animlist[anim_id].state_id;
            next_state = last_state;
            stepResult = AnimUpdate::NewAnim;
        }
    }

    // check end of animation:
    if(frame_id >= animlist[anim_id].frames.size())
    {
        if(cmdEntity)
        {
            for(AnimCommand acmd : animlist[anim_id].animCommands)  // end-of-anim cmdlist
            {
                cmdEntity->doAnimCommand(acmd);
            }
        }
        if(animlist[anim_id].next_anim)
        {
            frame_id = animlist[anim_id].next_frame;
            anim_id = animlist[anim_id].next_anim->id;

            // some overlay anims may have invalid nextAnim/nextFrame values:
            if(anim_id < animlist.size() && frame_id < animlist[anim_id].frames.size())
            {
                last_state = animlist[anim_id].state_id;
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
        for(AnimCommand acmd : animlist[anim_id].frames[frame_id].animCommands)  // frame cmdlist
        {
            cmdEntity->doAnimCommand(acmd);
        }
    }
    return stepResult;
}

} // namespace animation
} // namespace world
