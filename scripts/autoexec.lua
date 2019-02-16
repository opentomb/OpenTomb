dofile(base_path .. "scripts/character/weapons.lua");
dofile(base_path .. "scripts/misc.lua");

print("autoexec.lua->starting_items loaded !");

-----------------------------------
--   Tomb Raider Starting Items
-----------------------------------
local ver = getLevelVersion();
local level = getLevel();

addItem(player, ITEM_KEY_1, 1);
addItem(player, ITEM_KEY_2, 1);
addItem(player, ITEM_KEY_3, 1);
addItem(player, ITEM_KEY_4, 1);
addItem(player, ITEM_PUZZLE_1, 1);
addItem(player, ITEM_PUZZLE_2, 1);
addItem(player, ITEM_PUZZLE_3, 1);
addItem(player, ITEM_PUZZLE_4, 1);
addItem(player, ITEM_QUEST_1, 1);
addItem(player, ITEM_QUEST_2, 1);
addItem(player, ITEM_QUEST_3, 1);

addItem(player, ITEM_COMPASS, 1);
if (level > 0) then
	addItem(player, ITEM_PISTOL, 1);
end;

if (ver < TR_II) then
    addItem(player, ITEM_PASSPORT, 1);
    addItem(player, ITEM_CONTROLS, 1);
    addItem(player, ITEM_AUDIO, 1);
    addItem(player, ITEM_VIDEO, 1);
    
    if (level > 0) then
		addItem(player, ITEM_SHOTGUN, 1);
		addItem(player, ITEM_SHOTGUN_AMMO, 12);
		addItem(player, ITEM_UZI, 1);
		addItem(player, ITEM_UZI_AMMO, 12);
		addItem(player, ITEM_MAGNUM, 1);
		addItem(player, ITEM_MAGNUM_AMMO, 12);
	end;
elseif (ver < TR_III) then
    addItem(player, ITEM_PASSPORT, 1);
    addItem(player, ITEM_CONTROLS, 1);
    addItem(player, ITEM_AUDIO, 1);
    addItem(player, ITEM_VIDEO, 1);
    
    if (level > 0) then
		addItem(player, ITEM_SHOTGUN, 1);
		addItem(player, ITEM_SHOTGUN_AMMO, 1000);
		addItem(player, ITEM_UZI, 1);
		addItem(player, ITEM_UZI_AMMO, 1000);
		addItem(player, ITEM_AUTOMAGS, 1);
		addItem(player, ITEM_AUTOMAGS_AMMO, 1000);
		addItem(player, ITEM_M16, 1);
		addItem(player, ITEM_M16_AMMO, 1000);
		addItem(player, ITEM_HARPOONGUN, 1);
		addItem(player, ITEM_HARPOONGUN_AMMO, 1000);
		addItem(player, ITEM_GRENADEGUN, 1);
		addItem(player, ITEM_GRENADEGUN_AMMO, 1000);
	end;
elseif (ver < TR_IV) then
    addItem(player, ITEM_PASSPORT, 1);
    addItem(player, ITEM_CONTROLS, 1);
    addItem(player, ITEM_AUDIO, 1);
    addItem(player, ITEM_VIDEO, 1);
    
    if (level > 0) then
		addItem(player, ITEM_SHOTGUN, 1);
		addItem(player, ITEM_SHOTGUN_AMMO, 1000);
		addItem(player, ITEM_DESERTEAGLE, 1);
		addItem(player, ITEM_DESERTEAGLE_AMMO, 1000);
		addItem(player, ITEM_UZI, 1);
		addItem(player, ITEM_UZI_AMMO, 1000);
		addItem(player, ITEM_HARPOONGUN, 1);
		addItem(player, ITEM_HARPOONGUN_AMMO, 1000);
		addItem(player, ITEM_MP5, 1);
		addItem(player, ITEM_MP5_AMMO, 1000);
		addItem(player, ITEM_ROCKETGUN, 1);
		addItem(player, ITEM_ROCKETGUN_AMMO, 1000);
		addItem(player, ITEM_GRENADEGUN, 1);
		addItem(player, ITEM_GRENADEGUN_AMMO, 1000);
	end;
elseif (ver < TR_V) then
    print("autoexec->give_item: no system item for TR4 and TR4 Demo is implemented now !");
    -- no home in this version
	
    addItem(player, ITEM_UZI, 1);
    addItem(player, ITEM_UZI_AMMO, 1000);
    addItem(player, ITEM_SHOTGUN, 1);
    addItem(player, ITEM_SHOTGUN_NORMAL_AMMO, 1000);
    addItem(player, ITEM_SHOTGUN_WIDESHOT_AMMO, 1000);
    addItem(player, ITEM_CROSSBOW, 1);
    addItem(player, ITEM_CROSSBOW_NORMAL_AMMO, 1000);
    addItem(player, ITEM_CROSSBOW_POISON_AMMO, 1000);
    addItem(player, ITEM_CROSSBOW_EXPLOSIVE_AMMO, 1000);
    addItem(player, ITEM_GRENADEGUN, 1);
    addItem(player, ITEM_GRENADEGUN_NORMAL_AMMO, 1000);
    addItem(player, ITEM_GRENADEGUN_SUPER_AMMO, 1000);
    addItem(player, ITEM_GRENADEGUN_FLASH_AMMO, 1000);
    addItem(player, ITEM_REVOLVER, 1);
    addItem(player, ITEM_REVOLVER_AMMO, 1000);
elseif (ver == TR_V) then
    print("autoexec->give_item: no system item for TR5 is implemented now !");
    print("autoexec->give_item: you can only have revolver in the starting level !");
    -- no home in this version
	
	addItem(player, ITEM_UZI, 1);
	addItem(player, ITEM_UZI_AMMO, 1000);
	addItem(player, ITEM_SHOTGUN, 1);
	addItem(player, ITEM_SHOTGUN_NORMAL_AMMO, 1000);
	addItem(player, ITEM_SHOTGUN_WIDESHOT_AMMO, 1000);
    addItem(player, ITEM_REVOLVER, 1);
    addItem(player, ITEM_REVOLVER_AMMO, 1000);
	addItem(player, ITEM_MP5, 1);
    addItem(player, ITEM_MP5_AMMO, 1000);
	addItem(player, ITEM_GRAPPLEGUN, 1);
    addItem(player, ITEM_GRAPPLEGUN_AMMO, 1000);
end;

addItem(player, ITEM_SMALL_MEDIPACK, 3);
addItem(player, ITEM_LARGE_MEDIPACK, 1);

-----------------------------------
--  !Tomb Raider Starting Items
-----------------------------------