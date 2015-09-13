#include "gui.h"

#include <cstdint>
#include <map>

#ifdef __APPLE_CC__
#include <ImageIO/ImageIO.h>
#else
#include <SDL2/SDL_image.h>
#endif

#include "character_controller.h"
#include "console.h"
#include "engine/engine.h"
#include "engine/system.h"
#include "fader.h"
#include "gl_font.h"
#include "inventory.h"
#include "itemnotifier.h"
#include "progressbar.h"
#include "render/render.h"
#include "render/shader_description.h"
#include "render/shader_manager.h"
#include "render/vertex_array.h"
#include "strings.h"
#include "util/vmath.h"
#include "world/animation/animation.h"
#include "world/camera.h"
#include "world/character.h"

namespace engine
{
extern SDL_Window  *sdl_window;
} // namespace engine

namespace gui
{

std::unique_ptr<FontManager> fontManager = nullptr;

util::matrix4 guiProjectionMatrix = util::matrix4();

void init()
{
    initBars();
    initFaders();
    initNotifier();

    render::fillCrosshairBuffer();

    //main_inventory_menu = new gui_InventoryMenu();
    main_inventory_manager = new InventoryManager();
}

void initFontManager()
{
    fontManager.reset(new FontManager());
}

void destroy()
{
    destroyFaders();

    /*if(main_inventory_menu)
    {
        delete main_inventory_menu;
        main_inventory_menu = NULL;
    }*/

    if(main_inventory_manager)
    {
        delete main_inventory_manager;
        main_inventory_manager = nullptr;
    }

    fontManager.reset();
}

bool update()
{
    if(fontManager != nullptr)
    {
        fontManager->Update();
    }

    if(!Console::instance().isVisible() && engine::control_states.gui_inventory && main_inventory_manager)
    {
        if(engine::engine_world.character &&
           (main_inventory_manager->getCurrentState() == InventoryManager::InventoryState::Disabled))
        {
            main_inventory_manager->setInventory(&engine::engine_world.character->m_inventory);
            main_inventory_manager->send(InventoryManager::InventoryState::Open);
        }
        if(main_inventory_manager->getCurrentState() == InventoryManager::InventoryState::Idle)
        {
            main_inventory_manager->send(InventoryManager::InventoryState::Closed);
        }
    }

    if(Console::instance().isVisible() || main_inventory_manager->getCurrentState() != InventoryManager::InventoryState::Disabled)
    {
        return true;
    }
    return false;
}

void resize()
{
    resizeTextLines();

    resizeProgressBars();

    if(fontManager)
    {
        fontManager->Resize();
    }

    /* let us update console too */
    Console::instance().setLineInterval(Console::instance().spacing());
    render::fillCrosshairBuffer();
}

void render()
{
    glFrontFace(GL_CCW);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    glDisable(GL_DEPTH_TEST);
    if(engine::screen_info.show_debuginfo)
        render::drawCrosshair();
    drawBars();
    drawFaders();
    renderStrings();
    Console::instance().draw();

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
}

/*
 * Other GUI options
 */
void switchGLMode(bool is_gui)
{
    if(is_gui)                                                             // set gui coordinate system
    {
        const GLfloat far_dist = 4096.0f;
        const GLfloat near_dist = -1.0f;

        guiProjectionMatrix = util::matrix4{};                                        // identity matrix
        guiProjectionMatrix[0][0] = 2.0f / static_cast<GLfloat>(engine::screen_info.w);
        guiProjectionMatrix[1][1] = 2.0f / static_cast<GLfloat>(engine::screen_info.h);
        guiProjectionMatrix[2][2] =-2.0f / (far_dist - near_dist);
        guiProjectionMatrix[3][0] =-1.0f;
        guiProjectionMatrix[3][1] =-1.0f;
        guiProjectionMatrix[3][2] =-(far_dist + near_dist) / (far_dist - near_dist);
    }
    else                                                                        // set camera coordinate system
    {
        guiProjectionMatrix = engine::engine_camera.m_glProjMat;
    }
}

void drawInventory()
{
    //if (!main_inventory_menu->IsVisible())
    main_inventory_manager->frame(engine::engine_frame_time);
    if(main_inventory_manager->getCurrentState() == InventoryManager::InventoryState::Disabled)
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

    GLfloat upper_color[4] = {0.0,0.0,0.0,0.45f};
    GLfloat lower_color[4] = {0.0,0.0,0.0,0.75f};

    drawRect(0.0, 0.0, static_cast<GLfloat>(engine::screen_info.w), static_cast<GLfloat>(engine::screen_info.h),
                 upper_color, upper_color, lower_color, lower_color,
                 loader::BlendingMode::Opaque);

    glDepthMask(GL_TRUE);
    glPopAttrib();

    //GLfloat color[4] = {0,0,0,0.45};
    //Gui_DrawRect(0,0,(GLfloat)screen_info.w,(GLfloat)screen_info.h, color, color, color, color, GL_SRC_ALPHA + GL_ONE_MINUS_SRC_ALPHA);

    switchGLMode(false);
    //main_inventory_menu->Render(); //engine_world.character->character->inventory
    main_inventory_manager->render();
    switchGLMode(true);
}

void drawLoadScreen(int value)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    switchGLMode(true);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    glPixelStorei(GL_UNPACK_LSB_FIRST, GL_FALSE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    showLoadScreenFader();
    showLoadingProgressBar(value);

    glDepthMask(GL_TRUE);

    switchGLMode(false);

    SDL_GL_SwapWindow(engine::sdl_window);
}

namespace
{
    GLuint rectanglePositionBuffer = 0;
    GLuint rectangleColorBuffer = 0;
    std::unique_ptr<render::VertexArray> rectangleArray = nullptr;
}

/**
 * Draws simple colored rectangle with given parameters.
 */
void drawRect(const GLfloat &x, const GLfloat &y,
                  const GLfloat &width, const GLfloat &height,
                  const float colorUpperLeft[], const float colorUpperRight[],
                  const float colorLowerLeft[], const float colorLowerRight[],
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

    if(rectanglePositionBuffer == 0)
    {
        glGenBuffers(1, &rectanglePositionBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, rectanglePositionBuffer);
        GLfloat rectCoords[8] = { 0, 0,
            1, 0,
            1, 1,
            0, 1 };
        glBufferData(GL_ARRAY_BUFFER, sizeof(rectCoords), rectCoords, GL_STATIC_DRAW);

        glGenBuffers(1, &rectangleColorBuffer);

        render::VertexArrayAttribute attribs[] = {
            render::VertexArrayAttribute(render::GuiShaderDescription::position, 2, GL_FLOAT, false, rectanglePositionBuffer, sizeof(GLfloat[2]), 0),
            render::VertexArrayAttribute(render::GuiShaderDescription::color, 4, GL_FLOAT, false, rectangleColorBuffer, sizeof(GLfloat[4]), 0),
        };
        rectangleArray.reset(new render::VertexArray(0, 2, attribs));
    }

    glBindBuffer(GL_ARRAY_BUFFER, rectangleColorBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat[4]) * 4, nullptr, GL_STREAM_DRAW);
    GLfloat *rectColors = static_cast<GLfloat *>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
    memcpy(rectColors + 0, colorLowerLeft, sizeof(GLfloat) * 4);
    memcpy(rectColors + 4, colorLowerRight, sizeof(GLfloat) * 4);
    memcpy(rectColors + 8, colorUpperRight, sizeof(GLfloat) * 4);
    memcpy(rectColors + 12, colorUpperLeft, sizeof(GLfloat) * 4);
    glUnmapBuffer(GL_ARRAY_BUFFER);

    const GLfloat offset[2] = { x / (engine::screen_info.w*0.5f) - 1.f, y / (engine::screen_info.h*0.5f) - 1.f };
    const GLfloat factor[2] = { (width / engine::screen_info.w) * 2.0f, (height / engine::screen_info.h) * 2.0f };

    render::GuiShaderDescription *shader = render::renderer.shaderManager()->getGuiShader(texture != 0);
    glUseProgram(shader->program);
    glUniform1i(shader->sampler, 0);
    if(texture)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
    }
    glUniform2fv(shader->offset, 1, offset);
    glUniform2fv(shader->factor, 1, factor);

    rectangleArray->bind();

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

} // namespace gui
