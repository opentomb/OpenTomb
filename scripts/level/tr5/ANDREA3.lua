-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 5, ANDREA3

print("Level script loaded (ANDREA3.lua)");

level_PostLoad = function()
    addCharacterHair(player, HAIR_TR5_OLD);
    playStream(118);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;
