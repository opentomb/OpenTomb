-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 4, LAKE

print("Level script loaded (LAKE.lua)");

level_PostLoad = function()
    setEntityCollision(88, false);  -- model 457

    addCharacterHair(player, HAIR_TR4_OLD);
    playStream(110);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;