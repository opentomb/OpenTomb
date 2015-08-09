/* 
 * File:   engine_bullet.h
 * Author: nTesla
 *
 * Created on July 11, 2015, 1:27 PM
 */

#ifndef ENGINE_PHYSICS_H
#define	ENGINE_PHYSICS_H

#include <stdint.h>

struct engine_container_s;
struct entity_s;

#define DEFAULT_COLLSION_NODE_POOL_SIZE    (128)

typedef struct collision_node_s
{
    uint16_t                    part_from;
    uint16_t                    part_self;
    struct engine_container_s  *obj;
    struct collision_node_s    *next;
}collision_node_t, *collision_node_p;


typedef struct collision_result_s
{
    struct engine_container_s  *obj;
    uint16_t                    bone_num;
    btScalar                    point[3];
    btScalar                    normale[3];
}collision_result_t, *collision_result_p;


struct entity_s;
struct engine_container_s;
struct physics_data_s;
struct physics_object_s;

/* Common physics functions */
void Physics_Init();
void Physics_Destroy();
void Physics_StepSimulation(btScalar time);
void Physics_CleanUpObjects();

struct physics_data_s *Physics_CreatePhysicsData();
void Physics_DeletePhysicsData(struct physics_data_s *physics);

void Physics_GetGravity(btScalar g[3]);
void Physics_SetGravity(btScalar g[3]);

int  Physics_RayTest(struct collision_result_s *result, btScalar from[3], btScalar to[3], struct engine_container_s *cont);
int  Physics_SphereTest(struct collision_result_s *result, btScalar from[3], btScalar to[3], btScalar R, struct engine_container_s *cont);

// Bullet entity rigid body generating.
void Physics_GenEntityRigidBody(struct entity_s *ent);
void Physics_GenStaticMeshRigidBody(struct static_mesh_s *smesh);
void Physics_GenRoomRigidBody(struct room_s *room, struct sector_tween_s *tweens, int num_tweens);
void Physics_DeleteObject(struct physics_object_s *obj);
void Physics_EnableObject(struct physics_object_s *obj);
void Physics_DisableObject(struct physics_object_s *obj);

void Entity_EnableCollision(struct entity_s *ent);
void Entity_DisableCollision(struct entity_s *ent);
void Entity_SetCollisionScale(struct entity_s *ent);
void Entity_SetBodyMass(struct entity_s *ent, btScalar mass, uint16_t index);
void Entity_PushBody(struct entity_s *ent, btScalar speed[3], uint16_t index);
void Entity_SetLinearFactor(struct entity_s *ent, btScalar factor[3], uint16_t index);
void Entity_SetNoFixBodyPartFlag(struct entity_s *ent, uint32_t flag);
void Entity_SetNoFixAllFlag(struct entity_s *ent, uint8_t flag);
uint8_t Entity_GetNoFixAllFlag(struct entity_s *ent);
void Entity_CreateGhosts(struct entity_s *ent);
void Entity_GhostUpdate(struct entity_s *ent);
void Entity_UpdateCurrentCollisions(struct entity_s *ent);
int  Entity_GetPenetrationFixVector(struct entity_s *ent, btScalar reaction[3], btScalar move_global[3]);
void Entity_FixPenetrations(struct entity_s *ent, btScalar move[3]);
int  Entity_CheckNextPenetration(struct entity_s *ent, btScalar move[3]);
void Entity_UpdateRigidBody(struct entity_s *ent, int force);
bool Entity_WasCollisionBodyParts(struct entity_s *ent, uint32_t parts_flags);
void Entity_CleanCollisionAllBodyParts(struct entity_s *ent);
void Entity_CleanCollisionBodyParts(struct entity_s *ent, uint32_t parts_flags);
collision_node_p Entity_GetRemoveCollisionBodyParts(struct entity_s *ent, uint32_t parts_flags, uint32_t *curr_flag);


#endif	/* ENGINE_PHYSICS_H */
