#pragma once

#include "animation/animation.h"

#include <memory>

class btTypedConstraint;

namespace engine
{
class Engine;
}

namespace loader
{
struct AnimatedModel;
enum class Engine;
}

namespace world
{
namespace core
{
class BaseMesh;
}

class World;

using ModelId = uint32_t;

/*
 * skeletal model with animations data.
 * Animated skeletal model. Taken from openraider.
 * model -> animation -> frame -> bone
 * thanks to Terry 'Mongoose' Hendrix II
 */
class SkeletalModel
{
public:
    /*
    * mesh tree base element structure
    */
    struct MeshReference
    {
        enum StackOperation
        {
            UsePredecessor,
            Pop,
            Push,
            Top
        };

        std::shared_ptr<core::BaseMesh> mesh_base; //!< pointer to the first mesh in array
        std::shared_ptr<core::BaseMesh> mesh_skin = nullptr; //!< base skinned mesh for TR4+
        glm::vec3 position = { 0,0,0 }; //!< model position offset
        StackOperation stackOperation = UsePredecessor;
        uint32_t                    body_part = 0;
        uint8_t                     replace_mesh = 0;                                   // flag for shoot / guns animations (0x00, 0x01, 0x02, 0x03)
        bool                        replace_anim = false;
    };

private:
    ModelId m_id;

    bool m_hasTransparency = false;

    std::vector<animation::Animation> m_animations;

    std::vector<MeshReference> m_meshReferences;

    std::vector<size_t> m_collisionMap;

public:
    bool m_noFixAll = false;
    uint32_t m_noFixBodyParts = 0;

    std::vector<std::shared_ptr<btTypedConstraint>> m_btJoints;              // Ragdoll joints

public:
    explicit SkeletalModel(ModelId id)
        : m_id(id)
    {
    }

    ModelId getId() const noexcept
    {
        return m_id;
    }

    bool hasTransparency() const noexcept
    {
        return m_hasTransparency;
    }

    const animation::Animation& getAnimation(size_t idx) const
    {
        BOOST_ASSERT(idx < m_animations.size());
        return m_animations[idx];
    }

    size_t getAnimationCount() const
    {
        return m_animations.size();
    }

    bool isStaticAnimation() const
    {
        BOOST_ASSERT(!m_animations.empty());

        if(m_animations.size() != 1)
            return false;

        if(m_animations[0].getFrameDuration() != 1)
            return false;

        return true;
    }

    bool hasAnimations() const
    {
        return !m_animations.empty();
    }

    size_t getMeshReferenceCount() const
    {
        return m_meshReferences.size();
    }

    const MeshReference& getMeshReference(size_t idx) const
    {
        BOOST_ASSERT(idx < m_meshReferences.size());
        return m_meshReferences[idx];
    }

    bool hasMeshReferences() const
    {
        return !m_meshReferences.empty();
    }

    void clear();
    void updateTransparencyFlag();
    void fillSkinnedMeshMap();
    bool findStateChange(LaraState stateid, animation::AnimationId& animid_out, size_t& frameid_inout);

    void setMeshes(const std::vector<SkeletalModel::MeshReference>& src, size_t meshCount);
    void setSkinnedMeshes(const std::vector<SkeletalModel::MeshReference>& src, size_t meshCount);

    void generateAnimCommands(const World& world);
    void loadStateChanges(const World& world, const loader::Level& level, const loader::AnimatedModel& animatedModel);
    void setStaticAnimation();
    void loadAnimations(const loader::Level& level, size_t moveable);

    void swapAnimationsWith(SkeletalModel& rhs)
    {
        std::swap(m_animations, rhs.m_animations);
    }

    void patchLaraSkin(World& world, loader::Engine engineVersion);

    void addMeshReference(const MeshReference& mr)
    {
        m_meshReferences.emplace_back(mr);
        m_collisionMap.emplace_back(m_collisionMap.size());
    }

    void shrinkCollisionMap(size_t size)
    {
        if(size < m_collisionMap.size())
            m_collisionMap.resize(size);
    }

    void setCollisionMap(size_t idx, size_t val)
    {
        if(idx < m_collisionMap.size())
            m_collisionMap[idx] = val;
    }

    size_t getCollisionMapSize() const
    {
        return m_collisionMap.size();
    }

    size_t getCollisionMap(size_t idx) const
    {
        BOOST_ASSERT(idx < m_collisionMap.size());
        return m_collisionMap[idx];
    }

    static void lua_SetModelMeshReplaceFlag(engine::Engine& engine, ModelId id, size_t bone, int flag);
    static void lua_SetModelAnimReplaceFlag(engine::Engine& engine, ModelId id, size_t bone, bool flag);
    static void lua_CopyMeshFromModelToModel(engine::Engine& engine, ModelId id1, ModelId id2, size_t bone1, size_t bone2);
    static void lua_SetModelBodyPartFlag(engine::Engine& engine, ModelId id, int bone_id, int body_part_flag);
private:
    static size_t getAnimationCountForMoveable(const loader::Level& tr, size_t modelIndex);
};
} // namespace world
