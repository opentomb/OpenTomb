#include <sstream>

#include "shader_manager.h"

shader_manager::shader_manager()
{
    //Color mult prog
    static_mesh_shader = new unlit_tinted_shader_description("shaders/static_mesh.vsh", "shaders/static_mesh.fsh", 0);
    
    //Room prog
    GLhandleARB roomFragmentShader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
    loadShaderFromFile(roomFragmentShader, "shaders/room.fsh", 0);
    for (int isWater = 0; isWater < 2; isWater++)
    {
        for (int isFlicker = 0; isFlicker < 2; isFlicker++)
        {
            std::ostringstream stream;
            stream << "#define IS_WATER " << isWater << std::endl;
            stream << "#define IS_FLICKER " << isFlicker << std::endl;
            
            room_shaders[isWater][isFlicker] = new unlit_tinted_shader_description("shaders/room.vsh", roomFragmentShader, stream.str().c_str());
        }
    }
    glDeleteObjectARB(roomFragmentShader);
    
    // Entity prog
    GLhandleARB entityVertexShader = glCreateShaderObjectARB(GL_VERTEX_SHADER);
    loadShaderFromFile(entityVertexShader, "shaders/entity.vsh", 0);
    for (int i = 0; i < MAX_NUM_LIGHTS; i++) {
        std::ostringstream stream;
        stream << "#define NUMBER_OF_LIGHTS " << i << std::endl;
        
        entity_shader[i] = new lit_shader_description(entityVertexShader, "shaders/entity.fsh", stream.str().c_str());
    }
    glDeleteObjectARB(entityVertexShader);
    
    // GUI prog
    GLhandleARB guiVertexShader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
    loadShaderFromFile(guiVertexShader, "shaders/gui.vsh", 0);
    gui = new gui_shader_description(guiVertexShader, "shaders/gui.fsh");
    gui_textured = new gui_shader_description(guiVertexShader, "shaders/gui_tex.fsh");
    
    text = new text_shader_description("shaders/text.vsh", "shaders/text.fsh");
    sprites = new sprite_shader_description("shaders/sprite.vsh", "shaders/sprite.fsh");
}

shader_manager::~shader_manager()
{
    // Do nothing. All shaders are released by OpenGL anyway.
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
