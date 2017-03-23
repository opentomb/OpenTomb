-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 2, ICECAVE.TR2

print("Level script loaded (ICECAVE.lua)");

level_PostLoad = function()
    addCharacterHair(player, HAIR_TR2);
    playStream(31);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;
