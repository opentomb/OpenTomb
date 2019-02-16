#include "weapons.h"

#include "core/system.h"
#include "core/console.h"
#include "core/vmath.h"
#include "core/obb.h"

#include "script/script.h"
#include "room.h"
#include "world.h"

#include "character_controller.h"
#include "inventory.h"
#include "state_control/state_control_Lara.h"

weapons_s pistol;
weapons_s shotgun;
weapons_s uzi;
weapons_s magnum;
weapons_s automags;
weapons_s deserteagle;
weapons_s revolver;
weapons_s m16;
weapons_s mp5;
weapons_s rocketgun;
weapons_s grenadegun;
weapons_s harpoongun;
weapons_s crossbowgun;
weapons_s grapplingun;

void weapon_init()
{
	int32_t world = World_GetVersion();
	entity_s* player = World_GetPlayer();

	pistol.item_id = TYPE_PISTOLS;
	pistol.shot = SND_PISTOL;
	pistol.draw = SND_UNIVERSAL_DRAW;
	pistol.hide = SND_UNIVERSAL_HIDE;
	pistol.echo = SND_NULL;
	pistol.reload_1 = SND_UNIVERSAL_RELOAD;
	pistol.reload_2 = SND_NULL;
	pistol.damage = 1;
	pistol.damage_explosion = 0;
	pistol.firerate = RATE_PISTOL;
	pistol.bullet = 2;
	pistol.current_ammo = AMMO_UNLIMITED;
	pistol.range = RANGE_PISTOL;
	pistol.onWater = false;
	pistol.dealDmgAtImpact = false;
	pistol.haveGravity = false;
	pistol.muzzle_duration = 3.0f;
	pistol.muzzle_pos[0] = NULL;
	pistol.muzzle_pos[1] = NULL;
	pistol.muzzle_pos[2] = NULL;
	pistol.muzzle_orient[0] = NULL;
	pistol.muzzle_orient[1] = NULL;
	pistol.muzzle_orient[2] = NULL;

	shotgun.item_id = TYPE_SHOTGUN;
	shotgun.shot = SND_SHOTGUN;
	shotgun.draw = SND_UNIVERSAL_HIDE;
	shotgun.hide = SND_UNIVERSAL_HIDE;
	shotgun.echo = SND_NULL;
	shotgun.reload_1 = SND_UNIVERSAL_RELOAD;
	shotgun.reload_2 = SND_NULL;
	shotgun.damage = 3;
	shotgun.damage_explosion = 0;
	shotgun.firerate = RATE_SHOTGUN;
	shotgun.bullet = (shotgun.current_ammo == AMMO_SHOTGUN_WIDE) ? 5 : 6; // wide have 5 bullet
	if (world < TR_IV)
	{
		shotgun.current_ammo = AMMO_SHOTGUN;
	}
	else
	{
		shotgun.current_ammo = AMMO_SHOTGUN_NORMAL;
	}
	shotgun.range = RANGE_SHOTGUN;
	shotgun.onWater = false;
	shotgun.dealDmgAtImpact = false;
	shotgun.haveGravity = false;
	// shotgun not have muzzleflash but can be used for smoke
	shotgun.muzzle_pos[0] = NULL;
	shotgun.muzzle_pos[1] = NULL;
	shotgun.muzzle_pos[2] = NULL;
	shotgun.muzzle_orient[0] = NULL;
	shotgun.muzzle_orient[1] = NULL;
	shotgun.muzzle_orient[2] = NULL;

	uzi.item_id = TYPE_UZIS;
	uzi.shot = SND_UZI;
	uzi.draw = SND_UNIVERSAL_DRAW;
	uzi.hide = SND_UNIVERSAL_HIDE;
	uzi.echo = (world >= TR_II) ? SND_UZI_STOP : SND_NULL;
	uzi.reload_1 = SND_UNIVERSAL_RELOAD;
	uzi.reload_2 = SND_NULL;
	uzi.damage = 1;
	uzi.damage_explosion = 0;
	uzi.firerate = RATE_UZI;
	uzi.bullet = 2;
	uzi.current_ammo = AMMO_UZI;
	uzi.onWater = false;
	uzi.dealDmgAtImpact = false;
	uzi.haveGravity = false;
	uzi.muzzle_pos[0] = NULL;
	uzi.muzzle_pos[1] = NULL;
	uzi.muzzle_pos[2] = NULL;
	uzi.muzzle_orient[0] = NULL;
	uzi.muzzle_orient[1] = NULL;
	uzi.muzzle_orient[2] = NULL;

	magnum.item_id = TYPE_MAGNUMS;
	magnum.shot = SND_MAGNUM;
	magnum.draw = SND_UNIVERSAL_DRAW;
	magnum.hide = SND_UNIVERSAL_HIDE;
	magnum.echo = SND_NULL;
	magnum.reload_1 = SND_UNIVERSAL_RELOAD;
	magnum.reload_2 = SND_NULL;
	magnum.damage = 21;
	magnum.damage_explosion = 0;
	magnum.firerate = RATE_MAGNUM;
	magnum.bullet = 2;
	magnum.current_ammo = AMMO_MAGNUM;
	magnum.onWater = false;
	magnum.dealDmgAtImpact = false;
	magnum.haveGravity = false;
	magnum.muzzle_pos[0] = NULL;
	magnum.muzzle_pos[1] = NULL;
	magnum.muzzle_pos[2] = NULL;
	magnum.muzzle_orient[0] = NULL;
	magnum.muzzle_orient[1] = NULL;
	magnum.muzzle_orient[2] = NULL;

	deserteagle.item_id = TYPE_DESERTEAGLE;
	deserteagle.shot = SND_DESERTEAGLE;
	deserteagle.draw = SND_UNIVERSAL_DRAW;
	deserteagle.hide = SND_UNIVERSAL_HIDE;
	deserteagle.echo = SND_NULL;
	deserteagle.reload_1 = SND_UNIVERSAL_RELOAD;
	deserteagle.reload_2 = SND_NULL;
	deserteagle.damage = 21;
	deserteagle.damage_explosion = 0;
	deserteagle.firerate = RATE_DESERTEAGLE;
	deserteagle.bullet = 1;
	deserteagle.current_ammo = AMMO_DESERTEAGLE;
	deserteagle.onWater = false;
	deserteagle.dealDmgAtImpact = false;
	deserteagle.haveGravity = false;
	deserteagle.muzzle_pos[0] = NULL;
	deserteagle.muzzle_pos[1] = NULL;
	deserteagle.muzzle_pos[2] = NULL;
	deserteagle.muzzle_orient[0] = NULL;
	deserteagle.muzzle_orient[1] = NULL;
	deserteagle.muzzle_orient[2] = NULL;

	automags.item_id = TYPE_AUTOMAGS;
	automags.shot = SND_AUTOMAGS;
	automags.draw = SND_UNIVERSAL_DRAW;
	automags.hide = SND_UNIVERSAL_HIDE;
	automags.echo = SND_NULL;
	automags.reload_1 = SND_UNIVERSAL_RELOAD;
	automags.reload_2 = SND_NULL;
	automags.damage = 12;
	automags.damage_explosion = 0;
	automags.firerate = RATE_AUTOMAGS;
	automags.bullet = 2;
	automags.current_ammo = AMMO_AUTOMAGS;
	automags.onWater = false;
	automags.dealDmgAtImpact = false;
	automags.haveGravity = false;
	automags.muzzle_pos[0] = NULL;
	automags.muzzle_pos[1] = NULL;
	automags.muzzle_pos[2] = NULL;
	automags.muzzle_orient[0] = NULL;
	automags.muzzle_orient[1] = NULL;
	automags.muzzle_orient[2] = NULL;

	revolver.item_id = TYPE_REVOLVER;
	revolver.shot = SND_REVOLVER;
	revolver.draw = SND_UNIVERSAL_DRAW;
	revolver.hide = SND_UNIVERSAL_HIDE;
	revolver.echo = SND_NULL;
	revolver.reload_1 = SND_UNIVERSAL_RELOAD;
	revolver.reload_2 = SND_NULL;
	revolver.damage = 21;
	revolver.damage_explosion = 0;
	revolver.firerate = RATE_REVOLVER;
	revolver.bullet = 1;
	revolver.current_ammo = AMMO_REVOLVER;
	revolver.onWater = false;
	revolver.dealDmgAtImpact = false;
	revolver.haveGravity = false;
	revolver.muzzle_pos[0] = NULL;
	revolver.muzzle_pos[1] = NULL;
	revolver.muzzle_pos[2] = NULL;
	revolver.muzzle_orient[0] = NULL;
	revolver.muzzle_orient[1] = NULL;
	revolver.muzzle_orient[2] = NULL;

	m16.item_id = TYPE_M16;
	m16.shot = SND_M16;
	m16.draw = SND_UNIVERSAL_HIDE;
	m16.hide = SND_UNIVERSAL_HIDE;
	m16.echo = SND_M16_STOP;
	m16.reload_1 = SND_NULL;
	m16.reload_2 = SND_NULL;
	m16.damage = 12;
	m16.damage_explosion = 0;
	m16.firerate = (m16.alternateAim) ? RATE_M16_ALT : RATE_M16;
	m16.bullet = 1;
	m16.current_ammo = AMMO_M16;
	m16.onWater = false;
	m16.dealDmgAtImpact = false;
	m16.haveGravity = false;
	m16.alternateAim = false;           // define it at animation (M16Anim())
	m16.muzzle_pos[0] = NULL;
	m16.muzzle_pos[1] = NULL;
	m16.muzzle_pos[2] = NULL;
	m16.muzzle_orient[0] = NULL;
	m16.muzzle_orient[1] = NULL;
	m16.muzzle_orient[2] = NULL;

	mp5.item_id = TYPE_MP5;
	mp5.shot = SND_MP5;
	mp5.draw = SND_UNIVERSAL_HIDE;
	mp5.hide = SND_UNIVERSAL_HIDE;
	mp5.echo = SND_NULL;
	mp5.reload_1 = SND_NULL;
	mp5.reload_2 = SND_NULL;
	mp5.damage = 12;
	mp5.damage_explosion = 0;
	mp5.firerate = RATE_MP5;
	mp5.bullet = 1;
	mp5.current_ammo = AMMO_MP5;
	mp5.onWater = false;
	mp5.dealDmgAtImpact = false;
	mp5.haveGravity = false;
	mp5.muzzle_pos[0] = NULL;
	mp5.muzzle_pos[1] = NULL;
	mp5.muzzle_pos[2] = NULL;
	mp5.muzzle_orient[0] = NULL;
	mp5.muzzle_orient[1] = NULL;
	mp5.muzzle_orient[2] = NULL;

	rocketgun.item_id = TYPE_ROCKETGUN;
	rocketgun.shot = SND_ROCKETGUN;
	rocketgun.draw = SND_UNIVERSAL_HIDE;
	rocketgun.hide = SND_UNIVERSAL_HIDE;
	rocketgun.echo = SND_NULL;
	rocketgun.reload_1 = SND_ROCKETGUN_RELOAD;
	rocketgun.reload_2 = SND_NULL;
	rocketgun.damage = 0;
	rocketgun.damage_explosion = 100;
	rocketgun.firerate = RATE_ROCKETGUN;
	rocketgun.bullet = 1;
	rocketgun.current_ammo = AMMO_ROCKETGUN;
	rocketgun.onWater = false;
	rocketgun.dealDmgAtImpact = false;
	rocketgun.haveGravity = false;
	rocketgun.muzzle_pos[0] = NULL;
	rocketgun.muzzle_pos[1] = NULL;
	rocketgun.muzzle_pos[2] = NULL;
	rocketgun.muzzle_orient[0] = NULL;
	rocketgun.muzzle_orient[1] = NULL;
	rocketgun.muzzle_orient[2] = NULL;

	grenadegun.item_id = TYPE_GRENADEGUN;
	grenadegun.shot = SND_GRENADEGUN;
	grenadegun.draw = SND_UNIVERSAL_HIDE;
	grenadegun.hide = SND_UNIVERSAL_HIDE;
	grenadegun.echo = SND_NULL;
	if (world > TR_I_UB && world < TR_IV)
	{
		grenadegun.reload_1 = SND_TR2_3_GRENADEGUN_RELOAD;
		grenadegun.reload_2 = SND_TR2_3_GRENADEGUN_LOCK;
	}
	else if (world >= TR_IV)
	{
		grenadegun.reload_1 = SND_TR4_C_GRENADEGUN_RELOAD;
		grenadegun.reload_2 = SND_TR4_C_GRENADEGUN_LOCK;
	}
	grenadegun.damage = 1;
	grenadegun.firerate = RATE_GRENADEGUN;
	grenadegun.bullet = 1;
	grenadegun.current_ammo = AMMO_GRENADE;
	grenadegun.onWater = false;
	grenadegun.dealDmgAtImpact = false;
	grenadegun.haveGravity = true;
	grenadegun.muzzle_pos[0] = NULL;
	grenadegun.muzzle_pos[1] = NULL;
	grenadegun.muzzle_pos[2] = NULL;
	grenadegun.muzzle_orient[0] = NULL;
	grenadegun.muzzle_orient[1] = NULL;
	grenadegun.muzzle_orient[2] = NULL;

	harpoongun.item_id = TYPE_HARPOONGUN;
	harpoongun.shot = (player->move_type == MOVE_UNDERWATER) ? SND_HARPOON_WATER : SND_HARPOON_LAND;
	harpoongun.draw = (player->move_type == MOVE_UNDERWATER) ? SND_NULL : SND_UNIVERSAL_HIDE;
	harpoongun.hide = (player->move_type == MOVE_UNDERWATER) ? SND_NULL : SND_UNIVERSAL_HIDE;
	harpoongun.echo = SND_NULL;
	harpoongun.reload_1 = (player->move_type == MOVE_UNDERWATER) ? SND_HARPOON_RELOAD_WATER : SND_HARPOON_RELOAD_LAND;
	harpoongun.damage = 10;
	harpoongun.damage_explosion = 0;
	harpoongun.damage_water = 20;
	harpoongun.firerate = RATE_HARPOONGUN;
	harpoongun.bullet = 1;
	harpoongun.ammo_counter = 4;
	harpoongun.current_ammo = AMMO_HARPOON;
	harpoongun.onWater = (player->move_type == MOVE_UNDERWATER) ? true : false;
	harpoongun.haveGravity = (player->move_type == MOVE_UNDERWATER) ? false : true;
	harpoongun.dealDmgAtImpact = false;
	harpoongun.muzzle_pos[0] = NULL;
	harpoongun.muzzle_pos[1] = NULL;
	harpoongun.muzzle_pos[2] = NULL;
	harpoongun.muzzle_orient[0] = NULL;
	harpoongun.muzzle_orient[1] = NULL;
	harpoongun.muzzle_orient[2] = NULL;

	crossbowgun.item_id = TYPE_CROSSBOWGUN;
	crossbowgun.shot = SND_CROSSBOWGUN;
	crossbowgun.draw = SND_UNIVERSAL_HIDE;
	crossbowgun.hide = SND_UNIVERSAL_HIDE;
	crossbowgun.echo = SND_NULL;
	crossbowgun.reload_1 = SND_UNIVERSAL_RELOAD;
	crossbowgun.reload_2 = SND_NULL;
	crossbowgun.damage = 12;
	crossbowgun.firerate = RATE_CROSSBOWGUN;
	crossbowgun.bullet = 1;
	crossbowgun.ammo_counter = 1;
	crossbowgun.current_ammo = AMMO_CROSSBOW_NORMAL;
	crossbowgun.onWater = false;
	crossbowgun.dealDmgAtImpact = false;
	crossbowgun.haveGravity = false;
	crossbowgun.muzzle_pos[0] = NULL;
	crossbowgun.muzzle_pos[1] = NULL;
	crossbowgun.muzzle_pos[2] = NULL;
	crossbowgun.muzzle_orient[0] = NULL;
	crossbowgun.muzzle_orient[1] = NULL;
	crossbowgun.muzzle_orient[2] = NULL;

	grapplingun.item_id = TYPE_GRAPPLINGUN;
	// this "weapon" have sound ?
	grapplingun.shot = SND_NULL;
	grapplingun.draw = SND_NULL;
	grapplingun.hide = SND_NULL;
	grapplingun.echo = SND_NULL;
	grapplingun.reload_1 = SND_NULL;
	grapplingun.reload_2 = SND_NULL;
	grapplingun.damage = 0;
	grapplingun.firerate = RATE_GRAPPINGUN;
	grapplingun.bullet = 1;
	grapplingun.current_ammo = AMMO_GRAPPLIN;
	grapplingun.onWater = false;
	grapplingun.dealDmgAtImpact = false;
	grapplingun.haveGravity = false;
	// no muzzleflash for this ?
	grapplingun.muzzle_pos[0] = NULL;
	grapplingun.muzzle_pos[1] = NULL;
	grapplingun.muzzle_pos[2] = NULL;
	grapplingun.muzzle_orient[0] = NULL;
	grapplingun.muzzle_orient[1] = NULL;
	grapplingun.muzzle_orient[2] = NULL;
}

// if you define (UNLIMITED) the item have unlimited ammo
bool checkCanShoot(struct weapons_s item_id)
{
    entity_s* player = World_GetPlayer();

    if (item_id.current_ammo == -1)
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

bool consumeAmmo(struct weapons_s weapon)
{
	entity_p player = World_GetPlayer();

	// checking if ammo exists before consume !
	if (Inventory_GetItemsCount(player->inventory, weapon.current_ammo) > AMMO_EMPTY)
	{
		// checking if the current ammo is equal to ammo you want to consume !
		if (NOT_EMPTY_OR_UNLIMITED)
		{
			// consume item
			Inventory_RemoveItem(&player->inventory, weapon.current_ammo, 1);
			return true;
		}
	}

	// item not found or not equal to current ammo
	return false;
}

int CurrentWeaponToItemID(struct ss_animation_s* ss_anim)
{
    int32_t ver = World_GetVersion();

	if (GET_MODEL(TR_MODEL_PISTOL))
	{
		return ITEM_PISTOL;
	}
	else if ((ver > TR_I_UB) && GET_MODEL(TR_MODEL_SHOTGUN))
	{
		return ITEM_SHOTGUN;
	}

    if (ver < TR_II)  // tomb raider 1
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
    else if (ver < TR_III) // tomb raider 2
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
    else if (ver < TR_IV) // tomb raider 3
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
    else if (ver < TR_V)         // tomb raider 4 and 5
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
	else if (ver == TR_V)
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
			return ITEM_MP5;  // it's h&k gun !
		}
		else if (GET_MODEL(TR4C_MODEL_REVOLVER))
		{
			return ITEM_REVOLVER;
		}
	}

    // no id found
    return NULL;
}

int SetCurrentWeaponAnimation(struct entity_s* ent, struct ss_animation_s* ss_anim, float time, struct weapons_s weapon, ss_bone_tag_p b_tag, entity_p target, float* target_pos, int inc_state, bool do_aim, uint16_t targeted_bone_start, uint16_t targeted_bone_end)
{
	b_tag->is_targeted = (target) ? (0x01) : (0x00);

	switch (ss_anim->current_animation)
	{
		case ANIM_IDLE_TO_FIRING:
			ONEHAND_IDLE_TO_FIRING;
			return IS_IDLE;
		case ANIM_HIDE_TO_IDLE:
			ONEHAND_HIDE_TO_IDLE;
			return IS_DRAW;
		case ANIM_IDLE_AND_HIDE:
			ONEHAND_HIDE_AND_IDLE;
			return IS_SPECIAL;
		case ANIM_FIRING:
			if (checkCanShoot(weapon))
			{
				ONEHAND_FIRING(ANIM_FIRING, ANIM_IDLE_TO_FIRING);
				return IS_FIRING;
			}
			else
			{
				AutoSelect(TR_MODEL_PISTOL, ss_anim, ent, time);
				return IS_HIDE;
			}
			break;
	}

	return IS_NOT_DEFINED;
}

int ShotgunAnim(struct entity_s* ent, struct ss_animation_s* ss_anim, float time, struct weapons_s weapon, ss_bone_tag_p b_tag, entity_p target, float* target_pos, int inc_state, bool do_aim)
{
    int32_t ver = World_GetVersion();

    switch (ss_anim->current_animation)
    {
		case SHOTGUN_IDLE_TO_FIRING:
			ANIM_STATE;
			TWOHAND_IDLE_TO_FIRING(SHOTGUN_IDLE_TO_HIDE, SHOTGUN_FIRING);
			return IS_IDLE;

		case SHOTGUN_HIDE_TO_IDLE:
			TWOHAND_HIDE_TO_IDLE(SHOTGUN_IDLE_TO_FIRING);
			return IS_DRAW;

		case SHOTGUN_FIRING:
			if (checkCanShoot(weapon))
			{
				TWOHAND_FIRING(SHOTGUN_FIRING, SHOTGUN_FIRING_TO_IDLE, SHOTGUN_IDLE_TO_FIRING);
				return IS_FIRING;
			}
			else
			{
				TWOHAND_IDLE_TO_HIDE(TW_FRAME_IS_END, true);
			}
			return IS_NOT_DEFINED;

		case SHOTGUN_IDLE_TO_HIDE:
			TWOHAND_IDLE_TO_HIDE(TW_FRAME_IS_END, false);
			return IS_HIDE;

		case SHOTGUN_FIRING_TO_IDLE:
			TWOHAND_FIRING_TO_IDLE(SHOTGUN_IDLE_TO_FIRING);
			return IS_FIRING_TO_IDLE;
    }

	return IS_NOT_DEFINED;
}

int GrenadeGunAnim(struct entity_s* ent, struct ss_animation_s* ss_anim, float time, struct weapons_s weapon, ss_bone_tag_p b_tag, entity_p target, float* target_pos, int inc_state, bool do_aim)
{
	int32_t ver = World_GetVersion();

	switch (ss_anim->current_animation)
	{
		case GRENADEGUN_HIDE_TO_IDLE:
			TWOHAND_HIDE_TO_IDLE(GRENADEGUN_IDLE_TO_FIRING);
			return IS_DRAW;

		case GRENADEGUN_IDLE_TO_FIRING:
			ANIM_STATE;
			TWOHAND_IDLE_TO_FIRING(GRENADEGUN_IDLE_TO_HIDE, GRENADEGUN_FIRING);
			return IS_IDLE;

		case GRENADEGUN_FIRING_RELOAD:
			switch (ss_anim->current_frame)
			{
				case 26:
					if (ver > TR_I_UB && ver < TR_IV)
					{
						Audio_Send(SND_TR2_3_GRENADEGUN_RELOAD, TR_AUDIO_EMITTER_ENTITY, ent->id);
						return IS_SOUND;
					}
					else
					{
						Audio_Send(SND_TR4_C_GRENADEGUN_RELOAD, TR_AUDIO_EMITTER_ENTITY, ent->id);
						return IS_SOUND;
					}
					return IS_NOT_DEFINED;

				case 33:
					if (ver > TR_I_UB && ver < TR_IV)
					{
						Audio_Send(SND_TR2_3_GRENADEGUN_LOCK, TR_AUDIO_EMITTER_ENTITY, ent->id);
						return IS_SOUND;
					}
					else
					{
						Audio_Send(SND_TR4_C_GRENADEGUN_LOCK, TR_AUDIO_EMITTER_ENTITY, ent->id);
						return IS_SOUND;
					}
					return IS_NOT_DEFINED;
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
			return IS_RELOAD;

		case GRENADEGUN_FIRING:
			if (checkCanShoot(weapon))
			{
				TWOHAND_FIRING(GRENADEGUN_FIRING_RELOAD, GRENADEGUN_FIRING_TO_IDLE, GRENADEGUN_IDLE_TO_FIRING);
			}
			else
			{
				TWOHAND_IDLE_TO_HIDE(TW_FRAME_GRENADEGUN_IS_END, false);
			}
			
			return IS_FIRING;

		case GRENADEGUN_FIRING_TO_IDLE:
			TWOHAND_FIRING_TO_IDLE(GRENADEGUN_IDLE_TO_FIRING);
			return IS_FIRING_TO_IDLE;

		case GRENADEGUN_IDLE_TO_HIDE:
			TWOHAND_IDLE_TO_HIDE(TW_FRAME_GRENADEGUN_IS_END, false);
			return IS_HIDE;
	}

	return IS_NOT_DEFINED;
}

int HarpoonAnim(struct entity_s* ent, struct ss_animation_s* ss_anim, float time, struct weapons_s weapon, ss_bone_tag_p b_tag, entity_p target, float* target_pos, int inc_state, bool do_aim)
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
				ANIM_STATE;
				TWOHAND_IDLE_TO_FIRING(HARPOON_LAND_IDLE_TO_HIDE, HARPOON_LAND_FIRING);
				return IS_IDLE;
			}
			return IS_NOT_DEFINED;

		case HARPOON_LAND_HIDE_TO_IDLE:
			if (ent->move_type == MOVE_UNDERWATER)
			{
				TWOHAND_HIDE_TO_IDLE(HARPOON_WATER_IDLE_TO_FIRING);
				return IS_DRAW;
			}
			else
			{
				TWOHAND_HIDE_TO_IDLE(HARPOON_LAND_IDLE_TO_FIRING);
				return IS_DRAW;
			}
			return IS_NOT_DEFINED;

		case HARPOON_LAND_FIRING:
			if (checkCanShoot(weapon))
			{
				TWOHAND_FIRING(HARPOON_LAND_FIRING, HARPOON_LAND_FIRING_TO_IDLE, HARPOON_LAND_IDLE_TO_FIRING);
				return IS_FIRING;
			}
			else
			{
				AutoSelect(TR_MODEL_PISTOL, ss_anim, ent, time);
			}
			return IS_FIRING;

		case HARPOON_LAND_IDLE_TO_HIDE:
			TWOHAND_IDLE_TO_HIDE(TW_FRAME_IS_END, false);
			return IS_HIDE;

		case HARPOON_LAND_FIRING_TO_IDLE:
			TWOHAND_FIRING_TO_IDLE(HARPOON_LAND_IDLE_TO_FIRING);
			return IS_FIRING_TO_IDLE;

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
			return IS_RELOAD;

		case HARPOON_WATER_IDLE_TO_FIRING:
			ANIM_STATE;
			TWOHAND_IDLE_TO_FIRING(HARPOON_WATER_IDLE_TO_HIDE, HARPOON_WATER_FIRING);
			return IS_SPECIAL;

		case HARPOON_WATER_FIRING_TO_IDLE:
			TWOHAND_FIRING_TO_IDLE(HARPOON_WATER_IDLE_TO_FIRING);
			return IS_SPECIAL;

		case HARPOON_WATER_FIRING:
			if (checkCanShoot(weapon))
			{
				TWOHAND_FIRING(HARPOON_WATER_FIRING, HARPOON_WATER_FIRING_TO_IDLE, HARPOON_WATER_IDLE_TO_FIRING);
				return IS_FIRING;
			}
			else
			{
				AutoSelect(TR_MODEL_PISTOL, ss_anim, ent, time);
				return IS_HIDE;
			}
			return IS_SPECIAL;

		case HARPOON_WATER_IDLE_TO_HIDE:
			TWOHAND_IDLE_TO_HIDE(TW_FRAME_IS_END, false);
			return IS_SPECIAL;
	}

	return IS_NOT_DEFINED;
}

int MP5Anim(struct entity_s* ent, struct ss_animation_s* ss_anim, float time, struct weapons_s weapon, ss_bone_tag_p b_tag, entity_p target, float* target_pos, int inc_state, bool do_aim)
{
	switch (ss_anim->current_animation)
	{
		case MP5_IDLE_TO_FIRING:
			ANIM_STATE;
			TWOHAND_IDLE_TO_FIRING(MP5_IDLE_TO_HIDE, MP5_FIRING);
			return IS_IDLE;

		case MP5_HIDE_TO_IDLE:
			TWOHAND_HIDE_TO_IDLE(MP5_IDLE_TO_FIRING);
			return IS_DRAW;

		case MP5_FIRING:
			if (checkCanShoot(weapon))
			{
				TWOHAND_FIRING(MP5_FIRING, MP5_FIRING_TO_IDLE, MP5_IDLE_TO_FIRING);
				return IS_FIRING;
			}
			else
			{
				TWOHAND_IDLE_TO_HIDE(TW_FRAME_IS_END, true);
				return IS_HIDE;
			}
			return IS_NOT_DEFINED;

		case MP5_IDLE_TO_HIDE:
			TWOHAND_IDLE_TO_HIDE(TW_FRAME_IS_END, false);
			return IS_HIDE;

		case MP5_FIRING_TO_IDLE:
			TWOHAND_FIRING_TO_IDLE(MP5_IDLE_TO_FIRING);
			return IS_FIRING_TO_IDLE;
	}

	return IS_NOT_DEFINED;
}

int M16Anim(struct entity_s* ent, struct ss_animation_s* ss_anim, float time, struct weapons_s weapon, ss_bone_tag_p b_tag, entity_p target, float* target_pos, int inc_state, bool do_aim)
{
	switch (ss_anim->current_animation)
	{
		case M16_IDLE_TO_FIRING:
			ANIM_STATE;
			TWOHAND_IDLE_TO_FIRING(M16_IDLE_TO_HIDE, M16_FIRING);
			return IS_IDLE;

		case M16_HIDE_TO_IDLE:
			TWOHAND_HIDE_TO_IDLE(M16_IDLE_TO_FIRING);
			return IS_DRAW;

		case M16_FIRING:
			if (checkCanShoot(weapon))
			{
				TWOHAND_FIRING(M16_FIRING, M16_FIRING_TO_IDLE, M16_IDLE_TO_FIRING);
				return IS_FIRING;
			}
			else
			{
				TWOHAND_IDLE_TO_HIDE(TW_FRAME_IS_END, true);
				return IS_HIDE;
			}
			return IS_NOT_DEFINED;

		case M16_IDLE_TO_HIDE:
			TWOHAND_IDLE_TO_HIDE(TW_FRAME_IS_END, false);
			return IS_HIDE;

		case M16_FIRING_TO_IDLE:
			TWOHAND_FIRING_TO_IDLE(M16_IDLE_TO_FIRING);
			return IS_FIRING_TO_IDLE;
	}

	return IS_NOT_DEFINED;
}

int RocketGunAnim(struct entity_s* ent, struct ss_animation_s* ss_anim, float time, struct weapons_s weapon, ss_bone_tag_p b_tag, entity_p target, float* target_pos, int inc_state, bool do_aim)
{
	switch (ss_anim->current_animation)
	{
		case ROCKETGUN_IDLE_TO_FIRING:
			ANIM_STATE;
			TWOHAND_IDLE_TO_FIRING(ROCKETGUN_IDLE_TO_HIDE, ROCKETGUN_FIRING);
			return IS_IDLE;

		case ROCKETGUN_HIDE_TO_IDLE:
			TWOHAND_HIDE_TO_IDLE(ROCKETGUN_IDLE_TO_FIRING);
			return IS_DRAW;

		case ROCKETGUN_FIRING:
			if (checkCanShoot(weapon))
			{
				TWOHAND_FIRING(ROCKETGUN_FIRING, ROCKETGUN_FIRING_TO_IDLE, ROCKETGUN_IDLE_TO_FIRING);
				return IS_FIRING;
			}
			else
			{
				TWOHAND_IDLE_TO_HIDE(TW_FRAME_IS_END, true);
				return IS_HIDE;
			}
			break;

		case ROCKETGUN_IDLE_TO_HIDE:
			TWOHAND_IDLE_TO_HIDE(TW_FRAME_IS_END, false);
			return IS_HIDE;

		case ROCKETGUN_FIRING_TO_IDLE:
			TWOHAND_FIRING_TO_IDLE(ROCKETGUN_IDLE_TO_FIRING);
			return IS_FIRING_TO_IDLE;
	}

	return IS_NOT_DEFINED;
}

int CrossbowAnim(struct entity_s* ent, struct ss_animation_s* ss_anim, float time, struct weapons_s weapon, ss_bone_tag_p b_tag, entity_p target, float* target_pos, int inc_state, bool do_aim)
{
	switch (ss_anim->current_animation)
	{
		case CROSSBOW_IDLE_TO_FIRING:
			ANIM_STATE;
			TWOHAND_IDLE_TO_FIRING(CROSSBOW_IDLE_TO_HIDE, CROSSBOW_FIRING);
			return IS_IDLE;

		case CROSSBOW_HIDE_TO_IDLE:
			TWOHAND_HIDE_TO_IDLE(CROSSBOW_IDLE_TO_FIRING);
			return IS_DRAW;

		case CROSSBOW_FIRING:
			if (checkCanShoot(weapon))
			{
				TWOHAND_FIRING(CROSSBOW_FIRING, CROSSBOW_FIRING_TO_IDLE, CROSSBOW_IDLE_TO_FIRING);
				return IS_FIRING;
			}
			else
			{
				TWOHAND_IDLE_TO_HIDE(TW_FRAME_IS_END, true);
				return IS_HIDE;
			}
			break;
		case CROSSBOW_IDLE_TO_HIDE:
			TWOHAND_IDLE_TO_HIDE(TW_FRAME_IS_END, false);
			return IS_HIDE;

		case CROSSBOW_FIRING_TO_IDLE:
			TWOHAND_FIRING_TO_IDLE(CROSSBOW_IDLE_TO_FIRING);
			return IS_FIRING_TO_IDLE;
	}

	return IS_NOT_DEFINED;
}

int GrapplinGunAnim(struct entity_s* ent, struct ss_animation_s* ss_anim, float time, struct weapons_s weapon, ss_bone_tag_p b_tag, entity_p target, float* target_pos, int inc_state, bool do_aim)
{
	switch (ss_anim->current_animation)
	{
		case GRAPPLING_IDLE_TO_FIRING:
			ANIM_STATE;
			TWOHAND_IDLE_TO_FIRING(GRAPPLING_IDLE_TO_HIDE, GRAPPLING_FIRING);
			return IS_IDLE;

		case GRAPPLING_HIDE_TO_IDLE:
			TWOHAND_HIDE_TO_IDLE(GRAPPLING_IDLE_TO_FIRING);
			return IS_DRAW;

		case GRAPPLING_FIRING:
			if (checkCanShoot(weapon))
			{
				TWOHAND_FIRING(GRAPPLING_FIRING, GRAPPLING_FIRING_TO_IDLE, GRAPPLING_IDLE_TO_FIRING);
				return IS_FIRING;
			}
			else
			{
				TWOHAND_IDLE_TO_HIDE(TW_FRAME_IS_END, true);
				return IS_HIDE;
			}
			break;

		case GRAPPLING_IDLE_TO_HIDE:
			TWOHAND_IDLE_TO_HIDE(TW_FRAME_IS_END, false);
			return IS_HIDE;

		case GRAPPLING_FIRING_TO_IDLE:
			TWOHAND_FIRING_TO_IDLE(GRAPPLING_IDLE_TO_FIRING);
			return IS_FIRING_TO_IDLE;
	}

	return IS_NOT_DEFINED;
}

// if ammo is empty and you try to firing.
void AutoSelect(int model_id, ss_animation_s *ss_anim, entity_s* ent, float time)
{
	// needed
	ent->character->state.weapon_ready = 0;

	// animation only for one hand !
	if (ss_anim->model->animation_count == 4)
	{
		Anim_SetAnimation(ss_anim, 2, -1);
	}

	// when animation is finished, change weapon t
	if (Anim_IncTime(ss_anim, time))
	{
		Character_ChangeWeapon(ent, model_id);
	}

	// checking if entity (lara) is not dead and entity is moving on floor (not underwater)
	if (!ent->character->state.dead && ent->move_type == MOVE_ON_FLOOR)
	{
		
	}
	/*
	else
	{
		// starting idle to hide anim but no change weapon
		if (ss_anim->model->animation_count > 4)
		{
			Anim_SetAnimation(ss_anim, 3, 0);
		}
		else
		{
			Anim_SetAnimation(ss_anim, 2, -1);
		}
	}
	*/
}

// get the current version without ">" or "<" !
// - for checking the TR4 and TR5 in same if, use TR_IV_V !
// - returned -1 if not found !
// use GET_VERSION(game_id) 
int32_t getVersion(int id)
{
	int32_t ver = World_GetVersion();

	switch (id)
	{
		case TR_I:
		case TR_I_DEMO:
		case TR_I_UB:
			return (ver < TR_II);
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
	}

	return -1;
}

bool OneHand_IdleToFiring(entity_s* ent, ss_animation_s* ss_anim, weapons_s weapon, ss_bone_tag_p b_tag, entity_p target, bool do_aim, float time)
{
	b_tag->is_targeted = (target) ? (0x01) : (0x00);
	int inc_state = Anim_IncTime(ss_anim, (ent->character->state.weapon_ready && do_aim) ? (time) : (-time));

	if (ss_anim->model->animation_count == 4)
	{
		switch (inc_state)
		{
		case ARMED:
			if (ent->character->state.weapon_ready && ent->character->cmd.action)
			{
				///@FIXME: animation launch one time and do nothing (sound, ammo consume, shoot)
				Anim_SetAnimation(ss_anim, ANIM_FIRING, 0);
			}
			return true;

		case UNARMED:
			if (!ent->character->state.weapon_ready)
			{
				// fix exit fire loop bug
				Audio_Kill(weapon.shot, TR_AUDIO_EMITTER_ENTITY, ent->id);
				Anim_SetAnimation(ss_anim, ANIM_IDLE_AND_HIDE, -1);
				return true;
			}
			return false;
		}

		if (ent->character->state.weapon_ready && !ent->character->cmd.action)
		{
			if (ss_anim->anim_frame_flags == ANIM_FRAME_REVERSE)
			{
				switch (ss_anim->current_frame)
				{
				case 3:
					if (GET_VERSION(TR_II) && GET_MODEL(TR2_MODEL_UZI))
					{
						Audio_Send(weapon.echo, TR_AUDIO_EMITTER_ENTITY, ent->id);
					}
					else if (GET_VERSION(TR_III) && GET_MODEL(TR3_MODEL_UZI))
					{
						Audio_Send(weapon.echo, TR_AUDIO_EMITTER_ENTITY, ent->id);
					}
					else if (GET_VERSION(TR_IV_V) && GET_MODEL(TR4C_MODEL_UZI))
					{
						Audio_Send(weapon.echo, TR_AUDIO_EMITTER_ENTITY, ent->id);
					}
					return true;

				case 4:
					Audio_Kill(weapon.shot, TR_AUDIO_EMITTER_ENTITY, ent->id);
					return true;
				}
			}
		}
	}

	return false;
}

bool TwoHand_IdleToFiring(entity_s* ent, ss_animation_s* ss_anim, weapons_s weapon, ss_bone_tag_p b_tag, entity_p target, bool do_aim, float time, int inc_state, int anim_hide, int anim_firing)
{
	if (ss_anim->model->animation_count > 4)
	{
		b_tag->is_targeted = (target) ? (0x01) : (0x00);

		switch (inc_state)
		{
		case ARMED:
			if (ent->character->state.weapon_ready && ent->character->cmd.action)
			{
				///@FIXME: animation launch one time and do nothing (sound, ammo consume, shoot)
				Anim_SetAnimation(ss_anim, anim_firing, 0);
				return true;
			}
			return false;
		case UNARMED:
			if (!ent->character->state.weapon_ready)
			{
				// fix exit fire loop bug
				Audio_Kill(weapon.shot, TR_AUDIO_EMITTER_ENTITY, ent->id);
				Anim_SetAnimation(ss_anim, anim_hide, 0);
				return true;
			}
			return false;
		}
	}

	return false;
}

bool TwoHand_FiringToIdle(entity_s* ent, ss_animation_s* ss_anim, weapons_s weapon, float time, int anim_idle_to_firing)
{
	if (ss_anim->model->animation_count > 4)
	{
		switch (ss_anim->prev_frame)
		{
			case 1:
				// delete sound (loop mode killer)
				Audio_Kill(weapon.shot, TR_AUDIO_EMITTER_ENTITY, ent->id);
				break;
			case 3:
				// echo sound
				if (GET_VERSION(TR_II) && GET_MODEL(TR2_MODEL_M16))
				{
					Audio_Send(weapon.echo, TR_AUDIO_EMITTER_ENTITY, ent->id);
				}
				break;
		}

		if (Anim_IncTime(ss_anim, time))
		{
			Anim_SetAnimation(ss_anim, anim_idle_to_firing, 0);
			return true;
		}
	}

	return false;
}

bool OneHand_IdleToHide(entity_s* ent, ss_animation_s* ss_anim, weapons_s weapon, float time, bool do_aim)
{
	int inc_state = Anim_IncTime(ss_anim, (ent->character->state.weapon_ready && do_aim) ? (time) : (-time));

	if (ss_anim->model->animation_count == 4)
	{
		switch (inc_state)
		{
		case ARMED:
			if (ent->character->state.weapon_ready)
			{
				Anim_SetAnimation(ss_anim, ANIM_IDLE_TO_FIRING, 0);
				return true;
			}
			return false;

		case UNARMED:
			if (!ent->character->state.weapon_ready)
			{
				StateControl_SetWeaponMeshOn(ent->bf, ss_anim->model, HOLSTER_RIGHT);
				StateControl_SetWeaponMeshOn(ent->bf, ss_anim->model, HOLSTER_LEFT);
				StateControl_SetWeaponMeshOff(ent->bf, HAND_RIGHT);
				StateControl_SetWeaponMeshOff(ent->bf, HAND_LEFT);
				Anim_SetAnimation(ss_anim, ANIM_HIDE_TO_IDLE, -1);
				Audio_Send(weapon.hide, TR_AUDIO_EMITTER_ENTITY, ent->id);
				return true;
			}
			return false;
		}
	}

	return false;
}

bool TwoHand_IdleToHide(entity_s* ent, ss_animation_s* ss_anim, weapons_s weapon, float time, int inc_state, int frame_to_hide)
{
	if (ss_anim->model->animation_count > 4)
	{
		if (Anim_IncTime(ss_anim, time))
		{
			SSBoneFrame_DisableOverrideAnim(ent->bf, ss_anim);
		}

		if ((ss_anim->frame_changing_state >= 0x01) && (ss_anim->prev_frame == frame_to_hide))
		{
			if (GET_VERSION(TR_I))
				StateControl_SetWeaponMeshOn(ent->bf, ss_anim->model, TR1_BACK_WEAPON);
			else
				StateControl_SetWeaponMeshOn(ent->bf, ss_anim->model, TR2_BACK_WEAPON);

			StateControl_SetWeaponMeshOff(ent->bf, HAND_RIGHT);
			Audio_Send(weapon.hide, TR_AUDIO_EMITTER_ENTITY, ent->id);
		}
		return true;
	}

	return false;
}

bool OneHand_HideToIdle(entity_s* ent, ss_animation_s* ss_anim, weapons_s weapon, float time)
{
	int inc_state = Anim_IncTime(ss_anim, (ent->character->state.weapon_ready) ? (time) : (-time));

	if (ss_anim->model->animation_count == 4)
	{
		switch (inc_state)
		{
		case ARMED:
			if (ent->character->state.weapon_ready)
			{
				StateControl_SetWeaponMeshOn(ent->bf, ss_anim->model, HAND_RIGHT);
				StateControl_SetWeaponMeshOn(ent->bf, ss_anim->model, HAND_LEFT);
				StateControl_SetWeaponMeshOff(ent->bf, HOLSTER_RIGHT);
				StateControl_SetWeaponMeshOff(ent->bf, HOLSTER_LEFT);

				Anim_SetAnimation(ss_anim, ANIM_IDLE_AND_HIDE, 0);
				Audio_Send(weapon.draw, TR_AUDIO_EMITTER_ENTITY, ent->id);
				return true;
			}
			return false;

		case UNARMED:
			if (!ent->character->state.weapon_ready)
			{
				SSBoneFrame_DisableOverrideAnim(ent->bf, ss_anim);
				ent->character->state.weapon_ready = 0;
				return true;
			}
			return false;
		}
	}

	return false;
}

bool TwoHand_HideToIdle(entity_s* ent, ss_animation_s* ss_anim, weapons_s weapon, float time, int inc_state, int anim_idle)
{
	if (ss_anim->model->animation_count > 4)
	{
		if (Anim_IncTime(ss_anim, time))
		{
			Anim_SetAnimation(ss_anim, anim_idle, 0);  // to idle
		}

		if ((ss_anim->frame_changing_state >= 0x01) && (ss_anim->prev_frame == TW_FRAME))
		{
			StateControl_SetWeaponMeshOn(ent->bf, ss_anim->model, HAND_RIGHT);
			if (GET_VERSION(TR_I))
			{
				StateControl_SetWeaponMeshOff(ent->bf, TR1_BACK_WEAPON);
			}
			else
			{
				StateControl_SetWeaponMeshOff(ent->bf, TR2_BACK_WEAPON);
			}
			Audio_Send(weapon.draw, TR_AUDIO_EMITTER_ENTITY, ent->id);
			return true;
		}
	}

	return false;
}

bool OneHand_HideAndIdle(entity_s * ent, ss_animation_s * ss_anim, weapons_s weapon, float time)
{
	int inc_state = Anim_IncTime(ss_anim, (ent->character->state.weapon_ready) ? (time) : (-time));

	if (ss_anim->model->animation_count == 4)
	{
		switch (inc_state)
		{
		case ARMED:
			if (ent->character->state.weapon_ready)
			{
				Anim_SetAnimation(ss_anim, ANIM_IDLE_TO_FIRING, 0);
				return true;
			}
			break;
		case UNARMED:
			if (!ent->character->state.weapon_ready)
			{
				StateControl_SetWeaponMeshOn(ent->bf, ss_anim->model, 1);
				StateControl_SetWeaponMeshOn(ent->bf, ss_anim->model, 4);
				StateControl_SetWeaponMeshOff(ent->bf, 10);
				StateControl_SetWeaponMeshOff(ent->bf, 13);
				Anim_SetAnimation(ss_anim, ANIM_HIDE_TO_IDLE, -1);
				Audio_Send(weapon.hide, TR_AUDIO_EMITTER_ENTITY, ent->id);
				return true;
			}
			break;
		}
	}

	return false;
}

bool TwoHand_Firing(entity_s* ent, ss_animation_s* ss_anim, ss_bone_tag_p b_tag, entity_p target, float* target_pos, weapons_s weapon, float time, int anim_firing, int anim_firing_to_idle, int anim_idle)
{
	if ((ss_anim->frame_changing_state >= 4) | Anim_IncTime(ss_anim, time * weapon.firerate))
	{
		if (ent->character->state.weapon_ready && ent->character->cmd.action)
		{
			Anim_SetAnimation(ss_anim, anim_firing, 0);
			ss_anim->frame_changing_state = 0x01;
			Audio_Send(weapon.shot, TR_AUDIO_EMITTER_ENTITY, ent->id);
			consumeAmmo(weapon);

			{
				collision_result_t cs;
				float from[3], to[3], tr[16], dir[3], t;
				ss_bone_tag_p bt = ent->bf->bone_tags + ent->character->bone_r_hand_end;

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
						return true;
					}
				}
			}

			// the reload sound is here !!!!
			// Audio_Send(weapon.reload_1, TR_AUDIO_EMITTER_ENTITY, ent->id);
		}
		else if (target)
		{
			// if you have a target -> lock the enemie
			Anim_SetAnimation(ss_anim, anim_idle, -1);
			return true;
		}
		else
		{
			// to idle animation
			Anim_SetAnimation(ss_anim, anim_firing_to_idle, 0);
			return true;
		}
	}

	return false;
}

bool OneHand_Firing(entity_s* ent, ss_animation_s* ss_anim, ss_bone_tag_p b_tag, entity_p target, float* target_pos, weapons_s weapon, float time, uint16_t targeted_bone_start, uint16_t targeted_bone_end, int anim_firing, int anim_firing_to_idle)
{
	if ((ss_anim->frame_changing_state >= 4) | Anim_IncTime(ss_anim, time * weapon.firerate))
	{
		if (ent->character->state.weapon_ready && ent->character->cmd.action)
		{
			Anim_SetAnimation(ss_anim, anim_firing, 0);
			ss_anim->frame_changing_state = 0x01;
			Audio_Send(weapon.shot, TR_AUDIO_EMITTER_ENTITY, ent->id);
			consumeAmmo(weapon);

			{
				collision_result_t cs;
				float from[3], to[3], tr[16];
				ss_bone_tag_p bt = ent->bf->bone_tags + targeted_bone_start;

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
					return true;
				}
			}
		}
		else
		{
			Anim_SetAnimation(ss_anim, anim_firing_to_idle, -1);
			return true;
		}
	}

	return false;
}

#pragma region weapons
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
	return rocketgun;
}

struct weapons_s getHarpoonGun()
{
	return harpoongun;
}

struct weapons_s getGrenadeGun()
{
	return grenadegun;
}

struct weapons_s getCrossbowGun()
{
	return crossbowgun;
}

struct weapons_s getGrapplinGun()
{
	return grapplingun;
}

/* 
 * - used to get all weapon in one function
 *- 0: pistol;
 *- 1: shotgun;
 *- 2: magnum;
 *- 3: uzi;
 *- 4: automags;
 *- 5: desert_eagle;
 *- 6: revolver;
 *- 7: m16;
 *- 8: mp5;
 *- 9: rocketgun;
 *- 10: harpoongun;
 *- 11: grenadegun; 
 *- 12: crossbowgun;
 *- 13: grapplingun;
 */
struct weapons_s *getAll()
{
	weapons_s weapons[14];
	weapons[0] = pistol;
	weapons[1] = shotgun;
	weapons[2] = magnum;
	weapons[3] = uzi;
	weapons[4] = automags;
	weapons[5] = deserteagle;
	weapons[6] = revolver;
	weapons[7] = m16;
	weapons[8] = mp5;
	weapons[9] = rocketgun;
	weapons[10] = harpoongun;
	weapons[11] = grenadegun;
	weapons[12] = crossbowgun;
	weapons[13] = grapplingun;
	return weapons;
}
#pragma endregion