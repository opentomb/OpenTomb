-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 5, RICHCUT

print("Level script loaded (RICHCUT.lua)");

level_PostLoad = function()
    addCharacterHair(player, HAIR_TR5_OLD);
    playStream(129);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;
