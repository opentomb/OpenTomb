
#ifndef RENDER_H
#define RENDER_H

#include <stdint.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include "bullet/LinearMath/btScalar.h"
#include "bullet/btBulletDynamicsCommon.h"
#include "bullet/LinearMath/btIDebugDraw.h"

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

#define DEBUG_DRAWER_DEFAULT_BUFFER_SIZE        (128 * 1024)

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

class render_DebugDrawer:public btIDebugDraw
{
    uint32_t m_debugMode;
    uint32_t m_max_lines;
    uint32_t m_lines;
    bool     m_need_realloc;

    GLuint  m_gl_vbo;
    GLfloat m_color[3];
    GLfloat *m_buffer;
    
    struct obb_s *m_obb;

    public:
        // engine debug function
        render_DebugDrawer();
        ~render_DebugDrawer();
        bool IsEmpty()
        {
            return m_lines == 0;
        }
        void reset();
        void render();
        void setColor(GLfloat r, GLfloat g, GLfloat b)
        {
            m_color[0] = r;
            m_color[1] = g;
            m_color[2] = b;
        }
        void drawAxis(btScalar r, btScalar transform[16]);
        void drawPortal(struct portal_s *p);
        void drawFrustum(struct frustum_s *f);
        void drawBBox(btScalar bb_min[3], btScalar bb_max[3], btScalar *transform);
        void drawOBB(struct obb_s *obb);
        void drawMeshDebugLines(struct base_mesh_s *mesh, btScalar transform[16], const btScalar *overrideVertices, const btScalar *overrideNormals);
        void drawSkeletalModelDebugLines(struct ss_bone_frame_s *bframe, btScalar transform[16]);
        void drawEntityDebugLines(struct entity_s *entity);
        void drawSectorDebugLines(struct room_sector_s *rs);
        void drawRoomDebugLines(struct room_s *room, struct render_s *render);
        
        // bullet's debug interface
        virtual void   drawLine(const btVector3& from,const btVector3& to,const btVector3& color);
        virtual void   drawContactPoint(const btVector3& PointOnB,const btVector3& normalOnB,btScalar distance,int lifeTime,const btVector3& color);
        virtual void   reportErrorWarning(const char* warningString);
        virtual void   draw3dText(const btVector3& location, const char* textString);
        virtual void   setDebugMode(int debugMode);
        virtual int    getDebugMode() const {return m_debugMode;}
};


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


typedef struct render_list_s
{
    char               active;
    struct room_s     *room;
    btScalar           dist;
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
    int8_t    fog_enabled;
    GLfloat   fog_color[4];
    float     fog_start_depth;
    float     fog_end_depth;
}render_settings_t, *render_settings_p;

typedef struct render_s
{
    int8_t                      blocked;
    uint32_t                    style;                                          //
    struct world_s             *world;
    struct camera_s            *cam;
    struct render_settings_s    settings;
    class shader_manager *shader_manager;

    uint32_t                    r_list_size;
    uint32_t                    r_list_active_count;
    struct render_list_s       *r_list;
}render_t, *render_p;

extern render_t renderer;

void Render_DoShaders();
void Render_Empty(render_p render);
void Render_InitGlobals();
void Render_Init();

render_list_p Render_CreateRoomListArray(unsigned int count);
void Render_Entity(struct entity_s *entity, const btScalar modelViewMatrix[16], const btScalar modelViewProjectionMatrix[16]);                                    // отрисовка одного фрейма скелетной анимации
void Render_DynamicEntity(const lit_shader_description *shader, struct entity_s *entity, const btScalar modelViewMatrix[16], const btScalar modelViewProjectionMatrix[16]);
void Render_SkeletalModel(const struct lit_shader_description *shader, struct ss_bone_frame_s *bframe, const btScalar mvMatrix[16], const btScalar mvpMatrix[16]);
void Render_Hair(struct entity_s *entity, const btScalar modelViewMatrix[16], const btScalar modelViewProjectionMatrix[16]);
void Render_SkyBox(const btScalar matrix[16]);
void Render_Mesh(struct base_mesh_s *mesh, const btScalar *overrideVertices, const btScalar *overrideNormals);
void Render_BSPPolygon(struct bsp_polygon_s *p);
void Render_BSPFrontToBack(struct bsp_node_s *root);
void Render_BSPBackToFront(struct bsp_node_s *root);
void Render_SkinMesh(struct base_mesh_s *mesh, btScalar transform[16]);
void Render_UpdateAnimTextures();
void Render_CleanList();


void Render_Room(struct room_s *room, struct render_s *render, const btScalar matrix[16], const btScalar modelViewProjectionMatrix[16]);
void Render_Room_Sprites(struct room_s *room, struct render_s *render, const btScalar modelViewMatrix[16], const btScalar projectionMatrix[16]);
int Render_AddRoom(struct room_s *room);
void Render_DrawList();
void Render_DrawList_DebugLines();

int Render_HaveFrustumParent(struct room_s *room, struct frustum_s *frus);
int Render_ProcessRoom(struct portal_s *portal, struct frustum_s *frus);
void Render_GenWorldList();

void Render_SetWorld(struct world_s *world);

void Render_CalculateWaterTint(GLfloat *tint, uint8_t fixed_colour);

/*
 * DEBUG PRIMITIVES RENDERING
 */
void Render_SkyBox_DebugLines();


#endif
