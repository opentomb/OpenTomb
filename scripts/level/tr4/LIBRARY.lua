-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 4, LIBRARY

print("Level script loaded (LIBRARY.lua)");

level_PostLoad = function()
    addCharacterHair(player, getHairSetup(HAIR_TR4_OLD));
    playStream(108);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;