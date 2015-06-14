
#include <math.h>
#include "bullet/LinearMath/btScalar.h"
#include "bullet/BulletCollision/CollisionDispatch/btGhostObject.h"
#include "ragdoll.h"
#include "vmath.h"
#include "character_controller.h"

btScalar getInnerBBRadius(btScalar bb_min[3], btScalar bb_max[3])
{
    btScalar r = bb_max[0] - bb_min[0];
    btScalar t = bb_max[1] - bb_min[1];
    r = (t > r)?(r):(t);
    t = bb_max[2] - bb_min[2];
    return (t > r)?(r):(t);
}

bool Ragdoll_Create(entity_p entity, rd_setup_p setup)
{
    // No entity, setup or body count overflow - bypass function.

    if( (!entity) || (!setup) ||
        (setup->body_count > entity->bf.bone_tag_count) )
    {
        return false;
    }

    bool result = true;

    // If ragdoll already exists, overwrite it with new one.

    if(entity->bt_joint_count > 0)
    {
        result = Ragdoll_Delete(entity);
    }

    // Setup bodies.

    entity->bt_joint_count = 0;
    for(int i=0; i<setup->body_count; i++)
    {
        if( (i >= entity->bf.bone_tag_count) || (entity->bt_body[i] == NULL) )
        {
            result = false;
            continue;   // If body is absent, return false and bypass this body setup.
        }

        if(entity->bf.bone_tags[i].parent != NULL)
        {
            entity->bt_joint_count++;
        }

        // update current character animation and full fix body to avoid starting ragdoll partially inside the wall or floor...
        Entity_UpdateCurrentBoneFrame(&entity->bf, entity->transform);
        entity->character->no_fix_all = 0x00;
        entity->character->no_fix_body_parts = 0x00000000;
        Character_FixPenetrations(entity, NULL);

        btVector3 inertia (0.0, 0.0, 0.0);
        btScalar  mass = setup->body_setup[i].mass;

            bt_engine_dynamicsWorld->removeRigidBody(entity->bt_body[i]);

                entity->bt_body[i]->getCollisionShape()->calculateLocalInertia(mass, inertia);
                entity->bt_body[i]->setMassProps(mass, inertia);

                entity->bt_body[i]->updateInertiaTensor();
                entity->bt_body[i]->clearForces();

                entity->bt_body[i]->setLinearFactor (btVector3(1.0, 1.0, 1.0));
                entity->bt_body[i]->setAngularFactor(btVector3(1.0, 1.0, 1.0));

                entity->bt_body[i]->setDamping(setup->body_setup[i].damping[0], setup->body_setup[i].damping[1]);
                entity->bt_body[i]->setRestitution(setup->body_setup[i].restitution);
                entity->bt_body[i]->setFriction(setup->body_setup[i].friction);

                if(i == 0)
                {
                    entity->bf.bone_tags[i].mesh_base;
                    btScalar r = getInnerBBRadius(entity->bf.bone_tags[i].mesh_base->bb_min, entity->bf.bone_tags[i].mesh_base->bb_max);
                    entity->bt_body[i]->setCcdMotionThreshold(0.8 * r);
                    entity->bt_body[i]->setCcdSweptSphereRadius(r);
                }
    }

    //entity->type_flags |=  ENTITY_TYPE_DYNAMIC;           // cause wrong behavior in render!
    Entity_UpdateRigidBody(entity, 1);

    for(uint16_t i=0;i<entity->bf.bone_tag_count;i++)
    {
        bt_engine_dynamicsWorld->addRigidBody(entity->bt_body[i]);
        entity->bt_body[i]->activate();
        entity->bt_body[i]->setLinearVelocity(entity->speed);
        if(entity->character && entity->character->ghostObjects[i])
        {
            bt_engine_dynamicsWorld->removeCollisionObject(entity->character->ghostObjects[i]);
            bt_engine_dynamicsWorld->addCollisionObject(entity->character->ghostObjects[i], COLLISION_NONE, COLLISION_NONE);
        }
    }

    // Setup constraints.

    entity->bt_joint_count = setup->joint_count;
    entity->bt_joints = (btTypedConstraint**)calloc(entity->bt_joint_count, sizeof(btTypedConstraint*));

    for(int i=0; i<entity->bt_joint_count; i++)
    {
        if( (setup->joint_setup[i].body_index >= entity->bf.bone_tag_count) ||
            (entity->bt_body[setup->joint_setup[i].body_index] == NULL) )
        {
            result = false;
            break;       // If body 1 or body 2 are absent, return false and bypass this joint.
        }

        btTransform localA, localB;
        ss_bone_tag_p btB = entity->bf.bone_tags + setup->joint_setup[i].body_index;
        ss_bone_tag_p btA = btB->parent;
        if(btA == NULL)
        {
            result = false;
            break;
        }
#if 0
        localA.setFromOpenGLMatrix(btB->transform);
        localB.setIdentity();
#else
        localA.getBasis().setEulerZYX(setup->joint_setup[i].body1_angle[0], setup->joint_setup[i].body1_angle[1], setup->joint_setup[i].body1_angle[2]);
        //localA.setOrigin(setup->joint_setup[i].body1_offset);
        localA.setOrigin(btVector3(btB->transform[12+0], btB->transform[12+1], btB->transform[12+2]));

        localB.getBasis().setEulerZYX(setup->joint_setup[i].body2_angle[0], setup->joint_setup[i].body2_angle[1], setup->joint_setup[i].body2_angle[2]);
        //localB.setOrigin(setup->joint_setup[i].body2_offset);
        localB.setOrigin(btVector3(0.0, 0.0, 0.0));
#endif

        switch(setup->joint_setup[i].joint_type)
        {
            case RD_CONSTRAINT_POINT:
                {
                    btPoint2PointConstraint* pointC = new btPoint2PointConstraint(*entity->bt_body[btA->index], *entity->bt_body[btB->index], localA.getOrigin(), localB.getOrigin());
                    entity->bt_joints[i] = pointC;
                }
                break;

            case RD_CONSTRAINT_HINGE:
                {
                    btHingeConstraint* hingeC = new btHingeConstraint(*entity->bt_body[btA->index], *entity->bt_body[btB->index], localA, localB);
                    hingeC->setLimit(setup->joint_setup[i].joint_limit[0], setup->joint_setup[i].joint_limit[1], 0.9f, 0.3f, 0.3f);
                    entity->bt_joints[i] = hingeC;
                }
                break;

            case RD_CONSTRAINT_CONE:
                {
                    btConeTwistConstraint* coneC = new btConeTwistConstraint(*entity->bt_body[btA->index], *entity->bt_body[btB->index], localA, localB);
                    coneC->setLimit(setup->joint_setup[i].joint_limit[0], setup->joint_setup[i].joint_limit[1], setup->joint_setup[i].joint_limit[2], 0.9f, 0.3f, 0.7f);
                    entity->bt_joints[i] = coneC;
                }
                break;
        }

        for(int j=0; j<=5; j++)
        {
            entity->bt_joints[i]->setParam(BT_CONSTRAINT_STOP_CFM, setup->joint_cfm, j);
            entity->bt_joints[i]->setParam(BT_CONSTRAINT_STOP_ERP, setup->joint_erp, j);
        }

        entity->bt_joints[i]->setDbgDrawSize(64.0);
        bt_engine_dynamicsWorld->addConstraint(entity->bt_joints[i], true);
    }

    if(result == false) Ragdoll_Delete(entity);  // PARANOID: Clean up the mess, if something went wrong.
    return result;
}


bool Ragdoll_Delete(entity_p entity)
{
    if(entity->bt_joint_count == 0) return false;

    for(int i=0; i<entity->bt_joint_count; i++)
    {
        if(entity->bt_joints[i] != NULL)
        {
            bt_engine_dynamicsWorld->removeConstraint(entity->bt_joints[i]);
            delete entity->bt_joints[i];
            entity->bt_joints[i] = NULL;
        }
    }

    for(int i=0;i<entity->bf.bone_tag_count;i++)
    {
        bt_engine_dynamicsWorld->removeRigidBody(entity->bt_body[i]);
        entity->bt_body[i]->setMassProps(0, btVector3(0.0, 0.0, 0.0));
        bt_engine_dynamicsWorld->addRigidBody(entity->bt_body[i], COLLISION_GROUP_KINEMATIC, COLLISION_MASK_ALL);
        if(entity->character && entity->character->ghostObjects[i])
        {
            bt_engine_dynamicsWorld->removeCollisionObject(entity->character->ghostObjects[i]);
            bt_engine_dynamicsWorld->addCollisionObject(entity->character->ghostObjects[i], COLLISION_GROUP_CHARACTERS, COLLISION_GROUP_ALL);
        }
    }

    free(entity->bt_joints);
    entity->bt_joints = NULL;
    entity->bt_joint_count = 0;

    entity->type_flags &= ~ENTITY_TYPE_DYNAMIC;

    return true;

    // NB! Bodies remain in the same state!
    // To make them static again, additionally call setEntityBodyMass script function.
}



void Ragdoll_Update(entity_p entity)
{
    btScalar tr[16];
    btVector3 pos = entity->bt_body[0]->getWorldTransform().getOrigin();
    vec3_copy(entity->transform+12, pos.m_floats);
    Entity_UpdateRoomPos(entity);
    for(uint16_t i=0;i<entity->bf.bone_tag_count;i++)
    {
        entity->bt_body[i]->getWorldTransform().getOpenGLMatrix(tr);
        Mat4_inv_Mat4_affine_mul(entity->bf.bone_tags[i].full_transform, entity->transform, tr);
    }

    for(uint16_t i=0;i<entity->bf.bone_tag_count;i++)
    {
        if(entity->bf.bone_tags[i].parent != NULL)
        {
            Mat4_inv_Mat4_affine_mul(entity->bf.bone_tags[i].transform, entity->bf.bone_tags[i].parent->full_transform, entity->bf.bone_tags[i].full_transform);
        }
        else
        {
            Mat4_Copy(entity->bf.bone_tags[i].transform, entity->bf.bone_tags[i].full_transform);
        }
    }

    if(entity->character && entity->character->ghostObjects)
    {
        btScalar v[3];
        for(uint16_t i=0;i<entity->bf.bone_tag_count;i++)
        {
            entity->bt_body[i]->getWorldTransform().getOpenGLMatrix(tr);
            Mat4_vec3_mul(v, tr, entity->bf.bone_tags[i].mesh_base->centre);
            vec3_copy(tr+12, v);
            entity->character->ghostObjects[i]->getWorldTransform().setFromOpenGLMatrix(tr);
        }
    }
}


bool Ragdoll_GetSetup(int ragdoll_index, rd_setup_p setup)
{
    if(!setup) return false;

    bool result = true;

    int top = lua_gettop(engine_lua);
    assert(top >= 0);

    lua_getglobal(engine_lua, "getRagdollSetup");
    if(lua_isfunction(engine_lua, -1))
    {
        lua_pushinteger(engine_lua, ragdoll_index);
        if(lua_CallAndLog(engine_lua, 1, 1, 0))
        {
            if(lua_istable(engine_lua, -1))
            {
                lua_getfield(engine_lua, -1, "hit_callback");
                if(lua_isstring(engine_lua, -1))
                {
                    size_t string_length  = 0;
                    const char* func_name = lua_tolstring(engine_lua, -1, &string_length);

                    setup->hit_func = (char*)calloc(string_length, sizeof(char));
                    memcpy(setup->hit_func, func_name, string_length * sizeof(char));
                }
                else { result = false; }
                lua_pop(engine_lua, 1);

                setup->joint_count = (uint32_t)lua_GetScalarField(engine_lua, "joint_count");
                setup->body_count  = (uint32_t)lua_GetScalarField(engine_lua, "body_count");

                setup->joint_cfm   = lua_GetScalarField(engine_lua, "joint_cfm");
                setup->joint_erp   = lua_GetScalarField(engine_lua, "joint_erp");

                if(setup->body_count > 0)
                {
                    setup->body_setup  = (rd_body_setup_p)calloc(setup->body_count, sizeof(rd_body_setup_t));

                    lua_getfield(engine_lua, -1, "body");
                    if(lua_istable(engine_lua, -1))
                    {
                        for(int i=0; i<setup->body_count; i++)
                        {
                            lua_rawgeti(engine_lua, -1, i+1);
                            if(lua_istable(engine_lua, -1))
                            {
                                setup->body_setup[i].mass = lua_GetScalarField(engine_lua, "mass");
                                setup->body_setup[i].restitution = lua_GetScalarField(engine_lua, "restitution");
                                setup->body_setup[i].friction = lua_GetScalarField(engine_lua, "friction");

                                lua_getfield(engine_lua, -1, "damping");
                                if(lua_istable(engine_lua, -1))
                                {
                                    setup->body_setup[i].damping[0] = lua_GetScalarField(engine_lua, 1);
                                    setup->body_setup[i].damping[1] = lua_GetScalarField(engine_lua, 2);
                                }
                                else { result = false; }
                                lua_pop(engine_lua, 1);
                            }
                            else { result = false; }
                            lua_pop(engine_lua, 1);
                        }
                    }
                    else { result = false; }
                    lua_pop(engine_lua, 1);
                }
                else { result = false; }

                if(setup->joint_count > 0)
                {
                    setup->joint_setup = (rd_joint_setup_p)calloc(setup->joint_count, sizeof(rd_joint_setup_t));

                    lua_getfield(engine_lua, -1, "joint");
                    if(lua_istable(engine_lua, -1))
                    {
                        for(int i=0; i<setup->joint_count; i++)
                        {
                            lua_rawgeti(engine_lua, -1, i+1);
                            if(lua_istable(engine_lua, -1))
                            {
                                setup->joint_setup[i].body_index = (uint16_t)lua_GetScalarField(engine_lua, "body_index");
                                setup->joint_setup[i].joint_type = (uint16_t)lua_GetScalarField(engine_lua, "joint_type");

                                lua_getfield(engine_lua, -1, "body1_offset");
                                if(lua_istable(engine_lua, -1))
                                {
                                    setup->joint_setup[i].body1_offset.m_floats[0] = lua_GetScalarField(engine_lua, 1);
                                    setup->joint_setup[i].body1_offset.m_floats[1] = lua_GetScalarField(engine_lua, 2);
                                    setup->joint_setup[i].body1_offset.m_floats[2] = lua_GetScalarField(engine_lua, 3);
                                }
                                else { result = false; }
                                lua_pop(engine_lua, 1);

                                lua_getfield(engine_lua, -1, "body2_offset");
                                if(lua_istable(engine_lua, -1))
                                {
                                    setup->joint_setup[i].body2_offset.m_floats[0] = lua_GetScalarField(engine_lua, 1);
                                    setup->joint_setup[i].body2_offset.m_floats[1] = lua_GetScalarField(engine_lua, 2);
                                    setup->joint_setup[i].body2_offset.m_floats[2] = lua_GetScalarField(engine_lua, 3);
                                }
                                else { result = false; }
                                lua_pop(engine_lua, 1);

                                lua_getfield(engine_lua, -1, "body1_angle");
                                if(lua_istable(engine_lua, -1))
                                {
                                    setup->joint_setup[i].body1_angle[0] = lua_GetScalarField(engine_lua, 1);
                                    setup->joint_setup[i].body1_angle[1] = lua_GetScalarField(engine_lua, 2);
                                    setup->joint_setup[i].body1_angle[2] = lua_GetScalarField(engine_lua, 3);
                                }
                                else { result = false; }
                                lua_pop(engine_lua, 1);

                                lua_getfield(engine_lua, -1, "body2_angle");
                                if(lua_istable(engine_lua, -1))
                                {
                                    setup->joint_setup[i].body2_angle[0] = lua_GetScalarField(engine_lua, 1);
                                    setup->joint_setup[i].body2_angle[1] = lua_GetScalarField(engine_lua, 2);
                                    setup->joint_setup[i].body2_angle[2] = lua_GetScalarField(engine_lua, 3);
                                }
                                else { result = false; }
                                lua_pop(engine_lua, 1);

                                lua_getfield(engine_lua, -1, "joint_limit");
                                if(lua_istable(engine_lua, -1))
                                {
                                    setup->joint_setup[i].joint_limit[0] = lua_GetScalarField(engine_lua, 1);
                                    setup->joint_setup[i].joint_limit[1] = lua_GetScalarField(engine_lua, 2);
                                    setup->joint_setup[i].joint_limit[2] = lua_GetScalarField(engine_lua, 3);
                                }
                                else { result = false; }
                                lua_pop(engine_lua, 1);
                            }
                            else { result = false; }
                            lua_pop(engine_lua, 1);
                        }
                    }
                    else { result = false; }
                    lua_pop(engine_lua, 1);
                }
                else { result = false; }
            }
            else { result = false; }
        }
        else { result = false; }
    }
    else { result = false; }

    lua_settop(engine_lua, top);

    if(result == false) Ragdoll_ClearSetup(setup);  // PARANOID: Clean up the mess, if something went wrong.
    return result;
}


void Ragdoll_ClearSetup(rd_setup_p setup)
{
    if(!setup) return;

    free(setup->body_setup);
    setup->body_setup = NULL;
    setup->body_count = 0;

    free(setup->joint_setup);
    setup->joint_setup = NULL;
    setup->joint_count = 0;

    free(setup->hit_func);
    setup->hit_func = NULL;
}
