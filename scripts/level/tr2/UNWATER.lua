-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 2, UNWATER.TR2
print("level/tr2/unwater->level_loaded !");

level_PostLoad = function()
    addCharacterHair(player, getHairSetup(HAIR_TR2));
    playStream(34);
end;

level_PreLoad = function()
    --------------------------------------------------------------------------------
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;