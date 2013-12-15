
#ifndef RENDER_H
#define RENDER_H

#include <stdint.h>
#include "bullet/LinearMath/btScalar.h"

#define R_DRAW_WIRE             0x00000001                                      // провволочная отрисовка
#define R_DRAW_ROOMBOXES        0x00000002                                      // показывать границы комнаты
#define R_DRAW_BOXES            0x00000004                                      // показывать границы комнаты
#define R_DRAW_PORTALS          0x00000008                                      // показывать порталы
#define R_DRAW_FRUSTUMS         0x00000010                                      // показывать фрустумы
#define R_DRAW_NORMALS          0x00000020                                      // показывать нормали
#define R_DRAW_AXIS             0x00000040                                      // показывать оси
#define R_SKIP_ROOM             0x00000080                                      // hide rooms
#define R_SKIP_STATIC           0x00000100                                      // hide statics
#define R_SKIP_ENTITIES         0x00000200                                      // hide entities
#define R_DRAW_NULLMESHES       0x00000400                                      // draw nullmesh entities
#define R_DRAW_DUMMY_STATICS    0x00000800                                      // draw empty static meshes

#ifdef BT_USE_DOUBLE_PRECISION
    #define glMultMatrixbt glMultMatrixd
    #define GL_BT_SCALAR GL_DOUBLE
#else
    #define glMultMatrixbt glMultMatrixf
    #define GL_BT_SCALAR GL_FLOAT
#endif


// Native TR blending modes.

enum BlendingMode
{
    BM_OPAQUE,
    BM_TRANSPARENT,
    BM_MULTIPLY,
    BM_SIMPLE_SHADE,
    BM_TRANSPARENT_IGNORE_Z,
    BM_INVERT_SRC,
    BM_WIREFRAME,
    BM_TRANSPARENT_ALPHA,
    BM_INVERT_DEST,
    BM_SCREEN,
    BM_HIDE,
    BM_ANIMATED_TEX
};

// Animated texture types

#define TR_ANIMTEXTURE_FORWARD           0
#define TR_ANIMTEXTURE_BACKWARD          1
#define TR_ANIMTEXTURE_REVERSE           2

#define TR_ANIMTEXTURE_UVROTATE_FORWARD  0
#define TR_ANIMTEXTURE_UVROTATE_BACKWARD 1
#define TR_ANIMTEXTURE_UVROTATE_REVERSE  2

#define TR_ANIMTEXTURE_UPDATE_INTERVAL   0.0166   // 60 FPS

struct portal_s;
struct world_s;
struct room_s;
struct camera_s;
struct entity_s;
struct sprite_s;
struct base_mesh_s;
struct bounding_volume_s;

typedef struct render_list_s
{
    char active;
    struct room_s *room;
    btScalar dist;
}render_list_t, *render_list_p;

typedef struct render_settings_s
{
    float     lod_bias;
    uint32_t  mipmap_mode;
    uint32_t  mipmaps;
    uint32_t  anisotropy;
    int8_t    antialias;
    int8_t    antialias_samples;
    int8_t    texture_border;
    int8_t    z_depth;
}render_settings_t, *render_settings_p;

typedef struct render_s
{
    int8_t                      blocked;
    uint32_t                    style;                                          // 
    struct world_s             *world;
    struct camera_s            *cam;
    struct render_settings_s    settings;
    
    uint32_t                    r_list_size;
    uint32_t                    r_list_active_count;
    struct render_list_s       *r_list;

    uint32_t                    r_transparancy_list_size;
    uint32_t                    r_transparancy_list_active_count;
    struct render_list_s        *r_transparancy_list;
}render_t, *render_p;

extern render_t renderer;

void Render_Empty(render_p render);
void Render_InitGlobals();
void Render_Init();

render_list_p Render_CreateRoomListArray(unsigned int count);
void Render_Entity(struct entity_s *entity);                                    // отрисовка одного фрейма скелетной анимации
void Render_RoomSprite(struct room_sprite_s *sp);
void Render_SkeletalModel(struct ss_bone_frame_s *bframe);
void Render_Sprite(struct sprite_s *sprite);
void Render_SkyBox();
void Render_Mesh(struct base_mesh_s *mesh, const btScalar *overrideVertices, const btScalar *overrideNormals);
void Render_MeshTransparency(struct base_mesh_s *mesh);
void Render_SkinMesh(struct base_mesh_s *mesh, btScalar transform[16]);
void Render_AnimTexture(struct polygon_s *polygon);
void Render_UpdateAnimTextures();
void Render_CleanList();


void Render_Room(struct room_s *room, struct render_s *render);
void Render_Room_Sprites(struct room_s *room, struct render_s *render);
int Render_AddRoom(struct room_s *room);
void Render_DrawList();
void Render_DrawList_DebugLines();

int Render_HaveFrustumParent(struct room_s *room, struct frustum_s *frus);
int Render_ProcessRoom(struct portal_s *portal, struct frustum_s *frus);
void Render_GenWorldList();

void Render_SetWorld(struct world_s *world);


/*
 * DEBUG PRIMITIVES RENDERING
 */
void Render_SkyBox_DebugLines();
void Render_Entity_DebugLines(struct entity_s *entity);
void Render_SkeletalModel_DebugLines(struct ss_bone_frame_s *bframe);
void Render_Mesh_DebugLines(struct base_mesh_s *mesh, const btScalar *overrideVertices, const btScalar *overrideNormals);
void Render_Room_DebugLines(struct room_s *room, struct render_s *render);
void Render_DrawAxis(btScalar r);
void Render_BBox(btScalar bb_min[3], btScalar bb_max[3]);
void Render_SectorBorders(struct room_sector_s *sector);
void Render_BV(struct bounding_volume_s *bv);

#endif
