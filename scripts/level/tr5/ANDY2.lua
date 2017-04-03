-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 5, ANDY2

print("Level script loaded (ANDY2.lua)");

level_PostLoad = function()
    addCharacterHair(player, getHairSetup(HAIR_TR5_KID_1));
    addCharacterHair(player, getHairSetup(HAIR_TR5_KID_2));
    playStream(117);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;
