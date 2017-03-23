-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 2, SKIDO.TR2

print("Level script loaded (SKIDO.lua)");

level_PostLoad = function()
    addCharacterHair(player, HAIR_TR2);
    playStream(33);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;
