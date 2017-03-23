-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 4, CSPLIT2

print("Level script loaded (CSPLIT2.lua)");

level_PostLoad = function()
    addCharacterHair(player, HAIR_TR4_OLD);
    playStream(108);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;
