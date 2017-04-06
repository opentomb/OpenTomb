-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 3, QUADCHAS.TR2

print("Level script loaded (QUADCHAS.lua)");

level_PostLoad = function()
    addCharacterHair(player, getHairSetup(HAIR_TR3));
    playStream(34);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;
