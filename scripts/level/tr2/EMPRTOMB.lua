-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 2, EMPRTOMB.TR2
print("level/tr2/emprtomb->level_loaded !");

level_PostLoad = function()
    addCharacterHair(player, getHairSetup(HAIR_TR2));
    playStream(59);
end;

level_PreLoad = function()
    --------------------------------------------------------------------------------
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;