#pragma once

#include "shader_description.h"

#include <memory>

// Highest number of lights that will show up in the entity shader.
#define MAX_NUM_LIGHTS 8

class ShaderManager {
private:
    UnlitTintedShaderDescription *m_roomShaders[2][2];
    std::unique_ptr<UnlitTintedShaderDescription> m_staticMeshShader;
    std::unique_ptr<UnlitShaderDescription> m_stencil;
    std::unique_ptr<UnlitShaderDescription> m_debugline;
    LitShaderDescription *m_entityShader[MAX_NUM_LIGHTS+1][2];
    std::unique_ptr<GuiShaderDescription> m_gui;
    std::unique_ptr<GuiShaderDescription> m_guiTextured;
    std::unique_ptr<TextShaderDescription> m_text;
    std::unique_ptr<SpriteShaderDescription> m_sprites;

public:
    ShaderManager();
    ~ShaderManager();
    
    const LitShaderDescription *getEntityShader(unsigned numberOfLights, bool skin) const;
    
    const std::unique_ptr<UnlitTintedShaderDescription>& getStaticMeshShader() const { return m_staticMeshShader; }
    
    const std::unique_ptr<UnlitShaderDescription>& getStencilShader() const { return m_stencil; }
    
    const std::unique_ptr<UnlitShaderDescription>& getDebugLineShader() const { return m_debugline; }
    
    const UnlitTintedShaderDescription *getRoomShader(bool isFlickering, bool isWater) const;
    const std::unique_ptr<GuiShaderDescription> &getGuiShader(bool includingTexture) const;
    const std::unique_ptr<TextShaderDescription>& getTextShader() const { return m_text; }
    const std::unique_ptr<SpriteShaderDescription>& getSpriteShader() const { return m_sprites; }
};
