-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 5, RICH1

print("Level script loaded (RICH1.lua)");

level_PostLoad = function()
    addCharacterHair(player, getHairSetup(HAIR_TR5_OLD));
    playStream(129);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;
