#include "skeletalmodel.h"

#include "core/basemesh.h"
#include "util/vmath.h"

namespace world
{

void SkeletalModel::clear()
{
    meshes.clear();
    collision_map.clear();
    animations.clear();
}

/**
 * Expand compressed frames
 * For animations with rate>1, interpolate frames over
 * the real frame range.
 */
void SkeletalModel::interpolateFrames()
{

    for(animation::AnimationFrame& anim : animations)
    {
        if(anim.keyFrames.size() <= 1 || anim.original_frame_rate < 1)
            continue;

        int destIdx = ((anim.keyFrames.size() - 1) / anim.original_frame_rate) * anim.original_frame_rate;

        while(destIdx >= 0)
        {
            int srcIdx = destIdx / anim.original_frame_rate;
            for(int j = anim.original_frame_rate - 1; j>0; j--)
            {
                if(destIdx + j >= anim.keyFrames.size())
                    continue;

                glm::float_t lerp = static_cast<glm::float_t>(j) / static_cast<glm::float_t>(anim.original_frame_rate);

                anim.keyFrames[destIdx + j].position = glm::mix(anim.keyFrames[srcIdx].position, anim.keyFrames[srcIdx + 1].position, lerp);
                anim.keyFrames[destIdx + j].boundingBox.max = glm::mix(anim.keyFrames[srcIdx].boundingBox.max, anim.keyFrames[srcIdx + 1].boundingBox.max, lerp);
                anim.keyFrames[destIdx + j].boundingBox.min = glm::mix(anim.keyFrames[srcIdx].boundingBox.min, anim.keyFrames[srcIdx + 1].boundingBox.min, lerp);

                for(size_t k = 0; k < meshes.size(); k++)
                {
                    anim.keyFrames[destIdx + j].boneKeyFrames[k].offset = glm::mix(anim.keyFrames[srcIdx].boneKeyFrames[k].offset, anim.keyFrames[srcIdx + 1].boneKeyFrames[k].offset, lerp);
                    anim.keyFrames[destIdx + j].boneKeyFrames[k].qrotate = glm::slerp(anim.keyFrames[srcIdx].boneKeyFrames[k].qrotate, anim.keyFrames[srcIdx + 1].boneKeyFrames[k].qrotate, lerp);
                }
            }
            if(destIdx > 0)
            {
                anim.keyFrames[destIdx] = anim.keyFrames[srcIdx];
            }

            destIdx -= anim.original_frame_rate;
        }
    }
}

void SkeletalModel::updateTransparencyFlag()
{
    has_transparency = std::any_of(meshes.begin(), meshes.end(),
                                   [](const MeshReference& mesh) { return !mesh.mesh_base->m_transparencyPolygons.empty(); }
    );
}

void SkeletalModel::fillSkinnedMeshMap()
{
    for(MeshReference& mesh : meshes)
    {
        if(!mesh.mesh_skin)
        {
            continue;
        }

        mesh.mesh_skin->m_matrixIndices.clear();
        for(core::Vertex& v : mesh.mesh_skin->m_vertices)
        {
            if(const core::Vertex* rv = mesh.mesh_base->findVertex(v.position))
            {
                mesh.mesh_skin->m_matrixIndices.emplace_back(0,0);
                v.position = rv->position;
                v.normal = rv->normal;
                continue;
            }

            mesh.mesh_skin->m_matrixIndices.emplace_back(0,1);
            glm::vec3 tv = v.position + mesh.offset;
            for(const MeshReference& prevMesh : meshes)
            {
                const core::Vertex* rv = prevMesh.mesh_base->findVertex(tv);
                if(rv == nullptr)
                    continue;

                mesh.mesh_skin->m_matrixIndices.emplace_back(1,1);
                v.position = rv->position - mesh.offset;
                v.normal = rv->normal;
                break;
            }
        }
    }
}

void SkeletalModel::setMeshes(const std::vector<SkeletalModel::MeshReference>& src, size_t meshCount)
{
    BOOST_ASSERT( meshCount <= meshes.size() && meshCount <= src.size() );
    for(size_t i = 0; i < meshCount; i++)
    {
        meshes[i].mesh_base = src[i].mesh_base;
    }
}

void SkeletalModel::setSkinnedMeshes(const std::vector<SkeletalModel::MeshReference>& src, size_t meshCount)
{
    BOOST_ASSERT( meshCount <= meshes.size() && meshCount <= src.size() );
    for(size_t i = 0; i < meshCount; i++)
    {
        meshes[i].mesh_skin = src[i].mesh_base;
    }
}

/**
* Find a possible state change to \c stateid
* \param[in]      stateid  the desired target state
* \param[in,out]  animid   reference to anim id, receives found anim
* \param[in,out]  frameid  reference to frame id, receives found frame
* \return  true if state is found, false otherwise
*/
bool SkeletalModel::findStateChange(LaraState stateid, uint16_t& animid_inout, uint16_t& frameid_inout)
{
    for(const animation::StateChange& stc : animations[animid_inout].stateChanges)
    {
        if(stc.id != stateid)
            continue;

        for(const animation::AnimDispatch& dispatch : stc.anim_dispatch)
        {
            if(   frameid_inout >= dispatch.frame_low
               && frameid_inout <= dispatch.frame_high)
            {
                animid_inout = dispatch.next_anim;
                frameid_inout = dispatch.next_frame;
                return true;
            }
        }
    }
    return false;
}

} // namespace world
