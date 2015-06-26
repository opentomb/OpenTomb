
#ifndef RENDER_H
#define RENDER_H

#include <stdint.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include "bullet/LinearMath/btScalar.h"
#include "bullet/btBulletDynamicsCommon.h"
#include "bullet/LinearMath/btIDebugDraw.h"

#include <memory>
#include <vector>

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
#define INIT_FRAME_VERTEX_BUFFER_SIZE           (1024 * 1024)

#ifdef BT_USE_DOUBLE_PRECISION
    #define GL_BT_SCALAR GL_DOUBLE
#else
    #define GL_BT_SCALAR GL_FLOAT
#endif

#define STENCIL_FRUSTUM 1

struct portal_s;
struct frustum_s;
struct world_s;
struct Room;
struct camera_s;
struct Entity;
struct sprite_s;
struct base_mesh_s;
struct obb_s;
struct lit_shader_description;
class vertex_array;

class render_DebugDrawer:public btIDebugDraw
{
    uint32_t m_debugMode = 0;

    std::array<GLfloat,3> m_color{{0,0,0}};
    std::vector<std::array<GLfloat,3>> m_buffer;
    
    std::unique_ptr<obb_s> m_obb;

    void addLine(const std::array<GLfloat,3> &start, const std::array<GLfloat,3> &end);
    void addLine(const btVector3& start, const btVector3& end);
    void addLine(const std::array<GLfloat,3> &start, const std::array<GLfloat,3> &startColor, const std::array<GLfloat,3> &end, const std::array<GLfloat,3> &endColor);
    
    vertex_array *m_vertexArray = nullptr;
    GLuint m_glbuffer = 0;
    
    public:
        // engine debug function
        render_DebugDrawer();
        ~render_DebugDrawer();
        bool IsEmpty()
        {
            return m_buffer.empty();
        }
        void reset();
        void render();
        void setColor(GLfloat r, GLfloat g, GLfloat b)
        {
            m_color[0] = r;
            m_color[1] = g;
            m_color[2] = b;
        }
        void drawAxis(btScalar r, const btTransform& transform);
        void drawPortal(const portal_s &p);
        void drawFrustum(const frustum_s &f);
        void drawBBox(const btVector3 &bb_min, const btVector3 &bb_max, const btTransform *transform);
        void drawOBB(struct obb_s *obb);
        void drawMeshDebugLines(struct base_mesh_s *mesh, const btTransform& transform, const std::vector<btVector3> &overrideVertices, const std::vector<btVector3> &overrideNormals);
        void drawSkeletalModelDebugLines(struct ss_bone_frame_s *bframe, const btTransform& transform);
        void drawEntityDebugLines(std::shared_ptr<Entity> entity);
        void drawSectorDebugLines(struct room_sector_s *rs);
        void drawRoomDebugLines(std::shared_ptr<Room> room, struct render_s *render);
        
        // bullet's debug interface
        virtual void   drawLine(const btVector3& from, const btVector3& to, const btVector3 &color);
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
    std::shared_ptr<Room> room;
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
    class vertex_array_manager *vertex_array_manager;

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
void Render_Entity(std::shared_ptr<Entity> entity, const btTransform &modelViewMatrix, const btTransform &modelViewProjectionMatrix, const btTransform &projection);
void Render_DynamicEntity(const struct lit_shader_description *shader, std::shared_ptr<Entity> entity, const btTransform &modelViewMatrix, const btTransform &modelViewProjectionMatrix);
void Render_DynamicEntitySkin(const struct lit_shader_description *shader, std::shared_ptr<Entity> ent, const btTransform& pMatrix);
void Render_SkeletalModel(const struct lit_shader_description *shader, struct ss_bone_frame_s *bframe, const btTransform &mvMatrix, const btTransform &mvpMatrix);
void Render_SkeletalModelSkin(const struct lit_shader_description *shader, std::shared_ptr<Entity> ent, const btTransform &mvMatrix, const btTransform &pMatrix);
void Render_Hair(std::shared_ptr<Entity> entity, const btTransform& modelViewMatrix, const btTransform& modelViewProjectionMatrix);
void Render_SkyBox(const btTransform &matrix);
void Render_Mesh(struct base_mesh_s *mesh);
void Render_PolygonTransparency(uint16_t &currentTransparency, const struct bsp_face_ref_s *p, const struct unlit_tinted_shader_description *shader);
void Render_BSPFrontToBack(uint16_t &currentTransparency, struct bsp_node_s *root, const struct unlit_tinted_shader_description *shader);
void Render_BSPBackToFront(uint16_t &currentTransparency, struct bsp_node_s *root, const struct unlit_tinted_shader_description *shader);
void Render_UpdateAnimTextures();
void Render_CleanList();


void Render_Room(std::shared_ptr<Room> room, struct render_s *render, const btTransform& matrix, const btTransform& modelViewProjectionMatrix, const btTransform& projection);
void Render_Room_Sprites(std::shared_ptr<Room> room, struct render_s *render, const btTransform& modelViewMatrix, const btTransform& projectionMatrix);
int Render_AddRoom(std::shared_ptr<Room> room);
void Render_DrawList();
void Render_DrawList_DebugLines();

int Render_HaveFrustumParent(struct Room *room, struct frustum_s *frus);
int Render_ProcessRoom(struct portal_s *portal, struct frustum_s *frus);
void Render_GenWorldList();

void Render_SetWorld(struct world_s *world);

void Render_CalculateWaterTint(std::array<float,4> *tint, uint8_t fixed_colour);

/*
 * DEBUG PRIMITIVES RENDERING
 */
void Render_SkyBox_DebugLines();


#endif
