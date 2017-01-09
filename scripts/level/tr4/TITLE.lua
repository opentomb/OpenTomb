-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 4, TITLE

print("Level script loaded (TITLE.lua)");

level_PostLoad = function()
    addCharacterHair(player, HAIR_TR4_OLD);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;
