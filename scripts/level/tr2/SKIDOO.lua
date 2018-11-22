-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 2, SKIDO.TR2
print("level/tr2/skido->level_loaded !");

level_PostLoad = function()
    addCharacterHair(player, getHairSetup(HAIR_TR2));
    playStream(33);
end;

level_PreLoad = function()
    --------------------------------------------------------------------------------
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;