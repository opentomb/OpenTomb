#ifndef __OpenTomb__shader_manager__
#define __OpenTomb__shader_manager__

#include "shader_description.h"

// Highest number of lights that will show up in the entity shader.
#define MAX_NUM_LIGHTS 8

class ShaderManager {
    unlit_tinted_shader_description *room_shaders[2][2];
    unlit_tinted_shader_description *static_mesh_shader;
    unlit_shader_description *stencil;
    unlit_shader_description *debugline;
    lit_shader_description *entity_shader[MAX_NUM_LIGHTS+1][2];
    gui_shader_description *gui;
    gui_shader_description *gui_textured;
    text_shader_description *text;
    sprite_shader_description *sprites;

public:
    ShaderManager();
    ~ShaderManager();
    
    const lit_shader_description *getEntityShader(unsigned numberOfLights, bool skin) const;
    
    const unlit_tinted_shader_description *getStaticMeshShader() const { return static_mesh_shader; }
    
    const unlit_shader_description *getStencilShader() const { return stencil; }
    
    const unlit_shader_description *getDebugLineShader() const { return debugline; }
    
    const unlit_tinted_shader_description *getRoomShader(bool isFlickering, bool isWater) const;
    const gui_shader_description *getGuiShader(bool includingTexture) const;
    const text_shader_description *getTextShader() const { return text; }
    const sprite_shader_description *getSpriteShader() const { return sprites; }
};

#endif /* defined(__OpenTomb__shader_manager__) */
