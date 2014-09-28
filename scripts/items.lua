
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

    createBaseItem(20, 108, 108);           -- item: little medkit
    createBaseItem(21, 109, 109);           -- item: large medkit

    addItem(0, 1, 1);                       -- add compas
elseif(ver < TR_III) then

elseif(ver < TR_IV) then

elseif(ver < TR_V) then

elseif(ver == TR_V) then

end

print("items script loaded");
