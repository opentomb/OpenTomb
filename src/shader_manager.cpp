#include <cassert>
#include <sstream>

#include "shader_manager.h"

ShaderManager::ShaderManager()
{
    ShaderStage staticMeshVsh(GL_VERTEX_SHADER, "shaders/static_mesh.vsh");
    ShaderStage staticMeshFsh(GL_FRAGMENT_SHADER, "shaders/static_mesh.fsh");
    //Color mult prog
    m_staticMeshShader = new UnlitTintedShaderDescription(staticMeshVsh, staticMeshFsh);
    
    //Room prog
    ShaderStage roomFragmentShader(GL_FRAGMENT_SHADER, "shaders/room.fsh");
    for (int isWater = 0; isWater < 2; isWater++)
    {
        for (int isFlicker = 0; isFlicker < 2; isFlicker++)
        {
            std::ostringstream stream;
            stream << "#define IS_WATER " << isWater << std::endl;
            stream << "#define IS_FLICKER " << isFlicker << std::endl;

            ShaderStage roomVsh(GL_VERTEX_SHADER, "shaders/room.vsh", stream.str().c_str());
            m_roomShaders[isWater][isFlicker] = new UnlitTintedShaderDescription(roomVsh, roomFragmentShader);
        }
    }
    
    // Entity prog
    ShaderStage entityVertexShader(GL_VERTEX_SHADER, "shaders/entity.vsh");
    ShaderStage entitySkinVertexShader(GL_VERTEX_SHADER, "shaders/entity_skin.vsh");
    for (int i = 0; i <= MAX_NUM_LIGHTS; i++) {
        std::ostringstream stream;
        stream << "#define NUMBER_OF_LIGHTS " << i << std::endl;
        
        ShaderStage fragment(GL_FRAGMENT_SHADER, "shaders/entity.fsh", stream.str().c_str());
        m_entityShader[i][0] = new LitShaderDescription(entityVertexShader, fragment);
        m_entityShader[i][1] = new LitShaderDescription(entitySkinVertexShader, fragment);
    }
    
    // GUI prog
    ShaderStage guiVertexShader(GL_VERTEX_SHADER, "shaders/gui.vsh");
    ShaderStage guiFsh(GL_FRAGMENT_SHADER, "shaders/gui.fsh");
    m_gui = new GuiShaderDescription(guiVertexShader, guiFsh);
    
    ShaderStage guiTexFsh(GL_FRAGMENT_SHADER, "shaders/gui_tex.fsh");
    m_guiTextured = new GuiShaderDescription(guiVertexShader, guiTexFsh);
    
    ShaderStage textVsh(GL_VERTEX_SHADER, "shaders/text.vsh");
    ShaderStage textFsh(GL_FRAGMENT_SHADER, "shaders/text.fsh");
    m_text = new TextShaderDescription(textVsh, textFsh);
    
    ShaderStage spriteVsh(GL_VERTEX_SHADER, "shaders/sprite.vsh");
    ShaderStage spriteFsh(GL_FRAGMENT_SHADER, "shaders/sprite.fsh");
    m_sprites = new SpriteShaderDescription(spriteVsh, spriteFsh);
    
    ShaderStage stencilVsh(GL_VERTEX_SHADER, "shaders/stencil.vsh");
    ShaderStage stencilFsh(GL_FRAGMENT_SHADER, "shaders/stencil.fsh");
    m_stencil = new UnlitShaderDescription(stencilVsh, stencilFsh);

    ShaderStage debugVsh(GL_VERTEX_SHADER, "shaders/debuglines.vsh");
    ShaderStage debugFsh(GL_FRAGMENT_SHADER, "shaders/debuglines.fsh");
    m_debugline = new UnlitShaderDescription(debugVsh, debugFsh);
}
