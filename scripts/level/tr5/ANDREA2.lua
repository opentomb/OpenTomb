-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 5, ANDREA2

print("Level script loaded (ANDREA2.lua)");

level_PostLoad = function()
    addCharacterHair(player, getHairSetup(HAIR_TR5_OLD));
    playStream(126);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;
