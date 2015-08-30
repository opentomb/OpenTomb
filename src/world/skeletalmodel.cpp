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

void SkeletalModel::interpolateFrames()
{
    animation::AnimationFrame* anim = animations.data();

    for(uint16_t i = 0; i < animations.size(); i++, anim++)
    {
        if(anim->frames.size() > 1 && anim->original_frame_rate > 1)                      // we can't interpolate one frame or rate < 2!
        {
            std::vector<animation::BoneFrame> new_bone_frames(anim->original_frame_rate * (anim->frames.size() - 1) + 1);
            /*
             * the first frame does not changes
             */
            animation::BoneFrame* bf = new_bone_frames.data();
            bf->bone_tags.resize(mesh_count);
            bf->position.setZero();
            bf->move.setZero();
            bf->command = 0x00;
            bf->centre = anim->frames[0].centre;
            bf->position = anim->frames[0].position;
            bf->boundingBox = anim->frames[0].boundingBox;
            for(uint16_t k = 0; k < mesh_count; k++)
            {
                bf->bone_tags[k].offset = anim->frames[0].bone_tags[k].offset;
                bf->bone_tags[k].qrotate = anim->frames[0].bone_tags[k].qrotate;
            }
            bf++;

            for(uint16_t j = 1; j < anim->frames.size(); j++)
            {
                for(uint16_t l = 1; l <= anim->original_frame_rate; l++)
                {
                    bf->position.setZero();
                    bf->move.setZero();
                    bf->command = 0x00;
                    btScalar lerp = static_cast<btScalar>(l) / anim->original_frame_rate;
                    btScalar t = 1.0f - lerp;

                    bf->bone_tags.resize(mesh_count);

                    bf->centre = t * anim->frames[j - 1].centre + lerp * anim->frames[j].centre;

                    bf->position = t * anim->frames[j - 1].position + lerp * anim->frames[j].position;

                    bf->boundingBox.max = t * anim->frames[j - 1].boundingBox.max + lerp * anim->frames[j].boundingBox.max;
                    bf->boundingBox.min = t * anim->frames[j - 1].boundingBox.min + lerp * anim->frames[j].boundingBox.min;

                    for(uint16_t k = 0; k < mesh_count; k++)
                    {
                        bf->bone_tags[k].offset = anim->frames[j - 1].bone_tags[k].offset.lerp(anim->frames[j].bone_tags[k].offset, lerp);
                        bf->bone_tags[k].qrotate = util::Quat_Slerp(anim->frames[j - 1].bone_tags[k].qrotate, anim->frames[j].bone_tags[k].qrotate, lerp);
                    }
                    bf++;
                }
            }

            /*
             * swap old and new animation bone brames
             * free old bone frames;
             */
            anim->frames = std::move(new_bone_frames);
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
    MeshTreeTag* tree_tag = mesh_tree.data();
    for(uint16_t i = 0; i < mesh_count; i++, tree_tag++)
    {
        if(!tree_tag->mesh_skin)
        {
            return;
        }

        tree_tag->mesh_skin->m_matrixIndices.resize(tree_tag->mesh_skin->m_vertices.size());
        core::BaseMesh::MatrixIndex* ch = tree_tag->mesh_skin->m_matrixIndices.data();
        core::Vertex* v = tree_tag->mesh_skin->m_vertices.data();
        for(size_t k = 0; k < tree_tag->mesh_skin->m_vertices.size(); k++, v++, ch++)
        {
            core::Vertex* rv = tree_tag->mesh_base->findVertex(v->position);
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
                MeshTreeTag* prev_tree_tag = mesh_tree.data();
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


} // namespace world
