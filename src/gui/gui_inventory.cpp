
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
                Gameflow_Send(GF_OP_STARTFMV, 1);
                return Gameflow_SetGame(Gameflow_GetCurrentGameID(), 0);

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
    m_current_state             = INVENTORY_DISABLED;
    m_command                   = GUI_COMMAND_NONE;
    m_current_items_type        = GUI_MENU_ITEMTYPE_SYSTEM;
    m_next_items_type           = GUI_MENU_ITEMTYPE_SYSTEM;
    m_current_items_count       = 0;
    m_selected_item             = 0;

    m_item_time                 = 0.0f;
    m_ring_rotate_period        = 0.5f;
    m_ring_time                 = 0.0f;
    m_ring_angle                = 0.0f;
    m_ring_vertical_angle       = 0.0f;
    m_ring_angle_step           = 0.0f;
    m_base_ring_radius          = 600.0f;
    m_ring_radius               = 600.0f;
    m_vertical_offset           = 0.0f;

    m_item_rotate_period        = 4.0f;
    m_item_angle_z              = 0.0f;
    m_item_angle_x              = 0.0f;
    m_current_scale             = 1.0f;
    m_item_offset_z             = 0.0f;

    m_current_menu              = NULL;
    m_menu_mode                 = 0;
    m_inventory                 = NULL;
    m_owner_id                  = ENTITY_ID_NONE;

    m_label_title.x             = 0.0f;
    m_label_title.y             = 0.0f;
    m_label_title.line_width    = -1.0f;
    m_label_title.x_align       = GLTEXT_ALIGN_CENTER;
    m_label_title.y_align       = GLTEXT_ALIGN_TOP;
    m_label_title.next          = NULL;
    m_label_title.prev          = NULL;

    m_label_title.font_id       = FONT_PRIMARY;
    m_label_title.style_id      = FONTSTYLE_MENU_TITLE;
    m_label_title.text          = m_label_title_text;
    m_label_title_text[0]       = 0;
    m_label_title.show          = 0;

    m_label_item_name.x         = 0.0f;
    m_label_item_name.y         = 50.0f;
    m_label_item_name.line_width= -1.0f;
    m_label_item_name.x_align   = GLTEXT_ALIGN_CENTER;
    m_label_item_name.y_align   = GLTEXT_ALIGN_BOTTOM;
    m_label_item_name.next      = NULL;
    m_label_item_name.prev      = NULL;

    m_label_item_name.font_id   = FONT_PRIMARY;
    m_label_item_name.style_id  = FONTSTYLE_MENU_CONTENT;
    m_label_item_name.text      = m_label_item_name_text;
    m_label_item_name_text[0]   = 0;
    m_label_item_name.show      = 0;

    GLText_AddLine(&m_label_item_name);
    GLText_AddLine(&m_label_title);
}

gui_InventoryManager::~gui_InventoryManager()
{
    m_current_state = INVENTORY_DISABLED;
    m_command = GUI_COMMAND_CLOSE;
    m_inventory = NULL;

    m_label_item_name.show = 0;
    GLText_DeleteLine(&m_label_item_name);

    m_label_title.show = 0;
    GLText_DeleteLine(&m_label_title);

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
    for(inventory_node_p i = *m_inventory; i; i = i->next)
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
            m_item_angle_z -= 180.0f * time / m_ring_rotate_period;
            if(m_item_angle_z < 0.0f)
            {
                m_item_angle_z = 0.0f;
            }
        }
        else
        {
            m_item_angle_z += 180.0f * time / m_ring_rotate_period;
            if(m_item_angle_z >= 360.0f)
            {
                m_item_angle_z = 0.0f;
            }
        }
    }
}

void gui_InventoryManager::send(int cmd)
{
    m_command = cmd;
}

void gui_InventoryManager::setInventory(struct inventory_node_s **i, uint32_t owner_id)
{
    m_inventory = i;
    m_owner_id = owner_id;
    m_current_state = INVENTORY_DISABLED;
    m_command = GUI_COMMAND_NONE;
    m_label_item_name.show = 0;
    m_label_title.show = 0;
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

    Script_GetString(engine_lua, string_index, GUI_LINE_DEFAULTSIZE, m_label_title_text);
}

void gui_InventoryManager::updateCurrentRing()
{
    if(m_inventory && *m_inventory)
    {
        m_current_items_count = this->getItemElementsCountByType(m_current_items_type);
        setTitle(m_current_items_type);
        if(m_current_items_count)
        {
            m_ring_angle_step = 360.0f / m_current_items_count;
            m_selected_item %= m_current_items_count;
        }
        m_ring_angle = 180.0f;
    }
}

void gui_InventoryManager::frame(float time)
{
    if(m_inventory && *m_inventory)
    {
        this->frameStates(time);
        this->frameItems(time);
    }
    else
    {
        m_current_state = INVENTORY_DISABLED;
        m_command = GUI_COMMAND_CLOSE;
    }
}

void gui_InventoryManager::frameStates(float time)
{
    switch(m_current_state)
    {
        case INVENTORY_R_LEFT:
            m_command = GUI_COMMAND_NONE;
            m_ring_time += time;
            m_ring_angle = m_ring_angle_step * m_ring_time / m_ring_rotate_period;
            if(m_ring_time >= m_ring_rotate_period)
            {
                m_ring_time = 0.0f;
                m_ring_angle = 0.0f;
                m_current_state = INVENTORY_IDLE;
                m_selected_item--;
                if(m_selected_item < 0)
                {
                    m_selected_item = m_current_items_count - 1;
                }
            }
            restoreItemAngle(time);
            break;

        case INVENTORY_R_RIGHT:
            m_command = GUI_COMMAND_NONE;
            m_ring_time += time;
            m_ring_angle = -m_ring_angle_step * m_ring_time / m_ring_rotate_period;
            if(m_ring_time >= m_ring_rotate_period)
            {
                m_ring_time = 0.0f;
                m_ring_angle = 0.0f;
                m_current_state = INVENTORY_IDLE;
                m_selected_item++;
                if(m_selected_item >= m_current_items_count)
                {
                    m_selected_item = 0;
                }
            }
            restoreItemAngle(time);
            break;

        case INVENTORY_IDLE:
            m_ring_time = 0.0f;
            switch(m_command)
            {
                default:
                case GUI_COMMAND_NONE:
                    m_item_time += time;
                    m_item_angle_z = 360.0f * m_item_time / m_item_rotate_period;
                    if(m_item_time >= m_item_rotate_period)
                    {
                        m_item_time = 0.0f;
                        m_item_angle_z = 0.0f;
                    }
                    m_label_item_name.show = 1;
                    m_label_title.show = 1;
                    break;

                case GUI_COMMAND_ACTIVATE:
                    m_current_state = INVENTORY_ACTIVATING;
                    m_command = GUI_COMMAND_NONE;
                    break;

                case GUI_COMMAND_CLOSE:
                    Audio_Send(Script_GetGlobalSound(engine_lua, TR_AUDIO_SOUND_GLOBALID_MENUCLOSE));
                    m_label_item_name.show = 0;
                    m_label_title.show = 0;
                    m_current_state = INVENTORY_CLOSING;
                    break;

                case GUI_COMMAND_LEFT:
                case GUI_COMMAND_RIGHT:
                    if(m_current_items_count >= 1)
                    {
                        Audio_Send(TR_AUDIO_SOUND_MENUROTATE);
                        m_label_item_name.show = 0;
                        m_current_state = (m_command == GUI_COMMAND_LEFT) ? (INVENTORY_R_LEFT) : (INVENTORY_R_RIGHT);
                        m_item_time = 0.0f;
                    }
                    break;

                case GUI_COMMAND_UP:
                    if(m_current_items_type < GUI_MENU_ITEMTYPE_QUEST)
                    {
                        //Audio_Send(Script_GetGlobalSound(engine_lua, TR_AUDIO_SOUND_GLOBALID_MENUCLOSE));
                        m_next_items_type = m_current_items_type + 1;
                        m_current_state = INVENTORY_UP;
                        m_ring_time = 0.0f;
                    }
                    m_command = GUI_COMMAND_NONE;
                    m_label_item_name.show = 0;
                    m_label_title.show = 0;
                    break;

                case GUI_COMMAND_DOWN:
                    if(m_current_items_type > 0)
                    {
                        //Audio_Send(Script_GetGlobalSound(engine_lua, TR_AUDIO_SOUND_GLOBALID_MENUCLOSE));
                        m_next_items_type = m_current_items_type - 1;
                        m_current_state = INVENTORY_DOWN;
                        m_ring_time = 0.0f;
                    }
                    m_command = GUI_COMMAND_NONE;
                    m_label_item_name.show = 0;
                    m_label_title.show = 0;
                    break;
            };
            break;

        case INVENTORY_DISABLED:
            if(m_command == GUI_COMMAND_OPEN)
            {
                Audio_Send(Script_GetGlobalSound(engine_lua, TR_AUDIO_SOUND_GLOBALID_MENUOPEN));
                for(inventory_node_p i = *m_inventory; i; i = i->next)
                {
                    base_item_p bi = World_GetBaseItemByID(i->id);
                    if(bi)
                    {
                        if(bi->type == GUI_MENU_ITEMTYPE_SUPPLY)
                        {
                            m_current_items_type = GUI_MENU_ITEMTYPE_SUPPLY;
                            break;
                        }
                        else
                        {
                            m_current_items_type = bi->type;
                        }
                    }
                }
                this->updateCurrentRing();
                m_item_time = 0.0f;
                m_item_offset_z = 0.0f;
                m_current_scale = 1.0f;
                m_current_state = INVENTORY_OPENING;
                m_ring_angle = 180.0f;
                m_ring_vertical_angle = 180.0f;
            }
            break;

        case INVENTORY_UP:
            m_current_state = INVENTORY_UP;
            m_ring_time += time;
            if(m_ring_time < m_ring_rotate_period)
            {
                restoreItemAngle(time);
                m_ring_radius = m_base_ring_radius * (m_ring_rotate_period - m_ring_time) / m_ring_rotate_period;
                m_vertical_offset = - m_base_ring_radius * m_ring_time / m_ring_rotate_period;
                m_ring_angle += 180.0f * time / m_ring_rotate_period;
            }
            else if(m_ring_time < 2.0f * m_ring_rotate_period)
            {
                if(m_ring_time - time <= m_ring_rotate_period)
                {
                    //Audio_Send(lua_GetGlobalSound(engine_lua, TR_AUDIO_SOUND_GLOBALID_MENUOPEN));
                    m_ring_radius = 0.0f;
                    m_vertical_offset = m_base_ring_radius;
                    m_current_items_type = m_next_items_type;
                    updateCurrentRing();
                }
                m_ring_radius = m_base_ring_radius * (m_ring_time - m_ring_rotate_period) / m_ring_rotate_period;
                m_vertical_offset -= m_base_ring_radius * time / m_ring_rotate_period;
                m_ring_angle -= 180.0f * time / m_ring_rotate_period;
            }
            else
            {
                m_current_state = INVENTORY_IDLE;
                m_ring_angle = 0.0f;
                m_vertical_offset = 0.0f;
            }
            break;

        case INVENTORY_DOWN:
            m_current_state = INVENTORY_DOWN;
            m_ring_time += time;
            if(m_ring_time < m_ring_rotate_period)
            {
                restoreItemAngle(time);
                m_ring_radius = m_base_ring_radius * (m_ring_rotate_period - m_ring_time) / m_ring_rotate_period;
                m_vertical_offset = m_base_ring_radius * m_ring_time / m_ring_rotate_period;
                m_ring_angle += 180.0f * time / m_ring_rotate_period;
            }
            else if(m_ring_time < 2.0f * m_ring_rotate_period)
            {
                if(m_ring_time - time <= m_ring_rotate_period)
                {
                    //Audio_Send(lua_GetGlobalSound(engine_lua, TR_AUDIO_SOUND_GLOBALID_MENUOPEN));
                    m_ring_radius = 0.0f;
                    m_vertical_offset = -m_base_ring_radius;
                    m_current_items_type = m_next_items_type;
                    updateCurrentRing();
                }
                m_ring_radius = m_base_ring_radius * (m_ring_time - m_ring_rotate_period) / m_ring_rotate_period;
                m_vertical_offset += m_base_ring_radius * time / m_ring_rotate_period;
                m_ring_angle -= 180.0f * time / m_ring_rotate_period;
            }
            else
            {
                m_current_state = INVENTORY_IDLE;
                m_ring_angle = 0.0f;
                m_vertical_offset = 0.0f;
            }
            break;

        case INVENTORY_OPENING:
            m_ring_time += time;
            m_ring_radius = m_base_ring_radius * m_ring_time / m_ring_rotate_period;
            m_ring_angle -= 180.0f * time / m_ring_rotate_period;
            m_ring_vertical_angle -= 180.0f * time / m_ring_rotate_period;
            if(m_ring_time >= m_ring_rotate_period)
            {
                m_current_state = INVENTORY_IDLE;
                m_command = GUI_COMMAND_NONE;
                m_ring_vertical_angle = 0;

                m_ring_radius = m_base_ring_radius;
                m_ring_time = 0.0f;
                m_ring_angle = 0.0f;
                m_vertical_offset = 0.0f;
                setTitle(GUI_MENU_ITEMTYPE_SUPPLY);
            }
            break;

        case INVENTORY_CLOSING:
            Gui_SetCurrentMenu(NULL);
            m_ring_time += time;
            m_ring_radius = m_base_ring_radius * (m_ring_rotate_period - m_ring_time) / m_ring_rotate_period;
            m_ring_angle += 180.0f * time / m_ring_rotate_period;
            m_ring_vertical_angle += 180.0f * time / m_ring_rotate_period;
            if(m_ring_time >= m_ring_rotate_period)
            {
                m_current_state = INVENTORY_DISABLED;
                m_command = GUI_COMMAND_NONE;
                m_ring_vertical_angle = 180.0f;
                m_ring_time = 0.0f;
                m_label_title.show = 0;
                m_ring_radius = m_base_ring_radius;
                m_current_items_type = 1;
            }
            break;
    }
}

void gui_InventoryManager::frameItems(float time)
{
    int ring_item_index = 0;
    for(inventory_node_p i = (m_inventory) ? (*m_inventory) : (NULL); m_inventory && i; i = i->next)
    {
        base_item_p bi = World_GetBaseItemByID(i->id);
        if(bi && (bi->type == m_current_items_type))
        {
            if(ring_item_index == m_selected_item)
            {
                Item_Frame(bi->bf, 0.0f);
                if(m_current_state == INVENTORY_ACTIVATING)
                {
                    restoreItemAngle(time);
                    m_current_scale += time * 10.0f;
                    m_item_offset_z += time * 90.0f;
                    m_current_scale = (m_current_scale > 2.0) ? (4.0f) : (m_current_scale);
                    if((m_item_angle_z == 0.0f) && (m_current_scale >= 2.0f))
                    {
                        m_item_offset_z = 180.0f;
                        m_item_time = 0.0f;
                        m_current_state = INVENTORY_ACTIVATED;
                    }
                    m_command = GUI_COMMAND_NONE;
                }
                else if(m_current_state == INVENTORY_DEACTIVATING)
                {
                    restoreItemAngle(time);
                    m_current_scale -= time * 10.0f;
                    m_item_offset_z -= time * 90.0f;
                    m_current_scale = (m_current_scale < 1.0) ? (1.0f) : (m_current_scale);
                    if((m_item_angle_z == 0.0f) && (m_current_scale <= 1.0f))
                    {
                        m_item_time = 0.0f;
                        m_item_offset_z = 0.0f;
                        m_current_state = INVENTORY_IDLE;
                    }
                    m_command = GUI_COMMAND_NONE;
                }
                else if(m_current_state == INVENTORY_ACTIVATED)
                {
                    if(bi->id == ITEM_PASSPORT)
                    {
                        handlePassport(bi, time / 2.0f);
                    }
                    else if(bi->id == ITEM_COMPASS)
                    {
                        handleCompass(bi, time / 2.0f);
                    }
                    else if(bi->id == ITEM_CONTROLS)
                    {
                        handleControls(bi, time / 2.0f);
                    }
                    else if(m_command == GUI_COMMAND_ACTIVATE)
                    {
                        if(0 < Item_Use(m_inventory, bi->id, m_owner_id))
                        {
                            m_command = GUI_COMMAND_CLOSE;
                            m_current_state = INVENTORY_DEACTIVATING;
                        }
                        else
                        {
                            m_command = GUI_COMMAND_NONE;
                            m_current_state = INVENTORY_DEACTIVATING;
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
        m_command = GUI_COMMAND_NONE;
        break;

    case 1:  // load game
        if(bi->bf->animations.current_frame > 14)
        {
            Anim_IncTime(&bi->bf->animations, -time);
            m_command = GUI_COMMAND_NONE;
            break;
        }
        else if(bi->bf->animations.current_frame < 14)
        {
            Anim_IncTime(&bi->bf->animations, time);
            m_command = GUI_COMMAND_NONE;
            break;
        }

        if(!m_current_menu)
        {
            m_current_menu = Gui_BuildLoadGameMenu();
        }

        if(m_command == GUI_COMMAND_CLOSE)
        {
            m_menu_mode = 4;
            m_command = GUI_COMMAND_NONE;
        }
        else if(m_command == GUI_COMMAND_RIGHT)
        {
            m_command = GUI_COMMAND_NONE;
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
            m_command = GUI_COMMAND_NONE;
            break;
        }
        else if(bi->bf->animations.current_frame < 19)
        {
            Anim_IncTime(&bi->bf->animations, time);
            m_command = GUI_COMMAND_NONE;
            break;
        }

        if(!m_current_menu)
        {
            m_current_menu = Gui_BuildSaveGameMenu();
        }

        if(m_command == GUI_COMMAND_CLOSE)
        {
            m_menu_mode = 4;
        }
        else if(m_command == GUI_COMMAND_RIGHT)
        {
            m_menu_mode = 3;
            m_command = GUI_COMMAND_NONE;
            if(m_current_menu)
            {
                Gui_DeleteObjects(m_current_menu);
                m_current_menu = NULL;
            }
        }
        else if(m_command == GUI_COMMAND_LEFT)
        {
            m_menu_mode = 1;
            m_command = GUI_COMMAND_NONE;
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
            m_command = GUI_COMMAND_NONE;
            break;
        }
        else if(bi->bf->animations.current_frame < 24)
        {
            Anim_IncTime(&bi->bf->animations, time);
            m_command = GUI_COMMAND_NONE;
            break;
        }

        if(!m_current_menu)
        {
            m_current_menu = Gui_BuildNewGameMenu();
        }

        if(m_command == GUI_COMMAND_CLOSE)
        {
            m_menu_mode = 4;
        }
        else if(m_command == GUI_COMMAND_LEFT)
        {
            m_menu_mode = 2;
            m_command = GUI_COMMAND_NONE;
            if(m_current_menu)
            {
                Gui_DeleteObjects(m_current_menu);
                m_current_menu = NULL;
            }
        }
        break;

    case 4:  // leave menu
        m_command = GUI_COMMAND_NONE;
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
            m_command = GUI_COMMAND_NONE;
            m_current_state = INVENTORY_DEACTIVATING;
            m_menu_mode = 0;
        }
        break;
    }

    SSBoneFrame_Update(bi->bf, 0);
    Gui_SetCurrentMenu(m_current_menu);

    if(m_current_menu && m_current_menu->handlers.do_command
      && m_current_menu->handlers.do_command(m_current_menu, m_command))
    {
        m_command = GUI_COMMAND_NONE;
    }

    if(m_command == GUI_COMMAND_CLOSE)
    {
        m_menu_mode = 4;
        Gui_SetCurrentMenu(NULL);
        Gui_DeleteObjects(m_current_menu);
        m_current_menu = NULL;
    }
}

void gui_InventoryManager::handleCompass(struct base_item_s *bi, float time)
{
    switch(m_menu_mode)
    {
    case 0:  // enter menu
        if(m_current_menu)
        {
            Gui_DeleteObjects(m_current_menu);
            m_current_menu = NULL;
        }
        if(m_command == GUI_COMMAND_CLOSE)
        {
            m_command = GUI_COMMAND_NONE;
            m_current_state = INVENTORY_DEACTIVATING;
            m_menu_mode = 0;
        }
        else if(m_command == GUI_COMMAND_ACTIVATE)
        {
            m_command = GUI_COMMAND_NONE;
            m_menu_mode = 1;
        }
        break;

    case 1:  // load game
        if(bi->bf->animations.current_frame < 10)
        {
            Anim_IncTime(&bi->bf->animations, time);
            if((bi->bf->animations.frame_changing_state != SS_CHANGING_END_ANIM))
            {
                m_command = GUI_COMMAND_NONE;
                break;
            }
        }

        if(!m_current_menu)
        {
            m_current_menu = Gui_BuildStatisticsMenu();
        }
        break;

    case 2:  // leave menu
        m_command = GUI_COMMAND_NONE;
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
            m_command = GUI_COMMAND_NONE;
            m_current_state = INVENTORY_DEACTIVATING;
            m_menu_mode = 0;
        }
        break;
    }

    SSBoneFrame_Update(bi->bf, 0);
    if(m_command == GUI_COMMAND_ACTIVATE)
    {
        SSBoneFrame_Update(bi->bf, 0);
    }
    Gui_SetCurrentMenu(m_current_menu);

    if(m_current_menu && m_current_menu->handlers.do_command
      && m_current_menu->handlers.do_command(m_current_menu, m_command))
    {
        m_command = GUI_COMMAND_NONE;
    }

    if(m_command == GUI_COMMAND_CLOSE)
    {
        m_menu_mode = 2;
        Gui_SetCurrentMenu(NULL);
        Gui_DeleteObjects(m_current_menu);
        m_current_menu = NULL;
    }
}

void gui_InventoryManager::handleControls(struct base_item_s *bi, float time)
{
    switch(m_menu_mode)
    {
    case 0:  // enter menu
        if(m_current_menu)
        {
            Gui_DeleteObjects(m_current_menu);
            m_current_menu = NULL;
        }
        if(m_command == GUI_COMMAND_CLOSE)
        {
            m_command = GUI_COMMAND_NONE;
            m_current_state = INVENTORY_DEACTIVATING;
            m_menu_mode = 0;
        }
        else if(m_command == GUI_COMMAND_ACTIVATE)
        {
            m_command = GUI_COMMAND_NONE;
            m_menu_mode = 1;
        }
        break;

    case 1:  // load game
        if(!m_current_menu)
        {
            m_current_menu = Gui_BuildControlsMenu();
        }
        break;

    case 2:  // leave menu
        m_command = GUI_COMMAND_NONE;
        Gui_SetCurrentMenu(NULL);
        if(m_current_menu)
        {
            Gui_DeleteObjects(m_current_menu);
            m_current_menu = NULL;
        }
        Anim_SetAnimation(&bi->bf->animations, 0, 0);
        m_command = GUI_COMMAND_NONE;
        m_current_state = INVENTORY_DEACTIVATING;
        m_menu_mode = 0;
        break;
    }

    SSBoneFrame_Update(bi->bf, 0);
    if(m_command == GUI_COMMAND_ACTIVATE)
    {
        SSBoneFrame_Update(bi->bf, 0);
    }
    Gui_SetCurrentMenu(m_current_menu);

    if(m_current_menu && m_current_menu->handlers.do_command
      && m_current_menu->handlers.do_command(m_current_menu, m_command))
    {
        m_command = GUI_COMMAND_NONE;
    }

    if(m_command == GUI_COMMAND_CLOSE)
    {
        m_menu_mode = 2;
        Gui_SetCurrentMenu(NULL);
        Gui_DeleteObjects(m_current_menu);
        m_current_menu = NULL;
    }
}

void gui_InventoryManager::render()
{
    if((m_current_state != INVENTORY_DISABLED) && m_inventory && *m_inventory)
    {
        float matrix[16], offset[3], ang, scale;
        int ring_item_index = 0;
        m_label_title.x = screen_info.w / 2;
        m_label_title.y = screen_info.h - 30;
        m_label_item_name.x = screen_info.w / 2;
        if(m_current_items_count == 0)
        {
            strncpy(m_label_item_name_text, "No items", GUI_LINE_DEFAULTSIZE);
            return;
        }

        for(inventory_node_p i = *m_inventory; i; i = i->next)
        {
            base_item_p bi = World_GetBaseItemByID(i->id);
            if(bi && (bi->type == m_current_items_type))
            {
                Mat4_E_macro(matrix);
                matrix[12 + 2] = - m_base_ring_radius * 2.0f;
                ang = (25.0f + m_ring_vertical_angle) * M_PI / 180.0f;
                Mat4_RotateX_SinCos(matrix, sinf(ang), cosf(ang));
                ang = (m_ring_angle_step * (-m_selected_item + ring_item_index) + m_ring_angle) * M_PI / 180.0f;
                Mat4_RotateY_SinCos(matrix, sinf(ang), cosf(ang));
                offset[0] = 0.0f;
                offset[1] = m_vertical_offset;
                offset[2] = m_ring_radius;
                Mat4_Translate(matrix, offset);
                Mat4_RotateX_SinCos(matrix,-1.0f, 0.0f);  //-90.0
                Mat4_RotateZ_SinCos(matrix, 1.0f, 0.0f);  //90.0
                if(ring_item_index == m_selected_item)
                {
                    scale = 0.7f * m_current_scale;
                    if(bi->name[0])
                    {
                        if(i->count == 1)
                        {
                            strncpy(m_label_item_name_text, bi->name, GUI_LINE_DEFAULTSIZE);
                        }
                        else
                        {
                            snprintf(m_label_item_name_text, GUI_LINE_DEFAULTSIZE, "%s (%d)", bi->name, i->count);
                        }
                    }
                    else
                    {
                        snprintf(m_label_item_name_text, GUI_LINE_DEFAULTSIZE, "ITEM_ID_%d (%d)", i->id, i->count);
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

    m_item   = 0;
    m_active = false;
}

void gui_ItemNotifier::Start(int item, float time)
{
    Reset();

    m_item     = item;
    m_show_time = time;
    m_active   = true;
}

void gui_ItemNotifier::Animate(float time)
{
    if(!m_active)
    {
        return;
    }
    else
    {
        if(m_rotate_time)
        {
            m_curr_rot_x += (time * m_rotate_time);
            //mCurrRotY += (time * mRotateTime);

            m_curr_rot_x = (m_curr_rot_x > 360.0f) ? (m_curr_rot_x - 360.0f) : (m_curr_rot_x);
            //mCurrRotY = (mCurrRotY > 360.0f) ? (mCurrRotY - 360.0f) : (mCurrRotY);
        }

        float step = 0;

        if(m_curr_time == 0)
        {
            step = (m_curr_pos_x - m_end_pos_x) * (time * 4.0f);
            step = (step <= 0.5f) ? (0.5f) : (step);

            m_curr_pos_x -= step;
            m_curr_pos_x  = (m_curr_pos_x < m_end_pos_x) ? (m_end_pos_x) : (m_curr_pos_x);

            if(m_curr_pos_x == m_end_pos_x)
                m_curr_time += time;
        }
        else if(m_curr_time < m_show_time)
        {
            m_curr_time += time;
        }
        else
        {
            step = (m_curr_pos_x - m_end_pos_x) * (time * 4.0f);
            step = (step <= 0.5f) ? (0.5f) : (step);

            m_curr_pos_x += step;
            m_curr_pos_x  = (m_curr_pos_x > m_start_pos_x) ? (m_start_pos_x) : (m_curr_pos_x);

            if(m_curr_pos_x == m_start_pos_x)
                Reset();
        }
    }
}

void gui_ItemNotifier::Reset()
{
    m_active = false;
    m_curr_time = 0.0f;
    m_curr_rot_x = 0.0f;
    m_curr_rot_y = 0.0f;

    m_end_pos_x = 0.85f * screen_info.w;
    m_pos_y    = 0.15f * screen_info.h;
    m_curr_pos_x = screen_info.w + ((float)screen_info.w / GUI_NOTIFIER_OFFSCREEN_DIVIDER * m_size);
    m_start_pos_x = m_curr_pos_x;    // Equalize current and start positions.
}

void gui_ItemNotifier::Draw()
{
    if(m_active)
    {
        base_item_p item = World_GetBaseItemByID(m_item);
        if(item)
        {
            int curr_anim = item->bf->animations.prev_animation;
            int next_anim = item->bf->animations.current_animation;
            int curr_frame = item->bf->animations.prev_frame;
            int next_frame = item->bf->animations.current_frame;
            float time = item->bf->animations.frame_time;
            float ang = (m_curr_rot_x + m_rot_x) * M_PI / 180.0f;
            float matrix[16];
            Mat4_E_macro(matrix);

            matrix[12 + 0] = m_curr_pos_x;
            matrix[12 + 1] = m_pos_y;
            matrix[12 + 2] = -2048.0f;

            Mat4_RotateY_SinCos(matrix, sinf(ang), cosf(ang));
            ang = (m_curr_rot_y + m_rot_y) * M_PI / 180.0f;
            Mat4_RotateX_SinCos(matrix, sinf(ang), cosf(ang));

            Anim_SetAnimation(&item->bf->animations, 0, 0);
            SSBoneFrame_Update(item->bf, 0.0f);
            Gui_RenderItem(item->bf, m_size, matrix);

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
    m_rot_x = X;
    m_rot_y = Y;
}

void gui_ItemNotifier::SetSize(float size)
{
    m_size = size;
}

void gui_ItemNotifier::SetRotateTime(float time)
{
    m_rotate_time = (1000.0f / time) * 360.0f;
}
