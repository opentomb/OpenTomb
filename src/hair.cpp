
#include <math.h>
#include "bullet/LinearMath/btScalar.h"
#include "hair.h"

bool Hair_Create(hair_p hair, hair_setup_p setup, entity_p parent_entity)
{
    // No setup or parent to link to - bypass function.

    if( (!parent_entity) || (!setup)                           ||
        (setup->link_body >= parent_entity->bf.bone_tag_count) ||
        (!(parent_entity->bt_body[setup->link_body]))           ) return false;

    skeletal_model_p model = World_GetModelByID(&engine_world, setup->model);
    btScalar owner_body_transform[16];

    // No model to link to - bypass function.

    if((!model) || (model->mesh_count == 0)) return false;

    // Setup engine container. FIXME: DOESN'T WORK PROPERLY ATM.

    hair->container = Container_Create();
    hair->container->room = parent_entity->self->room;
    hair->container->object_type = OBJECT_HAIR;
    hair->container->object = hair;

    // Setup initial hair parameters.

    hair->owner_char = parent_entity;       // Entity to refer to.
    hair->owner_body = setup->link_body;    // Entity body to refer to.

    // Setup initial position / angles.

    Mat4_Mat4_mul(owner_body_transform, parent_entity->transform, parent_entity->bf.bone_tags[hair->owner_body].full_transform);
    // Number of elements (bodies) is equal to number of hair meshes.

    hair->element_count = model->mesh_count;
    hair->elements      = (hair_element_p)calloc(hair->element_count, sizeof(hair_element_t));

    // Root index should be always zero, as it is how engine determines that it is
    // connected to head and renders it properly. Tail index should be always the
    // last element of the hair, as it indicates absence of "child" constraint.

    hair->root_index = 0;
    hair->tail_index = hair->element_count-1;

    // Weight step is needed to determine the weight of each hair body.
    // It is derived from root body weight and tail body weight.

    btScalar weight_step = ((setup->root_weight - setup->tail_weight) / hair->element_count);
    btScalar current_weight = setup->root_weight;

    for(uint8_t i=0;i<hair->element_count;i++)
    {
        // Point to corresponding mesh.

        hair->elements[i].mesh = model->mesh_tree[i].mesh_base;

        // Begin creating ACTUAL physical hair mesh.

        btVector3   localInertia(0, 0, 0);
        btTransform startTransform;

        // Make collision shape out of mesh.

        hair->elements[i].shape = BT_CSfromMesh(hair->elements[i].mesh, true, true, false);
        hair->elements[i].shape->calculateLocalInertia((current_weight * setup->hair_inertia), localInertia);

        // Decrease next body weight to weight_step parameter.

        current_weight -= weight_step;

        // Initialize motion state for body.

        startTransform.setFromOpenGLMatrix(owner_body_transform);
        btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);

        // Make rigid body.

        hair->elements[i].body = new btRigidBody(current_weight, motionState, hair->elements[i].shape, localInertia);

        // Damping makes body stop in space by itself, to prevent it from continous movement.

        hair->elements[i].body->setDamping(setup->hair_damping[0], setup->hair_damping[1]);

        // Restitution and friction parameters define "bounciness" and "dullness" of hair.

        hair->elements[i].body->setRestitution(setup->hair_restitution);
        hair->elements[i].body->setFriction(setup->hair_friction);

        // Since hair is always moving with Lara, even if she's in still state (like, hanging
        // on a ledge), hair bodies shouldn't deactivate over time.

        hair->elements[i].body->forceActivationState(DISABLE_DEACTIVATION);

        // Hair bodies must not collide with each other, and also collide ONLY with kinematic
        // bodies (e. g. animated meshes), or else Lara's ghost object or anything else will be able to
        // collide with hair!

        hair->elements[i].body->setUserPointer(hair->container);
        bt_engine_dynamicsWorld->addRigidBody(hair->elements[i].body, COLLISION_GROUP_CHARACTERS, COLLISION_GROUP_KINEMATIC);

        hair->elements[i].body->activate();
    }

    // GENERATE CONSTRAINTS.
    // All constraints are generic 6-DOF type, as they seem perfect fit for hair.

    // Joint count is calculated from overall body amount multiplied by per-body constraint
    // count.

    hair->joint_count = hair->element_count;
    hair->joints      = (btGeneric6DofConstraint**)calloc(sizeof(btGeneric6DofConstraint*), hair->joint_count);

    // If multiple joints per body is specified, joints are placed in circular manner,
    // with obvious step of (SIMD_2_PI) / joint count. It means that all joints will form
    // circle-like figure.

    int curr_joint = 0;

    for(uint16_t i=0; i<hair->element_count; i++)
    {
        btRigidBody* prev_body;
        btScalar     body_length;

        // Each body width and height are used to calculate position of each joint.

        //btScalar body_width = fabs(hair->elements[i].mesh->bb_max[0] - hair->elements[i].mesh->bb_min[0]);
        //btScalar body_depth = fabs(hair->elements[i].mesh->bb_max[3] - hair->elements[i].mesh->bb_min[3]);

        btTransform localA; localA.setIdentity();
        btTransform localB; localB.setIdentity();

        //btScalar d       = 0.0; // Current "circle" position.
        btScalar joint_x = 0.0;
        btScalar joint_y = 0.0;

        if(i == 0)  // First joint group
        {
            // Adjust pivot point A to parent body.

            localA.setOrigin(setup->head_offset + btVector3(joint_x, 0.0, joint_y));
            localA.getBasis().setEulerZYX(setup->root_angle[0], setup->root_angle[1], setup->root_angle[2]);

            localB.setOrigin(btVector3(joint_x, 0.0, joint_y));
            localB.getBasis().setEulerZYX(0,-SIMD_HALF_PI,0);

            prev_body = parent_entity->bt_body[hair->owner_body];   // Previous body is parent body.
        }
        else
        {
            // Adjust pivot point A to previous mesh's length, considering mesh overlap multiplier.

            body_length = fabs(hair->elements[i-1].mesh->bb_max[1] - hair->elements[i-1].mesh->bb_min[1]) * setup->joint_overlap;

            localA.setOrigin(btVector3(joint_x, body_length, joint_y));
            localA.getBasis().setEulerZYX(0,SIMD_HALF_PI,0);

            // Pivot point B is automatically adjusted by Bullet.

            localB.setOrigin(btVector3(joint_x, 0.0, joint_y));
            localB.getBasis().setEulerZYX(0,SIMD_HALF_PI,0);

            prev_body = hair->elements[i-1].body;   // Previous body is preceiding hair mesh.
        }

        // Create 6DOF constraint.

        hair->joints[curr_joint] = new btGeneric6DofConstraint(*prev_body, *(hair->elements[i].body), localA, localB, true);

        // CFM and ERP parameters are critical for making joint "hard" and link
        // to Lara's head. With wrong values, constraints may become "elastic".

        for(int axis=0;axis<=5;axis++)
        {
            hair->joints[i]->setParam(BT_CONSTRAINT_STOP_CFM, setup->joint_cfm, axis);
            hair->joints[i]->setParam(BT_CONSTRAINT_STOP_ERP, setup->joint_erp, axis);
        }

        if(i == 0)
        {
            // First joint group should be more limited in motion, as it is connected
            // right to the head. NB: Should we make it scriptable as well?

            hair->joints[curr_joint]->setLinearLowerLimit(btVector3(0., 0., 0.));
            hair->joints[curr_joint]->setLinearUpperLimit(btVector3(0., 0., 0.));
            hair->joints[curr_joint]->setAngularLowerLimit(btVector3(-SIMD_HALF_PI,     0., -SIMD_HALF_PI*0.4));
            hair->joints[curr_joint]->setAngularUpperLimit(btVector3(-SIMD_HALF_PI*0.3, 0.,  SIMD_HALF_PI*0.4));

            // Increased solver iterations make constraint even more stable.

            hair->joints[curr_joint]->setOverrideNumSolverIterations(100);
        }
        else
        {
            // Normal joint with more movement freedom.

            hair->joints[curr_joint]->setLinearLowerLimit(btVector3(0., 0., 0.));
            hair->joints[curr_joint]->setLinearUpperLimit(btVector3(0., 0., 0.));
            hair->joints[curr_joint]->setAngularLowerLimit(btVector3(-SIMD_HALF_PI*0.5, 0., -SIMD_HALF_PI*0.5));
            hair->joints[curr_joint]->setAngularUpperLimit(btVector3( SIMD_HALF_PI*0.5, 0.,  SIMD_HALF_PI*0.5));

        }

        hair->joints[curr_joint]->setDbgDrawSize(btScalar(5.f));    // Draw constraint axes.

        // Add constraint to the world.

        bt_engine_dynamicsWorld->addConstraint(hair->joints[curr_joint], true);

        curr_joint++;   // Point to the next joint.
    }

    return true;
}

void Hair_Clear(hair_p hair)
{
    for(int i=0; i<hair->element_count; i++)
    {
        if(hair->elements[i].body)
        {
            hair->elements[i].body->setUserPointer(NULL);
            bt_engine_dynamicsWorld->removeRigidBody(hair->elements[i].body);
            delete hair->elements[i].body;
            hair->elements[i].body = NULL;
        }
        if(hair->elements[i].shape)
        {
            delete hair->elements[i].shape;
            hair->elements[i].shape = NULL;
        }
    }
    free(hair->elements);
    hair->elements = NULL;
    hair->element_count = 0;

    for(int i=0; i<hair->joint_count; i++)
    {
        if(hair->joints[i])
        {
            bt_engine_dynamicsWorld->removeConstraint(hair->joints[i]);
            delete hair->joints[i];
            hair->joints[i] = NULL;
        }
    }
    free(hair->joints);
    hair->joints = NULL;
    hair->joint_count = 0;

    free(hair->container);
    hair->container = NULL;

    hair->owner_char = NULL;
    hair->owner_body = 0;

    hair->root_index = 0;
    hair->tail_index = 0;
}

void Hair_Update(entity_p entity)
{
    if((!IsCharacter(entity)) || (entity->character->hair_count == 0)) return;

    hair_p hair = entity->character->hairs;

    for(int i=0; i<entity->character->hair_count; i++, hair++)
    {
        if((!hair) || (hair->element_count < 1)) continue;

        /*btScalar new_transform[16];

        Mat4_Mat4_mul(new_transform, entity->transform, entity->bf.bone_tags[hair->owner_body].full_transform);

        // Calculate mixed velocities.
        btVector3 mix_vel(new_transform[12+0] - hair->owner_body_transform[12+0],
                          new_transform[12+1] - hair->owner_body_transform[12+1],
                          new_transform[12+2] - hair->owner_body_transform[12+2]);
        mix_vel *= 1.0 / engine_frame_time;

        if(0)
        {
            btScalar sub_tr[16];
            btTransform ang_tr;
            btVector3 mix_ang;
            Mat4_inv_Mat4_affine_mul(sub_tr, hair->owner_body_transform, new_transform);
            ang_tr.setFromOpenGLMatrix(sub_tr);
            ang_tr.getBasis().getEulerYPR(mix_ang.m_floats[2], mix_ang.m_floats[1], mix_ang.m_floats[0]);
            mix_ang *= 1.0 / engine_frame_time;

            // Looks like angular velocity breaks up constraints on VERY fast moves,
            // like mid-air turn. Probably, I've messed up with multiplier value...

            hair->elements[hair->root_index].body->setAngularVelocity(mix_ang);
            hair->owner_char->bt_body[hair->owner_body]->setAngularVelocity(mix_ang);
        }
        Mat4_Copy(hair->owner_body_transform, new_transform);*/

        // Set mixed velocities to both parent body and first hair body.

        //hair->elements[hair->root_index].body->setLinearVelocity(mix_vel);
        //hair->owner_char->bt_body[hair->owner_body]->setLinearVelocity(mix_vel);

        /*mix_vel *= -10.0;                                                     ///@FIXME: magick speed coefficient (force air hair friction!);
        for(int j=0;j<hair->element_count;j++)
        {
            hair->elements[j].body->applyCentralForce(mix_vel);
        }*/

        hair->container->room = hair->owner_char->self->room;
    }
}

bool Hair_GetSetup(uint32_t hair_entry_index, hair_setup_p hair_setup)
{
    bool result = true;

    int top = lua_gettop(engine_lua);
    assert(top >= 0);

    lua_getglobal(engine_lua, "getHairSetup");
    if(lua_isfunction(engine_lua, -1))
    {
        lua_pushinteger(engine_lua, hair_entry_index);
        if(lua_CallAndLog(engine_lua, 1, 1, 0))
        {
            if(lua_istable(engine_lua, -1))
            {
                hair_setup->model               = (uint32_t)lua_GetScalarField(engine_lua, "model");
                hair_setup->link_body           = (uint32_t)lua_GetScalarField(engine_lua, "link_body");
                hair_setup->vertex_map_count    = (uint32_t)lua_GetScalarField(engine_lua, "v_count");

                lua_getfield(engine_lua, -1, "props");
                if(lua_istable(engine_lua, -1))
                {
                    hair_setup->root_weight      = lua_GetScalarField(engine_lua, "root_weight");
                    hair_setup->tail_weight      = lua_GetScalarField(engine_lua, "tail_weight");
                    hair_setup->hair_inertia     = lua_GetScalarField(engine_lua, "hair_inertia");
                    hair_setup->hair_friction    = lua_GetScalarField(engine_lua, "hair_friction");
                    hair_setup->hair_restitution = lua_GetScalarField(engine_lua, "hair_bouncing");
                    hair_setup->joint_radius     = lua_GetScalarField(engine_lua, "joint_radius");
                    hair_setup->joint_overlap    = lua_GetScalarField(engine_lua, "joint_overlap");
                    hair_setup->joint_cfm        = lua_GetScalarField(engine_lua, "joint_cfm");
                    hair_setup->joint_erp        = lua_GetScalarField(engine_lua, "joint_erp");

                    lua_getfield(engine_lua, -1, "hair_damping");
                    if(lua_istable(engine_lua, -1))
                    {
                        hair_setup->hair_damping[0] = lua_GetScalarField(engine_lua, 1);
                        hair_setup->hair_damping[1] = lua_GetScalarField(engine_lua, 2);
                    }
                    lua_pop(engine_lua, 1);
                }
                else { result = false; }
                lua_pop(engine_lua, 1);

                lua_getfield(engine_lua, -1, "v_index");
                if(lua_istable(engine_lua, -1))
                {
                    for(int i=1; i<=hair_setup->vertex_map_count; i++)
                    {
                        hair_setup->head_vertex_map[i-1] = (uint32_t)lua_GetScalarField(engine_lua, i);
                    }
                }
                else { result = false; }
                lua_pop(engine_lua, 1);

                lua_getfield(engine_lua, -1, "offset");
                if(lua_istable(engine_lua, -1))
                {
                    hair_setup->head_offset.m_floats[0] = lua_GetScalarField(engine_lua, 1);
                    hair_setup->head_offset.m_floats[1] = lua_GetScalarField(engine_lua, 2);
                    hair_setup->head_offset.m_floats[2] = lua_GetScalarField(engine_lua, 3);
                }
                else { result = false; }
                lua_pop(engine_lua, 1);

                lua_getfield(engine_lua, -1, "root_angle");
                if(lua_istable(engine_lua, -1))
                {
                    hair_setup->root_angle[0] = lua_GetScalarField(engine_lua, 1);
                    hair_setup->root_angle[1] = lua_GetScalarField(engine_lua, 2);
                    hair_setup->root_angle[2] = lua_GetScalarField(engine_lua, 3);
                }
                else { result = false; }
                lua_pop(engine_lua, 1);
            }
            else { result = false; }
        }
        else { result = false; }
    }
    else { result = false; }

    lua_settop(engine_lua, top);
    return result;
}
