
#include <stdint.h>
#include <string.h>
#include <stdio.h>
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
#include "../engine.h"
#include "../game.h"
#include "../inventory.h"
#include "../entity.h"
#include "../gameflow.h"
#include "../world.h"
#include "gui.h"
#include "gui_menu.h"
#include "gui_inventory.h"


extern GLuint backgroundBuffer;
extern GLfloat guiProjectionMatrix[16];
gui_ItemNotifier       Notifier;
gui_InventoryManager  *main_inventory_manager = NULL;

void Gui_InitNotifier()
{
    Notifier.SetRot(180.0f, 270.0f);
    Notifier.SetSize(128.0f);
    Notifier.SetRotateTime(2500.0f);
}

int32_t Item_Use(struct inventory_node_s **root, uint32_t item_id, uint32_t actor_id)
{
    inventory_node_p i = *root;
    base_item_p bi = NULL;

    for(; i; i = i->next)
    {
        if(i->id == item_id)
        {
            bi = World_GetBaseItemByID(i->id);
            break;
        }
    }

    if(bi)
    {
        switch(bi->id)
        {
            case ITEM_LARAHOME:
                return Gameflow_SetGame(Gameflow_GetCurrentGameID(), 0);

            case ITEM_PASSPORT:
                return Gameflow_SetGame(Gameflow_GetCurrentGameID(), 1);

            case ITEM_COMPASS:
            case ITEM_VIDEO:
            case ITEM_AUDIO:
            case ITEM_CONTROLS:
            case ITEM_LOAD:
            case ITEM_SAVE:
            case ITEM_MAP:
                break;

            default:
                return Script_UseItem(engine_lua, i->id, actor_id);
        }
    }

    return 0;
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
    qglUniform1fARB(shader->dist_fog, 65536.0f);

    if(size != 0.0f)
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
        size *= 0.8f;

        float scaledMatrix[16];
        Mat4_E(scaledMatrix);
        if(size < 1.0f)          // only reduce items size...
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
    m_command                    = NONE;
    mCurrentItemsType           = GUI_MENU_ITEMTYPE_SYSTEM;
    mNextItemsType              = GUI_MENU_ITEMTYPE_SYSTEM;
    mCurrentItemsCount          = 0;
    mSelectedItem               = 0;

    mItemTime                   = 0.0f;
    mRingRotatePeriod           = 0.5f;
    mRingTime                   = 0.0f;
    mRingAngle                  = 0.0f;
    mRingVerticalAngle          = 0.0f;
    mRingAngleStep              = 0.0f;
    mBaseRingRadius             = 600.0f;
    mRingRadius                 = 600.0f;
    mVerticalOffset             = 0.0f;

    mItemRotatePeriod           = 4.0f;
    m_item_angle_z              = 0.0f;
    m_item_angle_x              = 0.0f;
    m_current_scale             = 1.0f;
    m_item_offset_z             = 0.0f;

    m_current_menu              = NULL;
    m_menu_mode                 = 0;
    mInventory                  = NULL;
    mOwnerId                    = ENTITY_ID_NONE;

    mLabel_Title.x              = 0.0f;
    mLabel_Title.y              = 0.0f;
    mLabel_Title.line_width     = -1.0f;
    mLabel_Title.x_align        = GLTEXT_ALIGN_CENTER;
    mLabel_Title.y_align        = GLTEXT_ALIGN_TOP;
    mLabel_Title.next           = NULL;
    mLabel_Title.prev           = NULL;

    mLabel_Title.font_id        = FONT_PRIMARY;
    mLabel_Title.style_id       = FONTSTYLE_MENU_TITLE;
    mLabel_Title.text           = mLabel_Title_text;
    mLabel_Title_text[0]        = 0;
    mLabel_Title.show           = 0;

    mLabel_ItemName.x           = 0.0f;
    mLabel_ItemName.y           = 50.0f;
    mLabel_ItemName.line_width  = -1.0f;
    mLabel_ItemName.x_align     = GLTEXT_ALIGN_CENTER;
    mLabel_ItemName.y_align     = GLTEXT_ALIGN_BOTTOM;
    mLabel_ItemName.next        = NULL;
    mLabel_ItemName.prev        = NULL;

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
    m_command = CLOSE;
    mInventory = NULL;

    mLabel_ItemName.show = 0;
    GLText_DeleteLine(&mLabel_ItemName);

    mLabel_Title.show = 0;
    GLText_DeleteLine(&mLabel_Title);

    if(m_current_menu)
    {
        Gui_SetCurrentMenu(NULL);
        Gui_DeleteObjects(m_current_menu);
        m_current_menu = NULL;
    }
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
    if(m_item_angle_z > 0.0f)
    {
        if(m_item_angle_z <= 180.0f)
        {
            m_item_angle_z -= 180.0f * time / mRingRotatePeriod;
            if(m_item_angle_z < 0.0f)
            {
                m_item_angle_z = 0.0f;
            }
        }
        else
        {
            m_item_angle_z += 180.0f * time / mRingRotatePeriod;
            if(m_item_angle_z >= 360.0f)
            {
                m_item_angle_z = 0.0f;
            }
        }
    }
}

void gui_InventoryManager::send(Command cmd)
{
    m_command = cmd;
}

void gui_InventoryManager::setInventory(struct inventory_node_s **i, uint32_t owner_id)
{
    mInventory = i;
    mOwnerId = owner_id;
    mCurrentState = INVENTORY_DISABLED;
    m_command = NONE;
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

void gui_InventoryManager::updateCurrentRing()
{
    if(mInventory && *mInventory)
    {
        mCurrentItemsCount = this->getItemElementsCountByType(mCurrentItemsType);
        setTitle(mCurrentItemsType);
        if(mCurrentItemsCount)
        {
            mRingAngleStep = 360.0f / mCurrentItemsCount;
            mSelectedItem %= mCurrentItemsCount;
        }
        mRingAngle = 180.0f;
    }
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
        m_command = CLOSE;
    }
}

void gui_InventoryManager::frameStates(float time)
{
    switch(mCurrentState)
    {
        case INVENTORY_R_LEFT:
            mRingTime += time;
            mRingAngle = mRingAngleStep * mRingTime / mRingRotatePeriod;
            if(mRingTime >= mRingRotatePeriod)
            {
                mRingTime = 0.0f;
                mRingAngle = 0.0f;
                m_command = NONE;
                mCurrentState = INVENTORY_IDLE;
                mSelectedItem--;
                if(mSelectedItem < 0)
                {
                    mSelectedItem = mCurrentItemsCount - 1;
                }
            }
            restoreItemAngle(time);
            break;

        case INVENTORY_R_RIGHT:
            mRingTime += time;
            mRingAngle = -mRingAngleStep * mRingTime / mRingRotatePeriod;
            m_command = INVENTORY_R_RIGHT;
            if(mRingTime >= mRingRotatePeriod)
            {
                mRingTime = 0.0f;
                mRingAngle = 0.0f;
                m_command = NONE;
                mCurrentState = INVENTORY_IDLE;
                mSelectedItem++;
                if(mSelectedItem >= mCurrentItemsCount)
                {
                    mSelectedItem = 0;
                }
            }
            restoreItemAngle(time);
            break;

        case INVENTORY_IDLE:
            mRingTime = 0.0f;
            switch(m_command)
            {
                default:
                case NONE:
                    mItemTime += time;
                    m_item_angle_z = 360.0f * mItemTime / mItemRotatePeriod;
                    if(mItemTime >= mItemRotatePeriod)
                    {
                        mItemTime = 0.0f;
                        m_item_angle_z = 0.0f;
                    }
                    mLabel_ItemName.show = 1;
                    mLabel_Title.show = 1;
                    break;

                case ACTIVATE:
                    mCurrentState = INVENTORY_ACTIVATING;
                    m_command = NONE;
                    break;

                case CLOSE:
                    Audio_Send(Script_GetGlobalSound(engine_lua, TR_AUDIO_SOUND_GLOBALID_MENUCLOSE));
                    mLabel_ItemName.show = 0;
                    mLabel_Title.show = 0;
                    mCurrentState = INVENTORY_CLOSING;
                    break;

                case LEFT:
                case RIGHT:
                    if(mCurrentItemsCount >= 1)
                    {
                        Audio_Send(TR_AUDIO_SOUND_MENUROTATE);
                        mLabel_ItemName.show = 0;
                        mCurrentState = (m_command == LEFT) ? (INVENTORY_R_LEFT) : (INVENTORY_R_RIGHT);
                        mItemTime = 0.0f;
                    }
                    break;

                case UP:
                    if(mCurrentItemsType < GUI_MENU_ITEMTYPE_QUEST)
                    {
                        //Audio_Send(Script_GetGlobalSound(engine_lua, TR_AUDIO_SOUND_GLOBALID_MENUCLOSE));
                        mNextItemsType = mCurrentItemsType + 1;
                        mCurrentState = INVENTORY_UP;
                        mRingTime = 0.0f;
                    }
                    m_command = NONE;
                    mLabel_ItemName.show = 0;
                    mLabel_Title.show = 0;
                    break;

                case DOWN:
                    if(mCurrentItemsType > 0)
                    {
                        //Audio_Send(Script_GetGlobalSound(engine_lua, TR_AUDIO_SOUND_GLOBALID_MENUCLOSE));
                        mNextItemsType = mCurrentItemsType - 1;
                        mCurrentState = INVENTORY_DOWN;
                        mRingTime = 0.0f;
                    }
                    m_command = NONE;
                    mLabel_ItemName.show = 0;
                    mLabel_Title.show = 0;
                    break;
            };
            break;

        case INVENTORY_DISABLED:
            if(m_command == OPEN)
            {
                Audio_Send(Script_GetGlobalSound(engine_lua, TR_AUDIO_SOUND_GLOBALID_MENUOPEN));
                for(inventory_node_p i = *mInventory; i; i = i->next)
                {
                    base_item_p bi = World_GetBaseItemByID(i->id);
                    if(bi)
                    {
                        if(bi->type == GUI_MENU_ITEMTYPE_SUPPLY)
                        {
                            mCurrentItemsType = GUI_MENU_ITEMTYPE_SUPPLY;
                            break;
                        }
                        else
                        {
                            mCurrentItemsType = bi->type;
                        }
                    }
                }
                this->updateCurrentRing();
                mItemTime = 0.0f;
                m_item_offset_z = 0.0f;
                m_current_scale = 1.0f;
                mCurrentState = INVENTORY_OPENING;
                mRingAngle = 180.0f;
                mRingVerticalAngle = 180.0f;
            }
            break;

        case INVENTORY_UP:
            mCurrentState = INVENTORY_UP;
            mRingTime += time;
            if(mRingTime < mRingRotatePeriod)
            {
                restoreItemAngle(time);
                mRingRadius = mBaseRingRadius * (mRingRotatePeriod - mRingTime) / mRingRotatePeriod;
                mVerticalOffset = - mBaseRingRadius * mRingTime / mRingRotatePeriod;
                mRingAngle += 180.0f * time / mRingRotatePeriod;
            }
            else if(mRingTime < 2.0f * mRingRotatePeriod)
            {
                if(mRingTime - time <= mRingRotatePeriod)
                {
                    //Audio_Send(lua_GetGlobalSound(engine_lua, TR_AUDIO_SOUND_GLOBALID_MENUOPEN));
                    mRingRadius = 0.0f;
                    mVerticalOffset = mBaseRingRadius;
                    mCurrentItemsType = mNextItemsType;
                    updateCurrentRing();
                }
                mRingRadius = mBaseRingRadius * (mRingTime - mRingRotatePeriod) / mRingRotatePeriod;
                mVerticalOffset -= mBaseRingRadius * time / mRingRotatePeriod;
                mRingAngle -= 180.0f * time / mRingRotatePeriod;
            }
            else
            {
                mCurrentState = INVENTORY_IDLE;
                mRingAngle = 0.0f;
                mVerticalOffset = 0.0f;
            }
            break;

        case INVENTORY_DOWN:
            mCurrentState = INVENTORY_DOWN;
            mRingTime += time;
            if(mRingTime < mRingRotatePeriod)
            {
                restoreItemAngle(time);
                mRingRadius = mBaseRingRadius * (mRingRotatePeriod - mRingTime) / mRingRotatePeriod;
                mVerticalOffset = mBaseRingRadius * mRingTime / mRingRotatePeriod;
                mRingAngle += 180.0f * time / mRingRotatePeriod;
            }
            else if(mRingTime < 2.0f * mRingRotatePeriod)
            {
                if(mRingTime - time <= mRingRotatePeriod)
                {
                    //Audio_Send(lua_GetGlobalSound(engine_lua, TR_AUDIO_SOUND_GLOBALID_MENUOPEN));
                    mRingRadius = 0.0f;
                    mVerticalOffset = -mBaseRingRadius;
                    mCurrentItemsType = mNextItemsType;
                    updateCurrentRing();
                }
                mRingRadius = mBaseRingRadius * (mRingTime - mRingRotatePeriod) / mRingRotatePeriod;
                mVerticalOffset += mBaseRingRadius * time / mRingRotatePeriod;
                mRingAngle -= 180.0f * time / mRingRotatePeriod;
            }
            else
            {
                mCurrentState = INVENTORY_IDLE;
                mRingAngle = 0.0f;
                mVerticalOffset = 0.0f;
            }
            break;

        case INVENTORY_OPENING:
            mRingTime += time;
            mRingRadius = mBaseRingRadius * mRingTime / mRingRotatePeriod;
            mRingAngle -= 180.0f * time / mRingRotatePeriod;
            mRingVerticalAngle -= 180.0f * time / mRingRotatePeriod;
            if(mRingTime >= mRingRotatePeriod)
            {
                mCurrentState = INVENTORY_IDLE;
                m_command = NONE;
                mRingVerticalAngle = 0;

                mRingRadius = mBaseRingRadius;
                mRingTime = 0.0f;
                mRingAngle = 0.0f;
                mVerticalOffset = 0.0f;
                setTitle(GUI_MENU_ITEMTYPE_SUPPLY);
            }
            break;

        case INVENTORY_CLOSING:
            Gui_SetCurrentMenu(NULL);
            mRingTime += time;
            mRingRadius = mBaseRingRadius * (mRingRotatePeriod - mRingTime) / mRingRotatePeriod;
            mRingAngle += 180.0f * time / mRingRotatePeriod;
            mRingVerticalAngle += 180.0f * time / mRingRotatePeriod;
            if(mRingTime >= mRingRotatePeriod)
            {
                mCurrentState = INVENTORY_DISABLED;
                m_command = NONE;
                mRingVerticalAngle = 180.0f;
                mRingTime = 0.0f;
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
    for(inventory_node_p i = (mInventory) ? (*mInventory) : (NULL); i; i = i->next)
    {
        base_item_p bi = World_GetBaseItemByID(i->id);
        if(bi && (bi->type == mCurrentItemsType))
        {
            if(ring_item_index == mSelectedItem)
            {
                Item_Frame(bi->bf, 0.0f);
                if(mCurrentState == INVENTORY_ACTIVATING)
                {
                    restoreItemAngle(time);
                    m_current_scale += time * 10.0f;
                    m_item_offset_z += time * 90.0f;
                    m_current_scale = (m_current_scale > 2.0) ? (4.0f) : (m_current_scale);
                    if((m_item_angle_z == 0.0f) && (m_current_scale >= 2.0f))
                    {
                        m_item_offset_z = 180.0f;
                        mItemTime = 0.0f;
                        mCurrentState = INVENTORY_ACTIVATED;
                    }
                    m_command = NONE;
                }
                else if(mCurrentState == INVENTORY_DEACTIVATING)
                {
                    restoreItemAngle(time);
                    m_current_scale -= time * 10.0f;
                    m_item_offset_z -= time * 90.0f;
                    m_current_scale = (m_current_scale < 1.0) ? (1.0f) : (m_current_scale);
                    if((m_item_angle_z == 0.0f) && (m_current_scale <= 1.0f))
                    {
                        mItemTime = 0.0f;
                        m_item_offset_z = 0.0f;
                        mCurrentState = INVENTORY_IDLE;
                    }
                    m_command = NONE;
                }
                else if(mCurrentState == INVENTORY_ACTIVATED)
                {
                    if(bi->id == ITEM_PASSPORT)
                    {
                        handlePassport(bi, time / 2.0f);
                    }
                    else if(m_command == ACTIVATE)
                    {
                        if(0 < Item_Use(mInventory, bi->id, mOwnerId))
                        {
                            m_command = CLOSE;
                            mCurrentState = INVENTORY_DEACTIVATING;
                        }
                        else
                        {
                            m_command = NONE;
                            mCurrentState = INVENTORY_DEACTIVATING;
                        }
                    }
                }
            }
            else
            {
                Anim_SetAnimation(&bi->bf->animations, 0, 0);
                Item_Frame(bi->bf, 0.0f);
            }
            ring_item_index++;
        }
    }
}

void gui_InventoryManager::handlePassport(struct base_item_s *bi, float time)
{
    switch(m_menu_mode)
    {
    case 0:  // enter menu
        if(m_current_menu)
        {
            Gui_DeleteObjects(m_current_menu);
            m_current_menu = NULL;
        }
        Anim_IncTime(&bi->bf->animations, time);
        if(bi->bf->animations.current_frame >= 14)
        {
            Anim_SetAnimation(&bi->bf->animations, 0, 14);
            m_menu_mode = 1;
        }
        m_command = NONE;
        break;

    case 1:  // load game
        if(bi->bf->animations.current_frame > 14)
        {
            Anim_IncTime(&bi->bf->animations, -time);
            m_command = NONE;
            break;
        }
        else if(bi->bf->animations.current_frame < 14)
        {
            Anim_IncTime(&bi->bf->animations, time);
            m_command = NONE;
            break;
        }

        if(!m_current_menu)
        {
            m_current_menu = Gui_BuildLoadGameMenu();
        }

        if(m_command == CLOSE)
        {
            m_menu_mode = 4;
            m_command = NONE;
        }
        else if(m_command == RIGHT)
        {
            m_command = NONE;
            m_menu_mode = 2;
            if(m_current_menu)
            {
                Gui_DeleteObjects(m_current_menu);
                m_current_menu = NULL;
            }
        }
        break;

    case 2:  // save game
        if(bi->bf->animations.current_frame > 19)
        {
            Anim_IncTime(&bi->bf->animations, -time);
            m_command = NONE;
            break;
        }
        else if(bi->bf->animations.current_frame < 19)
        {
            Anim_IncTime(&bi->bf->animations, time);
            m_command = NONE;
            break;
        }

        if(!m_current_menu)
        {
            m_current_menu = Gui_BuildSaveGameMenu();
        }
        
        if(m_command == CLOSE)
        {
            m_menu_mode = 4;
        }
        else if(m_command == RIGHT)
        {
            m_menu_mode = 3;
            m_command = NONE;
            if(m_current_menu)
            {
                Gui_DeleteObjects(m_current_menu);
                m_current_menu = NULL;
            }
        }
        else if(m_command == LEFT)
        {
            m_menu_mode = 1;
            m_command = NONE;
            if(m_current_menu)
            {
                Gui_DeleteObjects(m_current_menu);
                m_current_menu = NULL;
            }
        }
        break;

    case 3:  // new game
        if(bi->bf->animations.current_frame > 24)
        {
            Anim_IncTime(&bi->bf->animations, -time);
            m_command = NONE;
            break;
        }
        else if(bi->bf->animations.current_frame < 24)
        {
            Anim_IncTime(&bi->bf->animations, time);
            m_command = NONE;
            break;
        }

        if(!m_current_menu)
        {
            m_current_menu = Gui_BuildNewGameMenu();
        }
        
        if(m_command == CLOSE)
        {
            m_menu_mode = 4;
        }
        else if(m_command == LEFT)
        {
            m_menu_mode = 2;
            m_command = NONE;
            if(m_current_menu)
            {
                Gui_DeleteObjects(m_current_menu);
                m_current_menu = NULL;
            }
        }
        break;

    case 4:  // leave menu
        m_command = NONE;
        Gui_SetCurrentMenu(NULL);
        if(m_current_menu)
        {
            Gui_DeleteObjects(m_current_menu);
            m_current_menu = NULL;
        }
        Anim_IncTime(&bi->bf->animations, time);
        if((bi->bf->animations.frame_changing_state == SS_CHANGING_END_ANIM))
        {
            Anim_SetAnimation(&bi->bf->animations, 0, 0);
            m_command = NONE;
            mCurrentState = INVENTORY_DEACTIVATING;
            m_menu_mode = 0;
        }
        break;
    }

    SSBoneFrame_Update(bi->bf, 0);
    Gui_SetCurrentMenu(m_current_menu);
    if(m_current_menu)
    {
        if(m_command == UP)
        {
            Gui_ListSaves(m_current_menu, 1);
        }
        else if(m_command == DOWN)
        {
            Gui_ListSaves(m_current_menu, -1);
        }
        else if(m_command == ACTIVATE)
        {
            gui_object_p obj = Gui_ListSaves(m_current_menu, 0);
            if(obj && obj->text)
            {
                Gui_SetCurrentMenu(NULL);
                if(m_menu_mode == 1)
                {
                    mInventory = NULL;
                    Game_Load(obj->text);
                }
                else if(m_menu_mode == 2)
                {
                    m_menu_mode = 4;
                    Game_Save(obj->text);
                }
                else if(m_menu_mode == 3)
                {
                    mInventory = NULL;
                    Gameflow_SetGame((int)obj->data, 1);
                }
                Gui_DeleteObjects(m_current_menu);
                m_current_menu = NULL;
            }
        }
        else if(m_command == CLOSE)
        {
            Gui_SetCurrentMenu(NULL);
            m_menu_mode = 4;
        }
        m_command = NONE;
    }
}

void gui_InventoryManager::render()
{
    if((mCurrentState != INVENTORY_DISABLED) && mInventory && *mInventory)
    {
        float matrix[16], offset[3], ang, scale;
        int ring_item_index = 0;
        mLabel_Title.x = screen_info.w / 2;
        mLabel_Title.y = screen_info.h - 30;
        mLabel_ItemName.x = screen_info.w / 2;
        if(mCurrentItemsCount == 0)
        {
            strncpy(mLabel_ItemName_text, "No items", GUI_LINE_DEFAULTSIZE);
            return;
        }

        for(inventory_node_p i = *mInventory; i; i = i->next)
        {
            base_item_p bi = World_GetBaseItemByID(i->id);
            if(bi && (bi->type == mCurrentItemsType))
            {
                Mat4_E_macro(matrix);
                matrix[12 + 2] = - mBaseRingRadius * 2.0f;
                ang = (25.0f + mRingVerticalAngle) * M_PI / 180.0f;
                Mat4_RotateX_SinCos(matrix, sinf(ang), cosf(ang));
                ang = (mRingAngleStep * (-mSelectedItem + ring_item_index) + mRingAngle) * M_PI / 180.0f;
                Mat4_RotateY_SinCos(matrix, sinf(ang), cosf(ang));
                offset[0] = 0.0f;
                offset[1] = mVerticalOffset;
                offset[2] = mRingRadius;
                Mat4_Translate(matrix, offset);
                Mat4_RotateX_SinCos(matrix,-1.0f, 0.0f);  //-90.0
                Mat4_RotateZ_SinCos(matrix, 1.0f, 0.0f);  //90.0
                if(ring_item_index == mSelectedItem)
                {
                    scale = 0.7f * m_current_scale;
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
                    ang = M_PI_2 + M_PI * m_item_angle_z / 180.0f - ang;
                    Mat4_RotateZ_SinCos(matrix, sinf(ang), cosf(ang));
                    ang = M_PI * m_item_angle_x / 180.0f;
                    Mat4_RotateX_SinCos(matrix, sinf(ang), cosf(ang));
                    offset[0] = 0.0f;
                    offset[1] = 0.0f;
                    offset[2] = m_item_offset_z;
                    Mat4_Translate(matrix, offset);
                }
                else
                {
                    scale = 0.7f;
                    ang = M_PI_2 - ang;
                    Mat4_RotateZ_SinCos(matrix, sinf(ang), cosf(ang));
                }
                offset[0] = -0.5f * bi->bf->centre[0];
                offset[1] = -0.5f * bi->bf->centre[1];
                offset[2] = -0.5f * bi->bf->centre[2];
                Mat4_Translate(matrix, offset);
                Mat4_Scale(matrix, scale, scale, scale);
                Gui_RenderItem(bi->bf, 0.0f, matrix);
                ring_item_index++;
            }
        }
    }
}

void Gui_DrawInventory(float time)
{
    main_inventory_manager->frame(time);
    if(!main_inventory_manager->isEnabled())
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
    SetRot(0, 0);
    SetSize(1.0f);
    SetRotateTime(1000.0f);

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

            mCurrRotX = (mCurrRotX > 360.0f) ? (mCurrRotX - 360.0f) : (mCurrRotX);
            //mCurrRotY = (mCurrRotY > 360.0f) ? (mCurrRotY - 360.0f) : (mCurrRotY);
        }

        float step = 0;

        if(mCurrTime == 0)
        {
            step = (mCurrPosX - mEndPosX) * (time * 4.0f);
            step = (step <= 0.5f) ? (0.5f) : (step);

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
            step = (mCurrPosX - mEndPosX) * (time * 4.0f);
            step = (step <= 0.5f) ? (0.5f) : (step);

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
    mCurrTime = 0.0f;
    mCurrRotX = 0.0f;
    mCurrRotY = 0.0f;

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
            int curr_anim = item->bf->animations.prev_animation;
            int next_anim = item->bf->animations.current_animation;
            int curr_frame = item->bf->animations.prev_frame;
            int next_frame = item->bf->animations.current_frame;
            float time = item->bf->animations.frame_time;
            float ang = (mCurrRotX + mRotX) * M_PI / 180.0f;
            float matrix[16];
            Mat4_E_macro(matrix);

            matrix[12 + 0] = mCurrPosX;
            matrix[12 + 1] = mPosY;
            matrix[12 + 2] = -2048.0f;

            Mat4_RotateY_SinCos(matrix, sinf(ang), cosf(ang));
            ang = (mCurrRotY + mRotY) * M_PI / 180.0f;
            Mat4_RotateX_SinCos(matrix, sinf(ang), cosf(ang));

            Anim_SetAnimation(&item->bf->animations, 0, 0);
            SSBoneFrame_Update(item->bf, 0.0f);
            Gui_RenderItem(item->bf, mSize, matrix);

            item->bf->animations.prev_animation = curr_anim;
            item->bf->animations.current_animation = next_anim;
            item->bf->animations.prev_frame = curr_frame;
            item->bf->animations.current_frame = next_frame;
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
