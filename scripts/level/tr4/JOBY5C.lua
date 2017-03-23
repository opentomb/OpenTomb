-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 4, JOBY5C

print("Level script loaded (JOBY5C.lua)");

level_PostLoad = function()
    addCharacterHair(player, HAIR_TR4_OLD);
    playStream(108);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;