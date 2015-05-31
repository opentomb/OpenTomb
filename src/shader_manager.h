#ifndef __OpenTomb__shader_manager__
#define __OpenTomb__shader_manager__

#include "shader_description.h"

// Highest number of lights that will show up in the entity shader. If you increase this, increase the limit in entity.fsh as well.
#define MAX_NUM_LIGHTS 8

class shader_manager {
    unlit_tinted_shader_description *room_shaders[2][2];
    unlit_tinted_shader_description *static_mesh_shader;
    lit_shader_description *entity_shader[MAX_NUM_LIGHTS];
    gui_shader_description *gui;
    gui_shader_description *gui_textured;

public:
    shader_manager();
    ~shader_manager();
    
    const lit_shader_description *getEntityShader(int numberOfLights) const { return entity_shader[numberOfLights]; }
    
    const unlit_tinted_shader_description *getStaticMeshShader() const { return static_mesh_shader; }
    
    const unlit_tinted_shader_description *getRoomShader(bool isFlickering, bool isWater) const;
    const gui_shader_description *getGuiShader(bool includingTexture) const;
};

#endif /* defined(__OpenTomb__shader_manager__) */
