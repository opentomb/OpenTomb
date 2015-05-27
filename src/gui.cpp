
#include <stdint.h>
#ifdef __APPLE_CC__
#include <ImageIO/ImageIO.h>
#else
#include <SDL2/SDL_image.h>
#endif

#include "gl_util.h"
#include "gl_font.h"

#include "gui.h"
#include "character_controller.h"
#include "engine.h"
#include "render.h"
#include "system.h"
#include "console.h"
#include "vmath.h"
#include "camera.h"
#include "string.h"

extern SDL_Window  *sdl_window;

gui_text_line_p     gui_base_lines = NULL;
gui_text_line_t     gui_temp_lines[GUI_MAX_TEMP_LINES];
uint16_t            temp_lines_used = 0;

gui_ItemNotifier    Notifier;
gui_ProgressBar     Bar[BAR_LASTINDEX];
gui_Fader           Fader[FADER_LASTINDEX];

gui_FontManager       *FontManager = NULL;
gui_InventoryManager  *main_inventory_manager = NULL;

GLhandleARB square_program;
GLint       square_program_offset;
GLint       square_program_factor;
GLhandleARB square_texture_program;
GLint       square_texture_program_sampler;
GLint       square_texture_program_offset;
GLint       square_texture_program_factor;

void Gui_Init()
{
    Gui_InitShaders();
    Gui_InitBars();
    Gui_InitFaders();
    Gui_InitNotifier();
    Gui_InitTempLines();

    //main_inventory_menu = new gui_InventoryMenu();
    main_inventory_manager = new gui_InventoryManager();
}

void Gui_InitShaders()
{
    // Colored square program
    GLhandleARB squareVertexShader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
    loadShaderFromFile(squareVertexShader, "shaders/square.vsh");
    GLhandleARB squareFragmentShader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
    loadShaderFromFile(squareFragmentShader, "shaders/square.fsh");
    GLhandleARB squareFragmentShaderTexture = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
    loadShaderFromFile(squareFragmentShaderTexture, "shaders/square_tex.fsh");

    square_program = glCreateProgramObjectARB();
    glAttachObjectARB(square_program, squareVertexShader);
    glAttachObjectARB(square_program, squareFragmentShader);
    glLinkProgramARB(square_program);
    square_texture_program_offset = glGetUniformLocationARB(square_program, "offset");
    square_texture_program_factor = glGetUniformLocationARB(square_program, "factor");
    printInfoLog(square_program);

    square_texture_program = glCreateProgramObjectARB();
    glAttachObjectARB(square_texture_program, squareVertexShader);
    glAttachObjectARB(square_texture_program, squareFragmentShaderTexture);
    glLinkProgramARB(square_texture_program);
    printInfoLog(square_texture_program);
    square_texture_program_sampler = glGetUniformLocationARB(square_texture_program, "color_map");
    square_texture_program_offset = glGetUniformLocationARB(square_texture_program, "offset");
    square_texture_program_factor = glGetUniformLocationARB(square_texture_program, "factor");

    glDeleteObjectARB(squareVertexShader);
    glDeleteObjectARB(squareFragmentShader);
    glDeleteObjectARB(squareFragmentShaderTexture);
}

void Gui_InitFontManager()
{
    FontManager = new gui_FontManager();
}

void Gui_InitTempLines()
{
    for(int i=0;i<GUI_MAX_TEMP_LINES;i++)
    {
        gui_temp_lines[i].text_size = GUI_LINE_DEFAULTSIZE;
        gui_temp_lines[i].text = (char*)malloc(GUI_LINE_DEFAULTSIZE * sizeof(char));
        gui_temp_lines[i].text[0] = 0;
        gui_temp_lines[i].show = 0;

        gui_temp_lines[i].next = NULL;
        gui_temp_lines[i].prev = NULL;

        gui_temp_lines[i].font_id  = FONT_SECONDARY;
        gui_temp_lines[i].style_id = FONTSTYLE_GENERIC;
    }
}

void Gui_InitBars()
{
    for(int i = 0; i < BAR_LASTINDEX; i++)
    {
        switch(i)
        {
            case BAR_HEALTH:
                {
                    Bar[i].Visible =      false;
                    Bar[i].Alternate =    false;
                    Bar[i].Invert =       false;
                    Bar[i].Vertical =     false;

                    Bar[i].SetSize(250, 15, 3);
                    Bar[i].SetPosition(GUI_ANCHOR_HOR_LEFT, 30, GUI_ANCHOR_VERT_TOP, 30);
                    Bar[i].SetColor(BASE_MAIN, 255, 50, 50, 200);
                    Bar[i].SetColor(BASE_FADE, 100, 255, 50, 200);
                    Bar[i].SetColor(ALT_MAIN, 255, 180, 0, 255);
                    Bar[i].SetColor(ALT_FADE, 255, 255, 0, 255);
                    Bar[i].SetColor(BACK_MAIN, 0, 0, 0, 160);
                    Bar[i].SetColor(BACK_FADE, 60, 60, 60, 130);
                    Bar[i].SetColor(BORDER_MAIN, 200, 200, 200, 50);
                    Bar[i].SetColor(BORDER_FADE, 80, 80, 80, 100);
                    Bar[i].SetValues(LARA_PARAM_HEALTH_MAX, LARA_PARAM_HEALTH_MAX / 3);
                    Bar[i].SetBlink(300);
                    Bar[i].SetExtrude(true, 100);
                    Bar[i].SetAutoshow(true, 2000, true, 400);
                }
                break;
            case BAR_AIR:
                {
                    Bar[i].Visible =      false;
                    Bar[i].Alternate =    false;
                    Bar[i].Invert =       true;
                    Bar[i].Vertical =     false;

                    Bar[i].SetSize(250, 15, 3);
                    Bar[i].SetPosition(GUI_ANCHOR_HOR_RIGHT, 30, GUI_ANCHOR_VERT_TOP, 30);
                    Bar[i].SetColor(BASE_MAIN, 0, 50, 255, 200);
                    Bar[i].SetColor(BASE_FADE, 190, 190, 255, 200);
                    Bar[i].SetColor(BACK_MAIN, 0, 0, 0, 160);
                    Bar[i].SetColor(BACK_FADE, 60, 60, 60, 130);
                    Bar[i].SetColor(BORDER_MAIN, 200, 200, 200, 50);
                    Bar[i].SetColor(BORDER_FADE, 80, 80, 80, 100);
                    Bar[i].SetValues(LARA_PARAM_AIR_MAX, (LARA_PARAM_AIR_MAX / 3));
                    Bar[i].SetBlink(300);
                    Bar[i].SetExtrude(true, 100);
                    Bar[i].SetAutoshow(true, 2000, true, 400);
                }
                break;
            case BAR_STAMINA:
                {
                    Bar[i].Visible =      false;
                    Bar[i].Alternate =    false;
                    Bar[i].Invert =       false;
                    Bar[i].Vertical =     false;

                    Bar[i].SetSize(250, 15, 3);
                    Bar[i].SetPosition(GUI_ANCHOR_HOR_LEFT, 30, GUI_ANCHOR_VERT_TOP, 55);
                    Bar[i].SetColor(BASE_MAIN, 255, 100, 50, 200);
                    Bar[i].SetColor(BASE_FADE, 255, 200, 0, 200);
                    Bar[i].SetColor(BACK_MAIN, 0, 0, 0, 160);
                    Bar[i].SetColor(BACK_FADE, 60, 60, 60, 130);
                    Bar[i].SetColor(BORDER_MAIN, 110, 110, 110, 100);
                    Bar[i].SetColor(BORDER_FADE, 60, 60, 60, 180);
                    Bar[i].SetValues(LARA_PARAM_STAMINA_MAX, 0);
                    Bar[i].SetExtrude(true, 100);
                    Bar[i].SetAutoshow(true, 500, true, 300);
                }
                break;
            case BAR_WARMTH:
                {
                    Bar[i].Visible =      false;
                    Bar[i].Alternate =    false;
                    Bar[i].Invert =       true;
                    Bar[i].Vertical =     false;

                    Bar[i].SetSize(250, 15, 3);
                    Bar[i].SetPosition(GUI_ANCHOR_HOR_RIGHT, 30, GUI_ANCHOR_VERT_TOP, 55);
                    Bar[i].SetColor(BASE_MAIN, 255, 0, 255, 255);
                    Bar[i].SetColor(BASE_FADE, 190, 120, 255, 255);
                    Bar[i].SetColor(BACK_MAIN, 0, 0, 0, 160);
                    Bar[i].SetColor(BACK_FADE, 60, 60, 60, 130);
                    Bar[i].SetColor(BORDER_MAIN, 200, 200, 200, 50);
                    Bar[i].SetColor(BORDER_FADE, 80, 80, 80, 100);
                    Bar[i].SetValues(LARA_PARAM_WARMTH_MAX, LARA_PARAM_WARMTH_MAX / 3);
                    Bar[i].SetBlink(200);
                    Bar[i].SetExtrude(true, 60);
                    Bar[i].SetAutoshow(true, 500, true, 300);
                }
                break;

            case BAR_LOADING:
                {
                    Bar[i].Visible =      true;
                    Bar[i].Alternate =    false;
                    Bar[i].Invert =       false;
                    Bar[i].Vertical =     false;

                    Bar[i].SetSize(800, 25, 3);
                    Bar[i].SetPosition(GUI_ANCHOR_HOR_CENTER, 0, GUI_ANCHOR_VERT_BOTTOM, 40);
                    Bar[i].SetColor(BASE_MAIN, 255, 225, 127, 230);
                    Bar[i].SetColor(BASE_FADE, 255, 187, 136, 230);
                    Bar[i].SetColor(BACK_MAIN, 30, 30, 30, 100);
                    Bar[i].SetColor(BACK_FADE, 60, 60, 60, 100);
                    Bar[i].SetColor(BORDER_MAIN, 200, 200, 200, 80);
                    Bar[i].SetColor(BORDER_FADE, 80, 80, 80, 80);
                    Bar[i].SetValues(1000, 0);
                    Bar[i].SetExtrude(true, 70);
                    Bar[i].SetAutoshow(false, 500, false, 300);
                }
                break;
        } // end switch(i)
    } // end for(int i = 0; i < BAR_LASTINDEX; i++)
}

void Gui_InitFaders()
{
    for(int i = 0; i < FADER_LASTINDEX; i++)
    {
        switch(i)
        {
            case FADER_LOADSCREEN:
                {
                    Fader[i].SetAlpha(255);
                    Fader[i].SetColor(0, 0, 0);
                    Fader[i].SetBlendingMode(BM_OPAQUE);
                    Fader[i].SetSpeed(500);
                    Fader[i].SetScaleMode(GUI_FADER_SCALE_ZOOM);
                }
                break;

            case FADER_EFFECT:
                {
                    Fader[i].SetAlpha(255);
                    Fader[i].SetColor(255,180,0);
                    Fader[i].SetBlendingMode(BM_MULTIPLY);
                    Fader[i].SetSpeed(10,800);
                }

            case FADER_BLACK:
                {
                    Fader[i].SetAlpha(255);
                    Fader[i].SetColor(0, 0, 0);
                    Fader[i].SetBlendingMode(BM_OPAQUE);
                    Fader[i].SetSpeed(500);
                    Fader[i].SetScaleMode(GUI_FADER_SCALE_ZOOM);
                }
                break;
        }
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
    for(int i = 0; i < GUI_MAX_TEMP_LINES ;i++)
    {
        gui_temp_lines[i].show = 0;
        gui_temp_lines[i].text_size = 0;
        free(gui_temp_lines[i].text);
        gui_temp_lines[i].text = NULL;
    }

    for(int i = 0; i < FADER_LASTINDEX; i++)
    {
        Fader[i].Cut();
    }

    temp_lines_used = GUI_MAX_TEMP_LINES;

    /*if(main_inventory_menu)
    {
        delete main_inventory_menu;
        main_inventory_menu = NULL;
    }*/

    if(main_inventory_manager)
    {
        delete main_inventory_manager;
        main_inventory_manager = NULL;
    }

    if(FontManager)
    {
        delete FontManager;
        FontManager = NULL;
    }

    glDeleteObjectARB(square_program);
    glDeleteObjectARB(square_texture_program);
}

void Gui_AddLine(gui_text_line_p line)
{
    if(gui_base_lines == NULL)
    {
        gui_base_lines = line;
        line->next = NULL;
        line->prev = NULL;
        return;
    }

    line->prev = NULL;
    line->next = gui_base_lines;
    gui_base_lines->prev = line;
    gui_base_lines = line;
}

// line must be in the list, otherway You crash engine!
void Gui_DeleteLine(gui_text_line_p line)
{
    if(line == gui_base_lines)
    {
        gui_base_lines = line->next;
        if(gui_base_lines != NULL)
        {
            gui_base_lines->prev = NULL;
        }
        return;
    }

    line->prev->next = line->next;
    if(line->next)
    {
        line->next->prev = line->prev;
    }
}

void Gui_MoveLine(gui_text_line_p line)
{
    line->absXoffset = line->X * screen_info.scale_factor;
    line->absYoffset = line->Y * screen_info.scale_factor;
}

/**
 * For simple temporary lines rendering.
 * Really all strings will be rendered in Gui_Render() function.
 */
gui_text_line_p Gui_OutTextXY(GLfloat x, GLfloat y, const char *fmt, ...)
{
    if(FontManager && (temp_lines_used < GUI_MAX_TEMP_LINES - 1))
    {
        va_list argptr;
        gui_text_line_p l = gui_temp_lines + temp_lines_used;

        l->font_id = FONT_SECONDARY;
        l->style_id = FONTSTYLE_GENERIC;

        va_start(argptr, fmt);
        vsnprintf(l->text, GUI_LINE_DEFAULTSIZE, fmt, argptr);
        va_end(argptr);

        l->next = NULL;
        l->prev = NULL;

        temp_lines_used++;

        l->X = x;
        l->Y = y;
        l->Xanchor = GUI_ANCHOR_HOR_LEFT;
        l->Yanchor = GUI_ANCHOR_VERT_BOTTOM;

        l->absXoffset = l->X * screen_info.scale_factor;
        l->absYoffset = l->Y * screen_info.scale_factor;

        l->show = 1;
        return l;
    }

    return NULL;
}

void Gui_Update()
{
    if(FontManager != NULL)
    {
        FontManager->Update();
    }
}

void Gui_Resize()
{
    gui_text_line_p l = gui_base_lines;

    while(l)
    {
        l->absXoffset = l->X * screen_info.scale_factor;
        l->absYoffset = l->Y * screen_info.scale_factor;

        l = l->next;
    }

    l = gui_temp_lines;
    for(uint16_t i=0;i<temp_lines_used;i++,l++)
    {
        l->absXoffset = l->X * screen_info.scale_factor;
        l->absYoffset = l->Y * screen_info.scale_factor;
    }

    for(int i = 0; i < BAR_LASTINDEX; i++)
    {
        Bar[i].Resize();
    }

    if(FontManager)
    {
        FontManager->Resize();
    }

    /* let us update console too */
    Con_SetLineInterval(con_base.spacing);
}

void Gui_Render()
{
    glPushAttrib(GL_ENABLE_BIT | GL_PIXEL_MODE_BIT | GL_COLOR_BUFFER_BIT);
    glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);

    glBindTexture(GL_TEXTURE_2D, 0);
    glPolygonMode(GL_FRONT, GL_FILL);
    glFrontFace(GL_CCW);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_LIGHTING);
    glDisable(GL_ALPHA_TEST);
    glDepthMask(GL_FALSE);

    glPixelStorei(GL_UNPACK_LSB_FIRST, GL_FALSE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glLoadIdentity();
    Gui_DrawCrosshair();
    Gui_DrawBars();
    Gui_DrawFaders();
    Gui_RenderStrings();
    Con_Draw();

    glDepthMask(GL_TRUE);
    glPopClientAttrib();
    glPopAttrib();
}

void Gui_RenderStringLine(gui_text_line_p l)
{
    GLfloat real_x = 0.0, real_y = 0.0;

    if(FontManager == NULL)
    {
        return;
    }

    gl_tex_font_p gl_font = FontManager->GetFont((font_Type)l->font_id);
    gui_fontstyle_p style = FontManager->GetFontStyle((font_Style)l->style_id);

    if((gl_font == NULL) || (style == NULL) || (!l->show) || (style->hidden))
    {
        return;
    }

    glf_get_string_bb(gl_font, l->text, -1, l->rect+0, l->rect+1, l->rect+2, l->rect+3);

    switch(l->Xanchor)
    {
        case GUI_ANCHOR_HOR_LEFT:
            real_x = l->absXoffset;   // Used with center and right alignments.
            break;
        case GUI_ANCHOR_HOR_RIGHT:
            real_x = (float)screen_info.w - (l->rect[2] - l->rect[0]) - l->absXoffset;
            break;
        case GUI_ANCHOR_HOR_CENTER:
            real_x = ((float)screen_info.w / 2.0) - ((l->rect[2] - l->rect[0]) / 2.0) + l->absXoffset;  // Absolute center.
            break;
    }

    switch(l->Yanchor)
    {
        case GUI_ANCHOR_VERT_BOTTOM:
            real_y += l->absYoffset;
            break;
        case GUI_ANCHOR_VERT_TOP:
            real_y = (float)screen_info.h - (l->rect[3] - l->rect[1]) - l->absYoffset;
            break;
        case GUI_ANCHOR_VERT_CENTER:
            real_y = ((float)screen_info.h / 2.0) + (l->rect[3] - l->rect[1]) - l->absYoffset;          // Consider the baseline.
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

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    if(style->shadowed)
    {
        gl_font->gl_font_color[0] = 0.0f;
        gl_font->gl_font_color[1] = 0.0f;
        gl_font->gl_font_color[2] = 0.0f;
        gl_font->gl_font_color[3] = (float)style->color[3] * GUI_FONT_SHADOW_TRANSPARENCY;// Derive alpha from base color.
        glf_render_str(gl_font,
                       (real_x + GUI_FONT_SHADOW_HORIZONTAL_SHIFT),
                       (real_y + GUI_FONT_SHADOW_VERTICAL_SHIFT  ),
                       l->text);
    }

    vec4_copy(gl_font->gl_font_color, style->real_color);
    glf_render_str(gl_font, real_x, real_y, l->text);
}

void Gui_RenderStrings()
{
    if(FontManager != NULL)
    {
        gui_text_line_p l = gui_base_lines;

        glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        while(l)
        {
            Gui_RenderStringLine(l);
            l = l->next;
        }

        l = gui_temp_lines;
        for(uint16_t i=0;i<temp_lines_used;i++,l++)
        {
            if(l->show)
            {
                Gui_RenderStringLine(l);
                l->show = 0;
            }
        }

        glPopClientAttrib();
        temp_lines_used = 0;
    }
}


/**
 * That function updates item animation and rebuilds skeletal matrices;
 * @param bf - extended bone frame of the item;
 */
void Item_Frame(struct ss_bone_frame_s *bf, btScalar time)
{
    int16_t frame, anim;
    long int t;
    btScalar dt;
    state_change_p stc;

    bf->animations.lerp = 0.0;
    stc = Anim_FindStateChangeByID(bf->animations.model->animations + bf->animations.current_animation, bf->animations.next_state);
    Entity_GetNextFrame(bf, time, stc, &frame, &anim, 0x00);
    if(anim != bf->animations.current_animation)
    {
        bf->animations.last_animation = bf->animations.current_animation;
        /*frame %= bf->model->animations[anim].frames_count;
        frame = (frame >= 0)?(frame):(bf->model->animations[anim].frames_count - 1 + frame);

        bf->last_state = bf->model->animations[anim].state_id;
        bf->next_state = bf->model->animations[anim].state_id;
        bf->current_animation = anim;
        bf->current_frame = frame;
        bf->next_animation = anim;
        bf->next_frame = frame;*/
        stc = Anim_FindStateChangeByID(bf->animations.model->animations + bf->animations.current_animation, bf->animations.next_state);
    }
    else if(bf->animations.current_frame != frame)
    {
        if(bf->animations.current_frame == 0)
        {
            bf->animations.last_animation = bf->animations.current_animation;
        }
        bf->animations.current_frame = frame;
    }

    bf->animations.frame_time += time;

    t = (bf->animations.frame_time) / bf->animations.period;
    dt = bf->animations.frame_time - (btScalar)t * bf->animations.period;
    bf->animations.frame_time = (btScalar)frame * bf->animations.period + dt;
    bf->animations.lerp = dt / bf->animations.period;
    Entity_GetNextFrame(bf, bf->animations.period, stc, &bf->animations.next_frame, &bf->animations.next_animation, 0x00);
    Entity_UpdateCurrentBoneFrame(bf, NULL);
}


/**
 * The base function, that draws one item by them id. Items may be animated.
 * This time for correct time calculation that function must be called every frame.
 * @param item_id - the base item id;
 * @param size - the item size on the screen;
 * @param str - item description - shows near / under item model;
 */
void Gui_RenderItem(struct ss_bone_frame_s *bf, btScalar size, const btScalar *matrix)
{
    if(size != 0.0)
    {
        btScalar bb[3];
        vec3_sub(bb, bf->bb_max, bf->bb_min);
        if(bb[0] >= bb[1])
        {
            size /= ((bb[0] >= bb[2])?(bb[0]):(bb[2]));
        }
        else
        {
            size /= ((bb[1] >= bb[2])?(bb[1]):(bb[2]));
        }
        size *= 0.8;

        btScalar scaledMatrix[16];
        Mat4_Copy(scaledMatrix, matrix);
        if(size < 1.0)          // only reduce items size...
        {
            Mat4_Scale(scaledMatrix, size, size, size);
        }
        Render_SkeletalModel(bf, scaledMatrix);
    }
    else
    {
        Render_SkeletalModel(bf, matrix);
    }
}

#if 0
void gui_InventoryMenu::UpdateMovements()
{
    mShiftBig = 0, mShiftSmall = 0;
    if(mMovementC == 0 && mMovementDirectionC == 0 && mMovementDirectionV == 0)
    {
        mMovementV = 0;
        mRowOffset = 1;
        mSelected = 0;
        mVisible = 0;
    }
    mMovementH -= engine_frame_time * 2.1 * mMovementDirectionH;
    if ((mMovementH < 0 && mMovementDirectionH == 1)||(mMovementH > 0 && mMovementDirectionH == -1))
    {
        mMovementH = 0;
        mMovementDirectionH = 0;
    }
    mMovementV += engine_frame_time * 2.3 * mMovementDirectionV;
    if(mMovementV < -2)
    {
        mMovementV = -2; mMovementDirectionV = 0;
    }
    if ((mMovementV < 0 && mMovementDirectionV == -1 && mRowOffset == 1)||(mMovementV > 0 && mMovementDirectionV == 1 && mRowOffset == 1))
    {
        mMovementV = 0; mMovementDirectionV = 0;
    }
    if(mMovementV > 2)
    {
        mMovementV = 2; mMovementDirectionV = 0;
    }
    mMovementC += engine_frame_time * 2.3 * mMovementDirectionC;
    if (mMovementC < 0 || mMovementC > 1)
    {
        if(mMovementC < 0)
            mMovementC = 0;
        else
            mMovementC = 1;
        if(mMovementDirectionV == 0)
            mMovementDirectionC = 0;
        else
            mMovementDirectionC = -mMovementDirectionC;
    }
    mShiftSmall = 200 * sin(mMovementC*3.14) ;
}


void gui_InventoryMenu::Render()
{
    if(FontManager == NULL)
    {
        return;
    }

    UpdateMovements();

    int items_count = 0, current_row;
    gui_invmenu_item_s *inv = NULL;

    if(mMovementV<=-1)
    {
        if (mMovementDirectionV==-1)
            mSelected = 0;
        mShiftBig = mMovementV + 2;
        items_count = mRow3Max;
        inv = mFirstInRow3;
        current_row = 2;
    }
    else if(mMovementV<1)
    {
        if (mMovementV<0 && mMovementDirectionV==1)
            mSelected = 0;
        else if (mMovementV>0 && mMovementDirectionV==-1)
            mSelected = 0;
        mShiftBig = mMovementV;
        items_count = mRow2Max;
        inv = mFirstInRow2;
        current_row = 1;
    }
    else if(mMovementV>=1)
    {
        if (mMovementDirectionV==1)
            mSelected = 0;
        mShiftBig = mMovementV - 2;
        items_count = mRow1Max;
        inv = mFirstInRow1;
        current_row = 0;
    }
    if ((items_count==0)||(inv==NULL))
        return;
    mAngle = -360/(GLfloat)items_count;

    int mov_dirC_sign = mMovementDirectionC;
    if(mov_dirC_sign == 0)
        mov_dirC_sign = 1;

    /*int anim = bf->current_animation;
        int frame = bf->current_frame;
        btScalar time = bf->frame_time;
        bf->current_animation = mAnim;
        bf->current_frame = mFrame;
        bf->frame_time = mTime;
        Item_Frame(bf, engine_frame_time);
        mAnim = bf->current_animation;
        mFrame = bf->current_frame;
        mTime = bf->frame_time;
        bf->current_animation = anim;
        bf->current_frame = frame;
        bf->frame_time = time;*/

    for(int i=0;inv;inv=inv->next,i++)
    {
        if(inv==NULL)
            break;
        base_item_p item = NULL;
        if(inv->linked_item)
            item = World_GetBaseItemByID(&engine_world, inv->linked_item->id);
        if(item == NULL)
        {
            continue;
        }
        Item_Frame(item->bf, 0.0);

        btScalar matrix[16];
        Mat4_E_macro(matrix);
        Mat4_Translate(matrix, 0.0, 50.0 - 800 * mShiftBig + mShiftSmall, -950.0);
        Mat4_RotateX(matrix, 10 + 80 * cos(1.57 * mMovementC));
        Mat4_RotateY(matrix, (i - mSelected + mMovementH) * mAngle - 90 + (180 * mMovementC * mov_dirC_sign));
        Mat4_Translate(matrix, -600 * mMovementC, 0.0, 0.0);
        Mat4_RotateX(matrix, -90.0);
        Mat4_RotateZ(matrix, 90.0);
        if(i == mSelected)
        {
            if(mMovementH==0 && (mMovementV==-2||mMovementV==0||mMovementV==2) && mMovementC==1)
                inv->angle -= engine_frame_time * 75.0;
            if(inv->angle < -180)
            {
                inv->angle_dir = 1;
                inv->angle += 360;
            }
            else if (inv->angle < 0)
            {
                inv->angle_dir = -1;
            }

            if(item->name[0])
            {
                if(inv->linked_item->id == 0 && (engine_world.version == 3 || engine_world.version == 4))
                {
                    //strcpy(mLabel_ItemName_text, "Statistics");
                    strncpy(mLabel_ItemName_text, item->name, 128); // <-- Not so easy. Gotta either implement a separate item
                                                                    // for each name or mess with the strings!
                }
                else
                {
                    if(inv->linked_item->count > 1)
                    {
                        snprintf(mLabel_ItemName_text, 128, "%s (%d)", item->name, inv->linked_item->count);
                    }
                    else
                    {
                        strncpy(mLabel_ItemName_text, item->name, 128);
                    }
                }
            }
        }
        else
        {
            if((inv->angle_dir==-1 && inv->angle>0)||(inv->angle_dir==1 && inv->angle<0))
            {
                inv->angle = 0;
                inv->angle_dir = 0;
            }
            if(inv->angle!=0 && inv->angle_dir!=0)
            {
                inv->angle -= inv->angle_dir * engine_frame_time * 75.0;
            }
        }
        Mat4_RotateZ(matrix, inv->angle);
        Mat4_Translate(matrix, -0.5 * item->bf->centre[0], -0.5 * item->bf->centre[1], -0.5 * item->bf->centre[2]);
        Mat4_Scale(matrix, 0.7, 0.7, 0.7);
        glLoadMatrixbt(matrix);
        Gui_RenderItem(item->bf, 0.0);
    }
}
#endif

/*
 * GUI RENDEDR CLASS
 */
gui_InventoryManager::gui_InventoryManager()
{
    mCurrentState               = INVENTORY_DISABLED;
    mNextState                  = INVENTORY_DISABLED;
    mCurrentItemsType           = GUI_MENU_ITEMTYPE_SYSTEM;
    mCurrentItemsCount          = 0;
    mItemsOffset                = 0;
    mNextItemsCount             = 0;

    mRingRotatePeriod           = 0.5;
    mRingTime                   = 0.0;
    mRingAngle                  = 0.0;
    mRingVerticalAngle          = 0.0;
    mRingAngleStep              = 0.0;
    mBaseRingRadius             = 600.0;
    mRingRadius                 = 600.0;
    mVerticalOffset             = 0.0;

    mItemRotatePeriod           = 4.0;
    mItemAngle                  = 0.0;

    mInventory                  = NULL;

    mLabel_Title.X              = 0.0;
    mLabel_Title.Y              = 30.0;
    mLabel_Title.Xanchor        = GUI_ANCHOR_HOR_CENTER;
    mLabel_Title.Yanchor        = GUI_ANCHOR_VERT_TOP;

    mLabel_Title.font_id        = FONT_PRIMARY;
    mLabel_Title.style_id       = FONTSTYLE_MENU_TITLE;
    mLabel_Title.text           = mLabel_Title_text;
    mLabel_Title_text[0]        = 0;
    mLabel_Title.show           = 0;

    mLabel_ItemName.X           = 0.0;
    mLabel_ItemName.Y           = 50.0;
    mLabel_ItemName.Xanchor     = GUI_ANCHOR_HOR_CENTER;
    mLabel_ItemName.Yanchor     = GUI_ANCHOR_VERT_BOTTOM;

    mLabel_ItemName.font_id     = FONT_PRIMARY;
    mLabel_ItemName.style_id    = FONTSTYLE_MENU_CONTENT;
    mLabel_ItemName.text        = mLabel_ItemName_text;
    mLabel_ItemName_text[0]     = 0;
    mLabel_ItemName.show        = 0;

    Gui_AddLine(&mLabel_ItemName);
    Gui_AddLine(&mLabel_Title);
}

gui_InventoryManager::~gui_InventoryManager()
{
    mCurrentState = INVENTORY_DISABLED;
    mNextState = INVENTORY_DISABLED;
    mInventory = NULL;

    mLabel_ItemName.show = 0;
    Gui_DeleteLine(&mLabel_ItemName);

    mLabel_Title.show = 0;
    Gui_DeleteLine(&mLabel_Title);
}

int gui_InventoryManager::getItemsTypeCount(int type)
{
    int ret = 0;
    for(inventory_node_p i=*mInventory;i!=NULL;i=i->next)
    {
        base_item_p bi = World_GetBaseItemByID(&engine_world, i->id);
        if((bi != NULL) && (bi->type == type))
        {
            ret++;
        }
    }
    return ret;
}

void gui_InventoryManager::restoreItemAngle(float time)
{
    if(mItemAngle > 0.0)
    {
        if(mItemAngle <= 180)
        {
            mItemAngle -= 180.0 * time / mRingRotatePeriod;
            if(mItemAngle < 0.0)
            {
                mItemAngle = 0.0;
            }
        }
        else
        {
            mItemAngle += 180.0 * time / mRingRotatePeriod;
            if(mItemAngle >= 360.0)
            {
                mItemAngle = 0.0;
            }
        }
    }
}

void gui_InventoryManager::setInventory(struct inventory_node_s **i)
{
    mInventory = i;
    mCurrentState = INVENTORY_DISABLED;
    mNextState = INVENTORY_DISABLED;
}

void gui_InventoryManager::setTitle(int items_type)
{
    int string_index;

    switch(items_type)
    {
        case GUI_MENU_ITEMTYPE_SYSTEM:
            string_index = STR_GEN_OPTIONS_TITLE;
            break;

        case GUI_MENU_ITEMTYPE_QUEST:
            string_index = STR_GEN_ITEMS;
            break;

        case GUI_MENU_ITEMTYPE_SUPPLY:
        default:
            string_index = STR_GEN_INVENTORY;
            break;
    }

    lua_GetString(engine_lua, string_index, GUI_LINE_DEFAULTSIZE, mLabel_Title_text);
}

int gui_InventoryManager::setItemsType(int type)
{
    if((mInventory == NULL) || (*mInventory == NULL))
    {
        mCurrentItemsType = type;
        return type;
    }

    int count = this->getItemsTypeCount(type);
    if(count == 0)
    {
        for(inventory_node_p i=*mInventory;i!=NULL;i=i->next)
        {
            base_item_p bi = World_GetBaseItemByID(&engine_world, i->id);
            if(bi != NULL)
            {
                type = bi->type;
                count = this->getItemsTypeCount(mCurrentItemsType);
                break;
            }
        }
    }

    if(count > 0)
    {
        mCurrentItemsCount = count;
        mCurrentItemsType = type;
        mRingAngleStep = 360.0 / mCurrentItemsCount;
        mItemsOffset %= count;
        mRingTime = 0.0;
        mRingAngle = 0.0;
        return type;
    }

    return -1;
}

void gui_InventoryManager::frame(float time)
{
    if((mInventory == NULL) || (*mInventory == NULL))
    {
        mCurrentState = INVENTORY_DISABLED;
        mNextState = INVENTORY_DISABLED;
        return;
    }

    switch(mCurrentState)
    {
        case INVENTORY_R_LEFT:
            mRingTime += time;
            mRingAngle = mRingAngleStep * mRingTime / mRingRotatePeriod;
            mNextState = INVENTORY_R_LEFT;
            if(mRingTime >= mRingRotatePeriod)
            {
                mRingTime = 0.0;
                mRingAngle = 0.0;
                mNextState = INVENTORY_IDLE;
                mCurrentState = INVENTORY_IDLE;
                mItemsOffset--;
                if(mItemsOffset < 0)
                {
                    mItemsOffset = mCurrentItemsCount - 1;
                }
            }
            restoreItemAngle(time);
            break;

        case INVENTORY_R_RIGHT:
            mRingTime += time;
            mRingAngle = -mRingAngleStep * mRingTime / mRingRotatePeriod;
            mNextState = INVENTORY_R_RIGHT;
            if(mRingTime >= mRingRotatePeriod)
            {
                mRingTime = 0.0;
                mRingAngle = 0.0;
                mNextState = INVENTORY_IDLE;
                mCurrentState = INVENTORY_IDLE;
                mItemsOffset++;
                if(mItemsOffset >= mCurrentItemsCount)
                {
                    mItemsOffset = 0;
                }
            }
            restoreItemAngle(time);
            break;

        case INVENTORY_IDLE:
            mRingTime = 0.0;
            switch(mNextState)
            {
                default:
                case INVENTORY_IDLE:
                    mItemTime += time;
                    mItemAngle = 360.0 * mItemTime / mItemRotatePeriod;
                    if(mItemTime >= mItemRotatePeriod)
                    {
                        mItemTime = 0.0;
                        mItemAngle = 0.0;
                    }
                    mLabel_ItemName.show = 1;
                    mLabel_Title.show = 1;
                    break;

                case INVENTORY_CLOSE:
                    Audio_Send(lua_GetGlobalSound(engine_lua, TR_AUDIO_SOUND_GLOBALID_MENUCLOSE));
                    mLabel_ItemName.show = 0;
                    mLabel_Title.show = 0;
                    mCurrentState = mNextState;
                    break;

                case INVENTORY_R_LEFT:
                case INVENTORY_R_RIGHT:
                    Audio_Send(TR_AUDIO_SOUND_MENUROTATE);
                    mLabel_ItemName.show = 0;
                    mCurrentState = mNextState;
                    mItemTime = 0.0;
                    break;

                case INVENTORY_UP:
                    mNextItemsCount = this->getItemsTypeCount(mCurrentItemsType + 1);
                    if(mNextItemsCount > 0)
                    {
                        //Audio_Send(lua_GetGlobalSound(engine_lua, TR_AUDIO_SOUND_GLOBALID_MENUCLOSE));
                        mCurrentState = mNextState;
                        mRingTime = 0.0;
                    }
                    else
                    {
                        mNextState = INVENTORY_IDLE;
                    }
                    mLabel_ItemName.show = 0;
                    mLabel_Title.show = 0;
                    break;

                case INVENTORY_DOWN:
                    mNextItemsCount = this->getItemsTypeCount(mCurrentItemsType - 1);
                    if(mNextItemsCount > 0)
                    {
                        //Audio_Send(lua_GetGlobalSound(engine_lua, TR_AUDIO_SOUND_GLOBALID_MENUCLOSE));
                        mCurrentState = mNextState;
                        mRingTime = 0.0;
                    }
                    else
                    {
                        mNextState = INVENTORY_IDLE;
                    }
                    mLabel_ItemName.show = 0;
                    mLabel_Title.show = 0;
                    break;
            };
            break;

        case INVENTORY_DISABLED:
            if(mNextState == INVENTORY_OPEN)
            {
                if(setItemsType(mCurrentItemsType) >= 0)
                {
                    Audio_Send(lua_GetGlobalSound(engine_lua, TR_AUDIO_SOUND_GLOBALID_MENUOPEN));
                    mCurrentState = INVENTORY_OPEN;
                    mRingAngle = 180.0;
                    mRingVerticalAngle = 180.0;
                }
            }
            break;

        case INVENTORY_UP:
            mCurrentState = INVENTORY_UP;
            mNextState = INVENTORY_UP;
            mRingTime += time;
            if(mRingTime < mRingRotatePeriod)
            {
                restoreItemAngle(time);
                mRingRadius = mBaseRingRadius * (mRingRotatePeriod - mRingTime) / mRingRotatePeriod;
                mVerticalOffset = - mBaseRingRadius * mRingTime / mRingRotatePeriod;
                mRingAngle += 180.0 * time / mRingRotatePeriod;
            }
            else if(mRingTime < 2.0 * mRingRotatePeriod)
            {
                if(mRingTime - time <= mRingRotatePeriod)
                {
                    //Audio_Send(lua_GetGlobalSound(engine_lua, TR_AUDIO_SOUND_GLOBALID_MENUOPEN));
                    mRingRadius = 0.0;
                    mVerticalOffset = mBaseRingRadius;
                    mRingAngleStep = 360.0 / mNextItemsCount;
                    mRingAngle = 180.0;
                    mCurrentItemsType++;
                    mCurrentItemsCount = mNextItemsCount;
                    mItemsOffset = 0;
                    setTitle(mCurrentItemsType);
                }
                mRingRadius = mBaseRingRadius * (mRingTime - mRingRotatePeriod) / mRingRotatePeriod;
                mVerticalOffset -= mBaseRingRadius * time / mRingRotatePeriod;
                mRingAngle -= 180.0 * time / mRingRotatePeriod;
            }
            else
            {
                mNextState = INVENTORY_IDLE;
                mCurrentState = INVENTORY_IDLE;
                mRingAngle = 0.0;
                mVerticalOffset = 0.0;
            }
            break;

        case INVENTORY_DOWN:
            mCurrentState = INVENTORY_DOWN;
            mNextState = INVENTORY_DOWN;
            mRingTime += time;
            if(mRingTime < mRingRotatePeriod)
            {
                restoreItemAngle(time);
                mRingRadius = mBaseRingRadius * (mRingRotatePeriod - mRingTime) / mRingRotatePeriod;
                mVerticalOffset = mBaseRingRadius * mRingTime / mRingRotatePeriod;
                mRingAngle += 180.0 * time / mRingRotatePeriod;
            }
            else if(mRingTime < 2.0 * mRingRotatePeriod)
            {
                if(mRingTime - time <= mRingRotatePeriod)
                {
                    //Audio_Send(lua_GetGlobalSound(engine_lua, TR_AUDIO_SOUND_GLOBALID_MENUOPEN));
                    mRingRadius = 0.0;
                    mVerticalOffset = -mBaseRingRadius;
                    mRingAngleStep = 360.0 / mNextItemsCount;
                    mRingAngle = 180.0;
                    mCurrentItemsType--;
                    mCurrentItemsCount = mNextItemsCount;
                    mItemsOffset = 0;
                    setTitle(mCurrentItemsType);
                }
                mRingRadius = mBaseRingRadius * (mRingTime - mRingRotatePeriod) / mRingRotatePeriod;
                mVerticalOffset += mBaseRingRadius * time / mRingRotatePeriod;
                mRingAngle -= 180.0 * time / mRingRotatePeriod;
            }
            else
            {
                mNextState = INVENTORY_IDLE;
                mCurrentState = INVENTORY_IDLE;
                mRingAngle = 0.0;
                mVerticalOffset = 0.0;
            }
            break;

        case INVENTORY_OPEN:
            mRingTime += time;
            mRingRadius = mBaseRingRadius * mRingTime / mRingRotatePeriod;
            mRingAngle -= 180.0 * time / mRingRotatePeriod;
            mRingVerticalAngle -= 180.0 * time / mRingRotatePeriod;
            if(mRingTime >= mRingRotatePeriod)
            {
                mCurrentState = INVENTORY_IDLE;
                mNextState = INVENTORY_IDLE;
                mRingVerticalAngle = 0;

                mRingRadius = mBaseRingRadius;
                mRingTime = 0.0;
                mRingAngle = 0.0;
                mVerticalOffset = 0.0;
                setTitle(GUI_MENU_ITEMTYPE_SUPPLY);
            }
            break;

        case INVENTORY_CLOSE:
            mRingTime += time;
            mRingRadius = mBaseRingRadius * (mRingRotatePeriod - mRingTime) / mRingRotatePeriod;
            mRingAngle += 180.0 * time / mRingRotatePeriod;
            mRingVerticalAngle += 180.0 * time / mRingRotatePeriod;
            if(mRingTime >= mRingRotatePeriod)
            {
                mCurrentState = INVENTORY_DISABLED;
                mNextState = INVENTORY_DISABLED;
                mRingVerticalAngle = 180.0;
                mRingTime = 0.0;
                mLabel_Title.show = 0;
                mRingRadius = mBaseRingRadius;
                mCurrentItemsType = 1;
            }
            break;
    }
}

void gui_InventoryManager::render()
{
    if((mCurrentState != INVENTORY_DISABLED) && (mInventory != NULL) && (*mInventory != NULL) && (FontManager != NULL))
    {
        int num = 0;
        for(inventory_node_p i=*mInventory;i!=NULL;i=i->next)
        {
            base_item_p bi = World_GetBaseItemByID(&engine_world, i->id);
            if((bi == NULL) || (bi->type != mCurrentItemsType))
            {
                continue;
            }

            btScalar matrix[16];
            Mat4_E_macro(matrix);
            Mat4_Translate(matrix, 0.0, 0.0, - mBaseRingRadius * 2.0);
            //Mat4_RotateX(matrix, 25.0);
            Mat4_RotateX(matrix, 25.0 + mRingVerticalAngle);
            btScalar ang = mRingAngleStep * (-mItemsOffset + num) + mRingAngle;
            Mat4_RotateY(matrix, ang);
            Mat4_Translate(matrix, 0.0, mVerticalOffset, mRingRadius);
            Mat4_RotateX(matrix, -90.0);
            Mat4_RotateZ(matrix, 90.0);
            if(num == mItemsOffset)
            {
                if(bi->name[0])
                {
                    strncpy(mLabel_ItemName_text, bi->name, GUI_LINE_DEFAULTSIZE);

                    if(i->count > 1)
                    {
                        char counter[32];
                        lua_GetString(engine_lua, STR_GEN_MASK_INVHEADER, 32, counter);
                        snprintf(mLabel_ItemName_text, GUI_LINE_DEFAULTSIZE, (const char*)counter, bi->name, i->count);

                    }
                }
                Mat4_RotateZ(matrix, 90.0 + mItemAngle - ang);
                Item_Frame(bi->bf, 0.0);                            // here will be time != 0 for using items animation
            }
            else
            {
                Mat4_RotateZ(matrix, 90.0 - ang);
                Item_Frame(bi->bf, 0.0);
            }
            Mat4_Translate(matrix, -0.5 * bi->bf->centre[0], -0.5 * bi->bf->centre[1], -0.5 * bi->bf->centre[2]);
            Mat4_Scale(matrix, 0.7, 0.7, 0.7);
            Gui_RenderItem(bi->bf, 0.0, matrix);

            num++;
        }
    }
}

/*
 * Other GUI options
 */
void Gui_SwitchGLMode(char is_gui)
{
    if(0 != is_gui)                                                             // set gui coordinate system
    {
        GLfloat M[16];
        const GLfloat far_dist = 4096.0f;
        const GLfloat near_dist = -1.0f;
        Mat4_E_macro(M);
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(M);
        glMatrixMode(GL_PROJECTION);
        M[0 * 4 + 0] = 2.0 / ((GLfloat)screen_info.w);
        M[1 * 4 + 1] = 2.0 / ((GLfloat)screen_info.h);
        M[2 * 4 + 2] =-2.0 / (far_dist - near_dist);
        M[3 * 4 + 0] =-1.0;
        M[3 * 4 + 1] =-1.0;
        M[3 * 4 + 2] =-(far_dist + near_dist) / (far_dist - near_dist);
        glLoadMatrixf(M);
        glMatrixMode(GL_MODELVIEW);
    }
    else                                                                        // set camera coordinate system
    {
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(engine_camera.gl_proj_mat);
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(engine_camera.gl_view_mat);
    }
}

void Gui_DrawCrosshair()
{
    GLfloat crosshair_buf[] = {
            (GLfloat) (screen_info.w/2.0f-5.f), ((GLfloat) screen_info.h/2.0f), 1.0f, 0.0f, 0.0f,
            (GLfloat) (screen_info.w/2.0f+5.f), ((GLfloat) screen_info.h/2.0f), 1.0f, 0.0f, 0.0f,
            (GLfloat) (screen_info.w/2.0f), ((GLfloat) screen_info.h/2.0f-5.f), 1.0f, 0.0f, 0.0f,
            (GLfloat) (screen_info.w/2.0f), ((GLfloat) screen_info.h/2.0f+5.f), 1.0f, 0.0f, 0.0f
    };

    glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT);
    glDisable(GL_DEPTH_TEST);
    glLineWidth(2.0);

    if(glBindBufferARB)glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    glVertexPointer(2, GL_FLOAT, 5 * sizeof(GLfloat), crosshair_buf);
    glColorPointer(3, GL_FLOAT, 5 * sizeof(GLfloat), crosshair_buf + 2);
    glDrawArrays(GL_LINES, 0, 4);

    glPopAttrib();
}

void Gui_DrawFaders()
{
    for(int i = 0; i < FADER_LASTINDEX; i++)
    {
        Fader[i].Show();
    }
}

void Gui_DrawBars()
{
    if(engine_world.Character && engine_world.Character->character)
    {
        if(engine_world.Character->character->weapon_current_state > WEAPON_STATE_HIDE_TO_READY)
            Bar[BAR_HEALTH].Forced = true;

        Bar[BAR_AIR].Show    (Character_GetParam(engine_world.Character, PARAM_AIR    ));
        Bar[BAR_STAMINA].Show(Character_GetParam(engine_world.Character, PARAM_STAMINA));
        Bar[BAR_HEALTH].Show (Character_GetParam(engine_world.Character, PARAM_HEALTH ));
        Bar[BAR_WARMTH].Show (Character_GetParam(engine_world.Character, PARAM_WARMTH ));
    }
}

void Gui_DrawInventory()
{
    //if (!main_inventory_menu->IsVisible())
    main_inventory_manager->frame(engine_frame_time);
    if(main_inventory_manager->getCurrentState() == gui_InventoryManager::INVENTORY_DISABLED)
    {
        return;
    }

    glClear(GL_DEPTH_BUFFER_BIT);

    glPopClientAttrib();
    glPushAttrib(GL_ENABLE_BIT | GL_PIXEL_MODE_BIT | GL_COLOR_BUFFER_BIT);
    glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);

    glBindTexture(GL_TEXTURE_2D, 0);
    glPolygonMode(GL_FRONT, GL_FILL);
    glFrontFace(GL_CCW);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_LIGHTING);
    glDisable(GL_ALPHA_TEST);
    glDepthMask(GL_FALSE);

    glPixelStorei(GL_UNPACK_LSB_FIRST, GL_FALSE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Background

    GLfloat upper_color[4] = {0.0,0.0,0.0,0.45};
    GLfloat lower_color[4] = {0.0,0.0,0.0,0.75};

    Gui_DrawRect(0.0, 0.0, (GLfloat)screen_info.w, (GLfloat)screen_info.h,
                 upper_color, upper_color, lower_color, lower_color,
                 BM_OPAQUE);

    glDepthMask(GL_TRUE);
    glPopClientAttrib();
    glPopAttrib();

    glEnable(GL_TEXTURE_2D);
    glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    //GLfloat color[4] = {0,0,0,0.45};
    //Gui_DrawRect(0,0,(GLfloat)screen_info.w,(GLfloat)screen_info.h, color, color, color, color, GL_SRC_ALPHA + GL_ONE_MINUS_SRC_ALPHA);

    Gui_SwitchGLMode(0);
    //main_inventory_menu->Render(); //engine_world.Character->character->inventory
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

    glPushAttrib(GL_ENABLE_BIT | GL_PIXEL_MODE_BIT | GL_COLOR_BUFFER_BIT);
    glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_LIGHTING);
    glDisable(GL_ALPHA_TEST);
    glDepthMask(GL_FALSE);

    glPixelStorei(GL_UNPACK_LSB_FIRST, GL_FALSE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glBindTexture(GL_TEXTURE_2D, 0);

    Fader[FADER_LOADSCREEN].Show();
    Bar[BAR_LOADING].Show(value);

    glDepthMask(GL_TRUE);
    glPopClientAttrib();
    glPopAttrib();

    Gui_SwitchGLMode(0);

    SDL_GL_SwapWindow(sdl_window);
}

/**
 * Draws simple colored rectangle with given parameters.
 */
void Gui_DrawRect(const GLfloat &x, const GLfloat &y,
                  const GLfloat &width, const GLfloat &height,
                  const float colorUpperLeft[], const float colorUpperRight[],
                  const float colorLowerLeft[], const float colorLowerRight[],
                  const int &blendMode,
                  const GLuint texture)
{
    switch(blendMode)
    {
        case BM_HIDE:
            return;
        case BM_MULTIPLY:
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            break;
        case BM_SIMPLE_SHADE:
            glBlendFunc(GL_ONE_MINUS_SRC_COLOR, GL_ONE_MINUS_SRC_ALPHA);
            break;
        case BM_SCREEN:
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            break;
        default:
        case BM_OPAQUE:
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            break;
    };

    glDisable(GL_DEPTH_TEST);

    if(glBindBufferARB)glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

    const GLfloat offset[2] = { x / screen_info.w - 1.f, y / screen_info.h - 1.f };
    const GLfloat factor[2] = { (width / screen_info.w) * 2.0f, (height / screen_info.h) * 2.0f };

    if(texture)
    {
        glUseProgramObjectARB(square_texture_program);
        glUniform1iARB(square_texture_program_sampler, 0);
        glActiveTextureARB(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform2fvARB(square_texture_program_offset, 1, offset);
        glUniform2fvARB(square_texture_program_factor, 1, factor);
    }
    else
    {
        glUseProgramObjectARB(square_program);
        glUniform2fvARB(square_program_offset, 1, offset);
        glUniform2fvARB(square_program_factor, 1, factor);
    }

    GLfloat rectCoords[8] = { 0, 0,
        1, 0,
        1, 1,
        0, 1 };
    glVertexPointer(2, GL_FLOAT, sizeof(GLfloat [2]), rectCoords);

    GLfloat rectColors[16];
    memcpy(rectColors + 0,  colorLowerLeft,  sizeof(GLfloat) * 4);
    memcpy(rectColors + 4,  colorLowerRight, sizeof(GLfloat) * 4);
    memcpy(rectColors + 8,  colorUpperLeft,  sizeof(GLfloat) * 4);
    memcpy(rectColors + 12, colorUpperRight, sizeof(GLfloat) * 4);
    glColorPointer(4, GL_FLOAT, sizeof(GLfloat [4]), rectColors);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    if(texture)
    {
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glUseProgramObjectARB(0);
}

bool Gui_FadeStart(int fader, int fade_direction)
{
    // If fader exists, and is not active, we engage it.

    if((fader < FADER_LASTINDEX) && (Fader[fader].IsFading() != GUI_FADER_STATUS_FADING))
    {
        Fader[fader].Engage(fade_direction);
        return true;
    }
    else
    {
        return false;
    }
}

bool Gui_FadeStop(int fader)
{
    if((fader < FADER_LASTINDEX) && (Fader[fader].IsFading() != GUI_FADER_STATUS_IDLE))
    {
        Fader[fader].Cut();
        return true;
    }
    else
    {
        return false;
    }
}

bool Gui_FadeAssignPic(int fader, const char* pic_name)
{
    if((fader >= 0) && (fader < FADER_LASTINDEX))
    {
        char buf[MAX_ENGINE_PATH];
        size_t len = strlen(pic_name);
        size_t ext_len = 0;

        ///@STICK: we can write incorrect image file extension, but engine will try all supported formats
        strncpy(buf, pic_name, MAX_ENGINE_PATH);
        if(!Engine_FileFound(buf, false))
        {
            for(;ext_len+1<len;ext_len++)
            {
                if(buf[len-ext_len-1] == '.')
                {
                    break;
                }
            }

            if(ext_len + 1 == len)
            {
                return false;
            }

            buf[len - ext_len + 0] = 'b';
            buf[len - ext_len + 1] = 'm';
            buf[len - ext_len + 2] = 'p';
            buf[len - ext_len + 3] = 0;
            if(!Engine_FileFound(buf, false))
            {
                buf[len - ext_len + 0] = 'j';
                buf[len - ext_len + 1] = 'p';
                buf[len - ext_len + 2] = 'g';
                if(!Engine_FileFound(buf, false))
                {
                    buf[len - ext_len + 0] = 'p';
                    buf[len - ext_len + 1] = 'n';
                    buf[len - ext_len + 2] = 'g';
                    if(!Engine_FileFound(buf, false))
                    {
                        buf[len - ext_len + 0] = 't';
                        buf[len - ext_len + 1] = 'g';
                        buf[len - ext_len + 2] = 'a';
                        if(!Engine_FileFound(buf, false))
                        {
                            return false;
                        }
                    }
                }
            }
        }

        return Fader[fader].SetTexture(buf);
    }

    return false;
}

void Gui_FadeSetup(int fader,
                   uint8_t alpha, uint8_t R, uint8_t G, uint8_t B, uint32_t blending_mode,
                   uint16_t fadein_speed, uint16_t fadeout_speed)
{
    if(fader >= FADER_LASTINDEX) return;

    Fader[fader].SetAlpha(alpha);
    Fader[fader].SetColor(R,G,B);
    Fader[fader].SetBlendingMode(blending_mode);
    Fader[fader].SetSpeed(fadein_speed,fadeout_speed);
}

int Gui_FadeCheck(int fader)
{
    if((fader >= 0) && (fader < FADER_LASTINDEX))
    {
        return Fader[fader].IsFading();
    }
    else
    {
        return false;
    }
}


// ===================================================================================
// ============================ FADER CLASS IMPLEMENTATION ===========================
// ===================================================================================

gui_Fader::gui_Fader()
{
    SetColor(0, 0, 0);
    SetBlendingMode(BM_OPAQUE);
    SetAlpha(255);
    SetSpeed(500);
    SetDelay(0);

    mActive             = false;
    mComplete           = true;  // All faders must be initialized as complete to receive proper start-up callbacks.
    mDirection          = GUI_FADER_DIR_IN;

    mTexture            = 0;
}

void gui_Fader::SetAlpha(uint8_t alpha)
{
    mMaxAlpha = (float)alpha / 255;
}

void gui_Fader::SetScaleMode(uint8_t mode)
{
    mTextureScaleMode = mode;
}

void gui_Fader::SetColor(uint8_t R, uint8_t G, uint8_t B, int corner)
{

    // Each corner of the fader could be colored independently, thus allowing
    // to create gradient faders. It is nifty yet not so useful feature, so
    // it is completely optional - if you won't specify corner, color will be
    // set for the whole fader.

    switch(corner)
    {
        case GUI_FADER_CORNER_TOPLEFT:
            mTopLeftColor[0] = (GLfloat)R / 255;
            mTopLeftColor[1] = (GLfloat)G / 255;
            mTopLeftColor[2] = (GLfloat)B / 255;
            break;

        case GUI_FADER_CORNER_TOPRIGHT:
            mTopRightColor[0] = (GLfloat)R / 255;
            mTopRightColor[1] = (GLfloat)G / 255;
            mTopRightColor[2] = (GLfloat)B / 255;
            break;

        case GUI_FADER_CORNER_BOTTOMLEFT:
            mBottomLeftColor[0] = (GLfloat)R / 255;
            mBottomLeftColor[1] = (GLfloat)G / 255;
            mBottomLeftColor[2] = (GLfloat)B / 255;
            break;

        case GUI_FADER_CORNER_BOTTOMRIGHT:
            mBottomRightColor[0] = (GLfloat)R / 255;
            mBottomRightColor[1] = (GLfloat)G / 255;
            mBottomRightColor[2] = (GLfloat)B / 255;
            break;

        default:
            mTopRightColor[0] = (GLfloat)R / 255;
            mTopRightColor[1] = (GLfloat)G / 255;
            mTopRightColor[2] = (GLfloat)B / 255;

            // Copy top right corner color to all other corners.

            memcpy(mTopLeftColor,     mTopRightColor, sizeof(GLfloat) * 4);
            memcpy(mBottomRightColor, mTopRightColor, sizeof(GLfloat) * 4);
            memcpy(mBottomLeftColor,  mTopRightColor, sizeof(GLfloat) * 4);
            break;
    }
}

void gui_Fader::SetBlendingMode(uint32_t mode)
{
    mBlendingMode = mode;
}

void gui_Fader::SetSpeed(uint16_t fade_speed, uint16_t fade_speed_secondary)
{
    mSpeed           = 1000.0 / (float)fade_speed;
    mSpeedSecondary  = 1000.0 / (float)fade_speed_secondary;
}

void gui_Fader::SetDelay(uint32_t delay_msec)
{
    mMaxTime         = (float)delay_msec / 1000.0;
}

void gui_Fader::SetAspect()
{
    if(mTexture)
    {
        if(((float)mTextureWidth / (float)screen_info.w) >= ((float)mTextureHeight / (float)screen_info.h))
        {
            mTextureWide = true;
            mTextureAspectRatio = (float)mTextureHeight / (float)mTextureWidth;
        }
        else
        {
            mTextureWide = false;
            mTextureAspectRatio = (float)mTextureWidth  / (float)mTextureHeight;
        }
    }
}

bool gui_Fader::SetTexture(const char *texture_path)
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
    if (status != kCGImageStatusComplete)
    {
        CFRelease(source);
        Con_Printf("Warning: image %s could not be loaded, status is %d", texture_path, status);
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
    CGContextRef context = CGBitmapContextCreate(data, width, height, 8, width*4, deviceRgb, kCGImageAlphaPremultipliedFirst);
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLuint) width, (GLuint) height, 0,
                 GL_BGRA, GL_UNSIGNED_INT_8_8_8_8, data);

    // Cleanup
    delete [] data;

    // Setup the additional required information
    mTextureWidth  = width;
    mTextureHeight = height;

    SetAspect();

    Con_Printf("Loaded fader picture: %s", texture_path);
    return true;
#else
    SDL_Surface *surface = IMG_Load(texture_path);
    GLenum       texture_format;
    GLint        color_depth;

    if(surface != NULL)
    {
        // Get the color depth of the SDL surface
        color_depth = surface->format->BytesPerPixel;

        if(color_depth == 4)        // Contains an alpha channel
        {
            if(surface->format->Rmask == 0x000000ff)
                texture_format = GL_RGBA;
            else
                texture_format = GL_BGRA;
        }
        else if(color_depth == 3)   // No alpha channel
        {
            if(surface->format->Rmask == 0x000000ff)
                texture_format = GL_RGB;
            else
                texture_format = GL_BGR;
        }
        else
        {
            Con_Warning(SYSWARN_NOT_TRUECOLOR_IMG, texture_path);
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
        Con_Warning(SYSWARN_IMG_NOT_LOADED_SDL, texture_path, SDL_GetError());
        return false;
    }

    // Unbind the texture - is it really necessary?
    // glBindTexture(GL_TEXTURE_2D, 0);

    // Free the SDL_Surface only if it was successfully created
    if(surface)
    {
        // Set additional parameters
        mTextureWidth  = surface->w;
        mTextureHeight = surface->h;

        SetAspect();

        Con_Notify(SYSNOTE_LOADED_FADER, texture_path);
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

bool gui_Fader::DropTexture()
{
    if(mTexture)
    {
        glBindTexture(GL_TEXTURE_2D, 0);
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

void gui_Fader::Engage(int fade_dir)
{
    mDirection    = fade_dir;
    mActive       = true;
    mComplete     = false;
    mCurrentTime  = 0.0;

    if(mDirection == GUI_FADER_DIR_IN)
    {
        mCurrentAlpha = mMaxAlpha;      // Fade in: set alpha to maximum.
    }
    else
    {
        mCurrentAlpha = 0.0;            // Fade out or timed: set alpha to zero.
    }
}

void gui_Fader::Cut()
{
    mActive        = false;
    mComplete      = false;
    mCurrentAlpha  = 0.0;
    mCurrentTime   = 0.0;

    DropTexture();
}

void gui_Fader::Show()
{
    if(!mActive)
    {
        mComplete = true;
        return;                                 // If fader is not active, don't render it.
    }

    if(mDirection == GUI_FADER_DIR_IN)          // Fade in case
    {
        if(mCurrentAlpha > 0.0)                 // If alpha is more than zero, continue to fade.
        {
            mCurrentAlpha -= engine_frame_time * mSpeed;
        }
        else
        {
            mComplete     = true;   // We've reached zero alpha, complete and disable fader.
            mActive       = false;
            mCurrentAlpha = 0.0;
            DropTexture();
        }
    }
    else if(mDirection == GUI_FADER_DIR_OUT)  // Fade out case
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
                mComplete     = true;          // We've reached zero alpha, complete and disable fader.
                mActive       = false;
                mCurrentAlpha = 0.0;
                mCurrentTime  = 0.0;
                DropTexture();
            }
        }
    }

    // Apply current alpha value to all vertices.

    mTopLeftColor[3]     = mCurrentAlpha;
    mTopRightColor[3]    = mCurrentAlpha;
    mBottomLeftColor[3]  = mCurrentAlpha;
    mBottomRightColor[3] = mCurrentAlpha;

    // Draw the rectangle.
    // We draw it from the very top left corner to the end of the screen.

    if(mTexture)
    {
        // Texture is always modulated with alpha!
        GLfloat tex_color[4] = {mCurrentAlpha, mCurrentAlpha, mCurrentAlpha, mCurrentAlpha};

        if(mTextureScaleMode == GUI_FADER_SCALE_LETTERBOX)
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
        else if(mTextureScaleMode == GUI_FADER_SCALE_ZOOM)
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

int gui_Fader::IsFading()
{
    if(mComplete)
    {
        return GUI_FADER_STATUS_COMPLETE;
    }
    else if(mActive)
    {
        return GUI_FADER_STATUS_FADING;
    }
    else
    {
        return GUI_FADER_STATUS_IDLE;
    }
}


// ===================================================================================
// ======================== PROGRESS BAR CLASS IMPLEMENTATION ========================
// ===================================================================================

gui_ProgressBar::gui_ProgressBar()
{
    // Set up some defaults.
    Visible   = false;
    Alternate = false;
    Invert    = false;
    Vertical  = false;
    Forced    = false;

    // Initialize parameters.
    // By default, bar is initialized with TR5-like health bar properties.
    SetPosition(GUI_ANCHOR_HOR_LEFT, 20, GUI_ANCHOR_VERT_TOP, 20);
    SetSize(250, 25, 3);
    SetColor(BASE_MAIN, 255, 50, 50, 150);
    SetColor(BASE_FADE, 100, 255, 50, 150);
    SetColor(ALT_MAIN, 255, 180, 0, 220);
    SetColor(ALT_FADE, 255, 255, 0, 220);
    SetColor(BACK_MAIN, 0, 0, 0, 160);
    SetColor(BACK_FADE, 60, 60, 60, 130);
    SetColor(BORDER_MAIN, 200, 200, 200, 50);
    SetColor(BORDER_FADE, 80, 80, 80, 100);
    SetValues(1000, 300);
    SetBlink(300);
    SetExtrude(true, 100);
    SetAutoshow(true, 5000, true, 1000);
}

// Resize bar.
// This function should be called every time resize event occurs.

void gui_ProgressBar::Resize()
{
    RecalculateSize();
    RecalculatePosition();
}

// Set specified color.
void gui_ProgressBar::SetColor(BarColorType colType,
                           uint8_t R, uint8_t G, uint8_t B, uint8_t A)
{
    float maxColValue = 255.0;

    switch(colType)
    {
        case BASE_MAIN:
            mBaseMainColor[0] = (float)R / maxColValue;
            mBaseMainColor[1] = (float)G / maxColValue;
            mBaseMainColor[2] = (float)B / maxColValue;
            mBaseMainColor[3] = (float)A / maxColValue;
            mBaseMainColor[4] = mBaseMainColor[3];
            return;
        case BASE_FADE:
            mBaseFadeColor[0] = (float)R / maxColValue;
            mBaseFadeColor[1] = (float)G / maxColValue;
            mBaseFadeColor[2] = (float)B / maxColValue;
            mBaseFadeColor[3] = (float)A / maxColValue;
            mBaseFadeColor[4] = mBaseFadeColor[3];
            return;
        case ALT_MAIN:
            mAltMainColor[0] = (float)R / maxColValue;
            mAltMainColor[1] = (float)G / maxColValue;
            mAltMainColor[2] = (float)B / maxColValue;
            mAltMainColor[3] = (float)A / maxColValue;
            mAltMainColor[4] = mAltMainColor[3];
            return;
        case ALT_FADE:
            mAltFadeColor[0] = (float)R / maxColValue;
            mAltFadeColor[1] = (float)G / maxColValue;
            mAltFadeColor[2] = (float)B / maxColValue;
            mAltFadeColor[3] = (float)A / maxColValue;
            mAltFadeColor[4] = mAltFadeColor[3];
            return;
        case BACK_MAIN:
            mBackMainColor[0] = (float)R / maxColValue;
            mBackMainColor[1] = (float)G / maxColValue;
            mBackMainColor[2] = (float)B / maxColValue;
            mBackMainColor[3] = (float)A / maxColValue;
            mBackMainColor[4] = mBackMainColor[3];
            return;
        case BACK_FADE:
            mBackFadeColor[0] = (float)R / maxColValue;
            mBackFadeColor[1] = (float)G / maxColValue;
            mBackFadeColor[2] = (float)B / maxColValue;
            mBackFadeColor[3] = (float)A / maxColValue;
            mBackFadeColor[4] = mBackFadeColor[3];
            return;
        case BORDER_MAIN:
            mBorderMainColor[0] = (float)R / maxColValue;
            mBorderMainColor[1] = (float)G / maxColValue;
            mBorderMainColor[2] = (float)B / maxColValue;
            mBorderMainColor[3] = (float)A / maxColValue;
            mBorderMainColor[4] = mBorderMainColor[3];
            return;
        case BORDER_FADE:
            mBorderFadeColor[0] = (float)R / maxColValue;
            mBorderFadeColor[1] = (float)G / maxColValue;
            mBorderFadeColor[2] = (float)B / maxColValue;
            mBorderFadeColor[3] = (float)A / maxColValue;
            mBorderFadeColor[4] = mBorderFadeColor[3];
            return;
        default:
            return;
    }
}

void gui_ProgressBar::SetPosition(int8_t anchor_X, float offset_X, int8_t anchor_Y, float offset_Y)
{
    mXanchor = anchor_X;
    mYanchor = anchor_Y;
    mAbsXoffset = offset_X;
    mAbsYoffset = offset_Y;

    RecalculatePosition();
}

// Set bar size
void gui_ProgressBar::SetSize(float width, float height, float borderSize)
{
    // Absolute values are needed to recalculate actual bar size according to resolution.
    mAbsWidth  = width;
    mAbsHeight = height;
    mAbsBorderSize = borderSize;

    RecalculateSize();
}

// Recalculate size, according to viewport resolution.
void gui_ProgressBar::RecalculateSize()
{
    mWidth  = (float)mAbsWidth  * screen_info.scale_factor;
    mHeight = (float)mAbsHeight * screen_info.scale_factor;

    mBorderWidth  = (float)mAbsBorderSize  * screen_info.scale_factor;
    mBorderHeight = (float)mAbsBorderSize  * screen_info.scale_factor;

    // Calculate range unit, according to maximum bar value set up.
    // If bar alignment is set to horizontal, calculate it from bar width.
    // If bar is vertical, then calculate it from height.

    mRangeUnit = (!Vertical)?( (mWidth) / mMaxValue ):( (mHeight) / mMaxValue );
}

// Recalculate position, according to viewport resolution.
void gui_ProgressBar::RecalculatePosition()
{
    switch(mXanchor)
    {
        case GUI_ANCHOR_HOR_LEFT:
            mX = (float)(mAbsXoffset+mAbsBorderSize) * screen_info.scale_factor;
            break;
        case GUI_ANCHOR_HOR_CENTER:
            mX = ((float)screen_info.w - ((float)(mAbsWidth+mAbsBorderSize*2) * screen_info.scale_factor)) / 2 +
                 ((float)mAbsXoffset * screen_info.scale_factor);
            break;
        case GUI_ANCHOR_HOR_RIGHT:
            mX = (float)screen_info.w - ((float)(mAbsXoffset+mAbsWidth+mAbsBorderSize*2)) * screen_info.scale_factor;
            break;
    }

    switch(mYanchor)
    {
        case GUI_ANCHOR_VERT_TOP:
            mY = (float)screen_info.h - ((float)(mAbsYoffset+mAbsHeight+mAbsBorderSize*2)) * screen_info.scale_factor;
            break;
        case GUI_ANCHOR_VERT_CENTER:
            mY = ((float)screen_info.h - ((float)(mAbsHeight+mAbsBorderSize*2) * screen_info.h_unit)) / 2 +
                 ((float)mAbsYoffset * screen_info.scale_factor);
            break;
        case GUI_ANCHOR_VERT_BOTTOM:
            mY = (mAbsYoffset + mAbsBorderSize) * screen_info.scale_factor;
            break;
    }
}

// Set maximum and warning state values.
void gui_ProgressBar::SetValues(float maxValue, float warnValue)
{
    mMaxValue  = maxValue;
    mWarnValue = warnValue;

    RecalculateSize();  // We need to recalculate size, because max. value is changed.
}

// Set warning state blinking interval.
void gui_ProgressBar::SetBlink(int interval)
{
    mBlinkInterval = (float)interval / 1000;
    mBlinkCnt      = (float)interval / 1000;  // Also reset blink counter.
}

// Set extrude overlay effect parameters.
void gui_ProgressBar::SetExtrude(bool enabled, uint8_t depth)
{
    mExtrude = enabled;
    memset(mExtrudeDepth, 0, sizeof(float) * 5);    // Set all colors to 0.
    mExtrudeDepth[3] = (float)depth / 255.0;        // We need only alpha transparency.
    mExtrudeDepth[4] = mExtrudeDepth[3];
}

// Set autoshow and fade parameters.
// Please note that fade parameters are actually independent of autoshow.
void gui_ProgressBar::SetAutoshow(bool enabled, int delay, bool fade, int fadeDelay)
{
    mAutoShow = enabled;

    mAutoShowDelay = (float)delay / 1000;
    mAutoShowCnt   = (float)delay / 1000;     // Also reset autoshow counter.

    mAutoShowFade = fade;
    mAutoShowFadeDelay = 1000 / (float)fadeDelay;
    mAutoShowFadeCnt = 0; // Initially, it's 0.
}

// Main bar show procedure.
// Draws a bar with a given value. Please note that it also accepts float,
// so effectively you can create bars for floating-point parameters.
void gui_ProgressBar::Show(float value)
{
    // Initial value limiters (to prevent bar overflow).
    value  = (value >= 0)?(value):(0);
    value  = (value > mMaxValue)?(mMaxValue):(value);

    // Enable blink mode, if value is gone below warning value.
    mBlink = (value <= mWarnValue)?(true):(false);

    if(mAutoShow)   // Check autoshow visibility conditions.
    {
        // 0. If bar drawing was forced, then show a bar without additional
        //    autoshow delay set. This condition has to be overwritten by
        //    any other conditions, that's why it is set first.
        if(Forced)
        {
            Visible = true;
            Forced  = false;
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
        mBaseMainColor[3]   = mBaseMainColor[4]   * mAutoShowFadeCnt;
        mBaseFadeColor[3]   = mBaseFadeColor[4]   * mAutoShowFadeCnt;
        mAltMainColor[3]    = mAltMainColor[4]    * mAutoShowFadeCnt;
        mAltFadeColor[3]    = mAltFadeColor[4]    * mAutoShowFadeCnt;
        mBackMainColor[3]   = mBackMainColor[4]   * mAutoShowFadeCnt;
        mBackFadeColor[3]   = mBackFadeColor[4]   * mAutoShowFadeCnt;
        mBorderMainColor[3] = mBorderMainColor[4] * mAutoShowFadeCnt;
        mBorderFadeColor[3] = mBorderFadeColor[4] * mAutoShowFadeCnt;
        mExtrudeDepth[3]    = mExtrudeDepth[4]    * mAutoShowFadeCnt;

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
                 BM_OPAQUE);

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
                       mBackMainColor, (Vertical)?(mBackFadeColor):(mBackMainColor),
                       (Vertical)?(mBackMainColor):(mBackFadeColor), mBackFadeColor,
                       BM_OPAQUE);
          return;
    }

    // Calculate base bar width, according to current value and range unit.
    mBaseSize  = mRangeUnit * value;
    mBaseRatio = value / mMaxValue;

    float RectAnchor;           // Anchor to stick base bar rect, according to Invert flag.
    float RectFirstColor[4];    // Used to recalculate gradient, according to current value.
    float RectSecondColor[4];

    // If invert decrease direction style flag is set, we position bar in a way
    // that it seems like it's decreasing to another side, and also swap main / fade colours.
    if(Invert)
    {
        memcpy(RectFirstColor,
               (Alternate)?(mAltMainColor):(mBaseMainColor),
               sizeof(float) * 4);

        // Main-fade gradient is recalculated according to current / maximum value ratio.
        for(int i = 0; i <= 3; i++)
            RectSecondColor[i] = (Alternate)?((mBaseRatio * mAltFadeColor[i])  + ((1 - mBaseRatio) * mAltMainColor[i]))
                                            :((mBaseRatio * mBaseFadeColor[i]) + ((1 - mBaseRatio) * mBaseMainColor[i]));

    }
    else
    {
        memcpy(RectSecondColor,
               (Alternate)?(mAltMainColor):(mBaseMainColor),
               sizeof(float) * 4);

        // Main-fade gradient is recalculated according to current / maximum value ratio.
        for(int i = 0; i <= 3; i++)
            RectFirstColor[i] = (Alternate)?((mBaseRatio * mAltFadeColor[i])  + ((1 - mBaseRatio) * mAltMainColor[i]))
                                           :((mBaseRatio * mBaseFadeColor[i]) + ((1 - mBaseRatio) * mBaseMainColor[i]));

    } // end if(Invert)

    // If vertical style flag is set, we draw bar base top-bottom, else we draw it left-right.
    if(Vertical)
    {
        RectAnchor = ( (Invert)?(mY + mHeight - mBaseSize):(mY) ) + mBorderHeight;

        // Draw actual bar base.
        Gui_DrawRect(mX + mBorderWidth, RectAnchor,
                     mWidth, mBaseSize,
                     RectFirstColor,  RectFirstColor,
                     RectSecondColor, RectSecondColor,
                     BM_OPAQUE);

        // Draw background rect.
        Gui_DrawRect(mX + mBorderWidth,
                     (Invert)?(mY + mBorderHeight):(RectAnchor + mBaseSize),
                     mWidth, mHeight - mBaseSize,
                     mBackMainColor, mBackFadeColor,
                     mBackMainColor, mBackFadeColor,
                     BM_OPAQUE);

        if(mExtrude)    // Draw extrude overlay, if flag is set.
        {
            float transparentColor[4] = {0};  // Used to set counter-shade to transparent.

            Gui_DrawRect(mX + mBorderWidth, RectAnchor,
                         mWidth / 2, mBaseSize,
                         mExtrudeDepth, transparentColor,
                         mExtrudeDepth, transparentColor,
                         BM_OPAQUE);
            Gui_DrawRect(mX + mBorderWidth + mWidth / 2, RectAnchor,
                         mWidth / 2, mBaseSize,
                         transparentColor, mExtrudeDepth,
                         transparentColor, mExtrudeDepth,
                         BM_OPAQUE);
        }
    }
    else
    {
        RectAnchor = ( (Invert)?(mX + mWidth - mBaseSize):(mX) ) + mBorderWidth;

        // Draw actual bar base.
        Gui_DrawRect(RectAnchor, mY + mBorderHeight,
                     mBaseSize, mHeight,
                     RectSecondColor, RectFirstColor,
                     RectSecondColor, RectFirstColor,
                     BM_OPAQUE);

        // Draw background rect.
        Gui_DrawRect((Invert)?(mX + mBorderWidth):(RectAnchor + mBaseSize),
                     mY + mBorderHeight,
                     mWidth - mBaseSize, mHeight,
                     mBackMainColor, mBackMainColor,
                     mBackFadeColor, mBackFadeColor,
                     BM_OPAQUE);

        if(mExtrude)    // Draw extrude overlay, if flag is set.
        {
            float transparentColor[4] = {0};  // Used to set counter-shade to transparent.

            Gui_DrawRect(RectAnchor, mY + mBorderHeight,
                         mBaseSize, mHeight / 2,
                         transparentColor, transparentColor,
                         mExtrudeDepth, mExtrudeDepth,
                         BM_OPAQUE);
            Gui_DrawRect(RectAnchor, mY + mBorderHeight + (mHeight / 2),
                         mBaseSize, mHeight / 2,
                         mExtrudeDepth, mExtrudeDepth,
                         transparentColor, transparentColor,
                         BM_OPAQUE);
        }
    } // end if(Vertical)
}

// ===================================================================================
// ======================== ITEM NOTIFIER CLASS IMPLEMENTATION =======================
// ===================================================================================

gui_ItemNotifier::gui_ItemNotifier()
{
    SetPos(850, 850);
    SetRot(0,0);
    SetSize(1.0);
    SetRotateTime(1000.0);

    mItem   = 0;
    mActive = false;
}

void gui_ItemNotifier::Start(int item, float time)
{
    Reset();

    mItem     = item;
    mShowTime = time;
    mActive   = true;
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

            mCurrRotX = (mCurrRotX > 360.0)?(mCurrRotX - 360.0):(mCurrRotX);
            //mCurrRotY = (mCurrRotY > 360.0)?(mCurrRotY - 360.0):(mCurrRotY);
        }

        float step = 0;

        if(mCurrTime == 0)
        {
            step = (mCurrPosX - mEndPosX) * (engine_frame_time * 4.0);
            step = (step <= 0.5)?(0.5):(step);

            mCurrPosX -= step;
            mCurrPosX  = (mCurrPosX < mEndPosX)?(mEndPosX):(mCurrPosX);

            if(mCurrPosX == mEndPosX)
                mCurrTime += engine_frame_time;
        }
        else if(mCurrTime < mShowTime)
        {
            mCurrTime += engine_frame_time;
        }
        else
        {
            step = (mCurrPosX - mEndPosX) * (engine_frame_time * 4.0);
            step = (step <= 0.5)?(0.5):(step);

            mCurrPosX += step;
            mCurrPosX  = (mCurrPosX > mStartPosX)?(mStartPosX):(mCurrPosX);

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

    mEndPosX = ((float)screen_info.w / GUI_SCREEN_METERING_RESOLUTION) * mAbsPosX;
    mPosY    = ((float)screen_info.h / GUI_SCREEN_METERING_RESOLUTION) * mAbsPosY;
    mCurrPosX = screen_info.w + ((float)screen_info.w / GUI_NOTIFIER_OFFSCREEN_DIVIDER * mSize);
    mStartPosX = mCurrPosX;    // Equalize current and start positions.
}

void gui_ItemNotifier::Draw()
{
    if(mActive)
    {
        base_item_p item = World_GetBaseItemByID(&engine_world, mItem);
        if(item)
        {
            int anim = item->bf->animations.current_animation;
            int frame = item->bf->animations.current_frame;
            btScalar time = item->bf->animations.frame_time;

            item->bf->animations.current_animation = 0;
            item->bf->animations.current_frame = 0;
            item->bf->animations.frame_time = 0.0;

            Item_Frame(item->bf, 0.0);
            btScalar matrix[16];
            Mat4_E_macro(matrix);
            Mat4_Translate(matrix, mCurrPosX, mPosY, -2048.0);
            Mat4_RotateY(matrix, mCurrRotX + mRotX);
            Mat4_RotateX(matrix, mCurrRotY + mRotY);
            Gui_RenderItem(item->bf, mSize, matrix);

            item->bf->animations.current_animation = anim;
            item->bf->animations.current_frame = frame;
            item->bf->animations.frame_time = time;
        }
    }
}

void gui_ItemNotifier::SetPos(float X, float Y)
{
    mAbsPosX = X;
    mAbsPosY = 1000.0 - Y;
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
    mRotateTime = (1000.0 / time) * 360.0;
}

// ===================================================================================
// ======================== FONT MANAGER  CLASS IMPLEMENTATION =======================
// ===================================================================================

gui_FontManager::gui_FontManager()
{
    this->font_library   = NULL;
    FT_Init_FreeType(&this->font_library);

    this->style_count     = 0;
    this->styles          = NULL;
    this->font_count      = 0;
    this->fonts           = NULL;

    this->mFadeValue      = 0.0;
    this->mFadeDirection  = true;
}

gui_FontManager::~gui_FontManager()
{
    this->font_count = 0;
    this->style_count = 0;

    for(gui_font_p next_font;this->fonts!=NULL;)
    {
        next_font=this->fonts->next;
        glf_free_font(this->fonts->gl_font);
        this->fonts->gl_font = NULL;
        free(this->fonts);
        this->fonts = next_font;
    }
    this->fonts = NULL;

    for(gui_fontstyle_p next_style;this->styles!=NULL;)
    {
        next_style=this->styles->next;
        free(this->styles);
        this->styles = next_style;
    }
    this->styles = NULL;

    FT_Done_FreeType(this->font_library);
    this->font_library = NULL;
}

gl_tex_font_p gui_FontManager::GetFont(const font_Type index)
{
    for(gui_font_p current_font=this->fonts;current_font!=NULL;current_font=current_font->next)
    {
        if(current_font->index == index)
        {
            return current_font->gl_font;
        }
    }

    return NULL;
}

gui_font_p gui_FontManager::GetFontAddress(const font_Type index)
{
    for(gui_font_p current_font=this->fonts;current_font!=NULL;current_font=current_font->next)
    {
        if(current_font->index == index)
        {
            return current_font;
        }
    }

    return NULL;
}

gui_fontstyle_p gui_FontManager::GetFontStyle(const font_Style index)
{
    for(gui_fontstyle_p current_style=this->styles;current_style!=NULL;current_style=current_style->next)
    {
        if(current_style->index == index)
        {
            return current_style;
        }
    }

    return NULL;
}

bool gui_FontManager::AddFont(const font_Type index, const uint32_t size, const char* path)
{
    if((size < GUI_MIN_FONT_SIZE) || (size > GUI_MAX_FONT_SIZE))
    {
        return false;
    }

    gui_font_s* desired_font = GetFontAddress(index);

    if(desired_font == NULL)
    {
        if(this->font_count >= GUI_MAX_FONTS)
        {
            return false;
        }

        this->font_count++;
        desired_font = (gui_font_p)malloc(sizeof(gui_font_t));
        desired_font->size = (uint16_t)size;
        desired_font->index = index;
        desired_font->next = this->fonts;
        this->fonts = desired_font;
    }
    else
    {
        glf_free_font(desired_font->gl_font);
    }

    desired_font->gl_font = glf_create_font(this->font_library, path, size);

    return true;
}

bool gui_FontManager::AddFontStyle(const font_Style index,
                                   const GLfloat R, const GLfloat G, const GLfloat B, const GLfloat A,
                                   const bool shadow, const bool fading,
                                   const bool rect, const GLfloat rect_border,
                                   const GLfloat rect_R, const GLfloat rect_G, const GLfloat rect_B, const GLfloat rect_A,
                                   const bool hide)
{
    gui_fontstyle_p desired_style = GetFontStyle(index);

    if(desired_style == NULL)
    {
        if(this->style_count >= GUI_MAX_FONTSTYLES)
        {
            return false;
        }

        this->style_count++;
        desired_style = (gui_fontstyle_p)malloc(sizeof(gui_fontstyle_t));
        desired_style->index = index;
        desired_style->next = this->styles;
        this->styles = desired_style;
    }

    desired_style->rect_border   = rect_border;
    desired_style->rect_color[0] = rect_R;
    desired_style->rect_color[1] = rect_G;
    desired_style->rect_color[2] = rect_B;
    desired_style->rect_color[3] = rect_A;

    desired_style->color[0]  = R;
    desired_style->color[1]  = G;
    desired_style->color[2]  = B;
    desired_style->color[3]  = A;

    memcpy(desired_style->real_color, desired_style->color, sizeof(GLfloat) * 4);

    desired_style->fading    = fading;
    desired_style->shadowed  = shadow;
    desired_style->rect      = rect;
    desired_style->hidden    = hide;

    return true;
}

bool gui_FontManager::RemoveFont(const font_Type index)
{
    gui_font_p previous_font, current_font;

    if(this->fonts == NULL)
    {
        return false;
    }

    if(this->fonts->index == index)
    {
        previous_font = this->fonts;
        this->fonts = this->fonts->next;
        glf_free_font(previous_font->gl_font);
        previous_font->gl_font = NULL;                                          ///@PARANOID
        free(previous_font);
        this->font_count--;
        return true;
    }

    previous_font = this->fonts;
    current_font = this->fonts->next;
    for(;current_font!=NULL;)
    {
        if(current_font->index == index)
        {
            previous_font->next = current_font->next;
            glf_free_font(current_font->gl_font);
            current_font->gl_font = NULL;                                       ///@PARANOID
            free(current_font);
            this->font_count--;
            return true;
        }

        previous_font = current_font;
        current_font = current_font->next;
    }

    return false;
}

bool gui_FontManager::RemoveFontStyle(const font_Style index)
{
    gui_fontstyle_p previous_style, current_style;

    if(this->styles == NULL)
    {
        return false;
    }

    if(this->styles->index == index)
    {
        previous_style = this->styles;
        this->styles = this->styles->next;
        free(previous_style);
        this->style_count--;
        return true;
    }

    previous_style = styles;
    current_style = styles->next;
    for(;current_style!=NULL;)
    {
        if(current_style->index == index)
        {
            previous_style->next = current_style->next;
            free(current_style);
            this->style_count--;
            return true;
        }

        previous_style = current_style;
        current_style = current_style->next;
    }

    return false;
}

void gui_FontManager::Update()
{
    if(this->mFadeDirection)
    {
        this->mFadeValue += engine_frame_time * GUI_FONT_FADE_SPEED;

        if(this->mFadeValue >= 1.0)
        {
            this->mFadeValue = 1.0;
            this->mFadeDirection = false;
        }
    }
    else
    {
        this->mFadeValue -= engine_frame_time * GUI_FONT_FADE_SPEED;

        if(this->mFadeValue <= GUI_FONT_FADE_MIN)
        {
            this->mFadeValue = GUI_FONT_FADE_MIN;
            this->mFadeDirection = true;
        }
    }

    for(gui_fontstyle_p current_style=this->styles;current_style!=NULL;current_style=current_style->next)
    {
        if(current_style->fading)
        {
            current_style->real_color[0] = current_style->color[0] * this->mFadeValue;
            current_style->real_color[1] = current_style->color[1] * this->mFadeValue;
            current_style->real_color[2] = current_style->color[2] * this->mFadeValue;
        }
        else
        {
            memcpy(current_style->real_color, current_style->color, sizeof(GLfloat) * 3);
        }
    }
}

void gui_FontManager::Resize()
{
    for(gui_font_p current_font=this->fonts;current_font!=NULL;current_font=current_font->next)
    {
        glf_resize(current_font->gl_font, (uint16_t)(((float)current_font->size) * screen_info.scale_factor));
    }
}
