#include "pose.h"

#include "loader/level.h"

namespace world
{
namespace animation
{
void SkeletonPose::load(const loader::Level& level, size_t poseDataOffset)
{
    if(poseDataOffset + 9 < level.m_poseData.size())
    {
        const int16_t* frame = &level.m_poseData[poseDataOffset];

        boundingBox.min[0] = frame[0];   // x_min
        boundingBox.min[1] = frame[4];   // y_min
        boundingBox.min[2] = -frame[3];  // z_min

        boundingBox.max[0] = frame[1];   // x_max
        boundingBox.max[1] = frame[5];   // y_max
        boundingBox.max[2] = -frame[2];  // z_max

        position[0] = frame[6];
        position[1] = frame[8];
        position[2] = -frame[7];
    }
    else
    {
        BOOST_LOG_TRIVIAL(warning) << "Reading animation data beyond end of frame data: offset = " << poseDataOffset << ", size = " << level.m_poseData.size();
        boundingBox.min = { 0,0,0 };
        boundingBox.max = { 0,0,0 };
        position = { 0,0,0 };
    }
}
}
}
