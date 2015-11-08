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
    struct MeshReference
    {
        std::shared_ptr<core::BaseMesh> mesh_base; //!< pointer to the first mesh in array
        std::shared_ptr<core::BaseMesh> mesh_skin = nullptr; //!< base skinned mesh for TR4+
        glm::vec3 offset = {0,0,0}; //!< model position offset
        uint16_t                    flag = 0;                                           // 0x0001 = POP, 0x0002 = PUSH, 0x0003 = RESET
        uint32_t                    body_part = 0;
        uint8_t                     replace_mesh = 0;                                   // flag for shoot / guns animations (0x00, 0x01, 0x02, 0x03)
        bool                        replace_anim = false;
    };

    uint32_t                    id;
    bool                        has_transparency = false;

    core::BoundingBox boundingBox;

    std::vector<animation::Animation> animations;

    std::vector<MeshReference> meshes;

    std::vector<size_t> collision_map;

    bool no_fix_all = false;
    uint32_t no_fix_body_parts = 0;

    std::vector<std::shared_ptr<btTypedConstraint>> bt_joints;              // Ragdoll joints

    void clear();
    void updateTransparencyFlag();
    void fillSkinnedMeshMap();
    bool findStateChange(LaraState stateid, uint16_t& animid_out, uint16_t& frameid_inout);

    void setMeshes(const std::vector<SkeletalModel::MeshReference>& src, size_t meshCount);
    void setSkinnedMeshes(const std::vector<SkeletalModel::MeshReference>& src, size_t meshCount);
};

} // namespace world
