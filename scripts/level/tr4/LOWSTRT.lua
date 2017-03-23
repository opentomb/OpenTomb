-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 4, LOWSTRT

print("Level script loaded (LOWSTRT.lua)");

level_PostLoad = function()
    addCharacterHair(player, HAIR_TR4_OLD);
    playStream(102);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;