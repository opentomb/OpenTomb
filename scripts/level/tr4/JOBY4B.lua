-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 4, JOBY4B

print("Level script loaded (JOBY4B.lua)");

level_PostLoad = function()
    addCharacterHair(player, HAIR_TR4_OLD);
    playStream(107);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;
