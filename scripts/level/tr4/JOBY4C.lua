-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 4, JOBY4C

print("Level script loaded (JOBY4C.lua)");

level_PostLoad = function()
    addCharacterHair(player, HAIR_TR4_OLD);
    playStream(111);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;