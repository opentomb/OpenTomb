#include "animation.h"

#include "engine/engine.h"
#include "world/core/basemesh.h"
#include "world/entity.h"
#include "world/skeletalmodel.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <boost/log/trivial.hpp>

#include <stack>

namespace world
{
namespace animation
{

void Skeleton::fromModel(SkeletalModel* model)
{
    m_hasSkin = false;
    m_boundingBox.min = { 0,0,0 };
    m_boundingBox.max = { 0,0,0 };
    m_position = { 0,0,0 };

    m_model = model;

    m_bones.resize(model->mesh_count);

    std::stack<Bone*> parents;
    parents.push(nullptr);
    m_bones[0].parent = nullptr;                                             // root
    for(size_t i = 0; i<m_bones.size(); i++)
    {
        m_bones[i].index = i;
        m_bones[i].mesh_base = model->mesh_tree[i].mesh_base;
        m_bones[i].mesh_skin = model->mesh_tree[i].mesh_skin;
        if(m_bones[i].mesh_skin)
            m_hasSkin = true;
        m_bones[i].mesh_slot = nullptr;
        m_bones[i].body_part = model->mesh_tree[i].body_part;

        m_bones[i].offset = model->mesh_tree[i].offset;
        m_bones[i].qrotate = { 0,0,0,0 };
        m_bones[i].transform = glm::mat4(1.0f);
        m_bones[i].full_transform = glm::mat4(1.0f);

        if(i <= 0)
            continue;

        m_bones[i].parent = &m_bones[i - 1];
        if(model->mesh_tree[i].flag & 0x01)                                 // POP
        {
            if(!parents.empty())
            {
                m_bones[i].parent = parents.top();
                parents.pop();
            }
        }
        if(model->mesh_tree[i].flag & 0x02)                                 // PUSH
        {
            if(parents.size() + 1 < model->mesh_count)
            {
                parents.push( m_bones[i].parent );
            }
        }
    }
}

void Skeleton::itemFrame(util::Duration time)
{
    stepAnimation(time, nullptr);
    updateCurrentBoneFrame();
}

void Skeleton::updateCurrentBoneFrame()
{
    const animation::SkeletonKeyFrame& lastKeyFrame = m_model->animations[m_lerpLastAnimation].keyFrames[m_lerpLastFrame];
    const animation::SkeletonKeyFrame& currentKeyFrame = m_model->animations[m_currentAnimation].keyFrames[m_currentFrame];

    m_boundingBox.max = glm::mix(lastKeyFrame.boundingBox.max, currentKeyFrame.boundingBox.max, m_lerp);
    m_boundingBox.min = glm::mix(lastKeyFrame.boundingBox.min, currentKeyFrame.boundingBox.min, m_lerp);
    m_position = glm::mix(lastKeyFrame.position, currentKeyFrame.position, m_lerp);

    for(size_t k = 0; k < lastKeyFrame.boneKeyFrames.size(); k++)
    {
        m_bones[k].offset = glm::mix(lastKeyFrame.boneKeyFrames[k].offset, currentKeyFrame.boneKeyFrames[k].offset, m_lerp);
        m_bones[k].transform = glm::translate(glm::mat4(1.0f), m_bones[k].offset);
        if(k == 0)
        {
            m_bones[k].transform = glm::translate(m_bones[k].transform, m_position);
        }

        m_bones[k].qrotate = glm::slerp(lastKeyFrame.boneKeyFrames[k].qrotate, currentKeyFrame.boneKeyFrames[k].qrotate, m_lerp);
        m_bones[k].transform *= glm::mat4_cast(m_bones[k].qrotate);
    }

    /*
     * build absolute coordinate matrix system
     */
    m_bones[0].full_transform = m_bones[0].transform;
    for(size_t k = 1; k < lastKeyFrame.boneKeyFrames.size(); k++)
    {
        m_bones[k].full_transform = m_bones[k].parent->full_transform * m_bones[k].transform;
    }
}

void Skeleton::copyMeshBinding(const SkeletalModel *model, bool resetMeshSlot)
{
    size_t meshes_to_copy = std::min(m_bones.size(), model->mesh_count);

    for(size_t i = 0; i < meshes_to_copy; i++)
    {
        m_bones[i].mesh_base = model->mesh_tree[i].mesh_base;
        m_bones[i].mesh_skin = model->mesh_tree[i].mesh_skin;
        if(resetMeshSlot)
            m_bones[i].mesh_slot = nullptr;
    }
}

void Skeleton::setAnimation(int animation, int frame)
{
    // FIXME: is anim < 0 actually happening?
    animation = (animation < 0) ? (0) : (animation);

    AnimationFrame* anim = &m_model->animations[animation];

    if(frame < 0)
        frame = anim->keyFrames.size() - 1 - ((-frame - 1) % anim->keyFrames.size());
    else
        frame %= anim->keyFrames.size();

    m_currentAnimation = animation;
    m_currentFrame = frame;
    m_lastState = anim->state_id;
    m_nextState = anim->state_id;

    //    m_bf.animations.lerp = 0.0f;
    //    m_bf.animations.frame_time = 0.0f;
}

/**
* Advance animation frame
* @param time          time step for animation
* @param cmdEntity     optional entity for which doAnimCommand is called
* @return  ENTITY_ANIM_NONE if frame is unchanged (time<rate), ENTITY_ANIM_NEWFRAME or ENTITY_ANIM_NEWANIM
*/
AnimUpdate Skeleton::stepAnimation(util::Duration time, Entity* cmdEntity) {
    AnimUpdate stepResult = AnimUpdate::NewFrame;

    // --------
    // FIXME: hack for reverse framesteps (weaponanims):
    if(time.count() < 0)
    {
        m_frameTime -= time;
        if(m_frameTime + animation::GameLogicFrameTime / 2.0f < FrameTime)
        {
            m_lerp = m_frameTime / FrameTime;
            return AnimUpdate::None;
        }
        else
        {
            m_lerpLastAnimation = m_currentAnimation;
            m_lerpLastFrame = m_currentFrame;
            m_frameTime = util::Duration(0);
            m_lerp = 0.0f;
            if(m_currentFrame > 0)
            {
                m_currentFrame--;
                stepResult = AnimUpdate::NewFrame;
            }
            else
            {
                m_currentFrame = getCurrentAnimationFrame().keyFrames.size() - 1;
                stepResult = AnimUpdate::NewAnim;
            }
        }
        return stepResult;
    }
    // --------

    m_frameTime += time;
    if(m_frameTime + animation::GameLogicFrameTime / 2.0f < FrameTime)
    {
        m_lerp = m_frameTime / FrameTime; // re-sync
        return AnimUpdate::None;
    }

    m_lerpLastAnimation = m_currentAnimation;
    m_lerpLastFrame = m_currentFrame;
    m_frameTime = util::Duration(0);
    m_lerp = 0.0f;

    uint16_t frame_id = m_currentFrame + 1;

    // check anim flags:
    if(m_mode == AnimationMode::LoopLastFrame)
    {
        if(frame_id >= static_cast<int>(getCurrentAnimationFrame().keyFrames.size()))
        {
            m_currentFrame = getCurrentAnimationFrame().keyFrames.size() - 1;
            return AnimUpdate::NewFrame;    // period time has passed so it's a new frame, or should this be none?
        }
    }
    else if(m_mode == AnimationMode::Locked)
    {
        m_currentFrame = 0;
        return AnimUpdate::NewFrame;
    }
    else if(m_mode == AnimationMode::WeaponCompat)
    {
        if(frame_id >= getCurrentAnimationFrame().keyFrames.size())
        {
            return AnimUpdate::NewAnim;
        }
        else
        {
            m_currentFrame = frame_id;
            return AnimUpdate::NewFrame;
        }
    }

    // check state change:
    uint16_t anim_id = m_currentAnimation;
    if(m_nextState != m_lastState)
    {
        if(m_model->findStateChange(m_nextState, anim_id, frame_id))
        {
            m_lastState = m_model->animations[anim_id].state_id;
            m_nextState = m_lastState;
            stepResult = AnimUpdate::NewAnim;
        }
    }

    // check end of animation:
    if(frame_id >= m_model->animations[anim_id].keyFrames.size())
    {
        if(cmdEntity)
        {
            for(AnimCommand acmd : m_model->animations[anim_id].animCommands)  // end-of-anim cmdlist
            {
                cmdEntity->doAnimCommand(acmd);
            }
        }
        if(m_model->animations[anim_id].next_anim)
        {
            frame_id = m_model->animations[anim_id].next_frame;
            anim_id = m_model->animations[anim_id].next_anim->id;

            // some overlay anims may have invalid nextAnim/nextFrame values:
            if(anim_id < m_model->animations.size() && frame_id < m_model->animations[anim_id].keyFrames.size())
            {
                m_lastState = m_model->animations[anim_id].state_id;
                m_nextState = m_lastState;
            }
            else
            {
                // invalid values:
                anim_id = m_currentAnimation;
                frame_id = 0;
            }
        }
        else
        {
            frame_id = 0;
        }
        stepResult = AnimUpdate::NewAnim;
    }

    m_currentAnimation = anim_id;
    m_currentFrame = frame_id;

    if(cmdEntity)
    {
        for(AnimCommand acmd : m_model->animations[anim_id].keyFrames[frame_id].animCommands)  // frame cmdlist
        {
            cmdEntity->doAnimCommand(acmd);
        }
    }
    return stepResult;
}

const AnimationFrame &Skeleton::getCurrentAnimationFrame() const
{
    return m_model->animations[m_currentAnimation];
}

void Skeleton::updateTransform(const BtEntityData& bt, const glm::mat4& transform)
{
    for(size_t i = 0; i < m_bones.size(); i++)
    {
        glm::mat4 tr;
        bt.bt_body[i]->getWorldTransform().getOpenGLMatrix(glm::value_ptr(tr));
        m_bones[i].full_transform = glm::inverse(transform) * tr;
    }

    // that cycle is necessary only for skinning models;
    for(Bone& bone : m_bones)
    {
        if(bone.parent != nullptr)
        {
            bone.transform = glm::inverse(bone.parent->full_transform) * bone.full_transform;
        }
        else
        {
            bone.transform = bone.full_transform;
        }
    }
}

void Skeleton::updateBoundingBox()
{
    m_boundingBox = m_bones[0].mesh_base->boundingBox;
    if(m_bones.size() > 1)
    {
        for(const Bone& bone : m_bones)
        {
            m_boundingBox.adjust(glm::vec3(bone.full_transform[3]), bone.mesh_base->boundingBox.getOuterDiameter() * 0.5f);
        }
    }
}

} // namespace animation
} // namespace world
