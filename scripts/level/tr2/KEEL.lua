-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 2, KEEL.TR2
print("level/tr2/keel->level_loaded !");

level_PostLoad = function()
    addCharacterHair(player, getHairSetup(HAIR_TR2));
    playStream(31);
end;

level_PreLoad = function()
    --------------------------------------------------------------------------------
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;
