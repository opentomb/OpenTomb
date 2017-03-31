
#ifndef PHYSICS_HAIR_H
#define	PHYSICS_HAIR_H

#include <stdint.h>

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


typedef struct hair_setup_s
{
    uint32_t     model_id;           // Hair model ID
    uint32_t     link_body;          // Lara's head mesh index

    float        root_weight;        // Root and tail hair body weight. Intermediate body
    float        tail_weight;        // weights are calculated from these two parameters

    float        hair_damping[2];    // Damping affects hair "plasticity"
    float        hair_inertia;       // Inertia affects hair "responsiveness"
    float        hair_restitution;   // "Bounciness" of the hair
    float        hair_friction;      // How much other bodies will affect hair trajectory

    float        joint_overlap;      // How much two hair bodies overlap each other

    float        joint_cfm;          // Constraint force mixing (joint softness)
    float        joint_erp;          // Error reduction parameter (joint "inertia")

    float        head_offset[3];     // Linear offset to place hair to
    float        root_angle[3];      // First constraint set angle (to align hair angle)

    uint32_t     vertex_map_count;   // Amount of REAL vertices to link head and hair
    uint32_t     hair_vertex_map[HAIR_VERTEX_MAP_LIMIT]; // Hair vertex indices to link
    uint32_t     head_vertex_map[HAIR_VERTEX_MAP_LIMIT]; // Head vertex indices to link
}hair_setup_t, *hair_setup_p;


// Gets scripted hair set-up to specified hair set-up structure.
hair_setup_p Hair_GetSetup(struct lua_State *lua, uint32_t hair_entry_index);


#endif	/* PHYSICS_HAIR_H */
