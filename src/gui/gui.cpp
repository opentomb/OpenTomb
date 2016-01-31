#include "gui.h"

#include "console.h"
#include "engine/engine.h"
#include "engine/system.h"
#include "fadermanager.h"
#include "gl_font.h"
#include "inventory.h"
#include "progressbarmanager.h"
#include "render/render.h"
#include "render/shader_description.h"
#include "render/shader_manager.h"
#include "render/vertex_array.h"
#include "world/camera.h"
#include "world/character.h"

#include <glm/gtc/type_ptr.hpp>

#include <SDL2/SDL_video.h>

namespace gui
{
Gui::Gui(engine::Engine* engine, boost::property_tree::ptree& config)
    : m_engine(engine)
    , m_console(engine, util::getSettingChild(config, "console"))
    , m_notifier(engine)
    , m_faders(engine)
    , m_progressBars(engine)
    , m_textlineManager(engine)
    , m_fontManager(engine)
{
}

Gui::~Gui()
{
    if(m_rectanglePositionBuffer != 0)
    {
        glDeleteBuffers(1, &m_rectanglePositionBuffer);
        glDeleteBuffers(1, &m_rectangleColorBuffer);
    }
}

bool Gui::update()
{
    m_fontManager.update();

    if(!m_console.isVisible() && m_engine->m_controlState.m_guiInventory)
    {
        if(m_engine->m_world.m_character &&
           m_engine->m_world.m_character->inventory().getCurrentState() == InventoryManager::InventoryState::Disabled)
        {
            m_engine->m_world.m_character->inventory().disable();
            m_engine->m_world.m_character->inventory().send(InventoryManager::InventoryState::Open);
        }
        if(m_engine->m_world.m_character->inventory().getCurrentState() == InventoryManager::InventoryState::Idle)
        {
            m_engine->m_world.m_character->inventory().send(InventoryManager::InventoryState::Closed);
        }
    }

    if(m_console.isVisible() || !m_engine->m_world.m_character || m_engine->m_world.m_character->inventory().getCurrentState() != InventoryManager::InventoryState::Disabled)
    {
        return true;
    }
    return false;
}

void Gui::resize()
{
    m_engine->m_gui.m_textlineManager.resizeTextLines();

    m_progressBars.resize();

    m_fontManager.resize();

    /* let us update console too */
    m_console.setSpacing(m_console.spacing());
    m_engine->renderer.fillCrosshairBuffer();
}

void Gui::render()
{
    glFrontFace(GL_CCW);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    glDisable(GL_DEPTH_TEST);
    if(m_engine->m_screenInfo.show_debuginfo)
        m_engine->renderer.drawCrosshair();
    m_progressBars.draw(m_engine->m_world);
    m_faders.drawFaders();
    m_engine->m_gui.m_textlineManager.renderStrings();
    m_console.draw();

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
}

/*
 * Other GUI options
 */
void Gui::switchGLMode(bool is_gui)
{
    if(is_gui)                                                             // set gui coordinate system
    {
        const glm::float_t far_dist = 4096.0f;
        const glm::float_t near_dist = -1.0f;

        m_guiProjectionMatrix = glm::mat4(1.0f);                                        // identity matrix
        m_guiProjectionMatrix[0][0] = 2.0f / static_cast<glm::float_t>(m_engine->m_screenInfo.w);
        m_guiProjectionMatrix[1][1] = 2.0f / static_cast<glm::float_t>(m_engine->m_screenInfo.h);
        m_guiProjectionMatrix[2][2] = -2.0f / (far_dist - near_dist);
        m_guiProjectionMatrix[3][0] = -1.0f;
        m_guiProjectionMatrix[3][1] = -1.0f;
        m_guiProjectionMatrix[3][2] = -(far_dist + near_dist) / (far_dist - near_dist);
    }
    else                                                                        // set camera coordinate system
    {
        m_guiProjectionMatrix = m_engine->m_camera.getProjection();
    }
}

void Gui::drawInventory()
{
    if(!m_engine->m_world.m_character)
        return;

    //if (!main_inventory_menu->IsVisible())
    m_engine->m_world.m_character->inventory().frame();
    if(m_engine->m_world.m_character->inventory().getCurrentState() == InventoryManager::InventoryState::Disabled)
    {
        return;
    }

    glClear(GL_DEPTH_BUFFER_BIT);

    glPushAttrib(GL_ENABLE_BIT | GL_PIXEL_MODE_BIT | GL_COLOR_BUFFER_BIT);

    glPolygonMode(GL_FRONT, GL_FILL);
    glFrontFace(GL_CCW);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_ALPHA_TEST);
    glDepthMask(GL_FALSE);

    // Background

    glm::vec4 upper_color{ 0, 0, 0, 0.45f };
    glm::vec4 lower_color{ 0, 0, 0, 0.75f };

    drawRect(0.0, 0.0, static_cast<glm::float_t>(m_engine->m_screenInfo.w), static_cast<glm::float_t>(m_engine->m_screenInfo.h),
             upper_color, upper_color, lower_color, lower_color,
             loader::BlendingMode::Opaque);

    glDepthMask(GL_TRUE);
    glPopAttrib();

    switchGLMode(false);
    //main_inventory_menu->Render(); //engine_world.character->character->inventory
    m_engine->m_world.m_character->inventory().render();
    switchGLMode(true);
}

void Gui::drawLoadScreen(int value)
{
    BOOST_LOG_TRIVIAL(debug) << "Loading screen progress: " << value;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    switchGLMode(true);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    glPixelStorei(GL_UNPACK_LSB_FIRST, GL_FALSE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    m_faders.showLoadScreenFader();
    m_progressBars.showLoading(value);

    glDepthMask(GL_TRUE);

    switchGLMode(false);

    SDL_GL_SwapWindow(m_engine->m_window);
}

/**
 * Draws simple colored rectangle with given parameters.
 */
void Gui::drawRect(glm::float_t x, glm::float_t y,
                   glm::float_t width, glm::float_t height,
                   const glm::vec4& colorUpperLeft, const glm::vec4& colorUpperRight,
                   const glm::vec4& colorLowerLeft, const glm::vec4& colorLowerRight,
                   const loader::BlendingMode blendMode,
                   const GLuint texture)
{
    switch(blendMode)
    {
        case loader::BlendingMode::Hide:
            return;
        case loader::BlendingMode::Multiply:
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            break;
        case loader::BlendingMode::SimpleShade:
            glBlendFunc(GL_ONE_MINUS_SRC_COLOR, GL_ONE_MINUS_SRC_ALPHA);
            break;
        case loader::BlendingMode::Screen:
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            break;
        default:
        case loader::BlendingMode::Opaque:
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            break;
    };

    if(m_rectanglePositionBuffer == 0)
    {
        glGenBuffers(1, &m_rectanglePositionBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, m_rectanglePositionBuffer);
        static const glm::vec2 rectCoords[4]{
            {0, 0},
            {1, 0},
            {1, 1},
            {0, 1}
        };
        glBufferData(GL_ARRAY_BUFFER, sizeof(rectCoords), rectCoords, GL_STATIC_DRAW);

        glGenBuffers(1, &m_rectangleColorBuffer);

        render::VertexArrayAttribute attribs[] = {
            render::VertexArrayAttribute(render::GuiShaderDescription::position, 2, GL_FLOAT, false, m_rectanglePositionBuffer, sizeof(glm::vec2), 0),
            render::VertexArrayAttribute(render::GuiShaderDescription::color, 4, GL_FLOAT, false, m_rectangleColorBuffer, sizeof(glm::vec4), 0),
        };
        m_rectangleArray.reset(new render::VertexArray(0, 2, attribs));
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_rectangleColorBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * 4, nullptr, GL_STREAM_DRAW);
    glm::vec4* rectColors = static_cast<glm::vec4*>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
    rectColors[0] = colorLowerLeft;
    rectColors[1] = colorLowerRight;
    rectColors[2] = colorUpperRight;
    rectColors[3] = colorUpperLeft;
    glUnmapBuffer(GL_ARRAY_BUFFER);

    const glm::vec2 offset{ x / (m_engine->m_screenInfo.w*0.5f) - 1.f, y / (m_engine->m_screenInfo.h*0.5f) - 1.f };
    const glm::vec2 factor{ width / m_engine->m_screenInfo.w * 2.0f, height / m_engine->m_screenInfo.h * 2.0f };

    render::GuiShaderDescription *shader = m_engine->renderer.shaderManager()->getGuiShader(texture != 0);
    glUseProgram(shader->program);
    glUniform1i(shader->sampler, 0);
    if(texture)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
    }
    glUniform2fv(shader->offset, 1, glm::value_ptr(offset));
    glUniform2fv(shader->factor, 1, glm::value_ptr(factor));

    m_rectangleArray->bind();

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}
} // namespace gui
