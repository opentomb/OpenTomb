
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include "../core/gl_util.h"
#include "../core/gl_font.h"
#include "../core/gl_text.h"
#include "../core/system.h"
#include "../core/console.h"
#include "../core/vmath.h"
#include "../engine_string.h"

#include "../render/camera.h"
#include "../render/render.h"
#include "../render/shader_description.h"
#include "../render/shader_manager.h"
#include "../skeletal_model.h"
#include "../script/script.h"
#include "../audio/audio.h"
#include "../inventory.h"
#include "../world.h"
#include "gui.h"
#include "gui_inventory.h"

extern GLuint backgroundBuffer;
extern GLfloat guiProjectionMatrix[16];
gui_ItemNotifier       Notifier;
gui_InventoryManager  *main_inventory_manager = NULL;

void Gui_InitNotifier()
{
    Notifier.SetRot(180.0, 270.0);
    Notifier.SetSize(128.0);
    Notifier.SetRotateTime(2500.0);
}

/**
 * That function updates item animation and rebuilds skeletal matrices;
 * @param bf - extended bone frame of the item;
 */
void Item_Frame(struct ss_bone_frame_s *bf, float time)
{
    Anim_SetNextFrame(&bf->animations, time);
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
            size /= ((bb[0] >= bb[2]) ? (bb[0]) : (bb[2]));
        }
        else
        {
            size /= ((bb[1] >= bb[2]) ? (bb[1]) : (bb[2]));
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
    mLabel_Title.line_width     = -1.0f;
    mLabel_Title.x_align        = GLTEXT_ALIGN_CENTER;
    mLabel_Title.y_align        = GLTEXT_ALIGN_TOP;

    mLabel_Title.font_id        = FONT_PRIMARY;
    mLabel_Title.style_id       = FONTSTYLE_MENU_TITLE;
    mLabel_Title.text           = mLabel_Title_text;
    mLabel_Title_text[0]        = 0;
    mLabel_Title.show           = 0;

    mLabel_ItemName.x           = 0.0;
    mLabel_ItemName.y           = 50.0;
    mLabel_ItemName.line_width  = -1.0f;
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
    if(mInventory && *mInventory)
    {
        this->frameStates(time);
        this->frameItems(time);
    }
    else
    {
        mCurrentState = INVENTORY_DISABLED;
        mNextState = INVENTORY_DISABLED;
    }
}

void gui_InventoryManager::frameStates(float time)
{
    switch(mCurrentState)
    {
        case INVENTORY_ACTIVATE:
            if(mNextState == INVENTORY_ACTIVATE)
            {
                mNextState = INVENTORY_IDLE;
                mCurrentState = INVENTORY_IDLE;
            }
            break;

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

                case INVENTORY_ACTIVATE:
                    mCurrentState = INVENTORY_ACTIVATE;
                    mNextState = INVENTORY_IDLE;
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

void gui_InventoryManager::frameItems(float time)
{
    int ring_item_index = 0;
    for(inventory_node_p i = *mInventory; i; i = i->next)
    {
        base_item_p bi = World_GetBaseItemByID(i->id);
        if(bi && (bi->type == mCurrentItemsType))
        {
            if((ring_item_index == mItemsOffset) && (mCurrentState == INVENTORY_ACTIVATE))
            {
                Item_Frame(bi->bf, time);
                if((bi->bf->animations.frame_changing_state == SS_CHANGING_END_ANIM))
                {
                    //ActivateItemCallback!
                    mNextState = INVENTORY_IDLE;
                    mCurrentState = INVENTORY_IDLE;
                }
            }
            else
            {
                Anim_SetAnimation(&bi->bf->animations, 0, 0);
                Item_Frame(bi->bf, 0.0);
            }
            ring_item_index++;
        }
    }
}

void gui_InventoryManager::render()
{
    if((mCurrentState != INVENTORY_DISABLED) && (mInventory != NULL) && (*mInventory != NULL))
    {
        float matrix[16], offset[3], ang;
        int ring_item_index = 0;
        mLabel_Title.x = screen_info.w / 2;
        mLabel_ItemName.x = screen_info.w / 2;
        for(inventory_node_p i = *mInventory; i; i = i->next)
        {
            base_item_p bi = World_GetBaseItemByID(i->id);
            if(bi && (bi->type == mCurrentItemsType))
            {
                Mat4_E_macro(matrix);
                matrix[12 + 2] = - mBaseRingRadius * 2.0;
                ang = (25.0f + mRingVerticalAngle) * M_PI / 180.0f;
                Mat4_RotateX_SinCos(matrix, sinf(ang), cosf(ang));
                ang = (mRingAngleStep * (-mItemsOffset + ring_item_index) + mRingAngle) * M_PI / 180.0f;
                Mat4_RotateY_SinCos(matrix, sinf(ang), cosf(ang));
                offset[0] = 0.0;
                offset[1] = mVerticalOffset;
                offset[2] = mRingRadius;
                Mat4_Translate(matrix, offset);
                Mat4_RotateX_SinCos(matrix,-1.0f, 0.0f);  //-90.0
                Mat4_RotateZ_SinCos(matrix, 1.0f, 0.0f);  //90.0
                if(ring_item_index == mItemsOffset)
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
                }
                else
                {
                    ang = M_PI_2 - ang;
                    Mat4_RotateZ_SinCos(matrix, sinf(ang), cosf(ang));
                }
                offset[0] = -0.5 * bi->bf->centre[0];
                offset[1] = -0.5 * bi->bf->centre[1];
                offset[2] = -0.5 * bi->bf->centre[2];
                Mat4_Translate(matrix, offset);
                Mat4_Scale(matrix, 0.7, 0.7, 0.7);
                Gui_RenderItem(bi->bf, 0.0, matrix);
                ring_item_index++;
            }
        }
    }
}

void Gui_DrawInventory(float time)
{
    main_inventory_manager->frame(time);
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

void Gui_DrawNotifier(float time)
{
    Notifier.Draw();
    Notifier.Animate(time);
}

// ===================================================================================
// ======================== ITEM NOTIFIER CLASS IMPLEMENTATION =======================
// ===================================================================================

gui_ItemNotifier::gui_ItemNotifier()
{
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

void gui_ItemNotifier::Animate(float time)
{
    if(!mActive)
    {
        return;
    }
    else
    {
        if(mRotateTime)
        {
            mCurrRotX += (time * mRotateTime);
            //mCurrRotY += (time * mRotateTime);

            mCurrRotX = (mCurrRotX > 360.0) ? (mCurrRotX - 360.0) : (mCurrRotX);
            //mCurrRotY = (mCurrRotY > 360.0) ? (mCurrRotY - 360.0) : (mCurrRotY);
        }

        float step = 0;

        if(mCurrTime == 0)
        {
            step = (mCurrPosX - mEndPosX) * (time * 4.0);
            step = (step <= 0.5) ? (0.5) : (step);

            mCurrPosX -= step;
            mCurrPosX  = (mCurrPosX < mEndPosX) ? (mEndPosX) : (mCurrPosX);

            if(mCurrPosX == mEndPosX)
                mCurrTime += time;
        }
        else if(mCurrTime < mShowTime)
        {
            mCurrTime += time;
        }
        else
        {
            step = (mCurrPosX - mEndPosX) * (time * 4.0);
            step = (step <= 0.5) ? (0.5) : (step);

            mCurrPosX += step;
            mCurrPosX  = (mCurrPosX > mStartPosX) ? (mStartPosX) : (mCurrPosX);

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

    mEndPosX = 0.85f * screen_info.w;
    mPosY    = 0.15f * screen_info.h;
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
