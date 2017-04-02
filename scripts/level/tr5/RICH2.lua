-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 5, RICH2

print("Level script loaded (RICH2.lua)");

level_PostLoad = function()
    addCharacterHair(player, getHairSetup(HAIR_TR5_OLD));
    playStream(129);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;
