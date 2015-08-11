#ifndef HAIR_H
#define HAIR_H

///@TODO: make IHair class and CHair class + fabric method;

#include <stdint.h>
#include "core/vmath.h"

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
struct entity_s;
struct base_mesh_s;
struct lua_State;

// Gets scripted hair set-up to specified hair set-up structure.
struct hair_setup_s *Hair_GetSetup(struct lua_State *lua, uint32_t hair_entry_index);

// Creates hair into allocated hair structure, using previously defined setup and
// entity index.
struct hair_s *Hair_Create(struct hair_setup_s *setup, struct entity_s *parent_entity);

// Removes specified hair from entity and clears it from memory.
void Hair_Delete(struct hair_s *hair);

// Constantly updates some specific parameters to keep hair aligned to entity.
void Hair_Update(struct entity_s *entity);

int Hair_GetElementsCount(struct hair_s *hair);

int Hair_GetElementInfo(struct hair_s *hair, int element, struct base_mesh_s **mesh, float tr[16]);

#endif  // HAIR_H
