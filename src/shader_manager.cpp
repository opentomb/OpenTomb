#include <cassert>
#include <sstream>

#include "shader_manager.h"

ShaderManager::ShaderManager()
{
    //Color mult prog
    m_staticMeshShader = std::make_shared<UnlitTintedShaderDescription>(ShaderStage(GL_VERTEX_SHADER_ARB, "shaders/static_mesh.vsh"), ShaderStage(GL_FRAGMENT_SHADER_ARB, "shaders/static_mesh.fsh"));
    
    //Room prog
    ShaderStage roomFragmentShader(GL_FRAGMENT_SHADER_ARB, "shaders/room.fsh");
    for (int isWater = 0; isWater < 2; isWater++)
    {
        for (int isFlicker = 0; isFlicker < 2; isFlicker++)
        {
            std::ostringstream stream;
            stream << "#define IS_WATER " << isWater << std::endl;
            stream << "#define IS_FLICKER " << isFlicker << std::endl;
            
            m_roomShaders[isWater][isFlicker] = std::make_shared<UnlitTintedShaderDescription>(ShaderStage(GL_VERTEX_SHADER_ARB, "shaders/room.vsh", stream.str().c_str()), roomFragmentShader);
        }
    }
    
    // Entity prog
    ShaderStage entityVertexShader(GL_VERTEX_SHADER_ARB, "shaders/entity.vsh");
    ShaderStage entitySkinVertexShader(GL_VERTEX_SHADER_ARB, "shaders/entity_skin.vsh");
    for (int i = 0; i <= MAX_NUM_LIGHTS; i++) {
        std::ostringstream stream;
        stream << "#define NUMBER_OF_LIGHTS " << i << std::endl;
        
        ShaderStage fragment(GL_FRAGMENT_SHADER_ARB, "shaders/entity.fsh", stream.str().c_str());
        m_entityShader[i][0] = std::make_shared<LitShaderDescription>(entityVertexShader, fragment);
        m_entityShader[i][1] = std::make_shared<LitShaderDescription>(entitySkinVertexShader, fragment);
    }
    
    // GUI prog
    ShaderStage guiVertexShader(GL_VERTEX_SHADER_ARB, "shaders/gui.vsh");
    m_gui = std::make_shared<GuiShaderDescription>(guiVertexShader, ShaderStage(GL_FRAGMENT_SHADER_ARB, "shaders/gui.fsh"));
    m_guiTextured = std::make_shared<GuiShaderDescription>(guiVertexShader, ShaderStage(GL_FRAGMENT_SHADER_ARB, "shaders/gui_tex.fsh"));
    
    m_text = std::make_shared<TextShaderDescription>(ShaderStage(GL_VERTEX_SHADER_ARB, "shaders/text.vsh"), ShaderStage(GL_FRAGMENT_SHADER_ARB, "shaders/text.fsh"));
    m_sprites = std::make_shared<SpriteShaderDescription>(ShaderStage(GL_VERTEX_SHADER_ARB, "shaders/sprite.vsh"), ShaderStage(GL_FRAGMENT_SHADER_ARB, "shaders/sprite.fsh"));
    
    m_stencil = std::make_shared<UnlitShaderDescription>(ShaderStage(GL_VERTEX_SHADER_ARB, "shaders/stencil.vsh"), ShaderStage(GL_FRAGMENT_SHADER_ARB, "shaders/stencil.fsh"));
    
    m_debugline = std::make_shared<UnlitShaderDescription>(ShaderStage(GL_VERTEX_SHADER_ARB, "shaders/debuglines.vsh"), ShaderStage(GL_FRAGMENT_SHADER_ARB, "shaders/debuglines.fsh"));
}






