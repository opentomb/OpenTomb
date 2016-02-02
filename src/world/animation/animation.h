#pragma once

#include "animcommands.h"
#include "common.h"
#include "pose.h"
#include "transition.h"
#include "world/statecontroller.h"

#include <boost/assert.hpp>

namespace world
{
namespace animation
{

/*
 * Animated version of vertex.
 *
 * Does not contain texture coordinate, because that is in a different VBO.
 */
struct AnimatedVertex
{
    glm::vec3 position;
    glm::vec4 color;
    glm::vec3 normal;
};

/**
 * A sequence of poses.
 */
struct Animation
{
    AnimationId id;
    int32_t speed_x; // Forward-backward speed
    int32_t accel_x; // Forward-backward accel
    int32_t speed_y; // Left-right speed
    int32_t accel_y; // Left-right accel
    size_t animationCommandIndex;
    size_t animationCommandCount;
    LaraState state_id;
    int firstFrame = 0;

    std::map<LaraState, Transition> m_transitions;

    Animation* next_anim = nullptr; // Next default animation
    size_t nextFrame;                 // Next default frame

    std::vector<AnimCommand> finalAnimCommands; // cmds for end-of-anim

    const Transition* findTransitionById(LaraState id) const noexcept
    {
        auto it = m_transitions.find(id);
        if(it == m_transitions.end())
            return nullptr;

        return &it->second;
    }

    Transition* findTransitionById(LaraState id) noexcept
    {
        auto it = m_transitions.find(id);
        if(it == m_transitions.end())
            return nullptr;

        return &it->second;
    }

    const BonePose& getInitialBoneKeyFrame() const
    {
        return m_poses.front().bonePoses.front();
    }

    SkeletonPose getInterpolatedPose(size_t frame) const;

    SkeletonPose& rawPose(size_t idx)
    {
        if(idx >= m_poses.size())
            throw std::out_of_range("Keyframe index out of bounds");
        return m_poses[idx];
    }

    size_t getFrameDuration() const
    {
        return m_duration;
    }

    void setDuration(size_t frames, size_t keyFrames, uint8_t stretchFactor)
    {
        BOOST_ASSERT(stretchFactor > 0);
        BOOST_ASSERT(frames > 0);
        m_poses.resize(keyFrames);
        m_duration = frames;
        m_stretchFactor = stretchFactor;
    }

    size_t getKeyFrameCount() const noexcept
    {
        return m_poses.size();
    }

    uint8_t getStretchFactor() const
    {
        return m_stretchFactor;
    }

    std::vector<AnimCommand>& animCommands(int frame)
    {
        return m_animCommands[frame];
    }

    const std::vector<AnimCommand>& getAnimCommands(int frame) const
    {
        static const std::vector<AnimCommand> empty{};
        auto it = m_animCommands.find(frame);
        if(it == m_animCommands.end())
            return empty;
        return it->second;
    }

private:
    std::vector<SkeletonPose> m_poses;
    uint8_t m_stretchFactor = 1; //!< Time scale (>1 means slowdown)
    std::map<int, std::vector<AnimCommand>> m_animCommands; //!< Maps from real frame index to commands
    size_t m_duration = 1; //!< Real frame duration
};

} // namespace animation
} // namespace world
