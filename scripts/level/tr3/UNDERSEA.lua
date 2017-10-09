-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 3 LA, UNDERSEA.TR2

print("Level script loaded (UNDERSEA.lua)");

level_PostLoad = function()
    addCharacterHair(player, getHairSetup(HAIR_TR3));
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;
