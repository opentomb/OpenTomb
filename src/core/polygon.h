
#ifndef POLYGON_H
#define POLYGON_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>

#include "vmath.h"

#define SPLIT_FRONT    0x00
#define SPLIT_BACK     0x01
#define SPLIT_IN_PLANE 0x02
#define SPLIT_IN_BOTH  0x03

#define SPLIT_EPSILON (0.02)

/*
 * The structure taken from Cochrane. Next I realise one in my style.
 * make it aligned... is it enough good?
 */
typedef struct vertex_s
{
    float           position[4];
    GLfloat         normal[4];
    GLfloat         color[4];
    GLfloat         tex_coord[2];
} vertex_t, *vertex_p;


typedef struct polygon_s
{
    struct vertex_s    *vertices;                                               // vertices data
    GLuint              texture_index;                                          // texture index
    uint16_t            vertex_count;                                           // number of vertices
    uint16_t            anim_id;                                                // anim texture ID
    uint16_t            frame_offset;                                           // anim texture frame offset
    uint16_t            transparency;                                           // transparency information
    uint8_t             double_side;                                            // double side flag
    float               plane[4];                                               // polygon plane equation

    struct polygon_s   *next;                                                   // polygon list (for BSP using)
}polygon_t, *polygon_p;

/*
 *  Animated sequence. Used globally with animated textures to refer its parameters and frame numbers.
 */

typedef struct tex_frame_s
{
    GLuint      texture_index;
    GLfloat     mat[4];
    GLfloat     move[2];
    GLfloat     uvrotate_max;           // Reference value used to restart rotation.
    GLfloat     current_uvrotate;       // Current coordinate window position.
}tex_frame_t, *tex_frame_p;

typedef struct anim_seq_s
{
    int8_t      uvrotate;               // UVRotate mode flag.
    int8_t      frame_lock;             // Single frame mode. Needed for TR4-5 compatible UVRotate.
    int8_t      anim_type;              // 0 = normal, 1 = back, 2 = reverse.
    int8_t      reverse_direction;      // Used only with type 2 to identify current animation direction.
    uint16_t    frames_count;           // Overall frames to use. If type is 3, it should be 1, else behaviour is undetermined.
    uint16_t    current_frame;          // Current frame for this sequence.
    GLfloat     frame_time;             // Time passed since last frame update.
    GLfloat     frame_rate;             // For types 0-1, specifies framerate, for type 3, should specify rotation speed.

    struct tex_frame_s  *frames;
    uint32_t            *frame_list;    // Offset into anim textures frame list.
}anim_seq_t, *anim_seq_p;


void ApplyAnimTextureTransformation(GLfloat *uv_out, const GLfloat *uv_in, const struct tex_frame_s *tf);

/*
 * polygons functions
 */
polygon_p Polygon_CreateArray(unsigned int pcount);

void Polygon_Resize(polygon_p p, unsigned int count);
void Polygon_Clear(polygon_p p);
int  Polygon_IsBroken(polygon_p p);
void Polygon_Copy(polygon_p dst, polygon_p src);

void Polygon_MoveSelf(polygon_p p, float move[3]);
void Polygon_Move(polygon_p ret, polygon_p src, float move[3]);
void Polygon_Transform(polygon_p ret, polygon_p src, float tr[16]);
void Polygon_TransformSelf(polygon_p p, float tr[16]);

void Polygon_FindNormale(polygon_p p);
int  Polygon_RayIntersect(polygon_p p, float dir[3], float dot[3], float *t);
int  Polygon_IntersectPolygon(polygon_p p1, polygon_p p2);

int  Polygon_SplitClassify(polygon_p p, float n[4]);
void Polygon_Split(polygon_p src, float n[4], polygon_p front, polygon_p back);

int Polygon_IsInsideBBox(polygon_p p, float bb_min[3], float bb_max[3]);
int Polygon_IsInsideBQuad(polygon_p p, float bb_min[3], float bb_max[3]);

#ifdef	__cplusplus
}
#endif
#endif
