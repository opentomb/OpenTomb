-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 2, UNWATER.TR2

print("Level script loaded (UNWATER.lua)");

level_PostLoad = function()
    addCharacterHair(player, HAIR_TR2);
    playStream(34);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;
