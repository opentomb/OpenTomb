-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 4, CSPLIT1

print("Level script loaded (CSPLIT1.lua)");

level_PostLoad = function()
    addCharacterHair(player, HAIR_TR4_OLD);
    playStream(108);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;
