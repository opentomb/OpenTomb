#ifndef __OpenTomb__shader_manager__
#define __OpenTomb__shader_manager__

#include "shader_description.h"

// Highest number of lights that will show up in the entity shader.
#define MAX_NUM_LIGHTS 8

class ShaderManager {
    UnlitTintedShaderDescription *room_shaders[2][2];
    UnlitTintedShaderDescription *static_mesh_shader;
    UnlitShaderDescription *stencil;
    UnlitShaderDescription *debugline;
    LitShaderDescription *entity_shader[MAX_NUM_LIGHTS+1][2];
    GuiShaderDescription *gui;
    GuiShaderDescription *gui_textured;
    TextShaderDescription *text;
    SpriteShaderDescription *sprites;

public:
    ShaderManager();
    ~ShaderManager();
    
    const LitShaderDescription *getEntityShader(unsigned numberOfLights, bool skin) const;
    
    const UnlitTintedShaderDescription *getStaticMeshShader() const { return static_mesh_shader; }
    
    const UnlitShaderDescription *getStencilShader() const { return stencil; }
    
    const UnlitShaderDescription *getDebugLineShader() const { return debugline; }
    
    const UnlitTintedShaderDescription *getRoomShader(bool isFlickering, bool isWater) const;
    const GuiShaderDescription *getGuiShader(bool includingTexture) const;
    const TextShaderDescription *getTextShader() const { return text; }
    const SpriteShaderDescription *getSpriteShader() const { return sprites; }
};

#endif /* defined(__OpenTomb__shader_manager__) */
