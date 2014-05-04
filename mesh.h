
#ifndef MESH_H
#define MESH_H

#define MESH_FULL_OPAQUE 0x00                                                   // Fully opaque object (all polygons are opaque: all t.flags < 0x02)
#define MESH_FULL_TRANSPERENCY 0x01                                             // Fully transparancy object (all polygons are transparancy: all t.flags >= 2)
#define MESH_PART_TRANSPERENCY 0x02                                             // Has transparancy and opaque polygons

#include <SDL2/SDL_opengl.h>
#include <stdint.h>
#include "bullet/LinearMath/btScalar.h"

class btCollisionShape;
class btRigidBody;
class btCollisionShape;

struct polygon_s;
struct room_s;
struct engine_container_s;
struct bounding_volume_s;
struct vertex_s;

/*
 * base mesh, uses everywhere
 */
typedef struct base_mesh_s
{
    uint32_t              ID;                                                   // mesh's ID
    uint32_t              transparancy_flags;                                   // transparancy flags
    
    uint32_t              transparancy_count;                                   // number of transparancy polygons
    uint32_t              poly_count;                                           // number of all mesh's polygons
    struct polygon_s     *polygons;                                             // polygons data
    
    uint32_t num_texture_pages;                                                 // face without structure wrapping
    uint32_t *element_count_per_texture;                                        //
    uint32_t *elements;                                                         //
    
    uint32_t              vertex_count;                                         // number of mesh's vertices
    struct vertex_s      *vertices;

    btScalar              centre[3];                                            // geometry centre of mesh
    btScalar              bb_min[3];                                            // AABB bounding volume
    btScalar              bb_max[3];                                            // AABB bounding volume
    btScalar              R;                                                    // radius of the bounbing sphere
    int8_t               *skin_map;                                             // vertices map for skin mesh

    GLuint                vbo_vertex_array;
    GLuint                vbo_index_array;
}base_mesh_t, *base_mesh_p;


/*
 * base sprite structure
 */
typedef struct sprite_s
{
    uint32_t            ID;                                                     // object's ID
    uint32_t            texture;                                                // texture number
    GLfloat             tex_coord[8];                                           // texture coordinates
    uint32_t            flag; 
    btScalar            left;                                                   // world sprite's gabarites
    btScalar            right;
    btScalar            top;
    btScalar            bottom;
}sprite_t, *sprite_p;


/*
 *  Animated sequence. Used globally with animated textures to refer its parameters and frame numbers.
 */
typedef struct anim_seq_s
{
    int8_t      type;             // 0 = normal, 1 = back, 2 = reverse.
    bool        type_flag;        // Used only with type 2 to identify current animation direction.
    
    bool        blend;            // Blend flag.  Reserved for future use!
    btScalar    blend_rate;       // Blend rate.  Reserved for future use!
    btScalar    blend_time;       // Blend value. Reserved for future use!
    
    bool        uvrotate;         // UVRotate mode flag.
    int8_t      uvrotate_type;    // 0 = normal, 1 = back, 2 = reverse.
    bool        uvrotate_flag;    // Used only with type 2 to identify current animation direction.
    btScalar    uvrotate_speed;   // Speed of UVRotation, in seconds.
    btScalar    uvrotate_time;    // Time passed since last UVRotate update.
    btScalar    uvrotate_max;     // Reference value used to restart rotation.
    btScalar    current_uvrotate; // Current coordinate window position.
    
    uint32_t    current_frame;    // Current frame for this sequence.
    btScalar    frame_rate;       // For types 0-1, specifies framerate, for type 3, should specify rotation speed.
    btScalar    frame_time;       // Time passed since last update.
    uint32_t    frame_count;      // Overall frames to use. If type is 3, it should be 1, else behaviour is undetermined.
    bool        frame_lock;       // Single frame mode. Needed for TR4-5 compatible UVRotate.
    uint32_t*   frame_list;       // Offset into anim textures frame list.
}anim_seq_t, *anim_seq_p;


/*
 * room static mesh.
 */
typedef struct static_mesh_s
{
    uint32_t                    object_id;                                      //
    uint8_t                     was_rendered;                                   // 0 - was not rendered, 1 - opaque, 2 - transparancy, 3 - full rendered
    uint8_t                     was_rendered_lines;
    uint8_t                     hide;                                           // disable static mesh rendering
    btScalar                    pos[3];                                         // model position
    btScalar                    rot[3];                                         // model angles
    
    btScalar                    vbb_min[3];                                     // visible bounding box
    btScalar                    vbb_max[3];
    btScalar                    cbb_min[3];                                     // collision bounding box
    btScalar                    cbb_max[3];
    
    btScalar                    transform[16];                                  // gl transformation matrix
    struct bounding_volume_s   *bv;
    struct engine_container_s  *self;
    
    struct base_mesh_s         *mesh;                                           // base model
    btRigidBody                *bt_body;
}static_mesh_t, *static_mesh_p;

/*
 * Animated skeletal model. Taken from openraider.
 * model -> animation -> frame -> bone
 * thanks to Terry 'Mongoose' Hendrix II
 */

/*
 * SMOOTHED ANIMATIONS STRUCTURES
 * stack matrices are needed for skinned mesh transformations.
 */
typedef struct ss_bone_tag_s
{
    base_mesh_p         mesh;                                                   // base mesh - pointer to the first mesh in array
    base_mesh_p         mesh2;                                                  // base skinned mesh for ТР4+
    btScalar            offset[3];                                              // model position offset
 
    btScalar            qrotate[4];                                             // quaternion rotation
    btScalar            transform[16];                                          // 4x4 OpenGL matrix for stack usage
    btScalar            full_transform[16];                                     // 4x4 OpenGL matrix for global usage
    
    uint16_t            flag;                                                   // 0x0001 = POP, 0x0002 = PUSH, 0x0003 = RESET
    uint16_t            overrided;                                              // flag for shoot / guns animations
}ss_bone_tag_t, *ss_bone_tag_p;

/*
 * base frame of animated skeletal model
 */
typedef struct ss_bone_frame_s
{
    uint16_t                    bone_tag_count;                                 // number of bones
    struct ss_bone_tag_s       *bone_tags;                                      // array of bones
    btScalar                    pos[3];                                         // position (base offset)
    btScalar                    bb_min[3];                                      // bounding box min coordinates
    btScalar                    bb_max[3];                                      // bounding box max coordinates
    btScalar                    centre[3];                                      // bounding box centre
}ss_bone_frame_t, *ss_bone_frame_p;

/*
 * ORIGINAL ANIMATIONS
 */
typedef struct bone_tag_s
{
    btScalar              offset[3];                                            // bone vector
    btScalar              qrotate[4];                                           // rotation quaternion
}bone_tag_t, *bone_tag_p;

/*
 * base frame of animated skeletal model
 */
typedef struct bone_frame_s
{
    uint16_t            bone_tag_count;                                         // number of bones
    struct bone_tag_s  *bone_tags;                                              // bones data
    btScalar            pos[3];                                                 // position (base offset)
    btScalar            bb_min[3];                                              // bounding box min coordinates
    btScalar            bb_max[3];                                              // bounding box max coordinates
    btScalar            centre[3];                                              // bounding box centre
}bone_frame_t, *bone_frame_p;

/*
 * mesh tree base element structure
 */
typedef struct mesh_tree_tag_s                                                  
{
    base_mesh_p                 mesh;                                           // base mesh - pointer to the first mesh in array
    base_mesh_p                 mesh2;                                          // base skinned mesh for ТР4+
    btScalar                    offset[3];                                      // model position offset
    uint16_t                    flag;                                           // 0x0001 = POP, 0x0002 = PUSH, 0x0003 = RESET
    uint16_t                    overrided;                                      // flag for shoot / guns animations
}mesh_tree_tag_t, *mesh_tree_tag_p;

/*
 * animation switching control structure
 */
typedef struct anim_dispath_s
{
    uint16_t    next_anim;                                                      // "switch to" animation 
    uint16_t    next_frame;                                                     // "switch to" frame 
    uint16_t    frame_low;                                                      // low border of state change condition
    uint16_t    frame_high;                                                     // high border of state change condition
} anim_dispath_t, *anim_dispath_p;

typedef struct state_change_s
{
    uint32_t                    ID;
    uint16_t                    anim_dispath_count;
    struct anim_dispath_s      *anim_dispath;
} state_change_t, *state_change_p;

/*
 * one animation frame structure
 */
typedef struct animation_frame_s                                                
{
    uint32_t                    ID;
    uint8_t                     original_frame_rate;
    int                         accel_hi;
    int                         accel_hi2;
    int                         accel_lo;
    int                         accel_lo2;
    int                         speed;
    int                         speed2;
    uint32_t                    anim_command;
    uint32_t                    num_anim_commands;
    uint16_t                    state_id;
    int16_t                     unknown;
    int16_t                     unknown2;
    uint16_t                    frames_count;                                   // number of frames
    struct bone_frame_s        *frames;                                         // frames data
    
    uint16_t                    state_change_count;                             // number of animation statechanges
    struct state_change_s      *state_change;                                   // animation statechanges data
    
    struct animation_frame_s   *next_anim;                                      // next default animation
    int                         next_frame;                                     // next default frame
}animation_frame_t, *animation_frame_p;

/*
 * skeletal model with animations data.
 */

typedef struct skeletal_model_s
{
    uint32_t                    ID;                                             // ID
    uint16_t                    transparancy_flags;                             // transparancy flags; 0 - opaque; 1 - alpha test; other - blending mode
    uint16_t                    hide;                                           // do not render
    btScalar                    bbox_min[3];                                    // bbox info
    btScalar                    bbox_max[3];
    btScalar                    centre[3];                                      // the centre of model
        
    uint16_t                    animation_count;                                // number of animations
    struct animation_frame_s   *animations;                                     // animations data
    
    uint16_t                    mesh_count;                                     // number of model meshes
    struct base_mesh_s         *mesh_offset;                                    // pointer to the first mesh in skeletal model mesh array
    struct mesh_tree_tag_s     *mesh_tree;                                      // base mesh tree.
}skeletal_model_t, *skeletal_model_p; 


void BaseMesh_Clear(base_mesh_p mesh);
void BaseMesh_FindBB(base_mesh_p mesh);
void Mesh_GenVBO(struct base_mesh_s *mesh);

void Mesh_DefaultColor(struct base_mesh_s *mesh);
void Mesh_MullColors(struct base_mesh_s *mesh, float *cl_mult);

void SkeletalModel_Clear(skeletal_model_p model);
void SkeletonModel_FillTransparancy(skeletal_model_p model);
void SkeletalModel_InterpolateFrames(skeletal_model_p models);
void FillSkinnedMeshMap(skeletal_model_p model);

void BoneFrame_Copy(bone_frame_p dst, bone_frame_p src);
mesh_tree_tag_p SkeletonClone(mesh_tree_tag_p src, int tags_count);
void SkeletonCopyMeshes(mesh_tree_tag_p dst, mesh_tree_tag_p src, int tags_count);
void SkeletonCopyMeshes2(mesh_tree_tag_p dst, mesh_tree_tag_p src, int tags_count);


uint32_t Mesh_AddVertex(base_mesh_p mesh, struct vertex_s *vertex);
void Mesh_GenFaces(base_mesh_p mesh);

#endif
