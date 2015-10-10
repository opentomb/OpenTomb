#pragma once

#include "animation/animation.h"
#include "core/boundingbox.h"

namespace world
{

namespace core
{
struct BaseMesh;
}

/*
 * skeletal model with animations data.
 * Animated skeletal model. Taken from openraider.
 * model -> animation -> frame -> bone
 * thanks to Terry 'Mongoose' Hendrix II
 */
struct SkeletalModel
{
    /*
    * mesh tree base element structure
    */
    struct MeshTreeTag
    {
        std::shared_ptr<core::BaseMesh> mesh_base; //!< pointer to the first mesh in array
        std::shared_ptr<core::BaseMesh> mesh_skin = nullptr; //!< base skinned mesh for ?R4+
        glm::vec3 offset = {0,0,0}; //!< model position offset
        uint16_t                    flag = 0;                                           // 0x0001 = POP, 0x0002 = PUSH, 0x0003 = RESET
        uint32_t                    body_part = 0;
        uint8_t                     replace_mesh = 0;                                   // flag for shoot / guns animations (0x00, 0x01, 0x02, 0x03)
        bool                        replace_anim = false;
    };

    uint32_t                    id;
    bool                        has_transparency;

    core::BoundingBox boundingBox;
    glm::vec3                   center;

    std::vector<animation::AnimationFrame> animations;

    size_t                      mesh_count = 0;
    std::vector<MeshTreeTag>    mesh_tree;                                      // base mesh tree.

    std::vector<uint16_t>       collision_map;

    void clear();
    void updateTransparencyFlag();
    void interpolateFrames();
    void fillSkinnedMeshMap();
    bool findStateChange(LaraState stateid, uint16_t& animid_out, uint16_t& frameid_inout);
};

SkeletalModel::MeshTreeTag* SkeletonClone(SkeletalModel::MeshTreeTag* src, int tags_count);
void SkeletonCopyMeshes(SkeletalModel::MeshTreeTag* dst, SkeletalModel::MeshTreeTag* src, int tags_count);
void SkeletonCopyMeshes2(SkeletalModel::MeshTreeTag* dst, SkeletalModel::MeshTreeTag* src, int tags_count);

} // namespace world
