-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 4, JOBY5A

print("Level script loaded (JOBY5A.lua)");

level_PostLoad = function()
    addCharacterHair(player, getHairSetup(HAIR_TR4_OLD));
    playStream(107);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;