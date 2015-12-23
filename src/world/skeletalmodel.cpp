#include "skeletalmodel.h"

#include "core/basemesh.h"

namespace world
{

void SkeletalModel::clear()
{
    meshes.clear();
    collision_map.clear();
    animations.clear();
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
bool SkeletalModel::findStateChange(LaraState stateid, animation::AnimationId& animid_inout, size_t& frameid_inout)
{
    const animation::StateChange* stc = animations[animid_inout].findStateChangeByID(stateid);
    if(!stc)
        return false;

    for(const animation::AnimationDispatch& dispatch : stc->dispatches)
    {
        if(   frameid_inout >= dispatch.start
           && frameid_inout <= dispatch.end)
        {
            animid_inout = dispatch.next.animation;
            frameid_inout = dispatch.next.frame;
            return true;
        }
    }

    return false;
}

} // namespace world
