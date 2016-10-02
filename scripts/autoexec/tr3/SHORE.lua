function autoexec_PreLoad()
    static_tbl[30] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX_BASE};         
end;

function autoexec_PostLoad()
    addCharacterHair(player, HAIR_TR3);

    moveEntityLocal(player,0,0,256);            -- Raise Lara from the floor
    setEntityMoveType(player, MOVE_UNDERWATER); -- Change to underwater state
    setEntityAnim(player, 108);                 -- Force underwater animation

    playStream(32);
end;