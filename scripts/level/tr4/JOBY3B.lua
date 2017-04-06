-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 4, JOBY3B

print("Level script loaded (JOBY3B.lua)");

level_PostLoad = function()
    addCharacterHair(player, getHairSetup(HAIR_TR4_OLD));
    playStream(107);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;
