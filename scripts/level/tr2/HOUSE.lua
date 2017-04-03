-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 2, HOUSE.TR2

print("Level script loaded (HOUSE.lua)");

level_PostLoad = function()
    playStream(63);
    setEntityAnim(player, ANIM_TYPE_BASE, 1, 0, 12);
    addCharacterHair(player, getHairSetup(HAIR_TR2));
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;
