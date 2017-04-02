-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 3, SHORE.TR2

print("Level script loaded (SHORE.lua)");

level_PostLoad = function()
    addCharacterHair(player, getHairSetup(HAIR_TR3));

    moveEntityLocal(player, 0, 0, 256);                -- Raise Lara from the floor
    setEntityMoveType(player, MOVE_UNDERWATER);     -- Change to underwater state
    setEntityAnim(player, ANIM_TYPE_BASE, 108, 0);  -- Force underwater animation

    playStream(32);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
    static_tbl[30] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_BOX_BASE};
end;
