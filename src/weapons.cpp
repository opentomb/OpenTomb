#include "weapons.h"

#include "core/system.h"
#include "core/console.h"
#include "core/vmath.h"
#include "core/obb.h"

#include "script/script.h"
#include "audio/audio.h"
#include "inventory.h"
#include "entity.h"
#include "room.h"
#include "world.h"

#include "character_controller.h"
#include "state_control/state_control_Lara.h"

#define SND_NULL (-1)

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
    int32_t ver = World_GetVersion();

    //=============================================//
    //                PISTOLS GUN                  //
    //=============================================//
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

    //=============================================//
    //                SHOTGUN GUN                  //
    //=============================================//
    shotgun.item_id = ITEM_SHOTGUN;
    shotgun.shot = TR_AUDIO_SOUND_SHOTSHOTGUN;
    shotgun.draw = TR_AUDIO_SOUND_HOLSTERIN;
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
    
    //=============================================//
    //                   UZI GUN                   //
    //=============================================//
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

    //=============================================//
    //                 MAGNUM GUN                  //
    //=============================================//
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

    //=============================================//
    //             DESERT EAGLE GUN                //
    //=============================================//
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

    //=============================================//
    //                AUTOMAGS GUN                 //
    //=============================================//
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

    //=============================================//
    //                REVOLVER GUN                 //
    //=============================================//
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

    //=============================================//
    //                  M16 GUN                    //
    //=============================================//
    m16.item_id = ITEM_M16;
    m16.shot = TR_AUDIO_SOUND_SHOTM16;
    m16.draw = TR_AUDIO_SOUND_HOLSTERIN;
    m16.hide = SND_NULL;
    m16.echo = TR_AUDIO_SOUND_SHOTM16_END;
    m16.reload_1 = SND_NULL;
    m16.reload_2 = SND_NULL;
    m16.damage = 12;
    m16.firerate = (mp5.alternateAim) ? RATE_M16_ALT : RATE_M16;
    m16.bullet = 1;
    m16.current_ammo = ITEM_M16_AMMO;
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
    mp5.item_id = ITEM_MP5;
    mp5.shot = TR_AUDIO_SOUND_SHOTMP5;
    mp5.draw = TR_AUDIO_SOUND_HOLSTERIN;
    mp5.hide = TR_AUDIO_SOUND_HOLSTERIN;
    mp5.echo = SND_NULL;
    mp5.reload_1 = SND_NULL;
    mp5.reload_2 = SND_NULL;
    mp5.damage = 12;
    mp5.damage_explosion = 0;
    mp5.firerate = (mp5.alternateAim) ? RATE_MP5_ALT : RATE_MP5;
    mp5.bullet = 1;
    mp5.current_ammo = ITEM_MP5_AMMO;
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

    //=============================================//
    //                GRENADE GUN                  //
    //=============================================//
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

    //=============================================//
    //                HARPOON GUN                  //
    //=============================================//
    harpoon.item_id = ITEM_HARPOONGUN;
    harpoon.shot = (player->move_type == MOVE_UNDERWATER) ? TR_AUDIO_SOUND_SHOTHARPOON_W : TR_AUDIO_SOUND_SHOTHARPOON_G;
    harpoon.draw = (player->move_type == MOVE_UNDERWATER) ? SND_NULL : TR_AUDIO_SOUND_HOLSTERIN;
    harpoon.hide = (player->move_type == MOVE_UNDERWATER) ? SND_NULL : TR_AUDIO_SOUND_HOLSTERIN;
    harpoon.echo = SND_NULL;
    harpoon.reload_1 = (player->move_type == MOVE_UNDERWATER) ? TR_AUDIO_SOUND_RELOADHARPOON_W : TR_AUDIO_SOUND_RELOADHARPOON_G;
    harpoon.damage = 10;
    harpoon.damage_explosion = 0;
    harpoon.damage_water = 20;
    harpoon.firerate = RATE_HARPOONGUN;
    harpoon.bullet = 1;
    harpoon.ammo_counter = 4;
    harpoon.current_ammo = ITEM_HARPOONGUN_AMMO;
    harpoon.onWater = (player->move_type == MOVE_UNDERWATER) ? true : false;
    harpoon.haveGravity = (player->move_type == MOVE_UNDERWATER) ? false : true;
    harpoon.dealDmgAtImpact = false;
    // harpoon have no muzzleflash or smoke

    //=============================================//
    //                CROSSBOW GUN                 //
    //=============================================//
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

    //=============================================//
    //                GRAPPLIN GUN                 //
    //=============================================//
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
