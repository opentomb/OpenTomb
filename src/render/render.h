
#ifndef RENDER_H
#define RENDER_H

#include <stdint.h>
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>

#include "../core/vmath.h"
#include "render_debug.h"

struct portal_s;
struct frustum_s;
struct world_s;
struct room_s;
struct camera_s;
struct entity_s;
struct sprite_s;
struct base_mesh_s;
struct obb_s;
struct lit_shader_description;

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
    int8_t    fog_enabled;
    GLfloat   fog_color[4];
    float     fog_start_depth;
    float     fog_end_depth;
    bool      show_fps;
}render_settings_t, *render_settings_p;


class CRender
{
    public:
        CRender();
       ~CRender();
        void DoShaders();
        void ResetWorld(struct room_s *rooms, uint32_t rooms_count, struct anim_seq_s *anim_sequences, uint32_t anim_sequences_count);
        void UpdateAnimTextures();

        void GenWorldList(struct camera_s *cam);
        void DrawList();
        void DrawListDebugLines();
        void CleanList();

        void DrawBSPPolygon(struct bsp_polygon_s *p);
        void DrawBSPFrontToBack(struct bsp_node_s *root);
        void DrawBSPBackToFront(struct bsp_node_s *root);

        void DrawMesh(struct base_mesh_s *mesh, const float *overrideVertices, const float *overrideNormals);
        void DrawSkinMesh(struct base_mesh_s *mesh, struct base_mesh_s *parent_mesh, uint32_t *map, float transform[16]);
        void DrawSkyBox(const float matrix[16]);

        void DrawSkeletalModel(const struct lit_shader_description *shader, struct ss_bone_frame_s *bframe, const float mvMatrix[16], const float mvpMatrix[16]);
        void DrawEntity(struct entity_s *entity, const float modelViewMatrix[16], const float modelViewProjectionMatrix[16]);

        void DrawRoom(struct room_s *room, const float matrix[16], const float modelViewProjectionMatrix[16]);
        void DrawRoomSprites(struct room_s *room);

        struct gl_text_line_s *OutTextXYZ(GLfloat x, GLfloat y, GLfloat z, const char *fmt, ...);

    private:
        struct render_list_s
        {
            char               active;
            struct room_s     *room;
            float              dist;
        };

        void InitSettings();
        int  AddRoom(struct room_s *room);
        int  ProcessRoom(struct portal_s *portal, struct frustum_s *frus);
        const lit_shader_description *SetupEntityLight(struct entity_s *entity, const float modelViewMatrix[16]);

        struct camera_s            *m_camera;

        struct room_s              *m_rooms;
        uint32_t                    m_rooms_count;
        struct anim_seq_s          *m_anim_sequences;
        uint32_t                    m_anim_sequences_count;

        uint16_t                    m_active_transparency;
        GLuint                      m_active_texture;

        GLfloat                     m_cam_right[3];
        uint32_t                    r_list_size;
        uint32_t                    r_list_active_count;
        struct render_list_s       *r_list;
        class CFrustumManager      *frustumManager;

    public:
        struct render_settings_s    settings;
        class shader_manager       *shaderManager;
        class CRenderDebugDrawer   *debugDrawer;
        class CDynamicBSP          *dynamicBSP;
        uint32_t                    r_flags;
};


extern CRender renderer;

#endif
