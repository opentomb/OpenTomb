#ifndef WEAPONS_H
#define WEAPONS_H

#include <stdint.h>
#include "audio/audio.h"
#include "skeletal_model.h"
#include "entity.h"

#define TR_IV_V 14      // 6+8 :x
#define GET_VERSION(id) getVersion(id)

/// "inc_state" state
#define ARMED 1
#define UNARMED 2

// Mesh for Two Hand and One Hand
#define HAND_RIGHT 10
#define HAND_LEFT 13
#define HOLSTER_LEFT 1
#define HOLSTER_RIGHT 4
#define TR1_BACK_WEAPON 7
#define TR2_BACK_WEAPON 14   // tr2 to tr5

// used by checkCanShoot
#define NOT_EMPTY_OR_UNLIMITED weapon.current_ammo != AMMO_EMPTY && weapon.current_ammo != AMMO_UNLIMITED

// Two Hand Frame Start and End
#define TW_FRAME 13
#define TW_FRAME_GRENADEGUN_IS_END 14
#define TW_FRAME_IS_END 23

///===========///
///  utility  ///
///===========///
#define SET_ANIM(next_anim) ss_anim->target_state = next_anim
#define ANIM_STATE inc_state = Anim_IncTime(ss_anim, (ent->character->state.weapon_ready && do_aim) ? (time) : (-time))
#define GET_MODEL(model_id) ss_anim->model->id == model_id

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

// all weapon id
enum weapon_item_e
{
    TYPE_NULL = 0,
    TYPE_PISTOLS = 10,
    TYPE_SHOTGUN = 11,
    TYPE_MAGNUMS = 12,
    TYPE_UZIS = 16,
    TYPE_AUTOMAGS = 13,
    TYPE_DESERTEAGLE = 14,
    TYPE_REVOLVER = 15,
    TYPE_M16 = 17,
    TYPE_MP5 = 18,
    TYPE_ROCKETGUN = 20,
    TYPE_HARPOONGUN = 21,
    TYPE_GRENADEGUN = 19,
    TYPE_CROSSBOWGUN = 22,
    TYPE_GRAPPLINGUN = 23
};

// all weapon ammo
enum weapon_ammo_e
{
    AMMO_EMPTY = 0,
    AMMO_UNLIMITED = -1,
    AMMO_SHOTGUN = 28,                                // TR1 to TR3
    AMMO_SHOTGUN_NORMAL = 29,                         // TR4+
    AMMO_SHOTGUN_WIDE = 30,
    AMMO_MAGNUM = 31,
    AMMO_UZI = 35,
    AMMO_AUTOMAGS = 33,
    AMMO_DESERTEAGLE = 32,
    AMMO_REVOLVER = 34,
    AMMO_M16 = 36,
    AMMO_MP5 = 37,
    AMMO_ROCKETGUN = 42,
    AMMO_HARPOON = 43,
    AMMO_GRENADE = 39,                                 // TR1 to TR3
    AMMO_GRENADE_NORMAL = 39,                          // TR4+
    AMMO_GRENADE_SUPER = 40,
    AMMO_GRENADE_FLASH = 41,
    AMMO_CROSSBOW_NORMAL = 44,
    AMMO_CROSSBOW_SUPER = 46,
    AMMO_CROSSBOW_POISON = 45,
    AMMO_GRAPPLIN = 47
};

// all weapon sound
enum weapon_sound_e
{
    SND_NULL = -1,
    SND_UNIVERSAL_RELOAD = TR_AUDIO_SOUND_RELOAD,
    SND_UNIVERSAL_DRAW = TR_AUDIO_SOUND_HOLSTEROUT,
    SND_UNIVERSAL_HIDE = TR_AUDIO_SOUND_HOLSTERIN,
    SND_PISTOL = TR_AUDIO_SOUND_SHOTPISTOLS,
    SND_SHOTGUN = TR_AUDIO_SOUND_SHOTSHOTGUN,
    SND_UZI = TR_AUDIO_SOUND_SHOTUZI,
    SND_UZI_STOP = TR_AUDIO_SOUND_SHOTUZI_END,
    SND_MAGNUM = TR_AUDIO_SOUND_SHOTMAGNUM,
    SND_AUTOMAGS = TR_AUDIO_SOUND_SHOTAUTOMAGS,
    SND_DESERTEAGLE = TR_AUDIO_SOUND_SHOTDESERTEAGLE,
    SND_REVOLVER = TR_AUDIO_SOUND_SHOTREVOLVER,
    SND_M16 = TR_AUDIO_SOUND_SHOTM16,
    SND_M16_STOP = TR_AUDIO_SOUND_SHOTM16_END,
    SND_MP5 = TR_AUDIO_SOUND_SHOTMP5,
    SND_ROCKETGUN = TR_AUDIO_SOUND_SHOTROCKETGUN,
    SND_ROCKETGUN_RELOAD = TR_AUDIO_SOUND_RELOADROCKETGUN,
    SND_HARPOON_LAND = TR_AUDIO_SOUND_SHOTHARPOON_G,
    SND_HARPOON_WATER = TR_AUDIO_SOUND_SHOTHARPOON_W,
    SND_HARPOON_RELOAD_LAND = TR_AUDIO_SOUND_RELOADHARPOON_G,
    SND_HARPOON_RELOAD_WATER = TR_AUDIO_SOUND_RELOADHARPOON_W,
    SND_GRENADEGUN = TR_AUDIO_SOUND_SHOTGRENADEGUN,
    SND_TR2_3_GRENADEGUN_RELOAD = TR_AUDIO_SOUND_TR2_3_RELOADGRENADEGUN,
    SND_TR2_3_GRENADEGUN_LOCK = TR_AUDIO_SOUND_TR2_3_RELOADGRENADEGUN_LOCK,
    SND_TR4_C_GRENADEGUN_RELOAD = TR_AUDIO_SOUND_TR4C_RELOADGRENADEGUN,
    SND_TR4_C_GRENADEGUN_LOCK = TR_AUDIO_SOUND_TR4C_RELOADGRENADEGUN_LOCK,
    SND_CROSSBOWGUN = TR_AUDIO_SOUND_SHOTCROSSBOW
};

#define RATE_LOWEST 0.1f
#define RATE_PISTOL 1.0f
#define RATE_SHOTGUN 1.0f
#define RATE_UZI 4.0f
#define RATE_MAGNUM 0.8f
#define RATE_AUTOMAGS 1.0f
#define RATE_DESERTEAGLE 0.8f
#define RATE_REVOLVER 0.8f
#define RATE_M16 1.0f
#define RATE_M16_ALT 1.5f
#define RATE_MP5 2.0f
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
    uint32_t item_id;
    int ammo_counter;
    int shot;
    int draw;
    int hide;
    int echo;
    int reload_1;     // classic reload
    int reload_2;     // grenadegun reload lock
    int damage = NULL;
    int damage_water = NULL;
    int damage_explosion = NULL;
    float firerate;
    int bullet;
    float range;
    uint32_t current_ammo;
    int32_t version;                  // tomb raider version (not used)
    bool haveGravity;                 // for grenade and harpoon (affected in water ?)
    bool dealDmgAtImpact;             // for projectile ammo (and crossbow poison ?), if false bullet is normal
    bool onWater;                     // weapon is in water ? (for sound or gravity check)
    bool alternateAim;                // if weapon have an alternate aim (can change firerate) !
    float muzzle_duration;              // time of the muzzle will rest in the world
    float muzzle_pos[3];                // muzzle_pos([0] = x, [1] = y, [2] = z) (need float ?)
    float muzzle_orient[3];             // muzzle_orient([0] = x, [1] = y, [2] = z) (need float ?)
};

// init all weapons
void weapon_init();

bool checkCanShoot(struct weapons_s item_id);
// consume ammo after firing (target or not !)
bool consumeAmmo(struct weapons_s weapon);
int CurrentWeaponToItemID(struct ss_animation_s* ss_anim); // this function can be found in state_control_weapons
// auto change weapon to model_id assigned
void AutoSelect(int model_id, ss_animation_s *ss_anim, entity_s* ent, float time);

int32_t getVersion(int id);

/// int return because for checking (just this yea !)
/// 0x01 for sound, 0x02 for hide, 0x03 for draw
/// 0x04 for firing, 0x05 for reload, 0x06, for reload 2, 0x07, for firing to idle
/// 0x08 for special (for items that contains more than 4 animation (include 0))

#define IS_NOT_DEFINED    -1
#define IS_IDLE           0x00
#define IS_SOUND          0x01
#define IS_HIDE           0x02
#define IS_DRAW           0x03
#define IS_FIRING         0x04
#define IS_RELOAD         0x05
#define IS_RELOAD2        0x06
#define IS_FIRING_TO_IDLE 0x07
#define IS_SPECIAL        0x08

// One Hand Weapon Animation Controller
int SetCurrentWeaponAnimation(struct entity_s* ent, struct ss_animation_s* ss_anim, float time, struct weapons_s weapon, ss_bone_tag_p b_tag, entity_p target, float* target_pos, int inc_state, bool do_aim, uint16_t targeted_bone_start, uint16_t targeted_bone_end);

/// for modder ?
//int PistolAnim(struct entity_s* ent, struct ss_animation_s* ss_anim, float time, struct weapons_s weapon, ss_bone_tag_p b_tag, entity_p target, float* target_pos, int inc_state, bool do_aim, uint16_t targeted_bone_start, uint16_t targeted_bone_end);
//int MagnumAnim(struct entity_s* ent, struct ss_animation_s* ss_anim, float time, struct weapons_s weapon, ss_bone_tag_p b_tag, entity_p target, float* target_pos, int inc_state, bool do_aim, uint16_t targeted_bone_start, uint16_t targeted_bone_end);
//int UziAnim(struct entity_s* ent, struct ss_animation_s* ss_anim, float time, struct weapons_s weapon, ss_bone_tag_p b_tag, entity_p target, float* target_pos, int inc_state, bool do_aim, uint16_t targeted_bone_start, uint16_t targeted_bone_end);
//int AutomagsAnim(struct entity_s* ent, struct ss_animation_s* ss_anim, float time, struct weapons_s weapon, ss_bone_tag_p b_tag, entity_p target, float* target_pos, int inc_state, bool do_aim, uint16_t targeted_bone_start, uint16_t targeted_bone_end);
//int DesertEagleAnim(struct entity_s* ent, struct ss_animation_s* ss_anim, float time, struct weapons_s weapon, ss_bone_tag_p b_tag, entity_p target, float* target_pos, int inc_state, bool do_aim, uint16_t targeted_bone_start, uint16_t targeted_bone_end);
//int RevolverAnim(struct entity_s* ent, struct ss_animation_s* ss_anim, float time, struct weapons_s weapon, ss_bone_tag_p b_tag, entity_p target, float* target_pos, int inc_state, bool do_aim, uint16_t targeted_bone_start, uint16_t targeted_bone_end);

/// Two Hand Animation Controller
int ShotgunAnim(struct entity_s* ent, struct ss_animation_s* ss_anim, float time, struct weapons_s weapon, ss_bone_tag_p b_tag, entity_p target, float* target_pos, int inc_state, bool do_aim);
int GrenadeGunAnim(struct entity_s* ent, struct ss_animation_s* ss_anim, float time, struct weapons_s weapon, ss_bone_tag_p b_tag, entity_p target, float* target_pos, int inc_state, bool do_aim);
int HarpoonAnim(struct entity_s* ent, struct ss_animation_s* ss_anim, float time, struct weapons_s weapon, ss_bone_tag_p b_tag, entity_p target, float* target_pos, int inc_state, bool do_aim);
int MP5Anim(struct entity_s* ent, struct ss_animation_s* ss_anim, float time, struct weapons_s weapon, ss_bone_tag_p b_tag, entity_p target, float* target_pos, int inc_state, bool do_aim);
int M16Anim(struct entity_s* ent, struct ss_animation_s* ss_anim, float time, struct weapons_s weapon, ss_bone_tag_p b_tag, entity_p target, float* target_pos, int inc_state, bool do_aim);
int RocketGunAnim(struct entity_s* ent, struct ss_animation_s* ss_anim, float time, struct weapons_s weapon, ss_bone_tag_p b_tag, entity_p target, float* target_pos, int inc_state, bool do_aim);
int CrossbowAnim(struct entity_s* ent, struct ss_animation_s* ss_anim, float time, struct weapons_s weapon, ss_bone_tag_p b_tag, entity_p target, float* target_pos, int inc_state, bool do_aim);
int GrapplinGunAnim(struct entity_s* ent, struct ss_animation_s* ss_anim, float time, struct weapons_s weapon, ss_bone_tag_p b_tag, entity_p target, float* target_pos, int inc_state, bool do_aim);

///===================================///
///       Weapon Fast Function        ///
///===================================///
#define ONEHAND_IDLE_TO_FIRING OneHand_IdleToFiring(ent, ss_anim, weapon, b_tag, target, do_aim, time)
bool OneHand_IdleToFiring(struct entity_s* ent, struct ss_animation_s* ss_anim, struct weapons_s weapon, ss_bone_tag_p b_tag, entity_p target, bool do_aim, float time);
#define TWOHAND_IDLE_TO_FIRING(anim_hide, anim_firing) TwoHand_IdleToFiring(ent, ss_anim, weapon, b_tag, target, do_aim, time, inc_state, anim_hide, anim_firing)
bool TwoHand_IdleToFiring(struct entity_s* ent, struct ss_animation_s* ss_anim, struct weapons_s weapon, ss_bone_tag_p b_tag, entity_p target, bool do_aim, float time, int inc_state, int anim_hide, int anim_firing);

#define TWOHAND_FIRING_TO_IDLE(anim_idle_to_firing) TwoHand_FiringToIdle(ent, ss_anim, weapon, time, anim_idle_to_firing)
bool TwoHand_FiringToIdle(struct entity_s* ent, struct ss_animation_s* ss_anim, struct weapons_s weapon, float time, int anim_idle_to_firing);

#define ONEHAND_IDLE_TO_HIDE OneHand_IdleToHide(ent, ss_anim, weapon, time, do_aim)
bool OneHand_IdleToHide(struct entity_s* ent, struct ss_animation_s* ss_anim, struct weapons_s weapon, float time, bool do_aim);
#define TWOHAND_IDLE_TO_HIDE(frame_to_hide, changeToPistol) TwoHand_IdleToHide(ent, ss_anim, weapon, time, inc_state, frame_to_hide)
bool TwoHand_IdleToHide(struct entity_s* ent, struct ss_animation_s* ss_anim, struct weapons_s weapon, float time, int inc_state, int frame_to_hide);

#define ONEHAND_HIDE_TO_IDLE OneHand_HideToIdle(ent, ss_anim, weapon, time)
bool OneHand_HideToIdle(struct entity_s* ent, struct ss_animation_s* ss_anim, struct weapons_s weapon, float time);
#define TWOHAND_HIDE_TO_IDLE(anim_idle) TwoHand_HideToIdle(ent, ss_anim, weapon, time, inc_state, anim_idle)
bool TwoHand_HideToIdle(struct entity_s* ent, struct ss_animation_s* ss_anim, struct weapons_s weapon, float time, int inc_state, int anim_idle);

#define ONEHAND_HIDE_AND_IDLE OneHand_HideAndIdle(ent, ss_anim, weapon, time)
bool OneHand_HideAndIdle(struct entity_s* ent, struct ss_animation_s* ss_anim, struct weapons_s weapon, float time);

#define ONEHAND_FIRING(anim_firing, anim_firing_to_idle) OneHand_Firing(ent, ss_anim, b_tag, target, target_pos, weapon, time, targeted_bone_start, targeted_bone_end, anim_firing, anim_firing_to_idle)
bool OneHand_Firing(struct entity_s* ent, struct ss_animation_s* ss_anim, ss_bone_tag_p b_tag, entity_p target, float* target_pos, struct weapons_s weapon, float time, uint16_t targeted_bone_start, uint16_t targeted_bone_end, int anim_firing, int anim_firing_to_idle);
#define TWOHAND_FIRING(anim_firing, anim_firing_to_idle, anim_idle) TwoHand_Firing(ent, ss_anim, b_tag, target, target_pos, weapon, time, anim_firing, anim_firing_to_idle, anim_idle)
bool TwoHand_Firing(struct entity_s* ent, struct ss_animation_s* ss_anim, ss_bone_tag_p b_tag, entity_p target, float* target_pos, struct weapons_s weapon, float time, int anim_firing, int anim_firing_to_idle, int anim_idle);

#pragma region weapons
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
struct weapons_s* getAll();
#pragma endregion
#endif