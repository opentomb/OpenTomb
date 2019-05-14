
#include "audio/audio.h"
#include "controls.h"
#include "inventory.h"
#include "entity.h"
#include "character_controller.h"
#include "world.h"

#include "state_control/state_control_Lara.h"

#include "weapons.h"

#define SND_NULL (-1)


struct weapons_s getPistol()
{
    static struct weapons_s pistol;
    pistol.item_id = ITEM_PISTOL;
    pistol.shot = TR_AUDIO_SOUND_SHOTPISTOLS;
    pistol.draw = TR_AUDIO_SOUND_HOLSTEROUT;
    pistol.hide = TR_AUDIO_SOUND_HOLSTERIN;
    pistol.echo = SND_NULL;
    pistol.reload_1 = TR_AUDIO_SOUND_RELOAD;
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
    return pistol;
}

struct weapons_s getShotgun()
{
    static struct weapons_s shotgun;
    int32_t ver = World_GetVersion();
    shotgun.item_id = ITEM_SHOTGUN;
    shotgun.shot = TR_AUDIO_SOUND_SHOTSHOTGUN;
    shotgun.draw = TR_AUDIO_SOUND_HOLSTEROUT;
    shotgun.hide = TR_AUDIO_SOUND_HOLSTERIN;
    shotgun.echo = SND_NULL;
    shotgun.reload_1 = TR_AUDIO_SOUND_RELOAD;
    shotgun.reload_2 = SND_NULL;
    shotgun.damage = 3;
    shotgun.firerate = RATE_SHOTGUN;
    shotgun.bullet = (shotgun.current_ammo == ITEM_SHOTGUN_WIDESHOT_AMMO) ? 5 : 6; // wide have 5 bullet

    if(ver < TR_IV)
    {
        shotgun.current_ammo = ITEM_SHOTGUN_AMMO;
    }
    else if((ver >= TR_IV))
    {
        shotgun.current_ammo = ITEM_SHOTGUN_NORMAL_AMMO;
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
    return shotgun;
}

struct weapons_s getMagnum()
{
    static struct weapons_s magnum;
    magnum.item_id = ITEM_MAGNUMS;
    magnum.shot = TR_AUDIO_SOUND_SHOTMAGNUM;
    magnum.draw = TR_AUDIO_SOUND_HOLSTEROUT;
    magnum.hide = TR_AUDIO_SOUND_HOLSTERIN;
    magnum.echo = SND_NULL;
    magnum.reload_1 = TR_AUDIO_SOUND_RELOAD;
    magnum.reload_2 = SND_NULL;
    magnum.damage = 21;
    magnum.firerate = RATE_MAGNUM;
    magnum.bullet = 2;
    magnum.current_ammo = ITEM_MAGNUM_AMMO;
    magnum.muzzle_duration = 3.0f;
    magnum.muzzle_pos[0] = 0;
    magnum.muzzle_pos[1] = 0;
    magnum.muzzle_pos[2] = 0;
    magnum.muzzle_orient[0] = 0;
    magnum.muzzle_orient[1] = 0;
    magnum.muzzle_orient[2] = 0;
    return magnum;
}

struct weapons_s getUzi()
{
    static struct weapons_s uzi;
    int32_t ver = World_GetVersion();
    uzi.item_id = ITEM_UZIS;
    uzi.shot = TR_AUDIO_SOUND_SHOTUZI;
    uzi.draw = TR_AUDIO_SOUND_HOLSTEROUT;
    uzi.hide = TR_AUDIO_SOUND_HOLSTERIN;
    uzi.echo = (!IS_TR_I(ver)) ? TR_AUDIO_SOUND_SHOTUZI_END : SND_NULL;
    uzi.reload_1 = TR_AUDIO_SOUND_RELOAD;
    uzi.reload_2 = SND_NULL;
    uzi.damage = 1;
    uzi.firerate = RATE_UZI;
    uzi.bullet = 2;
    uzi.current_ammo = ITEM_UZI_AMMO;
    uzi.muzzle_duration = 1.0f;
    uzi.muzzle_pos[0] = 0;
    uzi.muzzle_pos[1] = 0;
    uzi.muzzle_pos[2] = 0;
    uzi.muzzle_orient[0] = 0;
    uzi.muzzle_orient[1] = 0;
    uzi.muzzle_orient[2] = 0;
    return uzi;
}

struct weapons_s getAutomags()
{
    static struct weapons_s automags;
    automags.item_id = ITEM_AUTOMAGS;
    automags.shot = TR_AUDIO_SOUND_SHOTAUTOMAGS;
    automags.draw = TR_AUDIO_SOUND_HOLSTEROUT;
    automags.hide = TR_AUDIO_SOUND_HOLSTERIN;
    automags.echo = SND_NULL;
    automags.reload_1 = TR_AUDIO_SOUND_RELOAD;
    automags.reload_2 = SND_NULL;
    automags.damage = 12;
    automags.firerate = RATE_AUTOMAGS;
    automags.bullet = 2;
    automags.current_ammo = ITEM_AUTOMAGS_AMMO;
    automags.muzzle_duration = 3.0f;
    automags.muzzle_pos[0] = 0;
    automags.muzzle_pos[1] = 0;
    automags.muzzle_pos[2] = 0;
    automags.muzzle_orient[0] = 0;
    automags.muzzle_orient[1] = 0;
    automags.muzzle_orient[2] = 0;
    return automags;
}

struct weapons_s getDesertEagle()
{
    static struct weapons_s deserteagle;
    deserteagle.item_id = ITEM_DESERTEAGLE;
    deserteagle.shot = TR_AUDIO_SOUND_SHOTDESERTEAGLE;
    deserteagle.draw = TR_AUDIO_SOUND_HOLSTEROUT;
    deserteagle.hide = TR_AUDIO_SOUND_HOLSTERIN;
    deserteagle.echo = SND_NULL;
    deserteagle.reload_1 = TR_AUDIO_SOUND_RELOAD;
    deserteagle.reload_2 = SND_NULL;
    deserteagle.damage = 21;
    deserteagle.firerate = RATE_DESERTEAGLE;
    deserteagle.bullet = 1;
    deserteagle.current_ammo = ITEM_DESERTEAGLE_AMMO;
    deserteagle.muzzle_duration = 3.0f;
    deserteagle.muzzle_pos[0] = 0;
    deserteagle.muzzle_pos[1] = 0;
    deserteagle.muzzle_pos[2] = 0;
    deserteagle.muzzle_orient[0] = 0;
    deserteagle.muzzle_orient[1] = 0;
    deserteagle.muzzle_orient[2] = 0;
    return deserteagle;
}

struct weapons_s getRevolver()
{
    static struct weapons_s revolver;
    revolver.item_id = ITEM_REVOLVER;
    revolver.shot = TR_AUDIO_SOUND_SHOTREVOLVER;
    revolver.draw = TR_AUDIO_SOUND_HOLSTEROUT;
    revolver.hide = TR_AUDIO_SOUND_HOLSTERIN;
    revolver.echo = SND_NULL;
    revolver.reload_1 = TR_AUDIO_SOUND_RELOAD;
    revolver.reload_2 = SND_NULL;
    revolver.damage = 21;
    revolver.firerate = RATE_REVOLVER;
    revolver.bullet = 1;
    revolver.current_ammo = ITEM_REVOLVER_AMMO;
    revolver.itemIsEquipped = false;         // can be equiped of the lasersight !
    revolver.muzzle_duration = 3.0f;
    revolver.muzzle_pos[0] = 0;
    revolver.muzzle_pos[1] = 0;
    revolver.muzzle_pos[2] = 0;
    revolver.muzzle_orient[0] = 0;
    revolver.muzzle_orient[1] = 0;
    revolver.muzzle_orient[2] = 0;
    return revolver;
}

struct weapons_s getM16()
{
    static struct weapons_s m16;
    m16.item_id = ITEM_M16;
    m16.shot = TR_AUDIO_SOUND_SHOTM16;
    m16.draw = TR_AUDIO_SOUND_HOLSTERIN;
    m16.hide = SND_NULL;
    m16.echo = TR_AUDIO_SOUND_SHOTM16_END;
    m16.reload_1 = SND_NULL;
    m16.reload_2 = SND_NULL;
    m16.damage = 12;
    m16.alternateAim = false;           // define it at animation (M16Anim()) ?
    m16.firerate = (m16.alternateAim) ? RATE_M16_ALT : RATE_M16;
    m16.bullet = 1;
    m16.current_ammo = ITEM_M16_AMMO;
    m16.muzzle_duration = 3.0f;
    m16.muzzle_pos[0] = 0;
    m16.muzzle_pos[1] = 0;
    m16.muzzle_pos[2] = 0;
    m16.muzzle_orient[0] = 0;
    m16.muzzle_orient[1] = 0;
    m16.muzzle_orient[2] = 0;
    return m16;
}

struct weapons_s getMP5()
{
    static struct weapons_s mp5;
    mp5.item_id = ITEM_MP5;
    mp5.shot = TR_AUDIO_SOUND_SHOTMP5;
    mp5.draw = TR_AUDIO_SOUND_HOLSTERIN;
    mp5.hide = TR_AUDIO_SOUND_HOLSTERIN;
    mp5.echo = SND_NULL;
    mp5.reload_1 = SND_NULL;
    mp5.reload_2 = SND_NULL;
    mp5.damage = 12;
    mp5.damage_explosion = 0;
    mp5.alternateAim = false;
    mp5.firerate = (mp5.alternateAim) ? RATE_MP5_ALT : RATE_MP5;
    mp5.bullet = 1;
    mp5.current_ammo = ITEM_MP5_AMMO;
    mp5.muzzle_pos[0] = 0;
    mp5.muzzle_pos[1] = 0;
    mp5.muzzle_pos[2] = 0;
    mp5.muzzle_orient[0] = 0;
    mp5.muzzle_orient[1] = 0;
    mp5.muzzle_orient[2] = 0;
    return mp5;
}

struct weapons_s getRocketGun()
{
    static struct weapons_s rocket;
    rocket.item_id = ITEM_ROCKETGUN;
    rocket.shot = TR_AUDIO_SOUND_SHOTROCKETGUN;
    rocket.draw = TR_AUDIO_SOUND_HOLSTERIN;
    rocket.hide = TR_AUDIO_SOUND_HOLSTERIN;
    rocket.echo = SND_NULL;
    rocket.reload_1 = TR_AUDIO_SOUND_RELOADROCKETGUN;
    rocket.reload_2 = SND_NULL;
    rocket.damage_explosion = 100;
    rocket.firerate = RATE_ROCKETGUN;
    rocket.bullet = 1;
    rocket.current_ammo = ITEM_ROCKETGUN_AMMO;
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
    return rocket;
}

struct weapons_s getHarpoonGun(bool is_underwater)
{
    static struct weapons_s harpoon;
    harpoon.item_id = ITEM_HARPOONGUN;
    harpoon.shot = (is_underwater) ? TR_AUDIO_SOUND_SHOTHARPOON_W : TR_AUDIO_SOUND_SHOTHARPOON_G;
    harpoon.draw = (is_underwater) ? SND_NULL : TR_AUDIO_SOUND_HOLSTERIN;
    harpoon.hide = (is_underwater) ? SND_NULL : TR_AUDIO_SOUND_HOLSTERIN;
    harpoon.echo = SND_NULL;
    harpoon.reload_1 = (is_underwater) ? TR_AUDIO_SOUND_RELOADHARPOON_W : TR_AUDIO_SOUND_RELOADHARPOON_G;
    harpoon.damage = 10;
    harpoon.damage_explosion = 0;
    harpoon.damage_water = 20;
    harpoon.firerate = RATE_HARPOONGUN;
    harpoon.bullet = 1;
    harpoon.ammo_counter = 4;
    harpoon.current_ammo = ITEM_HARPOONGUN_AMMO;
    harpoon.onWater = (is_underwater) ? true : false;
    harpoon.haveGravity = (is_underwater) ? false : true;
    harpoon.dealDmgAtImpact = false;
    // harpoon have no muzzleflash or smoke
    return harpoon;
}

struct weapons_s getGrenadeGun()
{
    static struct weapons_s grenade;
    int32_t ver = World_GetVersion();
    grenade.item_id = ITEM_GRENADEGUN;
    grenade.shot = TR_AUDIO_SOUND_SHOTGRENADEGUN;
    grenade.draw = TR_AUDIO_SOUND_HOLSTERIN;
    grenade.hide = TR_AUDIO_SOUND_HOLSTERIN;
    grenade.echo = SND_NULL;
    if((ver >= TR_II) && (ver < TR_IV))
    {
        grenade.reload_1 = TR_AUDIO_SOUND_TR2_3_RELOADGRENADEGUN;
        grenade.reload_2 = TR_AUDIO_SOUND_TR2_3_RELOADGRENADEGUN_LOCK;
    }
    else if(ver >= TR_IV)
    {
        grenade.reload_1 = TR_AUDIO_SOUND_TR4C_RELOADGRENADEGUN;
        grenade.reload_2 = TR_AUDIO_SOUND_TR4C_RELOADGRENADEGUN_LOCK;
    }
    grenade.damage_explosion = 40;
    grenade.firerate = RATE_GRENADEGUN;
    grenade.bullet = 1;

    if((ver >= TR_II) && (ver < TR_IV))
    {
        grenade.current_ammo = ITEM_GRENADEGUN_AMMO;
    }
    else if((ver >= TR_IV))
    {
        grenade.current_ammo = ITEM_GRENADEGUN_NORMAL_AMMO;
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
    return grenade;
}

struct weapons_s getCrossbowGun()
{
    static struct weapons_s crossbow;
    crossbow.item_id = ITEM_CROSSBOW;
    crossbow.shot = TR_AUDIO_SOUND_SHOTCROSSBOW;
    crossbow.draw = TR_AUDIO_SOUND_HOLSTERIN;
    crossbow.hide = TR_AUDIO_SOUND_HOLSTERIN;
    crossbow.echo = SND_NULL;
    crossbow.reload_1 = TR_AUDIO_SOUND_RELOAD;
    crossbow.reload_2 = SND_NULL;
    crossbow.damage = 12;
    crossbow.firerate = RATE_CROSSBOWGUN;
    crossbow.bullet = 1;
    crossbow.ammo_counter = 1;
    crossbow.current_ammo = ITEM_CROSSBOW_NORMAL_AMMO;
    crossbow.onWater = false;
    crossbow.dealDmgAtImpact = false;
    crossbow.haveGravity = false;
    crossbow.itemIsEquipped = false;             // can be equiped of the lasersight
    // same as harpoon
    return crossbow;
}

struct weapons_s getGrapplinGun()
{
    static struct weapons_s grapplin;
    grapplin.item_id = ITEM_GRAPPLEGUN;
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
    grapplin.current_ammo = ITEM_GRAPPLEGUN_AMMO;
    grapplin.onWater = false;
    grapplin.dealDmgAtImpact = false;
    grapplin.haveGravity = false;
    // no muzzleflash for this ?
    // same as harpoon and crossbow
    return grapplin;
}

bool Weapons_GetIdsFromActionKey(int actionKey, int *pointerModelId, int *pointerInventoryItemId)
{
    typedef struct
    {
        int modelId;
        int inventoryItemId;
    } WeaponIds;

    static WeaponIds tombRaider1WeaponIds[] =
    {
        { TR_MODEL_PISTOL, ITEM_PISTOL },
        { TR1_MODEL_SHOTGUN, ITEM_SHOTGUN },
        { TR1_MODEL_MAGNUM, ITEM_MAGNUMS },
        { TR1_MODEL_UZI, ITEM_UZIS },
        { 0, 0 },
        { 0, 0 },
        { 0, 0 },
        { 0, 0 }
    };
    static WeaponIds tombRaider2WeaponIds[] =
    {
        { TR_MODEL_PISTOL, ITEM_PISTOL },
        { TR_MODEL_SHOTGUN, ITEM_SHOTGUN },
        { TR2_MODEL_AUTOMAGS, ITEM_AUTOMAGS },
        { TR2_MODEL_UZI, ITEM_UZIS },
        { TR2_MODEL_HARPOONGUN, ITEM_HARPOONGUN },
        { TR2_MODEL_M16, ITEM_M16 },
        { TR2_MODEL_GRENADEGUN, ITEM_GRENADEGUN },
        { 0, 0 }
    };
    static WeaponIds tombRaider3WeaponIds[] =
    {
        { TR_MODEL_PISTOL, ITEM_PISTOL },
        { TR_MODEL_SHOTGUN, ITEM_SHOTGUN },
        { TR3_MODEL_DESERTEAGLE, ITEM_DESERTEAGLE },
        { TR3_MODEL_UZI, ITEM_UZIS },
        { TR3_MODEL_HARPOONGUN, ITEM_HARPOONGUN },
        { TR3_MODEL_MP5, ITEM_MP5 },
        { TR3_MODEL_ROCKETGUN, ITEM_ROCKETGUN },
        { TR3_MODEL_GRENADEGUN, ITEM_GRENADEGUN }
    };
    static WeaponIds tombRaider4WeaponIds[] =
    {
        { TR_MODEL_PISTOL, ITEM_PISTOL },
        { TR_MODEL_SHOTGUN, ITEM_SHOTGUN },
        { TR4C_MODEL_REVOLVER, ITEM_REVOLVER },
        { TR4C_MODEL_UZI, ITEM_UZIS },
        { TR4C_MODEL_CROSSBOW, ITEM_CROSSBOW },
        { TR4C_MODEL_GRENADEGUN, ITEM_GRENADEGUN },
        { 0, 0 },
        { 0, 0 }
    };
    // TODO several lookup tables according to level sections (Rome, Russia...)
    static WeaponIds tombRaider5WeaponIds[] =
    {
        { TR_MODEL_PISTOL, ITEM_PISTOL },
        { TR_MODEL_SHOTGUN, ITEM_SHOTGUN },
        { TR4C_MODEL_REVOLVER, ITEM_REVOLVER },
        { TR4C_MODEL_UZI, ITEM_UZIS },
        { TR4C_MODEL_CROSSBOW, ITEM_CROSSBOW },
        { TR4C_MODEL_GRENADEGUN, ITEM_GRENADEGUN },
        { 0, 0 },
        { 0, 0 }
    };

    // Make sure a valid action key is provided
    if((actionKey < ACT_WEAPON1) || (actionKey > ACT_WEAPON8))
    {
        return false;
    }
    actionKey -= ACT_WEAPON1; // Lookup tables are zero-based

    // Select the right lookup table
    int gameVersion = World_GetVersion();
    if(gameVersion < TR_II)
    {
        *pointerModelId = tombRaider1WeaponIds[actionKey].modelId;
        *pointerInventoryItemId = tombRaider1WeaponIds[actionKey].inventoryItemId;
        return true;
    }
    else if(gameVersion < TR_III)
    {
        *pointerModelId = tombRaider2WeaponIds[actionKey].modelId;
        *pointerInventoryItemId = tombRaider2WeaponIds[actionKey].inventoryItemId;
        return true;
    }
    else if(gameVersion < TR_IV)
    {
        *pointerModelId = tombRaider3WeaponIds[actionKey].modelId;
        *pointerInventoryItemId = tombRaider3WeaponIds[actionKey].inventoryItemId;
        return true;
    }
    else if(gameVersion < TR_V)
    {
        *pointerModelId = tombRaider4WeaponIds[actionKey].modelId;
        *pointerInventoryItemId = tombRaider4WeaponIds[actionKey].inventoryItemId;
        return true;
    }
    else
    {
        *pointerModelId = tombRaider5WeaponIds[actionKey].modelId;
        *pointerInventoryItemId = tombRaider5WeaponIds[actionKey].inventoryItemId;
        return true;
    }
}
