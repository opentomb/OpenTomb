-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 5, JOBY2

print("Level script loaded (JOBY2.lua)");

level_PostLoad = function()
    addCharacterHair(player, HAIR_TR5_OLD);
    playStream(130);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;
