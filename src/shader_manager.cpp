#include <cassert>
#include <sstream>

#include "shader_manager.h"

shader_manager::shader_manager()
{
    //Color mult prog
    static_mesh_shader = new unlit_tinted_shader_description(shader_stage(GL_VERTEX_SHADER_ARB, "shaders/static_mesh.vsh"), shader_stage(GL_FRAGMENT_SHADER_ARB, "shaders/static_mesh.fsh"));
    
    //Room prog
    shader_stage roomFragmentShader(GL_FRAGMENT_SHADER_ARB, "shaders/room.fsh");
    for (int isWater = 0; isWater < 2; isWater++)
    {
        for (int isFlicker = 0; isFlicker < 2; isFlicker++)
        {
            std::ostringstream stream;
            stream << "#define IS_WATER " << isWater << std::endl;
            stream << "#define IS_FLICKER " << isFlicker << std::endl;
            
            room_shaders[isWater][isFlicker] = new unlit_tinted_shader_description(shader_stage(GL_VERTEX_SHADER_ARB, "shaders/room.vsh", stream.str().c_str()), roomFragmentShader);
        }
    }
    
    // Entity prog
    shader_stage entityVertexShader(GL_VERTEX_SHADER_ARB, "shaders/entity.vsh");
    shader_stage entitySkinVertexShader(GL_VERTEX_SHADER_ARB, "shaders/entity_skin.vsh");
    for (int i = 0; i <= MAX_NUM_LIGHTS; i++) {
        std::ostringstream stream;
        stream << "#define NUMBER_OF_LIGHTS " << i << std::endl;
        
        shader_stage fragment(GL_FRAGMENT_SHADER_ARB, "shaders/entity.fsh", stream.str().c_str());
        entity_shader[i][0] = new lit_shader_description(entityVertexShader, fragment);
        entity_shader[i][1] = new lit_shader_description(entitySkinVertexShader, fragment);
    }
    
    // GUI prog
    shader_stage guiVertexShader(GL_VERTEX_SHADER_ARB, "shaders/gui.vsh");
    gui = new gui_shader_description(guiVertexShader, shader_stage(GL_FRAGMENT_SHADER_ARB, "shaders/gui.fsh"));
    gui_textured = new gui_shader_description(guiVertexShader, shader_stage(GL_FRAGMENT_SHADER_ARB, "shaders/gui_tex.fsh"));
    
    text = new text_shader_description(shader_stage(GL_VERTEX_SHADER_ARB, "shaders/text.vsh"), shader_stage(GL_FRAGMENT_SHADER_ARB, "shaders/text.fsh"));
    sprites = new sprite_shader_description(shader_stage(GL_VERTEX_SHADER_ARB, "shaders/sprite.vsh"), shader_stage(GL_FRAGMENT_SHADER_ARB, "shaders/sprite.fsh"));
    
    stencil = new unlit_shader_description(shader_stage(GL_VERTEX_SHADER_ARB, "shaders/stencil.vsh"), shader_stage(GL_FRAGMENT_SHADER_ARB, "shaders/stencil.fsh"));
}

shader_manager::~shader_manager()
{
    // Do nothing. All shaders are released by OpenGL anyway.
}

const lit_shader_description *shader_manager::getEntityShader(unsigned numberOfLights, bool skin) const {
    assert(numberOfLights <= MAX_NUM_LIGHTS);
    
    return entity_shader[numberOfLights][skin ? 1 : 0];
}

const unlit_tinted_shader_description *shader_manager::getRoomShader(bool isFlickering, bool isWater) const
{
    return room_shaders[isWater ? 1 : 0][isFlickering ? 1 : 0];
}

const gui_shader_description *shader_manager::getGuiShader(bool includingTexture) const
{
    if (includingTexture)
        return gui_textured;
    else
        return gui;
}
