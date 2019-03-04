#ifndef ENGINE_INVENTORY_H
#define ENGINE_INVENTORY_H

//==================//
//   Type Section   //
//==================//

#define ITEM_TYPE_SYSTEM     0
#define ITEM_TYPE_AMMO       1
#define ITEM_TYPE_INVENTORY  2
#define ITEM_TYPE_QUEST      3

//==================//
//  System Section  //
//==================//

#define ITEM_COMPASS  1     // Aka Watch in TR2-3, Timex in TR5
#define ITEM_PASSPORT 2     // Exists only in TR1-3, not used in TR4 (diary)
#define ITEM_LARAHOME 3
#define ITEM_VIDEO    4     // Video settings. Exists only in TR1-3.
#define ITEM_AUDIO    5     // Audio settings. Exists only in TR1-3.
#define ITEM_CONTROLS 6     // Control settings. Exists only in TR1-3.
#define ITEM_LOAD     7     // Load game. Exists only in TR4-5.
#define ITEM_SAVE     8     // Save game. Exists only in TR4-5.
#define ITEM_MAP      9     // Map item only existed in TR1-3, not used.

//=====================//
//  Inventory Section  //
//=====================//

#define ITEM_PISTOL     10  // Exists in all game versions
#define ITEM_SHOTGUN    11  // Exists in all game versions
#define ITEM_MAGNUMS    12  // Exists in TR1
#define ITEM_AUTOMAGS   13
#define ITEM_DESERTEAGLE 14
#define ITEM_REVOLVER    15
#define ITEM_UZIS       16  // Exists in all game versions
#define ITEM_M16        17  // Exists only in TR2
#define ITEM_MP5        18  // Exists since TR3, aka H&K in TR5
#define ITEM_GRENADEGUN 19  // Exists since TR2, through all game versions
#define ITEM_ROCKETGUN  20  // Exists in TR3 only
#define ITEM_HARPOONGUN 21  // Exists in TR2-3 only (TRNG with customize command)
#define ITEM_CROSSBOW   22  // Exists since TR4.
#define ITEM_GRAPPLEGUN 23  // Grappling Gun in TR5

#define ITEM_LASERSIGHT 24
#define ITEM_BINOCULARS 25
#define ITEM_SILENCER   26  // Exists only in TR5, not used.

#define ITEM_PISTOL_AMMO 27
#define ITEM_SHOTGUN_AMMO 28
#define ITEM_SHOTGUN_NORMAL_AMMO 29
#define ITEM_SHOTGUN_WIDESHOT_AMMO 30
#define ITEM_MAGNUM_AMMO 31
#define ITEM_DESERTEAGLE_AMMO 32
#define ITEM_AUTOMAGS_AMMO 33
#define ITEM_REVOLVER_AMMO 34
#define ITEM_UZI_AMMO 35
#define ITEM_M16_AMMO 36
#define ITEM_MP5_AMMO 37
#define ITEM_GRENADEGUN_AMMO 38            // grenadegun ammo tr1 to tr3
#define ITEM_GRENADEGUN_NORMAL_AMMO 39     // tr4+
#define ITEM_GRENADEGUN_SUPER_AMMO 40
#define ITEM_GRENADEGUN_FLASH_AMMO 41
#define ITEM_ROCKETGUN_AMMO 42
#define ITEM_HARPOONGUN_AMMO 43
#define ITEM_CROSSBOW_NORMAL_AMMO 44
#define ITEM_CROSSBOW_POISON_AMMO 45
#define ITEM_CROSSBOW_EXPLOSIVE_AMMO 46
#define ITEM_GRAPPLEGUN_AMMO 47

#define ITEM_FLARES 48
#define ITEM_SINGLE_FLARE 49
#define ITEM_TORCH 50

#define ITEM_SMALL_MEDIPACK 51
#define ITEM_LARGE_MEDIPACK 52

typedef struct base_item_s
{
    uint32_t                    id;
    uint32_t                    world_model_id;
    uint16_t                    type;
    uint16_t                    count;
    char                        name[64];
    struct ss_bone_frame_s     *bf;
}base_item_t, *base_item_p;


typedef struct inventory_node_s
{
    uint32_t                    id;
    int32_t                     count;
    uint32_t                    max_count;
    struct inventory_node_s    *next;
}inventory_node_t, *inventory_node_p;

base_item_p BaseItem_Create(struct skeletal_model_s *model, uint32_t id);
void BaseItem_Delete(base_item_p item);

int32_t Inventory_AddItem(struct inventory_node_s **root, uint32_t item_id, int32_t count);       
int32_t Inventory_RemoveItem(struct inventory_node_s **root, uint32_t item_id, int32_t count);
int32_t Inventory_RemoveAllItems(struct inventory_node_s **root);
int32_t Inventory_GetItemsCount(struct inventory_node_s *root, uint32_t item_id);

#endif
