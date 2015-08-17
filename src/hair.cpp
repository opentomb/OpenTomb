
#include <stdlib.h>
#include <math.h>

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include "bullet/btBulletDynamicsCommon.h"
#include "bullet/BulletCollision/CollisionDispatch/btCollisionWorld.h"
#include "bullet/BulletCollision/CollisionShapes/btCollisionShape.h"
#include "bullet/BulletDynamics/ConstraintSolver/btTypedConstraint.h"
#include "bullet/BulletCollision/CollisionDispatch/btGhostObject.h"
#include "bullet/BulletCollision/BroadphaseCollision/btCollisionAlgorithm.h"

#include "engine.h"
#include "entity.h"
#include "character_controller.h"
#include "mesh.h"
#include "world.h"
#include "engine_physics.h"
#include "script.h"
#include "hair.h"


typedef struct physics_data_s
{
    // kinematic
    btRigidBody                       **bt_body;

    // dynamic
    btPairCachingGhostObject          **ghostObjects;           // like Bullet character controller for penetration resolving.
    btManifoldArray                    *manifoldArray;          // keep track of the contact manifolds
    uint16_t                            objects_count;          // Ragdoll joints
    uint16_t                            bt_joint_count;         // Ragdoll joints
    btTypedConstraint                 **bt_joints;              // Ragdoll joints

    struct engine_container_s          *cont;
}physics_data_t, *physics_data_p;


typedef struct hair_element_s
{
    struct base_mesh_s         *mesh;              // Pointer to rendered mesh.
    btCollisionShape           *shape;             // Pointer to collision shape.
    btRigidBody                *body;              // Pointer to dynamic body.
    btGeneric6DofConstraint    *joint;             // Array of joints.
}hair_element_t, *hair_element_p;


typedef struct hair_s
{
    engine_container_p        container;

    entity_p                  owner_char;         // Entity who owns this hair.
    uint32_t                  owner_body;         // Owner entity's body ID.

    uint8_t                   root_index;         // Index of "root" element.
    uint8_t                   tail_index;         // Index of "tail" element.

    uint8_t                   element_count;      // Overall amount of elements.
    hair_element_s           *elements;           // Array of elements.

    uint8_t                   vertex_map_count;
    uint32_t                 *hair_vertex_map;    // Hair vertex indices to link
    uint32_t                 *head_vertex_map;    // Head vertex indices to link

}hair_t, *hair_p;

typedef struct hair_setup_s
{
    uint32_t     model_id;           // Hair model ID
    uint32_t     link_body;          // Lara's head mesh index

    btScalar     root_weight;        // Root and tail hair body weight. Intermediate body
    btScalar     tail_weight;        // weights are calculated from these two parameters

    btScalar     hair_damping[2];    // Damping affects hair "plasticity"
    btScalar     hair_inertia;       // Inertia affects hair "responsiveness"
    btScalar     hair_restitution;   // "Bounciness" of the hair
    btScalar     hair_friction;      // How much other bodies will affect hair trajectory

    btScalar     joint_overlap;      // How much two hair bodies overlap each other

    btScalar     joint_cfm;          // Constraint force mixing (joint softness)
    btScalar     joint_erp;          // Error reduction parameter (joint "inertia")

    btScalar     head_offset[3];     // Linear offset to place hair to
    btScalar     root_angle[3];      // First constraint set angle (to align hair angle)

    uint32_t     vertex_map_count;   // Amount of REAL vertices to link head and hair
    uint32_t     hair_vertex_map[HAIR_VERTEX_MAP_LIMIT]; // Hair vertex indices to link
    uint32_t     head_vertex_map[HAIR_VERTEX_MAP_LIMIT]; // Head vertex indices to link

}hair_setup_t, *hair_setup_p;



extern btDiscreteDynamicsWorld     *bt_engine_dynamicsWorld;
btCollisionShape *BT_CSfromMesh(struct base_mesh_s *mesh, bool useCompression, bool buildBvh, bool is_static);

struct hair_s *Hair_Create(struct hair_setup_s *setup, struct entity_s *parent_entity)
{
    // No setup or parent to link to - bypass function.

    if( (!parent_entity) || (!setup)                           ||
        (setup->link_body >= parent_entity->bf->bone_tag_count)||
        (!(parent_entity->physics->bt_body[setup->link_body])) )
    {
        return NULL;
    }

    skeletal_model_p model = World_GetModelByID(&engine_world, setup->model_id);
    btScalar owner_body_transform[16];

    // No model to link to - bypass function.

    if((!model) || (model->mesh_count == 0))
    {
        return NULL;
    }

    // Setup engine container. FIXME: DOESN'T WORK PROPERLY ATM.

    struct hair_s *hair = (struct hair_s*)calloc(1, sizeof(struct hair_s));
    hair->container = Container_Create();
    hair->container->room = parent_entity->self->room;
    hair->container->object_type = OBJECT_HAIR;
    hair->container->object = hair;

    // Setup initial hair parameters.
    hair->owner_char = parent_entity;       // Entity to refer to.
    hair->owner_body = setup->link_body;    // Entity body to refer to.

    // Setup initial position / angles.

    Mat4_Mat4_mul(owner_body_transform, parent_entity->transform, parent_entity->bf->bone_tags[hair->owner_body].full_transform);
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
        hair->elements[i].joint = NULL;

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

    // If multiple joints per body is specified, joints are placed in circular manner,
    // with obvious step of (SIMD_2_PI) / joint count. It means that all joints will form
    // circle-like figure.

    for(uint16_t i=0; i<hair->element_count; i++)
    {
        btRigidBody* prev_body;
        btScalar     body_length;

        // Each body width and height are used to calculate position of each joint.

        //btScalar body_width = fabs(hair->elements[i].mesh->bb_max[0] - hair->elements[i].mesh->bb_min[0]);
        //btScalar body_depth = fabs(hair->elements[i].mesh->bb_max[3] - hair->elements[i].mesh->bb_min[3]);

        btTransform localA; localA.setIdentity();
        btTransform localB; localB.setIdentity();

        btScalar joint_x = 0.0;
        btScalar joint_y = 0.0;

        if(i == 0)  // First joint group
        {
            // Adjust pivot point A to parent body.
            localA.setOrigin(btVector3(setup->head_offset[0] + joint_x, setup->head_offset[1], setup->head_offset[2] + joint_y));
            localA.getBasis().setEulerZYX(setup->root_angle[0], setup->root_angle[1], setup->root_angle[2]);

            localB.setOrigin(btVector3(joint_x, 0.0, joint_y));
            localB.getBasis().setEulerZYX(0,-SIMD_HALF_PI,0);

            prev_body = parent_entity->physics->bt_body[hair->owner_body];      // Previous body is parent body.
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

        hair->elements[i].joint = new btGeneric6DofConstraint(*prev_body, *(hair->elements[i].body), localA, localB, true);

        // CFM and ERP parameters are critical for making joint "hard" and link
        // to Lara's head. With wrong values, constraints may become "elastic".

        for(int axis=0;axis<=5;axis++)
        {
            hair->elements[i].joint->setParam(BT_CONSTRAINT_STOP_CFM, setup->joint_cfm, axis);
            hair->elements[i].joint->setParam(BT_CONSTRAINT_STOP_ERP, setup->joint_erp, axis);
        }

        if(i == 0)
        {
            // First joint group should be more limited in motion, as it is connected
            // right to the head. NB: Should we make it scriptable as well?

            hair->elements[i].joint->setLinearLowerLimit(btVector3(0., 0., 0.));
            hair->elements[i].joint->setLinearUpperLimit(btVector3(0., 0., 0.));
            hair->elements[i].joint->setAngularLowerLimit(btVector3(-SIMD_HALF_PI,     0., -SIMD_HALF_PI*0.4));
            hair->elements[i].joint->setAngularUpperLimit(btVector3(-SIMD_HALF_PI*0.3, 0.,  SIMD_HALF_PI*0.4));

            // Increased solver iterations make constraint even more stable.

            hair->elements[i].joint->setOverrideNumSolverIterations(100);
        }
        else
        {
            // Normal joint with more movement freedom.

            hair->elements[i].joint->setLinearLowerLimit(btVector3(0., 0., 0.));
            hair->elements[i].joint->setLinearUpperLimit(btVector3(0., 0., 0.));
            hair->elements[i].joint->setAngularLowerLimit(btVector3(-SIMD_HALF_PI*0.5, 0., -SIMD_HALF_PI*0.5));
            hair->elements[i].joint->setAngularUpperLimit(btVector3( SIMD_HALF_PI*0.5, 0.,  SIMD_HALF_PI*0.5));
        }

        hair->elements[i].joint->setDbgDrawSize(btScalar(5.f));    // Draw constraint axes.

        // Add constraint to the world.
        bt_engine_dynamicsWorld->addConstraint(hair->elements[i].joint, true);
    }

    return hair;
}

void Hair_Delete(struct hair_s *hair)
{
    if(hair)
    {
        for(int i=0; i<hair->element_count; i++)
        {
            if(hair->elements[i].joint)
            {
                bt_engine_dynamicsWorld->removeConstraint(hair->elements[i].joint);
                delete hair->elements[i].joint;
                hair->elements[i].joint = NULL;
            }
        }

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

        free(hair->container);
        hair->container = NULL;

        hair->owner_char = NULL;
        hair->owner_body = 0;

        hair->root_index = 0;
        hair->tail_index = 0;
        free(hair);
    }
}

void Hair_Update(struct entity_s *entity)
{
    if((!IsCharacter(entity)) || (entity->character->hair_count == 0))
    {
        return;
    }

    for(int i=0; i<entity->character->hair_count; i++)
    {
        hair_p hair = entity->character->hairs[i];
        if(hair && (hair->element_count > 0))
        {
            /*btScalar new_transform[16];

            Mat4_Mat4_mul(new_transform, entity->transform, entity->bf->bone_tags[hair->owner_body].full_transform);

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
}

struct hair_setup_s *Hair_GetSetup(struct lua_State *lua, uint32_t hair_entry_index)
{
    struct hair_setup_s *hair_setup = NULL;

    int top = lua_gettop(lua);

    lua_getglobal(lua, "getHairSetup");
    if(lua_isfunction(lua, -1))
    {
        lua_pushinteger(lua, hair_entry_index);
        if(lua_CallAndLog(lua, 1, 1, 0))
        {
            if(lua_istable(lua, -1))
            {
                hair_setup = (struct hair_setup_s*)malloc(sizeof(struct hair_setup_s));
                hair_setup->model_id            = (uint32_t)lua_GetScalarField(lua, "model");
                hair_setup->link_body           = (uint32_t)lua_GetScalarField(lua, "link_body");
                hair_setup->vertex_map_count    = (uint32_t)lua_GetScalarField(lua, "v_count");

                lua_getfield(lua, -1, "props");
                if(lua_istable(lua, -1))
                {
                    hair_setup->root_weight      = lua_GetScalarField(lua, "root_weight");
                    hair_setup->tail_weight      = lua_GetScalarField(lua, "tail_weight");
                    hair_setup->hair_inertia     = lua_GetScalarField(lua, "hair_inertia");
                    hair_setup->hair_friction    = lua_GetScalarField(lua, "hair_friction");
                    hair_setup->hair_restitution = lua_GetScalarField(lua, "hair_bouncing");
                    hair_setup->joint_overlap    = lua_GetScalarField(lua, "joint_overlap");
                    hair_setup->joint_cfm        = lua_GetScalarField(lua, "joint_cfm");
                    hair_setup->joint_erp        = lua_GetScalarField(lua, "joint_erp");

                    lua_getfield(lua, -1, "hair_damping");
                    if(lua_istable(lua, -1))
                    {
                        hair_setup->hair_damping[0] = lua_GetScalarField(lua, 1);
                        hair_setup->hair_damping[1] = lua_GetScalarField(lua, 2);
                    }
                    lua_pop(lua, 1);
                }
                else
                {
                    free(hair_setup);
                    hair_setup = NULL;
                }
                lua_pop(lua, 1);

                lua_getfield(lua, -1, "v_index");
                if(lua_istable(lua, -1))
                {
                    for(int i=1; i<=hair_setup->vertex_map_count; i++)
                    {
                        hair_setup->head_vertex_map[i-1] = (uint32_t)lua_GetScalarField(lua, i);
                    }
                }
                else
                {
                    free(hair_setup);
                    hair_setup = NULL;
                }
                lua_pop(lua, 1);

                lua_getfield(lua, -1, "offset");
                if(lua_istable(lua, -1))
                {
                    hair_setup->head_offset[0] = lua_GetScalarField(lua, 1);
                    hair_setup->head_offset[1] = lua_GetScalarField(lua, 2);
                    hair_setup->head_offset[2] = lua_GetScalarField(lua, 3);
                }
                else
                {
                    free(hair_setup);
                    hair_setup = NULL;
                }
                lua_pop(lua, 1);

                lua_getfield(lua, -1, "root_angle");
                if(lua_istable(lua, -1))
                {
                    hair_setup->root_angle[0] = lua_GetScalarField(lua, 1);
                    hair_setup->root_angle[1] = lua_GetScalarField(lua, 2);
                    hair_setup->root_angle[2] = lua_GetScalarField(lua, 3);
                }
                else
                {
                    free(hair_setup);
                    hair_setup = NULL;
                }
                lua_pop(lua, 1);
            }
        }
    }
    lua_settop(lua, top);

    return hair_setup;
}

int Hair_GetElementsCount(struct hair_s *hair)
{
    return (hair)?(hair->element_count):(0);
}

int Hair_GetElementInfo(struct hair_s *hair, int element, struct base_mesh_s **mesh, float tr[16])
{
#ifndef BT_USE_DOUBLE_PRECISION
    hair->elements[element].body->getWorldTransform().getOpenGLMatrix(tr);
#else
    btScalar btTr[16];
    hair->elements[element].body->getWorldTransform().getOpenGLMatrix(btTr);
    for(int i = 0; i < 16; i++)
    {
        tr[i] = btTr[i];
    }
#endif
    *mesh = hair->elements[element].mesh;
}
