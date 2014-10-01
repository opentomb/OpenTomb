-- OPENTOMB INVENTORY ITEM DESCRIPTOR
-- by TeslaRus, Sep 2014

--------------------------------------------------------------------------------
-- Assigns global pickups for each game version.
-- Global pickups are items that can be found anywhere in the game, i. e. it is
-- NOT quest items, keys, puzzles and other level-specific pickups.
--------------------------------------------------------------------------------

local ver = getGameVersion();

if(ver < TR_II) then
    createBaseItem(ITEM_PASSPORT, 71, 71);                  -- item: journal
    createBaseItem(ITEM_COMPASS, 72, 72);                   -- item: compas (in difference levels model id's may be differet)
    createBaseItem(ITEM_VIDEO, 95, 95);                     -- item: video
    createBaseItem(ITEM_AUDIO, 96, 96);                     -- item: audio
    createBaseItem(ITEM_CONTROLS, 97, 97);                  -- item: control

    createBaseItem(ITEM_PISTOLS, 99, 99);                   -- item: pistols
    createBaseItem(ITEM_SHOTGUN, 100, 100);                 -- item: shotgun
    createBaseItem(ITEM_MAGNUMS, 101, 101);                 -- item: automatic pistols
    createBaseItem(ITEM_UZIS, 102, 102);                    -- item: uzi
    createBaseItem(ITEM_PISTOL_AMMO, 103, 103);             -- item: pistols ammo
    createBaseItem(ITEM_SHOTGUN_NORMAL_AMMO, 104, 104);     -- item: shotgun ammo
    createBaseItem(ITEM_MAGNUM_AMMO, 105, 105);             -- item: automatic pistols ammo
    createBaseItem(ITEM_UZI_AMMO, 106, 106);                -- item: uzi ammo

    createBaseItem(ITEM_SMALL_MEDIPACK, 108, 108);          -- item: little MEDIPACK
    createBaseItem(ITEM_LARGE_MEDIPACK, 109, 109);          -- item: large MEDIPACK

    addItem(0, ITEM_COMPASS, 1);                            -- add compas

elseif(ver < TR_III) then
    createBaseItem(ITEM_PASSPORT, 120, 120);                -- item: journal
    createBaseItem(ITEM_COMPASS, 121, 121);                 -- item: compas or timer?
    createBaseItem(ITEM_VIDEO, 153, 153);                   -- item: video
    createBaseItem(ITEM_AUDIO, 154, 154);                   -- item: audio
    createBaseItem(ITEM_CONTROLS, 155, 155);                -- item: control

    createBaseItem(ITEM_PISTOLS, 157, 157);                 -- item: pistols
    createBaseItem(ITEM_SHOTGUN, 158, 158);                 -- item: shotgun
    createBaseItem(ITEM_MAGNUMS, 159, 159);                 -- item: automatic pistols
    createBaseItem(ITEM_UZIS, 160, 160);                    -- item: uzi
    createBaseItem(ITEM_HARPOONGUN, 161, 161);              -- item: harpoon
    createBaseItem(ITEM_M16, 162, 162);                     -- item: M16
    createBaseItem(ITEM_GRENADEGUN, 163, 163);              -- item: grenade launcher

    createBaseItem(ITEM_PISTOL_AMMO, 164, 164);             -- item: pistols ammo
    createBaseItem(ITEM_SHOTGUN_NORMAL_AMMO, 165, 165);     -- item: shotgun ammo
    createBaseItem(ITEM_MAGNUM_AMMO, 166, 166);             -- item: automatic pistols ammo
    createBaseItem(ITEM_UZI_AMMO, 167, 167);                -- item: uzi ammo
    createBaseItem(ITEM_HARPOONGUN_AMMO, 168, 168);         -- item: harpoon ammo
    createBaseItem(ITEM_M16_AMMO, 169, 169);                -- item: M16 ammo
    createBaseItem(ITEM_GRENADEGUN_AMMO, 170, 170);         -- item: grenade launcher ammo

    createBaseItem(ITEM_SMALL_MEDIPACK, 171, 171);          -- item: little MEDIPACK
    createBaseItem(ITEM_LARGE_MEDIPACK, 172, 172);          -- item: large MEDIPACK
    createBaseItem(ITEM_FLARES, 173, 173);                  -- item: flares

    addItem(player, ITEM_COMPASS, 1);                       -- add compas
    
elseif(ver < TR_IV) then
    createBaseItem(ITEM_PASSPORT, 145, 145);                -- item: journal
    createBaseItem(ITEM_COMPASS, 146, 146);                 -- item: compas or timer?
    createBaseItem(ITEM_VIDEO, 181, 181);                   -- item: video
    createBaseItem(ITEM_AUDIO, 182, 182);                   -- item: audio
    createBaseItem(ITEM_CONTROLS, 183, 183);                -- item: control

    createBaseItem(ITEM_PISTOLS, 185, 160);                 -- item: pistols
    createBaseItem(ITEM_SHOTGUN, 186, 161);                 -- item: shotgun
    createBaseItem(ITEM_MAGNUMS, 187, 162);                 -- item: desert eagle
    createBaseItem(ITEM_UZIS, 188, 163);                    -- item: uzi
    createBaseItem(ITEM_HARPOONGUN, 189, 164);              -- item: harpoon
    createBaseItem(ITEM_M16, 190, 165);                     -- item: automatic rifle
    createBaseItem(ITEM_ROCKETGUN, 191, 166);               -- item: quake III arena
    createBaseItem(ITEM_GRENADEGUN, 192, 167);              -- item: grenade launcher

    createBaseItem(ITEM_PISTOL_AMMO, 193, 168);             -- item: pistols ammo
    createBaseItem(ITEM_SHOTGUN_NORMAL_AMMO, 194, 169);     -- item: shotgun ammo
    createBaseItem(ITEM_MAGNUM_AMMO, 195, 170);             -- item: desert eagle ammo
    createBaseItem(ITEM_UZI_AMMO, 196, 171);                -- item: uzi ammo
    createBaseItem(ITEM_HARPOONGUN_AMMO, 197, 172);         -- item: harpoon ammo
    createBaseItem(ITEM_M16_AMMO, 198, 173);                -- item: automatic rifle ammo
    createBaseItem(ITEM_ROCKETGUN, 199, 174);               -- item: quake III arena ammo
    createBaseItem(ITEM_GRENADEGUN, 200, 175);              -- item: grenade launcher ammo

    createBaseItem(ITEM_SMALL_MEDIPACK, 201, 176);          -- item: little MEDIPACK
    createBaseItem(ITEM_LARGE_MEDIPACK, 202, 177);          -- item: large MEDIPACK
    createBaseItem(ITEM_FLARES, 203, 178);                  -- item: flares

    addItem(player, ITEM_COMPASS, 1);                       -- add compas
    
elseif(ver < TR_V) then
    createBaseItem(ITEM_COMPASS, 375, 375);                 -- item: compas
    createBaseItem(ITEM_LOAD, 376, 376);                    -- item: load
    createBaseItem(ITEM_SAVE, 377, 377);                    -- item: save    

    createBaseItem(ITEM_PISTOLS, 349, 349);                 -- item: pistols
    createBaseItem(ITEM_PISTOL_AMMO, 350, 350);             -- item: pistols ammo
    createBaseItem(ITEM_UZIS, 351, 351);                    -- item: uzi
    createBaseItem(ITEM_UZI_AMMO, 352, 352);                -- item: uzi ammo ???
    createBaseItem(ITEM_SHOTGUN, 353, 353);                 -- item: shotgun
    createBaseItem(ITEM_SHOTGUN_NORMAL_AMMO, 354, 354);     -- item: shotgun ammo 1
    createBaseItem(ITEM_SHOTGUN_WIDESHOT_AMMO, 355, 355);   -- item: shotgun ammo 2
    createBaseItem(ITEM_CROSSBOW, 356, 356);                -- item: crossbow
    createBaseItem(ITEM_CROSSBOW_NORMAL_AMMO, 357, 357);    -- item: crossbow ammo 1
    createBaseItem(ITEM_CROSSBOW_POISON_AMMO, 358, 358);    -- item: crossbow ammo 2
    createBaseItem(ITEM_CROSSBOW_EXPLOSIVE_AMMO, 359, 359); -- item: crossbow ammo 3
    -- 360 - one bolt model - USED AS PROJECTILE ONLY!
    createBaseItem(ITEM_GRENADEGUN, 361, 361);              -- item: grenade launcher
    createBaseItem(ITEM_GRENADEGUN_NORMAL_AMMO, 362, 362);  -- item: grenade launcher ammo 1
    createBaseItem(ITEM_GRENADEGUN_SUPER_AMMO, 363, 363);   -- item: grenade launcher ammo 2
    createBaseItem(ITEM_GRENADEGUN_FLASH_AMMO, 364, 364);   -- item: grenade launcher ammo 3
    -- 365 - one grenade model - USED AS PROJECTILE ONLY!
    createBaseItem(ITEM_MAGNUMS, 366, 366);                 -- item: colt
    createBaseItem(ITEM_MAGNUM_AMMO, 367, 367);             -- item: colt ammo

    createBaseItem(ITEM_LARGE_MEDIPACK, 368, 368);          -- item: large MEDIPACK
    createBaseItem(ITEM_SMALL_MEDIPACK, 369, 369);          -- item: little MEDIPACK
    createBaseItem(ITEM_LASERSIGHT, 370, 370);              -- item: optic target
    createBaseItem(ITEM_BINOCULARS, 371, 371);              -- item: binocle
    createBaseItem(ITEM_SINGLE_FLARE, 372, 372);            -- item: one flare
    createBaseItem(ITEM_FLARES, 373, 373);                  -- item: flares
    
	addItem(player, ITEM_COMPASS, 1);                       -- add compas
    
elseif(ver == TR_V) then
    createBaseItem(ITEM_COMPASS, 356, 356);                 -- item: timer
    createBaseItem(ITEM_LOAD, 357, 357);                    -- item: load 1
    createBaseItem(ITEM_SAVE, 358, 358);                    -- item: save 1
    --createBaseItem(4, 359, 359);                          -- item: load 2 - PLAYSTATION MODEL, NOT USED.
    --createBaseItem(5, 360, 360);                          -- item: save 2 - PLAYSTATION MODEL, NOT USED.

    createBaseItem(ITEM_PISTOLS, 334, 334);                 -- item: pistols
    createBaseItem(ITEM_PISTOL_AMMO, 335, 335);             -- item: pistols ammo
    createBaseItem(ITEM_UZIS, 336, 336);                    -- item: uzi
    createBaseItem(ITEM_UZI_AMMO, 337, 337);                -- item: uzi ammo
    createBaseItem(ITEM_SHOTGUN, 338, 338);                 -- item: shotgun
    createBaseItem(ITEM_SHOTGUN_NORMAL_AMMO, 339, 339);     -- item: shotgun ammo 1
    createBaseItem(ITEM_SHOTGUN_WIDESHOT_AMMO, 340, 340);   -- item: shotgun ammo 2
    createBaseItem(ITEM_GRAPPLEGUN, 341, 341);              -- item: climb gun
    createBaseItem(ITEM_GRAPPLEGUN_AMMO, 342, 342);         -- item: climb gun ammo

    createBaseItem(ITEM_M16, 345, 345);                     -- item: automatic rifle
    createBaseItem(ITEM_M16_AMMO, 346, 346);                -- item: automatic rifle ammo
    createBaseItem(ITEM_MAGNUMS, 347, 347);                 -- item: colt / desert eagle
    createBaseItem(ITEM_MAGNUM_AMMO, 348, 348);             -- item: colt / desert eagle ammo

    createBaseItem(ITEM_LARGE_MEDIPACK, 349, 349);          -- item: large MEDIPACK
    createBaseItem(ITEM_SMALL_MEDIPACK, 350, 350);          -- item: little MEDIPACK
    createBaseItem(ITEM_LASERSIGHT, 351, 351);              -- item: optic target
    createBaseItem(ITEM_BINOCULARS, 352, 352);              -- item: binocle

    createBaseItem(ITEM_SINGLE_FLARE, 354, 354);            -- item: one flare
    createBaseItem(ITEM_FLARES, 355, 355);                  -- item: flares
	
	addItem(player, ITEM_COMPASS, 1);                       -- add compas
end

-- test TR_ITEMS
addItem(player, ITEM_COMPASS, 1);
addItem(player, ITEM_PASSPORT, 1);
addItem(player, ITEM_VIDEO, 1);
addItem(player, ITEM_AUDIO, 1);
addItem(player, ITEM_CONTROLS, 1);
addItem(player, ITEM_LOAD, 1);
addItem(player, ITEM_SAVE, 1);
addItem(player, ITEM_SMALL_MEDIPACK, 1);
addItem(player, ITEM_LARGE_MEDIPACK, 1);
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

print("items script loaded");
