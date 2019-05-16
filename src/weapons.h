
#ifndef WEAPONS_H
#define WEAPONS_H

#include <stdint.h>

/// "inc_state" state
#define ARMED (1)
#define UNARMED (2)

#define AMMO_EMPTY (0)
#define AMMO_UNLIMITED (-1)

/// DEFAULT PISTOLS (ALL TR HAVE PISTOL IN SLOT 1)
#define TR_MODEL_PISTOL     1
#define TR_MODEL_SHOTGUN    3        // tr2 to tr5

/// TR1 Constants
#define TR1_MODEL_SHOTGUN   2
#define TR1_MODEL_MAGNUM    3
#define TR1_MODEL_UZI       4

/// TR2 Constants
#define TR2_MODEL_AUTOMAGS      4    // autopistols
#define TR2_MODEL_UZI           5
#define TR2_MODEL_M16           6
#define TR2_MODEL_GRENADEGUN    7
#define TR2_MODEL_HARPOONGUN    8

/// TR3 Constants
#define TR3_MODEL_DESERTEAGLE    4
#define TR3_MODEL_UZI            5
#define TR3_MODEL_MP5            6
#define TR3_MODEL_ROCKETGUN      7
#define TR3_MODEL_GRENADEGUN     8
#define TR3_MODEL_HARPOONGUN     9

/// TR4 & 5 Constants
#define TR4C_MODEL_UZI           2    // TR4 - TR5
#define TR4C_MODEL_CROSSBOW      4    // TR4 - TR5 // grappling_gun at last level (id 4)
#define TR4C_MODEL_GRENADEGUN    5    // TR4 - TR5
#define TR4C_MODEL_REVOLVER      6    // TR4 - TR5


struct weapons_s
{
    uint32_t item_id; // really usefull ?
    int ammo_counter; // used by harpoon
    int shot;
    int draw;
    int hide;
    int echo;
    int reload_1;     // classic reload
    int reload_2;     // grenadegun reload lock
    int damage;                   // not used (for future update ?) (classic damage by weapon (impact))
    int damage_water;             // not used (for future update ?) (damage in water.)
    int damage_explosion;         // not used (for future update ?) (damage of explosion (damage is not added)
    float firerate;
    int bullet;
    float range;
    uint32_t current_ammo;            // not used (for ammo select function); TeslaRus: must be in character params section

    // this section is not used for now. (itemIsEquipped is used in gui_inventory.cpp)
    bool haveGravity;                 // for grenade and harpoon (affected in water ?)
    bool dealDmgAtImpact;             // for projectile ammo (and crossbow poison ?), if false bullet is normal
    bool onWater;                     // weapon is in water ? (for sound or gravity check) (usefull for harpoon underwater sound)
    bool alternateAim;                // if weapon have an alternate aim (can change firerate) !
    bool itemIsEquipped;              // if item like lasersight is equiped (to enable the mesh in inventory (and world ?)) (only one variable ?)
    float muzzle_duration;              // time of the muzzle will rest in the world
    float muzzle_pos[3];                // muzzle_pos([0] = x, [1] = y, [2] = z) (need float ?)
    float muzzle_orient[3];             // muzzle_orient([0] = x, [1] = y, [2] = z) (need float ?)
};

struct weapons_s getPistol();
struct weapons_s getShotgun();
struct weapons_s getMagnum();
struct weapons_s getUzi();
struct weapons_s getAutomags();
struct weapons_s getDesertEagle();
struct weapons_s getRevolver();
struct weapons_s getM16();
struct weapons_s getMP5();
struct weapons_s getRocketGun();
struct weapons_s getHarpoonGun(bool is_underwater);
struct weapons_s getGrenadeGun();
struct weapons_s getCrossbowGun();
struct weapons_s getGrapplinGun();

bool Weapons_GetIdsFromActionKey(int weaponId/*0.. 7*/, int *pointerModelId, int *pointerInventoryItemId);

#endif