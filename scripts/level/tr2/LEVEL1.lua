-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 2 GM, LEVEL1.TR2

print("Level script loaded (LEVEL1.lua)");

level_PostLoad = function()
    addCharacterHair(player, HAIR_TR2);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;
