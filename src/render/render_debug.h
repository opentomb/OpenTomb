
#ifndef RENDER_DEBUG_H
#define RENDER_DEBUG_H

#include <stdint.h>
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>

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
#define R_DRAW_FLYBY            0x00008000      // FlyBy cameras spline rendering
#define R_DRAW_CINEMATICS       0x00010000      // Cinematics path rendering
#define R_DRAW_CAMERAS          0x00020000      // Cameras and sinks drawing
#define R_DRAW_TRIGGERS         0x00040000      // Trigger sectors drawing
#define R_DRAW_AI_BOXES         0x00080000      // AI boxes drawing
#define R_DRAW_AI_OBJECTS       0x00100000      // AI objects drawing
#define R_DRAW_AI_PATH          0x00200000      // AI character target path drawing

struct portal_s;
struct frustum_s;
struct world_s;
struct room_s;
struct camera_s;
struct entity_s;
struct sprite_s;
struct base_mesh_s;
struct obb_s;

class CRenderDebugDrawer
{
    struct vertex_s
    {
        GLfloat     pos[3];
        uint8_t     rgba[4];
    };
    
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
        void SetColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a/* = 255*/)
        {
            m_rgba[0] = r;
            m_rgba[1] = g;
            m_rgba[2] = b;
            m_rgba[3] = a;
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
        void DrawRoomDebugLines(struct room_s *room, struct camera_s *cam);

        // physics debug interface
        void   DrawLine(const float from[3], const float to[3], const float color_from[3], const float color_to[3]);
        void   DrawContactPoint(const float pointOnB[3], const float normalOnB[3], float distance, int lifeTime, const float color[3]);
        void     SetDrawFlags(uint32_t flags) {m_drawFlags = flags;};
        uint32_t GetDrawFlags() const {return m_drawFlags;}

    private:
        uint32_t            m_drawFlags;
        uint32_t            m_max_lines;
        uint32_t            m_lines;
        bool                m_need_realloc;

        GLuint              m_gl_vbo;
        uint8_t             m_rgba[4];
        struct vertex_s    *m_buffer;
};

#endif
