#include <cassert>
#include <sstream>

#include "shader_manager.h"

ShaderManager::ShaderManager()
{
    ShaderStage* staticMeshVsh = ShaderStage::create(GL_VERTEX_SHADER, "shaders/static_mesh.vsh");
    ShaderStage* staticMeshFsh = ShaderStage::create(GL_FRAGMENT_SHADER, "shaders/static_mesh.fsh");
    m_stages.emplace_back(staticMeshVsh);
    m_stages.emplace_back(staticMeshFsh);
    //Color mult prog
    m_staticMeshShader = std::make_shared<UnlitTintedShaderDescription>(*staticMeshVsh, *staticMeshFsh);
    
    //Room prog
    ShaderStage* roomFragmentShader = ShaderStage::create(GL_FRAGMENT_SHADER, "shaders/room.fsh");
    m_stages.emplace_back(roomFragmentShader);
    for (int isWater = 0; isWater < 2; isWater++)
    {
        for (int isFlicker = 0; isFlicker < 2; isFlicker++)
        {
            std::ostringstream stream;
            stream << "#define IS_WATER " << isWater << std::endl;
            stream << "#define IS_FLICKER " << isFlicker << std::endl;

            ShaderStage* roomVsh = ShaderStage::create(GL_VERTEX_SHADER, "shaders/room.vsh", stream.str().c_str());
            m_stages.emplace_back(roomVsh);
            m_roomShaders[isWater][isFlicker] = std::make_shared<UnlitTintedShaderDescription>(*roomVsh, *roomFragmentShader);
        }
    }
    
    // Entity prog
    ShaderStage* entityVertexShader = ShaderStage::create(GL_VERTEX_SHADER, "shaders/entity.vsh");
    ShaderStage* entitySkinVertexShader = ShaderStage::create(GL_VERTEX_SHADER, "shaders/entity_skin.vsh");
    m_stages.emplace_back(entityVertexShader);
    m_stages.emplace_back(entitySkinVertexShader);
    for (int i = 0; i <= MAX_NUM_LIGHTS; i++) {
        std::ostringstream stream;
        stream << "#define NUMBER_OF_LIGHTS " << i << std::endl;
        
        ShaderStage* fragment = ShaderStage::create(GL_FRAGMENT_SHADER, "shaders/entity.fsh", stream.str().c_str());
        m_stages.emplace_back(fragment);
        m_entityShader[i][0] = std::make_shared<LitShaderDescription>(*entityVertexShader, *fragment);
        m_entityShader[i][1] = std::make_shared<LitShaderDescription>(*entitySkinVertexShader, *fragment);
    }
    
    // GUI prog
    ShaderStage* guiVertexShader = ShaderStage::create(GL_VERTEX_SHADER, "shaders/gui.vsh");
    ShaderStage* guiFsh = ShaderStage::create(GL_FRAGMENT_SHADER, "shaders/gui.fsh");
    m_stages.emplace_back(guiVertexShader);
    m_stages.emplace_back(guiFsh);
    m_gui = std::make_shared<GuiShaderDescription>(*guiVertexShader, *guiFsh);
    
    ShaderStage* guiTexFsh = ShaderStage::create(GL_FRAGMENT_SHADER, "shaders/gui_tex.fsh");
    m_stages.emplace_back(guiTexFsh);
    m_guiTextured = std::make_shared<GuiShaderDescription>(*guiVertexShader, *guiTexFsh);
    
    ShaderStage* textVsh = ShaderStage::create(GL_VERTEX_SHADER, "shaders/text.vsh");
    ShaderStage* textFsh = ShaderStage::create(GL_FRAGMENT_SHADER, "shaders/text.fsh");
    m_stages.emplace_back(textVsh);
    m_stages.emplace_back(textFsh);
    m_text = std::make_shared<TextShaderDescription>(*textVsh, *textFsh);
    
    ShaderStage* spriteVsh = ShaderStage::create(GL_VERTEX_SHADER, "shaders/sprite.vsh");
    ShaderStage* spriteFsh = ShaderStage::create(GL_FRAGMENT_SHADER, "shaders/sprite.fsh");
    m_stages.emplace_back(spriteVsh);
    m_stages.emplace_back(spriteFsh);
    m_sprites = std::make_shared<SpriteShaderDescription>(*spriteVsh, *spriteFsh);
    
    ShaderStage* stencilVsh = ShaderStage::create(GL_VERTEX_SHADER, "shaders/stencil.vsh");
    ShaderStage* stencilFsh = ShaderStage::create(GL_FRAGMENT_SHADER, "shaders/stencil.fsh");
    m_stages.emplace_back(stencilVsh);
    m_stages.emplace_back(stencilFsh);
    m_stencil = std::make_shared<UnlitShaderDescription>(*stencilVsh, *stencilFsh);

    ShaderStage* debugVsh = ShaderStage::create(GL_VERTEX_SHADER, "shaders/debuglines.vsh");
    ShaderStage* debugFsh = ShaderStage::create(GL_FRAGMENT_SHADER, "shaders/debuglines.fsh");
    m_stages.emplace_back(debugVsh);
    m_stages.emplace_back(debugFsh);
    m_debugline = std::make_shared<UnlitShaderDescription>(*debugVsh, *debugFsh);
}






