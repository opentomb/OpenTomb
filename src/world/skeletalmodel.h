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
        std::shared_ptr<core::BaseMesh> mesh_skin; //!< base skinned mesh for ?R4+
        glm::vec3 offset; //!< model position offset
        uint16_t                    flag;                                           // 0x0001 = POP, 0x0002 = PUSH, 0x0003 = RESET
        uint32_t                    body_part;
        uint8_t                     replace_mesh;                                   // flag for shoot / guns animations (0x00, 0x01, 0x02, 0x03)
        bool                        replace_anim;
    };

    uint32_t                    id;
    bool                        has_transparency;

    core::BoundingBox boundingBox;
    glm::vec3                   centre;

    std::vector<animation::AnimationFrame> animations;

    uint16_t                    mesh_count;
    std::vector<MeshTreeTag>    mesh_tree;                                      // base mesh tree.

    std::vector<uint16_t>       collision_map;

    void clear();
    void updateTransparencyFlag();
    void interpolateFrames();
    void fillSkinnedMeshMap();
};

SkeletalModel::MeshTreeTag* SkeletonClone(SkeletalModel::MeshTreeTag* src, int tags_count);
void SkeletonCopyMeshes(SkeletalModel::MeshTreeTag* dst, SkeletalModel::MeshTreeTag* src, int tags_count);
void SkeletonCopyMeshes2(SkeletalModel::MeshTreeTag* dst, SkeletalModel::MeshTreeTag* src, int tags_count);

} // namespace world
