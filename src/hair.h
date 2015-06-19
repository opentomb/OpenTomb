#ifndef HAIR_H
#define HAIR_H

#include <assert.h>
#include <stdint.h>

extern "C" {
#include "lua/lua.h"
#include "lua/lualib.h"
#include "lua/lauxlib.h"
}

#include "bullet/LinearMath/btScalar.h"
#include "bullet/LinearMath/btVector3.h"
#include "bullet/BulletDynamics/Dynamics/btRigidBody.h"

#include "character_controller.h"
#include "engine.h"
#include "entity.h"
#include "game.h"
#include "mesh.h"
#include "script.h"
#include "vmath.h"
#include "world.h"

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

typedef struct hair_element_s
{
    base_mesh_s        *mesh;           // Pointer to rendered mesh.
    btCollisionShape   *shape;          // Pointer to collision shape.
    btRigidBody        *body;           // Pointer to dynamic body.
    btScalar           position[3];     // Position of this hair element
    // relative to the model (NOT the parent!). Should be a matrix in theory,
    // but since this never has a rotation part, we can save a few bytes here.
}hair_element_t, *hair_element_p;

typedef struct hair_s
{
    engine_container_p        container;

    entity_p                  owner_char;         // Entity who owns this hair.
    uint32_t                  owner_body;         // Owner entity's body ID.
    btScalar                  owner_body_hair_root[16] __attribute__((aligned(16))); // transform from owner body to root of hair start

    uint8_t                   root_index;         // Index of "root" element.
    uint8_t                   tail_index;         // Index of "tail" element.

    uint8_t                   element_count;      // Overall amount of elements.
    hair_element_s           *elements;           // Array of elements.

    uint8_t                   joint_count;        // Overall amount of joints.
    btGeneric6DofConstraint **joints;             // Array of joints.

    uint8_t                   vertex_map_count;
    uint32_t                 *hair_vertex_map;    // Hair vertex indices to link
    uint32_t                 *head_vertex_map;    // Head vertex indices to link

    struct base_mesh_s       *mesh;               // Mesh containing all vertices of all parts of this hair object.
}hair_t, *hair_p;

typedef struct hair_setup_s
{
    uint32_t     model;              // Hair model ID
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

    btVector3    head_offset;        // Linear offset to place hair to
    btScalar     root_angle[3];      // First constraint set angle (to align hair angle)

    uint32_t     vertex_map_count;   // Amount of REAL vertices to link head and hair
    uint32_t     hair_vertex_map[HAIR_VERTEX_MAP_LIMIT]; // Hair vertex indices to link
    uint32_t     head_vertex_map[HAIR_VERTEX_MAP_LIMIT]; // Head vertex indices to link

}hair_setup_t, *hair_setup_p;

// Gets scripted hair set-up to specified hair set-up structure.

bool Hair_GetSetup(uint32_t hair_entry_index, hair_setup_p hair_setup);

// Creates hair into allocated hair structure, using previously defined setup and
// entity index.

bool Hair_Create(hair_p hair, hair_setup_p setup, entity_p parent_entity);

// Removes specified hair from entity and clears it from memory.

void Hair_Clear(hair_p hair);

// Constantly updates some specific parameters to keep hair aligned to entity.

void Hair_Update(entity_p entity);

#endif  // HAIR_H
