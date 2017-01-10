-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 4, JOBY3A

print("Level script loaded (JOBY3A.lua)");

level_PostLoad = function()
    addCharacterHair(player, HAIR_TR4_OLD);
    playStream(111);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;
