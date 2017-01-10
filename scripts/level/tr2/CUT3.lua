-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 2, CUT3.TR2

print("Level script loaded (CUT3.lua)");

level_PostLoad = function()
    addCharacterHair(player, HAIR_TR2);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;
