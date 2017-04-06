-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 4, JOBY1A

print("Level script loaded (JOBY1A.lua)");

level_PostLoad = function()
    addCharacterHair(player, getHairSetup(HAIR_TR4_OLD));
    playStream(111);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;
