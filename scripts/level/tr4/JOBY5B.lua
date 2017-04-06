-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 4, JOBY5B

print("Level script loaded (JOBY5B.lua)");

level_PostLoad = function()
    addCharacterHair(player, getHairSetup(HAIR_TR4_OLD));
    playStream(97);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;
