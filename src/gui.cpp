
#include <stdint.h>
#include <SDL2/SDL_image.h>

#include "gl_util.h"
#include "ftgl/FTGLTextureFont.h"

#include "gui.h"
#include "character_controller.h"
#include "engine.h"
#include "render.h"
#include "system.h"
#include "console.h"
#include "vmath.h"
#include "camera.h"

#define MAX_TEMP_LINES   (256)
#define TEMP_LINE_LENGTH (128)

extern SDL_Window  *sdl_window;

gui_text_line_p     gui_base_lines = NULL;
gui_text_line_t     gui_temp_lines[MAX_TEMP_LINES];
uint16_t            temp_lines_used = 0;

gui_ItemNotifier    Notifier;
gui_ProgressBar     Bar[BAR_LASTINDEX];
gui_Fader           Fader[FADER_LASTINDEX];

gui_FontManager    *FontManager = NULL;
gui_InventoryMenu  *main_inventory_menu = NULL;

void Gui_Init()
{
    Gui_InitBars();
    Gui_InitFaders();
    Gui_InitNotifier();
    Gui_InitTempLines();
    
    main_inventory_menu = new gui_InventoryMenu();
}

void Gui_InitFontManager()
{
    FontManager = new gui_FontManager();
}

void Gui_InitTempLines()
{
    for(int i=0;i<MAX_TEMP_LINES;i++)
    {
        gui_temp_lines[i].text_size = TEMP_LINE_LENGTH;
        gui_temp_lines[i].text = (char*)malloc(TEMP_LINE_LENGTH * sizeof(char));
        gui_temp_lines[i].text[0] = 0;
        gui_temp_lines[i].show = 0;
        
        gui_temp_lines[i].next = NULL;
        gui_temp_lines[i].prev = NULL;
        
        gui_temp_lines[i].style = NULL;
        gui_temp_lines[i].font  = NULL;
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
    for(int i = 0; i < MAX_TEMP_LINES ;i++)
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

    temp_lines_used = MAX_TEMP_LINES;

    if(main_inventory_menu)
    {
        delete main_inventory_menu;
        main_inventory_menu = NULL;
    }
    
    if(FontManager)
    {
        delete FontManager;
        FontManager = NULL;
    }
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
        
        l->font->BBox(l->text, llx, lly, llz, urx, ury, urz);
        l->rect[0] = llx;
        l->rect[1] = lly;
        l->rect[2] = urx;
        l->rect[3] = ury;
    }

    return l;
}

/**
 * For simple temporary lines rendering.
 * Really all strings will be rendered in Gui_Render() function.
 */
gui_text_line_p Gui_OutTextXY(GLfloat x, GLfloat y, const char *fmt, ...)
{
    if(temp_lines_used < MAX_TEMP_LINES - 1)
    {
        va_list argptr;
        gui_text_line_p l = gui_temp_lines + temp_lines_used;
        
        if(l->font == NULL)
            l->font = FontManager->GetFont(FONT_SECONDARY);
            
        if(l->style == NULL)
            l->style = FontManager->GetFontStyle(FONTSTYLE_GENERIC);

        va_start(argptr, fmt);
        vsnprintf(l->text, TEMP_LINE_LENGTH, fmt, argptr);
        va_end(argptr);

        l->next = NULL;
        l->prev = NULL;

        temp_lines_used++;
        
        l->x = x;
        l->y = y;
        
        l->align  = GUI_LINE_ALIGN_LEFT;
        l->real_x = x * screen_info.w_unit;
        l->real_y = screen_info.h - y * screen_info.h_unit;
        
        l->show = 1;
        return l;
    }

    return NULL;
}

void Gui_Update()
{
    FontManager->Update();
}

void Gui_Resize()
{
    gui_text_line_p l = gui_base_lines;

    while(l)
    {
        l->real_x = l->x * screen_info.w_unit;
        l->real_y = screen_info.h - l->y * screen_info.h_unit;
        
        l = l->next;
    }
    
    for(int i = 0; i < BAR_LASTINDEX; i++)
    {
        Bar[i].Resize();
    }
    
    FontManager->Resize();
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
    GLfloat real_x = l->real_x;   // Used with center and right alignments.
    
    if(l->font == NULL)
        l->font = FontManager->GetFont(FONT_SECONDARY);
        
    if(l->style == NULL)
        l->style = FontManager->GetFontStyle(FONTSTYLE_GENERIC);
        
    if((!l->show) || (l->style->hidden)) return;
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    if((l->style->rect) || (l->align != GUI_LINE_ALIGN_LEFT))
    {
        Gui_StringAutoRect(l);
        
        if(l->align == GUI_LINE_ALIGN_RIGHT)
        {
            real_x -= l->rect[2];
        }
        else if(l->align == GUI_LINE_ALIGN_CENTER)
        {
            real_x -= l->rect[2] / 2.0;
        }
    }
    
    if(l->style->rect)
    {
        GLfloat x0 = l->rect[0] + real_x    - l->style->rect_border * screen_info.w_unit;
        GLfloat y0 = l->rect[1] + l->real_y - l->style->rect_border * screen_info.h_unit;
        GLfloat x1 = l->rect[2] + real_x    + l->style->rect_border * screen_info.w_unit;
        GLfloat y1 = l->rect[3] + l->real_y + l->style->rect_border * screen_info.h_unit;
        
        GLfloat rectCoords[8];
        rectCoords[0] = x0; rectCoords[1] = y0;
        rectCoords[2] = x1; rectCoords[3] = y0;
        rectCoords[4] = x1; rectCoords[5] = y1;
        rectCoords[6] = x0; rectCoords[7] = y1;
        glColor4fv(l->style->rect_color);
        glVertexPointer(2, GL_FLOAT, 0, rectCoords);
        glDrawArrays(GL_POLYGON, 0, 4);
    }
    
    if(l->style->shadowed)
    {
        GLfloat temp[4] = {0.0,0.0,0.0,l->style->color[3] * GUI_FONT_SHADOW_TRANSPARENCY}; // Derive alpha from base color.
        
        glColor4fv(temp);
        glPushMatrix();
        glTranslatef((real_x+GUI_FONT_SHADOW_HORIZONTAL_SHIFT), (l->real_y+GUI_FONT_SHADOW_VERTICAL_SHIFT), 0.0);
        l->font->Render(l->text);
        glPopMatrix();
    }

    glColor4fv(l->style->real_color);
    glPushMatrix();
    glTranslatef(real_x, l->real_y, 0.0);
    l->font->Render(l->text);
    glPopMatrix();
}

void Gui_RenderStrings()
{
    gui_text_line_p l = gui_base_lines;

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
    temp_lines_used = 0;
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

    bf->lerp = 0.0;
    stc = Anim_FindStateChangeByID(bf->model->animations + bf->current_animation, bf->next_state);
    Entity_GetNextFrame(bf, time, stc, &frame, &anim, 0x00);
    if(anim != bf->current_animation)
    {
        bf->last_animation = bf->current_animation;
        /*frame %= bf->model->animations[anim].frames_count;
        frame = (frame >= 0)?(frame):(bf->model->animations[anim].frames_count - 1 + frame);

        bf->last_state = bf->model->animations[anim].state_id;
        bf->next_state = bf->model->animations[anim].state_id;
        bf->current_animation = anim;
        bf->current_frame = frame;
        bf->next_animation = anim;
        bf->next_frame = frame;*/
        stc = Anim_FindStateChangeByID(bf->model->animations + bf->current_animation, bf->next_state);
    }
    else if(bf->current_frame != frame)
    {
        if(bf->current_frame == 0)
        {
            bf->last_animation = bf->current_animation;
        }
        bf->current_frame = frame;
    }

    bf->frame_time += time;

    t = (bf->frame_time) / bf->period;
    dt = bf->frame_time - (btScalar)t * bf->period;
    bf->frame_time = (btScalar)frame * bf->period + dt;
    bf->lerp = dt / bf->period;
    Entity_GetNextFrame(bf, bf->period, stc, &bf->next_frame, &bf->next_animation, 0x00);
    Entity_UpdateCurrentBoneFrame(bf, NULL);
}


/**
 * The base function, that draws one item by them id. Items may be animated.
 * This time for correct time calculation that function must be called every frame.
 * @param item_id - the base item id;
 * @param size - the item size on the screen;
 * @param str - item description - shows near / under item model;
 */
void Gui_RenderItem(struct ss_bone_frame_s *bf, btScalar size)
{
    if(size!=NULL)
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

        glPushMatrix();
        if(size < 1.0)          // only reduce items size...
            glScalef(size, size, size);
        Render_SkeletalModel(bf);
        glPopMatrix();
    }
    else
        Render_SkeletalModel(bf);
}


gui_InventoryMenu::gui_InventoryMenu()
{
    mFont_Primary   = FontManager->GetFont(FONT_PRIMARY);
    mFont_Secondary = FontManager->GetFont(FONT_SECONDARY);
    
    mStyle_Title        = FontManager->GetFontStyle(FONTSTYLE_MENU_TITLE);
    mStyle_Heading1     = FontManager->GetFontStyle(FONTSTYLE_MENU_HEADING1);
    mStyle_Heading2     = FontManager->GetFontStyle(FONTSTYLE_MENU_HEADING2);
    mStyle_ItemActive   = FontManager->GetFontStyle(FONTSTYLE_MENU_ITEM_ACTIVE);
    mStyle_ItemInactive = FontManager->GetFontStyle(FONTSTYLE_MENU_ITEM_INACTIVE);
    mStyle_MenuContent  = FontManager->GetFontStyle(FONTSTYLE_MENU_CONTENT);
    
    mVisible = 0;

    mRowOffset = 1;
    mRow1Max = 0;
    mRow2Max = 1;
    mRow3Max = 1;
    mSelected = 0;
    mMaxItems = 0;

    mFirstInRow1 = NULL;
    mFirstInRow2 = new gui_invmenu_item_s;
    mFirstInRow3 = new gui_invmenu_item_s;

    mFirstInRow2->linked_item = NULL;
    mFirstInRow2->next = NULL;
    mFirstInRow2->ammo = NULL;
    mFirstInRow2->combinables = NULL;
    mFirstInRow2->description = NULL;
    mFirstInRow2->angle = 0;
    mFirstInRow2->angle_dir = 0;

    mFirstInRow3->linked_item = NULL;
    mFirstInRow3->next = NULL;
    mFirstInRow3->ammo = NULL;
    mFirstInRow3->combinables = NULL;
    mFirstInRow3->description = NULL;
    mFirstInRow3->angle = 0;
    mFirstInRow3->angle_dir = 0;

    mFrame = 0;
    mAnim = 0;
    mTime = 0.0;
    mMovementH = 0.0;
    mMovementV = 0.0;
    mMovementC = 0.0;
    mMovementDirectionH = 0;
    mMovementDirectionV = 0;
    mMovementDirectionC = 0;

    mFontSize = 18;
    mFontHeight = 12;
}


gui_InventoryMenu::~gui_InventoryMenu()
{
    DestroyItems();
}


void gui_InventoryMenu::Toggle()
{
    if(mMovementDirectionC!=0) return;
    
    if(mVisible && mMovementDirectionV == 0 && mMovementH == 0)
    {
        Audio_Send(lua_GetGlobalSound(engine_lua, TR_AUDIO_SOUND_GLOBALID_MENUCLOSE));
        mMovementDirectionC = -1;
    }
    else
    {
        Audio_Send(lua_GetGlobalSound(engine_lua, TR_AUDIO_SOUND_GLOBALID_MENUOPEN));
        mMovementDirectionC = 1;
    }
    mVisible = 1;
}

void gui_InventoryMenu::UpdateItemsOrder(int row)
{
    gui_invmenu_item_s *item = NULL, *last_item = NULL, *last_last_item = NULL, *temp = NULL;
    bool redo = 0; int items_count;

    switch(row)
    {
    case 0:
        items_count = mRow1Max;
        item = mFirstInRow1;
        break;
    case 1:
        items_count = mRow2Max;
        item = mFirstInRow2;
        break;
    case 2:
        items_count = mRow3Max;
        item = mFirstInRow3;
        break;
    }

    if (items_count<=1)
        return;

    while(item)
    {
        if(last_item && last_item->linked_item->id > item->linked_item->id)
            {
                if(item->next)
                    temp = item->next;
                item->next = last_item;
                if(temp)
                    last_item->next = temp;
                else
                    last_item->next = NULL;
                if(last_last_item)
                {
                    last_last_item->next = item;
                    redo = 1;
                }
            }
        if(item->next)
        {
            if(last_item)
                last_last_item = last_item;
            if(item->linked_item)
                last_item = item;
            item = item->next;
        }
        else
            break;
    }
    if(redo)
        UpdateItemsOrder(row);
}


void gui_InventoryMenu::MoveSelectHorisontal(int dx)
{
    Audio_Send(TR_AUDIO_SOUND_MENUROTATE);
    mSelected += dx;
    mMovementH = (float)dx;
    mMovementDirectionH = dx;

    int current_row_max = mRow2Max;

    switch(mRowOffset)
    {
    case 0:
        current_row_max = mRow1Max;
        break;
    case 1:
        current_row_max = mRow2Max;
        break;
    case 2:
        current_row_max = mRow3Max;
        break;
    }

    if (mSelected < 0)
        mSelected = current_row_max-1;
    if (mSelected >= current_row_max)
        mSelected = 0;
}


void gui_InventoryMenu::MoveSelectVertical(int dy)
{
    mRowOffset += dy;
    if (mRowOffset < 0)
        mRowOffset = 0;
    if (mRowOffset > 2)
        mRowOffset = 2;
    if(mRowOffset == 0 && mRow1Max == 0)
    {
        mRowOffset = 1;
        return;
    }

    mMovementDirectionC = -1;
    mMovementDirectionV = -dy;
}


void gui_InventoryMenu::AddItem(inventory_node_p item)
{
    int *items_count;
    gui_invmenu_item_s **inv;
    int correct_row = 1;

    switch(item->id)
    {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
        correct_row = 2;    // options ring
        break;
    case 0:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 20:
    case 21:
    case 22:
    case 30:
    case 31:
    case 32:
    case 33:
    case 34:
    case 35:
    case 36:
    case 37:
    case 38:
    case 39:
    case 40:
    case 41:
    case 42:
    case 43:
    case 45:
    case 46:
    case 47:
    case 50:
    case 51:
        correct_row = 1;    // weapons ring
        break;
    default:
        correct_row = 0;    // quest items ring
        break;
    }

    switch(correct_row)
    {
    case 0:
        items_count = &mRow1Max;
        inv = &mFirstInRow1;
        break;
    case 1:
        items_count = &mRow2Max;
        inv = &mFirstInRow2;
        break;
    case 2:
        items_count = &mRow3Max;
        inv = &mFirstInRow3;
        break;
    }

    gui_invmenu_item_s *cur = *inv, *last = NULL;
    while(cur)
    {
        if(item->id == 0 || item->id == 1)
        {
            cur->linked_item = item;
            return;
        }
        if(cur->linked_item)
            if(item->id == cur->linked_item->id)
                return;
        last = cur;
        cur = cur->next;
    }
    cur = new gui_invmenu_item_s;
    cur->linked_item = item;
    cur->next = NULL;
    cur->ammo = NULL;
    cur->combinables = NULL;
    cur->description = NULL;
    cur->angle = 0;
    cur->angle_dir = 0;
    // other stuff... //TBI

    if(*inv==NULL)
        *inv = cur;
    else
        last->next = cur;
    *items_count = *items_count + 1;

    items_count = NULL;
    inv = NULL;

    if(correct_row == 1)
        UpdateItemsOrder(1);
}


void gui_InventoryMenu::UpdateItemRemoval(inventory_node_p item)
{
    if(item == NULL)
        return;

    int *items_count;
    gui_invmenu_item_s **inv;
    int correct_row = 1;

    switch(item->id)
    {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
        correct_row = 2;    // options ring
        break;
    case 0:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 20:
    case 21:
    case 22:
    case 30:
    case 31:
    case 32:
    case 33:
    case 34:
    case 35:
    case 36:
    case 37:
    case 38:
    case 39:
    case 40:
    case 41:
    case 42:
    case 43:
    case 45:
    case 46:
    case 47:
    case 50:
    case 51:
        correct_row = 1;    // weapons ring
        break;
    default:
        correct_row = 0;    // quest items ring
        break;
    }

    switch(correct_row)
    {
    case 0:
        items_count = &mRow1Max;
        inv = &mFirstInRow1;
        break;
    case 1:
        items_count = &mRow2Max;
        inv = &mFirstInRow2;
        break;
    case 2:
        items_count = &mRow3Max;
        inv = &mFirstInRow3;
        break;
    }

    gui_invmenu_item_s *cur = *inv, *last = NULL, *next = NULL;
    while(cur)
    {
        next = cur->next;
        if(cur->linked_item && cur->linked_item->id == item->id)
        {
            if(cur->linked_item->count<2)
            {
                if(last)
                {
                    last->next = next;
                }
                else
                {
                    if(next)
                        *inv = next;
                    else
                        *inv = NULL;
                }
                delete cur;
                cur = NULL;
                *items_count = *items_count - 1;
            }
            break;
        }
        last = cur;
        cur = next;
    }

    items_count = NULL;
    inv = NULL;

    if(correct_row == 1)
        UpdateItemsOrder(1);
}


void gui_InventoryMenu::RemoveAllItems()
{
    int *items_count; bool redo;
    gui_invmenu_item_s *inv;
    gui_invmenu_item_s *cur = NULL, *last = NULL;

    for(int i=0; i<3; i++)
    {
        redo = 1;
        switch(i)
        {
        case 0:
            items_count = &mRow1Max;
            inv = mFirstInRow1;
            break;
        case 1:
            items_count = &mRow2Max;
            inv = mFirstInRow2;
            break;
        case 2:
            items_count = &mRow3Max;
            inv = mFirstInRow3;
            break;
        }

        cur = inv;
        while(redo)
        {
            while(cur &&  redo)
            {
                if(!cur->next)
                    if(last)
                    {
                        redo = 1;
                        last->next = NULL;
                        last = NULL;
                        delete cur;
                        cur = inv;
                        *items_count = *items_count - 1;
                    }
                    else
                    {
                        if(i == 0)
                        {
                            last = NULL;
                            delete cur;
                            cur = NULL;
                            mFirstInRow1 = NULL;
                            *items_count = *items_count - 1;
                        }
                        redo = 0;
                    }
                else
                {
                    last = cur;
                    cur = cur->next;
                }
            }
            if(cur == inv)
                break;
        }

        items_count = NULL;
        inv = NULL;
    }
}


void gui_InventoryMenu::DestroyItems()
{
    RemoveAllItems();
    delete mFirstInRow2;
    delete mFirstInRow3;
}


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
    UpdateMovements();

    int items_count, current_row;
    gui_invmenu_item_s *inv;

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

        glLoadIdentity();
        glPushMatrix();
            glTranslatef(0.0, 50.0 - 800 * mShiftBig + mShiftSmall, -950.0);
            glRotatef(10 + 80 * cos(1.57 * mMovementC), 1.0, 0.0, 0.0);
            glPushMatrix();
                glRotatef((i - mSelected + mMovementH) * mAngle - 90 + (180 * mMovementC * mov_dirC_sign), 0.0, 1.0, 0.0);
                glPushMatrix();
                    glTranslatef(-600 * mMovementC, 0.0, 0.0);
                    glRotatef(-90.0, 1.0, 0.0, 0.0);
                    glRotatef(90.0, 0.0, 0.0, 1.0);
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
                            inv->angle_dir = -1;
                        if(item->name[0])
                        {
                            if(inv->linked_item->id == 0 && (engine_world.version == 3 || engine_world.version == 4))
                                Gui_OutTextXY(screen_info.w/2 - 160, screen_info.h/2 - 200, "Statistics");
                            else
                                Gui_OutTextXY(screen_info.w/2 - 160, screen_info.h/2 - 200, "%s", item->name);
                        }
                        if(inv->linked_item->count > 1)
                                Gui_OutTextXY(screen_info.w/2 + 150, screen_info.h/2 - 200, "%d", inv->linked_item->count);
                    }
                    else
                    {
                        if((inv->angle_dir==-1 && inv->angle>0)||(inv->angle_dir==1 && inv->angle<0))
                        {
                            inv->angle = 0;
                            inv->angle_dir = 0;
                        }
                        if(inv->angle!=0 && inv->angle_dir!=0)
                            inv->angle -= inv->angle_dir * engine_frame_time * 75.0;
                    }
                    glRotatef(inv->angle, 0.0, 0.0, 1.0);
                    glTranslatef(-0.5 * item->bf->centre[0], -0.5 * item->bf->centre[1], -0.5 * item->bf->centre[2]);
                    glScalef(0.7, 0.7, 0.7);
                    Gui_RenderItem(item->bf, NULL);
                glPopMatrix();
            glPopMatrix();
        glPopMatrix();
    }

    /*glBindTexture(GL_TEXTURE_2D, 0);
    glColor3f(1.0, 0.0, 0.0);
    glBegin(GL_LINE_LOOP);
        glVertex2f(x0            , y0            );
        glVertex2f(x0 + mCellSize, y0            );
        glVertex2f(x0 + mCellSize, y0 - mCellSize);
        glVertex2f(x0            , y0 - mCellSize);
    glEnd();

    glColor3f(0.0, 0.0, 1.0);
    glBegin(GL_LINE_LOOP);
        glVertex2f(mLeft         , screen_info.h - mTop          );
        glVertex2f(mLeft + mWidth, screen_info.h - mTop          );
        glVertex2f(mLeft + mWidth, screen_info.h - mTop - mHeight);
        glVertex2f(mLeft         , screen_info.h - mTop - mHeight);
    glEnd();*/
}


void Gui_SwitchGLMode(char is_gui)
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
        glOrtho(0.0, (GLdouble)screen_info.w, 0.0, (GLdouble)screen_info.h, -1.0, 4096.0);
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
        Bar[BAR_AIR].Show    (Character_GetParam(engine_world.Character, PARAM_AIR    ));
        Bar[BAR_STAMINA].Show(Character_GetParam(engine_world.Character, PARAM_STAMINA));
        Bar[BAR_HEALTH].Show (Character_GetParam(engine_world.Character, PARAM_HEALTH ));
        Bar[BAR_WARMTH].Show (Character_GetParam(engine_world.Character, PARAM_WARMTH ));
    }
}

void Gui_DrawInventory()
{
    if (!main_inventory_menu->IsVisible())
        return;

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
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    //GLfloat color[4] = {0,0,0,0.45};
    //Gui_DrawRect(0,0,(GLfloat)screen_info.w,(GLfloat)screen_info.h, color, color, color, color, GL_SRC_ALPHA + GL_ONE_MINUS_SRC_ALPHA);

    Gui_SwitchGLMode(0);
    main_inventory_menu->Render(); //engine_world.Character->character->inventory
    Gui_SwitchGLMode(1);
}

void Gui_StartNotifier(int item)
{
    Notifier.Start(item, GUI_NOTIFIER_SHOWTIME);
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
    glDisable(GL_CULL_FACE);

    GLfloat texCoords[8];
    if(texture)
    {
        glBindTexture(GL_TEXTURE_2D, texture);
        texCoords[0] = 0; texCoords[1] = 1;
        texCoords[2] = 1; texCoords[3] = 1;
        texCoords[4] = 1; texCoords[5] = 0;
        texCoords[6] = 0; texCoords[7] = 0;
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(2, GL_FLOAT, 0, texCoords);
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    GLfloat rectCoords[8];
    rectCoords[0] = x; rectCoords[1] = y;
    rectCoords[2] = x + width; rectCoords[3] = y;
    rectCoords[4] = x + width; rectCoords[5] = y + height;
    rectCoords[6] = x; rectCoords[7] = y + height;
    glVertexPointer(2, GL_FLOAT, 0, rectCoords);
    
    GLfloat rectColors[16];
    memcpy(rectColors + 0,  colorLowerLeft,  sizeof(GLfloat) * 4);
    memcpy(rectColors + 4,  colorLowerRight, sizeof(GLfloat) * 4);
    memcpy(rectColors + 8,  colorUpperRight, sizeof(GLfloat) * 4);
    memcpy(rectColors + 12, colorUpperLeft,  sizeof(GLfloat) * 4);
    glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer(4, GL_FLOAT, 0, rectColors);
    
    glDrawArrays(GL_POLYGON, 0, 4);
    
    glDisableClientState(GL_COLOR_ARRAY);
    
    if(texture)
    {
        glBindTexture(GL_TEXTURE_2D, 0);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }
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

bool Gui_FadeAssignPic(int fader, const char* pic_name)
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

    if(fader < FADER_LASTINDEX)
    {
        return Fader[fader].SetTexture(buf);
    }
    else
    {
        return false;
    }
}

int Gui_FadeCheck(int fader)
{
    if(fader < FADER_LASTINDEX)
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
            Con_Printf("Warning: image %s is not truecolor - not supported!", texture_path);
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
        Con_Printf("SDL could not load %s: %s", texture_path, SDL_GetError());
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

        Con_Printf("Loaded fader picture: %s", texture_path);
        SDL_FreeSurface(surface);
        return true;
    }
    else
    {
        glDeleteTextures(1, &mTexture);
        mTexture = 0;
        return false;
    }
}

bool gui_Fader::DropTexture()
{
    if(mTexture)
    {
        glBindTexture(GL_TEXTURE_2D, 0);
        glDeleteTextures(1, &mTexture);
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

    if(mDirection == GUI_FADER_DIR_IN)           // Fade in case
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
    Visible =      false;
    Alternate =    false;
    Invert =       false;
    Vertical =     false;

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
    mWidth  = (float)mAbsWidth  * screen_info.w_unit;
    mHeight = (float)mAbsHeight * screen_info.h_unit * ((float)screen_info.w / screen_info.h);

    mBorderWidth  = (float)mAbsBorderSize  * screen_info.w_unit;
    mBorderHeight = (float)mAbsBorderSize  * screen_info.h_unit * ((float)screen_info.w / screen_info.h);

    // Calculate range unit, according to maximum bar value set up.
    // If bar alignment is set to horizontal, calculate it from bar width.
    // If bar is vertical, then calculate it from height.
    
    mRangeUnit = (!Vertical)?( (mWidth) / mMaxValue ):( (mHeight) / mMaxValue );
}

// Recalculate position, according to viewport resolution.
void gui_ProgressBar::RecalculatePosition()
{
    float offset_ratio = (screen_info.w >= screen_info.h)?(screen_info.w_unit):(screen_info.h_unit);
    
    switch(mXanchor)
    {
        case GUI_ANCHOR_HOR_LEFT:
            mX = (float)(mAbsXoffset+mAbsBorderSize) * offset_ratio;
            break;
        case GUI_ANCHOR_HOR_CENTER:
            mX = ((float)screen_info.w - ((float)(mAbsWidth+mAbsBorderSize*2) * screen_info.w_unit)) / 2 +
                 ((float)mAbsXoffset * offset_ratio);
            break;
        case GUI_ANCHOR_HOR_RIGHT:
            mX = (float)screen_info.w - ((float)(mAbsXoffset+mAbsWidth+mAbsBorderSize*2)) * offset_ratio;
            break;
    }
    
    switch(mYanchor)
    {
        case GUI_ANCHOR_VERT_TOP:
            mY = (float)screen_info.h - ((float)(mAbsYoffset+mAbsHeight+mAbsBorderSize*2)) * offset_ratio;
            break;
        case GUI_ANCHOR_VERT_CENTER:
            mY = ((float)screen_info.h - ((float)(mAbsHeight+mAbsBorderSize*2) * screen_info.h_unit)) / 2 +
                 ((float)mAbsYoffset * offset_ratio);
            break;
        case GUI_ANCHOR_VERT_BOTTOM:
            mY = (mAbsYoffset + mAbsBorderSize) * offset_ratio;
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
            int anim = item->bf->current_animation;
            int frame = item->bf->current_frame;
            btScalar time = item->bf->frame_time;

            item->bf->current_animation = 0;
            item->bf->current_frame = 0;
            item->bf->frame_time = 0.0;

            Item_Frame(item->bf, 0.0);
            glPushMatrix();
                glTranslatef(mCurrPosX, mPosY, -2048.0);
                glRotatef(mCurrRotX + mRotX, 0.0, 1.0, 0.0);
                glRotatef(mCurrRotY + mRotY, 1.0, 0.0, 0.0);
                Gui_RenderItem(item->bf, mSize);
            glPopMatrix();

            item->bf->current_animation = anim;
            item->bf->current_frame = frame;
            item->bf->frame_time = time;
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
    style_count     = 0;
    styles          = NULL;
    font_count      = 0;
    fonts           = NULL;
    
    mFadeValue      = 0.0;
    mFadeDirection  = true;
}

gui_FontManager::~gui_FontManager()
{
    for(uint32_t i = 0; i < font_count; i++)
    {
        delete fonts[i].font;
    }
    
    free(fonts);  fonts  = NULL; font_count  = 0;
    free(styles); styles = NULL; style_count = 0;
}

FTGLTextureFont* gui_FontManager::GetFont(const font_Type index)
{
    gui_font_p current_font = fonts;
    
    if(font_count == 0) return NULL;
    
    for(uint32_t i = 0; i < font_count; i++, current_font++)
    {
        if(current_font->index == index)
            return current_font->font;
    }
    
    return NULL;
}

gui_font_s* gui_FontManager::GetFontAddress(const font_Type index)
{
    gui_font_s* current_font = fonts;
    
    if(font_count == 0) return NULL;
    
    for(uint32_t i = 0; i < font_count; i++, current_font++)
    {
        if(current_font->index == index)
            return current_font;
    }
    
    return NULL;
}

gui_fontstyle_s* gui_FontManager::GetFontStyle(const font_Style index)
{
    gui_fontstyle_s* current_style = styles;
    
    if(style_count == 0) return NULL;
    
    for(uint32_t i = 0; i < style_count; i++, current_style++)
    {
        if(current_style->index == index)
            return current_style;
    }
    
    return NULL;
}

bool gui_FontManager::AddFont(const font_Type index, const uint32_t size, const char* path)
{
    if((size < GUI_MIN_FONT_SIZE) || (size > GUI_MAX_FONT_SIZE)) return false;
    
    gui_font_s* desired_font = GetFontAddress(index);
    
    if(desired_font == NULL)
    {
        if(font_count == GUI_MAX_FONTS) return false;
        
        font_count++;
        fonts = (gui_font_p)realloc(fonts, font_count * sizeof(gui_font_t));
        desired_font = fonts+(font_count-1);
        desired_font->size = (uint16_t)size;
        desired_font->index = index;
    }
    else
    {
        delete desired_font->font;
    }
    
    desired_font->font = NULL;
    desired_font->font = new FTGLTextureFont(path);
    desired_font->font->FaceSize(size);
    desired_font->font->CharMap(FT_ENCODING_UNICODE);
    
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
        if(style_count == GUI_MAX_FONTSTYLES) return false;
        
        style_count++;
        styles = (gui_fontstyle_p)realloc(styles, style_count * sizeof(gui_fontstyle_t));
        desired_style = styles+(style_count-1);
        desired_style->index = index;
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
    if(font_count == 0) return false;
    
    gui_font_p current_font = fonts;
    
    for(uint32_t i=0; i<font_count; i++, current_font++)
    {
        if(current_font->index == index)
        {
            font_count--;
            if(font_count != 0)
            {
                if(font_count != i)
                    memcpy(current_font, current_font+1, font_count-i);
                fonts = (gui_font_p)realloc(fonts, font_count * sizeof(gui_font_t));
            }
            else
            {
                free(fonts); fonts = NULL;
            }
            return true;
        }
    }
    
    return false;
}

bool gui_FontManager::RemoveFontStyle(const font_Style index)
{
    if(style_count == 0) return false;
    
    gui_fontstyle_p current_style = styles;
    
    for(uint32_t i=0; i<style_count; i++, current_style++)
    {
        if(current_style->index == index)
        {
            style_count--;
            if(style_count != 0)
            {
                if(style_count != i)
                    memcpy(current_style, current_style+1, style_count-i);
                styles = (gui_fontstyle_p)realloc(styles, style_count * sizeof(gui_fontstyle_t));
            }
            else
            {
                free(styles); styles = NULL;
            }
            return true;
        }
    }
    
    return false;
}

void gui_FontManager::Update()
{
    if(mFadeDirection)
    {
        mFadeValue += engine_frame_time * GUI_FONT_FADE_SPEED;
        
        if(mFadeValue >= 1.0)
        {
            mFadeValue = 1.0;
            mFadeDirection = false;
        }
    }
    else
    {
        mFadeValue -= engine_frame_time * GUI_FONT_FADE_SPEED;
        
        if(mFadeValue <= GUI_FONT_FADE_MIN)
        {
            mFadeValue = GUI_FONT_FADE_MIN;
            mFadeDirection = true;
        }
    }
    
    gui_fontstyle_p current_style = styles;
    
    for(uint32_t i = 0; i < style_count; i++, current_style++)
    {
        if(current_style->fading)
        {
            current_style->real_color[0] = current_style->color[0] * mFadeValue;
            current_style->real_color[1] = current_style->color[1] * mFadeValue;
            current_style->real_color[2] = current_style->color[2] * mFadeValue;
        }
        else
        {
            memcpy(current_style->real_color, current_style->color, sizeof(GLfloat) * 3);
        }
    }
}

void gui_FontManager::Resize()
{
    gui_font_p current_font = fonts;
    
    float scale_factor = (float)((screen_info.w > screen_info.h)?(screen_info.h):(screen_info.w)) / GUI_SCREEN_METERING_FACTOR;
    
    for(uint32_t i = 0; i < font_count; i++, current_font++)
    {
        current_font->font->FaceSize((unsigned int)(((float)current_font->size) * scale_factor));
    }
}
