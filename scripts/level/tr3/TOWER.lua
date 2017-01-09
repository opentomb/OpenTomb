-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 3, TOWER.TR2

print("Level script loaded (TOWER.lua)");

level_PostLoad = function()
    addCharacterHair(player, HAIR_TR3);
    playStream(31);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;
