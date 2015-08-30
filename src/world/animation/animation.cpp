#include "animation.h"

#include "world/core/mesh.h"
#include "world/skeletalmodel.h"

namespace world
{
namespace animation
{

void SSBoneFrame::fromModel(SkeletalModel* model)
{
    hasSkin = false;
    boundingBox.min.setZero();
    boundingBox.max.setZero();
    center.setZero();
    position.setZero();
    animations = SSAnimation();

    animations.next = nullptr;
    animations.onFrame = nullptr;
    animations.model = model;
    bone_tags.resize(model->mesh_count);

    int stack = 0;
    std::vector<SSBoneTag*> parents(bone_tags.size());
    parents[0] = NULL;
    bone_tags[0].parent = NULL;                                             // root
    for(uint16_t i=0;i<bone_tags.size();i++)
    {
        bone_tags[i].index = i;
        bone_tags[i].mesh_base = model->mesh_tree[i].mesh_base;
        bone_tags[i].mesh_skin = model->mesh_tree[i].mesh_skin;
        if(bone_tags[i].mesh_skin)
            hasSkin = true;
        bone_tags[i].mesh_slot = nullptr;
        bone_tags[i].body_part = model->mesh_tree[i].body_part;

        bone_tags[i].offset = model->mesh_tree[i].offset;
        bone_tags[i].qrotate = { 0,0,0,0 };
        bone_tags[i].transform.setIdentity();
        bone_tags[i].full_transform.setIdentity();

        if(i > 0)
        {
            bone_tags[i].parent = &bone_tags[i - 1];
            if(model->mesh_tree[i].flag & 0x01)                                 // POP
            {
                if(stack > 0)
                {
                    bone_tags[i].parent = parents[stack];
                    stack--;
                }
            }
            if(model->mesh_tree[i].flag & 0x02)                                 // PUSH
            {
                if(stack + 1 < static_cast<int16_t>(model->mesh_count))
                {
                    stack++;
                    parents[stack] = bone_tags[i].parent;
                }
            }
        }
    }
}

void BoneFrame_Copy(BoneFrame *dst, BoneFrame *src)
{
    dst->bone_tags.resize(src->bone_tags.size());
    dst->position = src->position;
    dst->center = src->center;
    dst->boundingBox = src->boundingBox;

    dst->command = src->command;
    dst->move = src->move;

    for(uint16_t i = 0; i < dst->bone_tags.size(); i++)
    {
        dst->bone_tags[i].qrotate = src->bone_tags[i].qrotate;
        dst->bone_tags[i].offset = src->bone_tags[i].offset;
    }
}

} // namespace animation
} // namespace world
