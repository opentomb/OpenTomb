-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 2, DECK.TR2
print("level/tr2/deck->level_loaded !");

level_PostLoad = function()
    addCharacterHair(player, getHairSetup(HAIR_TR2));
    playStream(31);
end;

level_PreLoad = function()
    --------------------------------------------------------------------------------
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;