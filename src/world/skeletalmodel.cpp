#include "skeletalmodel.h"

#include "core/basemesh.h"
#include "engine/engine.h"
#include "loader/level.h"
#include "resource.h"
#include "strings.h"
#include "world.h"

namespace world
{
void SkeletalModel::clear()
{
    m_meshReferences.clear();
    m_collisionMap.clear();
    m_animations.clear();
}

void SkeletalModel::updateTransparencyFlag()
{
    m_hasTransparency = std::any_of(m_meshReferences.begin(), m_meshReferences.end(),
                                   [](const MeshReference& mesh) { return !mesh.mesh_base->m_transparencyPolygons.empty(); }
    );
}

void SkeletalModel::fillSkinnedMeshMap()
{
    for(MeshReference& mesh : m_meshReferences)
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
                mesh.mesh_skin->m_matrixIndices.emplace_back(0, 0);
                v.position = rv->position;
                v.normal = rv->normal;
                continue;
            }

            mesh.mesh_skin->m_matrixIndices.emplace_back(0, 1);
            glm::vec3 tv = v.position + mesh.position;
            for(const MeshReference& prevMesh : m_meshReferences)
            {
                const core::Vertex* rv = prevMesh.mesh_base->findVertex(tv);
                if(rv == nullptr)
                    continue;

                mesh.mesh_skin->m_matrixIndices.emplace_back(1, 1);
                v.position = rv->position - mesh.position;
                v.normal = rv->normal;
                break;
            }
        }
    }
}

void SkeletalModel::setMeshes(const std::vector<SkeletalModel::MeshReference>& src, size_t meshCount)
{
    BOOST_ASSERT(meshCount <= m_meshReferences.size() && meshCount <= src.size());
    for(size_t i = 0; i < meshCount; i++)
    {
        m_meshReferences[i].mesh_base = src[i].mesh_base;
    }
}

void SkeletalModel::setSkinnedMeshes(const std::vector<SkeletalModel::MeshReference>& src, size_t meshCount)
{
    BOOST_ASSERT(meshCount <= m_meshReferences.size() && meshCount <= src.size());
    for(size_t i = 0; i < meshCount; i++)
    {
        m_meshReferences[i].mesh_skin = src[i].mesh_base;
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
    const animation::Transition* transition = m_animations[animid_inout].findTransitionById(stateid);
    if(!transition)
        return false;

    for(const animation::TransitionCase& transitionCase : transition->cases)
    {
        if(frameid_inout >= transitionCase.firstFrame
           && frameid_inout <= transitionCase.lastFrame)
        {
            animid_inout = transitionCase.target.animation;
            frameid_inout = transitionCase.target.frame;
            return true;
        }
    }

    return false;
}

void SkeletalModel::generateAnimCommands(const World& world)
{
    
    if(world.m_animCommands.empty())
    {
        return;
    }
    //Sys_DebugLog("anim_transform.txt", "MODEL[%d]", model.id);
    for(animation::Animation& anim : m_animations)
    {
        // Parse AnimCommands
        // Max. amount of AnimCommands is 255, larger numbers are considered as 0.
        // See http://evpopov.com/dl/TR4format.html#Animations for details.
        if(anim.animationCommandCount > 255)
        {
            continue;                                                           // If no anim commands or current anim has more than 255 (according to TRosettaStone).
        }

        if(anim.animationCommandCount == 0)
            continue;

        BOOST_ASSERT(anim.animationCommandIndex < world.m_animCommands.size());
        const int16_t *pointer = &world.m_animCommands[anim.animationCommandIndex];

        for(size_t i = 0; i < anim.animationCommandCount; i++)
        {
            const auto command = static_cast<animation::AnimCommandOpcode>(*pointer);
            ++pointer;
            switch(command)
            {
                /*
                * End-of-anim commands:
                */
                case animation::AnimCommandOpcode::SetPosition:
                    anim.finalAnimCommands.push_back({ command, pointer[0], pointer[1], pointer[2] });
                    // ConsoleInfo::instance().printf("ACmd MOVE: anim = %d, x = %d, y = %d, z = %d", static_cast<int>(anim), pointer[0], pointer[1], pointer[2]);
                    pointer += 3;
                    break;

                case animation::AnimCommandOpcode::SetVelocity:
                    anim.finalAnimCommands.push_back({ command, pointer[0], pointer[1], 0 });
                    // ConsoleInfo::instance().printf("ACmd JUMP: anim = %d, vVert = %d, vHoriz = %d", static_cast<int>(anim), pointer[0], pointer[1]);
                    pointer += 2;
                    break;

                case animation::AnimCommandOpcode::EmptyHands:
                    anim.finalAnimCommands.push_back({ command, 0, 0, 0 });
                    // ConsoleInfo::instance().printf("ACmd EMTYHANDS: anim = %d", static_cast<int>(anim));
                    break;

                case animation::AnimCommandOpcode::Kill:
                    anim.finalAnimCommands.push_back({ command, 0, 0, 0 });
                    // ConsoleInfo::instance().printf("ACmd KILL: anim = %d", static_cast<int>(anim));
                    break;

                    /*
                    * Per frame commands:
                    */
                case animation::AnimCommandOpcode::PlaySound:
                    if(pointer[0] >= anim.firstFrame && pointer[0] - anim.firstFrame < static_cast<int>(anim.getFrameDuration()))
                    {
                        anim.animCommands(pointer[0] - anim.firstFrame).push_back({ command, pointer[1], 0, 0 });
                    }
                    // ConsoleInfo::instance().printf("ACmd PLAYSOUND: anim = %d, frame = %d of %d", static_cast<int>(anim), pointer[0], static_cast<int>(af->frames.size()));
                    pointer += 2;
                    break;

                case animation::AnimCommandOpcode::PlayEffect:
                    if(pointer[0] >= anim.firstFrame && pointer[0] - anim.firstFrame < static_cast<int>(anim.getFrameDuration()))
                    {
                        anim.animCommands(pointer[0] - anim.firstFrame).push_back({ command, pointer[1], 0, 0 });
                    }
                    //                    ConsoleInfo::instance().printf("ACmd FLIPEFFECT: anim = %d, frame = %d of %d", static_cast<int>(anim), pointer[0], static_cast<int>(af->frames.size()));
                    pointer += 2;
                    break;
            }
        }
    }
}

void SkeletalModel::loadStateChanges(const World& world, const loader::Level& level, const loader::AnimatedModel& animatedModel)
{
#ifdef LOG_ANIM_DISPATCHES
    if(animations.size() > 1)
    {
        BOOST_LOG_TRIVIAL(debug) << "MODEL[" << model_num << "], anims = " << animations.size();
    }
#endif
    for(size_t i = 0; i < m_animations.size(); i++)
    {
        BOOST_ASSERT(animatedModel.animation_index + i < level.m_animations.size());

        animation::Animation* anim = &m_animations[i];
        anim->m_transitions.clear();

        const loader::Animation& trAnimation = level.m_animations[animatedModel.animation_index + i];
        int16_t animId = trAnimation.nextAnimation - animatedModel.animation_index;
        animId &= 0x7fff; // this masks out the sign bit
        BOOST_ASSERT(animId >= 0);
        if(static_cast<size_t>(animId) < m_animations.size())
        {
            anim->next_anim = &m_animations[animId];
            BOOST_ASSERT(level.m_animations[trAnimation.nextAnimation].firstFrame <= trAnimation.nextFrame);
            BOOST_ASSERT(level.m_animations[trAnimation.nextAnimation].lastFrame >= trAnimation.nextFrame);
            anim->nextFrame = trAnimation.nextFrame - level.m_animations[trAnimation.nextAnimation].firstFrame;
            anim->nextFrame %= anim->next_anim->getFrameDuration(); //!< @todo Paranoid?
#ifdef LOG_ANIM_DISPATCHES
            BOOST_LOG_TRIVIAL(debug) << "ANIM[" << i << "], next_anim = " << anim->next_anim->id << ", next_frame = " << anim->next_frame;
#endif
        }
        else
        {
            anim->next_anim = nullptr;
            anim->nextFrame = 0;
        }

        anim->m_transitions.clear();

        if(trAnimation.transitionsCount > 0 && m_animations.size() > 1)
        {
#ifdef LOG_ANIM_DISPATCHES
            BOOST_LOG_TRIVIAL(debug) << "ANIM[" << i << "], next_anim = " << (anim->next_anim ? anim->next_anim->id : -1) << ", next_frame = " << anim->next_frame;
#endif
            for(uint16_t j = 0; j < trAnimation.transitionsCount; j++)
            {
                BOOST_ASSERT(j + trAnimation.transitionsIndex < level.m_transitions.size());
                const loader::Transitions *tr_sch = &level.m_transitions[j + trAnimation.transitionsIndex];
                if(anim->findTransitionById(static_cast<LaraState>(tr_sch->stateId)) != nullptr)
                {
                    BOOST_LOG_TRIVIAL(warning) << "Multiple state changes for id " << tr_sch->stateId;
                }
                animation::Transition* transition = &anim->m_transitions[static_cast<LaraState>(tr_sch->stateId)];
                transition->id = static_cast<LaraState>(tr_sch->stateId);
                transition->cases.clear();
                for(uint16_t l = 0; l < tr_sch->transitionCaseCount; l++)
                {
                    BOOST_ASSERT(tr_sch->firstTransitionCase + l < level.m_transitionCases.size());
                    const loader::TransitionCase *tr_adisp = &level.m_transitionCases[tr_sch->firstTransitionCase + l];
                    uint16_t next_anim = tr_adisp->targetAnimation & 0x7fff;
                    uint16_t next_anim_ind = next_anim - (animatedModel.animation_index & 0x7fff);
                    if(next_anim_ind >= m_animations.size())
                        continue;

                    transition->cases.emplace_back();
                    animation::TransitionCase* transitionCase = &transition->cases.back();

                    BOOST_ASSERT(animatedModel.animation_index <= next_anim);
                    BOOST_ASSERT(next_anim - animatedModel.animation_index < m_animations.size());
                    size_t next_frames_count = m_animations[next_anim - animatedModel.animation_index].getFrameDuration();
                    BOOST_ASSERT(next_anim < level.m_animations.size());
                    size_t next_frame = tr_adisp->targetFrame - level.m_animations[next_anim].firstFrame;

                    uint16_t low = tr_adisp->firstFrame - trAnimation.firstFrame;
                    uint16_t high = tr_adisp->lastFrame - trAnimation.firstFrame;

                    // this is not good: frame_high can be frame_end+1 (for last-frame-loop statechanges,
                    // secifically fall anims (75,77 etc), which may fail to change state),
                    // And: if theses values are > framesize, then they're probably faulty and won't be fixed by modulo anyway:
                    //                        adsp->frame_low = low  % anim->frames.size();
                    //                        adsp->frame_high = (high - 1) % anim->frames.size();
                    if(low > anim->getFrameDuration() || high > anim->getFrameDuration())
                    {
                        //Sys_Warn("State range out of bounds: anim: %d, stid: %d, low: %d, high: %d", anim->id, sch_p->id, low, high);
                        world.m_engine->m_gui.getConsole().printf("State range out of bounds: anim: %d, stid: %d, low: %d, high: %d, duration: %d, timestretch: %d", anim->id, transition->id, low, high, int(anim->getFrameDuration()), int(anim->getStretchFactor()));
                    }
                    transitionCase->firstFrame = low;
                    transitionCase->lastFrame = high;
                    BOOST_ASSERT(next_anim >= animatedModel.animation_index);
                    transitionCase->target.animation = next_anim - animatedModel.animation_index;
                    transitionCase->target.frame = next_frame % next_frames_count;

#ifdef LOG_ANIM_DISPATCHES
                    BOOST_LOG_TRIVIAL(debug) << "anim_disp["
                        << l
                        << "], duration = "
                        << anim->getFrameDuration()
                        << ": interval["
                        << adsp->frame_low
                        << ".."
                        << adsp->frame_high
                        << "], next_anim = "
                        << adsp->next.animation
                        << ", next_frame = "
                        << adsp->next.frame;
#endif
                }
            }
        }
    }
}

void SkeletalModel::setStaticAnimation()
{
    m_animations.resize(1);
    m_animations.front().setDuration(1, 1, 1);
    animation::SkeletonPose& skeletonPose = m_animations.front().rawPose(0);

    m_animations.front().id = 0;
    m_animations.front().next_anim = nullptr;
    m_animations.front().nextFrame = 0;
    m_animations.front().m_transitions.clear();

    skeletonPose.bonePoses.resize(m_meshReferences.size());

    skeletonPose.position = { 0,0,0 };

    for(size_t k = 0; k < skeletonPose.bonePoses.size(); k++)
    {
        animation::BonePose& bonePose = skeletonPose.bonePoses[k];

        bonePose.rotation = util::trRotationToQuat({ 0,0,0 });
        bonePose.position = m_meshReferences[k].position;
    }
}

void SkeletalModel::loadAnimations(const loader::Level& level, size_t moveable)
{
    m_animations.resize(getAnimationCountForMoveable(level, moveable));
    if(m_animations.empty())
    {
        /*
        * the animation count must be >= 1
        */
        m_animations.resize(1);
    }

    /*
    *   Ok, let us calculate animations;
    *   there is no difficult:
    * - first 9 words are bounding box and frame offset coordinates.
    * - 10's word is a rotations count, must be equal to number of meshes in model.
    *   BUT! only in TR1. In TR2 - TR5 after first 9 words begins next section.
    * - in the next follows rotation's data. one word - one rotation, if rotation is one-axis (one angle).
    *   two words in 3-axis rotations (3 angles). angles are calculated with bit mask.
    */
    for(size_t i = 0; i < m_animations.size(); i++)
    {
        BOOST_ASSERT(level.m_animatedModels[moveable]->animation_index + i < level.m_animations.size());
        animation::Animation* anim = &m_animations[i];
        const loader::Animation& trAnimation = level.m_animations[level.m_animatedModels[moveable]->animation_index + i];
        anim->firstFrame = trAnimation.firstFrame;

        size_t poseDataOffset = trAnimation.poseDataOffset / 2;
        uint16_t l_start = 0x09;

        if(loader::gameToEngine(level.m_gameVersion) == loader::Engine::TR1)
        {
            l_start = 0x0A;
        }

        anim->id = i;
        // BOOST_LOG_TRIVIAL(debug) << "Anim " << i << " stretch factor = " << int(trAnimation.stretchFactor) << ", frame count = " << (trAnimation.lastFrame - trAnimation.firstFrame + 1);

        anim->speed_x = trAnimation.speed;
        anim->accel_x = trAnimation.accelleration;
        anim->speed_y = trAnimation.lateralAccelleration;
        anim->accel_y = trAnimation.lateralSpeed;

        anim->animationCommandIndex = trAnimation.animCommandIndex;
        anim->animationCommandCount = trAnimation.animCommandCount;
        anim->state_id = static_cast<LaraState>(trAnimation.state_id);

        //        anim->frames.resize(TR_GetNumFramesForAnimation(tr, tr_moveable->animation_index + i));
        // FIXME: number of frames is always (frame_end - frame_start + 1)
        // this matters for transitional anims, which run through their frame length with the same anim frame.
        // This should ideally be solved without filling identical frames,
        // but due to the amount of currFrame-indexing, waste dummy frames for now:
        // (I haven't seen this for framerate==1 animation, but it would be possible,
        //  also, resizing now saves re-allocations in interpolateFrames later)

        BOOST_ASSERT(trAnimation.firstFrame <= trAnimation.lastFrame);
        // const size_t keyFrameCount = getAnimationCountForMoveable(level, level.m_moveables[moveable]->animation_index + i);
        const size_t keyFrameCount = (trAnimation.lastFrame - trAnimation.firstFrame + trAnimation.stretchFactor) / trAnimation.stretchFactor;
        BOOST_ASSERT(keyFrameCount > 0);
        BOOST_ASSERT(keyFrameCount*trAnimation.stretchFactor >= trAnimation.lastFrame - trAnimation.firstFrame + 1u);
        anim->setDuration(trAnimation.lastFrame - trAnimation.firstFrame + 1, keyFrameCount, trAnimation.stretchFactor);

        //Sys_DebugLog(LOG_FILENAME, "Anim[%d], %d", tr_moveable->animation_index, TR_GetNumFramesForAnimation(tr, tr_moveable->animation_index));

        if(anim->getFrameDuration() == 0)
        {
            /*
            * number of animations must be >= 1, because frame contains base model offset
            */
            anim->setDuration(1, 1, anim->getStretchFactor());
        }

        /*
        * let us begin to load animations
        */
        for(size_t j = 0; j < anim->getKeyFrameCount(); ++j, poseDataOffset += trAnimation.poseDataSize)
        {
            animation::SkeletonPose* skeletonPose = &anim->rawPose(j);
            // !Need bonetags in empty frames:
            skeletonPose->bonePoses.resize(m_meshReferences.size());

            if(j >= keyFrameCount)
            {
                BOOST_LOG_TRIVIAL(warning) << "j=" << j << ", keyFrameCount=" << keyFrameCount << ", anim->getKeyFrameCount()=" << anim->getKeyFrameCount();
                continue;
            }

            skeletonPose->position = { 0,0,0 };
            skeletonPose->load(level, poseDataOffset);

            if(poseDataOffset >= level.m_poseData.size())
            {
                for(size_t k = 0; k < skeletonPose->bonePoses.size(); k++)
                {
                    animation::BonePose* bonePose = &skeletonPose->bonePoses[k];
                    bonePose->rotation = util::trRotationToQuat({ 0,0,0 });
                    bonePose->position = m_meshReferences[k].position;
                }
                continue;
            }

            uint16_t l = l_start;
            uint16_t temp1, temp2;
            float ang;

            for(size_t k = 0; k < skeletonPose->bonePoses.size(); k++)
            {
                animation::BonePose* bonePose = &skeletonPose->bonePoses[k];
                bonePose->rotation = util::trRotationToQuat({ 0,0,0 });
                bonePose->position = m_meshReferences[k].position;

                if(loader::gameToEngine(level.m_gameVersion) == loader::Engine::TR1)
                {
                    temp2 = level.m_poseData[poseDataOffset + l];
                    l++;
                    temp1 = level.m_poseData[poseDataOffset + l];
                    l++;
                    glm::vec3 rot;
                    rot[0] = static_cast<float>((temp1 & 0x3ff0) >> 4);
                    rot[2] = -static_cast<float>(((temp1 & 0x000f) << 6) | ((temp2 & 0xfc00) >> 10));
                    rot[1] = static_cast<float>(temp2 & 0x03ff);
                    rot *= 360.0 / 1024.0;
                    bonePose->rotation = util::trRotationToQuat(rot);
                }
                else
                {
                    temp1 = level.m_poseData[poseDataOffset + l];
                    l++;
                    if(loader::gameToEngine(level.m_gameVersion) >= loader::Engine::TR4)
                    {
                        ang = static_cast<float>(temp1 & 0x0fff);
                        ang *= 360.0 / 4096.0;
                    }
                    else
                    {
                        ang = static_cast<float>(temp1 & 0x03ff);
                        ang *= 360.0 / 1024.0;
                    }

                    switch(temp1 & 0xc000)
                    {
                        case 0x4000:    // x only
                            bonePose->rotation = util::trRotationToQuat({ ang,0,0 });
                            break;

                        case 0x8000:    // y only
                            bonePose->rotation = util::trRotationToQuat({ 0,0,-ang });
                            break;

                        case 0xc000:    // z only
                            bonePose->rotation = util::trRotationToQuat({ 0,ang,0 });
                            break;

                        default:
                        {        // all three
                            temp2 = level.m_poseData[poseDataOffset + l];
                            glm::vec3 rot;
                            rot[0] = static_cast<float>((temp1 & 0x3ff0) >> 4);
                            rot[2] = -static_cast<float>(((temp1 & 0x000f) << 6) | ((temp2 & 0xfc00) >> 10));
                            rot[1] = static_cast<float>(temp2 & 0x03ff);
                            rot *= 360.0 / 1024.0;
                            bonePose->rotation = util::trRotationToQuat(rot);
                            l++;
                            break;
                        }
                    };
                }
            }
        }
    }
}

size_t SkeletalModel::getAnimationCountForMoveable(const loader::Level& level, size_t modelIndex)
{
    BOOST_ASSERT(modelIndex < level.m_animatedModels.size());
    const std::unique_ptr<loader::AnimatedModel>& currModel = level.m_animatedModels[modelIndex];

    if(currModel->animation_index == 0xFFFF)
    {
        return 0;
    }

    if(modelIndex == level.m_animatedModels.size() - 1)
    {
        if(level.m_animations.size() < currModel->animation_index)
        {
            return 1;
        }

        return level.m_animations.size() - currModel->animation_index;
    }

    const loader::AnimatedModel* nextModel = level.m_animatedModels[modelIndex + 1].get();
    if(nextModel->animation_index == 0xFFFF)
    {
        if(modelIndex + 2 < level.m_animatedModels.size())                              // I hope there is no two neighboard movables with animation_index'es == 0xFFFF
        {
            nextModel = level.m_animatedModels[modelIndex + 2].get();
        }
        else
        {
            return 1;
        }
    }

    return std::min(static_cast<size_t>(nextModel->animation_index), level.m_animations.size()) - currModel->animation_index;
}

void SkeletalModel::patchLaraSkin(World& world, loader::Engine engineVersion)
{
    BOOST_ASSERT(m_id == 0);

    switch(engineVersion)
    {
        case loader::Engine::TR1:
            if(world.m_engine->m_gameflow.getLevelID() == 0)
            {
                if(std::shared_ptr<SkeletalModel> skinModel = world.getModelByID(TR_ITEM_LARA_SKIN_ALTERNATE_TR1))
                {
                    // In TR1, Lara has unified head mesh for all her alternate skins.
                    // Hence, we copy all meshes except head, to prevent Potato Raider bug.
                    setMeshes(skinModel->m_meshReferences, m_meshReferences.size() - 1);
                }
            }
            break;

        case loader::Engine::TR2:
            break;

        case loader::Engine::TR3:
            if(std::shared_ptr<SkeletalModel> skinModel = world.getModelByID(TR_ITEM_LARA_SKIN_TR3))
            {
                setMeshes(skinModel->m_meshReferences, m_meshReferences.size());
                auto tmp = world.getModelByID(11);                   // moto / quadro cycle animations
                if(tmp)
                {
                    tmp->setMeshes(skinModel->m_meshReferences, m_meshReferences.size());
                }
            }
            break;

        case loader::Engine::TR4:
        case loader::Engine::TR5:
            // base skeleton meshes
            if(std::shared_ptr<SkeletalModel> skinModel = world.getModelByID(TR_ITEM_LARA_SKIN_TR45))
            {
                setMeshes(skinModel->m_meshReferences, m_meshReferences.size());
            }
            // skin skeleton meshes
            if(std::shared_ptr<SkeletalModel> skinModel = world.getModelByID(TR_ITEM_LARA_SKIN_JOINTS_TR45))
            {
                setSkinnedMeshes(skinModel->m_meshReferences, m_meshReferences.size());
            }
            fillSkinnedMeshMap();
            break;

        case loader::Engine::Unknown:
            break;
    };
}

void SkeletalModel::lua_SetModelMeshReplaceFlag(engine::Engine& engine, ModelId id, size_t bone, int flag)
{
    auto sm = engine.m_world.getModelByID(id);
    if(sm != nullptr)
    {
        if(bone < sm->getMeshReferenceCount())
        {
            sm->m_meshReferences[bone].replace_mesh = flag;
        }
        else
        {
            engine.m_gui.getConsole().warning(SYSWARN_WRONG_BONE_NUMBER, bone);
        }
    }
    else
    {
        engine.m_gui.getConsole().warning(SYSWARN_WRONG_MODEL_ID, id);
    }
}

void SkeletalModel::lua_SetModelAnimReplaceFlag(engine::Engine& engine, ModelId id, size_t bone, bool flag)
{
    auto sm = engine.m_world.getModelByID(id);
    if(sm != nullptr)
    {
        if(bone < sm->getMeshReferenceCount())
        {
            sm->m_meshReferences[bone].replace_anim = flag;
        }
        else
        {
            engine.m_gui.getConsole().warning(SYSWARN_WRONG_BONE_NUMBER, bone);
        }
    }
    else
    {
        engine.m_gui.getConsole().warning(SYSWARN_WRONG_MODEL_ID, id);
    }
}

void SkeletalModel::lua_CopyMeshFromModelToModel(engine::Engine& engine, ModelId id1, ModelId id2, size_t bone1, size_t bone2)
{
    auto sm1 = engine.m_world.getModelByID(id1);
    if(sm1 == nullptr)
    {
        engine.m_gui.getConsole().warning(SYSWARN_WRONG_MODEL_ID, id1);
        return;
    }

    auto sm2 = engine.m_world.getModelByID(id2);
    if(sm2 == nullptr)
    {
        engine.m_gui.getConsole().warning(SYSWARN_WRONG_MODEL_ID, id2);
        return;
    }

    if(bone1 < sm1->getMeshReferenceCount() && bone2 < sm2->getMeshReferenceCount())
    {
        sm1->m_meshReferences[bone1].mesh_base = sm2->m_meshReferences[bone2].mesh_base;
    }
    else
    {
        engine.m_gui.getConsole().warning(SYSWARN_WRONG_BONE_NUMBER, bone1);
    }
}

void SkeletalModel::lua_SetModelBodyPartFlag(engine::Engine& engine, ModelId id, int bone_id, int body_part_flag)
{
    auto model = engine.m_world.getModelByID(id);

    if(model == nullptr)
    {
        engine.m_gui.getConsole().warning(SYSWARN_NO_SKELETAL_MODEL, id);
        return;
    }

    if(bone_id < 0 || static_cast<size_t>(bone_id) >= model->getMeshReferenceCount())
    {
        engine.m_gui.getConsole().warning(SYSWARN_WRONG_OPTION_INDEX, bone_id);
        return;
    }

    model->m_meshReferences[bone_id].body_part = body_part_flag;
}
} // namespace world
