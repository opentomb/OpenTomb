
#ifndef WEAPONS_H
#define WEAPONS_H

#include <stdint.h>

/// "inc_state" state
#define ARMED (1)
#define UNARMED (2)

#define AMMO_EMPTY (0)
#define AMMO_UNLIMITED (-1)

// Mesh for Two Hand and One Hand
#define HAND_RIGHT 10
#define HAND_LEFT 13
#define HOLSTER_LEFT 1
#define HOLSTER_RIGHT 4
#define TR1_BACK_WEAPON 7
#define TR2_BACK_WEAPON 14       // tr2 to tr5
#define TR4_REVOLVER_LASERSIGHT_MESH 4
#define TR4_CROSSBOW_LASERSIGHT_MESH 1

// Two Hand Frame Start and End
#define TW_FRAME 13
#define TW_FRAME_GRENADEGUN_IS_END 14
#define TW_FRAME_IS_END 23

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

/// Custom TR Define for State Control (sound, reload, fire, etc..)
/// it's for custom level (TR4) to add all weapon in same game :)
#define CTDS_PISTOL             3
#define CTDS_SHOTGUN            4
#define CTDS_AUTOPISTOL         5
#define CTDS_UZI                6
#define CTDS_DESERT_EAGLE       7
#define CTDS_M16                8
#define CTDS_MP5                9
#define CTDS_GRENADE_LAUNCHER   10
#define CTDS_ROCKET_LAUNCHER    11
#define CTDS_HARPOON_LAUNCHER   12

#define ANIM_IDLE_TO_FIRING 0
#define ANIM_HIDE_TO_IDLE 1
#define ANIM_IDLE_AND_HIDE 2
#define ANIM_FIRING 3

#define SHOTGUN_IDLE_TO_FIRING 0
#define SHOTGUN_HIDE_TO_IDLE 1
#define SHOTGUN_FIRING 2
#define SHOTGUN_IDLE_TO_HIDE 3
#define SHOTGUN_FIRING_TO_IDLE 4

#define HARPOON_LAND_IDLE_TO_FIRING 0
#define HARPOON_LAND_HIDE_TO_IDLE 1
#define HARPOON_LAND_FIRING 2
#define HARPOON_LAND_IDLE_TO_HIDE 3
#define HARPOON_LAND_FIRING_TO_IDLE 4
#define HARPOON_RELOAD 5
#define HARPOON_WATER_IDLE_TO_FIRING 6
#define HARPOON_WATER_FIRING_TO_IDLE 7
#define HARPOON_WATER_FIRING 8
#define HARPOON_WATER_IDLE_TO_HIDE 9
#define HARPOON_WATER_ALT_IDLE_TO_HIDE 10   // not used !

#define M16_IDLE_TO_FIRING 0
#define M16_HIDE_TO_IDLE 1
#define M16_FIRING 2
#define M16_IDLE_TO_HIDE 3
#define M16_FIRING_TO_IDLE 4
#define M16_MOVE_IDLE_TO_AIM 5
#define M16_MOVE_FIRING_TO_IDLE 6
#define M16_MOVE_FIRING 7

#define MP5_IDLE_TO_FIRING 0
#define MP5_HIDE_TO_IDLE 1
#define MP5_FIRING 2
#define MP5_IDLE_TO_HIDE 3
#define MP5_FIRING_TO_IDLE 4
#define MP5_MOVE_IDLE_TO_AIM 5
#define MP5_MOVE_FIRING_TO_IDLE 6
#define MP5_MOVE_FIRING 7

#define GRENADEGUN_HIDE_TO_IDLE 0
#define GRENADEGUN_IDLE_TO_FIRING 1     // -> firing (not reload ?)
#define GRENADEGUN_FIRING_RELOAD 2      // -> firing
#define GRENADEGUN_FIRING 3             // -> reload
#define GRENADEGUN_FIRING_TO_IDLE 4
#define GRENADEGUN_IDLE_TO_HIDE 5

#define ROCKETGUN_IDLE_TO_FIRING 0
#define ROCKETGUN_HIDE_TO_IDLE 1
#define ROCKETGUN_FIRING 2
#define ROCKETGUN_IDLE_TO_HIDE 3
#define ROCKETGUN_FIRING_TO_IDLE 4

#define CROSSBOW_IDLE_TO_FIRING 0
#define CROSSBOW_HIDE_TO_IDLE 1
#define CROSSBOW_FIRING 2
#define CROSSBOW_IDLE_TO_HIDE 3
#define CROSSBOW_FIRING_TO_IDLE 4

/// this weapon can only found at last level of tr5
#define GRAPPLING_IDLE_TO_FIRING 0
#define GRAPPLING_HIDE_TO_IDLE 1
#define GRAPPLING_FIRING 2             // no frame
#define GRAPPLING_IDLE_TO_HIDE 3
#define GRAPPLING_FIRING_TO_IDLE 4

#define RATE_LOWEST 0.1f
#define RATE_PISTOL 1.0f
#define RATE_SHOTGUN 1.0f
#define RATE_UZI 4.0f
#define RATE_MAGNUM 0.8f
#define RATE_AUTOMAGS 1.0f
#define RATE_DESERTEAGLE 0.8f
#define RATE_REVOLVER 0.8f
#define RATE_M16 1.0f
// used in standing
#define RATE_M16_ALT 1.5f
#define RATE_MP5 2.0f
// used in standing
#define RATE_MP5_ALT 2.5f
#define RATE_ROCKETGUN 1.0f
#define RATE_HARPOONGUN 1.0f
#define RATE_GRENADEGUN 1.0f
#define RATE_CROSSBOWGUN 1.0f
#define RATE_GRAPPINGUN 1.0f

#define RANGE_LOWEST 256.0f     // block "1/4"
#define RANGE_PISTOL 8192.0f
#define RANGE_SHOTGUN 8192.0f
#define RANGE_UZI 8192.0f
#define RANGE_MAGNUM 8192.0f
#define RANGE_AUTOMAGS 8192.0f
#define RANGE_DESERTEAGLE 8192.0f
#define RANGE_REVOLVER 8192.0f
#define RANGE_M16 16384.0f
#define RANGE_MP5 16384.0f
#define RANGE_ROCKET 65535.0f    // explose at impact (no range)
#define RANGE_GRENADE 8192.0f    // explose at impact (not in TR4+)
#define RANGE_CROSSBOW 16384.0f  // gravity ?
#define RANGE_GRAPPLIN 16384.0f  // don't know the range :x

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
    uint32_t current_ammo;            // not used (for ammo select function)

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

// init all weapons
void World_WeaponInit();

bool checkCanShoot(struct weapons_s *weapon);
// consume ammo after firing (target or not !)
bool consumeAmmo(struct weapons_s *weapon);

int CurrentWeaponModelToItemID(struct ss_animation_s *ss_anim); // this function can be found in state_control_weapons
// auto change weapon to model_id assigned
void AutoSelect(int model_id, struct ss_animation_s *ss_anim, struct entity_s *ent, float time);

// One Hand Weapon Animation Controller
void SetCurrentWeaponAnimation(struct entity_s *ent, struct ss_animation_s *ss_anim, float time, struct weapons_s *weapon, struct ss_bone_tag_s *b_tag, struct entity_s *target, float *target_pos, bool do_aim, uint16_t targeted_bone_start, uint16_t targeted_bone_end);

/// Two Hand Animation Controller
// may be add MORE arguments... ;-)
void ShotgunAnim(struct entity_s *ent,  ss_animation_s *ss_anim, float time, struct weapons_s *weapon, struct ss_bone_tag_s *b_tag, struct entity_s *target, float *target_pos, int inc_state, bool do_aim);
void GrenadeGunAnim(struct entity_s *ent, struct ss_animation_s *ss_anim, float time, struct weapons_s *weapon, struct ss_bone_tag_s *b_tag, struct entity_s *target, float *target_pos, int inc_state, bool do_aim);
void HarpoonAnim(struct entity_s *ent, struct ss_animation_s *ss_anim, float time, struct weapons_s *weapon, struct ss_bone_tag_s *b_tag, struct entity_s *target, float *target_pos, int inc_state, bool do_aim);
void MP5Anim(struct entity_s *ent, struct ss_animation_s *ss_anim, float time, struct weapons_s *weapon, struct ss_bone_tag_s *b_tag, struct entity_s *target, float *target_pos, int inc_state, bool do_aim);
void M16Anim(struct entity_s *ent, struct ss_animation_s *ss_anim, float time, struct weapons_s *weapon, struct ss_bone_tag_s *b_tag, struct entity_s *target, float *target_pos, int inc_state, bool do_aim);
void RocketGunAnim(struct entity_s *ent, struct ss_animation_s *ss_anim, float time, struct weapons_s *weapon, struct ss_bone_tag_s *b_tag, struct entity_s *target, float *target_pos, int inc_state, bool do_aim);
void CrossbowAnim(struct entity_s *ent, struct ss_animation_s *ss_anim, float time, struct weapons_s *weapon, struct ss_bone_tag_s *b_tag, struct entity_s *target, float *target_pos, int inc_state, bool do_aim);
void GrapplinGunAnim(struct entity_s *ent, struct ss_animation_s *ss_anim, float time, struct weapons_s *weapon, struct ss_bone_tag_s *b_tag, struct entity_s *target, float *target_pos, int inc_state, bool do_aim);

///===================================///
///       Weapon Fast Function        ///
///===================================///
void OneHand_IdleToFiring(struct entity_s *ent, struct ss_animation_s *ss_anim, struct weapons_s *weapon, struct ss_bone_tag_s *b_tag, struct entity_s *target, bool do_aim, float time);
void OneHand_HideToIdle(struct entity_s *ent, struct ss_animation_s *ss_anim, struct weapons_s *weapon, float time);
void OneHand_HideAndIdle(struct entity_s *ent, struct ss_animation_s *ss_anim, struct weapons_s *weapon, float time);
void OneHand_Firing(struct entity_s *ent, struct ss_animation_s *ss_anim, struct ss_bone_tag_s *b_tag, struct entity_s *target, float *target_pos, struct weapons_s *weapon, float time, uint16_t targeted_bone_start, uint16_t targeted_bone_end, int anim_firing, int anim_firing_to_idle);

void TwoHand_IdleToFiring(struct entity_s *ent, struct ss_animation_s *ss_anim, struct weapons_s *weapon, struct ss_bone_tag_s *b_tag, struct entity_s *target, bool do_aim, float time, int inc_state, int anim_hide, int anim_firing);
void TwoHand_HideToIdle(struct entity_s *ent, struct ss_animation_s *ss_anim, struct weapons_s *weapon, float time, int anim_idle);
void TwoHand_Firing(struct entity_s *ent, struct ss_animation_s *ss_anim, struct ss_bone_tag_s *b_tag, struct entity_s *target, float *target_pos, struct weapons_s *weapon, float time, int anim_firing, int anim_firing_to_idle, int anim_idle);
void TwoHand_IdleToHide(struct entity_s *ent, struct ss_animation_s *ss_anim, struct weapons_s *weapon, float time, int frame_to_hide);
void TwoHand_FiringToIdle(struct entity_s *ent, struct ss_animation_s *ss_anim, struct weapons_s *weapon, float time, int anim_idle_to_firing);

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
struct weapons_s getHarpoonGun();
struct weapons_s getGrenadeGun();
struct weapons_s getCrossbowGun();
struct weapons_s getGrapplinGun();

#endif