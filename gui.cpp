
#include <stdint.h>
#include "gl_util.h"
#include "ftgl/FTGLBitmapFont.h"
#include "ftgl/FTGLTextureFont.h"

#include "gui.h"
#include "character_controller.h"
#include "engine.h"
#include "render.h"
#include "system.h"
#include "console.h"
#include "vmath.h"

#define MAX_TEMP_LINES (128)
#define TEMP_LINE_LENGHT (128)

gui_text_line_p         gui_base_lines = NULL;
gui_text_line_t         gui_temp_lines[MAX_TEMP_LINES];
uint16_t                temp_lines_used = 0;

ProgressBar      Bar[BAR_LASTINDEX];

void Gui_Init()
{
    int i;
    for(i=0;i<MAX_TEMP_LINES;i++)
    {
        gui_temp_lines[i].buf_size = TEMP_LINE_LENGHT;
        gui_temp_lines[i].text = (char*)malloc(TEMP_LINE_LENGHT * sizeof(char));
        gui_temp_lines[i].text[0] = 0;
        gui_temp_lines[i].show = 0;
        gui_temp_lines[i].show_rect = 0;
        gui_temp_lines[i].rect_border = 2.0;
        gui_temp_lines[i].next = NULL;
        gui_temp_lines[i].prev = NULL;

        gui_temp_lines[i].font_color[0] = 0.0;
        gui_temp_lines[i].font_color[1] = 0.0;
        gui_temp_lines[i].font_color[2] = 0.0;
        gui_temp_lines[i].font_color[3] = 1.0;

        gui_temp_lines[i].rect_color[0] = 0.0;
        gui_temp_lines[i].rect_color[1] = 1.0;
        gui_temp_lines[i].rect_color[2] = 0.0;
        gui_temp_lines[i].rect_color[3] = 0.5;
    }
#if 1
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

                    Bar[i].SetDimensions(50, 30, 250, 25, 3);
                    Bar[i].SetColor(BASE_MAIN, 255, 50, 50, 200);
                    Bar[i].SetColor(BASE_FADE, 100, 255, 50, 200);
                    Bar[i].SetColor(ALT_MAIN, 255, 180, 0, 255);
                    Bar[i].SetColor(ALT_FADE, 255, 255, 0, 255);
                    Bar[i].SetColor(BACK_MAIN, 0, 0, 0, 160);
                    Bar[i].SetColor(BACK_FADE, 60, 60, 60, 130);
                    Bar[i].SetColor(BORDER_MAIN, 200, 200, 200, 50);
                    Bar[i].SetColor(BORDER_FADE, 80, 80, 80, 100);
                    Bar[i].SetValues(CHARACTER_OPTION_HEALTH_MAX, CHARACTER_OPTION_HEALTH_MAX / 3);
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

                    Bar[i].SetDimensions(700, 30, 250, 25, 3);
                    Bar[i].SetColor(BASE_MAIN, 0, 50, 255, 200);
                    Bar[i].SetColor(BASE_FADE, 190, 190, 255, 200);
                    Bar[i].SetColor(BACK_MAIN, 0, 0, 0, 160);
                    Bar[i].SetColor(BACK_FADE, 60, 60, 60, 130);
                    Bar[i].SetColor(BORDER_MAIN, 200, 200, 200, 50);
                    Bar[i].SetColor(BORDER_FADE, 80, 80, 80, 100);
                    Bar[i].SetValues(CHARACTER_OPTION_AIR_MAX, (CHARACTER_OPTION_AIR_MAX / 3));
                    Bar[i].SetBlink(300);
                    Bar[i].SetExtrude(true, 100);
                    Bar[i].SetAutoshow(true, 2000, true, 400);
                }
                break;
            case BAR_SPRINT:
                {
                    Bar[i].Visible =      false;
                    Bar[i].Alternate =    false;
                    Bar[i].Invert =       false;
                    Bar[i].Vertical =     false;

                    Bar[i].SetDimensions(50, 70, 250, 25, 3);
                    Bar[i].SetColor(BASE_MAIN, 255, 100, 50, 200);
                    Bar[i].SetColor(BASE_FADE, 255, 200, 0, 200);
                    Bar[i].SetColor(BACK_MAIN, 0, 0, 0, 160);
                    Bar[i].SetColor(BACK_FADE, 60, 60, 60, 130);
                    Bar[i].SetColor(BORDER_MAIN, 110, 110, 110, 100);
                    Bar[i].SetColor(BORDER_FADE, 60, 60, 60, 180);
                    Bar[i].SetValues(120, 0);
                    Bar[i].SetExtrude(true, 100);
                    Bar[i].SetAutoshow(true, 500, true, 300);
                }
                break;
            case BAR_FREEZE:
                {
                    Bar[i].Visible =      false;
                    Bar[i].Alternate =    false;
                    Bar[i].Invert =       true;
                    Bar[i].Vertical =     false;

                    Bar[i].SetDimensions(700, 70, 250, 25, 3);
                    Bar[i].SetColor(BASE_MAIN, 255, 0, 255, 255);
                    Bar[i].SetColor(BASE_FADE, 190, 120, 255, 255);
                    Bar[i].SetColor(BACK_MAIN, 0, 0, 0, 160);
                    Bar[i].SetColor(BACK_FADE, 60, 60, 60, 130);
                    Bar[i].SetColor(BORDER_MAIN, 200, 200, 200, 50);
                    Bar[i].SetColor(BORDER_FADE, 80, 80, 80, 100);
                    Bar[i].SetValues(CHARACTER_OPTION_FREEZE_MAX, CHARACTER_OPTION_FREEZE_MAX / 3);
                    Bar[i].SetBlink(200);
                    Bar[i].SetExtrude(true, 60);
                    Bar[i].SetAutoshow(true, 500, true, 300);
                }
                break;
        } // end switch(i)
    } // end for(int i = 0; i < BAR_LASTINDEX; i++)
#endif
}

void Gui_Destroy()
{
    int i;
    for(i=0;i<MAX_TEMP_LINES;i++)
    {
        gui_temp_lines[i].show = 0;
        gui_temp_lines[i].buf_size = 0;
        free(gui_temp_lines[i].text);
        gui_temp_lines[i].text = NULL;
    }
    temp_lines_used = MAX_TEMP_LINES;
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
        gui_base_lines->prev = NULL;
        return;
    }

    line->prev->next = line->next;
    if(line->next)
    {
        line->next->prev = line->prev;
    }
}

gui_text_line_p Gui_StringAutoRect(gui_text_line_p l)
{
    if(l)
    {
        float llx, lly, llz, urx, ury, urz;
        if(con_base.smooth)
        {
            con_base.font_texture->BBox(l->text, llx, lly, llz, urx, ury, urz);
        }
        else
        {
            con_base.font_bitmap->BBox(l->text, llx, lly, llz, urx, ury, urz);
        }
        l->rect[0] = llx + l->x;
        l->rect[1] = lly + l->y;
        l->rect[2] = urx + l->x;
        l->rect[3] = ury + l->y;
        l->show_rect = 1;
    }
    
    return l;
}

/**
 * For simple temporary lines rendering.
 * Really all strings will be rendered in Gui_Render() function.
 */
gui_text_line_p Gui_OutTextXY(int x, int y, const char *fmt, ...)
{
    if(temp_lines_used < MAX_TEMP_LINES - 1)
    {
        va_list argptr;
        gui_text_line_p l = gui_temp_lines + temp_lines_used;

        va_start(argptr, fmt);
        vsnprintf(l->text, TEMP_LINE_LENGHT, fmt, argptr);
        va_end(argptr);

        l->show_rect = 0;
        l->next = NULL;
        l->prev = NULL;
        l->rect_border = 2.0;
        
        l->font_color[0] = 0.0;
        l->font_color[1] = 0.0;
        l->font_color[2] = 0.0;
        l->font_color[3] = 1.0;
        
        l->rect_color[0] = 0.0;
        l->rect_color[1] = 1.0;
        l->rect_color[2] = 0.0;
        l->rect_color[3] = 0.25;
        
        temp_lines_used ++;
        l->x = x;
        l->y = y;
        Gui_StringAutoRect(l);
        l->show = 1;
        return l;
    }
    
    return NULL;
}

void Gui_Render()
{
    Gui_SwitchConGLMode(1);

    glPushAttrib(GL_ENABLE_BIT | GL_PIXEL_MODE_BIT | GL_COLOR_BUFFER_BIT);
    glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_LIGHTING);
    glDisable(GL_ALPHA_TEST);
    glDepthMask(GL_FALSE);

    glPixelStorei(GL_UNPACK_LSB_FIRST, GL_FALSE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
   
    Con_Draw();
    glBindTexture(GL_TEXTURE_2D, 0);                                            // in other case +textured font we lost background in all rects and console
    Gui_DrawCrosshair();
    Gui_DrawBars();
    Gui_RenderStrings();

    glDepthMask(GL_TRUE);
    glPopClientAttrib();
    glPopAttrib();

    Gui_SwitchConGLMode(0);
}

void Gui_RenderStringLine(gui_text_line_p l)
{
    GLfloat x0, y0, x1, y1;
    GLfloat rectCoords[8];
    
    glBindTexture(GL_TEXTURE_2D, 0);
    if(l->show_rect)
    {
        x0 = ((l->rect[0] >= 0)?(l->rect[0]):(screen_info.w + l->rect[0])) - l->rect_border;
        y0 = ((l->rect[1] >= 0)?(l->rect[1]):(screen_info.h + l->rect[1])) - l->rect_border;
        x1 = ((l->rect[2] >= 0)?(l->rect[2]):(screen_info.w + l->rect[2])) + l->rect_border;
        y1 = ((l->rect[3] >= 0)?(l->rect[3]):(screen_info.h + l->rect[3])) + l->rect_border;
        rectCoords[0] = x0; rectCoords[1] = y0;
        rectCoords[2] = x1; rectCoords[3] = y0;
        rectCoords[4] = x1; rectCoords[5] = y1;
        rectCoords[6] = x0; rectCoords[7] = y1;
        glColor4fv(l->rect_color);
        glVertexPointer(2, GL_FLOAT, 0, rectCoords);
        glDrawArrays(GL_POLYGON, 0, 4);
    }
    
    if(l->show)
    {     
        glColor4fv(l->font_color);      
        if(con_base.smooth)
        {
            glPushMatrix();
            glTranslatef((GLfloat)((l->x >= 0)?(l->x):(screen_info.w + l->x)), (GLfloat)((l->y >= 0)?(l->y):(screen_info.h + l->y)), 0.0);
            con_base.font_texture->RenderRaw(l->text);
            glPopMatrix();
        }
        else
        {
            glRasterPos2i(((l->x >= 0)?(l->x):(screen_info.w + l->x)), ((l->y >= 0)?(l->y):(screen_info.h + l->y)));
            con_base.font_bitmap->RenderRaw(l->text);
        }
    }
}

void Gui_RenderStrings()
{
    gui_text_line_p l = gui_base_lines;
    
    while(l)
    {
        Gui_RenderStringLine(l);
        l = l->next;
    }
    
    uint16_t i;
    l = gui_temp_lines;
    for(i=0;i<temp_lines_used;i++,l++)
    {
        if(l->show)
        {
            Gui_RenderStringLine(l);
            l->show_rect = 0;
            l->show = 0;
        }
    }
    temp_lines_used = 0;
}

void Gui_SwitchConGLMode(char is_gui)
{
    static char curr_mode = 0;
    if((0 != is_gui) && (0 == curr_mode))                                       // set gui coordinate system
    {
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0.0, (GLdouble)screen_info.w, 0.0, (GLdouble)screen_info.h, -1.0, 1.0);
        glMatrixMode(GL_MODELVIEW);

        curr_mode = is_gui;
    }
    else if((0 == is_gui) && (0 != curr_mode))                                  // restore coordinate system
    {
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();

        curr_mode = is_gui;
    }
}

void Gui_DrawCrosshair()
{
    GLfloat crosshairCoords[] = {
            (GLfloat) (screen_info.w/2.0f-5.f), ((GLfloat) screen_info.h/2.0f),
            (GLfloat) (screen_info.w/2.0f+5.f), ((GLfloat) screen_info.h/2.0f),
            (GLfloat) (screen_info.w/2.0f), ((GLfloat) screen_info.h/2.0f-5.f),
            (GLfloat) (screen_info.w/2.0f), ((GLfloat) screen_info.h/2.0f+5.f)
    };
    
    glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT);
    glDisable(GL_DEPTH_TEST);
    glLineWidth(2.0);

    glColor3f(1.0, 0.0, 0.0);

    if(glBindBufferARB)glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    glVertexPointer(2, GL_FLOAT, 0, crosshairCoords);
    glDrawArrays(GL_LINES, 0, 4);

    glPopAttrib();
}

void Gui_DrawBars()
{
    if(engine_world.Character && engine_world.Character->character)
    {
        Bar[BAR_AIR].Show(engine_world.Character->character->opt.air);
        Bar[BAR_SPRINT].Show(engine_world.Character->character->opt.sprint);
        Bar[BAR_HEALTH].Show(engine_world.Character->character->opt.health);
    }
}

/**
 * Draws simple colored rectangle with given parameters.
 */
void Gui_DrawRect(const GLfloat &x, const GLfloat &y,
                  const GLfloat &width, const GLfloat &height,
                  const float colorUpperLeft[], const float colorUpperRight[],
                  const float colorLowerLeft[], const float colorLowerRight[],
                  const int &blendMode)
{
    switch(blendMode)
    {
        case BM_HIDE:
            return;
        case BM_MULTIPLY:
            glBlendFunc(GL_ONE, GL_ONE);
            break;
        case BM_INVERT_SRC:
            glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
            break;
        case BM_INVERT_DEST:
            glBlendFunc(GL_ONE_MINUS_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);
            break;
        case BM_SCREEN:
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
            break;
        default:
        case BM_OPAQUE:
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            break;
    };

    glDisable(GL_DEPTH_TEST);

    glBegin(GL_POLYGON);

        glColor4fv(colorUpperLeft);     // Upper left
        glVertex2f(x, y + height);

        glColor4fv(colorUpperRight);    // Upper right
        glVertex2f(x + width, y + height);

        glColor4fv(colorLowerRight);    // Lower right
        glVertex2f(x + width, y);

        glColor4fv(colorLowerLeft);     // Lower left
        glVertex2f(x, y);

    glEnd();
}


// ===================================================================================
// ======================== PROGRESS BAR CLASS IMPLEMENTATION ========================
// ===================================================================================

ProgressBar::ProgressBar()
{
    // Set up some defaults.
    Visible =      false;
    Alternate =    false;
    Invert =       false;
    Vertical =     false;

    // Initialize parameters.
    // By default, bar is initialized with TR5-like health bar properties.
    SetDimensions(20, 20, 250, 25, 3);
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

// Update bar resolution.
// This function is also used to compare if resolution is changed.
bool ProgressBar::UpdateResolution(int scrWidth, int scrHeight)
{
    if( (scrWidth != mLastScrWidth) || (scrHeight != mLastScrHeight) )
    {
        mLastScrWidth  = scrWidth;
        mLastScrHeight = scrHeight;
        return true;
    }
    else
    {
        return false;
    }
}

// Set specified color.
void ProgressBar::SetColor(BarColorType colType,
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

// Set bar dimensions (coordinates and size)
void ProgressBar::SetDimensions(float X, float Y,
                                float width, float height, float borderSize)
{
    // Absolute values are needed to recalculate actual bar size according to resolution.
    mAbsX = X;
    mAbsY = Y;
    mAbsWidth  = width;
    mAbsHeight = height;
    mAbsBorderSize = borderSize;

    // If resolution has changed (or bar is being initialized), recalculate dimensions.
    if(UpdateResolution(screen_info.w, screen_info.h))
    {
        RecalculateSize();
        RecalculatePosition();
    }
}

// Recalculate size, according to viewport resolution.
void ProgressBar::RecalculateSize()
{
    mWidth  = mLastScrWidth  * ( (float)mAbsWidth / 1000 );
    mHeight = mLastScrHeight * ( (float)mAbsHeight / 1000 );
    mBorderWidth  = mLastScrWidth  * ( (float)mAbsBorderSize / 1000 );
    mBorderHeight = mLastScrHeight * ( (float)mAbsBorderSize / 1000 ) *
                    ((float)mLastScrWidth / (float)mLastScrHeight);

    // Calculate range unit, according to maximum bar value set up.
    // If bar alignment is set to horizontal, calculate it from bar width.
    // If bar is vertical, then calculate it from height.
    mRangeUnit = (!Vertical)?( (mWidth) / mMaxValue ):( (mHeight) / mMaxValue );
}

// Recalculate position, according to viewport resolution.
void ProgressBar::RecalculatePosition()
{
    mX = (mLastScrWidth  * ( (float)mAbsX / 1000 ) );
    mY = mLastScrHeight - mLastScrHeight * ( (mAbsY + mAbsHeight + (mAbsBorderSize * 2 * ((float)mLastScrWidth / (float)mLastScrHeight))) / 1000 );
}

// Set maximum and warning state values.
void ProgressBar::SetValues(float maxValue, float warnValue)
{
    mMaxValue  = maxValue;
    mWarnValue = warnValue;

    RecalculateSize();  // We need to recalculate size, because max. value is changed.
}

// Set warning state blinking interval.
void ProgressBar::SetBlink(int interval)
{
    mBlinkInterval = (float)interval / 1000;
    mBlinkCnt      = (float)interval / 1000;  // Also reset blink counter.
}

// Set extrude overlay effect parameters.
void ProgressBar::SetExtrude(bool enabled, uint8_t depth)
{
    mExtrude = enabled;
    memset(mExtrudeDepth, 0, sizeof(float) * 5);    // Set all colors to 0.
    mExtrudeDepth[3] = (float)depth / 255.0;        // We need only alpha transparency.
    mExtrudeDepth[4] = mExtrudeDepth[3];
}

// Set autoshow and fade parameters.
// Please note that fade parameters are actually independent of autoshow.
void ProgressBar::SetAutoshow(bool enabled, int delay, bool fade, int fadeDelay)
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
void ProgressBar::Show(float value)
{
    // Initial value limiters (to prevent bar overflow).
    value  = (value >= 0)?(value):(0);
    value  = (value > mMaxValue)?(mMaxValue):(value);

    // Enable blink mode, if value is gone below warning value.
    mBlink = (value <= mWarnValue)?(true):(false);

    if(mAutoShow)   // Check autoshow visibility conditions.
    {
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


    // If user changed screen resolution, automatically recalculate bar size:
    if( UpdateResolution(screen_info.w, screen_info.h) )
    {
        RecalculatePosition();
        RecalculateSize();
    }


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
// ===================== END OF PROGRESS BAR CLASS IMPLEMENTATION ====================
// ===================================================================================

