-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 4, NUTRENCH

print("Level script loaded (NUTRENCH.lua)");

level_PostLoad = function()
    addCharacterHair(player, HAIR_TR4_OLD);
    playStream(102);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;
