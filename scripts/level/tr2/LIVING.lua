-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 2, LIVING.TR2

print("Level script loaded (LIVING.lua)");

level_PostLoad = function()
    addCharacterHair(player, getHairSetup(HAIR_TR2));
    playStream(34);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;
