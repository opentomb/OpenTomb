-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 2, CUT3.TR2
print("level/tr2/cut3->cutscene_loaded !");

level_PostLoad = function()
    addCharacterHair(player, getHairSetup(HAIR_TR2));
end;

level_PreLoad = function()
    --------------------------------------------------------------------------------
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;