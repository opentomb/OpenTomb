-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 4, LIBEND

print("Level script loaded (LIBEND.lua)");

level_PostLoad = function()
    addCharacterHair(player, getHairSetup(HAIR_TR4_OLD));
    playStream(109);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;