-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 3, RAPIDS.TR2

print("Level script loaded (RAPIDS.lua)");

level_PostLoad = function()
    addCharacterHair(player, getHairSetup(HAIR_TR3));
    playStream(36);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;
