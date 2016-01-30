#include "animation.h"

namespace world
{
namespace animation
{
SkeletonPose Animation::getInterpolatedPose(size_t frame) const
{
    BOOST_ASSERT(frame < m_duration);
    const size_t frameIndex = frame / m_stretchFactor;
    BOOST_ASSERT(frameIndex < m_poses.size());
    const SkeletonPose& first = m_poses[frameIndex];
    const SkeletonPose& second = frameIndex + 1 >= m_poses.size()
        ? m_poses.back()
        : m_poses[frameIndex + 1];

    BOOST_ASSERT(first.bonePoses.size() == second.bonePoses.size());

    const size_t subOffset = frame - frameIndex*m_stretchFactor; // offset between keyframes
    if(subOffset == 0)
        return first; // no need to interpolate

    const glm::float_t lerp = static_cast<glm::float_t>(subOffset) / static_cast<glm::float_t>(m_stretchFactor);

    SkeletonPose result;
    result.position = glm::mix(first.position, second.position, lerp);
    result.boundingBox.max = glm::mix(first.boundingBox.max, second.boundingBox.max, lerp);
    result.boundingBox.min = glm::mix(first.boundingBox.min, second.boundingBox.min, lerp);

    result.bonePoses.resize(first.bonePoses.size());

    for(size_t k = 0; k < first.bonePoses.size(); k++)
    {
        result.bonePoses[k].position = glm::mix(first.bonePoses[k].position, second.bonePoses[k].position, lerp);
        result.bonePoses[k].rotation = glm::slerp(first.bonePoses[k].rotation, second.bonePoses[k].rotation, lerp);
    }

    return result;
}
} // namespace animation
} // namespace world
