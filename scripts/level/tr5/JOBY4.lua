-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 5, JOBY4

print("Level script loaded (JOBY4.lua)");

level_PostLoad = function()
    addCharacterHair(player, getHairSetup(HAIR_TR5_OLD));
    playStream(127);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;
