#include <cassert>
#include <sstream>

#include "shader_manager.h"

ShaderManager::ShaderManager()
{
    //Color mult prog
    static_mesh_shader = new UnlitTintedShaderDescription(ShaderStage(GL_VERTEX_SHADER_ARB, "shaders/static_mesh.vsh"), ShaderStage(GL_FRAGMENT_SHADER_ARB, "shaders/static_mesh.fsh"));
    
    //Room prog
    ShaderStage roomFragmentShader(GL_FRAGMENT_SHADER_ARB, "shaders/room.fsh");
    for (int isWater = 0; isWater < 2; isWater++)
    {
        for (int isFlicker = 0; isFlicker < 2; isFlicker++)
        {
            std::ostringstream stream;
            stream << "#define IS_WATER " << isWater << std::endl;
            stream << "#define IS_FLICKER " << isFlicker << std::endl;
            
            room_shaders[isWater][isFlicker] = new UnlitTintedShaderDescription(ShaderStage(GL_VERTEX_SHADER_ARB, "shaders/room.vsh", stream.str().c_str()), roomFragmentShader);
        }
    }
    
    // Entity prog
    ShaderStage entityVertexShader(GL_VERTEX_SHADER_ARB, "shaders/entity.vsh");
    ShaderStage entitySkinVertexShader(GL_VERTEX_SHADER_ARB, "shaders/entity_skin.vsh");
    for (int i = 0; i <= MAX_NUM_LIGHTS; i++) {
        std::ostringstream stream;
        stream << "#define NUMBER_OF_LIGHTS " << i << std::endl;
        
        ShaderStage fragment(GL_FRAGMENT_SHADER_ARB, "shaders/entity.fsh", stream.str().c_str());
        entity_shader[i][0] = new LitShaderDescription(entityVertexShader, fragment);
        entity_shader[i][1] = new LitShaderDescription(entitySkinVertexShader, fragment);
    }
    
    // GUI prog
    ShaderStage guiVertexShader(GL_VERTEX_SHADER_ARB, "shaders/gui.vsh");
    gui = new GuiShaderDescription(guiVertexShader, ShaderStage(GL_FRAGMENT_SHADER_ARB, "shaders/gui.fsh"));
    gui_textured = new GuiShaderDescription(guiVertexShader, ShaderStage(GL_FRAGMENT_SHADER_ARB, "shaders/gui_tex.fsh"));
    
    text = new TextShaderDescription(ShaderStage(GL_VERTEX_SHADER_ARB, "shaders/text.vsh"), ShaderStage(GL_FRAGMENT_SHADER_ARB, "shaders/text.fsh"));
    sprites = new SpriteShaderDescription(ShaderStage(GL_VERTEX_SHADER_ARB, "shaders/sprite.vsh"), ShaderStage(GL_FRAGMENT_SHADER_ARB, "shaders/sprite.fsh"));
    
    stencil = new UnlitShaderDescription(ShaderStage(GL_VERTEX_SHADER_ARB, "shaders/stencil.vsh"), ShaderStage(GL_FRAGMENT_SHADER_ARB, "shaders/stencil.fsh"));
    
    debugline = new UnlitShaderDescription(ShaderStage(GL_VERTEX_SHADER_ARB, "shaders/debuglines.vsh"), ShaderStage(GL_FRAGMENT_SHADER_ARB, "shaders/debuglines.fsh"));
}

ShaderManager::~ShaderManager()
{
    // Do nothing. All shaders are released by OpenGL anyway.
}

const LitShaderDescription *ShaderManager::getEntityShader(unsigned numberOfLights, bool skin) const {
    assert(numberOfLights <= MAX_NUM_LIGHTS);
    
    return entity_shader[numberOfLights][skin ? 1 : 0];
}

const UnlitTintedShaderDescription *ShaderManager::getRoomShader(bool isFlickering, bool isWater) const
{
    return room_shaders[isWater ? 1 : 0][isFlickering ? 1 : 0];
}

const GuiShaderDescription *ShaderManager::getGuiShader(bool includingTexture) const
{
    if (includingTexture)
        return gui_textured;
    else
        return gui;
}
