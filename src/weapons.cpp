#include "weapons.h"

#include "core/system.h"
#include "core/console.h"
#include "core/vmath.h"
#include "core/obb.h"

#include "script/script.h"
#include "room.h"
#include "world.h"

#include "character_controller.h"
#include "state_control/state_control_Lara.h"

struct weapons_s pistol;
struct weapons_s shotgun;
struct weapons_s uzi;
struct weapons_s magnum;
struct weapons_s automags;
struct weapons_s deserteagle;
struct weapons_s revolver;
struct weapons_s m16;
struct weapons_s mp5;
struct weapons_s rocket;
struct weapons_s grenade;
struct weapons_s harpoon;
struct weapons_s crossbow;
struct weapons_s grapplin;

/*=============================================//
//             WEAPON CLASS INIT               //
//=============================================*/
void World_WeaponInit()
{
    entity_s* player = World_GetPlayer();

    //=============================================//
    //                PISTOLS GUN                  //
    //=============================================//
    pistol.item_id = TYPE_PISTOLS;
    pistol.shot = SND_PISTOL;
    pistol.draw = SND_UNIVERSAL_DRAW;
    pistol.hide = SND_UNIVERSAL_HIDE;
    pistol.echo = SND_NULL;
    pistol.reload_1 = SND_UNIVERSAL_RELOAD;
    pistol.reload_2 = SND_NULL;
    pistol.damage = 1;
    pistol.firerate = RATE_PISTOL;
    pistol.bullet = 2;
    pistol.current_ammo = AMMO_UNLIMITED;
    pistol.range = RANGE_PISTOL;
    pistol.muzzle_duration = 3.0f;
    pistol.muzzle_pos[0] = 0;
    pistol.muzzle_pos[1] = 0;
    pistol.muzzle_pos[2] = 0;
    pistol.muzzle_orient[0] = 0;
    pistol.muzzle_orient[1] = 0;
    pistol.muzzle_orient[2] = 0;

    //=============================================//
    //                SHOTGUN GUN                  //
    //=============================================//
    shotgun.item_id = TYPE_SHOTGUN;
    shotgun.shot = SND_SHOTGUN;
    shotgun.draw = SND_UNIVERSAL_HIDE;
    shotgun.hide = SND_UNIVERSAL_HIDE;
    shotgun.echo = SND_NULL;
    shotgun.reload_1 = SND_UNIVERSAL_RELOAD;
    shotgun.reload_2 = SND_NULL;
    shotgun.damage = 3;
    shotgun.firerate = RATE_SHOTGUN;
    shotgun.bullet = (shotgun.current_ammo == AMMO_SHOTGUN_WIDE) ? 5 : 6; // wide have 5 bullet

    if (getVersion(TR_I_II_III))
    {
        shotgun.current_ammo = AMMO_SHOTGUN;
    }
    else if (getVersion(TR_IV_V))
    {
        shotgun.current_ammo = AMMO_SHOTGUN_NORMAL;
    }

    shotgun.range = RANGE_SHOTGUN;
    // shotgun not have muzzleflash but can be used for smoke
    shotgun.muzzle_duration = 0.0f;
    shotgun.muzzle_pos[0] = 0;
    shotgun.muzzle_pos[1] = 0;
    shotgun.muzzle_pos[2] = 0;
    shotgun.muzzle_orient[0] = 0;
    shotgun.muzzle_orient[1] = 0;
    shotgun.muzzle_orient[2] = 0;
    
    //=============================================//
    //                   UZI GUN                   //
    //=============================================//
    uzi.item_id = TYPE_UZIS;
    uzi.shot = SND_UZI;
    uzi.draw = SND_UNIVERSAL_DRAW;
    uzi.hide = SND_UNIVERSAL_HIDE;
    uzi.echo = (getVersion(TR_II_TO_V)) ? SND_UZI_STOP : SND_NULL;
    uzi.reload_1 = SND_UNIVERSAL_RELOAD;
    uzi.reload_2 = SND_NULL;
    uzi.damage = 1;
    uzi.firerate = RATE_UZI;
    uzi.bullet = 2;
    uzi.current_ammo = AMMO_UZI;
    uzi.muzzle_duration = 1.0f;
    uzi.muzzle_pos[0] = 0;
    uzi.muzzle_pos[1] = 0;
    uzi.muzzle_pos[2] = 0;
    uzi.muzzle_orient[0] = 0;
    uzi.muzzle_orient[1] = 0;
    uzi.muzzle_orient[2] = 0;

    //=============================================//
    //                 MAGNUM GUN                  //
    //=============================================//
    magnum.item_id = TYPE_MAGNUMS;
    magnum.shot = SND_MAGNUM;
    magnum.draw = SND_UNIVERSAL_DRAW;
    magnum.hide = SND_UNIVERSAL_HIDE;
    magnum.echo = SND_NULL;
    magnum.reload_1 = SND_UNIVERSAL_RELOAD;
    magnum.reload_2 = SND_NULL;
    magnum.damage = 21;
    magnum.firerate = RATE_MAGNUM;
    magnum.bullet = 2;
    magnum.current_ammo = AMMO_MAGNUM;
    magnum.muzzle_duration = 3.0f;
    magnum.muzzle_pos[0] = 0;
    magnum.muzzle_pos[1] = 0;
    magnum.muzzle_pos[2] = 0;
    magnum.muzzle_orient[0] = 0;
    magnum.muzzle_orient[1] = 0;
    magnum.muzzle_orient[2] = 0;

    //=============================================//
    //             DESERT EAGLE GUN                //
    //=============================================//
    deserteagle.item_id = TYPE_DESERTEAGLE;
    deserteagle.shot = SND_DESERTEAGLE;
    deserteagle.draw = SND_UNIVERSAL_DRAW;
    deserteagle.hide = SND_UNIVERSAL_HIDE;
    deserteagle.echo = SND_NULL;
    deserteagle.reload_1 = SND_UNIVERSAL_RELOAD;
    deserteagle.reload_2 = SND_NULL;
    deserteagle.damage = 21;
    deserteagle.firerate = RATE_DESERTEAGLE;
    deserteagle.bullet = 1;
    deserteagle.current_ammo = AMMO_DESERTEAGLE;
    deserteagle.muzzle_duration = 3.0f;
    deserteagle.muzzle_pos[0] = 0;
    deserteagle.muzzle_pos[1] = 0;
    deserteagle.muzzle_pos[2] = 0;
    deserteagle.muzzle_orient[0] = 0;
    deserteagle.muzzle_orient[1] = 0;
    deserteagle.muzzle_orient[2] = 0;

    //=============================================//
    //                AUTOMAGS GUN                 //
    //=============================================//
    automags.item_id = TYPE_AUTOMAGS;
    automags.shot = SND_AUTOMAGS;
    automags.draw = SND_UNIVERSAL_DRAW;
    automags.hide = SND_UNIVERSAL_HIDE;
    automags.echo = SND_NULL;
    automags.reload_1 = SND_UNIVERSAL_RELOAD;
    automags.reload_2 = SND_NULL;
    automags.damage = 12;
    automags.firerate = RATE_AUTOMAGS;
    automags.bullet = 2;
    automags.current_ammo = AMMO_AUTOMAGS;
    automags.muzzle_duration = 3.0f;
    automags.muzzle_pos[0] = 0;
    automags.muzzle_pos[1] = 0;
    automags.muzzle_pos[2] = 0;
    automags.muzzle_orient[0] = 0;
    automags.muzzle_orient[1] = 0;
    automags.muzzle_orient[2] = 0;

    //=============================================//
    //                REVOLVER GUN                 //
    //=============================================//
    revolver.item_id = TYPE_REVOLVER;
    revolver.shot = SND_REVOLVER;
    revolver.draw = SND_UNIVERSAL_DRAW;
    revolver.hide = SND_UNIVERSAL_HIDE;
    revolver.echo = SND_NULL;
    revolver.reload_1 = SND_UNIVERSAL_RELOAD;
    revolver.reload_2 = SND_NULL;
    revolver.damage = 21;
    revolver.firerate = RATE_REVOLVER;
    revolver.bullet = 1;
    revolver.current_ammo = AMMO_REVOLVER;
    revolver.itemIsEquipped = false;         // can be equiped of the lasersight !
    revolver.muzzle_duration = 3.0f;
    revolver.muzzle_pos[0] = 0;
    revolver.muzzle_pos[1] = 0;
    revolver.muzzle_pos[2] = 0;
    revolver.muzzle_orient[0] = 0;
    revolver.muzzle_orient[1] = 0;
    revolver.muzzle_orient[2] = 0;

    //=============================================//
    //                  M16 GUN                    //
    //=============================================//
    m16.item_id = TYPE_M16;
    m16.shot = SND_M16;
    m16.draw = SND_UNIVERSAL_HIDE;
    m16.hide = SND_NULL;
    m16.echo = SND_M16_STOP;
    m16.reload_1 = SND_NULL;
    m16.reload_2 = SND_NULL;
    m16.damage = 12;
    m16.firerate = (mp5.alternateAim) ? RATE_M16_ALT : RATE_M16;
    m16.bullet = 1;
    m16.current_ammo = AMMO_M16;
    m16.alternateAim = false;           // define it at animation (M16Anim()) ?
    m16.muzzle_duration = 3.0f;
    m16.muzzle_pos[0] = 0;
    m16.muzzle_pos[1] = 0;
    m16.muzzle_pos[2] = 0;
    m16.muzzle_orient[0] = 0;
    m16.muzzle_orient[1] = 0;
    m16.muzzle_orient[2] = 0;

    //=============================================//
    //                  MP5 GUN                    //
    //=============================================//
    mp5.item_id = TYPE_MP5;
    mp5.shot = SND_MP5;
    mp5.draw = SND_UNIVERSAL_HIDE;
    mp5.hide = SND_UNIVERSAL_HIDE;
    mp5.echo = SND_NULL;
    mp5.reload_1 = SND_NULL;
    mp5.reload_2 = SND_NULL;
    mp5.damage = 12;
    mp5.damage_explosion = 0;
    mp5.firerate = (mp5.alternateAim) ? RATE_MP5_ALT : RATE_MP5;
    mp5.bullet = 1;
    mp5.current_ammo = AMMO_MP5;
    mp5.alternateAim = false;
    mp5.muzzle_pos[0] = 0;
    mp5.muzzle_pos[1] = 0;
    mp5.muzzle_pos[2] = 0;
    mp5.muzzle_orient[0] = 0;
    mp5.muzzle_orient[1] = 0;
    mp5.muzzle_orient[2] = 0;

    //=============================================//
    //                 ROCKET GUN                  //
    //=============================================//
    rocket.item_id = TYPE_ROCKETGUN;
    rocket.shot = SND_ROCKETGUN;
    rocket.draw = SND_UNIVERSAL_HIDE;
    rocket.hide = SND_UNIVERSAL_HIDE;
    rocket.echo = SND_NULL;
    rocket.reload_1 = SND_ROCKETGUN_RELOAD;
    rocket.reload_2 = SND_NULL;
    rocket.damage_explosion = 100;
    rocket.firerate = RATE_ROCKETGUN;
    rocket.bullet = 1;
    rocket.current_ammo = AMMO_ROCKETGUN;
    rocket.onWater = false;
    rocket.dealDmgAtImpact = false;
    rocket.haveGravity = false;
    // rocket not have muzzleflash but smoke is used
    rocket.muzzle_duration = 0.0f;
    rocket.muzzle_pos[0] = 0;
    rocket.muzzle_pos[1] = 0;
    rocket.muzzle_pos[2] = 0;
    rocket.muzzle_orient[0] = 0;
    rocket.muzzle_orient[1] = 0;
    rocket.muzzle_orient[2] = 0;

    //=============================================//
    //                GRENADE GUN                  //
    //=============================================//
    grenade.item_id = TYPE_GRENADEGUN;
    grenade.shot = SND_GRENADEGUN;
    grenade.draw = SND_UNIVERSAL_HIDE;
    grenade.hide = SND_UNIVERSAL_HIDE;
    grenade.echo = SND_NULL;
    if (getVersion(TR_II_III))
    {
        grenade.reload_1 = SND_TR2_3_GRENADEGUN_RELOAD;
        grenade.reload_2 = SND_TR2_3_GRENADEGUN_LOCK;
    }
    else if (getVersion(TR_IV_V))
    {
        grenade.reload_1 = SND_TR4_C_GRENADEGUN_RELOAD;
        grenade.reload_2 = SND_TR4_C_GRENADEGUN_LOCK;
    }
    grenade.damage_explosion = 40;
    grenade.firerate = RATE_GRENADEGUN;
    grenade.bullet = 1;

    if (getVersion(TR_II_III))
    {
        grenade.current_ammo = AMMO_GRENADE;
    }
    else if (getVersion(TR_IV_V))
    {
        grenade.current_ammo = AMMO_GRENADE_NORMAL;
    }

    grenade.haveGravity = true;
    // smoke is used but no muzzleflash
    grenade.muzzle_duration = 0.0f;
    grenade.muzzle_pos[0] = 0;
    grenade.muzzle_pos[1] = 0;
    grenade.muzzle_pos[2] = 0;
    grenade.muzzle_orient[0] = 0;
    grenade.muzzle_orient[1] = 0;
    grenade.muzzle_orient[2] = 0;

    //=============================================//
    //                HARPOON GUN                  //
    //=============================================//
    harpoon.item_id = TYPE_HARPOONGUN;
    harpoon.shot = (player->move_type == MOVE_UNDERWATER) ? SND_HARPOON_WATER : SND_HARPOON_LAND;
    harpoon.draw = (player->move_type == MOVE_UNDERWATER) ? SND_NULL : SND_UNIVERSAL_HIDE;
    harpoon.hide = (player->move_type == MOVE_UNDERWATER) ? SND_NULL : SND_UNIVERSAL_HIDE;
    harpoon.echo = SND_NULL;
    harpoon.reload_1 = (player->move_type == MOVE_UNDERWATER) ? SND_HARPOON_RELOAD_WATER : SND_HARPOON_RELOAD_LAND;
    harpoon.damage = 10;
    harpoon.damage_explosion = 0;
    harpoon.damage_water = 20;
    harpoon.firerate = RATE_HARPOONGUN;
    harpoon.bullet = 1;
    harpoon.ammo_counter = 4;
    harpoon.current_ammo = AMMO_HARPOON;
    harpoon.onWater = (player->move_type == MOVE_UNDERWATER) ? true : false;
    harpoon.haveGravity = (player->move_type == MOVE_UNDERWATER) ? false : true;
    harpoon.dealDmgAtImpact = false;
    // harpoon have no muzzleflash or smoke

    //=============================================//
    //                CROSSBOW GUN                 //
    //=============================================//
    crossbow.item_id = TYPE_CROSSBOWGUN;
    crossbow.shot = SND_CROSSBOWGUN;
    crossbow.draw = SND_UNIVERSAL_HIDE;
    crossbow.hide = SND_UNIVERSAL_HIDE;
    crossbow.echo = SND_NULL;
    crossbow.reload_1 = SND_UNIVERSAL_RELOAD;
    crossbow.reload_2 = SND_NULL;
    crossbow.damage = 12;
    crossbow.firerate = RATE_CROSSBOWGUN;
    crossbow.bullet = 1;
    crossbow.ammo_counter = 1;
    crossbow.current_ammo = AMMO_CROSSBOW_NORMAL;
    crossbow.onWater = false;
    crossbow.dealDmgAtImpact = false;
    crossbow.haveGravity = false;
    crossbow.itemIsEquipped = false;             // can be equiped of the lasersight
    // same as harpoon

    //=============================================//
    //                GRAPPLIN GUN                 //
    //=============================================//
    grapplin.item_id = TYPE_GRAPPLINGUN;
    // this "weapon" have sound ?
    grapplin.shot = SND_NULL;
    grapplin.draw = SND_NULL;
    grapplin.hide = SND_NULL;
    grapplin.echo = SND_NULL;
    grapplin.reload_1 = SND_NULL;
    grapplin.reload_2 = SND_NULL;
    grapplin.damage = 0;
    grapplin.firerate = RATE_GRAPPINGUN;
    grapplin.bullet = 1;
    grapplin.current_ammo = AMMO_GRAPPLIN;
    grapplin.onWater = false;
    grapplin.dealDmgAtImpact = false;
    grapplin.haveGravity = false;
    // no muzzleflash for this ?
    // same as harpoon and crossbow
}

// if you define (AMMO_UNLIMITED) the item have unlimited ammo
bool checkCanShoot(weapons_s item_id)
{
    entity_s* player = World_GetPlayer();

    if (item_id.current_ammo == AMMO_UNLIMITED)
    {
        return true;
    }
    // ammo are empty when is 0 or go to negative ?
    else if (Inventory_GetItemsCount(player->inventory, item_id.current_ammo) > AMMO_EMPTY)
    {
        return true;
    }

    return false;
}

bool consumeAmmo(weapons_s weapon)
{
    entity_p player = World_GetPlayer();

    // checking if ammo exists before consume !
    if (Inventory_GetItemsCount(player->inventory, weapon.current_ammo) > AMMO_EMPTY)
    {
        // checking if the current ammo is not empty or unlimited !
        if (weapon.current_ammo != AMMO_EMPTY && weapon.current_ammo != AMMO_UNLIMITED)
        {
            // consume item
            Inventory_RemoveItem(&player->inventory, weapon.current_ammo, 1);
            return true;
        }
    }

    // item not found else if not equal to current ammo
    return false;
}

int CurrentWeaponModelToItemID(ss_animation_s* ss_anim)
{
    if (GET_MODEL(TR_MODEL_PISTOL))
    {
        return ITEM_PISTOL;
    }
    else if ((getVersion(TR_II_TO_V)) && GET_MODEL(TR_MODEL_SHOTGUN))
    {
        return ITEM_SHOTGUN;
    }

    if (getVersion(TR_I))  // tomb raider 1
    {
        if (GET_MODEL(TR1_MODEL_SHOTGUN))
        {
            return ITEM_SHOTGUN;
        }
        else if (GET_MODEL(TR1_MODEL_MAGNUM))
        {
            return ITEM_MAGNUMS;
        }
        else if (GET_MODEL(TR1_MODEL_UZI))
        {
            return ITEM_UZIS;
        }
    }
    else if (getVersion(TR_II)) // tomb raider 2
    {
        if (GET_MODEL(TR2_MODEL_AUTOMAGS))
        {
            return ITEM_AUTOMAGS;
        }
        else if (GET_MODEL(TR2_MODEL_UZI))
        {
            return ITEM_UZIS;
        }
        else if (GET_MODEL(TR2_MODEL_M16))
        {
            return ITEM_M16;
        }
        else if (GET_MODEL(TR2_MODEL_GRENADEGUN))
        {
            return ITEM_GRENADEGUN;
        }
        else if (GET_MODEL(TR2_MODEL_HARPOONGUN))
        {
            return ITEM_HARPOONGUN;
        }
    }
    else if (getVersion(TR_III)) // tomb raider 3
    {
        if (GET_MODEL(TR3_MODEL_DESERTEAGLE))
        {
            return ITEM_DESERTEAGLE;
        }
        else if (GET_MODEL(TR3_MODEL_UZI))
        {
            return ITEM_UZIS;
        }
        else if (GET_MODEL(TR3_MODEL_MP5))
        {
            return ITEM_MP5;
        }
        else if (GET_MODEL(TR3_MODEL_ROCKETGUN))
        {
            return ITEM_ROCKETGUN;
        }
        else if (GET_MODEL(TR3_MODEL_GRENADEGUN))
        {
            return ITEM_GRENADEGUN;
        }
        else if (GET_MODEL(TR3_MODEL_HARPOONGUN))
        {
            return ITEM_HARPOONGUN;
        }
    }
    else if (getVersion(TR_IV))         // tomb raider 4 and 5
    {
        if (GET_MODEL(TR4C_MODEL_UZI))
        {
            return ITEM_UZIS;
        }
        else if (GET_MODEL(TR4C_MODEL_CROSSBOW))
        {
            return ITEM_CROSSBOW;
        }
        else if (GET_MODEL(TR4C_MODEL_GRENADEGUN))
        {
            return ITEM_GRENADEGUN;
        }
        else if (GET_MODEL(TR4C_MODEL_REVOLVER))
        {
            return ITEM_REVOLVER;
        }
    }
    else if (getVersion(TR_V))
    {
        if (GET_MODEL(TR4C_MODEL_UZI))
        {
            return ITEM_UZIS;
        }
        else if (GET_MODEL(TR4C_MODEL_CROSSBOW))
        {
            return ITEM_GRAPPLEGUN;  // it's grappling gun !
        }
        else if (GET_MODEL(TR4C_MODEL_GRENADEGUN))
        {
            return ITEM_MP5;         // it's h&k gun !
        }
        else if (GET_MODEL(TR4C_MODEL_REVOLVER))
        {
            return ITEM_REVOLVER;
        }
    }

    // no id found
    return 0;
}

// if ammo is empty and you try to firing.
void AutoSelect(int model_id, ss_animation_s *ss_anim, entity_s* ent, float time)
{
    // needed
    ent->character->state.weapon_ready = 0;

    // animation only for one hand !
    if (ss_anim->model->animation_count == 4)
    {
        Anim_SetAnimation(ss_anim, 2, -1);  // hide and draw
        
        // when animation is finished, change weapon to model_id
        if (Anim_IncTime(ss_anim, time))
        {
            Character_ChangeWeapon(ent, model_id);
        }
    }
    else if (ss_anim->model->animation_count > 4)
    {
        Anim_SetAnimation(ss_anim, 3, 0);   // hide (back)
    }
}

// get the current version without ">" else if "<" !
// - for id use classic define (TR_I, TR_II etc..)
// - for the new define (like TR_IV_V), is in weapons.h (TR_ flag)
// - returned -1 if not found !
int32_t getVersion(int id)
{
    int32_t ver = World_GetVersion();

    switch (id)
    {
    case TR_I:
    case TR_I_DEMO:
    case TR_I_UB:
        return (ver >= TR_I && ver < TR_II);
    case TR_II:
    case TR_II_DEMO:
        return (ver > TR_I_UB && ver < TR_III);
    case TR_III:
        return (ver > TR_II && ver < TR_IV);
    case TR_IV:
    case TR_IV_DEMO:
        return (ver > TR_III && ver < TR_V);
    case TR_V:
        return (ver == TR_V);
    case TR_IV_V:
        return (ver > TR_III && ver <= TR_V);
    case TR_I_II_III:
        return (ver >= TR_I && ver < TR_IV);
    case TR_II_III:
        return (ver >= TR_II && ver < TR_IV);
    case TR_II_TO_V:
        return (ver >= TR_II && ver <= TR_V);
    }

    return -1;
}

void SetCurrentWeaponAnimation(entity_s* ent, ss_animation_s* ss_anim, float time, weapons_s weapon, ss_bone_tag_p b_tag, entity_p target, float* target_pos, bool do_aim, uint16_t targeted_bone_start, uint16_t targeted_bone_end)
{
    b_tag->is_targeted = (target) ? (0x01) : (0x00);

    switch (ss_anim->current_animation)
    {
        case ANIM_IDLE_TO_FIRING:
            OneHand_IdleToFiring(ent, ss_anim, weapon, b_tag, target, do_aim, time);
            break;

        case ANIM_HIDE_TO_IDLE:
            OneHand_HideToIdle(ent, ss_anim, weapon, time);
            break;

        case ANIM_IDLE_AND_HIDE:
            OneHand_HideAndIdle(ent, ss_anim, weapon, time);
            break;

        case ANIM_FIRING:
            if (checkCanShoot(weapon))
            {
                OneHand_Firing(ent, ss_anim, b_tag, target, target_pos, weapon, time, targeted_bone_start, targeted_bone_end, ANIM_FIRING, ANIM_IDLE_TO_FIRING);
            }
            else
            {
                AutoSelect(TR_MODEL_PISTOL, ss_anim, ent, time);
            }
            break;
    }
}

void ShotgunAnim(entity_s* ent, ss_animation_s* ss_anim, float time, weapons_s weapon, ss_bone_tag_p b_tag, entity_p target, float* target_pos, int inc_state, bool do_aim)
{
    switch (ss_anim->current_animation)
    {
        case SHOTGUN_IDLE_TO_FIRING:
            inc_state = Anim_IncTime(ss_anim, (ent->character->state.weapon_ready && do_aim) ? (time) : (-time));
            TwoHand_IdleToFiring(ent, ss_anim, weapon, b_tag, target, do_aim, time, inc_state, SHOTGUN_IDLE_TO_HIDE, SHOTGUN_FIRING);
            break;

        case SHOTGUN_HIDE_TO_IDLE:
            TwoHand_HideToIdle(ent, ss_anim, weapon, time, SHOTGUN_IDLE_TO_FIRING);
            break;

        case SHOTGUN_FIRING:
            if (checkCanShoot(weapon))
            {
                TwoHand_Firing(ent, ss_anim, b_tag, target, target_pos, weapon, time, SHOTGUN_FIRING, SHOTGUN_FIRING_TO_IDLE, SHOTGUN_IDLE_TO_FIRING);
            }
            else
            {
                AutoSelect(TR_MODEL_PISTOL, ss_anim, ent, time);
            }
            break;

        case SHOTGUN_IDLE_TO_HIDE:
            TwoHand_IdleToHide(ent, ss_anim, weapon, time, TW_FRAME_IS_END);
            break;

        case SHOTGUN_FIRING_TO_IDLE:
            TwoHand_FiringToIdle(ent, ss_anim, weapon, time, SHOTGUN_IDLE_TO_FIRING);
            break;
    }
}

void GrenadeGunAnim(entity_s* ent, ss_animation_s* ss_anim, float time, weapons_s weapon, ss_bone_tag_p b_tag, entity_p target, float* target_pos, int inc_state, bool do_aim)
{
    switch (ss_anim->current_animation)
    {
        case GRENADEGUN_IDLE_TO_FIRING:
            inc_state = Anim_IncTime(ss_anim, (ent->character->state.weapon_ready && do_aim) ? (time) : (-time));
            TwoHand_IdleToFiring(ent, ss_anim, weapon, b_tag, target, do_aim, time, inc_state, GRENADEGUN_IDLE_TO_HIDE, GRENADEGUN_FIRING);
            break;

        case GRENADEGUN_HIDE_TO_IDLE:
            TwoHand_HideToIdle(ent, ss_anim, weapon, time, GRENADEGUN_IDLE_TO_FIRING);
            break;

        case GRENADEGUN_FIRING_RELOAD:
            switch (ss_anim->current_frame)
            {
                case 26:
                    if (getVersion(TR_II_III))
                    {
                        Audio_Send(SND_TR2_3_GRENADEGUN_RELOAD, TR_AUDIO_EMITTER_ENTITY, ent->id);
                    }
                    else if (getVersion(TR_IV_V))
                    {
                        Audio_Send(SND_TR4_C_GRENADEGUN_RELOAD, TR_AUDIO_EMITTER_ENTITY, ent->id);
                    }
                    break;

                case 33:
                    if (getVersion(TR_II_III))
                    {
                        Audio_Send(SND_TR2_3_GRENADEGUN_LOCK, TR_AUDIO_EMITTER_ENTITY, ent->id);
                    }
                    else if (getVersion(TR_IV_V))
                    {
                        Audio_Send(SND_TR4_C_GRENADEGUN_LOCK, TR_AUDIO_EMITTER_ENTITY, ent->id);
                    }
                    break;
            }
            
            if (Anim_IncTime(ss_anim, time))
            {
                if (ent->character->cmd.action)
                {
                    Anim_SetAnimation(ss_anim, GRENADEGUN_FIRING, 0);
                }
                else
                {
                    Anim_SetAnimation(ss_anim, GRENADEGUN_FIRING_TO_IDLE, 0);
                }
            }
            break;

        case GRENADEGUN_FIRING:
            if (checkCanShoot(weapon))
            {
                TwoHand_Firing(ent, ss_anim, b_tag, target, target_pos, weapon, time, GRENADEGUN_FIRING_RELOAD, GRENADEGUN_FIRING_TO_IDLE, GRENADEGUN_IDLE_TO_FIRING);
            }
            else
            {
                AutoSelect(TR_MODEL_PISTOL, ss_anim, ent, time);
            }
            break;

        case GRENADEGUN_FIRING_TO_IDLE:
            TwoHand_FiringToIdle(ent, ss_anim, weapon, time, GRENADEGUN_IDLE_TO_FIRING);
            break;

        case GRENADEGUN_IDLE_TO_HIDE:
            TwoHand_IdleToHide(ent, ss_anim, weapon, time, TW_FRAME_GRENADEGUN_IS_END);
            break;
    }
}

void HarpoonAnim(entity_s* ent, ss_animation_s* ss_anim, float time, weapons_s weapon, ss_bone_tag_p b_tag, entity_p target, float* target_pos, int inc_state, bool do_aim)
{
    switch (ss_anim->current_animation)
    {
        case HARPOON_LAND_IDLE_TO_FIRING:
            if (ent->move_type == MOVE_UNDERWATER)
            {
                Anim_SetAnimation(ss_anim, HARPOON_WATER_IDLE_TO_FIRING, 0);
            }
            else
            {
                inc_state = Anim_IncTime(ss_anim, (ent->character->state.weapon_ready && do_aim) ? (time) : (-time));
                TwoHand_IdleToFiring(ent, ss_anim, weapon, b_tag, target, do_aim, time, inc_state, HARPOON_LAND_IDLE_TO_HIDE, HARPOON_LAND_FIRING);
            }
            break;

        case HARPOON_LAND_HIDE_TO_IDLE:
            if (ent->move_type == MOVE_UNDERWATER)
            {
                TwoHand_HideToIdle(ent, ss_anim, weapon, time, HARPOON_WATER_IDLE_TO_FIRING);
            }
            else
            {
                TwoHand_HideToIdle(ent, ss_anim, weapon, time, HARPOON_LAND_IDLE_TO_FIRING);
            }
            break;

        case HARPOON_LAND_FIRING:
            if (checkCanShoot(weapon))
            {
                TwoHand_Firing(ent, ss_anim, b_tag, target, target_pos, weapon, time, HARPOON_LAND_FIRING, HARPOON_LAND_FIRING_TO_IDLE, HARPOON_LAND_IDLE_TO_FIRING);
            }
            else
            {
                AutoSelect(TR_MODEL_PISTOL, ss_anim, ent, time);
            }
            break;

        case HARPOON_LAND_IDLE_TO_HIDE:
            TwoHand_IdleToHide(ent, ss_anim, weapon, time, TW_FRAME_IS_END);
            break;

        case HARPOON_LAND_FIRING_TO_IDLE:
            TwoHand_FiringToIdle(ent, ss_anim, weapon, time, HARPOON_LAND_IDLE_TO_FIRING);
            break;

        case HARPOON_RELOAD:
            if (Anim_IncTime(ss_anim, time))
            {
                if (ent->move_type == MOVE_UNDERWATER)
                {
                    Anim_SetAnimation(ss_anim, HARPOON_WATER_IDLE_TO_FIRING, 0);
                }
                else
                {
                    Anim_SetAnimation(ss_anim, HARPOON_LAND_IDLE_TO_FIRING, 0);
                }
            }
            break;

        case HARPOON_WATER_IDLE_TO_FIRING:
            inc_state = Anim_IncTime(ss_anim, (ent->character->state.weapon_ready && do_aim) ? (time) : (-time));
            TwoHand_IdleToFiring(ent, ss_anim, weapon, b_tag, target, do_aim, time, inc_state, HARPOON_WATER_IDLE_TO_HIDE, HARPOON_WATER_FIRING);
            break;

        case HARPOON_WATER_FIRING_TO_IDLE:
            TwoHand_FiringToIdle(ent, ss_anim, weapon, time, HARPOON_WATER_IDLE_TO_FIRING);
            break;

        case HARPOON_WATER_FIRING:
            if (checkCanShoot(weapon))
            {
                TwoHand_Firing(ent, ss_anim, b_tag, target, target_pos, weapon, time, HARPOON_WATER_FIRING, HARPOON_WATER_FIRING_TO_IDLE, HARPOON_WATER_IDLE_TO_FIRING);
            }
            else
            {
                AutoSelect(TR_MODEL_PISTOL, ss_anim, ent, time);
            }
            break;

        case HARPOON_WATER_IDLE_TO_HIDE:
            TwoHand_IdleToHide(ent, ss_anim, weapon, time, TW_FRAME_IS_END);
            break;
    }
}

void MP5Anim(entity_s* ent, ss_animation_s* ss_anim, float time, weapons_s weapon, ss_bone_tag_p b_tag, entity_p target, float* target_pos, int inc_state, bool do_aim)
{
    switch (ss_anim->current_animation)
    {
        case MP5_IDLE_TO_FIRING:
            inc_state = Anim_IncTime(ss_anim, (ent->character->state.weapon_ready && do_aim) ? (time) : (-time));
            TwoHand_IdleToFiring(ent, ss_anim, weapon, b_tag, target, do_aim, time, inc_state, MP5_IDLE_TO_HIDE, MP5_FIRING);
            break;

        case MP5_HIDE_TO_IDLE:
            TwoHand_HideToIdle(ent, ss_anim, weapon, time, MP5_IDLE_TO_FIRING);
            break;

        case MP5_FIRING:
            if (checkCanShoot(weapon))
            {
                TwoHand_Firing(ent, ss_anim, b_tag, target, target_pos, weapon, time, MP5_FIRING, MP5_FIRING_TO_IDLE, MP5_IDLE_TO_FIRING);
            }
            else
            {
                AutoSelect(TR_MODEL_PISTOL, ss_anim, ent, time);
            }
            break;

        case MP5_IDLE_TO_HIDE:
            TwoHand_IdleToHide(ent, ss_anim, weapon, time, TW_FRAME_IS_END);
            break;

        case MP5_FIRING_TO_IDLE:
            TwoHand_FiringToIdle(ent, ss_anim, weapon, time, MP5_IDLE_TO_FIRING);
            break;
    }
}

void M16Anim(entity_s* ent, ss_animation_s* ss_anim, float time, weapons_s weapon, ss_bone_tag_p b_tag, entity_p target, float* target_pos, int inc_state, bool do_aim)
{
    switch (ss_anim->current_animation)
    {
        case M16_IDLE_TO_FIRING:
            inc_state = Anim_IncTime(ss_anim, (ent->character->state.weapon_ready && do_aim) ? (time) : (-time));
            TwoHand_IdleToFiring(ent, ss_anim, weapon, b_tag, target, do_aim, time, inc_state, M16_IDLE_TO_HIDE, M16_FIRING);
            break;

        case M16_HIDE_TO_IDLE:
            TwoHand_HideToIdle(ent, ss_anim, weapon, time, M16_IDLE_TO_FIRING);
            break;

        case M16_FIRING:
            if (checkCanShoot(weapon))
            {
                TwoHand_Firing(ent, ss_anim, b_tag, target, target_pos, weapon, time, M16_FIRING, M16_FIRING_TO_IDLE, M16_IDLE_TO_FIRING);
            }
            else
            {
                AutoSelect(TR_MODEL_PISTOL, ss_anim, ent, time);
            }
            break;

        case M16_IDLE_TO_HIDE:
            TwoHand_IdleToHide(ent, ss_anim, weapon, time, TW_FRAME_IS_END);
            break;

        case M16_FIRING_TO_IDLE:
            TwoHand_FiringToIdle(ent, ss_anim, weapon, time, M16_IDLE_TO_FIRING);
            break;
    }
}

void RocketGunAnim(entity_s* ent, ss_animation_s* ss_anim, float time, weapons_s weapon, ss_bone_tag_p b_tag, entity_p target, float* target_pos, int inc_state, bool do_aim)
{
    switch (ss_anim->current_animation)
    {
        case ROCKETGUN_IDLE_TO_FIRING:
            inc_state = Anim_IncTime(ss_anim, (ent->character->state.weapon_ready && do_aim) ? (time) : (-time));
            TwoHand_IdleToFiring(ent, ss_anim, weapon, b_tag, target, do_aim, time, inc_state, ROCKETGUN_IDLE_TO_HIDE, ROCKETGUN_FIRING);
            break;

        case ROCKETGUN_HIDE_TO_IDLE:
            TwoHand_HideToIdle(ent, ss_anim, weapon, time, ROCKETGUN_IDLE_TO_FIRING);
            break;

        case ROCKETGUN_FIRING:
            if (checkCanShoot(weapon))
            {
                TwoHand_Firing(ent, ss_anim, b_tag, target, target_pos, weapon, time, ROCKETGUN_FIRING, ROCKETGUN_FIRING_TO_IDLE, ROCKETGUN_IDLE_TO_FIRING);
            }
            else
            {
                AutoSelect(TR_MODEL_PISTOL, ss_anim, ent, time);
            }
            break;

        case ROCKETGUN_IDLE_TO_HIDE:
            TwoHand_IdleToHide(ent, ss_anim, weapon, time, TW_FRAME_IS_END);
            break;

        case ROCKETGUN_FIRING_TO_IDLE:
            TwoHand_FiringToIdle(ent, ss_anim, weapon, time, ROCKETGUN_IDLE_TO_FIRING);
            break;
    }
}

void CrossbowAnim(entity_s* ent, ss_animation_s* ss_anim, float time, weapons_s weapon, ss_bone_tag_p b_tag, entity_p target, float* target_pos, int inc_state, bool do_aim)
{
    switch (ss_anim->current_animation)
    {
        case CROSSBOW_IDLE_TO_FIRING:
            inc_state = Anim_IncTime(ss_anim, (ent->character->state.weapon_ready && do_aim) ? (time) : (-time));
            TwoHand_IdleToFiring(ent, ss_anim, weapon, b_tag, target, do_aim, time, inc_state, CROSSBOW_IDLE_TO_HIDE, CROSSBOW_FIRING);
            break;

        case CROSSBOW_HIDE_TO_IDLE:
            TwoHand_HideToIdle(ent, ss_anim, weapon, time, CROSSBOW_IDLE_TO_FIRING);
            break;

        case CROSSBOW_FIRING:
            if (checkCanShoot(weapon))
            {
                TwoHand_Firing(ent, ss_anim, b_tag, target, target_pos, weapon, time, CROSSBOW_FIRING, CROSSBOW_FIRING_TO_IDLE, CROSSBOW_IDLE_TO_FIRING);
            }
            else
            {
                AutoSelect(TR_MODEL_PISTOL, ss_anim, ent, time);
            }
            break;
        case CROSSBOW_IDLE_TO_HIDE:
            TwoHand_IdleToHide(ent, ss_anim, weapon, time, TW_FRAME_IS_END);
            break;

        case CROSSBOW_FIRING_TO_IDLE:
            TwoHand_FiringToIdle(ent, ss_anim, weapon, time, CROSSBOW_IDLE_TO_FIRING);
            break;
    }
}

void GrapplinGunAnim(entity_s* ent, ss_animation_s* ss_anim, float time, weapons_s weapon, ss_bone_tag_p b_tag, entity_p target, float* target_pos, int inc_state, bool do_aim)
{
    switch (ss_anim->current_animation)
    {
        case GRAPPLING_IDLE_TO_FIRING:
            inc_state = Anim_IncTime(ss_anim, (ent->character->state.weapon_ready && do_aim) ? (time) : (-time));
            TwoHand_IdleToFiring(ent, ss_anim, weapon, b_tag, target, do_aim, time, inc_state, GRAPPLING_IDLE_TO_HIDE, GRAPPLING_FIRING);
            break;

        case GRAPPLING_HIDE_TO_IDLE:
            TwoHand_HideToIdle(ent, ss_anim, weapon, time, GRAPPLING_IDLE_TO_FIRING);
            break;

        case GRAPPLING_FIRING:
            if (checkCanShoot(weapon))
            {
                TwoHand_Firing(ent, ss_anim, b_tag, target, target_pos, weapon, time, GRAPPLING_FIRING, GRAPPLING_FIRING_TO_IDLE, GRAPPLING_IDLE_TO_FIRING);
            }
            else
            {
                AutoSelect(TR_MODEL_PISTOL, ss_anim, ent, time);
            }
            break;

        case GRAPPLING_IDLE_TO_HIDE:
            TwoHand_IdleToHide(ent, ss_anim, weapon, time, TW_FRAME_IS_END);
            break;

        case GRAPPLING_FIRING_TO_IDLE:
            TwoHand_FiringToIdle(ent, ss_anim, weapon, time, GRAPPLING_IDLE_TO_FIRING);
            break;
    }
}

void OneHand_IdleToFiring(entity_s* ent, ss_animation_s* ss_anim, weapons_s weapon, ss_bone_tag_p b_tag, entity_p target, bool do_aim, float time)
{
    int inc_state = Anim_IncTime(ss_anim, (ent->character->state.weapon_ready && do_aim) ? (time) : (-time));

    if ((inc_state == ARMED) && ent->character->state.weapon_ready && ent->character->cmd.action)
    {
        ///@FIXME: animation launch one time and do nothing (sound, ammo consume, shoot)
        Anim_SetAnimation(ss_anim, ANIM_FIRING, 0);
    }
    else if (ent->character->state.weapon_ready && !ent->character->cmd.action && (-time))
    {
        if (ss_anim->current_frame == 3)
        {
            Audio_Kill(weapon.shot, TR_AUDIO_EMITTER_ENTITY, ent->id);

            if (getVersion(TR_II) && GET_MODEL(TR2_MODEL_UZI) && ANIM_REVERSE)
            {
                Audio_Send(weapon.echo, TR_AUDIO_EMITTER_ENTITY, ent->id);
            }
            else if (getVersion(TR_III) && GET_MODEL(TR3_MODEL_UZI) && ANIM_REVERSE)
            {
                Audio_Send(weapon.echo, TR_AUDIO_EMITTER_ENTITY, ent->id);
            }
            else if (getVersion(TR_IV_V) && GET_MODEL(TR4C_MODEL_UZI) && ANIM_REVERSE)
            {
                Audio_Send(weapon.echo, TR_AUDIO_EMITTER_ENTITY, ent->id);
            }
        }
    }
    else if ((inc_state == UNARMED) && !ent->character->state.weapon_ready)
    {
        // fix exit fire loop bug (after pressing 
        Audio_Kill(weapon.shot, TR_AUDIO_EMITTER_ENTITY, ent->id);
        Anim_SetAnimation(ss_anim, ANIM_IDLE_AND_HIDE, -1);
    }
}

void TwoHand_IdleToFiring(entity_s* ent, ss_animation_s* ss_anim, weapons_s weapon, ss_bone_tag_p b_tag, entity_p target, bool do_aim, float time, int inc_state, int anim_hide, int anim_firing)
{
    b_tag->is_targeted = (target) ? (0x01) : (0x00);

    if ((inc_state == ARMED) && ent->character->state.weapon_ready && ent->character->cmd.action)
    {
        Anim_SetAnimation(ss_anim, anim_firing, 0);
    }
    else if ((inc_state == UNARMED) && !ent->character->state.weapon_ready)
    {
        // fix exit fire loop bug
        Audio_Kill(weapon.shot, TR_AUDIO_EMITTER_ENTITY, ent->id);
        Anim_SetAnimation(ss_anim, anim_hide, 0);
    }
}

void TwoHand_FiringToIdle(entity_s* ent, ss_animation_s* ss_anim, weapons_s weapon, float time, int anim_idle_to_firing)
{
    if (ss_anim->current_frame == 1)
    {
        Audio_Kill(weapon.shot, TR_AUDIO_EMITTER_ENTITY, ent->id);
    }
    else if (ss_anim->current_frame == 3 && (ss_anim->anim_frame_flags == ANIM_FRAME_REVERSE))
    {
        if (getVersion(TR_II) && GET_MODEL(TR2_MODEL_M16))
        {
            Audio_Send(weapon.echo, TR_AUDIO_EMITTER_ENTITY, ent->id);
        }
    }
    
    if (Anim_IncTime(ss_anim, time))
    {
        Anim_SetAnimation(ss_anim, anim_idle_to_firing, 0);
    }
}

void TwoHand_IdleToHide(entity_s* ent, ss_animation_s* ss_anim, weapons_s weapon, float time, int frame_to_hide)
{
    if (Anim_IncTime(ss_anim, time))
    {
        SSBoneFrame_DisableOverrideAnim(ent->bf, ss_anim);

        // fix animation of two hand if weapon have no ammo (AutoSelect not work with Two Hand after Hide)
        // when animation is finished and if weapon not have ammo, change weapon to model_id
        if (Inventory_GetItemsCount(ent->inventory, weapon.current_ammo) <= 0)
        {
            Character_ChangeWeapon(ent, TR_MODEL_PISTOL);
        }
    }

    if ((ss_anim->frame_changing_state >= 0x01) && (ss_anim->prev_frame == frame_to_hide))
    {
        if (getVersion(TR_I))
            StateControl_SetWeaponMeshOn(ent->bf, ss_anim->model, TR1_BACK_WEAPON);
        else
            StateControl_SetWeaponMeshOn(ent->bf, ss_anim->model, TR2_BACK_WEAPON);

        StateControl_SetWeaponMeshOff(ent->bf, HAND_RIGHT);
        Audio_Send(weapon.hide, TR_AUDIO_EMITTER_ENTITY, ent->id);
    }
}

void OneHand_HideToIdle(entity_s* ent, ss_animation_s* ss_anim, weapons_s weapon, float time)
{
    int inc_state = Anim_IncTime(ss_anim, (ent->character->state.weapon_ready) ? (time) : (-time));

    if ((inc_state == ARMED) && ent->character->state.weapon_ready)
    {
        StateControl_SetWeaponMeshOn(ent->bf, ss_anim->model, HAND_RIGHT);
        StateControl_SetWeaponMeshOn(ent->bf, ss_anim->model, HAND_LEFT);
        StateControl_SetWeaponMeshOff(ent->bf, HOLSTER_RIGHT);
        StateControl_SetWeaponMeshOff(ent->bf, HOLSTER_LEFT);
        Anim_SetAnimation(ss_anim, ANIM_IDLE_AND_HIDE, 0);
        Audio_Send(weapon.draw, TR_AUDIO_EMITTER_ENTITY, ent->id);
    }
    else if ((inc_state == UNARMED) && !ent->character->state.weapon_ready)
    {
        SSBoneFrame_DisableOverrideAnim(ent->bf, ss_anim);
        ent->character->state.weapon_ready = 0;
    }
}

void TwoHand_HideToIdle(entity_s* ent, ss_animation_s* ss_anim, weapons_s weapon, float time, int anim_idle)
{
    if (Anim_IncTime(ss_anim, time))
    {
        Anim_SetAnimation(ss_anim, anim_idle, 0);  // to idle
    }

    if ((ss_anim->frame_changing_state >= 0x01) && (ss_anim->prev_frame == TW_FRAME))
    {
        StateControl_SetWeaponMeshOn(ent->bf, ss_anim->model, HAND_RIGHT);
        if (getVersion(TR_I))
        {
            StateControl_SetWeaponMeshOff(ent->bf, TR1_BACK_WEAPON);
        }
        else
        {
            StateControl_SetWeaponMeshOff(ent->bf, TR2_BACK_WEAPON);
        }
        Audio_Send(weapon.draw, TR_AUDIO_EMITTER_ENTITY, ent->id);
    }
}

void OneHand_HideAndIdle(entity_s * ent, ss_animation_s * ss_anim, weapons_s weapon, float time)
{
    int inc_state = Anim_IncTime(ss_anim, (ent->character->state.weapon_ready) ? (time) : (-time));

    if ((inc_state == ARMED) && ent->character->state.weapon_ready)
    {
        Anim_SetAnimation(ss_anim, ANIM_IDLE_TO_FIRING, 0);
    }
    else if ((inc_state == UNARMED) && !ent->character->state.weapon_ready)
    {
        StateControl_SetWeaponMeshOn(ent->bf, ss_anim->model, 1);
        StateControl_SetWeaponMeshOn(ent->bf, ss_anim->model, 4);
        StateControl_SetWeaponMeshOff(ent->bf, 10);
        StateControl_SetWeaponMeshOff(ent->bf, 13);
        Anim_SetAnimation(ss_anim, ANIM_HIDE_TO_IDLE, -1);
        Audio_Send(weapon.hide, TR_AUDIO_EMITTER_ENTITY, ent->id);
    }
}

void TwoHand_Firing(entity_s* ent, ss_animation_s* ss_anim, ss_bone_tag_p b_tag, entity_p target, float* target_pos, weapons_s weapon, float time, int anim_firing, int anim_firing_to_idle, int anim_idle)
{
    if ((ss_anim->frame_changing_state >= 4) | Anim_IncTime(ss_anim, time * weapon.firerate))
    {
        if (ent->character->state.weapon_ready && ent->character->cmd.action)
        {
            collision_result_t cs;
            float from[3], to[3], tr[16], dir[3], t;
            ss_bone_tag_p bt = ent->bf->bone_tags + ent->character->bone_r_hand_end;

            Anim_SetAnimation(ss_anim, anim_firing, 0);
            ss_anim->frame_changing_state = 0x01;
            Audio_Send(weapon.shot, TR_AUDIO_EMITTER_ENTITY, ent->id);
            consumeAmmo(weapon);

            Mat4_Mat4_mul(tr, ent->transform.M4x4, bt->current_transform);
            vec3_copy(from, tr + 12);

            if (target && (bt->mod.current_slerp) > 0.99f)
            {
                vec3_sub(dir, target_pos, from);
                vec3_norm(dir, t);
            }
            else
            {
                vec3_copy_inv(dir, tr + 8);
            }

            // adding range
            vec3_add_mul(to, from, dir, weapon.range);

            for (int i = 1; i <= weapon.bullet; ++i)
            {
                //vec3_copy(d_from, from);
                //vec3_copy(d_to, to);
                t = (weapon.range * i) / weapon.bullet;
                vec3_add_mul(to, from, dir, t);
                t = 8.0f * i;

                switch (i % 4)
                {
                    case 0: vec3_add_mul(to, to, tr + 0, t); break;
                    case 1: vec3_add_mul(to, to, tr + 4, t); break;
                    case 2: vec3_add_mul(to, to, tr + 0, -t); break;
                    case 3: vec3_add_mul(to, to, tr + 4, -t); break;
                }

                if (Physics_RayTest(&cs, from, to, ent->self, COLLISION_FILTER_CHARACTER) && cs.obj && (cs.obj->object_type == OBJECT_ENTITY))
                {
                    target = (entity_p)cs.obj->object;
                    Script_ExecEntity(engine_lua, ENTITY_CALLBACK_SHOOT, ent->id, target->id);
                }
            }
        }
        else if (target)
        {
            // if you have a target -> lock the enemie
            Anim_SetAnimation(ss_anim, anim_idle, -1);
        }
        else
        {
            // to idle animation
            Anim_SetAnimation(ss_anim, anim_firing_to_idle, 0);
        }
    }

    if ((ss_anim->frame_changing_state == 0x01) && (weapon.reload_1))
    {
        if (ss_anim->prev_frame == 2)
        {
            Audio_Send(weapon.reload_1, TR_AUDIO_EMITTER_ENTITY, ent->id);
        }
    }
}

void OneHand_Firing(entity_s* ent, ss_animation_s* ss_anim, ss_bone_tag_p b_tag, entity_p target, float* target_pos, weapons_s weapon, float time, uint16_t targeted_bone_start, uint16_t targeted_bone_end, int anim_firing, int anim_firing_to_idle)
{
    if ((ss_anim->frame_changing_state >= 4) | Anim_IncTime(ss_anim, time * weapon.firerate))
    {
        if (ent->character->state.weapon_ready && ent->character->cmd.action)
        {
            collision_result_t cs;
            float from[3], to[3], tr[16];
            ss_bone_tag_p bt = ent->bf->bone_tags + targeted_bone_start;

            Anim_SetAnimation(ss_anim, anim_firing, 0);
            ss_anim->frame_changing_state = 0x01;
            Audio_Send(weapon.shot, TR_AUDIO_EMITTER_ENTITY, ent->id);
            consumeAmmo(weapon);

            Mat4_Mat4_mul(tr, ent->transform.M4x4, bt->current_transform);
            Mat4_vec3_mul(from, ent->transform.M4x4, ent->bf->bone_tags[targeted_bone_end].current_transform + 12);

            if (target && (bt->mod.current_slerp > 0.99))
            {
                vec3_copy(to, bt->mod.target_pos);
            }
            else
            {
                vec3_add_mul(to, from, tr + 8, -32768.0f);
            }

            if (Physics_RayTest(&cs, from, to, ent->self, COLLISION_FILTER_CHARACTER) && cs.obj && (cs.obj->object_type == OBJECT_ENTITY))
            {
                target = (entity_p)cs.obj->object;
                Script_ExecEntity(engine_lua, ENTITY_CALLBACK_SHOOT, ent->id, target->id);
            }
        }
        else
        {
            Anim_SetAnimation(ss_anim, anim_firing_to_idle, -1);
        }
    }
}

struct weapons_s getPistol()
{
    return pistol;
}

struct weapons_s getShotgun()
{
    return shotgun;
}

struct weapons_s getMagnum()
{
    return magnum;
}

struct weapons_s getUzi()
{
    return uzi;
}

struct weapons_s getAutomags()
{
    return automags;
}

struct weapons_s getDesertEagle()
{
    return deserteagle;
}

struct weapons_s getRevolver()
{
    return revolver;
}

struct weapons_s getM16()
{
    return m16;
}

struct weapons_s getMP5()
{
    return mp5;
}

struct weapons_s getRocketGun()
{
    return rocket;
}

struct weapons_s getHarpoonGun()
{
    return harpoon;
}

struct weapons_s getGrenadeGun()
{
    return grenade;
}

struct weapons_s getCrossbowGun()
{
    return crossbow;
}

struct weapons_s getGrapplinGun()
{
    return grapplin;
}
