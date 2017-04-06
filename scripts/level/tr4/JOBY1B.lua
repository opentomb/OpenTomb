-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 4, JOBY1B

print("Level script loaded (JOBY1B.lua)");

level_PostLoad = function()
    addCharacterHair(player, getHairSetup(HAIR_TR4_OLD));
    playStream(111);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;
