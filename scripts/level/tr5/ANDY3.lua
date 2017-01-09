-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 5, ANDY3

print("Level script loaded (ANDY3.lua)");

level_PostLoad = function()
    addCharacterHair(player, HAIR_TR5_KID_1);
    addCharacterHair(player, HAIR_TR5_KID_2);
    playStream(124);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;
