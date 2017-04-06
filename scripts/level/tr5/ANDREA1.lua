-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 5, ANDREA1

print("Level script loaded (ANDREA1.lua)");

level_PostLoad = function()
    addCharacterHair(player, getHairSetup(HAIR_TR5_OLD));
    playStream(128);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;
