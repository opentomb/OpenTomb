#ifndef __OpenTomb__shader_manager__
#define __OpenTomb__shader_manager__

#include "shader_description.h"

// Highest number of lights that will show up in the entity shader.
#define MAX_NUM_LIGHTS 8

class shader_manager {
    unlit_tinted_shader_description *room_shaders[2][2];
    unlit_tinted_shader_description *static_mesh_shader;
    lit_shader_description *entity_shader[MAX_NUM_LIGHTS+1];
    text_shader_description *text;

public:
    shader_manager();
    ~shader_manager();
    
    const lit_shader_description *getEntityShader(unsigned numberOfLights) const;
    
    const unlit_tinted_shader_description *getStaticMeshShader() const { return static_mesh_shader; }
    
    const unlit_tinted_shader_description *getRoomShader(bool isFlickering, bool isWater) const;
    
    const text_shader_description *getTextShader() const { return text; }
};

#endif /* defined(__OpenTomb__shader_manager__) */
