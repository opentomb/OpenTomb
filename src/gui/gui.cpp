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

TextLine*     gui_base_lines = nullptr;
TextLine      gui_temp_lines[MaxTempLines];
uint16_t      temp_lines_used = 0;

FontManager       *fontManager = nullptr;

GLuint crosshairBuffer;
render::VertexArray *crosshairArray;

util::matrix4 guiProjectionMatrix = util::matrix4();

void init()
{
    initBars();
    initFaders();
    initNotifier();
    initTempLines();

    glGenBuffers(1, &crosshairBuffer);
    fillCrosshairBuffer();

    //main_inventory_menu = new gui_InventoryMenu();
    main_inventory_manager = new InventoryManager();
}

void initFontManager()
{
    fontManager = new FontManager();
}

void initTempLines()
{
    for(int i = 0; i < MaxTempLines; i++)
    {
        gui_temp_lines[i].text.clear();
        gui_temp_lines[i].show = false;

        gui_temp_lines[i].next = nullptr;
        gui_temp_lines[i].prev = nullptr;

        gui_temp_lines[i].font_id = FontType::Secondary;
        gui_temp_lines[i].style_id = FontStyle::Generic;
    }
}

void destroy()
{
    for(int i = 0; i < MaxTempLines; i++)
    {
        gui_temp_lines[i].show = false;
        gui_temp_lines[i].text.clear();
    }

    destroyFaders();

    temp_lines_used = MaxTempLines;

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

    if(fontManager)
    {
        delete fontManager;
        fontManager = nullptr;
    }
}

void addLine(TextLine *line)
{
    if(gui_base_lines == nullptr)
    {
        gui_base_lines = line;
        line->next = nullptr;
        line->prev = nullptr;
        return;
    }

    line->prev = nullptr;
    line->next = gui_base_lines;
    gui_base_lines->prev = line;
    gui_base_lines = line;
}

// line must be in the list, otherway You crash engine!
void deleteLine(TextLine *line)
{
    if(line == gui_base_lines)
    {
        gui_base_lines = line->next;
        if(gui_base_lines != nullptr)
        {
            gui_base_lines->prev = nullptr;
        }
        return;
    }

    line->prev->next = line->next;
    if(line->next)
    {
        line->next->prev = line->prev;
    }
}

void moveLine(TextLine *line)
{
    line->absXoffset = line->X * engine::screen_info.scale_factor;
    line->absYoffset = line->Y * engine::screen_info.scale_factor;
}

/**
 * For simple temporary lines rendering.
 * Really all strings will be rendered in Gui_Render() function.
 */
TextLine *drawText(GLfloat x, GLfloat y, const char *fmt, ...)
{
    if(fontManager && (temp_lines_used < MaxTempLines - 1))
    {
        va_list argptr;
        TextLine* l = gui_temp_lines + temp_lines_used;

        l->font_id = FontType::Secondary;
        l->style_id = FontStyle::Generic;

        va_start(argptr, fmt);
        char tmpStr[LineDefaultSize];
        vsnprintf(tmpStr, LineDefaultSize, fmt, argptr);
        l->text = tmpStr;
        va_end(argptr);

        l->next = nullptr;
        l->prev = nullptr;

        temp_lines_used++;

        l->X = x;
        l->Y = y;
        l->Xanchor = HorizontalAnchor::Left;
        l->Yanchor = VerticalAnchor::Bottom;

        l->absXoffset = l->X * engine::screen_info.scale_factor;
        l->absYoffset = l->Y * engine::screen_info.scale_factor;

        l->show = true;
        return l;
    }

    return nullptr;
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
    TextLine* l = gui_base_lines;

    while(l)
    {
        l->absXoffset = l->X * engine::screen_info.scale_factor;
        l->absYoffset = l->Y * engine::screen_info.scale_factor;

        l = l->next;
    }

    l = gui_temp_lines;
    for(uint16_t i = 0; i < temp_lines_used; i++, l++)
    {
        l->absXoffset = l->X * engine::screen_info.scale_factor;
        l->absYoffset = l->Y * engine::screen_info.scale_factor;
    }

    resizeProgressBars();

    if(fontManager)
    {
        fontManager->Resize();
    }

    /* let us update console too */
    Console::instance().setLineInterval(Console::instance().spacing());
    fillCrosshairBuffer();
}

void render()
{
    glFrontFace(GL_CCW);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    glDisable(GL_DEPTH_TEST);
    if(engine::screen_info.show_debuginfo)
        drawCrosshair();
    drawBars();
    drawFaders();
    renderStrings();
    Console::instance().draw();

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
}

void renderStringLine(TextLine *l)
{
    GLfloat real_x = 0.0, real_y = 0.0;

    if(fontManager == nullptr)
    {
        return;
    }

    FontTexture* gl_font = fontManager->GetFont(static_cast<FontType>(l->font_id));
    FontStyleData* style = fontManager->GetFontStyle(static_cast<FontStyle>(l->style_id));

    if((gl_font == nullptr) || (style == nullptr) || (!l->show) || (style->hidden))
    {
        return;
    }

    glf_get_string_bb(gl_font, l->text.c_str(), -1, l->rect + 0, l->rect + 1, l->rect + 2, l->rect + 3);

    switch(l->Xanchor)
    {
        case HorizontalAnchor::Left:
            real_x = l->absXoffset;   // Used with center and right alignments.
            break;
        case HorizontalAnchor::Right:
            real_x = static_cast<float>(engine::screen_info.w) - (l->rect[2] - l->rect[0]) - l->absXoffset;
            break;
        case HorizontalAnchor::Center:
            real_x = (engine::screen_info.w / 2.0f) - ((l->rect[2] - l->rect[0]) / 2.0f) + l->absXoffset;  // Absolute center.
            break;
    }

    switch(l->Yanchor)
    {
        case VerticalAnchor::Bottom:
            real_y += l->absYoffset;
            break;
        case VerticalAnchor::Top:
            real_y = static_cast<float>(engine::screen_info.h) - (l->rect[3] - l->rect[1]) - l->absYoffset;
            break;
        case VerticalAnchor::Center:
            real_y = (engine::screen_info.h / 2.0f) + (l->rect[3] - l->rect[1]) - l->absYoffset;          // Consider the baseline.
            break;
    }

    // missing texture_coord pointer... GL_TEXTURE_COORD_ARRAY state are enabled here!
    /*if(style->rect)
    {
        glBindTexture(GL_TEXTURE_2D, 0);
        GLfloat x0 = l->rect[0] + real_x - style->rect_border * screen_info.w_unit;
        GLfloat y0 = l->rect[1] + real_y - style->rect_border * screen_info.h_unit;
        GLfloat x1 = l->rect[2] + real_x + style->rect_border * screen_info.w_unit;
        GLfloat y1 = l->rect[3] + real_y + style->rect_border * screen_info.h_unit;

        GLfloat rectCoords[8];
        rectCoords[0] = x0; rectCoords[1] = y0;
        rectCoords[2] = x1; rectCoords[3] = y0;
        rectCoords[4] = x1; rectCoords[5] = y1;
        rectCoords[6] = x0; rectCoords[7] = y1;
        color(style->rect_color);
        glVertexPointer(2, GL_FLOAT, 0, rectCoords);
        glDrawArrays(GL_POLYGON, 0, 4);
    }*/

    if(style->shadowed)
    {
        gl_font->gl_font_color[0] = 0.0f;
        gl_font->gl_font_color[1] = 0.0f;
        gl_font->gl_font_color[2] = 0.0f;
        gl_font->gl_font_color[3] = static_cast<float>(style->color[3]) * FontShadowTransparency;// Derive alpha from base color.
        glf_render_str(gl_font,
                       (real_x + FontShadowHorizontalShift),
                       (real_y + FontShadowVerticalShift),
                       l->text.c_str());
    }

    std::copy(style->real_color + 0, style->real_color + 4, gl_font->gl_font_color);
    glf_render_str(gl_font, real_x, real_y, l->text.c_str());
}

void renderStrings()
{
    if(fontManager != nullptr)
    {
        TextLine* l = gui_base_lines;

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        render::TextShaderDescription *shader = render::renderer.shaderManager()->getTextShader();
        glUseProgram(shader->program);
        GLfloat screenSize[2] = {
            static_cast<GLfloat>(engine::screen_info.w),
            static_cast<GLfloat>(engine::screen_info.h)
        };
        glUniform2fv(shader->screenSize, 1, screenSize);
        glUniform1i(shader->sampler, 0);

        while(l)
        {
            renderStringLine(l);
            l = l->next;
        }

        l = gui_temp_lines;
        for(uint16_t i = 0; i < temp_lines_used; i++, l++)
        {
            if(l->show)
            {
                renderStringLine(l);
                l->show = false;
            }
        }

        temp_lines_used = 0;
    }
}

/**
 * That function updates item animation and rebuilds skeletal matrices;
 * @param bf - extended bone frame of the item;
 */
void itemFrame(world::animation::SSBoneFrame *bf, btScalar time)
{
    bf->animations.stepAnimation(time);

    world::Entity::updateCurrentBoneFrame(bf);
}

/**
 * The base function, that draws one item by them id. Items may be animated.
 * This time for correct time calculation that function must be called every frame.
 * @param item_id - the base item id;
 * @param size - the item size on the screen;
 * @param str - item description - shows near / under item model;
 */
void renderItem(world::animation::SSBoneFrame *bf, btScalar size, const btTransform& mvMatrix)
{
    const render::LitShaderDescription *shader = render::renderer.shaderManager()->getEntityShader(0, false);
    glUseProgram(shader->program);
    glUniform1i(shader->number_of_lights, 0);
    glUniform4f(shader->light_ambient, 1.f, 1.f, 1.f, 1.f);

    if(size != 0.0)
    {
        auto bb = bf->boundingBox.getDiameter();
        if(bb[0] >= bb[1])
        {
            size /= ((bb[0] >= bb[2]) ? (bb[0]) : (bb[2]));
        }
        else
        {
            size /= ((bb[1] >= bb[2]) ? (bb[1]) : (bb[2]));
        }
        size *= 0.8f;

        btTransform scaledMatrix;
        scaledMatrix.setIdentity();
        if(size < 1.0)          // only reduce items size...
        {
            util::Mat4_Scale(scaledMatrix, size, size, size);
        }
        util::matrix4 scaledMvMatrix(mvMatrix * scaledMatrix);
        util::matrix4 mvpMatrix = guiProjectionMatrix * scaledMvMatrix;

        // Render with scaled model view projection matrix
        // Use original modelview matrix, as that is used for normals whose size shouldn't change.
        render::renderer.renderSkeletalModel(shader, bf, util::matrix4(mvMatrix), mvpMatrix/*, guiProjectionMatrix*/);
    }
    else
    {
        util::matrix4 mvpMatrix = guiProjectionMatrix * mvMatrix;
        render::renderer.renderSkeletalModel(shader, bf, util::matrix4(mvMatrix), mvpMatrix/*, guiProjectionMatrix*/);
    }
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

struct gui_buffer_entry_s
{
    GLfloat position[2];
    uint8_t color[4];
};

void fillCrosshairBuffer()
{
    gui_buffer_entry_s crosshair_buf[4] = {
        {{static_cast<GLfloat>(engine::screen_info.w / 2.0f - 5.f), (static_cast<GLfloat>(engine::screen_info.h) / 2.0f)}, {255, 0, 0, 255}},
        {{static_cast<GLfloat>(engine::screen_info.w / 2.0f + 5.f), (static_cast<GLfloat>(engine::screen_info.h) / 2.0f)}, {255, 0, 0, 255}},
        {{static_cast<GLfloat>(engine::screen_info.w / 2.0f), (static_cast<GLfloat>(engine::screen_info.h) / 2.0f - 5.f)}, {255, 0, 0, 255}},
        {{static_cast<GLfloat>(engine::screen_info.w / 2.0f), (static_cast<GLfloat>(engine::screen_info.h) / 2.0f + 5.f)}, {255, 0, 0, 255}}
    };

    glBindBuffer(GL_ARRAY_BUFFER, crosshairBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(crosshair_buf), crosshair_buf, GL_STATIC_DRAW);

    render::VertexArrayAttribute attribs[] = {
        render::VertexArrayAttribute(render::GuiShaderDescription::position, 2, GL_FLOAT, false, crosshairBuffer, sizeof(gui_buffer_entry_s), offsetof(gui_buffer_entry_s, position)),
        render::VertexArrayAttribute(render::GuiShaderDescription::color, 4, GL_UNSIGNED_BYTE, true, crosshairBuffer, sizeof(gui_buffer_entry_s), offsetof(gui_buffer_entry_s, color))
    };
    crosshairArray = new render::VertexArray(0, 2, attribs);
}

void drawCrosshair()
{
    render::GuiShaderDescription *shader = render::renderer.shaderManager()->getGuiShader(false);

    glUseProgram(shader->program);
    GLfloat factor[2] = {
        2.0f / engine::screen_info.w,
        2.0f / engine::screen_info.h
    };
    glUniform2fv(shader->factor, 1, factor);
    GLfloat offset[2] = { -1.f, -1.f };
    glUniform2fv(shader->offset, 1, offset);

    crosshairArray->bind();

    glDrawArrays(GL_LINES, 0, 4);
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
