
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include "../core/gl_util.h"
#include "../core/gl_font.h"
#include "../core/gl_text.h"
#include "../core/system.h"
#include "../core/console.h"
#include "../core/vmath.h"

#include "../render/camera.h"
#include "../render/render.h"
#include "../render/shader_description.h"
#include "../render/shader_manager.h"
#include "../script/script.h"
#include "../image.h"
#include "../mesh.h"
#include "../skeletal_model.h"
#include "../entity.h"
#include "../character_controller.h"
#include "../engine.h"
#include "../audio.h"
#include "../engine_string.h"
#include "../world.h"
#include "../inventory.h"
#include "gui.h"

gui_ItemNotifier    Notifier;
gui_ProgressBar     Bar[BAR_LASTINDEX];

gui_InventoryManager  *main_inventory_manager = NULL;

static GLuint crosshairBuffer = 0;
static GLuint backgroundBuffer = 0;
static GLuint rectBuffer = 0;
static GLuint load_screen_tex = 0;
static GLfloat guiProjectionMatrix[16];

static GLfloat screenSize[2];

static void Gui_FillCrosshairBuffer();
static void Gui_FillBackgroundBuffer();

void Gui_Init()
{
    Gui_InitBars();
    Gui_InitNotifier();

    qglGenBuffersARB(1, &crosshairBuffer);
    qglGenBuffersARB(1, &backgroundBuffer);
    qglGenBuffersARB(1, &rectBuffer);
    qglGenTextures(1, &load_screen_tex);
    Gui_FillCrosshairBuffer();
    Gui_FillBackgroundBuffer();

    main_inventory_manager = new gui_InventoryManager();
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

void Gui_InitNotifier()
{
    Notifier.SetPos(850.0, 850.0);
    Notifier.SetRot(180.0, 270.0);
    Notifier.SetSize(128.0);
    Notifier.SetRotateTime(2500.0);
}

void Gui_Destroy()
{
    if(main_inventory_manager)
    {
        delete main_inventory_manager;
        main_inventory_manager = NULL;
    }

    qglDeleteTextures(1, &load_screen_tex);
    qglDeleteBuffersARB(1, &crosshairBuffer);
    qglDeleteBuffersARB(1, &backgroundBuffer);
    qglDeleteBuffersARB(1, &rectBuffer);
}


void Gui_UpdateResize()
{
    for(int i = 0; i < BAR_LASTINDEX; i++)
    {
        Bar[i].Resize();
    }

    Gui_FillCrosshairBuffer();
    Gui_FillBackgroundBuffer();
}


void Gui_Render()
{
    const text_shader_description *shader = renderer.shaderManager->getTextShader();
    screenSize[0] = screen_info.w;
    screenSize[1] = screen_info.h;
    qglUseProgramObjectARB(shader->program);
    qglUniform1iARB(shader->sampler, 0);
    qglUniform2fvARB(shader->screenSize, 1, screenSize);
    qglUniform1fARB(shader->colorReplace, 0.0f);

    qglPushAttrib(GL_ENABLE_BIT | GL_PIXEL_MODE_BIT | GL_COLOR_BUFFER_BIT);
    qglPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT | GL_CLIENT_VERTEX_ARRAY_BIT);

    qglEnableClientState(GL_TEXTURE_COORD_ARRAY);
    qglPolygonMode(GL_FRONT, GL_FILL);
    qglFrontFace(GL_CCW);
    qglEnable(GL_BLEND);
    qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    qglDisable(GL_ALPHA_TEST);

    if(World_GetPlayer() && main_inventory_manager)
    {
        Gui_DrawInventory();
    }
    Gui_DrawNotifier();
    qglUseProgramObjectARB(shader->program);

    qglDepthMask(GL_FALSE);

    qglPixelStorei(GL_UNPACK_LSB_FIRST, GL_FALSE);
    qglPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    if(screen_info.crosshair != 0)
    {
        Gui_DrawCrosshair();
    }
    Gui_DrawBars();

    qglUniform1fARB(shader->colorReplace, 1.0f);
    GLText_RenderStrings();
    Con_Draw(engine_frame_time);

    qglUniform1fARB(shader->colorReplace, 0.0f);
    qglDepthMask(GL_TRUE);
    qglPopClientAttrib();
    qglPopAttrib();
}

/**
 * That function updates item animation and rebuilds skeletal matrices;
 * @param bf - extended bone frame of the item;
 */
void Item_Frame(struct ss_bone_frame_s *bf, float time)
{
    int frame_switch_state = Anim_SetNextFrame(&bf->animations, time);
    //Anim_GetNextFrame(&bf->animations, bf->animations.period, stc, &bf->animations.next_frame, &bf->animations.next_animation, &was_last_anim);
    SSBoneFrame_Update(bf, time);
}


/**
 * The base function, that draws one item by them id. Items may be animated.
 * This time for correct time calculation that function must be called every frame.
 * @param item_id - the base item id;
 * @param size - the item size on the screen;
 * @param str - item description - shows near / under item model;
 */
void Gui_RenderItem(struct ss_bone_frame_s *bf, float size, const float *mvMatrix)
{
    const lit_shader_description *shader = renderer.shaderManager->getEntityShader(0);
    qglUseProgramObjectARB(shader->program);
    qglUniform1iARB(shader->number_of_lights, 0);
    qglUniform4fARB(shader->light_ambient, 1.0f, 1.0f, 1.0f, 1.0f);

    if(size != 0.0)
    {
        float bb[3];
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

        float scaledMatrix[16];
        Mat4_E(scaledMatrix);
        if(size < 1.0)          // only reduce items size...
        {
            Mat4_Scale(scaledMatrix, size, size, size);
        }
        float scaledMvMatrix[16];
        Mat4_Mat4_mul(scaledMvMatrix, mvMatrix, scaledMatrix);
        float mvpMatrix[16];
        Mat4_Mat4_mul(mvpMatrix, guiProjectionMatrix, scaledMvMatrix);

        // Render with scaled model view projection matrix
        // Use original modelview matrix, as that is used for normals whose size shouldn't change.
        renderer.DrawSkeletalModel(shader, bf, mvMatrix, mvpMatrix);
    }
    else
    {
        float mvpMatrix[16];
        Mat4_Mat4_mul(mvpMatrix, guiProjectionMatrix, mvMatrix);
        renderer.DrawSkeletalModel(shader, bf, mvMatrix, mvpMatrix);
    }
}

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

    mLabel_Title.x              = 0.0;
    mLabel_Title.y              = 30.0;
    mLabel_Title.x_align        = GLTEXT_ALIGN_CENTER;
    mLabel_Title.y_align        = GLTEXT_ALIGN_TOP;

    mLabel_Title.font_id        = FONT_PRIMARY;
    mLabel_Title.style_id       = FONTSTYLE_MENU_TITLE;
    mLabel_Title.text           = mLabel_Title_text;
    mLabel_Title_text[0]        = 0;
    mLabel_Title.show           = 0;

    mLabel_ItemName.x           = 0.0;
    mLabel_ItemName.y           = 50.0;
    mLabel_ItemName.x_align     = GLTEXT_ALIGN_CENTER;
    mLabel_ItemName.y_align     = GLTEXT_ALIGN_BOTTOM;

    mLabel_ItemName.font_id     = FONT_PRIMARY;
    mLabel_ItemName.style_id    = FONTSTYLE_MENU_CONTENT;
    mLabel_ItemName.text        = mLabel_ItemName_text;
    mLabel_ItemName_text[0]     = 0;
    mLabel_ItemName.show        = 0;

    GLText_AddLine(&mLabel_ItemName);
    GLText_AddLine(&mLabel_Title);
}

gui_InventoryManager::~gui_InventoryManager()
{
    mCurrentState = INVENTORY_DISABLED;
    mNextState = INVENTORY_DISABLED;
    mInventory = NULL;

    mLabel_ItemName.show = 0;
    GLText_DeleteLine(&mLabel_ItemName);

    mLabel_Title.show = 0;
    GLText_DeleteLine(&mLabel_Title);
}

int gui_InventoryManager::getItemElementsCountByType(int type)
{
    int ret = 0;
    for(inventory_node_p i = *mInventory; i; i = i->next)
    {
        base_item_p bi = World_GetBaseItemByID(i->id);
        if(bi && (bi->type == type))
        {
            ret++;
        }
    }
    return ret;
}

void gui_InventoryManager::restoreItemAngle(float time)
{
    if(mItemAngle > 0.0f)
    {
        if(mItemAngle <= 180.0f)
        {
            mItemAngle -= 180.0f * time / mRingRotatePeriod;
            if(mItemAngle < 0.0f)
            {
                mItemAngle = 0.0f;
            }
        }
        else
        {
            mItemAngle += 180.0f * time / mRingRotatePeriod;
            if(mItemAngle >= 360.0f)
            {
                mItemAngle = 0.0f;
            }
        }
    }
}

void gui_InventoryManager::setInventory(struct inventory_node_s **i)
{
    mInventory = i;
    mCurrentState = INVENTORY_DISABLED;
    mNextState = INVENTORY_DISABLED;
    mLabel_ItemName.show = 0;
    mLabel_Title.show = 0;
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

    Script_GetString(engine_lua, string_index, GUI_LINE_DEFAULTSIZE, mLabel_Title_text);
}

int gui_InventoryManager::setItemsType(int type)
{
    if((mInventory == NULL) || (*mInventory == NULL))
    {
        mCurrentItemsType = type;
        return type;
    }

    int count = this->getItemElementsCountByType(type);
    if(count == 0)
    {
        for(inventory_node_p i = *mInventory; i; i = i->next)
        {
            base_item_p bi = World_GetBaseItemByID(i->id);
            if(bi)
            {
                type = bi->type;
                count = this->getItemElementsCountByType(mCurrentItemsType);
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
                    Audio_Send(Script_GetGlobalSound(engine_lua, TR_AUDIO_SOUND_GLOBALID_MENUCLOSE));
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
                    mNextItemsCount = this->getItemElementsCountByType(mCurrentItemsType + 1);
                    if(mNextItemsCount > 0)
                    {
                        //Audio_Send(Script_GetGlobalSound(engine_lua, TR_AUDIO_SOUND_GLOBALID_MENUCLOSE));
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
                    mNextItemsCount = this->getItemElementsCountByType(mCurrentItemsType - 1);
                    if(mNextItemsCount > 0)
                    {
                        //Audio_Send(Script_GetGlobalSound(engine_lua, TR_AUDIO_SOUND_GLOBALID_MENUCLOSE));
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
                    Audio_Send(Script_GetGlobalSound(engine_lua, TR_AUDIO_SOUND_GLOBALID_MENUOPEN));
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
    if((mCurrentState != INVENTORY_DISABLED) && (mInventory != NULL) && (*mInventory != NULL))
    {
        float matrix[16], offset[3], ang;
        int num = 0;
        mLabel_Title.x = screen_info.w / 2;
        mLabel_ItemName.x = screen_info.w / 2;
        for(inventory_node_p i = *mInventory; i; i = i->next)
        {
            base_item_p bi = World_GetBaseItemByID(i->id);
            if((bi == NULL) || (bi->type != mCurrentItemsType))
            {
                continue;
            }

            Mat4_E_macro(matrix);
            matrix[12 + 2] = - mBaseRingRadius * 2.0;
            ang = (25.0f + mRingVerticalAngle) * M_PI / 180.0f;
            Mat4_RotateX_SinCos(matrix, sinf(ang), cosf(ang));
            ang = (mRingAngleStep * (-mItemsOffset + num) + mRingAngle) * M_PI / 180.0f;
            Mat4_RotateY_SinCos(matrix, sinf(ang), cosf(ang));
            offset[0] = 0.0;
            offset[1] = mVerticalOffset;
            offset[2] = mRingRadius;
            Mat4_Translate(matrix, offset);
            Mat4_RotateX_SinCos(matrix,-1.0f, 0.0f);  //-90.0
            Mat4_RotateZ_SinCos(matrix, 1.0f, 0.0f);  //90.0
            if(num == mItemsOffset)
            {
                if(bi->name[0])
                {
                    if(i->count == 1)
                    {
                        strncpy(mLabel_ItemName_text, bi->name, GUI_LINE_DEFAULTSIZE);
                    }
                    else
                    {
                        snprintf(mLabel_ItemName_text, GUI_LINE_DEFAULTSIZE, "%s (%d)", bi->name, i->count);
                    }
                }
                else
                {
                    snprintf(mLabel_ItemName_text, GUI_LINE_DEFAULTSIZE, "ITEM_ID_%d (%d)", i->id, i->count);
                }
                ang = M_PI_2 + M_PI * mItemAngle / 180.0f - ang;
                Mat4_RotateZ_SinCos(matrix, sinf(ang), cosf(ang));
                Item_Frame(bi->bf, 0.0);                                        // here will be time != 0 for using items animation
            }
            else
            {
                ang = M_PI_2 - ang;
                Mat4_RotateZ_SinCos(matrix, sinf(ang), cosf(ang));
                Item_Frame(bi->bf, 0.0);
            }
            offset[0] = -0.5 * bi->bf->centre[0];
            offset[1] = -0.5 * bi->bf->centre[1];
            offset[2] = -0.5 * bi->bf->centre[2];
            Mat4_Translate(matrix, offset);
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
        const GLfloat far_dist = 4096.0f;
        const GLfloat near_dist = -1.0f;

        Mat4_E_macro(guiProjectionMatrix);
        guiProjectionMatrix[0 * 4 + 0] = 2.0 / ((GLfloat)screen_info.w);
        guiProjectionMatrix[1 * 4 + 1] = 2.0 / ((GLfloat)screen_info.h);
        guiProjectionMatrix[2 * 4 + 2] =-2.0 / (far_dist - near_dist);
        guiProjectionMatrix[3 * 4 + 0] =-1.0;
        guiProjectionMatrix[3 * 4 + 1] =-1.0;
        guiProjectionMatrix[3 * 4 + 2] =-(far_dist + near_dist) / (far_dist - near_dist);
    }
    else                                                                        // set camera coordinate system
    {
        memcpy(guiProjectionMatrix, engine_camera.gl_proj_mat, sizeof(GLfloat[16]));
    }
}

void Gui_FillBackgroundBuffer()
{
    GLfloat x0 = 0.0f;
    GLfloat y0 = 0.0f;
    GLfloat x1 = screen_info.w;
    GLfloat y1 = screen_info.h;
    GLfloat *v, backgroundArray[32];
    GLfloat color[4] = {0.0f, 0.0f, 0.0f, 0.5f};

    v = backgroundArray;
   *v++ = x0; *v++ = y0;
    vec4_copy(v, color);
    v += 4;
   *v++ = 0.0f; *v++ = 0.0f;

   *v++ = x1; *v++ = y0;
    vec4_copy(v, color);
    v += 4;
   *v++ = 1.0f; *v++ = 0.0f;

   *v++ = x1; *v++ = y1;
    vec4_copy(v, color);
    v += 4;
   *v++ = 1.0f; *v++ = 1.0f;

   *v++ = x0; *v++ = y1;
    vec4_copy(v, color);
    v += 4;
   *v++ = 0.0f; *v++ = 1.0f;

    qglBindBufferARB(GL_ARRAY_BUFFER, backgroundBuffer);
    qglBufferDataARB(GL_ARRAY_BUFFER, sizeof(GLfloat[32]), backgroundArray, GL_STATIC_DRAW);
}

void Gui_FillCrosshairBuffer()
{
    GLfloat x = (GLfloat)screen_info.w / 2.0f;
    GLfloat y = (GLfloat)screen_info.h / 2.0f;
    GLfloat *v, crosshairArray[32];
    const GLfloat size = 8.0f;
    const GLfloat color[4] = {1.0f, 0.0f, 0.0f, 1.0f};

    v = crosshairArray;
   *v++ = x; *v++ = y - size;
    vec4_copy(v, color);
    v += 4;
   *v++ = 0.0; *v++ = 0.0;

   *v++ = x; *v++ = y + size;
    vec4_copy(v, color);
    v += 4;
   *v++ = 0.0; *v++ = 0.0;

   *v++ = x - size; *v++ = y;
    vec4_copy(v, color);
    v += 4;
   *v++ = 0.0; *v++ = 0.0;

   *v++ = x + size; *v++ = y;
    vec4_copy(v, color);
    v += 4;
   *v++ = 0.0; *v++ = 0.0;

    // copy vertices into GL buffer
    qglBindBufferARB(GL_ARRAY_BUFFER, crosshairBuffer);
    qglBufferDataARB(GL_ARRAY_BUFFER, sizeof(GLfloat[32]), crosshairArray, GL_STATIC_DRAW);
}

void Gui_DrawCrosshair()
{
    // TBI: actual ingame crosshair
    BindWhiteTexture();
    qglBindBufferARB(GL_ARRAY_BUFFER_ARB, crosshairBuffer);
    qglVertexPointer(2, GL_FLOAT, 8 * sizeof(GLfloat), (void *)0);
    qglColorPointer(4, GL_FLOAT, 8 * sizeof(GLfloat), (void *)sizeof(GLfloat[2]));
    qglTexCoordPointer(2, GL_FLOAT, 8 * sizeof(GLfloat), (void *)sizeof(GLfloat[6]));
    qglDrawArrays(GL_LINES, 0, 4);
}

void Gui_DrawBars()
{
    entity_p player = World_GetPlayer();
    if(player && player->character)
    {
        if(player->character->weapon_current_state > WEAPON_STATE_HIDE_TO_READY)
        {
            Bar[BAR_HEALTH].Forced = true;
        }

        Bar[BAR_AIR].Show    (Character_GetParam(player, PARAM_AIR    ));
        Bar[BAR_STAMINA].Show(Character_GetParam(player, PARAM_STAMINA));
        Bar[BAR_HEALTH].Show (Character_GetParam(player, PARAM_HEALTH ));
        Bar[BAR_WARMTH].Show (Character_GetParam(player, PARAM_WARMTH ));
    }
}

void Gui_DrawInventory()
{
    main_inventory_manager->frame(engine_frame_time);
    if(main_inventory_manager->getCurrentState() == gui_InventoryManager::INVENTORY_DISABLED)
    {
        return;
    }

    qglDepthMask(GL_FALSE);
    {
        BindWhiteTexture();
        qglBindBufferARB(GL_ARRAY_BUFFER_ARB, backgroundBuffer);
        qglVertexPointer(2, GL_FLOAT, 8 * sizeof(GLfloat), (void *)0);
        qglColorPointer(4, GL_FLOAT, 8 * sizeof(GLfloat), (void *)sizeof(GLfloat[2]));
        qglTexCoordPointer(2, GL_FLOAT, 8 * sizeof(GLfloat), (void *)sizeof(GLfloat[6]));
        qglDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }
    qglDepthMask(GL_TRUE);
    qglClear(GL_DEPTH_BUFFER_BIT);

    qglPushAttrib(GL_ENABLE_BIT);
    qglEnable(GL_ALPHA_TEST);
    qglPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
    qglEnableClientState(GL_NORMAL_ARRAY);
    qglEnableClientState(GL_TEXTURE_COORD_ARRAY);

    Gui_SwitchGLMode(0);
    main_inventory_manager->render();
    Gui_SwitchGLMode(1);
    qglPopClientAttrib();
    qglPopAttrib();
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
    qglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    qglPushAttrib(GL_ENABLE_BIT | GL_PIXEL_MODE_BIT | GL_COLOR_BUFFER_BIT);
    qglPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
    qglPixelStorei(GL_UNPACK_LSB_FIRST, GL_FALSE);
    qglPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    qglEnableClientState(GL_VERTEX_ARRAY);
    qglEnableClientState(GL_TEXTURE_COORD_ARRAY);
    qglEnableClientState(GL_COLOR_ARRAY);
    qglDisableClientState(GL_NORMAL_ARRAY);

    qglEnable(GL_BLEND);
    qglEnable(GL_TEXTURE_2D);
    qglDisable(GL_ALPHA_TEST);
    qglDepthMask(GL_FALSE);

    qglPolygonMode(GL_FRONT, GL_FILL);
    qglFrontFace(GL_CCW);

    const GLfloat color_w[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    const text_shader_description *shader = renderer.shaderManager->getTextShader();
    screenSize[0] = screen_info.w;
    screenSize[1] = screen_info.h;
    qglUseProgramObjectARB(shader->program);
    qglUniform1iARB(shader->sampler, 0);
    qglUniform2fvARB(shader->screenSize, 1, screenSize);

    Gui_DrawRect(0.0, 0.0, screen_info.w, screen_info.h, color_w, color_w, color_w, color_w, BM_OPAQUE, load_screen_tex);
    Bar[BAR_LOADING].Show(value);

    qglDepthMask(GL_TRUE);
    qglPopClientAttrib();
    qglPopAttrib();

    Engine_GLSwapWindow();
}


bool Gui_LoadScreenAssignPic(const char* pic_name)
{
    size_t pic_len = strlen(pic_name);
    size_t base_len = strlen(Engine_GetBasePath());
    size_t buf_len = pic_len + base_len + 5;
    char image_name_buf[buf_len];
    int image_format = 0;

    strncpy(image_name_buf, Engine_GetBasePath(), buf_len);
    strncat(image_name_buf, pic_name, buf_len);
    if(pic_len > 3)
    {
        char *ext = image_name_buf + pic_len + base_len;
        if(strncpy(ext, ".png", 5) && Sys_FileFound(image_name_buf, 0))
        {
            image_format = IMAGE_FORMAT_PNG;
        }else if(strncpy(ext, ".pcx", 5) && Sys_FileFound(image_name_buf, 0))
        {
            image_format = IMAGE_FORMAT_PCX;
        }
    }

    uint8_t *img_pixels = NULL;
    uint32_t img_w = 0;
    uint32_t img_h = 0;
    uint32_t img_bpp = 32;
    if(Image_Load(image_name_buf, image_format, &img_pixels, &img_w, &img_h, &img_bpp))
    {
        GLenum       texture_format;
        GLuint       color_depth;

        if(img_bpp == 32)        // Contains an alpha channel
        {
            texture_format = GL_RGBA;
            color_depth = GL_RGBA;
        }
        else if(img_bpp == 24)   // No alpha channel
        {
            texture_format = GL_RGB;
            color_depth = GL_RGB;
        }
        else
        {
            free(img_pixels);
            return false;
        }

        // Bind the texture object
        qglBindTexture(GL_TEXTURE_2D, load_screen_tex);

        // Set the texture's stretching properties
        qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Edit the texture object's image data using the information SDL_Surface gives us
        qglTexImage2D(GL_TEXTURE_2D, 0, color_depth, img_w, img_h, 0,
                     texture_format, GL_UNSIGNED_BYTE, img_pixels);
        qglBindTexture(GL_TEXTURE_2D, 0);
        free(img_pixels);
        return true;
    }

    return false;
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
            qglBlendFunc(GL_SRC_ALPHA, GL_ONE);
            break;
        case BM_SIMPLE_SHADE:
            qglBlendFunc(GL_ONE_MINUS_SRC_COLOR, GL_ONE_MINUS_SRC_ALPHA);
            break;
        case BM_SCREEN:
            qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            break;
        default:
        case BM_OPAQUE:
            qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            break;
    };

    GLfloat x0 = x;
    GLfloat y0 = y + height;
    GLfloat x1 = x + width;
    GLfloat y1 = y;
    GLfloat *v, rectArray[32];

    v = rectArray;
   *v++ = x0; *v++ = y0;
   *v++ = 0.0f; *v++ = 0.0f;
    vec4_copy(v, colorUpperLeft);
    v += 4;

   *v++ = x1; *v++ = y0;
   *v++ = 1.0f; *v++ = 0.0f;
    vec4_copy(v, colorUpperRight);
    v += 4;

   *v++ = x1; *v++ = y1;
   *v++ = 1.0f; *v++ = 1.0f;
    vec4_copy(v, colorLowerRight);
    v += 4;

   *v++ = x0; *v++ = y1;
   *v++ = 0.0f; *v++ = 1.0f;
    vec4_copy(v, colorLowerLeft);

    if(qglIsTexture(texture))
    {
        qglBindTexture(GL_TEXTURE_2D, texture);
    }
    else
    {
        BindWhiteTexture();
    }
    qglBindBufferARB(GL_ARRAY_BUFFER, rectBuffer);
    qglBufferDataARB(GL_ARRAY_BUFFER, sizeof(GLfloat[32]), rectArray, GL_DYNAMIC_DRAW);
    qglVertexPointer(2, GL_FLOAT, 8 * sizeof(GLfloat), (void *)0);
    qglTexCoordPointer(2, GL_FLOAT, 8 * sizeof(GLfloat), (void *)sizeof(GLfloat[2]));
    qglColorPointer(4, GL_FLOAT, 8 * sizeof(GLfloat), (void *)sizeof(GLfloat[4]));
    qglDrawArrays(GL_TRIANGLE_FAN, 0, 4);
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
            mY = ((float)screen_info.h - ((float)(mAbsHeight+mAbsBorderSize * 2) * screen_info.h)) / 2 +
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
    mBlinkInterval = (float)interval / 1000.0f;
    mBlinkCnt      = (float)interval / 1000.0f;  // Also reset blink counter.
}

// Set extrude overlay effect parameters.
void gui_ProgressBar::SetExtrude(bool enabled, uint8_t depth)
{
    mExtrude = enabled;
    memset(mExtrudeDepth, 0, sizeof(float) * 5);    // Set all colors to 0.
    mExtrudeDepth[3] = (float)depth / 255.0f;       // We need only alpha transparency.
    mExtrudeDepth[4] = mExtrudeDepth[3];
}

// Set autoshow and fade parameters.
// Please note that fade parameters are actually independent of autoshow.
void gui_ProgressBar::SetAutoshow(bool enabled, int delay, bool fade, int fadeDelay)
{
    mAutoShow = enabled;

    mAutoShowDelay = (float)delay / 1000.0f;
    mAutoShowCnt   = (float)delay / 1000.0f;     // Also reset autoshow counter.

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
    value  = (value >= 0.0f)?(value):(0.0f);
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
        if(mAutoShowCnt > 0.0f)
        {
            Visible = true;
            mAutoShowCnt -= engine_frame_time;

            if(mAutoShowCnt <= 0.0f)
            {
                mAutoShowCnt = 0.0f;
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
            mBlinkCnt = mBlinkInterval * 2.0f;
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

    mEndPosX = mAbsPosX;
    mPosY    = mAbsPosY;
    mCurrPosX = screen_info.w + ((float)screen_info.w / GUI_NOTIFIER_OFFSCREEN_DIVIDER * mSize);
    mStartPosX = mCurrPosX;    // Equalize current and start positions.
}

void gui_ItemNotifier::Draw()
{
    if(mActive)
    {
        base_item_p item = World_GetBaseItemByID(mItem);
        if(item)
        {
            int anim = item->bf->animations.current_animation;
            int frame = item->bf->animations.current_frame;
            float time = item->bf->animations.frame_time;

            item->bf->animations.current_animation = 0;
            item->bf->animations.current_frame = 0;
            item->bf->animations.frame_time = 0.0;

            Item_Frame(item->bf, 0.0);
            float matrix[16];
            Mat4_E_macro(matrix);
            matrix[12 + 0] = mCurrPosX;
            matrix[12 + 1] = mPosY;
            matrix[12 + 2] = -2048.0;
            float ang = (mCurrRotX + mRotX) * M_PI / 180.0f;
            Mat4_RotateY_SinCos(matrix, sinf(ang), cosf(ang));
            ang = (mCurrRotY + mRotY) * M_PI / 180.0f;
            Mat4_RotateX_SinCos(matrix, sinf(ang), cosf(ang));
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
