-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 2, CUT1.TR2

print("Level script loaded (CUT1.lua)");

level_PostLoad = function()
    addCharacterHair(player, HAIR_TR2);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;
