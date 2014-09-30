
-- Here is only usual items. Keys and quest items is a level specific. Add them to level autoexec script. 

local ver = getGameVersion();

if(ver < TR_II) then
    createBaseItem(0, 71, 71);              -- item: journal
    createBaseItem(1, 72, 72);              -- item: compas (in difference levels model id's may be differet)
    createBaseItem(2, 95, 95);              -- item: video
    createBaseItem(3, 96, 96);              -- item: audio
    createBaseItem(4, 97, 97);              -- item: control

    createBaseItem(10, 99, 99);             -- item: pistols
    createBaseItem(11, 100, 100);           -- item: shotgun
    createBaseItem(12, 101, 101);           -- item: automatic pistols
    createBaseItem(13, 102, 102);           -- item: uzi
    createBaseItem(14, 103, 103);           -- item: pistols ammo
    createBaseItem(15, 104, 104);           -- item: shotgun ammo
    createBaseItem(16, 105, 105);           -- item: automatic pistols ammo
    createBaseItem(17, 106, 106);           -- item: uzi ammo

    createBaseItem(30, 108, 108);           -- item: little medkit
    createBaseItem(31, 109, 109);           -- item: large medkit

    addItem(0, 1, 1);                       -- add compas
elseif(ver < TR_III) then
    createBaseItem(0, 120, 120);            -- item: journal
    createBaseItem(1, 121, 121);            -- item: compas or timer?
    createBaseItem(2, 153, 153);            -- item: video
    createBaseItem(3, 154, 154);            -- item: audio
    createBaseItem(4, 155, 155);            -- item: control

    createBaseItem(10, 157, 157);           -- item: pistols
    createBaseItem(11, 158, 158);           -- item: shotgun
    createBaseItem(12, 159, 159);           -- item: automatic pistols
    createBaseItem(13, 160, 160);           -- item: uzi
    createBaseItem(14, 161, 161);           -- item: harpoon
    createBaseItem(15, 162, 162);           -- item: M16
    createBaseItem(16, 163, 163);           -- item: grenade launcher

    createBaseItem(20, 164, 164);          -- item: pistols ammo
    createBaseItem(21, 165, 165);          -- item: shotgun ammo
    createBaseItem(22, 166, 166);          -- item: automatic pistols ammo
    createBaseItem(23, 167, 167);          -- item: uzi ammo
    createBaseItem(24, 168, 168);          -- item: harpoon ammo
    createBaseItem(25, 169, 169);          -- item: M16 ammo
    createBaseItem(26, 170, 170);          -- item: grenade launcher ammo

    createBaseItem(30, 171, 171);          -- item: little medkit
    createBaseItem(31, 172, 172);          -- item: large medkit
    createBaseItem(32, 173, 173);          -- item: flares

    addItem(20, 1, 1);                     -- add compas
elseif(ver < TR_IV) then
    createBaseItem(0, 145, 145);           -- item: journal
    createBaseItem(1, 146, 146);           -- item: compas or timer?
    createBaseItem(2, 181, 181);           -- item: video
    createBaseItem(3, 182, 182);           -- item: audio
    createBaseItem(4, 183, 183);           -- item: control

    createBaseItem(10, 185, 160);          -- item: pistols
    createBaseItem(11, 186, 161);          -- item: shotgun
    createBaseItem(12, 187, 162);          -- item: desert eagle
    createBaseItem(13, 188, 163);          -- item: uzi
    createBaseItem(14, 189, 164);          -- item: harpoon
    createBaseItem(15, 190, 165);          -- item: automatic rifle
    createBaseItem(16, 191, 166);          -- item: quake III arena
    createBaseItem(17, 192, 167);          -- item: grenade launcher

    createBaseItem(20, 193, 168);          -- item: pistols ammo
    createBaseItem(21, 194, 169);          -- item: shotgun ammo
    createBaseItem(22, 195, 170);          -- item: desert eagle ammo
    createBaseItem(23, 196, 171);          -- item: uzi ammo
    createBaseItem(24, 197, 172);          -- item: harpoon ammo
    createBaseItem(25, 198, 173);          -- item: automatic rifle ammo
    createBaseItem(26, 199, 174);          -- item: quake III arena ammo
    createBaseItem(27, 200, 175);          -- item: grenade launcher ammo

    createBaseItem(30, 201, 176);          -- item: little medkit
    createBaseItem(31, 202, 177);          -- item: large medkit
    createBaseItem(32, 203, 178);          -- item: flares

    addItem(3, 1, 1);                      -- add compas
elseif(ver < TR_V) then
    createBaseItem(1, 375, 375);           -- item: compas
    createBaseItem(2, 376, 376);           -- item: load
    createBaseItem(3, 377, 377);           -- item: save    

    createBaseItem(10, 349, 349);          -- item: pistols
    createBaseItem(11, 350, 350);          -- item: pistols ammo
    createBaseItem(12, 351, 351);          -- item: uzi
    createBaseItem(13, 352, 352);          -- item: uzi ammo ???
    createBaseItem(14, 353, 353);          -- item: shotgun
    createBaseItem(15, 354, 354);          -- item: shotgun ammo 1
    createBaseItem(16, 355, 355);          -- item: shotgun ammo 2
    createBaseItem(17, 356, 356);          -- item: crossbow
    createBaseItem(18, 357, 357);          -- item: crossbow ammo 1
    createBaseItem(19, 358, 358);          -- item: crossbow ammo 2
    createBaseItem(20, 359, 359);          -- item: crossbow ammo 3
    -- 360 - one bolt model
    createBaseItem(21, 361, 361);          -- item: grenade launcher
    createBaseItem(22, 362, 362);          -- item: grenade launcher ammo 1
    createBaseItem(23, 363, 363);          -- item: grenade launcher ammo 2
    createBaseItem(24, 364, 364);          -- item: grenade launcher ammo 3
    -- 365 - one grenade model
    createBaseItem(25, 366, 366);          -- item: colt
    createBaseItem(26, 367, 367);          -- item: colt ammo

    createBaseItem(30, 368, 368);          -- item: large medkit
    createBaseItem(31, 369, 369);          -- item: little medkit
    createBaseItem(32, 370, 370);          -- item: optic target
    createBaseItem(33, 371, 371);          -- item: binocle
    createBaseItem(34, 372, 372);          -- item: one flare
    createBaseItem(35, 373, 373);          -- item: flares
    --addItem(51, 1, 1);                     -- add compas Lara has different id in different levels =(
elseif(ver == TR_V) then
    createBaseItem(1, 356, 356);           -- item: timer
    createBaseItem(2, 357, 357);           -- item: load 1
    createBaseItem(3, 358, 358);           -- item: save 1
    createBaseItem(4, 359, 359);           -- item: load 2
    createBaseItem(5, 360, 360);           -- item: save 2

    createBaseItem(10, 334, 334);          -- item: pistols
    createBaseItem(11, 335, 335);          -- item: pistols ammo
    createBaseItem(12, 336, 336);          -- item: uzi
    createBaseItem(13, 337, 337);          -- item: uzi ammo
    createBaseItem(14, 338, 338);          -- item: shotgun
    createBaseItem(15, 339, 339);          -- item: shotgun ammo 1
    createBaseItem(16, 340, 340);          -- item: shotgun ammo 2
    createBaseItem(17, 341, 341);          -- item: climb gun
    createBaseItem(18, 342, 342);          -- item: climb gun ammo

    createBaseItem(20, 345, 345);          -- item: automatic rifle
    createBaseItem(21, 346, 346);          -- item: automatic rifle ammo
    createBaseItem(22, 347, 347);          -- item: colt / desert eagle
    createBaseItem(23, 348, 348);          -- item: colt / desert eagle ammo

    createBaseItem(30, 349, 349);          -- item: large medkit
    createBaseItem(31, 350, 350);          -- item: little medkit
    createBaseItem(32, 351, 351);          -- item: optic target
    createBaseItem(33, 352, 352);          -- item: binocle

    createBaseItem(35, 354, 354);          -- item: one flare
    createBaseItem(36, 355, 355);          -- item: flares
end

print("items script loaded");
