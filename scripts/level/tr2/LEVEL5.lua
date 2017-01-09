-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 2 GM, LEVEL5.TR2

print("Level script loaded (LEVEL5.lua)");

level_PostLoad = function()
    addCharacterHair(player, HAIR_TR2);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;
