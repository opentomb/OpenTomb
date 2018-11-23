-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 2, LIVING.TR2
print("level/tr2/living->level_loaded !");

level_PostLoad = function()
    addCharacterHair(player, getHairSetup(HAIR_TR2));
    playStream(34);
end;

level_PreLoad = function()
    --------------------------------------------------------------------------------
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;