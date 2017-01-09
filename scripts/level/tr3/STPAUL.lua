-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 3, STPAUL.TR2

print("Level script loaded (STPAUL.lua)");

level_PostLoad = function()
    addCharacterHair(player, HAIR_TR3);
    playStream(30);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;
