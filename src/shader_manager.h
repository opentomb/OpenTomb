#pragma once

#include "shader_description.h"

#include <memory>
#include <assert.h>

// Highest number of lights that will show up in the entity shader.
#define MAX_NUM_LIGHTS 8

class ShaderManager {
private:
    std::shared_ptr<UnlitTintedShaderDescription> m_roomShaders[2][2];
    std::shared_ptr<UnlitTintedShaderDescription> m_staticMeshShader;
    std::shared_ptr<UnlitShaderDescription> m_stencil;
    std::shared_ptr<UnlitShaderDescription> m_debugline;
    std::shared_ptr<LitShaderDescription> m_entityShader[MAX_NUM_LIGHTS+1][2];
    std::shared_ptr<GuiShaderDescription> m_gui;
    std::shared_ptr<GuiShaderDescription> m_guiTextured;
    std::shared_ptr<TextShaderDescription> m_text;
    std::shared_ptr<SpriteShaderDescription> m_sprites;

public:
    ShaderManager();
    ~ShaderManager() = default;
    
    std::shared_ptr<LitShaderDescription> getEntityShader(unsigned numberOfLights, bool skin) const {
        assert(numberOfLights <= MAX_NUM_LIGHTS);

        return m_entityShader[numberOfLights][skin ? 1 : 0];
    }
    
    std::shared_ptr<UnlitTintedShaderDescription> getStaticMeshShader() const { return m_staticMeshShader; }
    
    std::shared_ptr<UnlitShaderDescription> getStencilShader() const { return m_stencil; }
    
    std::shared_ptr<UnlitShaderDescription> getDebugLineShader() const { return m_debugline; }
    
    std::shared_ptr<UnlitTintedShaderDescription> getRoomShader(bool isFlickering, bool isWater) const
    {
        return m_roomShaders[isWater ? 1 : 0][isFlickering ? 1 : 0];
    }

    std::shared_ptr<GuiShaderDescription> getGuiShader(bool includingTexture) const
    {
        if (includingTexture)
            return m_guiTextured;
        else
            return m_gui;
    }
    std::shared_ptr<TextShaderDescription> getTextShader() const { return m_text; }
    std::shared_ptr<SpriteShaderDescription> getSpriteShader() const { return m_sprites; }
};
