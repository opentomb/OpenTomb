-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 4, SEMER

print("Level script loaded (SEMER.lua)");

level_PostLoad = function()
    addCharacterHair(player, getHairSetup(HAIR_TR4_OLD));
    playStream(107);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
    -- PLANT statics (as listed in OBJECTS.H from TRLE)
    static_tbl[11] = {coll = COLLISION_NONE,      shape = COLLISION_SHAPE_TRIMESH}; -- Stairs
end;