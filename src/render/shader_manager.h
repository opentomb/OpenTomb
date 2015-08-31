#pragma once

#include "shader_description.h"

#include <cassert>

namespace render
{

namespace
{
// Highest number of lights that will show up in the entity shader.
constexpr int EntityShaderLightsLimit = 8;
} // anonymous namespace


// Class containing all shaders used by OpenTomb. The shader objects
// are owned by this manager and must not be deleted by anyone.

class ShaderManager
{
private:
    UnlitTintedShaderDescription    *m_roomShaders[2][2];
    UnlitTintedShaderDescription    *m_staticMeshShader;
    UnlitShaderDescription          *m_stencil;
    UnlitShaderDescription          *m_debugline;
    LitShaderDescription            *m_entityShader[EntityShaderLightsLimit][2];
    GuiShaderDescription            *m_gui;
    GuiShaderDescription            *m_guiTextured;
    TextShaderDescription           *m_text;
    SpriteShaderDescription         *m_sprites;

public:
    ShaderManager();
    ShaderManager(const ShaderManager&) = delete;
    ShaderManager& operator=(const ShaderManager&) = delete;

    // Deletes the manager. Note: This destructor does nothing and
    // does not delete the shaders, as the manager will only get
    // destroyed when the program exits anyway, in which case all the
    // shaders get destroyed by OpenGL.

    ~ShaderManager() = default;

    LitShaderDescription *getEntityShader(unsigned numberOfLights, bool skin) const
    {
        assert(numberOfLights < EntityShaderLightsLimit);

        return m_entityShader[numberOfLights][skin ? 1 : 0];
    }

    UnlitTintedShaderDescription *getStaticMeshShader() const
    {
        return m_staticMeshShader;
    }

    UnlitShaderDescription *getStencilShader() const
    {
        return m_stencil;
    }

    UnlitShaderDescription *getDebugLineShader() const
    {
        return m_debugline;
    }

    UnlitTintedShaderDescription *getRoomShader(bool isFlickering, bool isWater) const
    {
        return m_roomShaders[isWater ? 1 : 0][isFlickering ? 1 : 0];
    }

    GuiShaderDescription *getGuiShader(bool includingTexture) const
    {
        if(includingTexture)
            return m_guiTextured;
        else
            return m_gui;
    }
    TextShaderDescription *getTextShader() const
    {
        return m_text;
    }
    SpriteShaderDescription *getSpriteShader() const
    {
        return m_sprites;
    }
};

} // namespace render
