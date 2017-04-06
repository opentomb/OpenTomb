-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 2, RIG.TR2

print("Level script loaded (RIG.lua)");

level_PostLoad = function()
    addCharacterHair(player, getHairSetup(HAIR_TR2));
    playStream(58);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;
