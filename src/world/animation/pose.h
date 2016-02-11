#pragma once

#include "world/core/boundingbox.h"

#include <glm/gtc/quaternion.hpp>

#include <vector>

namespace loader
{
class Level;
}

namespace world
{
namespace animation
{
/**
* Defines position and rotation in the parent's coordinate system
*
* @remark A parent is either another BonePose, or a SkeletonPose.
*/
struct BonePose
{
    glm::vec3 position;
    glm::quat rotation;
};

/**
* Defines a full pose of a @c Skeleton.
*/
struct SkeletonPose
{
    std::vector<BonePose> bonePoses;
    glm::vec3 position = { 0,0,0 };
    core::BoundingBox boundingBox;

    void load(const loader::Level& level, size_t poseDataOffset);
};
}
}
