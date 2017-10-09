-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 3 LA, WILLSDEN.TR2

print("Level script loaded (WILLSDEN.lua)");

level_PostLoad = function()
    addCharacterHair(player, getHairSetup(HAIR_TR3));
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;
