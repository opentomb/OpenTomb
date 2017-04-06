-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 2, XIAN.TR2

print("Level script loaded (XIAN.lua)");

level_PostLoad = function()
    addCharacterHair(player, getHairSetup(HAIR_TR2));
    playStream(59)
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;
