-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 3 GM, LEVEL1.TR2

print("Level script loaded (LEVEL3.lua)");

level_PostLoad = function()
    addCharacterHair(player, getHairSetup(HAIR_TR2));
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;
