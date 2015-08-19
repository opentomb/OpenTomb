
#ifndef RENDER_H
#define RENDER_H

#include <stdint.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include "core/vmath.h"

#define R_DRAW_WIRE             0x00000001      // Wireframe rendering
#define R_DRAW_ROOMBOXES        0x00000002      // Show room bounds
#define R_DRAW_BOXES            0x00000004      // Show boxes
#define R_DRAW_PORTALS          0x00000008      // Show portals
#define R_DRAW_FRUSTUMS         0x00000010      // Show frustums
#define R_DRAW_NORMALS          0x00000020      // Show normals
#define R_DRAW_AXIS             0x00000040      // Show axes
#define R_SKIP_ROOM             0x00000080      // Hide rooms
#define R_SKIP_STATIC           0x00000100      // Hide statics
#define R_SKIP_ENTITIES         0x00000200      // Hide entities
#define R_DRAW_NULLMESHES       0x00000400      // Draw nullmesh entities
#define R_DRAW_DUMMY_STATICS    0x00000800      // Draw empty static meshes
#define R_DRAW_COLL             0x00001000      // Draw Bullet physics world
#define R_DRAW_SKYBOX           0x00002000      // Draw skybox
#define R_DRAW_POINTS           0x00004000      // Points rendering

#ifdef BT_USE_DOUBLE_PRECISION
    #define GL_BT_SCALAR GL_DOUBLE
#else
    #define GL_BT_SCALAR GL_FLOAT
#endif

#define STENCIL_FRUSTUM 1

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
}render_settings_t, *render_settings_p;


class CRenderDebugDrawer
{
    public:
        // engine debug function
        CRenderDebugDrawer();
        virtual ~CRenderDebugDrawer();
        bool IsEmpty()
        {
            return m_lines == 0;
        }
        void Reset();
        void Render();
        void SetColor(GLfloat r, GLfloat g, GLfloat b)
        {
            m_color[0] = r;
            m_color[1] = g;
            m_color[2] = b;
        }
        void DrawAxis(float r, float transform[16]);
        void DrawPortal(struct portal_s *p);
        void DrawFrustum(struct frustum_s *f);
        void DrawBBox(float bb_min[3], float bb_max[3], float *transform);
        void DrawOBB(struct obb_s *obb);
        void DrawMeshDebugLines(struct base_mesh_s *mesh, float transform[16], const float *overrideVertices, const float *overrideNormals);
        void DrawSkeletalModelDebugLines(struct ss_bone_frame_s *bframe, float transform[16]);
        void DrawEntityDebugLines(struct entity_s *entity);
        void DrawSectorDebugLines(struct room_sector_s *rs);
        void DrawRoomDebugLines(struct room_s *room);
        
        // physics debug interface
        void   DrawLine(const float from[3], const float to[3], const float color_from[3], const float color_to[3]);
        void   DrawContactPoint(const float pointOnB[3], const float normalOnB[3], float distance, int lifeTime, const float color[3]);
        void     SetDrawFlags(uint32_t flags) {m_drawFlags = flags;};
        uint32_t GetDrawFlags() const {return m_drawFlags;}
        
    private:
        uint32_t m_drawFlags;
        uint32_t m_max_lines;
        uint32_t m_lines;
        bool     m_need_realloc;

        GLuint   m_gl_vbo;
        GLfloat  m_color[3];
        GLfloat *m_buffer;

        struct obb_s *m_obb;
};


class CRender
{
    public:
        CRender();
       ~CRender();
        void DoShaders();
        void SetWorld(struct world_s *world);
        void UpdateAnimTextures();

        void GenWorldList(struct camera_s *cam);
        void DrawList();
        void DrawListDebugLines();
        void CleanList();

        void DrawBSPPolygon(struct bsp_polygon_s *p);
        void DrawBSPFrontToBack(struct bsp_node_s *root);
        void DrawBSPBackToFront(struct bsp_node_s *root);

        void DrawMesh(struct base_mesh_s *mesh, const float *overrideVertices, const float *overrideNormals);
        void DrawSkinMesh(struct base_mesh_s *mesh, float transform[16]);
        void DrawSkyBox(const float matrix[16]);

        void DrawSkeletalModel(const struct lit_shader_description *shader, struct ss_bone_frame_s *bframe, const float mvMatrix[16], const float mvpMatrix[16]);
        void DrawEntity(struct entity_s *entity, const float modelViewMatrix[16], const float modelViewProjectionMatrix[16]);

        void DrawRoom(struct room_s *room, const float matrix[16], const float modelViewProjectionMatrix[16]);
        void DrawRoomSprites(struct room_s *room, const float modelViewMatrix[16], const float projectionMatrix[16]);

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
        
        struct world_s             *m_world;
        struct camera_s            *m_camera;

        uint16_t                    m_active_transparency;
        GLuint                      m_active_texture;
        
        uint32_t                    r_list_size;
        uint32_t                    r_list_active_count;
        struct render_list_s       *r_list;
        class CFrustumManager      *frustumManager;
        
    public:
        uint32_t                    r_flags;
        struct render_settings_s    settings;
        class shader_manager       *shaderManager;
        class CRenderDebugDrawer   *debugDrawer;
        class CDynamicBSP          *dynamicBSP;
};


extern CRender renderer;

#endif
