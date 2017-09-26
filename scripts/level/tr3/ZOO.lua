-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 3 LA, ZOO.TR2

print("Level script loaded (ZOO.lua)");

level_PostLoad = function()
    addCharacterHair(player, getHairSetup(HAIR_TR3));
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;
