-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 2, CATACOMB.TR2
print("level/tr2/catacomb->level_loaded !");

level_PostLoad = function()
    addCharacterHair(player, getHairSetup(HAIR_TR2));
    playStream(31);
end;

level_PreLoad = function()
    --------------------------------------------------------------------------------
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;