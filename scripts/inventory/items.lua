-- OPENTOMB INVENTORY ITEM DESCRIPTOR
-- by TeslaRus, Sep 2014

--------------------------------------------------------------------------------
-- Assigns global pickups for each game version.
-- Global pickups are items that can be found anywhere in the game, i. e. it is
-- NOT quest items, keys, puzzles and other level-specific pickups.
--------------------------------------------------------------------------------

local ver = getGameVersion();

if(ver < TR_II) then
    createBaseItem(ITEM_PASSPORT, 71, 71, ITEM_NAME_PASSPORT);
    createBaseItem(ITEM_VIDEO, 95, 95, ITEM_NAME_VIDEO);
    createBaseItem(ITEM_AUDIO, 96, 96, ITEM_NAME_AUDIO); 
    createBaseItem(ITEM_CONTROLS, 97, 97, ITEM_NAME_CONTROLS);
    
    createBaseItem(ITEM_COMPASS, 72, 72, ITEM_NAME_COMPASS);
    createBaseItem(ITEM_MAP, 82, 82, ITEM_NAME_MAP);

    createBaseItem(ITEM_PISTOLS, 99, 99, ITEM_NAME_PISTOLS);
    createBaseItem(ITEM_SHOTGUN, 100, 100, ITEM_NAME_SHOTGUN);
    createBaseItem(ITEM_MAGNUMS, 101, 101, ITEM_NAME_MAGNUMS);
    createBaseItem(ITEM_UZIS, 102, 102, ITEM_NAME_UZIS);
    
    createBaseItem(ITEM_PISTOL_AMMO, 103, 103, ITEM_NAME_PISTOL_AMMO);
    createBaseItem(ITEM_SHOTGUN_NORMAL_AMMO, 104, 104, ITEM_NAME_SHOTGUN_AMMO);
    createBaseItem(ITEM_MAGNUM_AMMO, 105, 105, ITEM_NAME_MAGNUM_AMMO);
    createBaseItem(ITEM_UZI_AMMO, 106, 106, ITEM_NAME_UZI_AMMO);

    createBaseItem(ITEM_SMALL_MEDIPACK, 108, 108, ITEM_NAME_SMALL_MEDIPACK);
    createBaseItem(ITEM_LARGE_MEDIPACK, 109, 109, ITEM_NAME_LARGE_MEDIPACK);

elseif(ver < TR_III) then
    createBaseItem(ITEM_PASSPORT, 120, 120, ITEM_NAME_PASSPORT);
    createBaseItem(ITEM_VIDEO, 153, 153, ITEM_NAME_VIDEO);
    createBaseItem(ITEM_AUDIO, 154, 154, ITEM_NAME_AUDIO);
    createBaseItem(ITEM_CONTROLS, 155, 155, ITEM_NAME_CONTROLS);

    createBaseItem(ITEM_COMPASS, 121, 121, ITEM_NAME_COMPASS);
    createBaseItem(ITEM_MAP, 134, 134, ITEM_NAME_MAP);
    
    createBaseItem(ITEM_PISTOLS, 157, 157, ITEM_NAME_PISTOLS);
    createBaseItem(ITEM_SHOTGUN, 158, 158, ITEM_NAME_SHOTGUN);
    createBaseItem(ITEM_MAGNUMS, 159, 159, ITEM_NAME_MAGNUMS);
    createBaseItem(ITEM_UZIS, 160, 160, ITEM_NAME_UZIS);
    createBaseItem(ITEM_HARPOONGUN, 161, 161, ITEM_NAME_HARPOONGUN);
    createBaseItem(ITEM_M16, 162, 162, ITEM_NAME_M16);
    createBaseItem(ITEM_GRENADEGUN, 163, 163, ITEM_NAME_GRENADEGUN);

    createBaseItem(ITEM_PISTOL_AMMO, 164, 164, ITEM_NAME_PISTOL_AMMO);
    createBaseItem(ITEM_SHOTGUN_NORMAL_AMMO, 165, 165, ITEM_NAME_SHOTGUN_AMMO);
    createBaseItem(ITEM_MAGNUM_AMMO, 166, 166, ITEM_NAME_MAGNUM_AMMO);
    createBaseItem(ITEM_UZI_AMMO, 167, 167, ITEM_NAME_UZI_AMMO);
    createBaseItem(ITEM_HARPOONGUN_AMMO, 168, 168, ITEM_NAME_HARPOONGUN_AMMO);
    createBaseItem(ITEM_M16_AMMO, 169, 169, ITEM_NAME_M16_AMMO);
    createBaseItem(ITEM_GRENADEGUN_AMMO, 170, 170, ITEM_NAME_GRENADEGUN_AMMO);

    createBaseItem(ITEM_SMALL_MEDIPACK, 171, 171, ITEM_NAME_SMALL_MEDIPACK);
    createBaseItem(ITEM_LARGE_MEDIPACK, 172, 172, ITEM_NAME_LARGE_MEDIPACK);
    createBaseItem(ITEM_FLARES, 173, 173, ITEM_NAME_FLARES);
    createBaseItem(ITEM_SINGLE_FLARE, 152, 152);
    
elseif(ver < TR_IV) then
    createBaseItem(ITEM_PASSPORT, 145, 145, ITEM_NAME_PASSPORT);
    createBaseItem(ITEM_VIDEO, 181, 181, ITEM_NAME_VIDEO);
    createBaseItem(ITEM_AUDIO, 182, 182, ITEM_NAME_AUDIO);
    createBaseItem(ITEM_CONTROLS, 183, 183, ITEM_NAME_CONTROLS);
    
    createBaseItem(ITEM_COMPASS, 146, 146, ITEM_NAME_COMPASS);
    createBaseItem(ITEM_MAP, 159, 159, ITEM_NAME_MAP);
    
    createBaseItem(ITEM_PISTOLS, 185, 160, ITEM_NAME_PISTOLS);
    createBaseItem(ITEM_SHOTGUN, 186, 161, ITEM_NAME_SHOTGUN);
    createBaseItem(ITEM_MAGNUMS, 187, 162, ITEM_NAME_MAGNUMS);
    createBaseItem(ITEM_UZIS, 188, 163, ITEM_NAME_UZIS);
    createBaseItem(ITEM_HARPOONGUN, 189, 164, ITEM_NAME_HARPOONGUN);
    createBaseItem(ITEM_M16, 190, 165, ITEM_NAME_MP5);
    createBaseItem(ITEM_ROCKETGUN, 191, 166, ITEM_NAME_ROCKETGUN);
    createBaseItem(ITEM_GRENADEGUN, 192, 167, ITEM_NAME_GRENADEGUN);

    createBaseItem(ITEM_PISTOL_AMMO, 193, 168, ITEM_NAME_PISTOL_AMMO);
    createBaseItem(ITEM_SHOTGUN_NORMAL_AMMO, 194, 169, ITEM_NAME_SHOTGUN_AMMO);
    createBaseItem(ITEM_MAGNUM_AMMO, 195, 170, ITEM_NAME_MAGNUM_AMMO);
    createBaseItem(ITEM_UZI_AMMO, 196, 171, ITEM_NAME_UZI_AMMO);
    createBaseItem(ITEM_HARPOONGUN_AMMO, 197, 172, ITEM_NAME_HARPOONGUN_AMMO);
    createBaseItem(ITEM_M16_AMMO, 198, 173, ITEM_NAME_MP5_AMMO);
    createBaseItem(ITEM_ROCKETGUN_AMMO, 199, 174, ITEM_NAME_ROCKETGUN_AMMO);
    createBaseItem(ITEM_GRENADEGUN_AMMO, 200, 175, ITEM_NAME_GRENADEGUN_AMMO);

    createBaseItem(ITEM_SMALL_MEDIPACK, 201, 176, ITEM_NAME_SMALL_MEDIPACK);
    createBaseItem(ITEM_LARGE_MEDIPACK, 202, 177, ITEM_NAME_LARGE_MEDIPACK);
    createBaseItem(ITEM_FLARES, 203, 178, ITEM_NAME_FLARES);
    createBaseItem(ITEM_SINGLE_FLARE, 179, 179);
    
elseif(ver < TR_V) then
    createBaseItem(ITEM_COMPASS, 375, 375, ITEM_NAME_COMPASS);
    createBaseItem(ITEM_LOAD, 376, 376, ITEM_NAME_LOAD);
    createBaseItem(ITEM_SAVE, 377, 377, ITEM_NAME_SAVE);

    createBaseItem(ITEM_PISTOLS, 349, 349, ITEM_NAME_PISTOLS);
    createBaseItem(ITEM_MAGNUMS, 366, 366, ITEM_NAME_MAGNUMS);
    createBaseItem(ITEM_UZIS, 351, 351, ITEM_NAME_UZIS);
    createBaseItem(ITEM_SHOTGUN, 353, 353, ITEM_NAME_SHOTGUN);
    createBaseItem(ITEM_CROSSBOW, 356, 356, ITEM_NAME_CROSSBOW);
    createBaseItem(ITEM_GRENADEGUN, 361, 361, ITEM_NAME_GRENADEGUN);
    
    createBaseItem(ITEM_PISTOL_AMMO, 350, 350, ITEM_NAME_PISTOL_AMMO);
    createBaseItem(ITEM_MAGNUM_AMMO, 367, 367, ITEM_NAME_MAGNUM_AMMO);
    createBaseItem(ITEM_UZI_AMMO, 352, 352, ITEM_NAME_UZI_AMMO);
    createBaseItem(ITEM_SHOTGUN_NORMAL_AMMO, 354, 354, ITEM_NAME_SHOTGUN_NORMAL_AMMO);
    createBaseItem(ITEM_SHOTGUN_WIDESHOT_AMMO, 355, 355, ITEM_NAME_SHOTGUN_WIDESHOT_AMMO);
    createBaseItem(ITEM_CROSSBOW_NORMAL_AMMO, 357, 357, ITEM_NAME_CROSSBOW_NORMAL_AMMO);
    createBaseItem(ITEM_CROSSBOW_POISON_AMMO, 358, 358, ITEM_NAME_CROSSBOW_POISON_AMMO);
    createBaseItem(ITEM_CROSSBOW_EXPLOSIVE_AMMO, 359, 359, ITEM_NAME_CROSSBOW_EXPLOSIVE_AMMO);
    createBaseItem(ITEM_GRENADEGUN_NORMAL_AMMO, 362, 362, ITEM_NAME_GRENADEGUN_NORMAL_AMMO);
    createBaseItem(ITEM_GRENADEGUN_SUPER_AMMO, 363, 363, ITEM_NAME_GRENADEGUN_SUPER_AMMO);
    createBaseItem(ITEM_GRENADEGUN_FLASH_AMMO, 364, 364, ITEM_NAME_GRENADEGUN_FLASH_AMMO);
    
    createBaseItem(ITEM_LASERSIGHT, 370, 370, ITEM_NAME_LASERSIGHT);
    createBaseItem(ITEM_BINOCULARS, 371, 371, ITEM_NAME_BINOCULARS);

    createBaseItem(ITEM_LARGE_MEDIPACK, 368, 368, ITEM_NAME_LARGE_MEDIPACK);
    createBaseItem(ITEM_SMALL_MEDIPACK, 369, 369, ITEM_NAME_SMALL_MEDIPACK);
    createBaseItem(ITEM_FLARES, 373, 373, ITEM_NAME_FLARES);
    createBaseItem(ITEM_SINGLE_FLARE, 372, 372);
    createBaseItem(ITEM_TORCH, 247, 247);
    
elseif(ver == TR_V) then
    createBaseItem(ITEM_COMPASS, 356, 356, ITEM_NAME_COMPASS);
    createBaseItem(ITEM_LOAD, 357, 357, ITEM_NAME_LOAD);
    createBaseItem(ITEM_SAVE, 358, 358, ITEM_NAME_SAVE);

    createBaseItem(ITEM_PISTOLS, 334, 334, ITEM_NAME_PISTOLS);
    createBaseItem(ITEM_UZIS, 336, 336, ITEM_NAME_UZIS);
    createBaseItem(ITEM_SHOTGUN, 338, 338, ITEM_NAME_SHOTGUN);
    createBaseItem(ITEM_M16, 345, 345, ITEM_NAME_HK);
    createBaseItem(ITEM_GRAPPLEGUN, 341, 341, ITEM_NAME_GRAPPLEGUN);
    
    createBaseItem(ITEM_PISTOL_AMMO, 335, 335, ITEM_NAME_PISTOL_AMMO);
    createBaseItem(ITEM_UZI_AMMO, 337, 337, ITEM_NAME_UZI_AMMO);
    createBaseItem(ITEM_SHOTGUN_NORMAL_AMMO, 339, 339, ITEM_NAME_SHOTGUN_NORMAL_AMMO);
    createBaseItem(ITEM_SHOTGUN_WIDESHOT_AMMO, 340, 340, ITEM_NAME_SHOTGUN_WIDESHOT_AMMO);
    createBaseItem(ITEM_M16_AMMO, 346, 346, ITEM_NAME_HK_AMMO);
    createBaseItem(ITEM_GRAPPLEGUN_AMMO, 342, 342, ITEM_NAME_GRAPPLEGUN_AMMO);
    
    -- Magnum slot is shared between Revolver and Desert Eagle in Rome/Russia levels,
    -- so we just left the name uninitialized until level script is used.
    createBaseItem(ITEM_MAGNUMS, 347, 347);
    createBaseItem(ITEM_MAGNUM_AMMO, 348, 348);
    
    createBaseItem(ITEM_LASERSIGHT, 351, 351, ITEM_NAME_LASERSIGHT);
    createBaseItem(ITEM_BINOCULARS, 352, 352, ITEM_NAME_BINOCULARS);

    createBaseItem(ITEM_LARGE_MEDIPACK, 349, 349, ITEM_NAME_LARGE_MEDIPACK);
    createBaseItem(ITEM_SMALL_MEDIPACK, 350, 350, ITEM_NAME_SMALL_MEDIPACK);
    createBaseItem(ITEM_FLARES, 355, 355, ITEM_NAME_FLARES);
    createBaseItem(ITEM_SINGLE_FLARE, 354, 354);
    createBaseItem(ITEM_TORCH, 241, 241);
    
end

-- test TR_ITEMS

addItem(player, ITEM_COMPASS, 1);
addItem(player, ITEM_SMALL_MEDIPACK, 3);
addItem(player, ITEM_LARGE_MEDIPACK, 1);

--[[
addItem(player, ITEM_VIDEO, 1);
addItem(player, ITEM_AUDIO, 1);
addItem(player, ITEM_CONTROLS, 1);
addItem(player, ITEM_LOAD, 1);
addItem(player, ITEM_SAVE, 1);

addItem(player, ITEM_PISTOLS, 1);
addItem(player, ITEM_SHOTGUN, 1);
addItem(player, ITEM_MAGNUMS, 1);
addItem(player, ITEM_UZIS, 1);
addItem(player, ITEM_M16, 1);
addItem(player, ITEM_GRENADEGUN, 1);
addItem(player, ITEM_ROCKETGUN, 1);
addItem(player, ITEM_HARPOONGUN, 1);
addItem(player, ITEM_CROSSBOW, 1);
addItem(player, ITEM_GRAPPLEGUN, 1);
addItem(player, ITEM_LASERSIGHT, 1);
addItem(player, ITEM_BINOCULARS, 1);

addItem(player, ITEM_PISTOL_AMMO, 1);
addItem(player, ITEM_SHOTGUN_NORMAL_AMMO, 1);
addItem(player, ITEM_SHOTGUN_WIDESHOT_AMMO, 1);
addItem(player, ITEM_MAGNUM_AMMO, 1);
addItem(player, ITEM_UZI_AMMO, 1);
addItem(player, ITEM_M16_AMMO, 1);
addItem(player, ITEM_GRENADEGUN_NORMAL_AMMO, 1);
addItem(player, ITEM_GRENADEGUN_SUPER_AMMO, 1);
addItem(player, ITEM_GRENADEGUN_FLASH_AMMO, 1);
addItem(player, ITEM_ROCKETGUN_AMMO, 1);
addItem(player, ITEM_HARPOONGUN_AMMO, 1);
addItem(player, ITEM_CROSSBOW_NORMAL_AMMO, 1);
addItem(player, ITEM_CROSSBOW_POISON_AMMO, 1);
addItem(player, ITEM_CROSSBOW_EXPLOSIVE_AMMO, 1);
addItem(player, ITEM_GRAPPLEGUN_AMMO, 1);

addItem(player, ITEM_FLARES, 1);
addItem(player, ITEM_SINGLE_FLARE, 1);
--]]
print("items script loaded");
