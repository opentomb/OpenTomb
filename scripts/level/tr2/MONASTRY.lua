-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 2, MONASTRY.TR2

print("Level script loaded (MONASTRY.lua)");

level_PostLoad = function()
    addCharacterHair(player, getHairSetup(HAIR_TR2));
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;
