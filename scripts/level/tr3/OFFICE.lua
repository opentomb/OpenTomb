-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 3, OFFICE.TR2

print("Level script loaded (OFFICE.lua)");

level_PostLoad = function()
    addCharacterHair(player, HAIR_TR3);
    playStream(78);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;
