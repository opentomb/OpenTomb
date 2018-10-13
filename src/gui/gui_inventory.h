
#ifndef ENGINE_GUI_INVENTORY_H
#define ENGINE_GUI_INVENTORY_H

#include <stdint.h>
#include "../core/gl_text.h"
#include "../core/gui/gui_obj.h"

struct inventory_node_s;


#define GUI_MENU_ITEMTYPE_SYSTEM 0
#define GUI_MENU_ITEMTYPE_SUPPLY 1
#define GUI_MENU_ITEMTYPE_QUEST  2

// Offscreen divider specifies how far item notifier will be placed from
// the final slide position. Usually it's enough to be 1/8 of the screen
// width, but if you want to increase or decrease notifier size, you must
// change this value properly.

#define GUI_NOTIFIER_OFFSCREEN_DIVIDER 8.0f

// Notifier show time is a time notifier stays on screen (excluding slide
// effect). Maybe it's better to move it to script later.

#define GUI_NOTIFIER_SHOWTIME 2.0f

class gui_ItemNotifier
{
public:
    gui_ItemNotifier();

    void    Start(int item, float time);
    void    Reset();
    void    Animate(float time);
    void    Draw();

    void    SetRot(float X, float Y);
    void    SetSize(float size);
    void    SetRotateTime(float time);

private:
    bool    m_active;
    int     m_item;

    float   m_pos_y;
    float   m_start_pos_x;
    float   m_end_pos_x;
    float   m_curr_pos_x;

    float   m_rot_x;
    float   m_rot_y;
    float   m_curr_rot_x;
    float   m_curr_rot_y;

    float   m_size;

    float   m_show_time;
    float   m_curr_time;
    float   m_rotate_time;
};

void Gui_InitNotifier();

/**
 * Inventory rendering / manipulation functions
 */
void Item_Frame(struct ss_bone_frame_s *bf, float time);
void Gui_RenderItem(struct ss_bone_frame_s *bf, float size, const float *mvMatrix);
/*
 * Inventory renderer class
 */
class gui_InventoryManager
{
enum inventoryState
{
    INVENTORY_DISABLED = 0,
    INVENTORY_IDLE,
    INVENTORY_OPENING,
    INVENTORY_CLOSING,
    INVENTORY_R_LEFT,
    INVENTORY_R_RIGHT,
    INVENTORY_UP,
    INVENTORY_DOWN,
    INVENTORY_ACTIVATING,
    INVENTORY_DEACTIVATING,
    INVENTORY_ACTIVATED
};
    
public:   
    gui_InventoryManager();
   ~gui_InventoryManager();

   bool isEnabled()
   {
       return m_current_state != INVENTORY_DISABLED;
   }

   bool isIdle()
   {
       return (m_current_state == INVENTORY_IDLE) || (m_current_state == INVENTORY_ACTIVATED);
   }
   
    void send(int cmd);

    int getItemsType()
    {
        return m_current_items_type;
    }

    void setInventory(struct inventory_node_s **i, uint32_t owner_id);
    void setTitle(int items_type);
    void frame(float time);
    void render();

    gl_text_line_t              m_label_title;
    char                        m_label_title_text[GUI_LINE_DEFAULTSIZE];
    gl_text_line_t              m_label_item_name;
    char                        m_label_item_name_text[GUI_LINE_DEFAULTSIZE];

private:
    int                         m_menu_mode;
    gui_object_p                m_current_menu;
    struct inventory_node_s   **m_inventory;
    uint32_t                    m_owner_id;
    int                         m_current_state;
    int                         m_command;

    int                         m_current_items_type;
    int                         m_next_items_type;
    int                         m_current_items_count;
    int                         m_selected_item;
    
    float                       m_ring_rotate_period;
    float                       m_ring_time;
    float                       m_ring_angle;
    float                       m_ring_vertical_angle;
    float                       m_ring_angle_step;
    float                       m_base_ring_radius;
    float                       m_ring_radius;
    float                       m_vertical_offset;

    float                       m_item_rotate_period;
    float                       m_item_time;
    float                       m_item_angle_z;
    float                       m_item_angle_x;
    float                       m_item_offset_z;
    float                       m_current_scale;

    int getItemElementsCountByType(int type);
    void updateCurrentRing();
    void frameStates(float time);
    void frameItems(float time);
    void handlePassport(struct base_item_s *bi, float time);
    void handleCompass(struct base_item_s *bi, float time);
    void handleControls(struct base_item_s *bi, float time);
    void restoreItemAngle(float time);
};


extern gui_InventoryManager  *main_inventory_manager;

/**
 * Item notifier functions.
 */
void Gui_NotifierStart(int item);
void Gui_NotifierStop();

/**
 * General GUI drawing routines.
 */
void Gui_DrawInventory(float time);
void Gui_DrawNotifier(float time);

#endif
