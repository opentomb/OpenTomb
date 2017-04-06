-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 3, TRIBOSS.TR2

print("Level script loaded (TRIBOSS.lua)");

level_PostLoad = function()
    addCharacterHair(player, getHairSetup(HAIR_TR3));
    playStream(30);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;
