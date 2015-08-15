#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <vector>

// glew must be included BEFORE btIDebugDraw.h
#include <GL/glew.h>

#include <LinearMath/btScalar.h>
#include <LinearMath/btIDebugDraw.h>

#include "matrix4.h"
#include "vertex_array.h"
#include "loader/datatypes.h"

#define DEBUG_DRAWER_DEFAULT_BUFFER_SIZE        (128 * 1024)
#define INIT_FRAME_VERTEX_BUFFER_SIZE           (1024 * 1024)

#define STENCIL_FRUSTUM 1

struct Portal;
struct Frustum;
struct World;
struct Room;
class Camera;
struct Entity;
struct Sprite;
struct BaseMesh;
struct OBB;
struct LitShaderDescription;
struct SSBoneFrame;
struct RoomSector;
class Render;

class RenderDebugDrawer : public btIDebugDraw
{
    uint32_t m_debugMode = 0;

    std::array<GLfloat, 3> m_color{ {0,0,0} };
    std::vector<std::array<GLfloat, 3>> m_buffer;

    std::unique_ptr<OBB> m_obb;

    void addLine(const std::array<GLfloat, 3> &start, const std::array<GLfloat, 3> &end);
    void addLine(const btVector3& start, const btVector3& end);
    void addLine(const std::array<GLfloat, 3> &start, const std::array<GLfloat, 3> &startColor, const std::array<GLfloat, 3> &end, const std::array<GLfloat, 3> &endColor);

    std::unique_ptr<VertexArray> m_vertexArray{};
    GLuint m_glbuffer = 0;

public:
    // engine debug function
    RenderDebugDrawer();
    ~RenderDebugDrawer();
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
    void drawPortal(const Portal &p);
    void drawFrustum(const Frustum &f);
    void drawBBox(const btVector3 &bb_min, const btVector3 &bb_max, const btTransform *transform);
    void drawOBB(OBB *obb);
    void drawMeshDebugLines(const std::shared_ptr<BaseMesh> &mesh, const btTransform& transform, const std::vector<btVector3> &overrideVertices, const std::vector<btVector3> &overrideNormals, Render* render);
    void drawSkeletalModelDebugLines(SSBoneFrame *bframe, const btTransform& transform, Render *render);
    void drawEntityDebugLines(Entity *entity, Render *render);
    void drawSectorDebugLines(RoomSector *rs);
    void drawRoomDebugLines(const Room *room, Render *render);

    // bullet's debug interface
    virtual void   drawLine(const btVector3& from, const btVector3& to, const btVector3 &color) override;
    virtual void   drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) override;
    virtual void   reportErrorWarning(const char* warningString) override;
    virtual void   draw3dText(const btVector3& location, const char* textString) override;
    virtual void   setDebugMode(int debugMode) override;
    virtual int    getDebugMode() const override
    {
        return m_debugMode;
    }
};

// Animated texture types

#define TR_ANIMTEXTURE_FORWARD           0
#define TR_ANIMTEXTURE_BACKWARD          1
#define TR_ANIMTEXTURE_REVERSE           2

struct RenderSettings
{
    float     lod_bias = 0;
    uint32_t  mipmap_mode = 3;
    uint32_t  mipmaps = 3;
    uint32_t  anisotropy = 0;

    bool      antialias = false;
    int       antialias_samples = 0;

    int       texture_border = 8;
    bool      save_texture_memory = false;

    int       z_depth = 16;

    bool      fog_enabled = true;
    GLfloat   fog_color[4]{ 0,0,0,1 };
    float     fog_start_depth = 10000;
    float     fog_end_depth = 16000;

    bool      use_gl3 = false;
};

class ShaderManager;
struct BSPNode;
struct UnlitTintedShaderDescription;
struct SSBoneFrame;
struct BSPFaceRef;
struct Character;

class Render
{
    friend class RenderDebugDrawer;
private:
    bool m_blocked = true;
    World* m_world = nullptr;
    Camera* m_cam = nullptr;
    RenderSettings m_settings;
    std::unique_ptr<ShaderManager> m_shaderManager;

    std::vector<Room*> m_renderList{};

    bool m_drawWire = false;
    bool m_drawRoomBoxes = false;
    bool m_drawBoxes = false;
    bool m_drawPortals = false;
    bool m_drawFrustums = false;
    bool m_drawNormals = false;
    bool m_drawAxis = false;
    bool m_skipRoom = false;
    bool m_skipStatic = false;
    bool m_skipEntities = false;
    bool m_drawAllModels = false;
    bool m_drawDummyStatics = false;
    bool m_drawColl = false;
    bool m_drawSkybox = false;
    bool m_drawPoints = false;
public:
    void cleanList();
    void genWorldList();
    void drawList();
    void drawListDebugLines();
    void doShaders();
    void initGlobals();
    void init();
    void empty();
    bool addRoom(Room *room);
    void setWorld(World* m_world);
    void resetWorld()
    {
        m_world = nullptr;
        m_renderList.clear();
    }

    const std::unique_ptr<ShaderManager>& shaderManager()
    {
        return m_shaderManager;
    }
    Camera* camera()
    {
        return m_cam;
    }
    void setCamera(Camera* cam)
    {
        m_cam = cam;
    }

    World* world()
    {
        return m_world;
    }
    const RenderSettings& settings() const
    {
        return m_settings;
    }
    RenderSettings& settings()
    {
        return m_settings;
    }

    void hideSkyBox()
    {
        m_drawSkybox = false;
    }
    void toggleWireframe()
    {
        m_drawWire = !m_drawWire;
    }
    void toggleDrawPoints()
    {
        m_drawPoints = !m_drawPoints;
    }
    void toggleDrawColl()
    {
        m_drawColl = !m_drawColl;
    }
    void toggleDrawNormals()
    {
        m_drawNormals = !m_drawNormals;
    }
    void toggleDrawPortals()
    {
        m_drawPortals = !m_drawPortals;
    }
    void toggleDrawFrustums()
    {
        m_drawFrustums = !m_drawFrustums;
    }
    void toggleDrawRoomBoxes()
    {
        m_drawRoomBoxes = !m_drawRoomBoxes;
    }
    void toggleDrawBoxes()
    {
        m_drawBoxes = !m_drawBoxes;
    }
    void toggleDrawAxis()
    {
        m_drawAxis = !m_drawAxis;
    }
    void toggleDrawAllModels()
    {
        m_drawAllModels = !m_drawAllModels;
    }
    void toggleDrawDummyStatics()
    {
        m_drawDummyStatics = !m_drawDummyStatics;
    }
    void toggleSkipRoom()
    {
        m_skipRoom = !m_skipRoom;
    }

    void renderEntity(Entity *entity, const matrix4 &modelViewMatrix, const matrix4 &modelViewProjectionMatrix, const matrix4 &projection);
    void renderDynamicEntity(const LitShaderDescription *shader, Entity *entity, const matrix4 &modelViewMatrix, const matrix4 &modelViewProjectionMatrix);
    void renderDynamicEntitySkin(const LitShaderDescription *shader, Entity *ent, const matrix4 &mvMatrix, const matrix4 &pMatrix);
    void renderSkeletalModel(const LitShaderDescription *shader, SSBoneFrame* bframe, const matrix4 &mvMatrix, const matrix4 &mvpMatrix);
    void renderSkeletalModelSkin(const LitShaderDescription *shader, Entity *ent, const matrix4 &mvMatrix, const matrix4 &pMatrix);
    void renderHair(std::shared_ptr<Character> entity, const matrix4 &modelViewMatrix, const matrix4 & modelViewProjectionMatrix);
    void renderSkyBox(const matrix4& matrix);
    void renderMesh(const std::shared_ptr<BaseMesh> &mesh);
    void renderPolygonTransparency(loader::BlendingMode& currentTransparency, const BSPFaceRef &p, const UnlitTintedShaderDescription *shader);
    void renderBSPFrontToBack(loader::BlendingMode& currentTransparency, const std::unique_ptr<BSPNode> &root, const UnlitTintedShaderDescription *shader);
    void renderBSPBackToFront(loader::BlendingMode& currentTransparency, const std::unique_ptr<BSPNode> &root, const UnlitTintedShaderDescription *shader);
    void renderRoom(const Room *room, const matrix4 &matrix, const matrix4 &modelViewProjectionMatrix, const matrix4 &projection);
    void renderRoomSprites(const Room *room, const matrix4 &modelViewMatrix, const matrix4 &projectionMatrix);

    int processRoom(Portal *portal, const std::shared_ptr<Frustum> &frus);

private:
    const LitShaderDescription *setupEntityLight(Entity *entity, const matrix4 &modelViewMatrix, bool skin);
};

extern Render renderer;
