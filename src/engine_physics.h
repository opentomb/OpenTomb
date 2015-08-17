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
    float                       point[3];
    float                       normale[3];
}collision_result_t, *collision_result_p;


struct entity_s;
struct engine_container_s;
struct physics_data_s;
struct physics_object_s;

/* Common physics functions */
void Physics_Init();
void Physics_Destroy();
void Physics_StepSimulation(float time);
void Physics_DebugDrawWorld();
void Physics_CleanUpObjects();

struct physics_data_s *Physics_CreatePhysicsData(struct engine_container_s *cont);
void Physics_DeletePhysicsData(struct physics_data_s *physics);

void Physics_GetGravity(float g[3]);
void Physics_SetGravity(float g[3]);

int  Physics_RayTest(struct collision_result_s *result, float from[3], float to[3], struct engine_container_s *cont);
int  Physics_SphereTest(struct collision_result_s *result, float from[3], float to[3], float R, struct engine_container_s *cont);

/* Physics object manipulation functions */
int  Physics_IsBodyesInited(struct physics_data_s *physics);
int  Physics_IsGhostsInited(struct physics_data_s *physics);
void Physics_GetBodyWorldTransform(struct physics_data_s *physics, float tr[16], uint16_t index);
void Physics_SetBodyWorldTransform(struct physics_data_s *physics, float tr[16], uint16_t index);
void Physics_GetGhostWorldTransform(struct physics_data_s *physics, float tr[16], uint16_t index);
void Physics_SetGhostWorldTransform(struct physics_data_s *physics, float tr[16], uint16_t index);
int  Physics_GetGhostPenetrationFixVector(struct physics_data_s *physics, uint16_t index, float correction[3]);

// Bullet entity rigid body generating.
void Physics_GenRigidBody(struct physics_data_s *physics, struct ss_bone_frame_s *bf, float transform[16]);
void Physics_CreateGhosts(struct physics_data_s *physics, struct ss_bone_frame_s *bf, float transform[16]);
void Physics_GenStaticMeshRigidBody(struct static_mesh_s *smesh);
void Physics_GenRoomRigidBody(struct room_s *room, struct sector_tween_s *tweens, int num_tweens);
void Physics_DeleteObject(struct physics_object_s *obj);
void Physics_EnableObject(struct physics_object_s *obj);
void Physics_DisableObject(struct physics_object_s *obj);

void Physics_EnableCollision(struct physics_data_s *physics);
void Physics_DisableCollision(struct physics_data_s *physics);
void Physics_SetCollisionScale(struct physics_data_s *physics, float scaling[3]);
void Physics_SetBodyMass(struct physics_data_s *physics, float mass, uint16_t index);
void Physics_PushBody(struct physics_data_s *physics, float speed[3], uint16_t index);
void Physics_SetLinearFactor(struct physics_data_s *physics, float factor[3], uint16_t index);
struct collision_node_s *Physics_GetCurrentCollisions(struct physics_data_s *physics);


#endif	/* ENGINE_PHYSICS_H */
