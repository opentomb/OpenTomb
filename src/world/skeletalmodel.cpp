#include "skeletalmodel.h"

#include "core/basemesh.h"
#include "util/vmath.h"

namespace world
{

void SkeletalModel::clear()
{
    mesh_tree.clear();
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
    animation::AnimationFrame* anim = animations.data();

    for(uint16_t i = 0; i < animations.size(); i++, anim++)
    {
        int length = anim->keyFrames.size();
        int rate = anim->original_frame_rate;
        if(length > 1 && rate > 1)
        {
            int destIdx = ((length - 1) / rate) * rate;

            while(destIdx >= 0)
            {
                int srcIdx = destIdx / rate;
                for(int j = rate - 1; j>0; j--)
                {
                    if(destIdx + j >= length) continue;

                    glm::float_t lerp = static_cast<glm::float_t>(j) / static_cast<glm::float_t>(rate);

                    anim->keyFrames[destIdx + j].center = glm::mix(anim->keyFrames[srcIdx].center, anim->keyFrames[srcIdx + 1].center, lerp);
                    anim->keyFrames[destIdx + j].position = glm::mix(anim->keyFrames[srcIdx].position, anim->keyFrames[srcIdx + 1].position, lerp);
                    anim->keyFrames[destIdx + j].boundingBox.max = glm::mix(anim->keyFrames[srcIdx].boundingBox.max, anim->keyFrames[srcIdx + 1].boundingBox.max, lerp);
                    anim->keyFrames[destIdx + j].boundingBox.min = glm::mix(anim->keyFrames[srcIdx].boundingBox.min, anim->keyFrames[srcIdx + 1].boundingBox.min, lerp);

                    for(uint16_t k = 0; k < mesh_count; k++)
                    {
                        anim->keyFrames[destIdx + j].boneKeyFrames[k].offset = glm::mix(anim->keyFrames[srcIdx].boneKeyFrames[k].offset, anim->keyFrames[srcIdx + 1].boneKeyFrames[k].offset, lerp);
                        anim->keyFrames[destIdx + j].boneKeyFrames[k].qrotate = glm::slerp(anim->keyFrames[srcIdx].boneKeyFrames[k].qrotate, anim->keyFrames[srcIdx + 1].boneKeyFrames[k].qrotate, lerp);
                    }
                }
                if(destIdx > 0)
                {
                    anim->keyFrames[destIdx] = anim->keyFrames[srcIdx];
                }

                destIdx -= rate;
            }
        }
    }
}

void SkeletalModel::updateTransparencyFlag()
{
    has_transparency = false;
    for(uint16_t i = 0; i < mesh_count; i++)
    {
        if(!mesh_tree[i].mesh_base->m_transparencyPolygons.empty())
        {
            has_transparency = true;
            return;
        }
    }
}

void SkeletalModel::fillSkinnedMeshMap()
{
    core::Vertex* v, *rv;
    MeshTreeTag* tree_tag, *prev_tree_tag;

    tree_tag = mesh_tree.data();
    for(uint16_t i = 0; i < mesh_count; i++, tree_tag++)
    {
        if(!tree_tag->mesh_skin)
        {
            return;
        }

        tree_tag->mesh_skin->m_matrixIndices.resize(tree_tag->mesh_skin->m_vertices.size());
        core::BaseMesh::MatrixIndex* ch = tree_tag->mesh_skin->m_matrixIndices.data();
        v = tree_tag->mesh_skin->m_vertices.data();
        for(size_t k = 0; k < tree_tag->mesh_skin->m_vertices.size(); k++, v++, ch++)
        {
            rv = tree_tag->mesh_base->findVertex(v->position);
            if(rv != nullptr)
            {
                ch->i = 0;
                ch->j = 0;
                v->position = rv->position;
                v->normal = rv->normal;
            }
            else
            {
                ch->i = 0;
                ch->j = 1;
                auto tv = v->position + tree_tag->offset;
                prev_tree_tag = mesh_tree.data();
                for(uint16_t l = 0; l < mesh_count; l++, prev_tree_tag++)
                {
                    rv = prev_tree_tag->mesh_base->findVertex(tv);
                    if(rv != nullptr)
                    {
                        ch->i = 1;
                        ch->j = 1;
                        v->position = rv->position - tree_tag->offset;
                        v->normal = rv->normal;
                        break;
                    }
                }
            }
        }
    }
}

SkeletalModel::MeshTreeTag *SkeletonClone(SkeletalModel::MeshTreeTag *src, int tags_count)
{
    SkeletalModel::MeshTreeTag* ret = new SkeletalModel::MeshTreeTag[tags_count];

    for(int i = 0; i < tags_count; i++)
    {
        ret[i].mesh_base = src[i].mesh_base;
        ret[i].mesh_skin = src[i].mesh_skin;
        ret[i].flag = src[i].flag;
        ret[i].offset = src[i].offset;
        ret[i].replace_anim = src[i].replace_anim;
        ret[i].replace_mesh = src[i].replace_mesh;
    }
    return ret;
}

void SkeletonCopyMeshes(SkeletalModel::MeshTreeTag *dst, SkeletalModel::MeshTreeTag *src, int tags_count)
{
    for(int i = 0; i < tags_count; i++)
    {
        dst[i].mesh_base = src[i].mesh_base;
    }
}

void SkeletonCopyMeshes2(SkeletalModel::MeshTreeTag *dst, SkeletalModel::MeshTreeTag *src, int tags_count)
{
    for(int i = 0; i < tags_count; i++)
    {
        dst[i].mesh_skin = src[i].mesh_base;
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
        if(stc.id == stateid)
        {
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
    }
    return false;
}

} // namespace world
