-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 4, PALACES

print("Level script loaded (PALACES.lua)");

level_PostLoad = function()
    addCharacterHair(player, getHairSetup(HAIR_TR4_OLD));
    playStream(107);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;
