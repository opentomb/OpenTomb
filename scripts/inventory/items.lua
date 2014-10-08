-- OPENTOMB INVENTORY ITEM DESCRIPTOR
-- by TeslaRus, Sep 2014

--------------------------------------------------------------------------------
-- Assigns global pickups for each game version.
-- Global pickups are items that can be found anywhere in the game, i. e. it is
-- NOT quest items, keys, puzzles and other level-specific pickups.
--------------------------------------------------------------------------------

local ver = getGameVersion();

if(ver < TR_II) then
    createBaseItem(ITEM_PASSPORT, 71, 71, 0, ITEM_NAME_PASSPORT);
    createBaseItem(ITEM_VIDEO, 95, 95, 0, ITEM_NAME_VIDEO);
    createBaseItem(ITEM_AUDIO, 96, 96, 0, ITEM_NAME_AUDIO); 
    createBaseItem(ITEM_CONTROLS, 97, 97, 0, ITEM_NAME_CONTROLS);
    
    createBaseItem(ITEM_COMPASS, 72, 72, 0, ITEM_NAME_COMPASS);
    createBaseItem(ITEM_MAP, 82, 82, 0, ITEM_NAME_MAP);

    createBaseItem(ITEM_PISTOLS, 99, 99, 1, ITEM_NAME_PISTOLS);
    createBaseItem(ITEM_SHOTGUN, 100, 100, 1, ITEM_NAME_SHOTGUN);
    createBaseItem(ITEM_MAGNUMS, 101, 101, 1, ITEM_NAME_MAGNUMS);
    createBaseItem(ITEM_UZIS, 102, 102, 1, ITEM_NAME_UZIS);
    
    createBaseItem(ITEM_PISTOL_AMMO, 103, 103, 1, ITEM_NAME_PISTOL_AMMO);
    createBaseItem(ITEM_SHOTGUN_NORMAL_AMMO, 104, 104, 1, ITEM_NAME_SHOTGUN_AMMO);
    createBaseItem(ITEM_MAGNUM_AMMO, 105, 105, 1, ITEM_NAME_MAGNUM_AMMO);
    createBaseItem(ITEM_UZI_AMMO, 106, 106, 1, ITEM_NAME_UZI_AMMO);

    createBaseItem(ITEM_SMALL_MEDIPACK, 108, 108, 1, ITEM_NAME_SMALL_MEDIPACK);
    createBaseItem(ITEM_LARGE_MEDIPACK, 109, 109, 1, ITEM_NAME_LARGE_MEDIPACK);

elseif(ver < TR_III) then
    createBaseItem(ITEM_PASSPORT, 120, 120, ITEM_NAME_PASSPORT);
    createBaseItem(ITEM_VIDEO, 153, 153, ITEM_NAME_VIDEO);
    createBaseItem(ITEM_AUDIO, 154, 154, ITEM_NAME_AUDIO);
    createBaseItem(ITEM_CONTROLS, 155, 155, ITEM_NAME_CONTROLS);

    createBaseItem(ITEM_COMPASS, 121, 121, ITEM_NAME_COMPASS);
    createBaseItem(ITEM_MAP, 134, 134, ITEM_NAME_MAP);
    
    createBaseItem(ITEM_PISTOLS, 157, 157, 1, ITEM_NAME_PISTOLS);
    createBaseItem(ITEM_SHOTGUN, 158, 158, 1, ITEM_NAME_SHOTGUN);
    createBaseItem(ITEM_MAGNUMS, 159, 159, 1, ITEM_NAME_MAGNUMS);
    createBaseItem(ITEM_UZIS, 160, 160, 1, ITEM_NAME_UZIS);
    createBaseItem(ITEM_HARPOONGUN, 161, 161, 1, ITEM_NAME_HARPOONGUN);
    createBaseItem(ITEM_M16, 162, 162, 1, ITEM_NAME_M16);
    createBaseItem(ITEM_GRENADEGUN, 163, 163, 1, ITEM_NAME_GRENADEGUN);

    createBaseItem(ITEM_PISTOL_AMMO, 164, 164, 1, ITEM_NAME_PISTOL_AMMO);
    createBaseItem(ITEM_SHOTGUN_NORMAL_AMMO, 165, 165, 1, ITEM_NAME_SHOTGUN_AMMO);
    createBaseItem(ITEM_MAGNUM_AMMO, 166, 166, 1, ITEM_NAME_MAGNUM_AMMO);
    createBaseItem(ITEM_UZI_AMMO, 167, 167, 1, ITEM_NAME_UZI_AMMO);
    createBaseItem(ITEM_HARPOONGUN_AMMO, 168, 168, 1, ITEM_NAME_HARPOONGUN_AMMO);
    createBaseItem(ITEM_M16_AMMO, 169, 169, 1, ITEM_NAME_M16_AMMO);
    createBaseItem(ITEM_GRENADEGUN_AMMO, 170, 170, 1, ITEM_NAME_GRENADEGUN_AMMO);

    createBaseItem(ITEM_SMALL_MEDIPACK, 171, 171, 1, ITEM_NAME_SMALL_MEDIPACK);
    createBaseItem(ITEM_LARGE_MEDIPACK, 172, 172, 1, ITEM_NAME_LARGE_MEDIPACK);
    createBaseItem(ITEM_FLARES, 173, 173, 1, ITEM_NAME_FLARES);
    createBaseItem(ITEM_SINGLE_FLARE, 152, 152, 1);
    
elseif(ver < TR_IV) then
    createBaseItem(ITEM_PASSPORT, 145, 145, 0, ITEM_NAME_PASSPORT);
    createBaseItem(ITEM_VIDEO, 181, 181, 0, ITEM_NAME_VIDEO);
    createBaseItem(ITEM_AUDIO, 182, 182, 0, ITEM_NAME_AUDIO);
    createBaseItem(ITEM_CONTROLS, 183, 183, 0, ITEM_NAME_CONTROLS);
    
    createBaseItem(ITEM_COMPASS, 146, 146, 0, ITEM_NAME_COMPASS);
    createBaseItem(ITEM_MAP, 159, 159, 0, ITEM_NAME_MAP);
    
    createBaseItem(ITEM_PISTOLS, 185, 160, 1, ITEM_NAME_PISTOLS);
    createBaseItem(ITEM_SHOTGUN, 186, 161, 1, ITEM_NAME_SHOTGUN);
    createBaseItem(ITEM_MAGNUMS, 187, 162, 1, ITEM_NAME_MAGNUMS);
    createBaseItem(ITEM_UZIS, 188, 163, 1, ITEM_NAME_UZIS);
    createBaseItem(ITEM_HARPOONGUN, 189, 164, 1, ITEM_NAME_HARPOONGUN);
    createBaseItem(ITEM_M16, 190, 165, 1, ITEM_NAME_MP5);
    createBaseItem(ITEM_ROCKETGUN, 191, 166, 1, ITEM_NAME_ROCKETGUN);
    createBaseItem(ITEM_GRENADEGUN, 192, 167, 1, ITEM_NAME_GRENADEGUN);

    createBaseItem(ITEM_PISTOL_AMMO, 193, 168, 1, ITEM_NAME_PISTOL_AMMO);
    createBaseItem(ITEM_SHOTGUN_NORMAL_AMMO, 194, 1, 169, ITEM_NAME_SHOTGUN_AMMO);
    createBaseItem(ITEM_MAGNUM_AMMO, 195, 170, 1, ITEM_NAME_MAGNUM_AMMO);
    createBaseItem(ITEM_UZI_AMMO, 196, 171, 1, ITEM_NAME_UZI_AMMO);
    createBaseItem(ITEM_HARPOONGUN_AMMO, 197, 172, 1, ITEM_NAME_HARPOONGUN_AMMO);
    createBaseItem(ITEM_M16_AMMO, 198, 173, 1, ITEM_NAME_MP5_AMMO);
    createBaseItem(ITEM_ROCKETGUN_AMMO, 199, 174, 1, ITEM_NAME_ROCKETGUN_AMMO);
    createBaseItem(ITEM_GRENADEGUN_AMMO, 200, 175, 1, ITEM_NAME_GRENADEGUN_AMMO);

    createBaseItem(ITEM_SMALL_MEDIPACK, 201, 176, 1, ITEM_NAME_SMALL_MEDIPACK);
    createBaseItem(ITEM_LARGE_MEDIPACK, 202, 177, 1, ITEM_NAME_LARGE_MEDIPACK);
    createBaseItem(ITEM_FLARES, 203, 178, 1, ITEM_NAME_FLARES);
    createBaseItem(ITEM_SINGLE_FLARE, 179, 179, 1);
    
elseif(ver < TR_V) then
    createBaseItem(ITEM_COMPASS, 375, 375, 0, ITEM_NAME_COMPASS);
    createBaseItem(ITEM_LOAD, 376, 376, 0, ITEM_NAME_LOAD);
    createBaseItem(ITEM_SAVE, 377, 377, 0, ITEM_NAME_SAVE);

    createBaseItem(ITEM_PISTOLS, 349, 349, 1, ITEM_NAME_PISTOLS);
    createBaseItem(ITEM_MAGNUMS, 366, 366, 1, ITEM_NAME_MAGNUMS);
    createBaseItem(ITEM_UZIS, 351, 351, 1, ITEM_NAME_UZIS);
    createBaseItem(ITEM_SHOTGUN, 353, 353, 1, ITEM_NAME_SHOTGUN);
    createBaseItem(ITEM_CROSSBOW, 356, 356, 1, ITEM_NAME_CROSSBOW);
    createBaseItem(ITEM_GRENADEGUN, 361, 361, 1, ITEM_NAME_GRENADEGUN);
    
    createBaseItem(ITEM_PISTOL_AMMO, 350, 350, 1, ITEM_NAME_PISTOL_AMMO);
    createBaseItem(ITEM_MAGNUM_AMMO, 367, 367, 1, ITEM_NAME_MAGNUM_AMMO);
    createBaseItem(ITEM_UZI_AMMO, 352, 352, 1, ITEM_NAME_UZI_AMMO);
    createBaseItem(ITEM_SHOTGUN_NORMAL_AMMO, 354, 354, 1, ITEM_NAME_SHOTGUN_NORMAL_AMMO);
    createBaseItem(ITEM_SHOTGUN_WIDESHOT_AMMO, 355, 355, 1, ITEM_NAME_SHOTGUN_WIDESHOT_AMMO);
    createBaseItem(ITEM_CROSSBOW_NORMAL_AMMO, 357, 357, 1, ITEM_NAME_CROSSBOW_NORMAL_AMMO);
    createBaseItem(ITEM_CROSSBOW_POISON_AMMO, 358, 358, 1, ITEM_NAME_CROSSBOW_POISON_AMMO);
    createBaseItem(ITEM_CROSSBOW_EXPLOSIVE_AMMO, 359, 359, 1, ITEM_NAME_CROSSBOW_EXPLOSIVE_AMMO);
    createBaseItem(ITEM_GRENADEGUN_NORMAL_AMMO, 362, 362, 1, ITEM_NAME_GRENADEGUN_NORMAL_AMMO);
    createBaseItem(ITEM_GRENADEGUN_SUPER_AMMO, 363, 363, 1, ITEM_NAME_GRENADEGUN_SUPER_AMMO);
    createBaseItem(ITEM_GRENADEGUN_FLASH_AMMO, 364, 364, 1, ITEM_NAME_GRENADEGUN_FLASH_AMMO);
    
    createBaseItem(ITEM_LASERSIGHT, 370, 370, 1, ITEM_NAME_LASERSIGHT);
    createBaseItem(ITEM_BINOCULARS, 371, 371, 1, ITEM_NAME_BINOCULARS);

    createBaseItem(ITEM_LARGE_MEDIPACK, 368, 368, 1, ITEM_NAME_LARGE_MEDIPACK);
    createBaseItem(ITEM_SMALL_MEDIPACK, 369, 369, 1, ITEM_NAME_SMALL_MEDIPACK);
    createBaseItem(ITEM_FLARES, 373, 373, 1, ITEM_NAME_FLARES);
    createBaseItem(ITEM_SINGLE_FLARE, 372, 372, 1);
    createBaseItem(ITEM_TORCH, 247, 247, 1);
    
elseif(ver == TR_V) then
    createBaseItem(ITEM_COMPASS, 356, 356, 0, ITEM_NAME_COMPASS);
    createBaseItem(ITEM_LOAD, 357, 357, 0, ITEM_NAME_LOAD);
    createBaseItem(ITEM_SAVE, 358, 358, 0, ITEM_NAME_SAVE);

    createBaseItem(ITEM_PISTOLS, 334, 334, 1, ITEM_NAME_PISTOLS);
    createBaseItem(ITEM_UZIS, 336, 336, 1, ITEM_NAME_UZIS);
    createBaseItem(ITEM_SHOTGUN, 338, 338, 1, ITEM_NAME_SHOTGUN);
    createBaseItem(ITEM_M16, 345, 345, 1, ITEM_NAME_HK);
    createBaseItem(ITEM_GRAPPLEGUN, 341, 341, 1, ITEM_NAME_GRAPPLEGUN);
    
    createBaseItem(ITEM_PISTOL_AMMO, 335, 335, 1, ITEM_NAME_PISTOL_AMMO);
    createBaseItem(ITEM_UZI_AMMO, 337, 337, 1, ITEM_NAME_UZI_AMMO);
    createBaseItem(ITEM_SHOTGUN_NORMAL_AMMO, 339, 339, 1, ITEM_NAME_SHOTGUN_NORMAL_AMMO);
    createBaseItem(ITEM_SHOTGUN_WIDESHOT_AMMO, 340, 340, 1, ITEM_NAME_SHOTGUN_WIDESHOT_AMMO);
    createBaseItem(ITEM_M16_AMMO, 346, 346, 1, ITEM_NAME_HK_AMMO);
    createBaseItem(ITEM_GRAPPLEGUN_AMMO, 342, 342, 1, ITEM_NAME_GRAPPLEGUN_AMMO);
    
    -- Magnum slot is shared between Revolver and Desert Eagle in Rome/Russia levels,
    -- so we just left the name uninitialized until level script is used.
    createBaseItem(ITEM_MAGNUMS, 347, 347, 1);
    createBaseItem(ITEM_MAGNUM_AMMO, 348, 348, 1);
    
    createBaseItem(ITEM_LASERSIGHT, 351, 351, 1, ITEM_NAME_LASERSIGHT);
    createBaseItem(ITEM_BINOCULARS, 352, 352, 1, ITEM_NAME_BINOCULARS);

    createBaseItem(ITEM_LARGE_MEDIPACK, 349, 349, 1, ITEM_NAME_LARGE_MEDIPACK);
    createBaseItem(ITEM_SMALL_MEDIPACK, 350, 350, 1, ITEM_NAME_SMALL_MEDIPACK);
    createBaseItem(ITEM_FLARES, 355, 355, 1, ITEM_NAME_FLARES);
    createBaseItem(ITEM_SINGLE_FLARE, 354, 354, 1);
    createBaseItem(ITEM_TORCH, 241, 241, 1);
    
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
