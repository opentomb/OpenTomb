-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 3, ROOFS.TR2

print("Level script loaded (ROOFS.lua)");

level_PostLoad = function()
    addCharacterHair(player, getHairSetup(HAIR_TR3));
    playStream(73);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;
