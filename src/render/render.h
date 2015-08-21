#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <vector>

// glew must be included BEFORE btIDebugDraw.h
#include <GL/glew.h>

#include <LinearMath/btScalar.h>
#include <LinearMath/btIDebugDraw.h>

#include "util/matrix4.h"
#include "vertex_array.h"
#include "loader/datatypes.h"
#include "world/core/obb.h"

namespace world
{
struct Portal;
struct World;
struct Room;
struct RoomSector;
class Camera;
struct Entity;

namespace core
{
struct Frustum;
struct OBB;
struct Sprite;
struct BaseMesh;
struct SSBoneFrame;
} // namespace core
} // namespace world

struct Character;

namespace render
{

struct BSPNode;
struct BSPFaceRef;

namespace
{
constexpr int DebugDrawerDefaultBufferSize = 128 * 1024;
constexpr int InitFrameVertexBufferSize = 1024 * 1024;
} // anonymous namespace

#define STENCIL_FRUSTUM 1

struct LitShaderDescription;
class Render;

class RenderDebugDrawer : public btIDebugDraw
{
    uint32_t m_debugMode = 0;

    std::array<GLfloat, 3> m_color{ {0,0,0} };
    std::vector<std::array<GLfloat, 3>> m_buffer;

    world::core::OBB m_obb;

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
    void drawPortal(const world::Portal &p);
    void drawFrustum(const world::core::Frustum &f);
    void drawBBox(const btVector3 &bb_min, const btVector3 &bb_max, const btTransform *transform);
    void drawOBB(const world::core::OBB& obb);
    void drawMeshDebugLines(const std::shared_ptr<world::core::BaseMesh> &mesh, const btTransform& transform, const std::vector<btVector3> &overrideVertices, const std::vector<btVector3> &overrideNormals, Render* render);
    void drawSkeletalModelDebugLines(world::core::SSBoneFrame *bframe, const btTransform& transform, Render *render);
    void drawEntityDebugLines(world::Entity *entity, Render *render);
    void drawSectorDebugLines(world::RoomSector *rs);
    void drawRoomDebugLines(const world::Room *room, Render *render, const world::Camera& cam);

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

class  ShaderManager;
struct UnlitTintedShaderDescription;

class Render
{
    friend class RenderDebugDrawer;
private:
    bool m_blocked = true;
    world::World* m_world = nullptr;
    world::Camera* m_cam = nullptr;
    RenderSettings m_settings;
    std::unique_ptr<ShaderManager> m_shaderManager;

    std::vector<world::Room*> m_renderList{};

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
    bool addRoom(world::Room *room);
    void setWorld(world::World* m_world);
    void resetWorld()
    {
        m_world = nullptr;
        m_renderList.clear();
    }

    const std::unique_ptr<ShaderManager>& shaderManager()
    {
        return m_shaderManager;
    }
    world::Camera* camera()
    {
        return m_cam;
    }
    void setCamera(world::Camera* cam)
    {
        m_cam = cam;
    }

    world::World* world()
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

    void renderEntity(world::Entity *entity, const util::matrix4 &modelViewMatrix, const util::matrix4 &modelViewProjectionMatrix, const util::matrix4 &projection);
    void renderDynamicEntity(const LitShaderDescription *shader, world::Entity *entity, const util::matrix4 &modelViewMatrix, const util::matrix4 &modelViewProjectionMatrix);
    void renderDynamicEntitySkin(const LitShaderDescription *shader, world::Entity *ent, const util::matrix4 &mvMatrix, const util::matrix4 &pMatrix);
    void renderSkeletalModel(const LitShaderDescription *shader, world::core::SSBoneFrame* bframe, const util::matrix4 &mvMatrix, const util::matrix4 &mvpMatrix);
    void renderSkeletalModelSkin(const LitShaderDescription *shader, world::Entity *ent, const util::matrix4 &mvMatrix, const util::matrix4 &pMatrix);
    void renderHair(std::shared_ptr<Character> entity, const util::matrix4 &modelViewMatrix, const util::matrix4 & modelViewProjectionMatrix);
    void renderSkyBox(const util::matrix4& matrix);
    void renderMesh(const std::shared_ptr<world::core::BaseMesh> &mesh);
    void renderPolygonTransparency(loader::BlendingMode& currentTransparency, const BSPFaceRef &p, const UnlitTintedShaderDescription *shader);
    void renderBSPFrontToBack(loader::BlendingMode& currentTransparency, const std::unique_ptr<BSPNode> &root, const UnlitTintedShaderDescription *shader);
    void renderBSPBackToFront(loader::BlendingMode& currentTransparency, const std::unique_ptr<BSPNode> &root, const UnlitTintedShaderDescription *shader);
    void renderRoom(const world::Room *room, const util::matrix4 &matrix, const util::matrix4 &modelViewProjectionMatrix, const util::matrix4 &projection);
    void renderRoomSprites(const world::Room *room, const util::matrix4 &modelViewMatrix, const util::matrix4 &projectionMatrix);

    int processRoom(world::Portal* portal, const world::core::Frustum& frus);

private:
    const LitShaderDescription *setupEntityLight(world::Entity *entity, const util::matrix4 &modelViewMatrix, bool skin);
};

extern Render renderer;

} // namespace render
