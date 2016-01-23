#pragma once

#include "vertex_array.h"
#include "loader/datatypes.h"
#include "world/core/orientedboundingbox.h"

#include <cstdint>
#include <memory>
#include <set>
#include <vector>

// glew must be included BEFORE btIDebugDraw.h
#include <GL/glew.h>

#include <LinearMath/btIDebugDraw.h>

#include <glm/glm.hpp>

namespace world
{
struct Portal;
struct World;
struct Room;
struct RoomSector;
class Camera;
class Entity;
struct Character;

namespace core
{
class Frustum;
struct OrientedBoundingBox;
struct Sprite;
struct BaseMesh;
} // namespace core

namespace animation
{
class Skeleton;
} // namespace animation
} // namespace world

namespace render
{

struct BSPNode;
struct BSPFaceRef;

struct TransparentPolygonReference
{
    const world::core::Polygon *polygon;
    std::shared_ptr< VertexArray > used_vertex_array;
    size_t firstIndex;
    size_t count;
    bool isAnimated;
};

namespace
{
constexpr int DebugDrawerDefaultBufferSize = 128 * 1024;
constexpr int InitFrameVertexBufferSize = 1024 * 1024;
} // anonymous namespace

struct LitShaderDescription;
class Render;

class RenderDebugDrawer : public btIDebugDraw
{
    uint32_t m_debugMode = 0;

    glm::vec3 m_color = {0,0,0};
    std::vector<glm::vec3> m_buffer;

    world::core::OrientedBoundingBox m_obb;

    void addLine(const glm::vec3& start, const glm::vec3& end);
    void addLine(const glm::vec3& start, const glm::vec3& startColor, const glm::vec3& end, const glm::vec3& endColor);

    std::unique_ptr<VertexArray> m_vertexArray{};
    GLuint m_glbuffer = 0;

public:
    // engine debug function
    RenderDebugDrawer();
    ~RenderDebugDrawer();
    bool IsEmpty() const
    {
        return m_buffer.empty();
    }
    void reset();
    void render();
    void setColor(glm::float_t r, glm::float_t g, glm::float_t b)
    {
        m_color[0] = r;
        m_color[1] = g;
        m_color[2] = b;
    }
    void drawAxis(glm::float_t r, const glm::mat4& transform);
    void drawPortal(const world::Portal &p);
    void drawBBox(const world::core::BoundingBox &boundingBox, const glm::mat4 *transform);
    void drawOBB(const world::core::OrientedBoundingBox& obb);
    void drawMeshDebugLines(const std::shared_ptr<world::core::BaseMesh> &mesh, const glm::mat4& transform, const Render& render);
    void drawSkeletalModelDebugLines(const world::animation::Skeleton &skeleton, const glm::mat4& transform, const Render& render);
    static void drawEntityDebugLines(const world::Entity& entity, const Render& render);
    void drawSectorDebugLines(const world::RoomSector& rs);
    static void drawRoomDebugLines(const world::Room& room, const Render& render);

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

extern RenderDebugDrawer debugDrawer;

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
    glm::vec4 fog_color = { 0,0,0,1 };
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

    std::set<const world::Room*> m_renderList{};

    bool m_drawWire = false;
    bool m_drawRoomBoxes = false;
    bool m_drawBoxes = false;
    bool m_drawPortals = false;
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
    bool addRoom(const world::Room *room);
    void setWorld(world::World* world);
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

    void renderEntity(const world::Entity& entity);
    void renderDynamicEntity(const LitShaderDescription& shader, const world::Entity& entity);
    void renderDynamicEntitySkin(const LitShaderDescription& shader, const world::Entity& ent);
    void renderSkeletalModel(const LitShaderDescription& shader, const world::animation::Skeleton& bframe, const glm::mat4 &mvMatrix, const glm::mat4 &mvpMatrix);
    void renderSkeletalModelSkin(const LitShaderDescription& shader, const world::Entity& ent, const glm::mat4 &mvMatrix);
    void renderHair(std::shared_ptr<world::Character> entity);
    void renderSkyBox();
    void renderMesh(const std::shared_ptr<world::core::BaseMesh> &mesh) const;
    void renderPolygonTransparency(loader::BlendingMode currentTransparency, const BSPFaceRef &p, const UnlitTintedShaderDescription& shader) const;
    void renderBSPFrontToBack(loader::BlendingMode currentTransparency, const std::unique_ptr<BSPNode> &root, const UnlitTintedShaderDescription& shader);
    void renderBSPBackToFront(loader::BlendingMode currentTransparency, const std::unique_ptr<BSPNode> &root, const UnlitTintedShaderDescription& shader);
    void renderRoom(const world::Room& room);
    void renderRoomSprites(const world::Room& room) const;

    void processRoom(const world::Room& room);

private:
    const LitShaderDescription *setupEntityLight(const world::Entity& entity, bool skin) const;
};

extern Render renderer;

void fillCrosshairBuffer();
void drawCrosshair();
void renderItem(const world::animation::Skeleton& bf, glm::float_t size, const glm::mat4& mvMatrix, const glm::mat4& guiProjectionMatrix);

} // namespace render
