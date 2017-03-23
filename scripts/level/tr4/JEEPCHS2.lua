-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 4, JEEPCHS2

print("Level script loaded (JEEPCHS2.lua)");

level_PostLoad = function()
    addCharacterHair(player, HAIR_TR4_OLD);
    playStream(98);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;
