-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 4, JEEPCHAS

print("Level script loaded (JEEPCHAS.lua)");

level_PostLoad = function()
    addCharacterHair(player, HAIR_TR4_OLD);
    playStream(110);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;
