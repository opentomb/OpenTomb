-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 2, BOAT.TR2

print("Level script loaded (BOAT.lua)");

level_PostLoad = function()
    addCharacterHair(player, HAIR_TR2);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;
