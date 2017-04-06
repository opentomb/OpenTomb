-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 3, SEWER.TR2

print("Level script loaded (SEWER.lua)");

level_PostLoad = function()
    addCharacterHair(player, getHairSetup(HAIR_TR3));
    playStream(74);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;
