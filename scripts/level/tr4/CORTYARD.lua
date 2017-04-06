-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 4, CORTYYARD

print("Level script loaded (CORTYYARD.lua)");

level_PostLoad = function()
    addCharacterHair(player, getHairSetup(HAIR_TR4_OLD));
    playStream(102);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;
