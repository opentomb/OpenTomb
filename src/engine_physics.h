/* 
 * File:   engine_bullet.h
 * Author: nTesla
 *
 * Created on July 11, 2015, 1:27 PM
 */

#ifndef ENGINE_BULLET_H
#define	ENGINE_BULLET_H

#include <stdint.h>

struct engine_container_s;
struct entity_s;

#define MAX_OBJECTS_IN_COLLSION_NODE    (4)

typedef struct collision_result_s
{
    struct engine_container_s  *obj;
    uint16_t                    bone_num;
    btScalar                    point[3];
    btScalar                    normale[3];
}collision_result_t, *collision_result_p;


/***** BEGIN INTERNAL STRUCTURES: HIDE *****/
#include "bullet/btBulletDynamicsCommon.h"
#include "bullet/BulletCollision/CollisionDispatch/btCollisionWorld.h"
#include "bullet/BulletCollision/CollisionShapes/btCollisionShape.h"
#include "bullet/BulletDynamics/ConstraintSolver/btTypedConstraint.h"
#include "bullet/BulletCollision/CollisionDispatch/btGhostObject.h"
#include "bullet/BulletCollision/BroadphaseCollision/btCollisionAlgorithm.h"

struct entity_s;
struct engine_container_s;

class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;
class btPairCachingGhostObject;

class btCollisionShape;
class btRigidBody;

extern btDefaultCollisionConfiguration         *bt_engine_collisionConfiguration;
extern btCollisionDispatcher                   *bt_engine_dispatcher;
extern btBroadphaseInterface                   *bt_engine_overlappingPairCache;
extern btSequentialImpulseConstraintSolver     *bt_engine_solver ;
extern btDiscreteDynamicsWorld                 *bt_engine_dynamicsWorld;



typedef struct entity_collision_node_s
{
    uint16_t                    obj_count;
    btCollisionObject          *obj[MAX_OBJECTS_IN_COLLSION_NODE];
}entity_collision_node_t, *entity_collision_node_p;


typedef struct physics_data_s
{
    // kinematic
    btCollisionShape                  **shapes;
    btRigidBody                       **bt_body;
    
    // dynamic
    uint32_t                            no_fix_skeletal_parts;
    int8_t                              no_fix_all;
    btPairCachingGhostObject          **ghostObjects;           // like Bullet character controller for penetration resolving.
    btManifoldArray                    *manifoldArray;          // keep track of the contact manifolds
    uint16_t                            objects_count;          // Ragdoll joints
    uint16_t                            bt_joint_count;         // Ragdoll joints
    btTypedConstraint                 **bt_joints;              // Ragdoll joints
    
    struct entity_collision_node_s     *last_collisions;
}physics_data_t, *physics_data_p;
/***** END INTERNAL STRUCTURES: HIDE *****/

/* Common physics functions */
void Physics_Init();
void Physics_Destroy();

struct physics_data_s *Physics_CreatePhysicsData();
void Physics_DeletePhysicsData(struct physics_data_s *physics);

void Physics_GetGravity(btScalar g[3]);
void Physics_SetGravity(btScalar g[3]);

int  Physics_RayTest(struct collision_result_s *result, btScalar from[3], btScalar to[3], struct engine_container_s *cont);
int  Physics_SphereTest(struct collision_result_s *result, btScalar from[3], btScalar to[3], btScalar R, struct engine_container_s *cont);

/* bullet collision model calculation */
btCollisionShape* BT_CSfromBBox(btScalar *bb_min, btScalar *bb_max);
btCollisionShape* BT_CSfromMesh(struct base_mesh_s *mesh, bool useCompression, bool buildBvh, bool is_static = true);
btCollisionShape* BT_CSfromHeightmap(struct room_sector_s *heightmap, struct sector_tween_s *tweens, int tweens_size, bool useCompression, bool buildBvh);

// Bullet entity rigid body generating.
void BT_GenEntityRigidBody(struct entity_s *ent);
void Entity_CreateGhosts(struct entity_s *ent);
void Entity_GhostUpdate(struct entity_s *ent);
void Entity_UpdateCurrentCollisions(struct entity_s *ent);
int  Entity_GetPenetrationFixVector(struct entity_s *ent, btScalar reaction[3], btScalar move_global[3]);
void Entity_FixPenetrations(struct entity_s *ent, btScalar move[3]);
int  Entity_CheckNextPenetration(struct entity_s *ent, btScalar move[3]);
void Entity_CheckCollisionCallbacks(struct entity_s *ent);
bool Entity_WasCollisionBodyParts(struct entity_s *ent, uint32_t parts_flags);
void Entity_CleanCollisionAllBodyParts(struct entity_s *ent);
void Entity_CleanCollisionBodyParts(struct entity_s *ent, uint32_t parts_flags);
btCollisionObject *Entity_GetRemoveCollisionBodyParts(struct entity_s *ent, uint32_t parts_flags, uint32_t *curr_flag);


#endif	/* ENGINE_BULLET_H */
