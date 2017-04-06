-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 4, HIGHSTRT

print("Level script loaded (HIGHSTRT.lua)");

level_PostLoad = function()
    addCharacterHair(player, getHairSetup(HAIR_TR4_OLD));
    playStream(102);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;
