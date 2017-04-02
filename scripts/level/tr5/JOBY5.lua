-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 5, JOBY5

print("Level script loaded (JOBY5.lua)");

level_PostLoad = function()
    addCharacterHair(player, getHairSetup(HAIR_TR5_OLD));
    playStream(121);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;
