
#include <math.h>
#include "bullet/LinearMath/btScalar.h"
#include "ragdoll.h"

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
    
    entity->bt_joint_count = setup->joint_count;
    entity->bt_joints = (btTypedConstraint**)calloc(entity->bt_joint_count, sizeof(btTypedConstraint*));
    
    for(int i=0; i<entity->bt_joint_count; i++)
    {
        if( (setup->joint_setup[i].body1_index >= entity->bf.bone_tag_count) ||
            (setup->joint_setup[i].body2_index >= entity->bf.bone_tag_count) ||
            (entity->bt_body[setup->joint_setup[i].body1_index] == NULL)     ||
            (entity->bt_body[setup->joint_setup[i].body2_index] == NULL)      )
        {
            result = false;
            continue;       // If body 1 or body 2 are absent, return false and bypass this joint.
        }
        
        btTransform localA, localB;
        localA.setIdentity(); localB.setIdentity();
        
        localA.getBasis().setEulerZYX(setup->joint_setup[i].body1_angle[0], setup->joint_setup[i].body1_angle[1], setup->joint_setup[i].body1_angle[2]);
        localA.setOrigin(setup->joint_setup[i].body1_offset);
        
        localB.getBasis().setEulerZYX(setup->joint_setup[i].body2_angle[0], setup->joint_setup[i].body2_angle[1], setup->joint_setup[i].body2_angle[2]);
        localB.setOrigin(setup->joint_setup[i].body2_offset);
        
        switch(setup->joint_setup[i].joint_type)
        {
            case RD_CONSTRAINT_POINT:
                {
                    btPoint2PointConstraint* pointC = new btPoint2PointConstraint(*entity->bt_body[setup->joint_setup[i].body1_index], *entity->bt_body[setup->joint_setup[i].body2_index], (const btVector3)setup->joint_setup[i].body1_offset, (const btVector3)setup->joint_setup[i].body2_offset);
                    entity->bt_joints[i] = pointC;
                }
                break;
                
            case RD_CONSTRAINT_HINGE:
                {
                    btHingeConstraint* hingeC = new btHingeConstraint(*entity->bt_body[setup->joint_setup[i].body1_index], *entity->bt_body[setup->joint_setup[i].body2_index], localA, localB);
                    hingeC->setLimit(setup->joint_setup[i].joint_limit[0], setup->joint_setup[i].joint_limit[1]);
                    entity->bt_joints[i] = hingeC;
                    
                }
                break;
                
            case RD_CONSTRAINT_CONE:
                {
                    btConeTwistConstraint* coneC = new btConeTwistConstraint(*entity->bt_body[setup->joint_setup[i].body1_index], *entity->bt_body[setup->joint_setup[i].body2_index], localA, localB);
                    coneC->setLimit(setup->joint_setup[i].joint_limit[0], setup->joint_setup[i].joint_limit[1], setup->joint_setup[i].joint_limit[2]);
                    entity->bt_joints[i] = coneC;
                }
                break;
        }
        
        entity->bt_joints[i]->setDbgDrawSize(64.0);        
        bt_engine_dynamicsWorld->addConstraint(entity->bt_joints[i], true);
    }
    
    for(int i=0; i<setup->body_count; i++)
    {
        if( (i >= entity->bf.bone_tag_count) || (entity->bt_body[i] == NULL) )
        {
            result = false;
            continue;   // If body is absent, return false and bypass this body setup.
        }
           
        btVector3 inertia (0.0, 0.0, 0.0);
        btScalar  mass = setup->body_setup[i].weight;
        
            bt_engine_dynamicsWorld->removeRigidBody(entity->bt_body[i]);

                entity->bt_body[i]->getCollisionShape()->calculateLocalInertia(mass, inertia);
                entity->bt_body[i]->setMassProps(mass, inertia);

                entity->bt_body[i]->updateInertiaTensor();
                entity->bt_body[i]->clearForces();

                entity->bt_body[i]->setLinearFactor (btVector3(1.0, 1.0, 1.0));
                entity->bt_body[i]->setAngularFactor(btVector3(1.0, 1.0, 1.0));

            bt_engine_dynamicsWorld->addRigidBody(entity->bt_body[i]);

            entity->bt_body[i]->activate();
    }
    
    entity->type_flags |=  ENTITY_TYPE_DYNAMIC;
    Entity_UpdateRigidBody(entity, 1);
    
    return result;
}


bool Ragdoll_Delete(entity_p entity)
{
    if(entity->bt_joint_count == 0) return false;
    
    for(int i=0; i<entity->bt_joint_count; i++)
    {
        bt_engine_dynamicsWorld->removeConstraint(entity->bt_joints[i]);
        delete entity->bt_joints[i];
        entity->bt_joints[i] = NULL;
    }
    
    free(entity->bt_joints);
    entity->bt_joints = NULL;
    entity->bt_joint_count = 0;
    
    return true;
    
    // NB! Bodies remain in the same state!
    // To make them static again, additionally call setEntityBodyMass script function.
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
                setup->body_count = (uint32_t)lua_GetScalarField(engine_lua, "body_count");
                
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
                                setup->body_setup[i].weight = lua_GetScalarField(engine_lua, "weight");
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
                                setup->joint_setup[i].body1_index = (uint32_t)lua_GetScalarField(engine_lua, "body1_index");
                                setup->joint_setup[i].body2_index = (uint32_t)lua_GetScalarField(engine_lua, "body2_index");
                                
                                setup->joint_setup[i].joint_type  = (uint32_t)lua_GetScalarField(engine_lua, "joint_type");
                                setup->joint_setup[i].joint_cfm   = lua_GetScalarField(engine_lua, "joint_cfm");
                                setup->joint_setup[i].joint_erp   = lua_GetScalarField(engine_lua, "joint_erp");
                                
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
