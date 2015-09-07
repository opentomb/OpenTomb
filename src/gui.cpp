#include "gui.h"

#include <cstdint>
#include <map>

#ifdef __APPLE_CC__
#include <ImageIO/ImageIO.h>
#else
#include <SDL2/SDL_image.h>
#endif

#include "camera.h"
#include "character_controller.h"
#include "console.h"
#include "engine.h"
#include "gl_font.h"
#include "gl_util.h"
#include "render.h"
#include "script.h"
#include "shader_description.h"
#include "shader_manager.h"
#include "strings.h"
#include "system.h"
#include "vertex_array.h"
#include "vmath.h"
#include "inventory.h"

extern SDL_Window  *sdl_window;

TextLine*     gui_base_lines = nullptr;
TextLine      gui_temp_lines[MaxTempLines];
uint16_t      temp_lines_used = 0;

gui_ItemNotifier    Notifier;
std::map<BarType, ProgressBar>     Bar;
std::map<FaderType, Fader>         faderType;

FontManager       *fontManager = nullptr;

GLuint crosshairBuffer;
VertexArray *crosshairArray;

matrix4 guiProjectionMatrix = matrix4();

void Gui_Init()
{
    Gui_InitBars();
    Gui_InitFaders();
    Gui_InitNotifier();
    Gui_InitTempLines();

    glGenBuffers(1, &crosshairBuffer);
    Gui_FillCrosshairBuffer();

    //main_inventory_menu = new gui_InventoryMenu();
    main_inventory_manager = new InventoryManager();
}

void Gui_InitFontManager()
{
    fontManager = new FontManager();
}

void Gui_InitTempLines()
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

void Gui_InitBars()
{
    {
        const auto i = BarType::Health;
        Bar[i].Visible = false;
        Bar[i].Alternate = false;
        Bar[i].Invert = false;
        Bar[i].Vertical = false;

        Bar[i].SetSize(250, 15, 3);
        Bar[i].SetPosition(HorizontalAnchor::Left, 30, VerticalAnchor::Top, 30);
        Bar[i].SetColor(BarColorType::BaseMain, 255, 50, 50, 200);
        Bar[i].SetColor(BarColorType::BaseFade, 100, 255, 50, 200);
        Bar[i].SetColor(BarColorType::AltMain, 255, 180, 0, 255);
        Bar[i].SetColor(BarColorType::AltFade, 255, 255, 0, 255);
        Bar[i].SetColor(BarColorType::BackMain, 0, 0, 0, 160);
        Bar[i].SetColor(BarColorType::BackFade, 60, 60, 60, 130);
        Bar[i].SetColor(BarColorType::BorderMain, 200, 200, 200, 50);
        Bar[i].SetColor(BarColorType::BorderFade, 80, 80, 80, 100);
        Bar[i].SetValues(LARA_PARAM_HEALTH_MAX, LARA_PARAM_HEALTH_MAX / 3);
        Bar[i].SetBlink(300);
        Bar[i].SetExtrude(true, 100);
        Bar[i].SetAutoshow(true, 2000, true, 400);
    }
    {
        const auto i = BarType::Air;
        Bar[i].Visible = false;
        Bar[i].Alternate = false;
        Bar[i].Invert = true;
        Bar[i].Vertical = false;

        Bar[i].SetSize(250, 15, 3);
        Bar[i].SetPosition(HorizontalAnchor::Right, 30, VerticalAnchor::Top, 30);
        Bar[i].SetColor(BarColorType::BaseMain, 0, 50, 255, 200);
        Bar[i].SetColor(BarColorType::BaseFade, 190, 190, 255, 200);
        Bar[i].SetColor(BarColorType::BackMain, 0, 0, 0, 160);
        Bar[i].SetColor(BarColorType::BackFade, 60, 60, 60, 130);
        Bar[i].SetColor(BarColorType::BorderMain, 200, 200, 200, 50);
        Bar[i].SetColor(BarColorType::BorderFade, 80, 80, 80, 100);
        Bar[i].SetValues(LARA_PARAM_AIR_MAX, (LARA_PARAM_AIR_MAX / 3));
        Bar[i].SetBlink(300);
        Bar[i].SetExtrude(true, 100);
        Bar[i].SetAutoshow(true, 2000, true, 400);
    }
    {
        const auto i = BarType::Stamina;
        Bar[i].Visible = false;
        Bar[i].Alternate = false;
        Bar[i].Invert = false;
        Bar[i].Vertical = false;

        Bar[i].SetSize(250, 15, 3);
        Bar[i].SetPosition(HorizontalAnchor::Left, 30, VerticalAnchor::Top, 55);
        Bar[i].SetColor(BarColorType::BaseMain, 255, 100, 50, 200);
        Bar[i].SetColor(BarColorType::BaseFade, 255, 200, 0, 200);
        Bar[i].SetColor(BarColorType::BackMain, 0, 0, 0, 160);
        Bar[i].SetColor(BarColorType::BackFade, 60, 60, 60, 130);
        Bar[i].SetColor(BarColorType::BorderMain, 110, 110, 110, 100);
        Bar[i].SetColor(BarColorType::BorderFade, 60, 60, 60, 180);
        Bar[i].SetValues(LARA_PARAM_STAMINA_MAX, 0);
        Bar[i].SetExtrude(true, 100);
        Bar[i].SetAutoshow(true, 500, true, 300);
    }
    {
        const auto i = BarType::Warmth;
        Bar[i].Visible = false;
        Bar[i].Alternate = false;
        Bar[i].Invert = true;
        Bar[i].Vertical = false;

        Bar[i].SetSize(250, 15, 3);
        Bar[i].SetPosition(HorizontalAnchor::Right, 30, VerticalAnchor::Top, 55);
        Bar[i].SetColor(BarColorType::BaseMain, 255, 0, 255, 255);
        Bar[i].SetColor(BarColorType::BaseFade, 190, 120, 255, 255);
        Bar[i].SetColor(BarColorType::BackMain, 0, 0, 0, 160);
        Bar[i].SetColor(BarColorType::BackFade, 60, 60, 60, 130);
        Bar[i].SetColor(BarColorType::BorderMain, 200, 200, 200, 50);
        Bar[i].SetColor(BarColorType::BorderFade, 80, 80, 80, 100);
        Bar[i].SetValues(LARA_PARAM_WARMTH_MAX, LARA_PARAM_WARMTH_MAX / 3);
        Bar[i].SetBlink(200);
        Bar[i].SetExtrude(true, 60);
        Bar[i].SetAutoshow(true, 500, true, 300);
    }
    {
        const auto i = BarType::Loading;
        Bar[i].Visible = true;
        Bar[i].Alternate = false;
        Bar[i].Invert = false;
        Bar[i].Vertical = false;

        Bar[i].SetSize(800, 25, 3);
        Bar[i].SetPosition(HorizontalAnchor::Center, 0, VerticalAnchor::Bottom, 40);
        Bar[i].SetColor(BarColorType::BaseMain, 255, 225, 127, 230);
        Bar[i].SetColor(BarColorType::BaseFade, 255, 187, 136, 230);
        Bar[i].SetColor(BarColorType::BackMain, 30, 30, 30, 100);
        Bar[i].SetColor(BarColorType::BackFade, 60, 60, 60, 100);
        Bar[i].SetColor(BarColorType::BorderMain, 200, 200, 200, 80);
        Bar[i].SetColor(BarColorType::BorderFade, 80, 80, 80, 80);
        Bar[i].SetValues(1000, 0);
        Bar[i].SetExtrude(true, 70);
        Bar[i].SetAutoshow(false, 500, false, 300);
    }
}

void Gui_InitFaders()
{
    {
        const auto i = FaderType::LoadScreen;
        faderType[i].SetAlpha(255);
        faderType[i].SetColor(0, 0, 0);
        faderType[i].SetBlendingMode(loader::BlendingMode::Opaque);
        faderType[i].SetSpeed(500);
        faderType[i].SetScaleMode(FaderScale::Zoom);
    }

    {
        const auto i = FaderType::Effect;
        faderType[i].SetAlpha(255);
        faderType[i].SetColor(255, 180, 0);
        faderType[i].SetBlendingMode(loader::BlendingMode::Multiply);
        faderType[i].SetSpeed(10, 800);
    }

    {
        const auto i = FaderType::Black;
        faderType[i].SetAlpha(255);
        faderType[i].SetColor(0, 0, 0);
        faderType[i].SetBlendingMode(loader::BlendingMode::Opaque);
        faderType[i].SetSpeed(500);
        faderType[i].SetScaleMode(FaderScale::Zoom);
    }
}

void Gui_InitNotifier()
{
    Notifier.SetPos(850.0, 850.0);
    Notifier.SetRot(180.0, 270.0);
    Notifier.SetSize(128.0);
    Notifier.SetRotateTime(2500.0);
}

void Gui_Destroy()
{
    for(int i = 0; i < MaxTempLines; i++)
    {
        gui_temp_lines[i].show = false;
        gui_temp_lines[i].text.clear();
    }

    for(auto& fader : faderType)
    {
        fader.second.Cut();
    }

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

void Gui_AddLine(TextLine *line)
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
void Gui_DeleteLine(TextLine *line)
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

void Gui_MoveLine(TextLine *line)
{
    line->absXoffset = line->X * screen_info.scale_factor;
    line->absYoffset = line->Y * screen_info.scale_factor;
}

/**
 * For simple temporary lines rendering.
 * Really all strings will be rendered in Gui_Render() function.
 */
TextLine *Gui_OutTextXY(GLfloat x, GLfloat y, const char *fmt, ...)
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

        l->absXoffset = l->X * screen_info.scale_factor;
        l->absYoffset = l->Y * screen_info.scale_factor;

        l->show = true;
        return l;
    }

    return nullptr;
}

bool Gui_Update()
{
    if(fontManager != nullptr)
    {
        fontManager->Update();
    }

    if(!ConsoleInfo::instance().isVisible() && control_states.gui_inventory && main_inventory_manager)
    {
        if(engine_world.character &&
           (main_inventory_manager->getCurrentState() == InventoryManager::InventoryState::Disabled))
        {
            main_inventory_manager->setInventory(&engine_world.character->m_inventory);
            main_inventory_manager->send(InventoryManager::InventoryState::Open);
        }
        if(main_inventory_manager->getCurrentState() == InventoryManager::InventoryState::Idle)
        {
            main_inventory_manager->send(InventoryManager::InventoryState::Closed);
        }
    }

    if(ConsoleInfo::instance().isVisible() || main_inventory_manager->getCurrentState() != InventoryManager::InventoryState::Disabled)
    {
        return true;
    }
    return false;
}

void Gui_Resize()
{
    TextLine* l = gui_base_lines;

    while(l)
    {
        l->absXoffset = l->X * screen_info.scale_factor;
        l->absYoffset = l->Y * screen_info.scale_factor;

        l = l->next;
    }

    l = gui_temp_lines;
    for(uint16_t i = 0; i < temp_lines_used; i++, l++)
    {
        l->absXoffset = l->X * screen_info.scale_factor;
        l->absYoffset = l->Y * screen_info.scale_factor;
    }

    for(auto& i : Bar)
    {
        i.second.Resize();
    }

    if(fontManager)
    {
        fontManager->Resize();
    }

    /* let us update console too */
    ConsoleInfo::instance().setLineInterval(ConsoleInfo::instance().spacing());
    Gui_FillCrosshairBuffer();
}

void Gui_Render()
{
    glFrontFace(GL_CCW);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    glDisable(GL_DEPTH_TEST);
    if(screen_info.show_debuginfo) Gui_DrawCrosshair();
    Gui_DrawBars();
    Gui_DrawFaders();
    Gui_RenderStrings();
    ConsoleInfo::instance().draw();

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
}

void Gui_RenderStringLine(TextLine *l)
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
            real_x = static_cast<float>(screen_info.w) - (l->rect[2] - l->rect[0]) - l->absXoffset;
            break;
        case HorizontalAnchor::Center:
            real_x = (static_cast<float>(screen_info.w) / 2.0) - ((l->rect[2] - l->rect[0]) / 2.0) + l->absXoffset;  // Absolute center.
            break;
    }

    switch(l->Yanchor)
    {
        case VerticalAnchor::Bottom:
            real_y += l->absYoffset;
            break;
        case VerticalAnchor::Top:
            real_y = static_cast<float>(screen_info.h) - (l->rect[3] - l->rect[1]) - l->absYoffset;
            break;
        case VerticalAnchor::Center:
            real_y = (static_cast<float>(screen_info.h) / 2.0) + (l->rect[3] - l->rect[1]) - l->absYoffset;          // Consider the baseline.
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

void Gui_RenderStrings()
{
    if(fontManager != nullptr)
    {
        TextLine* l = gui_base_lines;

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        TextShaderDescription *shader = renderer.shaderManager()->getTextShader();
        glUseProgram(shader->program);
        GLfloat screenSize[2] = {
            static_cast<GLfloat>(screen_info.w),
            static_cast<GLfloat>(screen_info.h)
        };
        glUniform2fv(shader->screenSize, 1, screenSize);
        glUniform1i(shader->sampler, 0);

        while(l)
        {
            Gui_RenderStringLine(l);
            l = l->next;
        }

        l = gui_temp_lines;
        for(uint16_t i = 0; i < temp_lines_used; i++, l++)
        {
            if(l->show)
            {
                Gui_RenderStringLine(l);
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
void Item_Frame(struct SSBoneFrame *bf, btScalar time)
{
    bf->animations.stepAnimation(time);

    Entity::updateCurrentBoneFrame(bf);
}

/**
 * The base function, that draws one item by them id. Items may be animated.
 * This time for correct time calculation that function must be called every frame.
 * @param item_id - the base item id;
 * @param size - the item size on the screen;
 * @param str - item description - shows near / under item model;
 */
void Gui_RenderItem(SSBoneFrame *bf, btScalar size, const btTransform& mvMatrix)
{
    const LitShaderDescription *shader = renderer.shaderManager()->getEntityShader(0, false);
    glUseProgram(shader->program);
    glUniform1i(shader->number_of_lights, 0);
    glUniform4f(shader->light_ambient, 1.f, 1.f, 1.f, 1.f);

    if(size != 0.0)
    {
        auto bb = bf->bb_max - bf->bb_min;
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
            Mat4_Scale(scaledMatrix, size, size, size);
        }
        matrix4 scaledMvMatrix(mvMatrix * scaledMatrix);
        matrix4 mvpMatrix = guiProjectionMatrix * scaledMvMatrix;

        // Render with scaled model view projection matrix
        // Use original modelview matrix, as that is used for normals whose size shouldn't change.
        renderer.renderSkeletalModel(shader, bf, matrix4(mvMatrix), mvpMatrix/*, guiProjectionMatrix*/);
    }
    else
    {
        matrix4 mvpMatrix = guiProjectionMatrix * mvMatrix;
        renderer.renderSkeletalModel(shader, bf, matrix4(mvMatrix), mvpMatrix/*, guiProjectionMatrix*/);
    }
}

/*
 * Other GUI options
 */
void Gui_SwitchGLMode(char is_gui)
{
    if(0 != is_gui)                                                             // set gui coordinate system
    {
        const GLfloat far_dist = 4096.0f;
        const GLfloat near_dist = -1.0f;

        guiProjectionMatrix = matrix4{};                                        // identity matrix
        guiProjectionMatrix[0][0] = 2.0f / static_cast<GLfloat>(screen_info.w);
        guiProjectionMatrix[1][1] = 2.0f / static_cast<GLfloat>(screen_info.h);
        guiProjectionMatrix[2][2] =-2.0f / (far_dist - near_dist);
        guiProjectionMatrix[3][0] =-1.0f;
        guiProjectionMatrix[3][1] =-1.0f;
        guiProjectionMatrix[3][2] =-(far_dist + near_dist) / (far_dist - near_dist);
    }
    else                                                                        // set camera coordinate system
    {
        guiProjectionMatrix = engine_camera.m_glProjMat;
    }
}

struct gui_buffer_entry_s
{
    GLfloat position[2];
    uint8_t color[4];
};

void Gui_FillCrosshairBuffer()
{
    gui_buffer_entry_s crosshair_buf[4] = {
        {{static_cast<GLfloat>(screen_info.w / 2.0f - 5.f), (static_cast<GLfloat>(screen_info.h) / 2.0f)}, {255, 0, 0, 255}},
        {{static_cast<GLfloat>(screen_info.w / 2.0f + 5.f), (static_cast<GLfloat>(screen_info.h) / 2.0f)}, {255, 0, 0, 255}},
        {{static_cast<GLfloat>(screen_info.w / 2.0f), (static_cast<GLfloat>(screen_info.h) / 2.0f - 5.f)}, {255, 0, 0, 255}},
        {{static_cast<GLfloat>(screen_info.w / 2.0f), (static_cast<GLfloat>(screen_info.h) / 2.0f + 5.f)}, {255, 0, 0, 255}}
    };

    glBindBuffer(GL_ARRAY_BUFFER, crosshairBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(crosshair_buf), crosshair_buf, GL_STATIC_DRAW);

    VertexArrayAttribute attribs[] = {
        VertexArrayAttribute(GuiShaderDescription::position, 2, GL_FLOAT, false, crosshairBuffer, sizeof(gui_buffer_entry_s), offsetof(gui_buffer_entry_s, position)),
        VertexArrayAttribute(GuiShaderDescription::color, 4, GL_UNSIGNED_BYTE, true, crosshairBuffer, sizeof(gui_buffer_entry_s), offsetof(gui_buffer_entry_s, color))
    };
    crosshairArray = new VertexArray(0, 2, attribs);
}

void Gui_DrawCrosshair()
{
    GuiShaderDescription *shader = renderer.shaderManager()->getGuiShader(false);

    glUseProgram(shader->program);
    GLfloat factor[2] = {
        2.0f / screen_info.w,
        2.0f / screen_info.h
    };
    glUniform2fv(shader->factor, 1, factor);
    GLfloat offset[2] = { -1.f, -1.f };
    glUniform2fv(shader->offset, 1, offset);

    crosshairArray->bind();

    glDrawArrays(GL_LINES, 0, 4);
}

void Gui_DrawFaders()
{
    for(auto& i : faderType)
    {
        i.second.Show();
    }
}

void Gui_DrawBars()
{
    if(engine_world.character)
    {
        if(engine_world.character->m_weaponCurrentState > WeaponState::HideToReady)
            Bar[BarType::Health].Forced = true;

        if(engine_world.character->getParam(PARAM_POISON) > 0.0)
            Bar[BarType::Health].Alternate = true;

        Bar[BarType::Air].Show(engine_world.character->getParam(PARAM_AIR));
        Bar[BarType::Stamina].Show(engine_world.character->getParam(PARAM_STAMINA));
        Bar[BarType::Health].Show(engine_world.character->getParam(PARAM_HEALTH));
        Bar[BarType::Warmth].Show(engine_world.character->getParam(PARAM_WARMTH));
    }
}

void Gui_DrawInventory()
{
    //if (!main_inventory_menu->IsVisible())
    main_inventory_manager->frame(engine_frame_time);
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

    Gui_DrawRect(0.0, 0.0, static_cast<GLfloat>(screen_info.w), static_cast<GLfloat>(screen_info.h),
                 upper_color, upper_color, lower_color, lower_color,
                 loader::BlendingMode::Opaque);

    glDepthMask(GL_TRUE);
    glPopAttrib();

    //GLfloat color[4] = {0,0,0,0.45};
    //Gui_DrawRect(0,0,(GLfloat)screen_info.w,(GLfloat)screen_info.h, color, color, color, color, GL_SRC_ALPHA + GL_ONE_MINUS_SRC_ALPHA);

    Gui_SwitchGLMode(0);
    //main_inventory_menu->Render(); //engine_world.character->character->inventory
    main_inventory_manager->render();
    Gui_SwitchGLMode(1);
}

void Gui_NotifierStart(int item)
{
    Notifier.Start(item, GUI_NOTIFIER_SHOWTIME);
}

void Gui_NotifierStop()
{
    Notifier.Reset();
}

void Gui_DrawNotifier()
{
    Notifier.Draw();
    Notifier.Animate();
}

void Gui_DrawLoadScreen(int value)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Gui_SwitchGLMode(1);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    glPixelStorei(GL_UNPACK_LSB_FIRST, GL_FALSE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    faderType[FaderType::LoadScreen].Show();
    Bar[BarType::Loading].Show(value);

    glDepthMask(GL_TRUE);

    Gui_SwitchGLMode(0);

    SDL_GL_SwapWindow(sdl_window);
}

namespace
{
    GLuint rectanglePositionBuffer = 0;
    GLuint rectangleColorBuffer = 0;
    std::unique_ptr<VertexArray> rectangleArray = nullptr;
}

/**
 * Draws simple colored rectangle with given parameters.
 */
void Gui_DrawRect(const GLfloat &x, const GLfloat &y,
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

        VertexArrayAttribute attribs[] = {
            VertexArrayAttribute(GuiShaderDescription::position, 2, GL_FLOAT, false, rectanglePositionBuffer, sizeof(GLfloat[2]), 0),
            VertexArrayAttribute(GuiShaderDescription::color, 4, GL_FLOAT, false, rectangleColorBuffer, sizeof(GLfloat[4]), 0),
        };
        rectangleArray.reset(new VertexArray(0, 2, attribs));
    }

    glBindBuffer(GL_ARRAY_BUFFER, rectangleColorBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat[4]) * 4, nullptr, GL_STREAM_DRAW);
    GLfloat *rectColors = static_cast<GLfloat *>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
    memcpy(rectColors + 0, colorLowerLeft, sizeof(GLfloat) * 4);
    memcpy(rectColors + 4, colorLowerRight, sizeof(GLfloat) * 4);
    memcpy(rectColors + 8, colorUpperRight, sizeof(GLfloat) * 4);
    memcpy(rectColors + 12, colorUpperLeft, sizeof(GLfloat) * 4);
    glUnmapBuffer(GL_ARRAY_BUFFER);

    const GLfloat offset[2] = { x / (screen_info.w*0.5f) - 1.f, y / (screen_info.h*0.5f) - 1.f };
    const GLfloat factor[2] = { (width / screen_info.w) * 2.0f, (height / screen_info.h) * 2.0f };

    GuiShaderDescription *shader = renderer.shaderManager()->getGuiShader(texture != 0);
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

bool Gui_FadeStart(FaderType fader, FaderDir fade_direction)
{
    // If fader exists, and is not active, we engage it.

    if((fader < FaderType::Sentinel) && (faderType[fader].IsFading() != FaderStatus::Fading))
    {
        faderType[fader].Engage(fade_direction);
        return true;
    }
    else
    {
        return false;
    }
}

bool Gui_FadeStop(FaderType fader)
{
    if((fader < FaderType::Sentinel) && (faderType[fader].IsFading() != FaderStatus::Idle))
    {
        faderType[fader].Cut();
        return true;
    }
    else
    {
        return false;
    }
}

bool Gui_FadeAssignPic(FaderType fader, const std::string& pic_name)
{
    if((fader >= FaderType::Effect) && (fader < FaderType::Sentinel))
    {
        char buf[MAX_ENGINE_PATH];

        ///@STICK: we can write incorrect image file extension, but engine will try all supported formats
        strncpy(buf, pic_name.c_str(), MAX_ENGINE_PATH);
        if(!Engine_FileFound(buf, false))
        {
            size_t ext_len = 0;

            for(; ext_len + 1 < pic_name.length(); ext_len++)
            {
                if(buf[pic_name.length() - ext_len - 1] == '.')
                {
                    break;
                }
            }

            if(ext_len + 1 == pic_name.length())
            {
                return false;
            }

            buf[pic_name.length() - ext_len + 0] = 'b';
            buf[pic_name.length() - ext_len + 1] = 'm';
            buf[pic_name.length() - ext_len + 2] = 'p';
            buf[pic_name.length() - ext_len + 3] = 0;
            if(!Engine_FileFound(buf, false))
            {
                buf[pic_name.length() - ext_len + 0] = 'j';
                buf[pic_name.length() - ext_len + 1] = 'p';
                buf[pic_name.length() - ext_len + 2] = 'g';
                if(!Engine_FileFound(buf, false))
                {
                    buf[pic_name.length() - ext_len + 0] = 'p';
                    buf[pic_name.length() - ext_len + 1] = 'n';
                    buf[pic_name.length() - ext_len + 2] = 'g';
                    if(!Engine_FileFound(buf, false))
                    {
                        buf[pic_name.length() - ext_len + 0] = 't';
                        buf[pic_name.length() - ext_len + 1] = 'g';
                        buf[pic_name.length() - ext_len + 2] = 'a';
                        if(!Engine_FileFound(buf, false))
                        {
                            return false;
                        }
                    }
                }
            }
        }

        return faderType[fader].SetTexture(buf);
    }

    return false;
}

void Gui_FadeSetup(FaderType fader,
                   uint8_t alpha, uint8_t R, uint8_t G, uint8_t B, loader::BlendingMode blending_mode,
                   uint16_t fadein_speed, uint16_t fadeout_speed)
{
    if(fader >= FaderType::Sentinel) return;

    faderType[fader].SetAlpha(alpha);
    faderType[fader].SetColor(R, G, B);
    faderType[fader].SetBlendingMode(blending_mode);
    faderType[fader].SetSpeed(fadein_speed, fadeout_speed);
}

FaderStatus Gui_FadeCheck(FaderType fader)
{
    if((fader >= FaderType::Effect) && (fader < FaderType::Sentinel))
    {
        return faderType[fader].IsFading();
    }
    else
    {
        return FaderStatus::Invalid;
    }
}

// ===================================================================================
// ============================ FADER CLASS IMPLEMENTATION ===========================
// ===================================================================================

Fader::Fader()
{
    SetColor(0, 0, 0);
    SetBlendingMode(loader::BlendingMode::Opaque);
    SetAlpha(255);
    SetSpeed(500);
    SetDelay(0);

    mActive = false;
    mComplete = true;  // All faders must be initialized as complete to receive proper start-up callbacks.
    mDirection = FaderDir::In;

    mTexture = 0;
}

void Fader::SetAlpha(uint8_t alpha)
{
    mMaxAlpha = static_cast<float>(alpha) / 255;
}

void Fader::SetScaleMode(FaderScale mode)
{
    mTextureScaleMode = mode;
}

void Fader::SetColor(uint8_t R, uint8_t G, uint8_t B, FaderCorner corner)
{
    // Each corner of the fader could be colored independently, thus allowing
    // to create gradient faders. It is nifty yet not so useful feature, so
    // it is completely optional - if you won't specify corner, color will be
    // set for the whole fader.

    switch(corner)
    {
        case FaderCorner::TopLeft:
            mTopLeftColor[0] = static_cast<GLfloat>(R) / 255;
            mTopLeftColor[1] = static_cast<GLfloat>(G) / 255;
            mTopLeftColor[2] = static_cast<GLfloat>(B) / 255;
            break;

        case FaderCorner::TopRight:
            mTopRightColor[0] = static_cast<GLfloat>(R) / 255;
            mTopRightColor[1] = static_cast<GLfloat>(G) / 255;
            mTopRightColor[2] = static_cast<GLfloat>(B) / 255;
            break;

        case FaderCorner::BottomLeft:
            mBottomLeftColor[0] = static_cast<GLfloat>(R) / 255;
            mBottomLeftColor[1] = static_cast<GLfloat>(G) / 255;
            mBottomLeftColor[2] = static_cast<GLfloat>(B) / 255;
            break;

        case FaderCorner::BottomRight:
            mBottomRightColor[0] = static_cast<GLfloat>(R) / 255;
            mBottomRightColor[1] = static_cast<GLfloat>(G) / 255;
            mBottomRightColor[2] = static_cast<GLfloat>(B) / 255;
            break;

        default:
            mTopRightColor[0] = static_cast<GLfloat>(R) / 255;
            mTopRightColor[1] = static_cast<GLfloat>(G) / 255;
            mTopRightColor[2] = static_cast<GLfloat>(B) / 255;

            // Copy top right corner color to all other corners.

            memcpy(mTopLeftColor, mTopRightColor, sizeof(GLfloat) * 4);
            memcpy(mBottomRightColor, mTopRightColor, sizeof(GLfloat) * 4);
            memcpy(mBottomLeftColor, mTopRightColor, sizeof(GLfloat) * 4);
            break;
    }
}

void Fader::SetBlendingMode(loader::BlendingMode mode)
{
    mBlendingMode = mode;
}

void Fader::SetSpeed(uint16_t fade_speed, uint16_t fade_speed_secondary)
{
    mSpeed = 1000.0 / static_cast<float>(fade_speed);
    mSpeedSecondary = 1000.0 / static_cast<float>(fade_speed_secondary);
}

void Fader::SetDelay(uint32_t delay_msec)
{
    mMaxTime = static_cast<float>(delay_msec) / 1000.0;
}

void Fader::SetAspect()
{
    if(mTexture)
    {
        if((static_cast<float>(mTextureWidth) / static_cast<float>(screen_info.w)) >= (static_cast<float>(mTextureHeight) / static_cast<float>(screen_info.h)))
        {
            mTextureWide = true;
            mTextureAspectRatio = static_cast<float>(mTextureHeight) / static_cast<float>(mTextureWidth);
        }
        else
        {
            mTextureWide = false;
            mTextureAspectRatio = static_cast<float>(mTextureWidth) / static_cast<float>(mTextureHeight);
        }
    }
}

bool Fader::SetTexture(const char *texture_path)
{
#ifdef __APPLE_CC__
    // Load the texture file using ImageIO
    CGDataProviderRef provider = CGDataProviderCreateWithFilename(texture_path);
    CFDictionaryRef empty = CFDictionaryCreate(kCFAllocatorDefault, nullptr, nullptr, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CGImageSourceRef source = CGImageSourceCreateWithDataProvider(provider, empty);
    CGDataProviderRelease(provider);
    CFRelease(empty);

    // Check whether loading succeeded
    CGImageSourceStatus status = CGImageSourceGetStatus(source);
    if(status != kCGImageStatusComplete)
    {
        CFRelease(source);
        ConsoleInfo::instance().warning(SYSWARN_IMAGE_NOT_LOADED, texture_path, status);
        return false;
    }

    // Get the image
    CGImageRef image = CGImageSourceCreateImageAtIndex(source, 0, nullptr);
    CFRelease(source);
    size_t width = CGImageGetWidth(image);
    size_t height = CGImageGetHeight(image);

    // Prepare the data to write to
    uint8_t *data = new uint8_t[width * height * 4];

    // Write image to bytes. This is done by drawing it into an off-screen image context using our data as the backing store
    CGColorSpaceRef deviceRgb = CGColorSpaceCreateDeviceRGB();
    CGContextRef context = CGBitmapContextCreate(data, width, height, 8, width * 4, deviceRgb, kCGImageAlphaPremultipliedFirst);
    CGColorSpaceRelease(deviceRgb);
    assert(context);

    CGContextDrawImage(context, CGRectMake(0, 0, width, height), image);

    CGContextRelease(context);
    CGImageRelease(image);

    // Drop previously assigned texture, if it exists.
    DropTexture();

    // Have OpenGL generate a texture object handle for us
    glGenTextures(1, &mTexture);

    // Bind the texture object
    glBindTexture(GL_TEXTURE_2D, mTexture);

    // Set the texture's stretching properties
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Load texture. The weird format works out to ARGB8 in the end
    // (on little-endian systems), which is what we specified above and what
    // OpenGL prefers internally.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLuint)width, (GLuint)height, 0,
                 GL_BGRA, GL_UNSIGNED_INT_8_8_8_8, data);

    // Cleanup
    delete[] data;

    // Setup the additional required information
    mTextureWidth = width;
    mTextureHeight = height;

    SetAspect();

    ConsoleInfo::instance().notify(SYSNOTE_LOADED_FADER, texture_path);
    return true;
#else
    SDL_Surface *surface = IMG_Load(texture_path);
    GLenum       texture_format;
    GLint        color_depth;

    if(surface != nullptr)
    {
        // Get the color depth of the SDL surface
        color_depth = surface->format->BytesPerPixel;

        if(color_depth == 4)        // Contains an alpha channel
        {
            if(surface->format->Rmask == 0x000000ff)
                texture_format = GL_RGBA;
            else
                texture_format = GL_BGRA;

            color_depth = GL_RGBA;
        }
        else if(color_depth == 3)   // No alpha channel
        {
            if(surface->format->Rmask == 0x000000ff)
                texture_format = GL_RGB;
            else
                texture_format = GL_BGR;

            color_depth = GL_RGB;
        }
        else
        {
            ConsoleInfo::instance().warning(SYSWARN_NOT_TRUECOLOR_IMG, texture_path);
            SDL_FreeSurface(surface);
            return false;
        }

        // Drop previously assigned texture, if it exists.
        DropTexture();

        // Have OpenGL generate a texture object handle for us
        glGenTextures(1, &mTexture);

        // Bind the texture object
        glBindTexture(GL_TEXTURE_2D, mTexture);

        // Set the texture's stretching properties
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Edit the texture object's image data using the information SDL_Surface gives us
        glTexImage2D(GL_TEXTURE_2D, 0, color_depth, surface->w, surface->h, 0,
                     texture_format, GL_UNSIGNED_BYTE, surface->pixels);
    }
    else
    {
        ConsoleInfo::instance().warning(SYSWARN_IMG_NOT_LOADED_SDL, texture_path, SDL_GetError());
        return false;
    }

    // Unbind the texture - is it really necessary?
    // glBindTexture(GL_TEXTURE_2D, 0);

    // Free the SDL_Surface only if it was successfully created
    if(surface)
    {
        // Set additional parameters
        mTextureWidth = surface->w;
        mTextureHeight = surface->h;

        SetAspect();

        ConsoleInfo::instance().notify(SYSNOTE_LOADED_FADER, texture_path);
        SDL_FreeSurface(surface);
        return true;
    }
    else
    {
        /// if mTexture == 0 then trouble
        if(glIsTexture(mTexture))
        {
            glDeleteTextures(1, &mTexture);
        }
        mTexture = 0;
        return false;
    }
#endif
}

bool Fader::DropTexture()
{
    if(mTexture)
    {
        /// if mTexture is incorrect then maybe trouble
        if(glIsTexture(mTexture))
        {
            glDeleteTextures(1, &mTexture);
        }
        mTexture = 0;
        return true;
    }
    else
    {
        return false;
    }
}

void Fader::Engage(FaderDir fade_dir)
{
    mDirection = fade_dir;
    mActive = true;
    mComplete = false;
    mCurrentTime = 0.0;

    if(mDirection == FaderDir::In)
    {
        mCurrentAlpha = mMaxAlpha;      // Fade in: set alpha to maximum.
    }
    else
    {
        mCurrentAlpha = 0.0;            // Fade out or timed: set alpha to zero.
    }
}

void Fader::Cut()
{
    mActive = false;
    mComplete = false;
    mCurrentAlpha = 0.0;
    mCurrentTime = 0.0;

    DropTexture();
}

void Fader::Show()
{
    if(!mActive)
    {
        mComplete = true;
        return;                                 // If fader is not active, don't render it.
    }

    if(mDirection == FaderDir::In)          // Fade in case
    {
        if(mCurrentAlpha > 0.0)                 // If alpha is more than zero, continue to fade.
        {
            mCurrentAlpha -= engine_frame_time * mSpeed;
        }
        else
        {
            mComplete = true;   // We've reached zero alpha, complete and disable fader.
            mActive = false;
            mCurrentAlpha = 0.0;
            DropTexture();
        }
    }
    else if(mDirection == FaderDir::Out)  // Fade out case
    {
        if(mCurrentAlpha < mMaxAlpha)   // If alpha is less than maximum, continue to fade.
        {
            mCurrentAlpha += engine_frame_time * mSpeed;
        }
        else
        {
            // We've reached maximum alpha, so complete fader but leave it active.
            // This is needed for engine to receive proper callback in case some events are
            // delayed to the next frame - e.g., level loading.

            mComplete = true;
            mCurrentAlpha = mMaxAlpha;
        }
    }
    else    // Timed fader case
    {
        if(mCurrentTime <= mMaxTime)
        {
            if(mCurrentAlpha == mMaxAlpha)
            {
                mCurrentTime += engine_frame_time;
            }
            else if(mCurrentAlpha < mMaxAlpha)
            {
                mCurrentAlpha += engine_frame_time * mSpeed;
            }
            else
            {
                mCurrentAlpha = mMaxAlpha;
            }
        }
        else
        {
            if(mCurrentAlpha > 0.0)
            {
                mCurrentAlpha -= engine_frame_time * mSpeedSecondary;
            }
            else
            {
                mComplete = true;          // We've reached zero alpha, complete and disable fader.
                mActive = false;
                mCurrentAlpha = 0.0;
                mCurrentTime = 0.0;
                DropTexture();
            }
        }
    }

    // Apply current alpha value to all vertices.

    mTopLeftColor[3] = mCurrentAlpha;
    mTopRightColor[3] = mCurrentAlpha;
    mBottomLeftColor[3] = mCurrentAlpha;
    mBottomRightColor[3] = mCurrentAlpha;

    // Draw the rectangle.
    // We draw it from the very top left corner to the end of the screen.

    if(mTexture)
    {
        // Texture is always modulated with alpha!
        GLfloat tex_color[4] = { mCurrentAlpha, mCurrentAlpha, mCurrentAlpha, mCurrentAlpha };

        if(mTextureScaleMode == FaderScale::LetterBox)
        {
            if(mTextureWide)        // Texture is wider than the screen... Do letterbox.
            {
                // Draw lower letterbox.
                Gui_DrawRect(0.0,
                             0.0,
                             screen_info.w,
                             (screen_info.h - (screen_info.w * mTextureAspectRatio)) / 2,
                             mBottomLeftColor, mBottomRightColor, mBottomLeftColor, mBottomRightColor,
                             mBlendingMode);

                // Draw texture.
                Gui_DrawRect(0.0,
                             (screen_info.h - (screen_info.w * mTextureAspectRatio)) / 2,
                             screen_info.w,
                             screen_info.w * mTextureAspectRatio,
                             tex_color, tex_color, tex_color, tex_color,
                             mBlendingMode,
                             mTexture);

                // Draw upper letterbox.
                Gui_DrawRect(0.0,
                             screen_info.h - (screen_info.h - (screen_info.w * mTextureAspectRatio)) / 2,
                             screen_info.w,
                             (screen_info.h - (screen_info.w * mTextureAspectRatio)) / 2,
                             mTopLeftColor, mTopRightColor, mTopLeftColor, mTopRightColor,
                             mBlendingMode);
            }
            else        // Texture is taller than the screen... Do pillarbox.
            {
                // Draw left pillarbox.
                Gui_DrawRect(0.0,
                             0.0,
                             (screen_info.w - (screen_info.h / mTextureAspectRatio)) / 2,
                             screen_info.h,
                             mTopLeftColor, mTopLeftColor, mBottomLeftColor, mBottomLeftColor,
                             mBlendingMode);

                // Draw texture.
                Gui_DrawRect((screen_info.w - (screen_info.h / mTextureAspectRatio)) / 2,
                             0.0,
                             screen_info.h / mTextureAspectRatio,
                             screen_info.h,
                             tex_color, tex_color, tex_color, tex_color,
                             mBlendingMode,
                             mTexture);

                // Draw right pillarbox.
                Gui_DrawRect(screen_info.w - (screen_info.w - (screen_info.h / mTextureAspectRatio)) / 2,
                             0.0,
                             (screen_info.w - (screen_info.h / mTextureAspectRatio)) / 2,
                             screen_info.h,
                             mTopRightColor, mTopRightColor, mBottomRightColor, mBottomRightColor,
                             mBlendingMode);
            }
        }
        else if(mTextureScaleMode == FaderScale::Zoom)
        {
            if(mTextureWide)    // Texture is wider than the screen - scale vertical.
            {
                Gui_DrawRect(-(((screen_info.h / mTextureAspectRatio) - screen_info.w) / 2),
                             0.0,
                             screen_info.h / mTextureAspectRatio,
                             screen_info.h,
                             tex_color, tex_color, tex_color, tex_color,
                             mBlendingMode,
                             mTexture);
            }
            else                // Texture is taller than the screen - scale horizontal.
            {
                Gui_DrawRect(0.0,
                             -(((screen_info.w / mTextureAspectRatio) - screen_info.h) / 2),
                             screen_info.w,
                             screen_info.w / mTextureAspectRatio,
                             tex_color, tex_color, tex_color, tex_color,
                             mBlendingMode,
                             mTexture);
            }
        }
        else    // Simple stretch!
        {
            Gui_DrawRect(0.0,
                         0.0,
                         screen_info.w,
                         screen_info.h,
                         tex_color, tex_color, tex_color, tex_color,
                         mBlendingMode,
                         mTexture);
        }
    }
    else    // No texture, simply draw colored rect.
    {
        Gui_DrawRect(0.0, 0.0, screen_info.w, screen_info.h,
                     mTopLeftColor, mTopRightColor, mBottomLeftColor, mBottomRightColor,
                     mBlendingMode);
    }   // end if(mTexture)
}

FaderStatus Fader::IsFading()
{
    if(mComplete)
    {
        return FaderStatus::Complete;
    }
    else if(mActive)
    {
        return FaderStatus::Fading;
    }
    else
    {
        return FaderStatus::Idle;
    }
}

// ===================================================================================
// ======================== PROGRESS BAR CLASS IMPLEMENTATION ========================
// ===================================================================================

ProgressBar::ProgressBar()
{
    // Set up some defaults.
    Visible = false;
    Alternate = false;
    Invert = false;
    Vertical = false;
    Forced = false;

    // Initialize parameters.
    // By default, bar is initialized with TR5-like health bar properties.
    SetPosition(HorizontalAnchor::Left, 20, VerticalAnchor::Top, 20);
    SetSize(250, 25, 3);
    SetColor(BarColorType::BaseMain, 255, 50, 50, 150);
    SetColor(BarColorType::BaseFade, 100, 255, 50, 150);
    SetColor(BarColorType::AltMain, 255, 180, 0, 220);
    SetColor(BarColorType::AltFade, 255, 255, 0, 220);
    SetColor(BarColorType::BackMain, 0, 0, 0, 160);
    SetColor(BarColorType::BackFade, 60, 60, 60, 130);
    SetColor(BarColorType::BorderMain, 200, 200, 200, 50);
    SetColor(BarColorType::BorderFade, 80, 80, 80, 100);
    SetValues(1000, 300);
    SetBlink(300);
    SetExtrude(true, 100);
    SetAutoshow(true, 5000, true, 1000);
}

// Resize bar.
// This function should be called every time resize event occurs.

void ProgressBar::Resize()
{
    RecalculateSize();
    RecalculatePosition();
}

// Set specified color.
void ProgressBar::SetColor(BarColorType colType,
                               uint8_t R, uint8_t G, uint8_t B, uint8_t A)
{
    float maxColValue = 255.0;

    switch(colType)
    {
        case BarColorType::BaseMain:
            mBaseMainColor[0] = static_cast<float>(R) / maxColValue;
            mBaseMainColor[1] = static_cast<float>(G) / maxColValue;
            mBaseMainColor[2] = static_cast<float>(B) / maxColValue;
            mBaseMainColor[3] = static_cast<float>(A) / maxColValue;
            mBaseMainColor[4] = mBaseMainColor[3];
            return;
        case BarColorType::BaseFade:
            mBaseFadeColor[0] = static_cast<float>(R) / maxColValue;
            mBaseFadeColor[1] = static_cast<float>(G) / maxColValue;
            mBaseFadeColor[2] = static_cast<float>(B) / maxColValue;
            mBaseFadeColor[3] = static_cast<float>(A) / maxColValue;
            mBaseFadeColor[4] = mBaseFadeColor[3];
            return;
        case BarColorType::AltMain:
            mAltMainColor[0] = static_cast<float>(R) / maxColValue;
            mAltMainColor[1] = static_cast<float>(G) / maxColValue;
            mAltMainColor[2] = static_cast<float>(B) / maxColValue;
            mAltMainColor[3] = static_cast<float>(A) / maxColValue;
            mAltMainColor[4] = mAltMainColor[3];
            return;
        case BarColorType::AltFade:
            mAltFadeColor[0] = static_cast<float>(R) / maxColValue;
            mAltFadeColor[1] = static_cast<float>(G) / maxColValue;
            mAltFadeColor[2] = static_cast<float>(B) / maxColValue;
            mAltFadeColor[3] = static_cast<float>(A) / maxColValue;
            mAltFadeColor[4] = mAltFadeColor[3];
            return;
        case BarColorType::BackMain:
            mBackMainColor[0] = static_cast<float>(R) / maxColValue;
            mBackMainColor[1] = static_cast<float>(G) / maxColValue;
            mBackMainColor[2] = static_cast<float>(B) / maxColValue;
            mBackMainColor[3] = static_cast<float>(A) / maxColValue;
            mBackMainColor[4] = mBackMainColor[3];
            return;
        case BarColorType::BackFade:
            mBackFadeColor[0] = static_cast<float>(R) / maxColValue;
            mBackFadeColor[1] = static_cast<float>(G) / maxColValue;
            mBackFadeColor[2] = static_cast<float>(B) / maxColValue;
            mBackFadeColor[3] = static_cast<float>(A) / maxColValue;
            mBackFadeColor[4] = mBackFadeColor[3];
            return;
        case BarColorType::BorderMain:
            mBorderMainColor[0] = static_cast<float>(R) / maxColValue;
            mBorderMainColor[1] = static_cast<float>(G) / maxColValue;
            mBorderMainColor[2] = static_cast<float>(B) / maxColValue;
            mBorderMainColor[3] = static_cast<float>(A) / maxColValue;
            mBorderMainColor[4] = mBorderMainColor[3];
            return;
        case BarColorType::BorderFade:
            mBorderFadeColor[0] = static_cast<float>(R) / maxColValue;
            mBorderFadeColor[1] = static_cast<float>(G) / maxColValue;
            mBorderFadeColor[2] = static_cast<float>(B) / maxColValue;
            mBorderFadeColor[3] = static_cast<float>(A) / maxColValue;
            mBorderFadeColor[4] = mBorderFadeColor[3];
            return;
        default:
            return;
    }
}

void ProgressBar::SetPosition(HorizontalAnchor anchor_X, float offset_X, VerticalAnchor anchor_Y, float offset_Y)
{
    mXanchor = anchor_X;
    mYanchor = anchor_Y;
    mAbsXoffset = offset_X;
    mAbsYoffset = offset_Y;

    RecalculatePosition();
}

// Set bar size
void ProgressBar::SetSize(float width, float height, float borderSize)
{
    // Absolute values are needed to recalculate actual bar size according to resolution.
    mAbsWidth = width;
    mAbsHeight = height;
    mAbsBorderSize = borderSize;

    RecalculateSize();
}

// Recalculate size, according to viewport resolution.
void ProgressBar::RecalculateSize()
{
    mWidth = static_cast<float>(mAbsWidth)  * screen_info.scale_factor;
    mHeight = static_cast<float>(mAbsHeight) * screen_info.scale_factor;

    mBorderWidth = static_cast<float>(mAbsBorderSize)  * screen_info.scale_factor;
    mBorderHeight = static_cast<float>(mAbsBorderSize)  * screen_info.scale_factor;

    // Calculate range unit, according to maximum bar value set up.
    // If bar alignment is set to horizontal, calculate it from bar width.
    // If bar is vertical, then calculate it from height.

    mRangeUnit = (!Vertical) ? ((mWidth) / mMaxValue) : ((mHeight) / mMaxValue);
}

// Recalculate position, according to viewport resolution.
void ProgressBar::RecalculatePosition()
{
    switch(mXanchor)
    {
        case HorizontalAnchor::Left:
            mX = static_cast<float>(mAbsXoffset + mAbsBorderSize) * screen_info.scale_factor;
            break;
        case HorizontalAnchor::Center:
            mX = (static_cast<float>(screen_info.w) - (static_cast<float>(mAbsWidth + mAbsBorderSize * 2) * screen_info.scale_factor)) / 2 +
                (static_cast<float>(mAbsXoffset) * screen_info.scale_factor);
            break;
        case HorizontalAnchor::Right:
            mX = static_cast<float>(screen_info.w) - static_cast<float>(mAbsXoffset + mAbsWidth + mAbsBorderSize * 2) * screen_info.scale_factor;
            break;
    }

    switch(mYanchor)
    {
        case VerticalAnchor::Top:
            mY = static_cast<float>(screen_info.h) - static_cast<float>(mAbsYoffset + mAbsHeight + mAbsBorderSize * 2) * screen_info.scale_factor;
            break;
        case VerticalAnchor::Center:
            mY = (static_cast<float>(screen_info.h) - (static_cast<float>(mAbsHeight + mAbsBorderSize * 2) * screen_info.h_unit)) / 2 +
                (static_cast<float>(mAbsYoffset) * screen_info.scale_factor);
            break;
        case VerticalAnchor::Bottom:
            mY = (mAbsYoffset + mAbsBorderSize) * screen_info.scale_factor;
            break;
    }
}

// Set maximum and warning state values.
void ProgressBar::SetValues(float maxValue, float warnValue)
{
    mMaxValue = maxValue;
    mWarnValue = warnValue;

    RecalculateSize();  // We need to recalculate size, because max. value is changed.
}

// Set warning state blinking interval.
void ProgressBar::SetBlink(int interval)
{
    mBlinkInterval = static_cast<float>(interval) / 1000;
    mBlinkCnt = static_cast<float>(interval) / 1000;  // Also reset blink counter.
}

// Set extrude overlay effect parameters.
void ProgressBar::SetExtrude(bool enabled, uint8_t depth)
{
    mExtrude = enabled;
    memset(mExtrudeDepth, 0, sizeof(float) * 5);    // Set all colors to 0.
    mExtrudeDepth[3] = static_cast<float>(depth) / 255.0;        // We need only alpha transparency.
    mExtrudeDepth[4] = mExtrudeDepth[3];
}

// Set autoshow and fade parameters.
// Please note that fade parameters are actually independent of autoshow.
void ProgressBar::SetAutoshow(bool enabled, int delay, bool fade, int fadeDelay)
{
    mAutoShow = enabled;

    mAutoShowDelay = static_cast<float>(delay) / 1000;
    mAutoShowCnt = static_cast<float>(delay) / 1000;     // Also reset autoshow counter.

    mAutoShowFade = fade;
    mAutoShowFadeDelay = 1000 / static_cast<float>(fadeDelay);
    mAutoShowFadeCnt = 0; // Initially, it's 0.
}

// Main bar show procedure.
// Draws a bar with a given value. Please note that it also accepts float,
// so effectively you can create bars for floating-point parameters.
void ProgressBar::Show(float value)
{
    // Initial value limiters (to prevent bar overflow).
    value = (value >= 0) ? (value) : (0);
    value = (value > mMaxValue) ? (mMaxValue) : (value);

    // Enable blink mode, if value is gone below warning value.
    mBlink = (value <= mWarnValue) ? (true) : (false);

    if(mAutoShow)   // Check autoshow visibility conditions.
    {
        // 0. If bar drawing was forced, then show a bar without additional
        //    autoshow delay set. This condition has to be overwritten by
        //    any other conditions, that's why it is set first.
        if(Forced)
        {
            Visible = true;
            Forced = false;
        }
        else
        {
            Visible = false;
        }

        // 1. If bar value gone less than warning value, we show it
        //    in any case, bypassing all other conditions.
        if(value <= mWarnValue)
            Visible = true;

        // 2. Check if bar's value changed,
        //    and if so, start showing it automatically for a given delay time.
        if(mLastValue != value)
        {
            mLastValue = value;
            Visible = true;
            mAutoShowCnt = mAutoShowDelay;
        }

        // 3. If autoshow time is up, then we hide bar,
        //    otherwise decrease delay counter.
        if(mAutoShowCnt > 0)
        {
            Visible = true;
            mAutoShowCnt -= engine_frame_time;

            if(mAutoShowCnt <= 0)
            {
                mAutoShowCnt = 0;
                Visible = false;
            }
        }
    } // end if(AutoShow)

    if(mAutoShowFade)   // Process fade-in and fade-out effect, if enabled.
    {
        if(!Visible)
        {
            // If visibility flag is off and bar is still on-screen, gradually decrease
            // fade counter, else simply don't draw anything and exit.
            if(mAutoShowFadeCnt == 0)
            {
                return;
            }
            else
            {
                mAutoShowFadeCnt -= engine_frame_time * mAutoShowFadeDelay;
                if(mAutoShowFadeCnt < 0)
                    mAutoShowFadeCnt = 0;
            }
        }
        else
        {
            // If visibility flag is on, and bar is not yet fully visible, gradually
            // increase fade counter, until it's 1 (i. e. fully opaque).
            if(mAutoShowFadeCnt < 1)
            {
                mAutoShowFadeCnt += engine_frame_time * mAutoShowFadeDelay;
                if(mAutoShowFadeCnt > 1)
                    mAutoShowFadeCnt = 1;
            }
        } // end if(!Visible)

        // Multiply all layers' alpha by current fade counter.
        mBaseMainColor[3] = mBaseMainColor[4] * mAutoShowFadeCnt;
        mBaseFadeColor[3] = mBaseFadeColor[4] * mAutoShowFadeCnt;
        mAltMainColor[3] = mAltMainColor[4] * mAutoShowFadeCnt;
        mAltFadeColor[3] = mAltFadeColor[4] * mAutoShowFadeCnt;
        mBackMainColor[3] = mBackMainColor[4] * mAutoShowFadeCnt;
        mBackFadeColor[3] = mBackFadeColor[4] * mAutoShowFadeCnt;
        mBorderMainColor[3] = mBorderMainColor[4] * mAutoShowFadeCnt;
        mBorderFadeColor[3] = mBorderFadeColor[4] * mAutoShowFadeCnt;
        mExtrudeDepth[3] = mExtrudeDepth[4] * mAutoShowFadeCnt;
    }
    else
    {
        if(!Visible) return;   // Obviously, quit, if bar is not visible.
    } // end if(mAutoShowFade)

    // Draw border rect.
    // Border rect should be rendered first, as it lies beneath actual bar,
    // and additionally, we need to show it in any case, even if bar is in
    // warning state (blinking).
    Gui_DrawRect(mX, mY, mWidth + (mBorderWidth * 2), mHeight + (mBorderHeight * 2),
                 mBorderMainColor, mBorderMainColor,
                 mBorderFadeColor, mBorderFadeColor,
                 loader::BlendingMode::Opaque);

    // SECTION FOR BASE BAR RECTANGLE.

    // We check if bar is in a warning state. If it is, we blink it continously.
    if(mBlink)
    {
        mBlinkCnt -= engine_frame_time;
        if(mBlinkCnt > mBlinkInterval)
        {
            value = 0; // Force zero value, which results in empty bar.
        }
        else if(mBlinkCnt <= 0)
        {
            mBlinkCnt = mBlinkInterval * 2;
        }
    }

    // If bar value is zero, just render background overlay and immediately exit.
    // It is needed in case bar is used as a simple UI box to bypass unnecessary calculations.
    if(!value)
    {
        // Draw full-sized background rect (instead of base bar rect)
        Gui_DrawRect(mX + mBorderWidth, mY + mBorderHeight, mWidth, mHeight,
                     mBackMainColor, (Vertical) ? (mBackFadeColor) : (mBackMainColor),
                     (Vertical) ? (mBackMainColor) : (mBackFadeColor), mBackFadeColor,
                     loader::BlendingMode::Opaque);
        return;
    }

    // Calculate base bar width, according to current value and range unit.
    mBaseSize = mRangeUnit * value;
    mBaseRatio = value / mMaxValue;

    float RectAnchor;           // Anchor to stick base bar rect, according to Invert flag.
    float RectFirstColor[4];    // Used to recalculate gradient, according to current value.
    float RectSecondColor[4];

    // If invert decrease direction style flag is set, we position bar in a way
    // that it seems like it's decreasing to another side, and also swap main / fade colours.
    if(Invert)
    {
        memcpy(RectFirstColor,
               (Alternate) ? (mAltMainColor) : (mBaseMainColor),
               sizeof(float) * 4);

        // Main-fade gradient is recalculated according to current / maximum value ratio.
        for(int i = 0; i <= 3; i++)
            RectSecondColor[i] = (Alternate) ? ((mBaseRatio * mAltFadeColor[i]) + ((1 - mBaseRatio) * mAltMainColor[i]))
            : ((mBaseRatio * mBaseFadeColor[i]) + ((1 - mBaseRatio) * mBaseMainColor[i]));
    }
    else
    {
        memcpy(RectSecondColor,
               (Alternate) ? (mAltMainColor) : (mBaseMainColor),
               sizeof(float) * 4);

        // Main-fade gradient is recalculated according to current / maximum value ratio.
        for(int i = 0; i <= 3; i++)
            RectFirstColor[i] = (Alternate) ? ((mBaseRatio * mAltFadeColor[i]) + ((1 - mBaseRatio) * mAltMainColor[i]))
            : ((mBaseRatio * mBaseFadeColor[i]) + ((1 - mBaseRatio) * mBaseMainColor[i]));
    } // end if(Invert)

    // We need to reset Alternate flag each frame, cause behaviour is immediate.

    Alternate = false;

    // If vertical style flag is set, we draw bar base top-bottom, else we draw it left-right.
    if(Vertical)
    {
        RectAnchor = ((Invert) ? (mY + mHeight - mBaseSize) : (mY)) + mBorderHeight;

        // Draw actual bar base.
        Gui_DrawRect(mX + mBorderWidth, RectAnchor,
                     mWidth, mBaseSize,
                     RectFirstColor, RectFirstColor,
                     RectSecondColor, RectSecondColor,
                     loader::BlendingMode::Opaque);

        // Draw background rect.
        Gui_DrawRect(mX + mBorderWidth,
                     (Invert) ? (mY + mBorderHeight) : (RectAnchor + mBaseSize),
                     mWidth, mHeight - mBaseSize,
                     mBackMainColor, mBackFadeColor,
                     mBackMainColor, mBackFadeColor,
                     loader::BlendingMode::Opaque);

        if(mExtrude)    // Draw extrude overlay, if flag is set.
        {
            float transparentColor[4] = { 0 };  // Used to set counter-shade to transparent.

            Gui_DrawRect(mX + mBorderWidth, RectAnchor,
                         mWidth / 2, mBaseSize,
                         mExtrudeDepth, transparentColor,
                         mExtrudeDepth, transparentColor,
                         loader::BlendingMode::Opaque);
            Gui_DrawRect(mX + mBorderWidth + mWidth / 2, RectAnchor,
                         mWidth / 2, mBaseSize,
                         transparentColor, mExtrudeDepth,
                         transparentColor, mExtrudeDepth,
                         loader::BlendingMode::Opaque);
        }
    }
    else
    {
        RectAnchor = ((Invert) ? (mX + mWidth - mBaseSize) : (mX)) + mBorderWidth;

        // Draw actual bar base.
        Gui_DrawRect(RectAnchor, mY + mBorderHeight,
                     mBaseSize, mHeight,
                     RectSecondColor, RectFirstColor,
                     RectSecondColor, RectFirstColor,
                     loader::BlendingMode::Opaque);

        // Draw background rect.
        Gui_DrawRect((Invert) ? (mX + mBorderWidth) : (RectAnchor + mBaseSize),
                     mY + mBorderHeight,
                     mWidth - mBaseSize, mHeight,
                     mBackMainColor, mBackMainColor,
                     mBackFadeColor, mBackFadeColor,
                     loader::BlendingMode::Opaque);

        if(mExtrude)    // Draw extrude overlay, if flag is set.
        {
            float transparentColor[4] = { 0 };  // Used to set counter-shade to transparent.

            Gui_DrawRect(RectAnchor, mY + mBorderHeight,
                         mBaseSize, mHeight / 2,
                         transparentColor, transparentColor,
                         mExtrudeDepth, mExtrudeDepth,
                         loader::BlendingMode::Opaque);
            Gui_DrawRect(RectAnchor, mY + mBorderHeight + (mHeight / 2),
                         mBaseSize, mHeight / 2,
                         mExtrudeDepth, mExtrudeDepth,
                         transparentColor, transparentColor,
                         loader::BlendingMode::Opaque);
        }
    } // end if(Vertical)
}

// ===================================================================================
// ======================== ITEM NOTIFIER CLASS IMPLEMENTATION =======================
// ===================================================================================

gui_ItemNotifier::gui_ItemNotifier()
{
    SetPos(850, 850);
    SetRot(0, 0);
    SetSize(1.0);
    SetRotateTime(1000.0);

    mItem = 0;
    mActive = false;
}

void gui_ItemNotifier::Start(int item, float time)
{
    Reset();

    mItem = item;
    mShowTime = time;
    mActive = true;
}

void gui_ItemNotifier::Animate()
{
    if(!mActive)
    {
        return;
    }
    else
    {
        if(mRotateTime)
        {
            mCurrRotX += (engine_frame_time * mRotateTime);
            //mCurrRotY += (engine_frame_time * mRotateTime);

            mCurrRotX = (mCurrRotX > 360.0) ? (mCurrRotX - 360.0) : (mCurrRotX);
            //mCurrRotY = (mCurrRotY > 360.0)?(mCurrRotY - 360.0):(mCurrRotY);
        }

        if(mCurrTime == 0)
        {
            float step = (mCurrPosX - mEndPosX) * (engine_frame_time * 4.0);
            step = (step <= 0.5) ? (0.5) : (step);

            mCurrPosX -= step;
            mCurrPosX = (mCurrPosX < mEndPosX) ? (mEndPosX) : (mCurrPosX);

            if(mCurrPosX == mEndPosX)
                mCurrTime += engine_frame_time;
        }
        else if(mCurrTime < mShowTime)
        {
            mCurrTime += engine_frame_time;
        }
        else
        {
            float step = (mCurrPosX - mEndPosX) * (engine_frame_time * 4.0);
            step = (step <= 0.5) ? (0.5) : (step);

            mCurrPosX += step;
            mCurrPosX = (mCurrPosX > mStartPosX) ? (mStartPosX) : (mCurrPosX);

            if(mCurrPosX == mStartPosX)
                Reset();
        }
    }
}

void gui_ItemNotifier::Reset()
{
    mActive = false;
    mCurrTime = 0.0;
    mCurrRotX = 0.0;
    mCurrRotY = 0.0;

    mEndPosX = (static_cast<float>(screen_info.w) / ScreenMeteringResolution) * mAbsPosX;
    mPosY = (static_cast<float>(screen_info.h) / ScreenMeteringResolution) * mAbsPosY;
    mCurrPosX = screen_info.w + (static_cast<float>(screen_info.w) / GUI_NOTIFIER_OFFSCREEN_DIVIDER * mSize);
    mStartPosX = mCurrPosX;    // Equalize current and start positions.
}

void gui_ItemNotifier::Draw()
{
    if(!mActive)
        return;

    auto item = engine_world.getBaseItemByID(mItem);
    if(!item)
        return;

    int anim = item->bf->animations.current_animation;
    int frame = item->bf->animations.current_frame;
    btScalar time = item->bf->animations.frame_time;

    item->bf->animations.current_animation = 0;
    item->bf->animations.current_frame = 0;
    item->bf->animations.frame_time = 0.0;

    Item_Frame(item->bf.get(), 0.0);
    btTransform matrix;
    matrix.setIdentity();
    Mat4_Translate(matrix, mCurrPosX, mPosY, -2048.0);
    Mat4_RotateY(matrix, mCurrRotX + mRotX);
    Mat4_RotateX(matrix, mCurrRotY + mRotY);
    Gui_RenderItem(item->bf.get(), mSize, matrix);

    item->bf->animations.current_animation = anim;
    item->bf->animations.current_frame = frame;
    item->bf->animations.frame_time = time;
}

void gui_ItemNotifier::SetPos(float X, float Y)
{
    mAbsPosX = X;
    mAbsPosY = 1000.0f - Y;
}

void gui_ItemNotifier::SetRot(float X, float Y)
{
    mRotX = X;
    mRotY = Y;
}

void gui_ItemNotifier::SetSize(float size)
{
    mSize = size;
}

void gui_ItemNotifier::SetRotateTime(float time)
{
    mRotateTime = (1000.0f / time) * 360.0f;
}

// ===================================================================================
// ======================== FONT MANAGER  CLASS IMPLEMENTATION =======================
// ===================================================================================

FontManager::FontManager()
{
    this->font_library = nullptr;
    FT_Init_FreeType(&this->font_library);

    this->mFadeValue = 0.0;
    this->mFadeDirection = true;
}

FontManager::~FontManager()
{
    // must be freed before releasing the library
    styles.clear();
    fonts.clear();
    FT_Done_FreeType(this->font_library);
    this->font_library = nullptr;
}

FontTexture *FontManager::GetFont(const FontType index)
{
    for(const Font& current_font : this->fonts)
    {
        if(current_font.index == index)
        {
            return current_font.gl_font.get();
        }
    }

    return nullptr;
}

Font *FontManager::GetFontAddress(const FontType index)
{
    for(Font& current_font : this->fonts)
    {
        if(current_font.index == index)
        {
            return &current_font;
        }
    }

    return nullptr;
}

FontStyleData *FontManager::GetFontStyle(const FontStyle index)
{
    for(FontStyleData& current_style : this->styles)
    {
        if(current_style.index == index)
        {
            return &current_style;
        }
    }

    return nullptr;
}

bool FontManager::AddFont(const FontType index, const uint32_t size, const char* path)
{
    if((size < MinFontSize) || (size > MaxFontSize))
    {
        return false;
    }

    Font* desired_font = GetFontAddress(index);

    if(desired_font == nullptr)
    {
        if(this->fonts.size() >= MaxFonts)
        {
            return false;
        }

        this->fonts.emplace_front();
        desired_font = &this->fonts.front();
        desired_font->size = static_cast<uint16_t>(size);
        desired_font->index = index;
    }

    desired_font->gl_font = glf_create_font(this->font_library, path, size);

    return true;
}

bool FontManager::AddFontStyle(const FontStyle index,
                                   const GLfloat R, const GLfloat G, const GLfloat B, const GLfloat A,
                                   const bool shadow, const bool fading,
                                   const bool rect, const GLfloat rect_border,
                                   const GLfloat rect_R, const GLfloat rect_G, const GLfloat rect_B, const GLfloat rect_A,
                                   const bool hide)
{
    FontStyleData* desired_style = GetFontStyle(index);

    if(desired_style == nullptr)
    {
        if(this->styles.size() >= static_cast<int>(FontStyle::Sentinel))
        {
            return false;
        }

        this->styles.emplace_front();
        desired_style = &this->styles.front();
        desired_style->index = index;
    }

    desired_style->rect_border = rect_border;
    desired_style->rect_color[0] = rect_R;
    desired_style->rect_color[1] = rect_G;
    desired_style->rect_color[2] = rect_B;
    desired_style->rect_color[3] = rect_A;

    desired_style->color[0] = R;
    desired_style->color[1] = G;
    desired_style->color[2] = B;
    desired_style->color[3] = A;

    memcpy(desired_style->real_color, desired_style->color, sizeof(GLfloat) * 4);

    desired_style->fading = fading;
    desired_style->shadowed = shadow;
    desired_style->rect = rect;
    desired_style->hidden = hide;

    return true;
}

bool FontManager::RemoveFont(const FontType index)
{
    if(this->fonts.empty())
    {
        return false;
    }

    for(auto it = this->fonts.begin(); it != this->fonts.end(); ++it)
    {
        if(it->index == index)
        {
            this->fonts.erase(it);
            return true;
        }
    }

    return false;
}

bool FontManager::RemoveFontStyle(const FontStyle index)
{
    if(this->styles.empty())
    {
        return false;
    }

    for(auto it = this->styles.begin(); it != this->styles.end(); ++it)
    {
        if(it->index == index)
        {
            this->styles.erase(it);
            return true;
        }
    }

    return false;
}

void FontManager::Update()
{
    if(this->mFadeDirection)
    {
        this->mFadeValue += engine_frame_time * FontFadeSpeed;

        if(this->mFadeValue >= 1.0)
        {
            this->mFadeValue = 1.0;
            this->mFadeDirection = false;
        }
    }
    else
    {
        this->mFadeValue -= engine_frame_time * FontFadeSpeed;

        if(this->mFadeValue <= FontFadeMin)
        {
            this->mFadeValue = FontFadeMin;
            this->mFadeDirection = true;
        }
    }

    for(FontStyleData& current_style : this->styles)
    {
        if(current_style.fading)
        {
            current_style.real_color[0] = current_style.color[0] * this->mFadeValue;
            current_style.real_color[1] = current_style.color[1] * this->mFadeValue;
            current_style.real_color[2] = current_style.color[2] * this->mFadeValue;
        }
        else
        {
            std::copy_n( current_style.color, 3, current_style.real_color );
        }
    }
}

void FontManager::Resize()
{
    for(Font& current_font : this->fonts)
    {
        glf_resize(current_font.gl_font.get(), static_cast<uint16_t>(static_cast<float>(current_font.size) * screen_info.scale_factor));
    }
}
