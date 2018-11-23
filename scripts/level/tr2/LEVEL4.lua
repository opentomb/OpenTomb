-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 2 GM, LEVEL4.TR2
print("level/tr2/level4->level_loaded !");

level_PostLoad = function()
    addCharacterHair(player, getHairSetup(HAIR_TR2));
end;

level_PreLoad = function()
    --------------------------------------------------------------------------------
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;