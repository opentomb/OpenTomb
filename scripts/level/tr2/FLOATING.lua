-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 2, FLOATING.TR2

print("Level script loaded (FLOATING.lua)");

level_PostLoad = function()
    addCharacterHair(player, HAIR_TR2);
    playStream(59);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;
