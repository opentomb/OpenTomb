-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 4, TRAIN

print("Level script loaded (TRAIN.lua)");

level_PostLoad = function()
    addCharacterHair(player, getHairSetup(HAIR_TR4_OLD));
    playStream(106);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;
