/* 
 * File:   engine_bullet.h
 * Author: nTesla
 *
 * Created on July 11, 2015, 1:27 PM
 */

#ifndef ENGINE_PHYSICS_H
#define	ENGINE_PHYSICS_H

#include <stdint.h>


#define DEFAULT_COLLSION_NODE_POOL_SIZE    (128)
// non zero value prevents to "smooth" normales calculations near edges, 
// that is wrong for slide state checking.
#define COLLISION_MARGIN_DEFAULT           (0.0f)


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
    uint16_t                    hit;
    float                       fraction;
    float                       point[3];
    float                       normale[3];
}collision_result_t, *collision_result_p;


typedef struct ghost_shape_s
{
    float bb_min[3];
    float bb_max[3];
}ghost_shape_t, *ghost_shape_p;


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

int  Physics_RayTest(struct collision_result_s *result, float from[3], float to[3], struct engine_container_s *cont, int16_t filter);
int  Physics_RayTestFiltered(struct collision_result_s *result, float from[3], float to[3], struct engine_container_s *cont, int16_t filter);
int  Physics_SphereTest(struct collision_result_s *result, float from[3], float to[3], float R, struct engine_container_s *cont, int16_t filter);

/* Physics object manipulation functions */
int  Physics_IsBodyesInited(struct physics_data_s *physics);
int  Physics_IsGhostsInited(struct physics_data_s *physics);
int  Physics_GetBodiesCount(struct physics_data_s *physics);
void Physics_GetBodyWorldTransform(struct physics_data_s *physics, float tr[16], uint16_t index);
void Physics_SetBodyWorldTransform(struct physics_data_s *physics, float tr[16], uint16_t index);
void Physics_GetGhostWorldTransform(struct physics_data_s *physics, float tr[16], uint16_t index);
void Physics_SetGhostWorldTransform(struct physics_data_s *physics, float tr[16], uint16_t index);
int  Physics_GetGhostPenetrationFixVector(struct physics_data_s *physics, uint16_t index, int16_t filter, float correction[3]);

// Bullet entity rigid body generating.
void Physics_GenRigidBody(struct physics_data_s *physics, struct ss_bone_frame_s *bf);
void Physics_CreateGhosts(struct physics_data_s *physics, struct ss_bone_frame_s *bf, struct ghost_shape_s *boxes);
void Physics_SetGhostCollisionShape(struct physics_data_s *physics, uint16_t index, struct ghost_shape_s *shape_info);
void Physics_GenStaticMeshRigidBody(struct static_mesh_s *smesh);
struct physics_object_s* Physics_GenRoomRigidBody(struct room_s *room, struct room_sector_s *heightmap, uint32_t sectors_count, struct sector_tween_s *tweens, int num_tweens);
void Physics_SetOwnerObject(struct physics_object_s *obj, struct engine_container_s *self);
void Physics_DeleteObject(struct physics_object_s *obj);
void Physics_EnableObject(struct physics_object_s *obj);
void Physics_DisableObject(struct physics_object_s *obj);

void Physics_EnableCollision(struct physics_data_s *physics);
void Physics_DisableCollision(struct physics_data_s *physics);
void Physics_SetCollisionScale(struct physics_data_s *physics, float scaling[3]);
void Physics_SetBodyMass(struct physics_data_s *physics, float mass, uint16_t index);
void Physics_PushBody(struct physics_data_s *physics, float speed[3], uint16_t index);
void Physics_SetLinearFactor(struct physics_data_s *physics, float factor[3], uint16_t index);
struct collision_node_s *Physics_GetCurrentCollisions(struct physics_data_s *physics, int16_t filter);


/* Ragdoll interface */
#define RD_CONSTRAINT_POINT 0
#define RD_CONSTRAINT_HINGE 1
#define RD_CONSTRAINT_CONE  2

#define RD_DEFAULT_SLEEPING_THRESHOLD 10.0

struct rd_setup_s;

bool Ragdoll_Create(struct physics_data_s *physics, struct ss_bone_frame_s *bf, struct rd_setup_s *setup);
bool Ragdoll_Delete(struct physics_data_s *physics);

struct rd_setup_s *Ragdoll_GetSetup(struct lua_State *lua, int ragdoll_index);
void Ragdoll_DeleteSetup(struct rd_setup_s *setup);


/* Hair interface */
#define HAIR_TR1       0
#define HAIR_TR2       1
#define HAIR_TR3       2
#define HAIR_TR4_KID_1 3
#define HAIR_TR4_KID_2 4
#define HAIR_TR4_OLD   5
#define HAIR_TR5_KID_1 6
#define HAIR_TR5_KID_2 7
#define HAIR_TR5_OLD   8

// Maximum amount of joint hair vertices. By default, TR4-5 used four
// vertices for each hair (unused TR1 hair mesh even used three).
// It's hardly possible if anyone will exceed a limit of 8 vertices,
// but if it happens, this should be edited.

#define HAIR_VERTEX_MAP_LIMIT 8

// Since we apply TR4 hair scheme to TR2-3 as well, we need to discard
// polygons which are unused. These are 0 and 5 in both TR2 and TR3.

#define HAIR_DISCARD_ROOT_FACE 0
#define HAIR_DISCARD_TAIL_FACE 5

struct hair_s;
struct hair_setup_s;

// Gets scripted hair set-up to specified hair set-up structure.
struct hair_setup_s *Hair_GetSetup(struct lua_State *lua, uint32_t hair_entry_index);

// Creates hair into allocated hair structure, using previously defined setup and
// entity index.
struct hair_s *Hair_Create(struct hair_setup_s *setup, struct physics_data_s *physics);

// Removes specified hair from entity and clears it from memory.
void Hair_Delete(struct hair_s *hair);

void Hair_Update(struct hair_s *hair, struct physics_data_s *physics);

int Hair_GetElementsCount(struct hair_s *hair);

void Hair_GetElementInfo(struct hair_s *hair, int element, struct base_mesh_s **mesh, float tr[16]);

#endif	/* ENGINE_PHYSICS_H */
